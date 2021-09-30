#include <algorithm>
#include <cassert>
#include <chrono>
#include <limits>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "vktut/hello_triangle/application.hpp"

#include <GLFW/glfw3.h>
#include <config.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <tiny_obj_loader.h>
#include <vktut/shaders/uniform_buffer_object.hpp>
#include <vktut/utilities/files.hpp>
#include <vktut/vulkan/debug.hpp>
#include <vktut/vulkan/queue_family_indices.hpp>

void vktut::hello_triangle::application::run()
{
  main_loop();
}

vktut::hello_triangle::application::application()
    : m_window(nullptr)
    , m_instance(nullptr)
    , m_debug_messenger(nullptr)
    , m_physical_device(nullptr)
    , m_device(nullptr)
    , m_graphics_queue(nullptr)
    , m_surface(nullptr)
    , m_present_queue(nullptr)
    , m_transfer_queue(nullptr)
    , m_swap_chain(nullptr)
    , m_swap_chain_image_format()
    , m_swap_chain_extent()
    , m_render_pass(nullptr)
    , m_descriptor_set_layout(nullptr)
    , m_pipeline_layout(nullptr)
    , m_graphics_pipeline(nullptr)
    , m_command_pool(nullptr)
    , m_transfer_command_pool(nullptr)
    , m_vertex_buffer(nullptr)
    , m_vertex_buffer_memory(nullptr)
    , m_index_buffer(nullptr)
    , m_index_buffer_memory(nullptr)
    , m_descriptor_pool(nullptr)
    , m_mip_levels(0)
    , m_texture_image(nullptr)
    , m_texture_image_memory(nullptr)
    , m_texture_image_view(nullptr)
    , m_texture_sampler(nullptr)
    , m_depth_image(nullptr)
    , m_depth_image_memory(nullptr)
    , m_depth_image_view(nullptr)
    , m_color_image(nullptr)
    , m_color_image_memory(nullptr)
    , m_color_image_view(nullptr)
{
  init_window();
  init_vulkan();
}

vktut::hello_triangle::application::~application()
{
  cleanup();
}

void vktut::hello_triangle::application::init_window()
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  m_window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
  glfwSetWindowUserPointer(m_window, this);
  glfwSetFramebufferSizeCallback(m_window, framebuffer_resize_callback);
}

void vktut::hello_triangle::application::init_vulkan()
{
  if (validation_layers_enabled && !check_validation_layers_support()) {
    throw std::runtime_error {
        "validation layers requested, but not available!"};
  }
  m_instance = std::make_unique<vktut::vulkan::instance>(
      "Hello Triangle", validation_layers_enabled, validation_layers);
  setup_debug_messenger();
  create_surface();
  pick_physical_device();
  create_logical_device();
  create_swap_chain();
  create_image_views();
  create_render_pass();
  create_descriptor_set_layout();
  create_graphics_pipeline();
  create_command_pools();
  create_color_resources();
  create_depth_resources();
  create_framebuffers();
  create_texture_image();
  create_texture_image_view();
  create_texture_sampler();
  load_model();
  create_vertex_buffer();
  create_index_buffer();
  create_uniform_buffers();
  create_descriptor_pool();
  create_descriptor_sets();
  create_command_buffers();
  create_sync_objects();
}

void vktut::hello_triangle::application::create_image_views()
{
  m_swap_chain_image_views.resize(m_swap_chain_images.size());
  for (size_t i = 0; i < m_swap_chain_images.size(); ++i) {
    const auto& swap_chain_image = m_swap_chain_images[i];
    m_swap_chain_image_views[i] = create_image_view(swap_chain_image,
                                                    m_swap_chain_image_format,
                                                    VK_IMAGE_ASPECT_COLOR_BIT,
                                                    1);
  }
}

