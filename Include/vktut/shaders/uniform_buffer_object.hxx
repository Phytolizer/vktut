#pragma once

#include <glm/glm.hpp>

namespace vktut::shaders
{
struct uniform_buffer_object
{
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};
}  // namespace vktut::shaders
