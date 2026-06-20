#pragma once

#include "../../util/CharSet.h"
#include "XmlParser.h"
#include <format>
#include <string_view>

namespace sde4::parser {

class StatefulXmlParser {
  XmlParseResult m_result{};
  std::size_t m_currentPosition{};
  std::string_view m_xmlContent;

public:
  explicit constexpr StatefulXmlParser(std::string_view xmlContent)
      : m_xmlContent(xmlContent) {}

  constexpr XmlParseResult parse() {
    m_currentPosition = 0;
    while (!eof()) {
      skipWhitespace();
      if (eof())
        break;
      if (consume("<!--")) {
        domain::TreeNode* comment = parseComment(nullptr);
        m_result.m_rootNodes.push_back(comment);
      } else if (consume("<?")) {
        domain::TreeNode* pi = parseProcessingInstruction(nullptr);
        m_result.m_rootNodes.push_back(pi);
      } else if (consume("<!DOCTYPE")) {
        domain::TreeNode* dt = parseDoctype();
        m_result.m_rootNodes.push_back(dt);
      } else if (peek() == '<') {
        domain::TreeNode* root = parseNode(nullptr);
        m_result.m_rootNodes.push_back(root);
      } else {
        addError("Unexpected content at top level.");
        // skip to next '<' to avoid infinite loop
        auto next = m_xmlContent.find('<', m_currentPosition);
        if (next == std::string_view::npos) {
          m_currentPosition = m_xmlContent.size();
        } else {
          m_currentPosition = next;
        }
      }
    }
    return std::exchange(m_result, {});
  }

private:
  constexpr domain::TreeNode* parseNode(domain::TreeNode* parent) {
    expect('<');
    domain::TreeNode& element{m_result.m_allNodes.emplace_back()};
    element.m_parent = parent;
    element.m_name = readName();
    element.m_type = domain::NodeType::Element;
    skipWhitespace();

    // parse attributes (before '>' or '/>')
    while (peek() != '>' && peek() != '/') {
      domain::TreeNode* attribute = &m_result.m_allNodes.emplace_back();
      element.m_children.push_back(attribute);
      attribute->m_parent = &element;
      attribute->m_name = readName();
      attribute->m_type = domain::NodeType::Attribute;
      skipWhitespace();
      expect('=');
      skipWhitespace();
      attribute->m_value = parseQuotedValue();
      skipWhitespace();
    }

    // self-closing tag
    if (consume("/>")) {
      return &element;
    }

    expect('>');
    skipWhitespace();

    // parse children
    while (true) {
      if (peek() == '<') {
        if (consume("</")) {
          skipWhitespace();
          expect(element.m_name);
          expect('>');
          return &element;
        }
        if (consume("<!--")) {
          domain::TreeNode* comment = parseComment(&element);
          element.m_children.push_back(comment);
          skipWhitespace();
          if (eof()) {
            return &element;
          }
        } else if (consume("<?")) {
          domain::TreeNode* pi = parseProcessingInstruction(&element);
          element.m_children.push_back(pi);
          skipWhitespace();
          if (eof()) {
            return &element;
          }
        } else {
          domain::TreeNode* child = parseNode(&element);
          element.m_children.push_back(child);
          skipWhitespace();
        }
      } else if (eof()) {
        addError("Unexpected end of file.");
        return &element;
      } else {
        // text content — skip to next '<'
        auto next = m_xmlContent.find('<', m_currentPosition);
        if (next == std::string_view::npos) {
          addError("Expected '<'.");
          m_currentPosition = m_xmlContent.size();
          return &element;
        }
        element.m_value =
            m_xmlContent.substr(m_currentPosition, next - m_currentPosition);
        m_currentPosition = next;
      }
    }
  }

  constexpr bool consume(char c) {
    expectNoEof();
    if (peek() == c) {
      ++m_currentPosition;
      return true;
    }
    return false;
  }

  constexpr bool consume(std::string_view str) {
    expectNoEof();
    if (m_xmlContent.compare(m_currentPosition, str.size(), str) == 0) {
      m_currentPosition += str.size();
      return true;
    }
    return false;
  }

  constexpr void expect(char c) {
    if (!consume(c)) {
      addError(std::format("Expected: {}. Found: {}.", c, peek()));
    }
  }

