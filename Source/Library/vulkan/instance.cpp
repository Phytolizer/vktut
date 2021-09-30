#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "vktut/vulkan/instance.hpp"

vktut::vulkan::instance::~instance()
{
  vkDestroyInstance(m_handle, nullptr);
}

VkInstance vktut::vulkan::instance::get()
{
  return m_handle;
}

std::vector<const char*> vktut::vulkan::instance::get_required_extensions(
    bool enable_validation_layers)
{
  std::uint32_t glfw_extension_count = 0;
  const char** glfw_extensions =
      glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  std::vector<const char*> extensions = {
      glfw_extensions,
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      &glfw_extensions[glfw_extension_count]};

  if (enable_validation_layers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}
