#include "GraphicsSystem.h"
#include "Initiative\Systems\GraphicsSystem\Components\WindowHandle.h"

#include "glfw3.h"

#include "Initiative\FileUtils.h"

#include "Initiative\math.h"



namespace itv
{
	struct Vertex {
		math::vec2 pos;
		math::vec3 color;

		

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			return attributeDescriptions;
		}

	};

	struct UniformBufferObject {
		math::mat4 model;
		math::mat4 view;
		math::mat4 proj;
	};

	const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	};

	const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
	};

	constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	GraphicsSystem::GraphicsSystem()
	{
		

	}

	GraphicsSystem::~GraphicsSystem()
	{
		vkDeviceWaitIdle(mLogicalDevice);
		cleanup();
	}

	void GraphicsSystem::cleanup()
	{
		cleanupSwapChain();

		vkDestroyDescriptorSetLayout(mLogicalDevice, descriptorSetLayout, nullptr);

		vkDestroyBuffer( mLogicalDevice, mIndexBuffer, nullptr);
		vkFreeMemory( mLogicalDevice, mIndexBufferMemory, nullptr);

		vkDestroyBuffer( mLogicalDevice, mVertexBuffer, nullptr);
		vkFreeMemory( mLogicalDevice, mVertexBufferMemory, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		{
			vkDestroySemaphore(mLogicalDevice, mImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(mLogicalDevice, mRenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(mLogicalDevice, mInFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(mLogicalDevice, mCommandPool, nullptr);

		vkDestroyDevice( mLogicalDevice, nullptr );

		if constexpr (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(mInstance, mSurface, nullptr);

		vkDestroyInstance(mInstance, nullptr);
	}

	void GraphicsSystem::cleanupSwapChain()
	{
		for (size_t i = 0; i < mSwapChainImages.size(); i++) {
			vkDestroyBuffer( mLogicalDevice, mUniformBuffers[i], nullptr);
			vkFreeMemory(mLogicalDevice, mUniformBuffersMemory[i], nullptr);
		}
		vkDestroyDescriptorPool(mLogicalDevice, mDescriptorPool, nullptr);

		vkFreeCommandBuffers(mLogicalDevice, mCommandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		for (auto framebuffer : mSwapChainFramebuffers) {
			vkDestroyFramebuffer(mLogicalDevice, framebuffer, nullptr);
		}

		vkDestroyPipeline(mLogicalDevice, mGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(mLogicalDevice, mPipelineLayout, nullptr);
		vkDestroyRenderPass(mLogicalDevice, mRenderPass, nullptr);

		for (auto imageView : mSwapChainImageViews) {
			vkDestroyImageView(mLogicalDevice, imageView, nullptr);
		}


		vkDestroySwapchainKHR(mLogicalDevice, mSwapChain, nullptr);
	}

	std::vector<const char*> GraphicsSystem::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if constexpr( enableValidationLayers ) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	bool GraphicsSystem::checkValidationLayerSupport()
	{
		//[1] Get Available Layers

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		//[2] Ensure that each layer in mValidationLayer is available

		for (const char* layerName : mValidationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	void GraphicsSystem::initVulkan()
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		findSuitablePhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createFrameBuffers();
		createCommandPool();
		createVertexBuffers();
		createIndexBuffers();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
	}

	void GraphicsSystem::createInstance()
	{
		if constexpr( enableValidationLayers ) 
		{
			if (!checkValidationLayerSupport())
			{
				throw std::runtime_error("validation layers requested, but not available!");
			}
		}

		//[1] Init App Info
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Initiative";
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		appInfo.pEngineName = "Initiative Engine";
		appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		//[2] Init Create Info using appInfo
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

			//[2.1] Fill in validation layer names if in debug

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

		if constexpr(enableValidationLayers) 
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
			createInfo.ppEnabledLayerNames = mValidationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		//[3] Get GLFW extensions to fill in create info
		std::vector<const char*> requiredExtensions = getRequiredExtensions();

		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		//[4] Create VkInstance using createInfo
		VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance);

		if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
		else
		{
			ITV_LOG("VULKAN: Create Instance Successfull!");
		}

		//[5] Log Vulkan Extentions

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		ITV_LOG("VULKAN: Available Extensions: ");

		for (auto extension : extensions)
		{
			ITV_LOG("	{0}",extension.extensionName);
		}

	}

	void GraphicsSystem::createSurface()
	{
		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(mWindow->GetWindowPtr());

		if ( glfwCreateWindowSurface(mInstance, glfwWindow, nullptr, &mSurface) )
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void GraphicsSystem::findSuitablePhysicalDevice()
	{
		//[1] Find Number of devices that support vulkan in this machine
		uint32_t physicalDeviceCount;

		vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr);

		//[2] If There aren't any we can't continue
		if (physicalDeviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		//[3] Find all physical devices
		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

		vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, physicalDevices.data());

		//[4] Select first suitable device
		for (VkPhysicalDevice device : physicalDevices)
		{
			if (isDeviceSuitable(device))
			{
				mPhysicalDevice = device;
				break;
			}
		}

		if (mPhysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}



	}

	void GraphicsSystem::createLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);

		//[1] Fill in information about the creation of the device queue

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };


		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		//[2] Specify physical device features we want to use

		VkPhysicalDeviceFeatures deviceFeatures{};

		//[3] Fill in initialization info for logical device

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>( queueCreateInfos.size() );

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>( mDeviceExtensions.size() );
		createInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
			createInfo.ppEnabledLayerNames = mValidationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mLogicalDevice) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(mLogicalDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
		vkGetDeviceQueue(mLogicalDevice, indices.presentFamily.value(), 0, &mPresentQueue);
	}

	void GraphicsSystem::createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(mPhysicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		bool bIsSwapChainImgCountNotInfinite = swapChainSupport.capabilities.maxImageCount > 0;
		bool bIsCurrentCountBiggerThanMaxImgCount = imageCount > swapChainSupport.capabilities.maxImageCount;

		if (bIsSwapChainImgCountNotInfinite && bIsCurrentCountBiggerThanMaxImgCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mSurface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies( mPhysicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; 
			createInfo.pQueueFamilyIndices = nullptr; 
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR( mLogicalDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR( mLogicalDevice, mSwapChain, &imageCount, nullptr);
		mSwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR( mLogicalDevice, mSwapChain, &imageCount, mSwapChainImages.data());

		mSwapChainImageFormat = surfaceFormat.format;
		mSwapChainExtent = extent;

	}

	void GraphicsSystem::createImageViews()
	{
		mSwapChainImageViews.resize(mSwapChainImages.size());

		for (size_t i = 0; i < mSwapChainImages.size(); i++) 
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = mSwapChainImages[i];

			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = mSwapChainImageFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(mLogicalDevice, &createInfo, nullptr, &mSwapChainImageViews[i]) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to create image views!");
			}

		}

	}

	void GraphicsSystem::createRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = mSwapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // (in this case, the previous subpass)
		dependency.dstSubpass = 0; // current subpass 

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		

		if (vkCreateRenderPass(mLogicalDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void GraphicsSystem::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(mLogicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

	}

	void GraphicsSystem::createGraphicsPipeline()
	{
		//[1] Initialize Vertex and Fragment Shaders
		auto vertShaderCode = readFile("Shaders/vert.spv");
		auto fragShaderCode = readFile("Shaders/frag.spv");

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();


		//[2] Describe how vertex input will be organized
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		//[2] Describe how the geometry will be drawn
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		//[3] Initialize the viewport
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width =  (float)mSwapChainExtent.width;
		viewport.height = (float)mSwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		//[4] Initialize Scissor Triangle
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = mSwapChainExtent;

		//[5] Initialize Viewport state using previously initialized VkViewport and VkRect2D
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		//[6] Initialize Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		//[7] Initialize Multisampling
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


		//[8] Turn off both methods of color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = 
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT 
			| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		


		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkDynamicState dynamicStates[] = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		if (vkCreatePipelineLayout(mLogicalDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = mPipelineLayout;

		pipelineInfo.renderPass = mRenderPass;
		pipelineInfo.subpass = 0;

		if ( vkCreateGraphicsPipelines(mLogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline ) != VK_SUCCESS ) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(mLogicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(mLogicalDevice, vertShaderModule, nullptr);
	}

	void GraphicsSystem::createFrameBuffers()
	{
		mSwapChainFramebuffers.resize( mSwapChainImageViews.size() );

		for (size_t i = 0; i < mSwapChainImageViews.size(); i++)
		{
			VkImageView attachments[] = { mSwapChainImageViews[i] };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = mRenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = mSwapChainExtent.width;
			framebufferInfo.height = mSwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer( mLogicalDevice, &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}


		}
	}

	void GraphicsSystem::createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(mPhysicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = 0; // Optional

		if (vkCreateCommandPool(mLogicalDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void GraphicsSystem::createIndexBuffers()
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(mLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(mLogicalDevice, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mIndexBuffer, mIndexBufferMemory);

		copyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

		vkDestroyBuffer(mLogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(mLogicalDevice, stagingBufferMemory, nullptr);
	}

	void GraphicsSystem::createVertexBuffers()
	{
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(mLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(mLogicalDevice, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mVertexBuffer, mVertexBufferMemory);

		copyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

		vkDestroyBuffer( mLogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory( mLogicalDevice, stagingBufferMemory, nullptr);
	}

	void GraphicsSystem::createUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		mUniformBuffers.resize( mSwapChainImages.size() );
		mUniformBuffersMemory.resize( mSwapChainImages.size() );

		for (size_t i = 0; i < mSwapChainImages.size(); i++) 
		{
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				mUniformBuffers[i], mUniformBuffersMemory[i]);
		}
	}

	void GraphicsSystem::createDescriptorPool()
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>( mSwapChainImages.size() );

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;

		poolInfo.maxSets = static_cast<uint32_t>( mSwapChainImages.size() );

		if (vkCreateDescriptorPool(mLogicalDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}

	}

	void GraphicsSystem::createDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts( mSwapChainImages.size(), descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>( mSwapChainImages.size());
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize( mSwapChainImages.size());
		if (vkAllocateDescriptorSets( mLogicalDevice, &allocInfo, descriptorSets.data() ) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < mSwapChainImages.size(); i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = mUniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;

			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(mLogicalDevice, 1, &descriptorWrite, 0, nullptr);


		}
	}

	void GraphicsSystem::createCommandBuffers()
	{
		commandBuffers.resize( mSwapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = mCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers( mLogicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = mRenderPass;
			renderPassInfo.framebuffer = mSwapChainFramebuffers[i];

			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = mSwapChainExtent;

			VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

			VkBuffer vertexBuffers[] = { mVertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffers[i], mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
				mPipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}


	}

	void GraphicsSystem::createSyncObjects()
	{
		mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		mImagesInFlight.resize(mSwapChainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if ( vkCreateSemaphore(mLogicalDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i] ) != VK_SUCCESS ||
				 vkCreateSemaphore(mLogicalDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i] ) != VK_SUCCESS || 
				vkCreateFence(mLogicalDevice, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS ){

				throw std::runtime_error("failed to create semaphores for a frame!");
			}
		}
	}

	void GraphicsSystem::recreateSwapChain()
	{
		GLFWwindow* windowPtr = static_cast<GLFWwindow * > ( mWindow->GetWindowPtr() );

		int width = 0, height = 0;
		glfwGetFramebufferSize(windowPtr, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(windowPtr, &width, &height);
			glfwWaitEvents();
		}



		vkDeviceWaitIdle(mLogicalDevice);

		cleanupSwapChain();

		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFrameBuffers();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();

	}

	bool GraphicsSystem::isDeviceSuitable(VkPhysicalDevice deviceToCheck)
	{
		QueueFamilyIndices indices = findQueueFamilies(deviceToCheck);

		bool extensionsSupported = isExtensionsSupportedByDevice(deviceToCheck);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(deviceToCheck);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	bool GraphicsSystem::isExtensionsSupportedByDevice(VkPhysicalDevice deviceToCheck)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(deviceToCheck, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(deviceToCheck, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());

		for (VkExtensionProperties& extensionPropertyAvailable : availableExtensions)
		{
			requiredExtensions.erase(extensionPropertyAvailable.extensionName);
		}

		return requiredExtensions.empty();
	}

	GraphicsSystem::SwapChainSupportDetails GraphicsSystem::querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	GraphicsSystem::QueueFamilyIndices GraphicsSystem::findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		//[1] Find all queue families
		uint32_t queuFamiliyCount;
		vkGetPhysicalDeviceQueueFamilyProperties( device, &queuFamiliyCount, nullptr );

		std::vector<VkQueueFamilyProperties> queueFamilies(queuFamiliyCount);

		vkGetPhysicalDeviceQueueFamilyProperties( device, &queuFamiliyCount, queueFamilies.data() );

		for (size_t i = 0; i < queueFamilies.size() ; i++)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);

			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if ( indices.isComplete() )
			{
				break;
			}

		}



		return indices;
	}

	VkSurfaceFormatKHR GraphicsSystem::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR GraphicsSystem::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D GraphicsSystem::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;

			GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(mWindow->GetWindowPtr());
			glfwGetFramebufferSize(glfwWindow, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	VkShaderModule GraphicsSystem::createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(mLogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	void GraphicsSystem::setupDebugMessenger()
	{
		if constexpr (!enableValidationLayers) return;

		//[1] Initialize Info for craeting debug messenger
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		//[2] Create Debug Messenger
		if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	VkResult GraphicsSystem::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void GraphicsSystem::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	void GraphicsSystem::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
	}

	uint32_t GraphicsSystem::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void GraphicsSystem::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
		VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(mLogicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(mLogicalDevice, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(mLogicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}
		
		vkBindBufferMemory( mLogicalDevice, buffer, bufferMemory, 0);
	}

	void GraphicsSystem::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = mCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(mLogicalDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit( mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(mGraphicsQueue);

		vkFreeCommandBuffers(mLogicalDevice, mCommandPool, 1, &commandBuffer);
	}

	void GraphicsSystem::updateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = math::rotate(math::mat4(1.0f), time * math::radians(90.0f), math::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = math::lookAt(math::vec3(2.0f, 2.0f, 2.0f), math::vec3(0.0f, 0.0f, 0.0f), math::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = math::perspective(math::radians(45.0f), mSwapChainExtent.width / (float) mSwapChainExtent.height, 0.1f, 10.0f);

		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(mLogicalDevice, mUniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(mLogicalDevice, mUniformBuffersMemory[currentImage]);


	}

	VKAPI_ATTR VkBool32 VKAPI_CALL GraphicsSystem::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) 
	{

		ITV_LOG("validation layer: {0}", pCallbackData->pMessage);

		return VK_FALSE;
	}

	void GraphicsSystem::RegisterComponents()
	{
		RegisterComponent<WindowHandle>();

	}

	void GraphicsSystem::BeginRun()
	{
		Entity worldEnt = GetWorldEntity();

		auto optionalWindowHandle = worldEnt.GetComponent<WindowHandle>();
		assert(optionalWindowHandle.has_value());

		mWindow = worldEnt.GetComponent<WindowHandle>().value().get().window;

		initVulkan();
	}

	void GraphicsSystem::Run()
	{
		vkWaitForFences(mLogicalDevice, 1, &mInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
		

		uint32_t imageIndex;
		vkAcquireNextImageKHR
		(mLogicalDevice, mSwapChain, UINT64_MAX, mImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		
		

		if (mImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences( mLogicalDevice, 1, &mImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		mImagesInFlight[imageIndex] = mInFlightFences[currentFrame];

		updateUniformBuffer(imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(mLogicalDevice, 1, &mInFlightFences[currentFrame]);

		if (vkQueueSubmit( mGraphicsQueue, 1, &submitInfo, mInFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		VkResult result = vkQueuePresentKHR(mPresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}


		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

}

