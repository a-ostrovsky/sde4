#include "XmlFileParser.h"
#include "../../domain/Arena.h"
#include "FileReader.h"
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

    sde4::domain::Arena arena {
      .m_fileContent = std::move(fileContentOrError.value())
    };

    ParseResult result{
        .m_path = path,
    };

    return result;
  }

};
} // namespace

namespace sde4::parser {
std::unique_ptr<XmlFileParser> XmlFileParser::create() {
  return std::make_unique<Impl>();
}
} // namespace sde4::parser
