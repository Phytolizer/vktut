#pragma once

#include <deque>
#include <functional>
#include <string_view>
#include <vector>

#include <SDL_video.h>
#include <vk_pipeline.h>
#include <vk_types.h>

namespace vulkan
{
struct deletion_queue
{
private:
  std::deque<std::function<void()>> m_deleters;

public:
  void push_function(std::function<void()>&& function);
  void flush();
};

struct engine
{
private:
  bool m_initialized;
  int m_frame_number;
  VkExtent2D m_window_extent;
  SDL_Window* m_window;

  VkInstance m_instance {};
  VkDebugUtilsMessengerEXT m_debug_messenger {};
  VkPhysicalDevice m_chosen_gpu {};
  VkDevice m_device {};
  VkSurfaceKHR m_surface {};

  VkSwapchainKHR m_swapchain {};
  VkFormat m_swapchain_image_format {};
  std::vector<VkImage> m_swapchain_images;
  std::vector<VkImageView> m_swapchain_image_views;

  VkQueue m_graphics_queue {};
  std::uint32_t m_graphics_queue_family {};

  VkCommandPool m_command_pool {};
  VkCommandBuffer m_main_command_buffer {};

  VkRenderPass m_render_pass {};
  std::vector<VkFramebuffer> m_framebuffers;

  VkSemaphore m_present_semaphore {};
  VkSemaphore m_render_semaphore {};
  VkFence m_render_fence {};

  VkPipelineLayout m_triangle_pipeline_layout {};

  VkPipeline m_triangle_pipeline{};
  VkPipeline m_red_triangle_pipeline{};

  int m_selected_shader;

  deletion_queue m_main_deletion_queue;

public:
  engine();
  void init();
  void cleanup();
  void run();

private:
  void init_vulkan();
  void init_swapchain();
  void init_commands();
  void init_default_renderpass();
  void init_framebuffers();
  void init_sync_structures();
  void init_pipelines();
  void draw();
  bool load_shader_module(std::string_view file_path,
                          VkShaderModule* out_shader_module) const;
};
}  // namespace vulkan