  constexpr void expect(std::string_view str) {
    if (!consume(str)) {
      addError(std::format("Expected: {}. Found: {}.", str, peek()));
    }
  }

  constexpr void expectNoEof() {
    if (eof()) {
      addError("Unexpected end of file.");
    }
  }

  constexpr bool eof() const {
    return m_currentPosition >= m_xmlContent.size();
  }

  constexpr char peek() const { return m_xmlContent.at(m_currentPosition); }

  constexpr void addError(std::string message) {
    m_result.m_errors.emplace_back(std::move(message), m_currentPosition);
  }

  constexpr void skipWhitespace() {
    m_currentPosition =
        util::findFirstNotWhitespace(m_xmlContent, m_currentPosition);
  }

  constexpr std::string_view readName() {
    skipWhitespace();
    const auto start = m_currentPosition;
    m_currentPosition = util::findFirstNotNameChar(m_xmlContent, start);
    return m_xmlContent.substr(start, m_currentPosition - start);
  }

  constexpr domain::TreeNode* parseComment(domain::TreeNode* parent) {
    const auto start = m_currentPosition;
    auto endPos = m_xmlContent.find("-->", start);
    if (endPos == std::string_view::npos) {
      addError("Expected closing '-->'.");
      m_currentPosition = m_xmlContent.size();
    }
    domain::TreeNode& comment{m_result.m_allNodes.emplace_back()};
    comment.m_parent = parent;
    comment.m_type = domain::NodeType::Comment;
    comment.m_value = m_xmlContent.substr(start, endPos - start);
    consume("-->");
    return &comment;
  }

  constexpr domain::TreeNode*
  parseProcessingInstruction(domain::TreeNode* parent) {
    domain::TreeNode& pi{m_result.m_allNodes.emplace_back()};
    pi.m_parent = parent;
    pi.m_type = domain::NodeType::ProcessingInstruction;
    pi.m_name = readName();
    skipWhitespace();
    const auto start = m_currentPosition;
    auto endPos = m_xmlContent.find("?>", start);
    if (endPos == std::string_view::npos) {
      addError("Expected closing '?>'.");
      m_currentPosition = m_xmlContent.size();
    }
    pi.m_value = m_xmlContent.substr(start, endPos - start);
    consume("?>");
    return &pi;
  }

  constexpr domain::TreeNode* parseDoctype() {
    domain::TreeNode& dt{m_result.m_allNodes.emplace_back()};
    dt.m_parent = nullptr;
    dt.m_type = domain::NodeType::Doctype;
    dt.m_name = readName();
    skipWhitespace();
    const auto start = m_currentPosition;
    std::size_t bracketDepth = 0;
    bool inQuotes = false;
    char quoteChar = 0;
    while (m_currentPosition < m_xmlContent.size()) {
      if (inQuotes) {
        if (peek() == quoteChar) {
          inQuotes = false;
        }
      } else if (peek() == '"' || peek() == '\'') {
        inQuotes = true;
        quoteChar = peek();
      } else if (peek() == '[') {
        ++bracketDepth;
      } else if (peek() == ']') {
        if (bracketDepth > 0) {
          --bracketDepth;
        }
      } else if (peek() == '>' && bracketDepth == 0) {
        dt.m_value = m_xmlContent.substr(start, m_currentPosition - start);
        ++m_currentPosition;
        return &dt;
      }
      ++m_currentPosition;
    }
    addError("Expected closing '>' for DOCTYPE.");
    dt.m_value = m_xmlContent.substr(start);
    return &dt;
  }

  constexpr std::string_view parseQuotedValue() {
    if (peek() != '"' && peek() != '\'') {
      addError(std::format("Expected '\"' or '\\'', found: {}.", peek()));
      return {};
    }
    const char quoteChar = peek();
    ++m_currentPosition; // skip opening quote
    const auto start = m_currentPosition;
    m_currentPosition =
        m_xmlContent.find(quoteChar, start); // TODO: handle escaped quotes
    if (m_currentPosition == std::string_view::npos) {
      addError(std::format("Expected closing quote '{}'.", quoteChar));
      m_currentPosition = m_xmlContent.size();
    }
    auto ret = m_xmlContent.substr(start, m_currentPosition - start);
    expect(quoteChar); // skip closing quote
    return ret;
  }
};

} // namespace sde4::parser
