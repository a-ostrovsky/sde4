#include "../../../src/parser/private/XmlParser.h"
#include <doctest.h>
#include <string>
#include <vector>

using namespace sde4::parser;
using namespace sde4::domain;

TEST_CASE("parse xml declaration as pi") {
  const std::string xml = R"(<?xml version="1.0"?><root/>)";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::ProcessingInstruction);
  CHECK(result.m_rootNodes.at(0)->m_name == "xml");
  CHECK(result.m_rootNodes.at(0)->m_value == R"(version="1.0")");
  CHECK(result.m_rootNodes.at(1)->m_name == "root");
}

TEST_CASE("parse malformed pi missing closing") {
  const std::string xml = "<?pi unclosed";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::ProcessingInstruction);
  CHECK(result.m_rootNodes.at(0)->m_name == "pi");
  CHECK(result.m_rootNodes.at(0)->m_value == "unclosed");
  CHECK_FALSE(result.m_errors.empty());
}

TEST_CASE("parse empty pi") {
  const std::string xml = "<?empty?><root/>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_type == NodeType::ProcessingInstruction);
  CHECK(result.m_rootNodes.at(0)->m_name == "empty");
  CHECK(result.m_rootNodes.at(0)->m_value.empty());
  CHECK(result.m_rootNodes.at(1)->m_name == "root");
}

TEST_CASE("parse pi in epilog") {
  const std::string xml = "<root/><?pi after?>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_name == "root");
  CHECK(result.m_rootNodes.at(1)->m_type == NodeType::ProcessingInstruction);
  CHECK(result.m_rootNodes.at(1)->m_name == "pi");
  CHECK(result.m_rootNodes.at(1)->m_value == "after");
}

TEST_CASE("parse pi inside element") {
  const std::string xml = "<root><?pi data?></root>";
  const auto result = XmlParser::parse(xml);
  CHECK(result.m_rootNodes.at(0)->m_children.size() == 1);
  CHECK(result.m_rootNodes.at(0)->m_children.at(0)->m_type ==
        NodeType::ProcessingInstruction);
  CHECK(result.m_rootNodes.at(0)->m_children.at(0)->m_name == "pi");
  CHECK(result.m_rootNodes.at(0)->m_children.at(0)->m_value == "data");
}
