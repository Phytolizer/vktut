#include "vktut/shaders/vertex.hpp"

bool vktut::shaders::vertex::operator==(const vertex& other) const
{
  return pos == other.pos && color == other.color
      && tex_coord == other.tex_coord;
}

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

std::size_t std::hash<vktut::shaders::vertex>::operator()(
    const vktut::shaders::vertex& v) const
{
  return ((hash<glm::vec3>()(v.pos) ^ (hash<glm::vec3>()(v.color) << 1)) >> 1)
      ^ (hash<glm::vec2>()(v.tex_coord) << 1);
}
