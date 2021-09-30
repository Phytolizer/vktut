#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vktut::vulkan
{
struct buffer_and_memory
{
  VkBuffer buffer;
  VkDeviceMemory memory;
};
}  // namespace vktut::vulkan
