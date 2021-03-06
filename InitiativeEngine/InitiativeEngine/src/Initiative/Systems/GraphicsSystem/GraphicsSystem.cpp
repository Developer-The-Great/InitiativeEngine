#include "GraphicsSystem.h"
#include "Initiative\Systems\GraphicsSystem\Components\WindowHandle.h"
#include "Initiative\Systems\GraphicsSystem\Components\Camera.h"
#include "Initiative\GenericComponents\GenericComponents.h"
#include "Initiative\Systems\GraphicsSystem\Components\Mesh.h"
#include "Initiative\SystemUtils\GraphicsUtils\GraphicsObjectLoader.h"

#include <glfw3.h>

#include "Initiative\FileUtils.h"

#include "Initiative\math.h"



#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

constexpr int MAX_RENDER_OBJECTS = 1000;
constexpr int MAX_IMAGE_DESCRIPTORS = 100;

namespace itv
{
	
	struct GPUObjectData
	{
		math::mat4 modelMatrix;
	};

	struct UniformBufferObject {
		math::mat4 model;
		math::mat4 view;
		math::mat4 proj;
	};

	//std::vector<Vertex> vertices;
	//std::vector<uint32_t> indices;

	constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	int GraphicsSystem::LoadMeshIntoGraphicsSystem(const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices)
	{
		//get index for new vertex buffer
		int index = mTriangleMeshBuffers.size();

		mTriangleMeshBuffers.push_back( VulkanVertexBuffer() );

		//create vertex buffer
		createVertexBuffers( mTriangleMeshBuffers[index].mVertexBuffer, mTriangleMeshBuffers[index].mVertexBufferMemory,vertices );
		createIndexBuffers( mTriangleMeshBuffers[index].mIndexBuffer, mTriangleMeshBuffers[index].mIndexBufferMemory, indices);

		return index;
		
	}

