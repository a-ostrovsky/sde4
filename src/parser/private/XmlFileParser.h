#pragma once

#include "../IXmlFileParser.h"

namespace sde4::parser {

struct XmlFileParser : IXmlFileParser {
  static std::unique_ptr<XmlFileParser> create();
};

} // namespace sde4::parser
