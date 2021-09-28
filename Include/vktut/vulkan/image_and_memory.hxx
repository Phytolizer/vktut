#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vktut::vulkan
{
struct image_and_memory
{
  VkImage image;
  VkDeviceMemory memory;
};
}  // namespace vktut::vulkan
