#include "vk_pipeline.h"

VkPipeline vulkan::pipeline_builder::build_pipeline(VkDevice device,
                                                    VkRenderPass pass)
{
  VkPipelineViewportStateCreateInfo viewport_state {};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.pNext = nullptr;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  VkPipelineColorBlendStateCreateInfo color_blending {};
  color_blending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.pNext = nullptr;
  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY;
  color_blending.attachmentCount = 1;
  color_blending.pAttachments = &color_blend_attachment;

  VkGraphicsPipelineCreateInfo pipeline_info {};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.pNext = nullptr;
  pipeline_info.stageCount = static_cast<std::uint32_t>(shader_stages.size());
  pipeline_info.pStages = shader_stages.data();
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &rasterizer;
  pipeline_info.pMultisampleState = &multisampling;
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.layout = pipeline_layout;
  pipeline_info.renderPass = pass;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

  VkPipeline new_pipeline;
  if (vkCreateGraphicsPipelines(
          device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &new_pipeline)
      != VK_SUCCESS)
  {
    return VK_NULL_HANDLE;
  }
  return new_pipeline;
}
