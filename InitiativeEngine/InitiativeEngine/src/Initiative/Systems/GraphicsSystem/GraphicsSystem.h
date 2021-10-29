#pragma once
#include "Initiative\ECS\ECSSystemBase.h"
#include "vulkan\vulkan.h"

namespace itv
{
	class Window;

#ifdef NDEBUG
	constexpr bool enableValidationLayers = false;
#else
	constexpr bool enableValidationLayers = true;
#endif

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

		VkBuffer mVertexBuffer;
		VkDeviceMemory mVertexBufferMemory;

		VkBuffer mIndexBuffer;
		VkDeviceMemory mIndexBufferMemory;

		VkDescriptorPool mDescriptorPool;

		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;

		std::vector<VkDescriptorSet> descriptorSets;

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
		void createIndexBuffers();
		void createVertexBuffers();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();
		void createCommandBuffers();
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
	public:
	

		

		GraphicsSystem();
		~GraphicsSystem() override;

		void						    RegisterComponents() override;

		void							BeginRun() override;

		void						    Run() override;

	};

}

