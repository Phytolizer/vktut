#pragma once

#include <glm/glm.hpp>

namespace vktut::shaders
{
struct uniform_buffer_object
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};
}  // namespace vktut::shaders
