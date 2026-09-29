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

    // We should disable SSO as XmlParser creates references into the string.
    // In case of short files (e.g. <a/>) moving the string will just copy it
    // and the references will dangle.
    disableSSOForString(fileContent);

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

  constexpr static void disableSSOForString(std::string& s) {
#if !defined(_LIBCPP_VERSION) && \
    !defined(__GLIBCXX__) && \
    !defined(_MSVC_STL_VERSION)
#error "Review std::string storage and move behavior."
#endif
    // Increase capacity beyond what fits inside the string object
    // to force heap storage.
    s.reserve(sizeof(std::string) + 1);
  }
};
} // namespace

namespace sde4::parser {
std::unique_ptr<XmlFileParser> XmlFileParser::create() {
  return std::make_unique<Impl>();
}
} // namespace sde4::parser
