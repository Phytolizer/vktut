#include "vktut/vulkan/debug.hpp"

VkResult vktut::vulkan::debug::create_debug_utils_messenger_ext(
    instance* instance,
    const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* debug_messenger)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance->get(), "vkCreateDebugUtilsMessengerEXT"));
  if (func != nullptr) {
    return func(instance->get(), create_info, allocator, debug_messenger);
  }
  return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void vktut::vulkan::debug::destroy_debug_utils_messenger_ext(
    instance* instance,
    VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* allocator)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance->get(),
                            "vkDestroyDebugUtilsMessengerEXT"));
  if (func != nullptr) {
    func(instance->get(), debug_messenger, allocator);
  }
}
