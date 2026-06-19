#include "XmlFileParser.h"
#include "../../domain/Arena.h"
#include "FileReader.h"
#include "XmlParser.h"
#include <filesystem>

namespace {
using namespace sde4::parser;

struct Impl : XmlFileParser {
  ParseResultOrParseError
  parseFile(const std::filesystem::path& path) override {
    auto fileContentOrError = FileReader::loadFileIntoMemory(path);
    if (!fileContentOrError) {
      return std::unexpected(fileContentOrError.error());
    }

    auto fileContent = std::move(fileContentOrError.value());

    auto parseResult = XmlParser::parse(fileContent);

    if (!parseResult.m_errors.empty()) {
      return std::unexpected(ParseError{
          .m_message = parseResult.m_errors.front().m_message,
      });
    }

    sde4::domain::Arena arena{
        .m_nodes = std::move(parseResult.m_allNodes),
        .m_fileContent = std::move(fileContent),
    };

    return ParseResult{
        .m_arena = std::move(arena),
        .m_rootNodes = std::move(parseResult.m_rootNodes),
        .m_path = path,
    };
  }

};
} // namespace

namespace sde4::parser {
std::unique_ptr<XmlFileParser> XmlFileParser::create() {
  return std::make_unique<Impl>();
}
} // namespace sde4::parser
