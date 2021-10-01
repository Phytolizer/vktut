#include <fstream>
#include <ranges>
#include <sstream>
#include <stdexcept>

#include "vk_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <VkBootstrap.h>
#include <config.hpp>
#include <vk_initializers.h>
#include <vk_types.h>

#define VK_CHECK(x) \
  do { \
    VkResult err = x; \
    if (err) { \
      std::ostringstream ss; \
      ss << "Detected Vulkan error: " << err; \
      throw std::runtime_error {ss.str()}; \
    } \
  } while (0)

void vulkan::deletion_queue::push_function(std::function<void()>&& function)
{
  m_deleters.emplace_back(std::move(function));
}

void vulkan::deletion_queue::flush()
{
  for (auto it = m_deleters.rbegin(); it != m_deleters.rend(); ++it) {
    (*it)();
  }
  m_deleters.clear();
}

vulkan::engine::engine()
    : m_initialized(false)
    , m_frame_number(0)
    , m_window_extent({1700, 900})
    , m_window(nullptr)
    , m_selected_shader(0)
{
}

void vulkan::engine::init()
{
  SDL_Init(SDL_INIT_VIDEO);
  constexpr auto window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_VULKAN);
  m_window = SDL_CreateWindow("Vulkan Engine",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              static_cast<int>(m_window_extent.width),
                              static_cast<int>(m_window_extent.height),
                              window_flags);

  init_vulkan();
  init_swapchain();
  init_commands();
  init_default_renderpass();
  init_framebuffers();
  init_sync_structures();
  init_pipelines();

  m_initialized = true;
}

void vulkan::engine::init_vulkan()
{
  vkb::InstanceBuilder builder;

  auto inst_ret = builder.set_app_name("Example Vulkan Application")
                      .request_validation_layers(/*enable_validation=*/true)
                      .require_api_version(1, 1, 0)
                      .use_default_debug_messenger()
                      .build();
  const vkb::Instance vkb_inst = inst_ret.value();
  m_instance = vkb_inst.instance;
  m_debug_messenger = vkb_inst.debug_messenger;

  SDL_Vulkan_CreateSurface(m_window, m_instance, &m_surface);

  vkb::PhysicalDeviceSelector selector {vkb_inst};
  vkb::PhysicalDevice physical_device = selector.set_minimum_version(1, 1)
                                            .set_surface(m_surface)
                                            .select()
                                            .value();

  vkb::DeviceBuilder device_builder {physical_device};
  auto vkb_device = device_builder.build().value();

  m_device = vkb_device.device;
  m_chosen_gpu = physical_device.physical_device;

  m_graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
  m_graphics_queue_family =
      vkb_device.get_queue_index(vkb::QueueType::graphics).value();
}

void vulkan::engine::init_swapchain()
{
  vkb::SwapchainBuilder swapchain_builder {m_chosen_gpu, m_device, m_surface};
  vkb::Swapchain vkb_swapchain =
      swapchain_builder.use_default_format_selection()
          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .set_desired_extent(m_window_extent.width, m_window_extent.height)
          .build()
          .value();

  m_swapchain = vkb_swapchain.swapchain;
  m_swapchain_images = vkb_swapchain.get_images().value();
  m_swapchain_image_views = vkb_swapchain.get_image_views().value();
  m_swapchain_image_format = vkb_swapchain.image_format;

  m_main_deletion_queue.push_function(
      [this]() { vkDestroySwapchainKHR(m_device, m_swapchain, nullptr); });
}

