#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vktut/shaders/vertex.hxx>
#include <vktut/vulkan/buffer_and_memory.hxx>
#include <vktut/vulkan/instance.hxx>
#include <vktut/vulkan/swap_chain_support_details.hxx>

namespace vktut::hello_triangle
{
struct application
{
private:
  GLFWwindow* m_window;
  std::unique_ptr<vktut::vulkan::instance> m_instance;
  VkDebugUtilsMessengerEXT m_debug_messenger;
  VkPhysicalDevice m_physical_device;
  VkDevice m_device;
  VkQueue m_graphics_queue;
  VkSurfaceKHR m_surface;
  VkQueue m_present_queue;
  VkQueue m_transfer_queue;
  VkSwapchainKHR m_swap_chain;
  std::vector<VkImage> m_swap_chain_images;
  VkFormat m_swap_chain_image_format;
  VkExtent2D m_swap_chain_extent;
  std::vector<VkImageView> m_swap_chain_image_views;
  VkRenderPass m_render_pass;
  VkDescriptorSetLayout m_descriptor_set_layout;
  VkPipelineLayout m_pipeline_layout;
  VkPipeline m_graphics_pipeline;
  std::vector<VkFramebuffer> m_swap_chain_framebuffers;
  VkCommandPool m_command_pool;
  VkCommandPool m_transfer_command_pool;
  std::vector<VkCommandBuffer> m_command_buffers;
  std::vector<VkCommandBuffer> m_transfer_command_buffers;
  std::vector<VkSemaphore> m_image_available_semaphores;
  std::vector<VkSemaphore> m_render_finished_semaphores;
  std::vector<VkFence> m_in_flight_fences;
  std::vector<VkFence> m_images_in_flight;
  std::size_t m_current_frame = 0;
  bool m_framebuffer_resized = false;
  VkBuffer m_vertex_buffer;
  VkDeviceMemory m_vertex_buffer_memory;
  VkBuffer m_index_buffer;
  VkDeviceMemory m_index_buffer_memory;
  std::vector<VkBuffer> m_uniform_buffers;
  std::vector<VkDeviceMemory> m_uniform_buffers_memory;
  VkDescriptorPool m_descriptor_pool;
  std::vector<VkDescriptorSet> m_descriptor_sets;

  static constexpr std::uint32_t width = 800;
  static constexpr std::uint32_t height = 600;
  static constexpr std::array validation_layers = {
      "VK_LAYER_KHRONOS_validation",
  };
  static constexpr std::array device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };
  static constexpr int max_frames_in_flight = 2;

  static constexpr std::array vertices = {
      shaders::vertex {{-0.5, -0.5}, {1, 0, 0}},
      shaders::vertex {{0.5, -0.5}, {0, 1, 0}},
      shaders::vertex {{0.5, 0.5}, {0, 0, 1}},
      shaders::vertex {{-0.5, 0.5}, {1, 1, 1}},
  };
  static constexpr std::array indices = {
      std::uint16_t {0},
      std::uint16_t {1},
      std::uint16_t {2},
      std::uint16_t {2},
      std::uint16_t {3},
      std::uint16_t {0},
  };

#ifdef NDEBUG
  static constexpr bool validation_layers_enabled = false;
#else
  static constexpr bool validation_layers_enabled = true;
#endif

public:
  application();
  ~application();
  application(const application&) = delete;
  application& operator=(const application&) = delete;
  application(application&&) = default;
  application& operator=(application&&) = default;

  void run();

private:
  void init_window();
  void init_vulkan();
  void create_swap_chain();
  void create_image_views();
  void create_render_pass();
  bool check_validation_layers_support();
  void main_loop();
  void cleanup();
  void create_descriptor_set_layout();
  void create_graphics_pipeline();
  void create_vertex_buffer();
  void create_index_buffer();
  void create_uniform_buffers();
  void create_descriptor_pool();
  void create_descriptor_sets();
  vulkan::buffer_and_memory create_buffer(VkDeviceSize size,
                                          VkBufferUsageFlags usage,
                                          VkMemoryPropertyFlags properties);
  VkShaderModule create_shader_module(const std::vector<char>& code);
  void setup_debug_messenger();
  void pick_physical_device();
  void create_logical_device();
  void create_surface();
  void create_framebuffers();
  void create_command_pools();
  void create_command_buffers();
  void create_sync_objects();
  void draw_frame();
  void recreate_swap_chain();
  void cleanup_swap_chain();
  void update_uniform_buffer(std::uint32_t current_image);
  vulkan::swap_chain_support_details query_swap_chain_support(
      VkPhysicalDevice device);
  int rate_device_suitability(VkPhysicalDevice device);
  VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
  std::uint32_t find_memory_type(std::uint32_t type_filter,
                                 VkMemoryPropertyFlags properties);
  static VkPresentModeKHR choose_swap_present_mode(
      const std::vector<VkPresentModeKHR>& available_present_modes);
  static VkSurfaceFormatKHR choose_swap_surface_format(
      const std::vector<VkSurfaceFormatKHR>& available_formats);
  static bool check_device_extensions_support(VkPhysicalDevice device);
  static bool check_validation_layer_support(
      const char* layer,
      const std::vector<VkLayerProperties>& available_layers);
  static void framebuffer_resize_callback(GLFWwindow* window,
                                          int width,
                                          int height);
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                 VkDebugUtilsMessageTypeFlagsEXT message_type,
                 const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                 void* user_data);
};
}  // namespace vktut::hello_triangle
