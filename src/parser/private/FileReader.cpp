#include "FileReader.h"
#include <cstdio>
#include <cstring>

namespace sde4::parser {

std::expected<std::string, ParseError>
FileReader::loadFileIntoMemory(const std::filesystem::path& path) {
  const auto sz = std::filesystem::file_size(path);
  FILE* f = fopen(path.string().c_str(), "rb");
  if (!f) {
    const auto errorMsg = std::strerror(errno);
    return std::unexpected(ParseError{
        .m_message = std::string{"Failed to open file: "} + errorMsg,
    });
  }

  std::string buffer;
  buffer.resize_and_overwrite(sz, [&](char* data, std::size_t n) {
    std::size_t total = 0;
    while (total < n) {
      const std::size_t chunk = std::fread(data + total, 1, n - total, f);
      if (chunk == 0) {
        break;
      }
      total += chunk;
    }
    return total;
  });
  std::fclose(f);

  return buffer;
}

} // namespace sde4::parser
