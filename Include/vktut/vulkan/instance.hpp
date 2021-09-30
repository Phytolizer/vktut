#pragma once

#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vktut::vulkan
{
struct instance
{
private:
  VkInstance m_handle;

public:
  template<std::size_t N>
  instance(const std::string& application_name,
           bool enable_validation_layers,
           const std::array<const char*, N>& validation_layers);
  ~instance();
  instance(const instance&) = delete;
  instance& operator=(const instance&) = delete;
  instance(instance&&) = default;
  instance& operator=(instance&&) = default;

  VkInstance get();

private:
  static std::vector<const char*> get_required_extensions(
      bool enable_validation_layers);
};

template<std::size_t N>
inline instance::instance(const std::string& application_name,
                          bool enable_validation_layers,
                          const std::array<const char*, N>& validation_layers)
    // initialized by vkCreateInstance()
    : m_handle(nullptr)
{
  VkApplicationInfo app_info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = application_name.c_str(),
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_0,
  };

  VkInstanceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app_info,
  };

  std::vector<const char*> glfw_extensions =
      get_required_extensions(enable_validation_layers);

  create_info.enabledExtensionCount = glfw_extensions.size();
  create_info.ppEnabledExtensionNames = glfw_extensions.data();

  if (enable_validation_layers) {
    create_info.enabledLayerCount =
        static_cast<std::uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();
  } else {
    create_info.enabledLayerCount = 0;
  }

  std::uint32_t extension_count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
  std::vector<VkExtensionProperties> extensions;
  extensions.resize(extension_count);
  vkEnumerateInstanceExtensionProperties(
      nullptr, &extension_count, extensions.data());

  std::cout << "[vktut::vulkan::instance::instance()] available extensions:\n";
  for (const auto& extension : extensions) {
    std::cout << "\t" << static_cast<const char*>(extension.extensionName)
              << "\n";
  }

  VkResult result = vkCreateInstance(&create_info, nullptr, &m_handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error {"failed to create instance!"};
  }
}
}  // namespace vktut::vulkan
