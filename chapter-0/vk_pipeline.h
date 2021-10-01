#pragma once

#include <vector>

#include <vk_types.h>

namespace vulkan
{
struct pipeline_builder
{
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
  VkPipelineVertexInputStateCreateInfo vertex_input_info {};
  VkPipelineInputAssemblyStateCreateInfo input_assembly {};
  VkViewport viewport {};
  VkRect2D scissor {};
  VkPipelineRasterizationStateCreateInfo rasterizer {};
  VkPipelineColorBlendAttachmentState color_blend_attachment {};
  VkPipelineMultisampleStateCreateInfo multisampling {};
  VkPipelineLayout pipeline_layout {};

  pipeline_builder() = default;
  VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
};
}  // namespace vulkan