void vktut::hello_triangle::application::create_render_pass()
{
  VkAttachmentDescription color_attachment = {
      .format = m_swap_chain_image_format,
      .samples = m_msaa_samples,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  VkAttachmentReference color_attachment_ref = {
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  VkAttachmentDescription depth_attachment = {
      .format = find_depth_format(),
      .samples = m_msaa_samples,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };

  VkAttachmentReference depth_attachment_ref = {
      .attachment = 1,
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };

  VkAttachmentDescription color_attachment_resolve = {
      .format = m_swap_chain_image_format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };

  VkAttachmentReference color_attachment_resolve_ref = {
      .attachment = 2,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  VkSubpassDescription subpass = {
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment_ref,
      .pResolveAttachments = &color_attachment_resolve_ref,
      .pDepthStencilAttachment = &depth_attachment_ref,
  };

  VkSubpassDependency dependency = {
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };

  std::array attachments = {
      color_attachment,
      depth_attachment,
      color_attachment_resolve,
  };

  VkRenderPassCreateInfo render_pass_info = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = attachments.size(),
      .pAttachments = attachments.data(),
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency,
  };

  if (vkCreateRenderPass(m_device, &render_pass_info, nullptr, &m_render_pass)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to create render pass!"};
  }
}

bool vktut::hello_triangle::application::check_validation_layers_support()
{
  std::uint32_t layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers;
  available_layers.resize(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  return std::all_of(
      validation_layers.begin(),
      validation_layers.end(),
      [&available_layers, this](const auto& layer)
      { return check_validation_layer_support(layer, available_layers); });
}

bool vktut::hello_triangle::application::check_validation_layer_support(
    const char* layer, const std::vector<VkLayerProperties>& available_layers)
{
  return std::any_of(
      available_layers.begin(),
      available_layers.end(),
      [layer](const auto& available_layer)
      {
        return std::string {layer}
        == std::string {static_cast<const char*>(available_layer.layerName)};
      });
}

void vktut::hello_triangle::application::framebuffer_resize_callback(
    GLFWwindow* window, int /*width*/, int /*height*/)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* app = reinterpret_cast<hello_triangle::application*>(
      glfwGetWindowUserPointer(window));
  app->m_framebuffer_resized = true;
}

void vktut::hello_triangle::application::main_loop()
{
  auto program_start = std::chrono::high_resolution_clock::now();
  std::size_t frame_count = 0;
  while (glfwWindowShouldClose(m_window) == 0) {
    glfwPollEvents();
    draw_frame();
    ++frame_count;

    auto current_time = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration<float, std::chrono::seconds::period>(
            current_time - program_start)
            .count()
        > 1)
    {
      std::cout << "FPS: " << frame_count << "\n";
      frame_count = 0;
      program_start = current_time;
    }
  }

  vkDeviceWaitIdle(m_device);
}

void vktut::hello_triangle::application::cleanup()
{
  cleanup_swap_chain();

  vkDestroySampler(m_device, m_texture_sampler, nullptr);
  vkDestroyImageView(m_device, m_texture_image_view, nullptr);

  vkDestroyImage(m_device, m_texture_image, nullptr);
  vkFreeMemory(m_device, m_texture_image_memory, nullptr);

  vkDestroyDescriptorSetLayout(m_device, m_descriptor_set_layout, nullptr);

  vkDestroyBuffer(m_device, m_index_buffer, nullptr);
  vkFreeMemory(m_device, m_index_buffer_memory, nullptr);
  vkDestroyBuffer(m_device, m_vertex_buffer, nullptr);
  vkFreeMemory(m_device, m_vertex_buffer_memory, nullptr);

  for (auto* fence : m_in_flight_fences) {
    vkDestroyFence(m_device, fence, nullptr);
  }
  for (auto* semaphore : m_image_available_semaphores) {
    vkDestroySemaphore(m_device, semaphore, nullptr);
  }
  for (auto* semaphore : m_render_finished_semaphores) {
    vkDestroySemaphore(m_device, semaphore, nullptr);
  }

  vkDestroyCommandPool(m_device, m_transfer_command_pool, nullptr);
  vkDestroyCommandPool(m_device, m_command_pool, nullptr);
  vkDestroyDevice(m_device, nullptr);
  vkDestroySurfaceKHR(m_instance->get(), m_surface, nullptr);
  if (validation_layers_enabled) {
    vktut::vulkan::debug::destroy_debug_utils_messenger_ext(
        m_instance.get(), m_debug_messenger, nullptr);
  }

  glfwDestroyWindow(m_window);
  glfwTerminate();
}

void vktut::hello_triangle::application::load_model()
{
  tinyobj::ObjReader reader;
  tinyobj::ObjReaderConfig config;
  config.triangulate = true;
  if (!reader.ParseFromFile(std::string {model_path}, config)) {
    throw std::runtime_error {reader.Warning() + reader.Error()};
  }
  const auto& attrib = reader.GetAttrib();
  const auto& shapes = reader.GetShapes();
  const auto& materials = reader.GetMaterials();

  std::unordered_map<shaders::vertex, std::uint32_t> unique_vertices;

  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      auto vert = shaders::vertex {};
      vert.pos = {
          attrib.vertices[3 * static_cast<std::size_t>(index.vertex_index) + 0],
          attrib.vertices[3 * static_cast<std::size_t>(index.vertex_index) + 1],
          attrib.vertices[3 * static_cast<std::size_t>(index.vertex_index) + 2],
      };
      vert.color = {1, 1, 1};
      if (index.texcoord_index != -1) {
        vert.tex_coord = {
            attrib.texcoords[2 * static_cast<std::size_t>(index.texcoord_index)
                             + 0],
            1.0F
                - attrib.texcoords
                      [2 * static_cast<std::size_t>(index.texcoord_index) + 1],
        };
      }

      if (unique_vertices.count(vert) == 0) {
        unique_vertices.emplace(vert,
                                static_cast<std::uint32_t>(m_vertices.size()));
        m_vertices.emplace_back(vert);
      }
      m_indices.push_back(unique_vertices[vert]);
    }
  }
}

void vktut::hello_triangle::application::create_descriptor_set_layout()
{
  VkDescriptorSetLayoutBinding ubo_layout_binding = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = nullptr,
  };

  VkDescriptorSetLayoutBinding sampler_layout_binding = {
      .binding = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      .pImmutableSamplers = nullptr,
  };

  std::array bindings = {
      ubo_layout_binding,
      sampler_layout_binding,
  };

  VkDescriptorSetLayoutCreateInfo layout_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = bindings.size(),
      .pBindings = bindings.data(),
  };

  if (vkCreateDescriptorSetLayout(
          m_device, &layout_info, nullptr, &m_descriptor_set_layout)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to create descriptor set layout!"};
  }
}

void vktut::hello_triangle::application::setup_debug_messenger()
{
  if (!validation_layers_enabled) {
    return;
  }

  VkDebugUtilsMessengerCreateInfoEXT create_info = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
          | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
          | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
          | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
          | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
          | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = debug_callback,
      .pUserData = nullptr,
  };

  if (vktut::vulkan::debug::create_debug_utils_messenger_ext(
          m_instance.get(), &create_info, nullptr, &m_debug_messenger)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to set up debug messenger!"};
  }
}

void vktut::hello_triangle::application::pick_physical_device()
{
  std::uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(m_instance->get(), &device_count, nullptr);

  if (device_count == 0) {
    throw std::runtime_error {"no physical devices with Vulkan support found!"};
  }

  std::vector<VkPhysicalDevice> devices;
  devices.resize(device_count);
  vkEnumeratePhysicalDevices(m_instance->get(), &device_count, devices.data());
  std::multimap<int, VkPhysicalDevice> sorted_devices;
  for (const auto& device : devices) {
    sorted_devices.emplace(rate_device_suitability(device), device);
  }
  auto best_device = sorted_devices.rbegin();
  if (best_device->first == 0) {
    throw std::runtime_error {"no suitable devices found!"};
  }
  m_physical_device = best_device->second;
  m_msaa_samples = get_max_usable_sample_count();
}

void vktut::hello_triangle::application::create_logical_device()
{
  auto indices =
      vulkan::queue_family_indices::find(m_physical_device, m_surface);

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::unordered_set<std::uint32_t> unique_queue_families = {
      *indices.graphics_family,
      *indices.present_family,
      *indices.transfer_family,
  };
  queue_create_infos.reserve(unique_queue_families.size());
  float queue_priority = 1;
  std::transform(unique_queue_families.begin(),
                 unique_queue_families.end(),
                 std::back_inserter(queue_create_infos),
                 [&queue_priority](std::uint32_t queue_family)
                 {
                   return VkDeviceQueueCreateInfo {
                       .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                       .queueFamilyIndex = queue_family,
                       .queueCount = 1,
                       .pQueuePriorities = &queue_priority,
                   };
                 });

  VkPhysicalDeviceFeatures device_features = {
      .sampleRateShading = VK_TRUE,
      .samplerAnisotropy = VK_TRUE,
  };

  VkDeviceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount =
          static_cast<std::uint32_t>(queue_create_infos.size()),
      .pQueueCreateInfos = queue_create_infos.data(),
      .enabledExtensionCount = device_extensions.size(),
      .ppEnabledExtensionNames = device_extensions.data(),
      .pEnabledFeatures = &device_features,
  };

  if (vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to create logical device!"};
  }
  vkGetDeviceQueue(m_device, *indices.graphics_family, 0, &m_graphics_queue);
  vkGetDeviceQueue(m_device, *indices.present_family, 0, &m_present_queue);
  vkGetDeviceQueue(m_device, *indices.transfer_family, 0, &m_transfer_queue);
}

