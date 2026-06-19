#include "FileReader.h"
#include <cstdio>
#include <cstring>
#include <memory>

namespace sde4::parser {

std::expected<std::string, ParseError>
FileReader::loadFileIntoMemory(const std::filesystem::path& path) {
  const auto sz = std::filesystem::file_size(path);
  constexpr auto closeFile = [](FILE* f) noexcept { std::fclose(f); };
  std::unique_ptr<FILE, decltype(closeFile)> f{
      fopen(path.string().c_str(), "rb"),
  };
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
      const std::size_t chunk = std::fread(data + total, 1, n - total, f.get());
      if (chunk == 0) {
        break;
      }
      total += chunk;
    }
    return total;
  });

  return buffer;
}

} // namespace sde4::parser
