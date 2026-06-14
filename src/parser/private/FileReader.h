#pragma once

#include "../IXmlFileParser.h"
#include <expected>
#include <filesystem>
#include <string>

namespace sde4::parser {

struct FileReader {
  static std::expected<std::string, ParseError>
  loadFileIntoMemory(const std::filesystem::path& path);
};

} // namespace sde4::parser
