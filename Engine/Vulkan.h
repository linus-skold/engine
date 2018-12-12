#pragma once
#ifdef _VULKAN_BUILD_
#include "IGraphicsAPI.h"
#include <vulkan/vulkan.h>
class Vulkan : public IGraphicsAPI
{
public:
	Vulkan() = default;
	//__________________________
	// Virtual Functions

	bool Initiate(CreateInfo create_info) override;
	bool CleanUp() override;

	void Present(uint8 anInterval, uint8 flags) override;
	void Clear() override;

	void OnAltEnter() override;

	void EnableZBuffer() override;
	void DisableZBuffer() override;

private:

	struct SwapchainCreateInfo
	{
		VkPresentModeKHR present_mode_out = VK_PRESENT_MODE_FIFO_KHR;
		VkSurfaceTransformFlagBitsKHR surface_transform_out;
		VkExtent2D swapchain_extent;
		uint32 swapchain_image_count;
		uint32 present_queue_family_index;
		uint32 graphics_queue_family_index;
	};

private:
	bool CreateVKInstance();
	bool EnumerateDevices();
	bool CreateDevice();
	bool CreateCommandPool(uint32 queue_family_index);
	bool CreateCommandBuffer();
	bool CreateSurface(uint32 queue_family_index, uint32 queue_family_count_in, SwapchainCreateInfo& swapchain_create_info);
	bool CreateSwapchain(SwapchainCreateInfo& swapchain_create_info);
	bool GetPresentMode(SwapchainCreateInfo& swapchain_create_info);


	VkDevice* m_Device;
	VkCommandBuffer* m_CommandBuffer;
	VkCommandPool* m_CommandPool;
	VkInstance* m_Instance;
	VkPhysicalDevice* m_PhysDevice;
	VkSurfaceKHR* m_Surface;
	VkSwapchainKHR* m_Swapchain;
	VkFormat m_Format;





};
#endif