void vktut::hello_triangle::application::create_surface()
{
  if (glfwCreateWindowSurface(m_instance->get(), m_window, nullptr, &m_surface)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to create window surface!"};
  }
}

void vktut::hello_triangle::application::create_framebuffers()
{
  m_swap_chain_framebuffers.resize(m_swap_chain_image_views.size());

  for (size_t i = 0; i < m_swap_chain_image_views.size(); ++i) {
    std::array attachments = {
        m_color_image_view,
        m_depth_image_view,
        m_swap_chain_image_views[i],
    };

    VkFramebufferCreateInfo framebuffer_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = m_render_pass,
        .attachmentCount = attachments.size(),
        .pAttachments = attachments.data(),
        .width = m_swap_chain_extent.width,
        .height = m_swap_chain_extent.height,
        .layers = 1,
    };

    if (vkCreateFramebuffer(
            m_device, &framebuffer_info, nullptr, &m_swap_chain_framebuffers[i])
        != VK_SUCCESS)
    {
      throw std::runtime_error {"failed to create framebuffer!"};
    }
  }
}

void vktut::hello_triangle::application::create_command_pools()
{
  auto queue_family_indices =
      vulkan::queue_family_indices::find(m_physical_device, m_surface);

  VkCommandPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = 0,
      .queueFamilyIndex = *queue_family_indices.graphics_family,
  };

  if (vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to create command pool!"};
  }

  pool_info.queueFamilyIndex = *queue_family_indices.transfer_family;
  if (vkCreateCommandPool(
          m_device, &pool_info, nullptr, &m_transfer_command_pool)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to create transfer command pool!"};
  }
}

void vktut::hello_triangle::application::create_command_buffers()
{
  m_command_buffers.resize(m_swap_chain_framebuffers.size());
  m_transfer_command_buffers.resize(m_swap_chain_framebuffers.size());

  VkCommandBufferAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = m_command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount =
          static_cast<std::uint32_t>(m_command_buffers.size()),
  };

  if (vkAllocateCommandBuffers(m_device, &alloc_info, m_command_buffers.data())
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to allocate command buffers!"};
  }

  alloc_info.commandPool = m_transfer_command_pool;
  alloc_info.commandBufferCount =
      static_cast<std::uint32_t>(m_transfer_command_buffers.size());
  if (vkAllocateCommandBuffers(
          m_device, &alloc_info, m_transfer_command_buffers.data())
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to allocate transfer command buffers!"};
  }

  for (size_t i = 0; i < m_command_buffers.size(); ++i) {
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        .pInheritanceInfo = nullptr,
    };

    if (vkBeginCommandBuffer(m_command_buffers[i], &begin_info) != VK_SUCCESS) {
      throw std::runtime_error {"failed to begin recording command buffer!"};
    }

    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_render_pass,
        .framebuffer = m_swap_chain_framebuffers[i],
        .renderArea =
            {
                .offset = {0, 0},
                .extent = m_swap_chain_extent,
            },
    };

    // should match attachment order in create_render_pass()
    std::array clear_values = {
        VkClearValue {
            .color = {0, 0, 0, 1},
        },
        VkClearValue {
            .depthStencil = {1, 0},
        },
    };
    render_pass_info.clearValueCount = clear_values.size();
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(
        m_command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(m_command_buffers[i],
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_graphics_pipeline);
    std::array vertex_buffers = {
        m_vertex_buffer,
    };
    std::array offsets = {
        VkDeviceSize {0},
    };
    vkCmdBindVertexBuffers(
        m_command_buffers[i], 0, 1, vertex_buffers.data(), offsets.data());
    vkCmdBindIndexBuffer(
        m_command_buffers[i], m_index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(m_command_buffers[i],
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline_layout,
                            0,
                            1,
                            &m_descriptor_sets[i],
                            0,
                            nullptr);
    vkCmdDrawIndexed(m_command_buffers[i], m_indices.size(), 1, 0, 0, 0);
    vkCmdEndRenderPass(m_command_buffers[i]);
    if (vkEndCommandBuffer(m_command_buffers[i]) != VK_SUCCESS) {
      throw std::runtime_error {"failed to record command buffer!"};
    }
  }
}

void vktut::hello_triangle::application::create_sync_objects()
{
  m_image_available_semaphores.resize(max_frames_in_flight);
  m_render_finished_semaphores.resize(max_frames_in_flight);
  m_in_flight_fences.resize(max_frames_in_flight);
  m_images_in_flight.resize(m_swap_chain_images.size(), VK_NULL_HANDLE);
  VkSemaphoreCreateInfo semaphore_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VkFenceCreateInfo fence_info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  for (size_t i = 0; i < max_frames_in_flight; ++i) {
    if (vkCreateSemaphore(m_device,
                          &semaphore_info,
                          nullptr,
                          &m_image_available_semaphores[i])
            != VK_SUCCESS
        || vkCreateSemaphore(m_device,
                             &semaphore_info,
                             nullptr,
                             &m_render_finished_semaphores[i])
            != VK_SUCCESS
        || vkCreateFence(m_device, &fence_info, nullptr, &m_in_flight_fences[i])
            != VK_SUCCESS)
    {
      throw std::runtime_error {
          "failed to create synchronization objects for a frame!"};
    }
  }
}

void vktut::hello_triangle::application::draw_frame()
{
  vkWaitForFences(m_device,
                  1,
                  &m_in_flight_fences[m_current_frame],
                  VK_TRUE,
                  std::numeric_limits<std::uint64_t>::max());
  // 1. acquire an image from the swap chain
  std::uint32_t image_index = 0;
  VkResult result =
      vkAcquireNextImageKHR(m_device,
                            m_swap_chain,
                            std::numeric_limits<std::uint64_t>::max(),
                            m_image_available_semaphores[m_current_frame],
                            VK_NULL_HANDLE,
                            &image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreate_swap_chain();
    return;
  }
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error {"failed to acquire swap chain image!"};
  }
  // 1b. if a previous frame is using this image, wait
  if (m_images_in_flight[image_index] != VK_NULL_HANDLE) {
    vkWaitForFences(m_device,
                    1,
                    &m_images_in_flight[image_index],
                    VK_TRUE,
                    std::numeric_limits<std::uint64_t>::max());
  }
  // 2. execute the command buffer with that image
  update_uniform_buffer(image_index);

  VkSubmitInfo submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
  };

  std::array wait_semaphores = {
      m_image_available_semaphores[m_current_frame],
  };
  std::array<VkPipelineStageFlags, 1> wait_stages = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
  };
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores.data();
  submit_info.pWaitDstStageMask = wait_stages.data();
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &m_command_buffers[image_index];

  std::array signal_semaphores = {
      m_render_finished_semaphores[m_current_frame],
  };
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores.data();

  vkResetFences(m_device, 1, &m_in_flight_fences[m_current_frame]);
  if (vkQueueSubmit(m_graphics_queue,
                    1,
                    &submit_info,
                    m_in_flight_fences[m_current_frame])
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to submit draw command buffer!"};
  }
  // 3. return the image to the swap chain for presentation
  VkPresentInfoKHR present_info = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = signal_semaphores.data(),
  };

  std::array swap_chains = {
      m_swap_chain,
  };
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swap_chains.data();
  present_info.pImageIndices = &image_index;
  present_info.pResults = nullptr;
  result = vkQueuePresentKHR(m_present_queue, &present_info);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR
      || m_framebuffer_resized)
  {
    m_framebuffer_resized = false;
    recreate_swap_chain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error {"failed to present swap chain image!"};
  }
  m_current_frame = (m_current_frame + 1) % max_frames_in_flight;
}