	int GraphicsSystem::LoadTextureIntoGraphicsSystem(unsigned char* pixels, int texWidth, int texHeight)
	{
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		VkImage newImage;
		VkDeviceMemory newImageMemory;

		createBuffer(imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(mLogicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(mLogicalDevice, stagingBufferMemory);

		createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, newImage, newImageMemory);

		transitionImageLayout(newImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(stagingBuffer, newImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

		transitionImageLayout(newImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(mLogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(mLogicalDevice, stagingBufferMemory, nullptr);

		mImagesAllocated.emplace_back( ImageAllocation{ newImage ,newImageMemory } );

		VkImageView newImageView = createImageView(newImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);


		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &mSingleTextureDescriptorSetLayout;

		VkDescriptorSet newDescriptorSet;

		if (vkAllocateDescriptorSets(mLogicalDevice, &allocInfo, &newDescriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = newImageView;
		imageInfo.sampler = mTextureSampler;

		
		VkWriteDescriptorSet textureWrite{};
		textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		textureWrite.dstSet = newDescriptorSet;
		textureWrite.dstBinding = 0;
		textureWrite.dstArrayElement = 0;
		textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureWrite.descriptorCount = 1;
		textureWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(mLogicalDevice, 1, &textureWrite, 0, nullptr);

		int descriptorIndex = mTextureDescriptors.size();
		mTextureDescriptors.push_back(newDescriptorSet);

		return descriptorIndex;
	}

	GraphicsSystem::GraphicsSystem()
	{
		GraphicsObjectLoader::Init(this);

	}

	GraphicsSystem::~GraphicsSystem()
	{
		vkDeviceWaitIdle(mLogicalDevice);
		cleanup();
	}

	void GraphicsSystem::cleanup()
	{
		for (VulkanVertexBuffer& allocatedBuffer : mTriangleMeshBuffers)
		{
			vkDestroyBuffer(mLogicalDevice,allocatedBuffer.mVertexBuffer,nullptr);
			vkFreeMemory(mLogicalDevice, allocatedBuffer.mVertexBufferMemory,nullptr);

			vkDestroyBuffer(mLogicalDevice, allocatedBuffer.mIndexBuffer, nullptr);
			vkFreeMemory(mLogicalDevice, allocatedBuffer.mIndexBufferMemory, nullptr);
		}

		for (ImageAllocation& imageAlloc : mImagesAllocated)
		{
			vkDestroyImage(mLogicalDevice, imageAlloc.image, nullptr);
			vkFreeMemory(mLogicalDevice, imageAlloc.imageMemory, nullptr);
		}

		vkDestroyImageView(mLogicalDevice, depthImageView, nullptr);
		vkDestroyImage(mLogicalDevice, depthImage, nullptr);
		vkFreeMemory(mLogicalDevice, depthImageMemory, nullptr);

		cleanupSwapChain();

		vkDestroySampler(mLogicalDevice, mTextureSampler, nullptr);

		vkDestroyDescriptorSetLayout(mLogicalDevice, descriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(mLogicalDevice, mObjectDescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(mLogicalDevice, mSingleTextureDescriptorSetLayout, nullptr);

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
		for (size_t i = 0; i < mSwapChainImages.size(); i++)
		{
			vkDestroyBuffer(mLogicalDevice, mObjectStorageBuffer[i], nullptr);
			vkFreeMemory(mLogicalDevice, mObjectStorageBufferMemory[i], nullptr);
		}

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
		createCommandPool();
		createDepthResources();
		createFrameBuffers();

		createTextureSampler();
		createUniformBuffers();
		createStorageBuffers();
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
		appInfo.apiVersion = VK_API_VERSION_1_1;

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
		int bIsglfwVulkanSupported = glfwVulkanSupported();
		if (VkResult windowCreate = glfwCreateWindowSurface(mInstance, glfwWindow, nullptr, &mSurface) )
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
		deviceFeatures.samplerAnisotropy = VK_TRUE;
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
			mSwapChainImageViews[i] = createImageView(mSwapChainImages[i], mSwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}

	}

	void GraphicsSystem::createRenderPass()
	{
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // (in this case, the previous subpass)
		dependency.dstSubpass = 0; // current subpass 

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;

		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachments.size();
		renderPassInfo.pAttachments = attachments.data();
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
		// Uniform Descriptor

		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(mLogicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		// Storage Descriptor

		VkDescriptorSetLayoutBinding objectLayoutBinding{};
		objectLayoutBinding.binding = 0;
		objectLayoutBinding.descriptorCount = 1;
		objectLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		objectLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		objectLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo objectLayoutInfo{};
		objectLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		objectLayoutInfo.bindingCount = 1;
		objectLayoutInfo.pBindings = &objectLayoutBinding;

		if (vkCreateDescriptorSetLayout(mLogicalDevice, &objectLayoutInfo, nullptr, &mObjectDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		// Texture Descriptor
		VkDescriptorSetLayoutBinding newSamplerLayoutBinding{};
		newSamplerLayoutBinding.binding = 0;
		newSamplerLayoutBinding.descriptorCount = 1;
		newSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		newSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		newSamplerLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo samplerLayoutInfo{};
		samplerLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		samplerLayoutInfo.bindingCount = 1;
		samplerLayoutInfo.pBindings = &newSamplerLayoutBinding;

		if (vkCreateDescriptorSetLayout(mLogicalDevice, &samplerLayoutInfo, nullptr, &mSingleTextureDescriptorSetLayout) != VK_SUCCESS) {
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

		auto bindingDescription = getBindingDescription();
		auto attributeDescriptions = getAttributeDescriptions();


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
		
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

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

		std::array<VkDescriptorSetLayout,3> setLayouts = { descriptorSetLayout, mObjectDescriptorSetLayout,mSingleTextureDescriptorSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
		pipelineLayoutInfo.pSetLayouts = setLayouts.data();

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
		pipelineInfo.pDepthStencilState = &depthStencil;
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
			std::array<VkImageView, 2> attachments = { mSwapChainImageViews[i], depthImageView };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = mRenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>( attachments.size() );
			framebufferInfo.pAttachments = attachments.data();
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
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

		if (vkCreateCommandPool(mLogicalDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void GraphicsSystem::createDepthResources()
	{
		VkFormat depthFormat = findDepthFormat();

		createImage(mSwapChainExtent.width, mSwapChainExtent.height,
			depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
		depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	}

	void GraphicsSystem::createIndexBuffers( VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory, const std::vector<uint32_t>& indices )
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
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(mLogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(mLogicalDevice, stagingBufferMemory, nullptr);
	}

	void GraphicsSystem::createVertexBuffers(VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory, const std::vector<Vertex>& vertices)
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
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer( mLogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory( mLogicalDevice, stagingBufferMemory, nullptr);
	}

	void GraphicsSystem::createStorageBuffers()
	{
		VkDeviceSize bufferSize = sizeof(GPUObjectData) * MAX_RENDER_OBJECTS;

		mObjectStorageBuffer.resize(mSwapChainImages.size());
		mObjectStorageBufferMemory.resize(mSwapChainImages.size());

		for (size_t i = 0; i < mSwapChainImages.size(); i++)
		{
			createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				mObjectStorageBuffer[i], mObjectStorageBufferMemory[i]);
		}
	}

	void GraphicsSystem::createTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_TRUE;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);

		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(mLogicalDevice, &samplerInfo, nullptr, &mTextureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}

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
		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(mSwapChainImages.size());

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = MAX_IMAGE_DESCRIPTORS;

		poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[2].descriptorCount = static_cast<uint32_t>(mSwapChainImages.size());

		int numberOfUniformDescriptorSetsCreated = mSwapChainImages.size();
		int numberOfObjectDescriptorsSetsCreated = mSwapChainImages.size();
		constexpr int numberOfSingleTextureDescriptorSetsCreated = 100;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>
			(numberOfUniformDescriptorSetsCreated + numberOfObjectDescriptorsSetsCreated + numberOfSingleTextureDescriptorSetsCreated) ;

		if (vkCreateDescriptorPool(mLogicalDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}

	}

	void GraphicsSystem::createDescriptorSets()
	{
		//init object descriptor sets
		std::vector<VkDescriptorSetLayout> objectLayouts(mSwapChainImages.size(), mObjectDescriptorSetLayout);
		VkDescriptorSetAllocateInfo objectDescriptorAllocInfo{};
		objectDescriptorAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		objectDescriptorAllocInfo.descriptorPool = mDescriptorPool;
		objectDescriptorAllocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapChainImages.size());
		objectDescriptorAllocInfo.pSetLayouts = objectLayouts.data();

		mObjectDescriptorSets.resize(mSwapChainImages.size());
		if (vkAllocateDescriptorSets(mLogicalDevice, &objectDescriptorAllocInfo, mObjectDescriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}


		//init uniform descriptor sets
		std::vector<VkDescriptorSetLayout> layouts( mSwapChainImages.size(), descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>( mSwapChainImages.size());
		allocInfo.pSetLayouts = layouts.data();

		mUniformDescriptorSets.resize( mSwapChainImages.size());
		if (vkAllocateDescriptorSets( mLogicalDevice, &allocInfo, mUniformDescriptorSets.data() ) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		


		for (size_t i = 0; i < mSwapChainImages.size(); i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = mUniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

	/*		VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = mTextureImageView;
			imageInfo.sampler = mTextureSampler;*/

			VkDescriptorBufferInfo storageBufferInfo{};
			storageBufferInfo.buffer = mObjectStorageBuffer[i];
			storageBufferInfo.offset = 0;
			storageBufferInfo.range = sizeof(GPUObjectData) * MAX_RENDER_OBJECTS;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			//Uniform Buffer
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = mUniformDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			//descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			//descriptorWrites[1].dstSet = mUniformDescriptorSets[i];
			//descriptorWrites[1].dstBinding = 1;
			//descriptorWrites[1].dstArrayElement = 0;
			//descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			//descriptorWrites[1].descriptorCount = 1;
			//descriptorWrites[1].pImageInfo = &imageInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = mObjectDescriptorSets[i];
			descriptorWrites[1].dstBinding = 0;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &storageBufferInfo;

			vkUpdateDescriptorSets(mLogicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			

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

	}

	void GraphicsSystem::recordCommandBuffers( uint32_t frameIndex )
	{
		ArchetypeQuery renderableQuery = FindArchetypesWith<Transform, Mesh>();

		vkResetCommandBuffer(commandBuffers[frameIndex], 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[frameIndex], &beginInfo) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mSwapChainFramebuffers[frameIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = mSwapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.5f, 0.75f, 1.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass( commandBuffers[frameIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

		for (Archetype& renderableArchetype : renderableQuery)
		{
			size_t currentArchetpyeCount = renderableArchetype.GetEntityCount();

			auto meshArray = renderableArchetype.GetComponentArray<Mesh>();

			for (size_t i = 0; i < renderableArchetype.GetEntityCount(); i++)
			{
				vkCmdBindPipeline(commandBuffers[frameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

				VulkanVertexBuffer& buffer = mTriangleMeshBuffers[meshArray[i].BufferIndex]; //TODO should only bind new buffers when buffers actually change

				VkBuffer vertexBuffers[] = { buffer.mVertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffers[frameIndex], 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(commandBuffers[frameIndex], buffer.mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

				vkCmdBindDescriptorSets(commandBuffers[frameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
					mPipelineLayout, 0, 1, &mUniformDescriptorSets[frameIndex], 0, nullptr);

				vkCmdBindDescriptorSets(commandBuffers[frameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
					mPipelineLayout, 1, 1, &mObjectDescriptorSets[frameIndex], 0, nullptr);

				vkCmdBindDescriptorSets(commandBuffers[frameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
					mPipelineLayout, 2, 1, 
					&mTextureDescriptors[meshArray[i].MeshMaterial.GetTexture(TextureUsageType::ALBEDO_TEXTURE)], 0, nullptr);

				vkCmdDrawIndexed(commandBuffers[frameIndex], static_cast<uint32_t>(meshArray[i].Indices.size()), 1, 0, 0, i);
				//vkCmdDraw(commandBuffers[frameIndex], static_cast<uint32_t>(vertices.size()), 1, 0, i);

			}
		}

		vkCmdEndRenderPass(commandBuffers[frameIndex]);

		if (vkEndCommandBuffer(commandBuffers[frameIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
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
		createDepthResources();
		createFrameBuffers();
		createStorageBuffers();
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

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(deviceToCheck, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
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
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}

	void GraphicsSystem::updateUniformBuffer(uint32_t currentImage)
	{
		ArchetypeQuery cameraQuery = FindArchetypesWith< Transform,Camera >();

		Transform trans;
		Camera firstCamera;

		//find first camera we can find
		for (Archetype& cameraArchetype : cameraQuery)
		{
			ComponentArrayHandle<Transform> compHandleTransform = cameraArchetype.GetComponentArray<Transform>();
			trans = compHandleTransform[0];

			ComponentArrayHandle<Camera> compHandleCamera = cameraArchetype.GetComponentArray<Camera>();
			firstCamera = compHandleCamera[0];

			break;
		}

		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		math::mat4 correctRot = math::rotate(math::mat4(1.0f), math::radians(-90.0f) , math::vec3(0.0f, 1.0f, 0.0f));

		auto camTransform = trans.GetLocalTransform();

		auto forward = trans.GetForward();
		auto up = trans.GetUp();

		math::vec3 pos = camTransform[3];

		UniformBufferObject ubo{};
		
		ubo.view = math::lookAt(pos, pos + forward, up);
		ubo.proj = math::perspective(firstCamera.Fovy , mSwapChainExtent.width / (float) mSwapChainExtent.height, firstCamera.NearPlane, firstCamera.FarPlane);
		
		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(mLogicalDevice, mUniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(mLogicalDevice, mUniformBuffersMemory[currentImage]);


		void* objectData;
		vkMapMemory(mLogicalDevice, mObjectStorageBufferMemory[currentImage], 0, sizeof(GPUObjectData) * MAX_RENDER_OBJECTS,0, &objectData);
		GPUObjectData* objectSSBO = (GPUObjectData*)objectData;

		ArchetypeQuery renderableQuery = FindArchetypesWith<Transform, Mesh>();

		size_t entityCount = 0;
		for (Archetype& renderArchetype : renderableQuery)
		{
			auto transformArray = renderArchetype.GetComponentArray<Transform>();
	

			for (size_t i = 0; i < renderArchetype.GetEntityCount(); i++)
			{
				objectSSBO[entityCount].modelMatrix = transformArray[i].GetLocalTransform();


				entityCount++;
			}
		}


		vkUnmapMemory(mLogicalDevice, mObjectStorageBufferMemory[currentImage]);
	}

	void GraphicsSystem::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(mLogicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(mLogicalDevice, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(mLogicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(mLogicalDevice, image, imageMemory, 0);

	}

	void GraphicsSystem::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		barrier.srcAccessMask = 0; // TODO
		barrier.dstAccessMask = 0; // TODO


		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer);


	}

	VkCommandBuffer GraphicsSystem::beginSingleTimeCommands()
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

		return commandBuffer;
	}

	void GraphicsSystem::endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(mGraphicsQueue);

		vkFreeCommandBuffers(mLogicalDevice, mCommandPool, 1, &commandBuffer);
	}

	void GraphicsSystem::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		endSingleTimeCommands(commandBuffer);
	}

	VkImageView GraphicsSystem::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView( mLogicalDevice, &viewInfo, nullptr, &imageView ) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	VkFormat GraphicsSystem::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates) 
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}

			throw std::runtime_error("failed to find supported format!");
		}
	}

	VkFormat GraphicsSystem::findDepthFormat()
	{
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
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
		RegisterComponent<Camera>();
		RegisterComponent<Mesh>();
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
		recordCommandBuffers(imageIndex);
		

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

