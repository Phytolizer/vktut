#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <config.hpp>

#include <vktut/shaders/vertex.hpp>
#include <vktut/vulkan/buffer_and_memory.hpp>
#include <vktut/vulkan/image_and_memory.hpp>
#include <vktut/vulkan/instance.hpp>
#include <vktut/vulkan/swap_chain_support_details.hpp>

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
  std::vector<shaders::vertex> m_vertices;
  std::vector<std::uint32_t> m_indices;
  VkBuffer m_vertex_buffer;
  VkDeviceMemory m_vertex_buffer_memory;
  VkBuffer m_index_buffer;
  VkDeviceMemory m_index_buffer_memory;
  std::vector<VkBuffer> m_uniform_buffers;
  std::vector<VkDeviceMemory> m_uniform_buffers_memory;
  VkDescriptorPool m_descriptor_pool;
  std::vector<VkDescriptorSet> m_descriptor_sets;
  std::uint32_t m_mip_levels;
  VkImage m_texture_image;
  VkDeviceMemory m_texture_image_memory;
  VkImageView m_texture_image_view;
  VkSampler m_texture_sampler;
  VkImage m_depth_image;
  VkDeviceMemory m_depth_image_memory;
  VkImageView m_depth_image_view;
  VkImage m_color_image;
  VkDeviceMemory m_color_image_memory;
  VkImageView m_color_image_view;
  VkSampleCountFlagBits m_msaa_samples = VK_SAMPLE_COUNT_1_BIT;

  static constexpr std::uint32_t width = 800;
  static constexpr std::uint32_t height = 600;
  static constexpr std::string_view model_path =
      PROJECT_SOURCE_DIR "/Resources/Models/sculpt.obj";
  static constexpr std::array texture_paths = {
      PROJECT_SOURCE_DIR "/Resources/Textures/tex_0.jpg",
      PROJECT_SOURCE_DIR "/Resources/Textures/tex_1.jpg",
  };
  static constexpr std::array validation_layers = {
      "VK_LAYER_KHRONOS_validation",
  };
  static constexpr std::array device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };
  static constexpr int max_frames_in_flight = 2;

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
  void load_model();
  void create_descriptor_set_layout();
  void create_graphics_pipeline();
  void create_vertex_buffer();
  void create_index_buffer();
  void create_uniform_buffers();
  void create_descriptor_pool();
  void create_descriptor_sets();
  void create_texture_image();
  void create_texture_image_view();
  void create_texture_sampler();
  void create_depth_resources();
  void create_color_resources();
  VkImageView create_image_view(VkImage image,
                                VkFormat format,
                                VkImageAspectFlags aspect_flags,
                                std::uint32_t mip_levels);
  vktut::vulkan::image_and_memory create_image(
      std::uint32_t width,
      std::uint32_t height,
      std::uint32_t mip_levels,
      VkSampleCountFlagBits num_samples,
      VkFormat format,
      VkImageTiling tiling,
      VkImageUsageFlags usage,
      VkMemoryPropertyFlags properties);
  vulkan::buffer_and_memory create_buffer(VkDeviceSize size,
                                          VkBufferUsageFlags usage,
                                          VkMemoryPropertyFlags properties);
  VkShaderModule create_shader_module(const std::vector<char>& code);
  VkCommandBuffer begin_single_time_commands(VkCommandPool command_pool);
  void copy_buffer(VkBuffer src_buffer,
                   VkBuffer dst_buffer,
                   VkDeviceSize size,
                   VkCommandPool command_pool,
                   VkQueue queue);
  void end_single_time_commands(VkCommandBuffer command_buffer,
                                VkCommandPool command_pool,
                                VkQueue queue);
  void transition_image_layout(VkImage image,
                               VkFormat format,
                               VkImageLayout old_layout,
                               VkImageLayout new_layout,
                               std::uint32_t mip_levels,
                               VkCommandPool command_pool,
                               VkQueue queue);
  void generate_mipmaps(VkImage image,
                        VkFormat image_format,
                        std::int32_t tex_width,
                        std::int32_t tex_height,
                        std::uint32_t mip_levels);
  void copy_buffer_to_image(VkBuffer buffer,
                            VkImage image,
                            std::uint32_t width,
                            std::uint32_t height);
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
  VkFormat find_supported_format(const std::vector<VkFormat>& candidates,
                                 VkImageTiling tiling,
                                 VkFormatFeatureFlags features);
  VkFormat find_depth_format();
  VkSampleCountFlagBits get_max_usable_sample_count();
  static bool has_stencil_component(VkFormat format);
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