void vktut::hello_triangle::application::recreate_swap_chain()
{
  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(m_window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(m_window, &width, &height);
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(m_device);

  cleanup_swap_chain();

  create_swap_chain();
  create_image_views();
  create_render_pass();
  create_graphics_pipeline();
  create_color_resources();
  create_depth_resources();
  create_framebuffers();
  create_uniform_buffers();
  create_descriptor_pool();
  create_descriptor_sets();
  create_command_buffers();
}

void vktut::hello_triangle::application::cleanup_swap_chain()
{
  vkDestroyImageView(m_device, m_color_image_view, nullptr);
  vkDestroyImage(m_device, m_color_image, nullptr);
  vkFreeMemory(m_device, m_color_image_memory, nullptr);
  vkDestroyImageView(m_device, m_depth_image_view, nullptr);
  vkDestroyImage(m_device, m_depth_image, nullptr);
  vkFreeMemory(m_device, m_depth_image_memory, nullptr);

  for (auto* framebuffer : m_swap_chain_framebuffers) {
    vkDestroyFramebuffer(m_device, framebuffer, nullptr);
  }
  vkFreeCommandBuffers(m_device,
                       m_command_pool,
                       m_command_buffers.size(),
                       m_command_buffers.data());
  vkDestroyPipeline(m_device, m_graphics_pipeline, nullptr);
  vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
  vkDestroyRenderPass(m_device, m_render_pass, nullptr);
  for (auto* image_view : m_swap_chain_image_views) {
    vkDestroyImageView(m_device, image_view, nullptr);
  }
  vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);

  for (size_t i = 0; i < m_swap_chain_images.size(); ++i) {
    vkDestroyBuffer(m_device, m_uniform_buffers[i], nullptr);
    vkFreeMemory(m_device, m_uniform_buffers_memory[i], nullptr);
  }

  vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
}

void vktut::hello_triangle::application::update_uniform_buffer(
    std::uint32_t current_image)
{
  static auto start_time = std::chrono::high_resolution_clock::now();

  auto current_time = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(
                   current_time - start_time)
                   .count();

  shaders::uniform_buffer_object ubo = {
      .model = glm::rotate(glm::mat4 {1.0F},
                           time * glm::radians(90.0F),
                           glm::vec3 {0.0F, 0.0F, 1.0F}),
      .view = glm::lookAt(glm::vec3 {30.0F, 30.0F, 30.0F},
                          glm::vec3 {0.0F, 0.0F, 0.0F},
                          glm::vec3 {0.0F, 0.0F, 1.0F}),
      .proj =
          glm::perspective(glm::radians(45.0F),
                           static_cast<float>(m_swap_chain_extent.width)
                               / static_cast<float>(m_swap_chain_extent.height),
                           0.1F,
                           1000.0F),
  };

  // flip y axis, vulkan has a sensible y axis unlike ogl
  ubo.proj[1][1] *= -1;

  void* data = nullptr;
  vkMapMemory(m_device,
              m_uniform_buffers_memory[current_image],
              0,
              sizeof(shaders::uniform_buffer_object),
              0,
              &data);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  std::copy(&ubo, &ubo + 1, static_cast<shaders::uniform_buffer_object*>(data));
  vkUnmapMemory(m_device, m_uniform_buffers_memory[current_image]);
}

vktut::vulkan::swap_chain_support_details
vktut::hello_triangle::application::query_swap_chain_support(
    VkPhysicalDevice device)
{
  vulkan::swap_chain_support_details details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, m_surface, &details.capabilities);

  std::uint32_t format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(
      device, m_surface, &format_count, nullptr);
  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, m_surface, &format_count, details.formats.data());
  }

  std::uint32_t present_mode_count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, m_surface, &present_mode_count, nullptr);
  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, m_surface, &present_mode_count, details.present_modes.data());
  }

  return details;
}

void vktut::hello_triangle::application::create_swap_chain()
{
  auto swap_chain_support = query_swap_chain_support(m_physical_device);

  auto surface_format = choose_swap_surface_format(swap_chain_support.formats);
  auto present_mode =
      choose_swap_present_mode(swap_chain_support.present_modes);
  auto extent = choose_swap_extent(swap_chain_support.capabilities);

  std::uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
  if (swap_chain_support.capabilities.maxImageCount > 0
      && image_count > swap_chain_support.capabilities.maxImageCount)
  {
    image_count = swap_chain_support.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR create_info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = m_surface,
      .minImageCount = image_count,
      .imageFormat = surface_format.format,
      .imageColorSpace = surface_format.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
  };

  auto indices =
      vulkan::queue_family_indices::find(m_physical_device, m_surface);
  std::array queue_family_indices = {
      *indices.graphics_family,
      *indices.present_family,
      *indices.transfer_family,
  };

  std::array dedup_queue_family_indices = {
      *indices.graphics_family,
      *indices.transfer_family,
  };

  create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
  if (indices.graphics_family != indices.present_family) {
    create_info.queueFamilyIndexCount = 3;
    create_info.pQueueFamilyIndices = queue_family_indices.data();
  } else {
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = dedup_queue_family_indices.data();
  }

  create_info.preTransform = swap_chain_support.capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;
  create_info.oldSwapchain = VK_NULL_HANDLE;
  if (vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swap_chain)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to create swap chain!"};
  }

  vkGetSwapchainImagesKHR(m_device, m_swap_chain, &image_count, nullptr);
  m_swap_chain_images.resize(image_count);
  vkGetSwapchainImagesKHR(
      m_device, m_swap_chain, &image_count, m_swap_chain_images.data());
  m_swap_chain_image_format = surface_format.format;
  m_swap_chain_extent = extent;
}

