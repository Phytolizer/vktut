#include <algorithm>
#include <vector>

#include "vktut/vulkan/queue_family_indices.hxx"

vktut::vulkan::queue_family_indices vktut::vulkan::queue_family_indices::find(
    VkPhysicalDevice device, VkSurfaceKHR surface)
{
  vktut::vulkan::queue_family_indices indices;

  std::uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(
      device, &queue_family_count, nullptr);

  std::vector<VkQueueFamilyProperties> queue_families;
  queue_families.resize(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(
      device, &queue_family_count, queue_families.data());
  auto graphics_family =
      std::find_if(queue_families.begin(),
                   queue_families.end(),
                   [](const auto& queue_family)
                   { return queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT; });
  if (graphics_family != queue_families.end()) {
    indices.graphics_family = graphics_family - queue_families.begin();
  }

  for (std::uint32_t i = 0; i < queue_families.size(); ++i) {
    VkBool32 present_support = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
    if (present_support != VK_FALSE) {
      indices.present_family = i;
    }
  }

  return indices;
}

bool vktut::vulkan::queue_family_indices::is_complete() const
{
  return graphics_family.has_value() && present_family.has_value();
}
