#pragma once

#include "../domain/Arena.h"
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace sde4::parser {

struct ParseError {
  std::string m_message{};
};

struct ParseResult {
  domain::Arena m_arena{};
  std::vector<domain::TreeNode*> m_rootNodes{};
  std::filesystem::path m_path{};
};

using ParseResultOrParseError = std::expected<ParseResult, ParseError>;

struct IXmlFileParser {
  virtual ~IXmlFileParser() = default;
  virtual ParseResultOrParseError parseFile(const std::filesystem::path&) = 0;
};

} // namespace sde4::parser
