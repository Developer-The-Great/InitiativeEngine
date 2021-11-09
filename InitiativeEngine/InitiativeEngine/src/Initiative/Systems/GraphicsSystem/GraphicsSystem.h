#pragma once
#include "Initiative\ECS\ECSSystemBase.h"
#include "vulkan\vulkan.h"
#include "Initiative\Systems\GraphicsSystem\Components\Mesh.h"

namespace itv
{
	class Window;



	class GraphicsSystem : public ECSSystemBase
	{
	private:

		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete() 
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};


		
		VkInstance mInstance;
		VkDebugUtilsMessengerEXT mDebugMessenger;
		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		VkDevice mLogicalDevice = VK_NULL_HANDLE;
		VkQueue mGraphicsQueue;
		VkQueue mPresentQueue;

		VkSwapchainKHR mSwapChain;
		std::vector<VkImage> mSwapChainImages;
		VkFormat mSwapChainImageFormat;
		VkExtent2D mSwapChainExtent;

		VkDescriptorSetLayout mObjectDescriptorSetLayout;

		VkDescriptorSetLayout descriptorSetLayout;
		VkPipelineLayout mPipelineLayout;
		VkRenderPass mRenderPass;
		
		std::vector<VkImageView>   mSwapChainImageViews;
		std::vector<VkFramebuffer> mSwapChainFramebuffers;

		VkSurfaceKHR mSurface;

		VkPipeline mGraphicsPipeline;

		VkCommandPool mCommandPool;

		std::shared_ptr<Window> mWindow = nullptr;

		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;

		std::vector<VkFence> mInFlightFences;
		std::vector<VkFence> mImagesInFlight;

		std::vector < VkBuffer > mObjectStorageBuffer;
		std::vector < VkDeviceMemory > mObjectStorageBufferMemory;

		VkBuffer mVertexBuffer;
		VkDeviceMemory mVertexBufferMemory;

		VkBuffer mIndexBuffer;
		VkDeviceMemory mIndexBufferMemory;

		VkImage textureImage;
		VkDeviceMemory textureImageMemory;

		VkDescriptorPool mDescriptorPool;

		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;

		std::vector<VkDescriptorSet> mUniformDescriptorSets;
		std::vector<VkDescriptorSet> mObjectDescriptorSets;


		VkImageView mTextureImageView;
		VkSampler mTextureSampler;

		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;

		size_t currentFrame = 0;

		const std::vector<const char*> mValidationLayers = { "VK_LAYER_KHRONOS_validation" };

		const std::vector<const char*> mDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		std::vector<const char*> getRequiredExtensions();

		void initVulkan();
		void createInstance();
		void createSurface();
		void findSuitablePhysicalDevice();
		void createLogicalDevice();
		void createSwapChain();
		void createImageViews();
		void createRenderPass(); 
		void createDescriptorSetLayout();
		void createGraphicsPipeline();
		void createFrameBuffers();
		void createCommandPool();
		void createDepthResources();
		void createIndexBuffers();
		void loadModel();
		void createVertexBuffers();
		void createStorageBuffers();
		void createTextureImage();
		void createTextureImageView();
		void createTextureSampler();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();
		void createCommandBuffers();
		void recordCommandBuffers(uint32_t frameIndex);
		void createSyncObjects();

		void recreateSwapChain();

		bool isDeviceSuitable(VkPhysicalDevice deviceToCheck);

		bool isExtensionsSupportedByDevice(VkPhysicalDevice deviceToCheck);

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkShaderModule createShaderModule(const std::vector<char>& code);

		void cleanup();
		void cleanupSwapChain();

		void setupDebugMessenger();
		bool checkValidationLayerSupport();

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT
			debugMessenger, const VkAllocationCallbacks* pAllocator);

		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
			VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		void updateUniformBuffer(uint32_t currentImage);

		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);

		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
			VkFormatFeatureFlags features);

		VkFormat findDepthFormat();

		bool hasStencilComponent(VkFormat format) {
			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
		}

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}


	public:
	

		

		GraphicsSystem();
		~GraphicsSystem() override;

		void						    RegisterComponents() override;

		void							BeginRun() override;

		void						    Run() override;

	};

}

