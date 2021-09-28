#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "instance.hxx"

namespace vktut::vulkan
{
struct debug
{
  static VkResult create_debug_utils_messenger_ext(
      instance* instance,
      const VkDebugUtilsMessengerCreateInfoEXT* create_info,
      const VkAllocationCallbacks* allocator,
      VkDebugUtilsMessengerEXT* debug_messenger);
  static void destroy_debug_utils_messenger_ext(
      instance* instance,
      VkDebugUtilsMessengerEXT debug_messenger,
      const VkAllocationCallbacks* allocator);
};
}  // namespace vktut::vulkan