void vktut::hello_triangle::application::create_graphics_pipeline()
{
  auto vert_shader_code =
      utilities::files::read_file("Resources/Shaders/basic.vert.spv");
  auto frag_shader_code =
      utilities::files::read_file("Resources/Shaders/basic.frag.spv");

  VkShaderModule vert_shader_module = create_shader_module(vert_shader_code);
  VkShaderModule frag_shader_module = create_shader_module(frag_shader_code);

  VkPipelineShaderStageCreateInfo vert_shader_stage_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vert_shader_module,
      .pName = "main",
  };
  VkPipelineShaderStageCreateInfo frag_shader_stage_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = frag_shader_module,
      .pName = "main",
  };

  std::array shader_stages = {
      vert_shader_stage_info,
      frag_shader_stage_info,
  };

  auto binding_description = shaders::vertex::binding_description();
  auto attribute_descriptions = shaders::vertex::attribute_descriptions();

  VkPipelineVertexInputStateCreateInfo vertex_input_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &binding_description,
      .vertexAttributeDescriptionCount = attribute_descriptions.size(),
      .pVertexAttributeDescriptions = attribute_descriptions.data(),
  };

  VkPipelineInputAssemblyStateCreateInfo input_assembly = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };

  VkViewport viewport = {
      .x = 0,
      .y = 0,
      .width = static_cast<float>(m_swap_chain_extent.width),
      .height = static_cast<float>(m_swap_chain_extent.height),
      .minDepth = 0,
      .maxDepth = 1,
  };

  VkRect2D scissor = {
      .offset = {0, 0},
      .extent = m_swap_chain_extent,
  };

  VkPipelineViewportStateCreateInfo viewport_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = &viewport,
      .scissorCount = 1,
      .pScissors = &scissor,
  };

  VkPipelineRasterizationStateCreateInfo rasterizer = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .depthBiasConstantFactor = 0,
      .depthBiasClamp = 0,
      .depthBiasSlopeFactor = 0,
      .lineWidth = 1,
  };

  VkPipelineMultisampleStateCreateInfo multisampling = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = m_msaa_samples,
      .sampleShadingEnable = VK_TRUE,
      .minSampleShading = 0.2F,
      .pSampleMask = nullptr,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable = VK_FALSE,
  };

  VkPipelineColorBlendAttachmentState color_blend_attachment = {
      .blendEnable = VK_FALSE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
          | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo color_blending = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &color_blend_attachment,
      .blendConstants = {0, 0, 0, 0},
  };

  std::array dynamic_states = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_LINE_WIDTH,
  };

  // not used, but could be to avoid recreating the entire pipeline when these
  // params are changed
  VkPipelineDynamicStateCreateInfo dynamic_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = 2,
      .pDynamicStates = dynamic_states.data(),
  };
  // explicitly unused to suppress warning, read comment above
  (void)dynamic_state;

  VkPipelineLayoutCreateInfo pipeline_layout_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &m_descriptor_set_layout,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr,
  };

  if (vkCreatePipelineLayout(
          m_device, &pipeline_layout_info, nullptr, &m_pipeline_layout)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to create pipeline layout!"};
  }

  VkPipelineDepthStencilStateCreateInfo depth_stencil = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp = VK_COMPARE_OP_LESS,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE,
      .front = {},
      .back = {},
      .minDepthBounds = 0,
      .maxDepthBounds = 1,
  };

  VkGraphicsPipelineCreateInfo pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = 2,
      .pStages = shader_stages.data(),
      .pVertexInputState = &vertex_input_info,
      .pInputAssemblyState = &input_assembly,
      .pViewportState = &viewport_state,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pDepthStencilState = &depth_stencil,
      .pColorBlendState = &color_blending,
      .pDynamicState = nullptr,
      .layout = m_pipeline_layout,
      .renderPass = m_render_pass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
  };

  if (vkCreateGraphicsPipelines(m_device,
                                VK_NULL_HANDLE,
                                1,
                                &pipeline_info,
                                nullptr,
                                &m_graphics_pipeline)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to create graphics pipeline!"};
  }

  vkDestroyShaderModule(m_device, vert_shader_module, nullptr);
  vkDestroyShaderModule(m_device, frag_shader_module, nullptr);
}