void vulkan::engine::init_commands()
{
  const VkCommandPoolCreateInfo command_pool_info =
      initializers::command_pool_create_info(
          m_graphics_queue_family,
          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  VK_CHECK(vkCreateCommandPool(
      m_device, &command_pool_info, nullptr, &m_command_pool));

  const VkCommandBufferAllocateInfo cmd_alloc_info =
      initializers::command_buffer_allocate_info(m_command_pool, 1);
  VK_CHECK(vkAllocateCommandBuffers(
      m_device, &cmd_alloc_info, &m_main_command_buffer));

  m_main_deletion_queue.push_function(
      [this]() { vkDestroyCommandPool(m_device, m_command_pool, nullptr); });
}

void vulkan::engine::init_default_renderpass()
{
  VkAttachmentDescription color_attachment = {};
  color_attachment.format = m_swapchain_image_format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref = {};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  VkRenderPassCreateInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;

  VK_CHECK(
      vkCreateRenderPass(m_device, &render_pass_info, nullptr, &m_render_pass));

  m_main_deletion_queue.push_function(
      [this]() { vkDestroyRenderPass(m_device, m_render_pass, nullptr); });
}

void vulkan::engine::init_framebuffers()
{
  VkFramebufferCreateInfo fb_info = {};
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = nullptr;
  fb_info.renderPass = m_render_pass;
  fb_info.attachmentCount = 1;
  fb_info.width = m_window_extent.width;
  fb_info.height = m_window_extent.height;
  fb_info.layers = 1;

  const auto swapchain_image_count =
      static_cast<std::uint32_t>(m_swapchain_images.size());
  m_framebuffers.resize(swapchain_image_count);

  for (std::uint32_t i = 0; i < swapchain_image_count; ++i) {
    fb_info.pAttachments = &m_swapchain_image_views[i];
    VK_CHECK(
        vkCreateFramebuffer(m_device, &fb_info, nullptr, &m_framebuffers[i]));

    m_main_deletion_queue.push_function(
        [this, i]()
        {
          vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
          vkDestroyImageView(m_device, m_swapchain_image_views[i], nullptr);
        });
  }
}

void vulkan::engine::init_sync_structures()
{
  VkFenceCreateInfo fence_create_info = {};
  fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_create_info.pNext = nullptr;
  fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  VK_CHECK(
      vkCreateFence(m_device, &fence_create_info, nullptr, &m_render_fence));

  VkSemaphoreCreateInfo semaphore_create_info = {};
  semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphore_create_info.pNext = nullptr;
  semaphore_create_info.flags = 0;

  VK_CHECK(vkCreateSemaphore(
      m_device, &semaphore_create_info, nullptr, &m_present_semaphore));
  VK_CHECK(vkCreateSemaphore(
      m_device, &semaphore_create_info, nullptr, &m_render_semaphore));

  m_main_deletion_queue.push_function(
      [this]()
      {
        vkDestroySemaphore(m_device, m_present_semaphore, nullptr);
        vkDestroySemaphore(m_device, m_render_semaphore, nullptr);
      });
}

void vulkan::engine::init_pipelines()
{
  VkShaderModule triangle_frag_shader = nullptr;
  if (!load_shader_module(PROJECT_BINARY_DIR
                          "/shaders/colored_triangle.frag.spv",
                          &triangle_frag_shader))
  {
    throw std::runtime_error {"failed to load shader module!"};
  }

  VkShaderModule triangle_vert_shader = nullptr;
  if (!load_shader_module(PROJECT_BINARY_DIR
                          "/shaders/colored_triangle.vert.spv",
                          &triangle_vert_shader))
  {
    throw std::runtime_error {"failed to load shader module!"};
  }

  VkShaderModule red_triangle_frag_shader = nullptr;
  if (!load_shader_module(PROJECT_BINARY_DIR "/shaders/triangle.frag.spv",
                          &red_triangle_frag_shader))
  {
    throw std::runtime_error {"failed to load shader module!"};
  }

  VkShaderModule red_triangle_vert_shader = nullptr;
  if (!load_shader_module(PROJECT_BINARY_DIR "/shaders/triangle.vert.spv",
                          &red_triangle_vert_shader))
  {
    throw std::runtime_error {"failed to load shader module!"};
  }

  const VkPipelineLayoutCreateInfo pipeline_layout_info =
      initializers::pipeline_layout_create_info();
  VK_CHECK(vkCreatePipelineLayout(
      m_device, &pipeline_layout_info, nullptr, &m_triangle_pipeline_layout));

  pipeline_builder pipeline_builder;
  pipeline_builder.shader_stages.emplace_back(
      initializers::pipeline_shader_stage_create_info(
          VK_SHADER_STAGE_VERTEX_BIT, triangle_vert_shader));
  pipeline_builder.shader_stages.emplace_back(
      initializers::pipeline_shader_stage_create_info(
          VK_SHADER_STAGE_FRAGMENT_BIT, triangle_frag_shader));

  pipeline_builder.vertex_input_info =
      initializers::vertex_input_state_create_info();
  pipeline_builder.input_assembly = initializers::input_assembly_create_info(
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  pipeline_builder.viewport.x = 0;
  pipeline_builder.viewport.y = 0;
  pipeline_builder.viewport.width = static_cast<float>(m_window_extent.width);
  pipeline_builder.viewport.height = static_cast<float>(m_window_extent.height);
  pipeline_builder.viewport.minDepth = 0;
  pipeline_builder.viewport.maxDepth = 1;
  pipeline_builder.scissor.offset = {0, 0};
  pipeline_builder.scissor.extent = m_window_extent;
  pipeline_builder.rasterizer =
      initializers::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
  pipeline_builder.multisampling =
      initializers::multisampling_state_create_info();
  pipeline_builder.color_blend_attachment =
      initializers::color_blend_attachment_state();
  pipeline_builder.pipeline_layout = m_triangle_pipeline_layout;
  m_triangle_pipeline =
      pipeline_builder.build_pipeline(m_device, m_render_pass);

  pipeline_builder.shader_stages.clear();
  pipeline_builder.shader_stages.emplace_back(
      initializers::pipeline_shader_stage_create_info(
          VK_SHADER_STAGE_VERTEX_BIT, red_triangle_vert_shader));
  pipeline_builder.shader_stages.emplace_back(
      initializers::pipeline_shader_stage_create_info(
          VK_SHADER_STAGE_FRAGMENT_BIT, red_triangle_frag_shader));
  m_red_triangle_pipeline =
      pipeline_builder.build_pipeline(m_device, m_render_pass);

  vkDestroyShaderModule(m_device, red_triangle_vert_shader, nullptr);
  vkDestroyShaderModule(m_device, red_triangle_frag_shader, nullptr);
  vkDestroyShaderModule(m_device, triangle_vert_shader, nullptr);
  vkDestroyShaderModule(m_device, triangle_frag_shader, nullptr);

  m_main_deletion_queue.push_function(
      [this]()
      {
        vkDestroyPipeline(m_device, m_red_triangle_pipeline, nullptr);
        vkDestroyPipeline(m_device, m_triangle_pipeline, nullptr);

        vkDestroyPipelineLayout(m_device, m_triangle_pipeline_layout, nullptr);
      });
}

void vulkan::engine::cleanup()
{
  if (m_initialized) {
    vkWaitForFences(m_device, 1, &m_render_fence, VK_TRUE, 1000000000);
    m_main_deletion_queue.flush();
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    SDL_DestroyWindow(m_window);
  }
}

void vulkan::engine::draw()
{
  VK_CHECK(vkWaitForFences(m_device, 1, &m_render_fence, true, 1000000000));
  VK_CHECK(vkResetFences(m_device, 1, &m_render_fence));

  std::uint32_t swapchain_image_index = 0;
  VK_CHECK(vkAcquireNextImageKHR(m_device,
                                 m_swapchain,
                                 1000000000,
                                 m_present_semaphore,
                                 nullptr,
                                 &swapchain_image_index));

  VK_CHECK(vkResetCommandBuffer(m_main_command_buffer, 0));

  VkCommandBuffer cmd = m_main_command_buffer;
  VkCommandBufferBeginInfo cmd_begin_info = {};
  cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmd_begin_info.pNext = nullptr;
  cmd_begin_info.pInheritanceInfo = nullptr;
  cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));

  VkClearValue clear_value;
  float flash = fabs(sinf(static_cast<float>(m_frame_number) / 120.0F));
  clear_value.color = {{0.0F, 0.0F, flash, 1.0F}};

  VkRenderPassBeginInfo rp_info = {};
  rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rp_info.pNext = nullptr;
  rp_info.renderPass = m_render_pass;
  rp_info.renderArea.offset.x = 0;
  rp_info.renderArea.offset.y = 0;
  rp_info.renderArea.extent = m_window_extent;
  rp_info.framebuffer = m_framebuffers[swapchain_image_index];
  rp_info.clearValueCount = 1;
  rp_info.pClearValues = &clear_value;

  vkCmdBeginRenderPass(cmd, &rp_info, VK_SUBPASS_CONTENTS_INLINE);
  if (m_selected_shader == 0) {
    vkCmdBindPipeline(
        cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_triangle_pipeline);
  } else {
    vkCmdBindPipeline(
        cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_red_triangle_pipeline);
  }
  vkCmdDraw(cmd, 3, 1, 0, 0);
  vkCmdEndRenderPass(cmd);

  VK_CHECK(vkEndCommandBuffer(cmd));

  VkSubmitInfo submit = {};
  submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit.pNext = nullptr;
  VkPipelineStageFlags wait_stage =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  submit.pWaitDstStageMask = &wait_stage;
  submit.waitSemaphoreCount = 1;
  submit.pWaitSemaphores = &m_present_semaphore;
  submit.signalSemaphoreCount = 1;
  submit.pSignalSemaphores = &m_render_semaphore;
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &cmd;

  VK_CHECK(vkQueueSubmit(m_graphics_queue, 1, &submit, m_render_fence));

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.pNext = nullptr;
  present_info.pSwapchains = &m_swapchain;
  present_info.swapchainCount = 1;
  present_info.pWaitSemaphores = &m_render_semaphore;
  present_info.waitSemaphoreCount = 1;
  present_info.pImageIndices = &swapchain_image_index;

  VK_CHECK(vkQueuePresentKHR(m_graphics_queue, &present_info));

  m_frame_number++;
}

bool vulkan::engine::load_shader_module(std::string_view file_path,
                                        VkShaderModule* out_shader_module) const
{
  std::ifstream file {std::string {file_path},
                      std::ios::ate | std::ios::binary};
  if (!file) {
    return false;
  }

  const std::size_t file_size = file.tellg();
  std::vector<std::uint32_t> buffer(file_size / sizeof(std::uint32_t));
  file.seekg(0);
  file.read(reinterpret_cast<char*>(buffer.data()),
            static_cast<std::streamsize>(file_size));
  file.close();

  VkShaderModuleCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.codeSize = buffer.size() * sizeof(decltype(buffer)::value_type);
  create_info.pCode = buffer.data();

  VkShaderModule shader_module = nullptr;
  if (vkCreateShaderModule(m_device, &create_info, nullptr, &shader_module)
      != VK_SUCCESS)
  {
    return false;
  }
  *out_shader_module = shader_module;
  return true;
}

void vulkan::engine::run()
{
  SDL_Event e;
  bool quit = false;
  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      } else if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_SPACE) {
          m_selected_shader = (m_selected_shader + 1) % 2;
        }
      }
    }

    draw();
  }
}
