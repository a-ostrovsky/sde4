#include "../../../src/parser/private/XmlParser.h"
#include <doctest.h>
#ifdef _MSC_VER
#include <ostream>
#endif
#include <string>
#include <vector>

using namespace sde4::parser;
using namespace sde4::domain;

TEST_CASE("parse xml declaration as pi") {
  const std::string xml = R"(<?xml version="1.0"?><root/>)";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::ProcessingInstruction);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "xml");
  CHECK(result.m_rootNodes.at(0)->m_content.getValue() == R"(version="1.0")");
  CHECK(result.m_rootNodes.at(1)->m_content.getName() == "root");
}

TEST_CASE("parse malformed pi missing closing") {
  const std::string xml = "<?pi unclosed";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::ProcessingInstruction);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "pi");
  CHECK(result.m_rootNodes.at(0)->m_content.getValue() == "unclosed");
  CHECK_FALSE(result.m_errors.empty());
}

TEST_CASE("parse empty pi") {
  const std::string xml = "<?empty?><root/>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::ProcessingInstruction);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "empty");
  CHECK(result.m_rootNodes.at(0)->m_content.getValue().empty());
  CHECK(result.m_rootNodes.at(1)->m_content.getName() == "root");
}

TEST_CASE("parse pi in epilog") {
  const std::string xml = "<root/><?pi after?>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_content.getName() == "root");
  CHECK(result.m_rootNodes.at(1)->m_type == NodeType::ProcessingInstruction);
  CHECK(result.m_rootNodes.at(1)->m_content.getName() == "pi");
  CHECK(result.m_rootNodes.at(1)->m_content.getValue() == "after");
}

TEST_CASE("parse pi inside element") {
  const std::string xml = "<root><?pi data?></root>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_numChildren == 1);
  CHECK(result.m_rootNodes.at(0)->m_firstChild->m_type ==
        NodeType::ProcessingInstruction);
  CHECK(result.m_rootNodes.at(0)->m_firstChild->m_content.getName() == "pi");
  CHECK(result.m_rootNodes.at(0)->m_firstChild->m_content.getValue() == "data");
}