void vktut::hello_triangle::application::create_vertex_buffer()
{
  auto buffer_size =
      sizeof(decltype(m_vertices)::value_type) * m_vertices.size();
  auto staging_buffer_and_memory =
      create_buffer(buffer_size,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  VkBuffer staging_buffer = staging_buffer_and_memory.buffer;
  VkDeviceMemory staging_memory = staging_buffer_and_memory.memory;
  void* data = nullptr;
  vkMapMemory(m_device, staging_memory, 0, buffer_size, 0, &data);
  std::copy(m_vertices.begin(),
            m_vertices.end(),
            static_cast<shaders::vertex*>(data));
  vkUnmapMemory(m_device, staging_memory);

  auto vertex_buffer_and_memory = create_buffer(
      buffer_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  m_vertex_buffer = vertex_buffer_and_memory.buffer;
  m_vertex_buffer_memory = vertex_buffer_and_memory.memory;

  copy_buffer(staging_buffer,
              m_vertex_buffer,
              buffer_size,
              m_transfer_command_pool,
              m_transfer_queue);

  vkDestroyBuffer(m_device, staging_buffer, nullptr);
  vkFreeMemory(m_device, staging_memory, nullptr);
}

void vktut::hello_triangle::application::create_index_buffer()
{
  VkDeviceSize buffer_size =
      sizeof(decltype(m_indices)::value_type) * m_indices.size();

  auto staging = create_buffer(buffer_size,
                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  void* data = nullptr;
  vkMapMemory(m_device, staging.memory, 0, buffer_size, 0, &data);
  std::copy(m_indices.begin(),
            m_indices.end(),
            static_cast<decltype(m_indices)::value_type*>(data));
  vkUnmapMemory(m_device, staging.memory);

  auto index = create_buffer(
      buffer_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  m_index_buffer = index.buffer;
  m_index_buffer_memory = index.memory;
  copy_buffer(staging.buffer,
              index.buffer,
              buffer_size,
              m_transfer_command_pool,
              m_transfer_queue);

  vkDestroyBuffer(m_device, staging.buffer, nullptr);
  vkFreeMemory(m_device, staging.memory, nullptr);
}

void vktut::hello_triangle::application::create_uniform_buffers()
{
  VkDeviceSize buffer_size = sizeof(shaders::uniform_buffer_object);

  m_uniform_buffers.resize(m_swap_chain_images.size());
  m_uniform_buffers_memory.resize(m_swap_chain_images.size());

  for (size_t i = 0; i < m_swap_chain_images.size(); ++i) {
    auto uniform = create_buffer(buffer_size,
                                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                     | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_uniform_buffers[i] = uniform.buffer;
    m_uniform_buffers_memory[i] = uniform.memory;
  }
}

void vktut::hello_triangle::application::create_descriptor_pool()
{
  std::array pool_sizes = {
      VkDescriptorPoolSize {
          .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount =
              static_cast<std::uint32_t>(m_swap_chain_images.size()),
      },
      VkDescriptorPoolSize {
          .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount =
              static_cast<std::uint32_t>(m_swap_chain_images.size()),
      },
  };

  VkDescriptorPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = 0,
      .maxSets = static_cast<std::uint32_t>(m_swap_chain_images.size()),
      .poolSizeCount = pool_sizes.size(),
      .pPoolSizes = pool_sizes.data(),
  };

  if (vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_descriptor_pool)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to create descriptor pool!"};
  }
}

void vktut::hello_triangle::application::create_descriptor_sets()
{
  std::vector<VkDescriptorSetLayout> layouts;
  layouts.resize(m_swap_chain_images.size(), m_descriptor_set_layout);

  VkDescriptorSetAllocateInfo allocate_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = m_descriptor_pool,
      .descriptorSetCount =
          static_cast<std::uint32_t>(m_swap_chain_images.size()),
      .pSetLayouts = layouts.data(),
  };

  m_descriptor_sets.resize(m_swap_chain_images.size());
  if (vkAllocateDescriptorSets(
          m_device, &allocate_info, m_descriptor_sets.data())
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to allocate descriptor sets!"};
  }

  for (std::size_t i = 0; i < m_swap_chain_images.size(); ++i) {
    VkDescriptorBufferInfo buffer_info = {
        .buffer = m_uniform_buffers[i],
        .offset = 0,
        .range = sizeof(shaders::uniform_buffer_object),
    };

    VkDescriptorImageInfo image_info = {
        .sampler = m_texture_sampler,
        .imageView = m_texture_image_view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    std::array descriptor_writes = {
        VkWriteDescriptorSet {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_descriptor_sets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &buffer_info,
            .pTexelBufferView = nullptr,
        },
        VkWriteDescriptorSet {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_descriptor_sets[i],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &image_info,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr,
        },
    };

    vkUpdateDescriptorSets(m_device,
                           descriptor_writes.size(),
                           descriptor_writes.data(),
                           0,
                           nullptr);
  }
}

void vktut::hello_triangle::application::create_texture_image()
{
  int tex_width = 0;
  int tex_height = 0;
  int tex_channels = 0;
  stbi_uc* pixels = stbi_load(
      texture_paths[0], &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
  if (pixels == nullptr) {
    throw std::runtime_error {"failed to load texture image!"};
  }
  auto image_size = static_cast<VkDeviceSize>(tex_width)
      * static_cast<VkDeviceSize>(tex_height) * 4;
  m_mip_levels = static_cast<std::uint32_t>(
                     std::floor(std::log2(std::max(tex_width, tex_height))))
      + 1;

  auto staging = create_buffer(image_size,
                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  void* data = nullptr;
  vkMapMemory(m_device, staging.memory, 0, image_size, 0, &data);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  std::copy(pixels, &pixels[image_size], static_cast<stbi_uc*>(data));
  vkUnmapMemory(m_device, staging.memory);

  stbi_image_free(pixels);

  auto image = create_image(tex_width,
                            tex_height,
                            m_mip_levels,
                            VK_SAMPLE_COUNT_1_BIT,
                            VK_FORMAT_R8G8B8A8_SRGB,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                                | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                | VK_IMAGE_USAGE_SAMPLED_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  m_texture_image = image.image;
  m_texture_image_memory = image.memory;

  transition_image_layout(m_texture_image,
                          VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          m_mip_levels,
                          m_transfer_command_pool,
                          m_transfer_queue);
  copy_buffer_to_image(staging.buffer,
                       m_texture_image,
                       static_cast<std::uint32_t>(tex_width),
                       static_cast<std::uint32_t>(tex_height));
  generate_mipmaps(m_texture_image,
                   VK_FORMAT_R8G8B8A8_SRGB,
                   tex_width,
                   tex_height,
                   m_mip_levels);

  vkDestroyBuffer(m_device, staging.buffer, nullptr);
  vkFreeMemory(m_device, staging.memory, nullptr);
}

void vktut::hello_triangle::application::create_texture_image_view()
{
  m_texture_image_view = create_image_view(m_texture_image,
                                           VK_FORMAT_R8G8B8A8_SRGB,
                                           VK_IMAGE_ASPECT_COLOR_BIT,
                                           m_mip_levels);
}

void vktut::hello_triangle::application::create_texture_sampler()
{
  VkSamplerCreateInfo sampler_info = {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      // use nearest for crisp retro textures
      .magFilter = VK_FILTER_LINEAR,
      // don't use nearest unless you like ugly walls
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipLodBias = 0.0F,
      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy = 16,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .minLod = 0.0F,
      .maxLod = static_cast<float>(m_mip_levels),
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE,
  };

  if (vkCreateSampler(m_device, &sampler_info, nullptr, &m_texture_sampler)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to create texture sampler!"};
  }
}

void vktut::hello_triangle::application::create_depth_resources()
{
  VkFormat depth_format = find_depth_format();
  auto depth = create_image(m_swap_chain_extent.width,
                            m_swap_chain_extent.height,
                            1,
                            m_msaa_samples,
                            depth_format,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  m_depth_image = depth.image;
  m_depth_image_memory = depth.memory;
  m_depth_image_view = create_image_view(
      m_depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

  transition_image_layout(m_depth_image,
                          depth_format,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                          1,
                          m_command_pool,
                          m_graphics_queue);
}

void vktut::hello_triangle::application::create_color_resources()
{
  VkFormat color_format = m_swap_chain_image_format;

  auto color = create_image(m_swap_chain_extent.width,
                            m_swap_chain_extent.height,
                            1,
                            m_msaa_samples,
                            color_format,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
                                | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  m_color_image = color.image;
  m_color_image_memory = color.memory;
  m_color_image_view = create_image_view(
      m_color_image, color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

VkImageView vktut::hello_triangle::application::create_image_view(
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspect_flags,
    std::uint32_t mip_levels)
{
  VkImageViewCreateInfo view_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = format,
      .components =
          {
              .r = VK_COMPONENT_SWIZZLE_IDENTITY,
              .g = VK_COMPONENT_SWIZZLE_IDENTITY,
              .b = VK_COMPONENT_SWIZZLE_IDENTITY,
              .a = VK_COMPONENT_SWIZZLE_IDENTITY,
          },
      .subresourceRange =
          {
              .aspectMask = aspect_flags,
              .baseMipLevel = 0,
              .levelCount = mip_levels,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  VkImageView image_view = nullptr;
  if (vkCreateImageView(m_device, &view_info, nullptr, &image_view)
      != VK_SUCCESS) {
    throw std::runtime_error {"failed to create texture image view!"};
  }

  return image_view;
}

vktut::vulkan::image_and_memory
vktut::hello_triangle::application::create_image(
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t mip_levels,
    VkSampleCountFlagBits num_samples,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties)
{
  VkImageCreateInfo image_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .flags = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent =
          {
              .width = static_cast<std::uint32_t>(width),
              .height = static_cast<std::uint32_t>(height),
              .depth = 1,
          },
      .mipLevels = mip_levels,
      .arrayLayers = 1,
      .samples = num_samples,
      .tiling = tiling,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VkImage image = nullptr;
  if (vkCreateImage(m_device, &image_info, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error {"failed to create image!"};
  }

  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(m_device, image, &memory_requirements);

  VkMemoryAllocateInfo allocate_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex =
          find_memory_type(memory_requirements.memoryTypeBits, properties),
  };

  VkDeviceMemory memory = nullptr;
  if (vkAllocateMemory(m_device, &allocate_info, nullptr, &memory)
      != VK_SUCCESS) {
    throw std::runtime_error {"failed to allocate image memory!"};
  }

  vkBindImageMemory(m_device, image, memory, 0);
  return vulkan::image_and_memory {image, memory};
}

vktut::vulkan::buffer_and_memory
vktut::hello_triangle::application::create_buffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties)
{
  auto indices =
      vulkan::queue_family_indices::find(m_physical_device, m_surface);
  std::array queue_family_indices = {
      *indices.graphics_family,
      *indices.transfer_family,
  };
  VkBufferCreateInfo buffer_info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_CONCURRENT,
      .queueFamilyIndexCount = 2,
      .pQueueFamilyIndices = queue_family_indices.data(),
  };

  VkBuffer buffer = nullptr;
  if (vkCreateBuffer(m_device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error {"failed to create vertex buffer!"};
  }

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(m_device, buffer, &memory_requirements);

  VkMemoryAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex =
          find_memory_type(memory_requirements.memoryTypeBits, properties),
  };

  VkDeviceMemory buffer_memory = nullptr;
  if (vkAllocateMemory(m_device, &alloc_info, nullptr, &buffer_memory)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to allocate vertex buffer memory!"};
  }

  vkBindBufferMemory(m_device, buffer, buffer_memory, 0);
  return vulkan::buffer_and_memory {
      .buffer = buffer,
      .memory = buffer_memory,
  };
}

VkShaderModule vktut::hello_triangle::application::create_shader_module(
    const std::vector<char>& code)
{
  VkShaderModuleCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = code.size(),
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      .pCode = reinterpret_cast<const std::uint32_t*>(code.data()),
  };

  VkShaderModule shader_module = {};
  if (vkCreateShaderModule(m_device, &create_info, nullptr, &shader_module)
      != VK_SUCCESS)
  {
    throw std::runtime_error {"failed to create shader module!"};
  }
  return shader_module;
}

VkCommandBuffer vktut::hello_triangle::application::begin_single_time_commands(
    VkCommandPool command_pool)
{
  VkCommandBufferAllocateInfo allocate_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };

  VkCommandBuffer command_buffer = nullptr;
  vkAllocateCommandBuffers(m_device, &allocate_info, &command_buffer);

  VkCommandBufferBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  vkBeginCommandBuffer(command_buffer, &begin_info);

  return command_buffer;
}

void vktut::hello_triangle::application::copy_buffer(VkBuffer src_buffer,
                                                     VkBuffer dst_buffer,
                                                     VkDeviceSize size,
                                                     VkCommandPool command_pool,
                                                     VkQueue queue)
{
  VkCommandBuffer command_buffer = begin_single_time_commands(command_pool);

  VkBufferCopy copy_region = {
      .srcOffset = 0,
      .dstOffset = 0,
      .size = size,
  };
  vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

  end_single_time_commands(command_buffer, command_pool, queue);
}

void vktut::hello_triangle::application::end_single_time_commands(
    VkCommandBuffer command_buffer, VkCommandPool command_pool, VkQueue queue)
{
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &command_buffer,
  };

  vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(queue);

  vkFreeCommandBuffers(m_device, command_pool, 1, &command_buffer);
}

void vktut::hello_triangle::application::transition_image_layout(
    VkImage image,
    VkFormat format,
    VkImageLayout old_layout,
    VkImageLayout new_layout,
    std::uint32_t mip_levels,
    VkCommandPool command_pool,
    VkQueue queue)
{
  VkCommandBuffer command_buffer = begin_single_time_commands(command_pool);

  VkImageMemoryBarrier barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcAccessMask = 0,
      .dstAccessMask = 0,
      .oldLayout = old_layout,
      .newLayout = new_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = mip_levels,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (has_stencil_component(format)) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  }
  // else keep the color bit

  auto source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  auto destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED
      && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
             && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED
             && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else {
    throw std::invalid_argument {"unsupported layout transition!"};
  }

  vkCmdPipelineBarrier(command_buffer,
                       source_stage,
                       destination_stage,
                       0,
                       0,
                       nullptr,
                       0,
                       nullptr,
                       1,
                       &barrier);

  end_single_time_commands(command_buffer, command_pool, queue);
}

void vktut::hello_triangle::application::generate_mipmaps(
    VkImage image,
    VkFormat image_format,
    std::int32_t tex_width,
    std::int32_t tex_height,
    std::uint32_t mip_levels)
{
  VkFormatProperties format_properties;
  vkGetPhysicalDeviceFormatProperties(
      m_physical_device, image_format, &format_properties);

  if ((format_properties.optimalTilingFeatures
       & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
      == 0U)
  {
    throw std::runtime_error {
        "texture image format does not support linear blitting!"};
  }

  VkCommandBuffer command_buffer = begin_single_time_commands(m_command_pool);

  VkImageMemoryBarrier barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  std::int32_t mip_width = tex_width;
  std::int32_t mip_height = tex_height;
  for (std::uint32_t i = 1; i < mip_levels; ++i) {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &barrier);

    VkImageBlit blit = {
        .srcSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i - 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .srcOffsets =
            {
                {0, 0, 0},
                {mip_width, mip_height, 1},
            },
        .dstSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .dstOffsets =
            {
                {0, 0, 0},
                {
                    mip_width > 1 ? mip_width / 2 : 1,
                    mip_height > 1 ? mip_height / 2 : 1,
                    1,
                },
            },
    };

    vkCmdBlitImage(command_buffer,
                   image,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1,
                   &blit,
                   VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &barrier);

    if (mip_width > 1) {
      mip_width /= 2;
    }
    if (mip_height > 1) {
      mip_height /= 2;
    }
  }

  barrier.subresourceRange.baseMipLevel = mip_levels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(command_buffer,
                       VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                       0,
                       0,
                       nullptr,
                       0,
                       nullptr,
                       1,
                       &barrier);

  end_single_time_commands(command_buffer, m_command_pool, m_graphics_queue);
}

void vktut::hello_triangle::application::copy_buffer_to_image(
    VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height)
{
  VkCommandBuffer command_buffer =
      begin_single_time_commands(m_transfer_command_pool);

  VkBufferImageCopy region = {
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
      .imageOffset = {0, 0, 0},
      .imageExtent = {width, height, 1},
  };

  vkCmdCopyBufferToImage(command_buffer,
                         buffer,
                         image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         1,
                         &region);

  end_single_time_commands(
      command_buffer, m_transfer_command_pool, m_transfer_queue);
}

VKAPI_ATTR VkBool32 VKAPI_CALL
vktut::hello_triangle::application::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT /*message_severity*/,
    VkDebugUtilsMessageTypeFlagsEXT /*message_type*/,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* /*user_data*/)
{
  std::cerr << "[vktut::hello_triangle::application::debug_callback] "
            << callback_data->pMessage << "\n";

  return VK_FALSE;
}

int vktut::hello_triangle::application::rate_device_suitability(
    VkPhysicalDevice device)
{
  int score = 0;
  VkPhysicalDeviceProperties device_properties;
  VkPhysicalDeviceFeatures device_features;
  vkGetPhysicalDeviceProperties(device, &device_properties);
  vkGetPhysicalDeviceFeatures(device, &device_features);

  if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 1000;
  }
  score += static_cast<int>(device_properties.limits.maxImageDimension2D);

  auto indices = vktut::vulkan::queue_family_indices::find(device, m_surface);
  if (!indices.is_complete()) {
    return 0;
  }

  if (!check_device_extensions_support(device)) {
    return 0;
  }

  vulkan::swap_chain_support_details swap_chain_support =
      query_swap_chain_support(device);
  if (swap_chain_support.formats.empty()
      || swap_chain_support.present_modes.empty())
  {
    return 0;
  }
  score += static_cast<int>(swap_chain_support.present_modes.size()
                            + swap_chain_support.formats.size());

  if (device_features.samplerAnisotropy == VK_FALSE) {
    return 0;
  }

  return score;
}

VkExtent2D vktut::hello_triangle::application::choose_swap_extent(
    const VkSurfaceCapabilitiesKHR& capabilities)
{
  if (capabilities.currentExtent.width
      != std::numeric_limits<std::uint32_t>::max())
  {
    return capabilities.currentExtent;
  }

  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(m_window, &width, &height);

  VkExtent2D actual_extent = {static_cast<std::uint32_t>(width),
                              static_cast<std::uint32_t>(height)};
  actual_extent.width = std::clamp(actual_extent.width,
                                   capabilities.minImageExtent.width,
                                   capabilities.maxImageExtent.width);
  actual_extent.height = std::clamp(actual_extent.height,
                                    capabilities.minImageExtent.height,
                                    capabilities.maxImageExtent.height);

  return actual_extent;
}

std::uint32_t vktut::hello_triangle::application::find_memory_type(
    std::uint32_t type_filter, VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memory_properties);

#ifndef NDEBUG
  // NOLINTNEXTLINE(hicpp-no-array-decay,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  assert(memory_properties.memoryTypeCount <= 32);
#endif

  for (std::uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
    if ((type_filter & (1 << i)) != 0U
        // as long as memory_properties.memoryTypeCount <= 32 this is safe...
        // that property is asserted above
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        && (memory_properties.memoryTypes[i].propertyFlags & properties)
            == properties)
    {
      return i;
    }
  }

  throw std::runtime_error {"failed to find suitable memory type!"};
}

VkFormat vktut::hello_triangle::application::find_supported_format(
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features)
{
  for (auto format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR
        && (props.linearTilingFeatures & features) == features)
    {
      return format;
    }
    if (tiling == VK_IMAGE_TILING_OPTIMAL
        && (props.optimalTilingFeatures & features) == features)
    {
      return format;
    }
  }

  throw std::runtime_error {"failed to find supported format!"};
}

VkFormat vktut::hello_triangle::application::find_depth_format()
{
  return find_supported_format(
      std::vector {
          VK_FORMAT_D32_SFLOAT,
          VK_FORMAT_D32_SFLOAT_S8_UINT,
          VK_FORMAT_D24_UNORM_S8_UINT,
      },
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkSampleCountFlagBits
vktut::hello_triangle::application::get_max_usable_sample_count()
{
  VkPhysicalDeviceProperties physical_device_properties;
  vkGetPhysicalDeviceProperties(m_physical_device, &physical_device_properties);

  VkSampleCountFlags counts =
      physical_device_properties.limits.framebufferColorSampleCounts
      & physical_device_properties.limits.framebufferDepthSampleCounts;
  for (auto count : std::vector {
           VK_SAMPLE_COUNT_64_BIT,
           VK_SAMPLE_COUNT_32_BIT,
           VK_SAMPLE_COUNT_16_BIT,
           VK_SAMPLE_COUNT_8_BIT,
           VK_SAMPLE_COUNT_4_BIT,
           VK_SAMPLE_COUNT_2_BIT,
       })
  {
    if ((counts & count) != 0U) {
      return count;
    }
  }
  return VK_SAMPLE_COUNT_1_BIT;
}

bool vktut::hello_triangle::application::has_stencil_component(VkFormat format)
{
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT
      || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkPresentModeKHR vktut::hello_triangle::application::choose_swap_present_mode(
    const std::vector<VkPresentModeKHR>& available_present_modes)
{
  if (std::find(available_present_modes.begin(),
                available_present_modes.end(),
                VK_PRESENT_MODE_MAILBOX_KHR)
      != available_present_modes.end())
  {
    return VK_PRESENT_MODE_MAILBOX_KHR;
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR
vktut::hello_triangle::application::choose_swap_surface_format(
    const std::vector<VkSurfaceFormatKHR>& available_formats)
{
  auto format = std::find_if(
      available_formats.begin(),
      available_formats.end(),
      [](const auto& available_format)
      {
        return available_format.format == VK_FORMAT_B8G8R8A8_SRGB
            && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      });

  if (format == available_formats.end()) {
    return available_formats[0];
  }
  return *format;
}

bool vktut::hello_triangle::application::check_device_extensions_support(
    VkPhysicalDevice device)
{
  std::uint32_t extension_count = 0;
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &extension_count, nullptr);

  std::vector<VkExtensionProperties> available_extensions;
  available_extensions.resize(extension_count);
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &extension_count, available_extensions.data());

  std::unordered_set<std::string> required_extensions = {
      device_extensions.begin(), device_extensions.end()};
  for (const auto& extension : available_extensions) {
    required_extensions.erase(
        static_cast<const char*>(extension.extensionName));
  }

  return required_extensions.empty();
}
