#pragma once

#include "XmlParser.h"
#include <format>
#include <string_view>

namespace sde4::parser {

class StatefulXmlParser {
  static constexpr std::string AllWhitespaces = " \t\r\n\f\t\v";
  static constexpr std::string_view ValidNameChars =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789"
      "_-:.";
  XmlParseResult m_result;
  std::string_view m_xmlContent;
  std::size_t m_currentPosition{};

public:
  explicit constexpr StatefulXmlParser(std::string_view xmlContent)
      : m_xmlContent(xmlContent) {}

  constexpr XmlParseResult parse() {
    m_currentPosition = 0;
    domain::TreeNode* root = parseNode(nullptr);
    m_result.m_rootNodes.push_back(root);
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
    expectNoEof();
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
    // skip whitespaces
    m_currentPosition =
        m_xmlContent.find_first_not_of(AllWhitespaces, m_currentPosition);
  }

  constexpr std::string_view readName() {
    skipWhitespace();
    const auto start = m_currentPosition;
    m_currentPosition = m_xmlContent.find_first_not_of(ValidNameChars, start);
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
