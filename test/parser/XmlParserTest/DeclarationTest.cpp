#include "../../../src/parser/private/XmlParser.h"
#include <doctest.h>
#ifdef _MSC_VER
#include <ostream>
#endif
#include <string>
#include <vector>

using namespace sde4::parser;
using namespace sde4::domain;

TEST_CASE("parse xml declaration with full pseudo-attributes") {
  const std::string xml =
      R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><root/>)";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::Declaration);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "xml");
  CHECK(result.m_rootNodes.at(0)->m_content.getValue() ==
        R"(version="1.0" encoding="UTF-8" standalone="yes")");
  CHECK(result.m_rootNodes.at(1)->m_content.getName() == "root");
}

TEST_CASE("parse xml declaration malformed missing closing") {
  const std::string xml = R"(<?xml version="1.0")";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::Declaration);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "xml");
  CHECK(result.m_rootNodes.at(0)->m_content.getValue() == R"(version="1.0")");
  CHECK_FALSE(result.m_errors.empty());
}

TEST_CASE("parse xml declaration") {
  const std::string xml = R"(<?xml version="1.0"?><root/>)";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::Declaration);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "xml");
  CHECK(result.m_rootNodes.at(0)->m_content.getValue() == R"(version="1.0")");
  CHECK(result.m_rootNodes.at(1)->m_content.getName() == "root");
}
