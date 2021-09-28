#pragma once

#include <string_view>
#include <vector>
namespace vktut::utilities
{
struct files
{
  static std::vector<char> read_file(std::string_view file_name);
};
}  // namespace vktut::utilities
