#include "XmlParser.h"
#include "StatefulXmlParser.h"

namespace sde4::parser {

XmlParseResult XmlParser::parse(std::string_view xmlContent) {
  auto parser = StatefulXmlParser{xmlContent};
  return parser.parse();
}

} // namespace sde4::parser
