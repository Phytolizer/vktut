#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vktut::vulkan
{
struct buffer_utilities
{
  static void copy_buffer(VkBuffer src_buffer,
                          VkBuffer dst_buffer,
                          VkDeviceSize size,
                          VkQueue transfer_queue,
                          VkDevice device,
                          VkCommandPool command_pool);
};
}  // namespace vktut::vulkan
