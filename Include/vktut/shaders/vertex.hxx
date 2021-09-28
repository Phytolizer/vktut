#pragma once

#include <array>

#include <glm/glm.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vktut::shaders
{
struct vertex
{
  glm::vec2 pos;
  glm::vec3 color;
  glm::vec2 tex_coord;

  static VkVertexInputBindingDescription binding_description();
  static std::array<VkVertexInputAttributeDescription, 3>
  attribute_descriptions();
};
}  // namespace vktut::shaders
