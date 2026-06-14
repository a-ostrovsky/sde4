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

  std::vector<char> buffer(sz);

  size_t total = 0;
  while (total < sz) {
    const size_t chunk = std::fread(buffer.data() + total, 1, sz - total, f);
    if (chunk == 0) {
      break;
    }
    total += chunk;
  }
  std::fclose(f);

  buffer.resize(total);

  return std::string(buffer.data(), buffer.size());
}

} // namespace sde4::parser
