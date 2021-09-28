#pragma once

#include <cstdint>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vktut::vulkan
{
struct queue_family_indices
{
  std::optional<std::uint32_t> graphics_family;
  std::optional<std::uint32_t> present_family;
  std::optional<std::uint32_t> transfer_family;

  static vktut::vulkan::queue_family_indices find(VkPhysicalDevice device,
                                                  VkSurfaceKHR surface);

  [[nodiscard]] bool is_complete() const;

private:
  queue_family_indices() = default;
};
}  // namespace vktut::vulkan
