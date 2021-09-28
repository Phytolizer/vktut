#include "vktut/shaders/vertex.hxx"

VkVertexInputBindingDescription vktut::shaders::vertex::binding_description()
{
  VkVertexInputBindingDescription binding_description = {
      .binding = 0,
      .stride = sizeof(vertex),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };

  return binding_description;
}

std::array<VkVertexInputAttributeDescription, 3>
vktut::shaders::vertex::attribute_descriptions()
{
  std::array attribute_descriptions = {
      VkVertexInputAttributeDescription {
          .location = 0,
          .binding = 0,
          .format = VK_FORMAT_R32G32B32_SFLOAT,
          .offset = offsetof(vertex, pos),
      },
      VkVertexInputAttributeDescription {
          .location = 1,
          .binding = 0,
          .format = VK_FORMAT_R32G32B32_SFLOAT,
          .offset = offsetof(vertex, color),
      },
      VkVertexInputAttributeDescription {
          .location = 2,
          .binding = 0,
          .format = VK_FORMAT_R32G32_SFLOAT,
          .offset = offsetof(vertex, tex_coord),
      },
  };

  return attribute_descriptions;
}
