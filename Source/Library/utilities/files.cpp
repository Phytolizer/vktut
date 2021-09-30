#include <fstream>
#include <stdexcept>
#include <string>

#include "vktut/utilities/files.hpp"

std::vector<char> vktut::utilities::files::read_file(std::string_view file_name)
{
  std::ifstream file {std::string {file_name},
                      std::ios::ate | std::ios::binary};
  if (!file) {
    throw std::runtime_error {"failed to open file!"};
  }

  std::size_t file_size = file.tellg();
  std::vector<char> buffer;
  buffer.resize(file_size);
  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(file_size));

  return buffer;
}
