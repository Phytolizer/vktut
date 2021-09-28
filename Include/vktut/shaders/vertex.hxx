#pragma once

#include <array>
#include <unordered_map>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vktut::shaders
{
struct vertex
{
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 tex_coord;

  bool operator==(const vertex& other) const;

  static VkVertexInputBindingDescription binding_description();
  static std::array<VkVertexInputAttributeDescription, 3>
  attribute_descriptions();
};
}  // namespace vktut::shaders

namespace std
{
template<>
struct hash<vktut::shaders::vertex>
{
  std::size_t operator()(const vktut::shaders::vertex& v) const;
};
}  // namespace std
