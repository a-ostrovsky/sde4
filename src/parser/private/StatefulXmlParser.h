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

  // Advance past malformed input if a parser loop makes no progress.
  class ProgressGuard {
    std::size_t& m_position;
    const std::size_t m_end;
    const std::size_t m_start;

  public:
    constexpr ProgressGuard(std::size_t& position, std::size_t end) noexcept
        : m_position{position}, m_end{end}, m_start{position} {}

    ProgressGuard(const ProgressGuard&) = delete;
    ProgressGuard& operator=(const ProgressGuard&) = delete;
    ProgressGuard(ProgressGuard&&) = delete;
    ProgressGuard& operator=(ProgressGuard&&) = delete;

    constexpr ~ProgressGuard() noexcept {
      if (m_position == m_start && m_position < m_end) {
        ++m_position;
      }
    }
  };

public:
  explicit constexpr StatefulXmlParser(std::string_view xmlContent)
      : m_xmlContent(xmlContent) {}

  constexpr XmlParseResult parse() {
    m_currentPosition = 0;
    while (!eof()) {
      const ProgressGuard progress{m_currentPosition, m_xmlContent.size()};
      skipWhitespace();
      if (eof())
        break;
      if (consume("<!--")) {
        domain::TreeNode* comment = parseComment(nullptr);
        m_result.m_rootNodes.push_back(comment);
      } else if (consume("<?xml")) {
        if (!eof() && (util::isWhitespace(peek()) || peek() == '?')) {
          domain::TreeNode* decl = parseDeclaration();
          m_result.m_rootNodes.push_back(decl);
        } else { // <?xml-stylesheet etc. - fall through to PI
          m_currentPosition -= sizeof("<?xml") - 1;
          consume("<?");
          domain::TreeNode* pi = parseProcessingInstruction(nullptr);
          m_result.m_rootNodes.push_back(pi);
        }
      } else if (consume("<?")) {
        domain::TreeNode* pi = parseProcessingInstruction(nullptr);
        m_result.m_rootNodes.push_back(pi);
      } else if (consume("<!DOCTYPE")) {
        domain::TreeNode* dt = parseDoctype();
        m_result.m_rootNodes.push_back(dt);
      } else if (consume("<![CDATA[")) {
        domain::TreeNode* cdata = parseCdata(nullptr);
        m_result.m_rootNodes.push_back(cdata);
      } else if (peek() == '<') {
        domain::TreeNode* root = parseNode(nullptr);
        m_result.m_rootNodes.push_back(root);
      } else {
        addError("Unexpected content at top level.");
        // skip to next '<' to avoid infinite loop
        const auto next = m_xmlContent.find('<', m_currentPosition);
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
    element.m_content.setName(readName());
    element.m_type = domain::NodeType::Element;
    skipWhitespace();

    // parse attributes (before '>' or '/>')
    while (peek() != '>' && peek() != '/') {
      const ProgressGuard progress{m_currentPosition, m_xmlContent.size()};
      domain::TreeNode* attribute = &m_result.m_allNodes.emplace_back();
      appendChild(element, attribute);
      attribute->m_content.setName(readName());
      attribute->m_type = domain::NodeType::Attribute;
      skipWhitespace();
      expect('=');
      skipWhitespace();
      attribute->m_content.setValue(parseQuotedValue());
      skipWhitespace();
    }

    // self-closing tag
    if (consume("/>")) {
      return &element;
    }

    expect('>');

    // parse children
    while (true) {
      const ProgressGuard progress{m_currentPosition, m_xmlContent.size()};
      if (peek() == '<') {
        if (consume("</")) {
          skipWhitespace();
          expect(element.m_content.getName());
          expect('>');
          return &element;
        }
        if (consume("<!--")) {
          domain::TreeNode* comment = parseComment(&element);
          appendChild(element, comment);
          if (eof()) {
            return &element;
          }
        } else if (consume("<?")) {
          domain::TreeNode* pi = parseProcessingInstruction(&element);
          appendChild(element, pi);
          if (eof()) {
            return &element;
          }
        } else if (consume("<![CDATA[")) {
          domain::TreeNode* cdata = parseCdata(&element);
          appendChild(element, cdata);
          if (eof()) {
            return &element;
          }
        } else {
          domain::TreeNode* child = parseNode(&element);
          appendChild(element, child);
        }
      } else if (eof()) {
        addError("Unexpected end of file.");
        return &element;
      } else {
        domain::TreeNode* textNode = parseText(&element);
        if (textNode == nullptr) {
          return &element;
        }
        appendChild(element, textNode);
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
    const auto endPos = m_xmlContent.find("-->", start);
    if (endPos == std::string_view::npos) {
      addError("Expected closing '-->'.");
      m_currentPosition = m_xmlContent.size();
    } else {
      m_currentPosition = endPos;
    }
    domain::TreeNode& comment{m_result.m_allNodes.emplace_back()};
    comment.m_parent = parent;
    comment.m_type = domain::NodeType::Comment;
    comment.m_content.setValue(m_xmlContent.substr(start, endPos - start));
    consume("-->");
    return &comment;
  }

  constexpr domain::TreeNode* parseCdata(domain::TreeNode* parent) {
    const auto start = m_currentPosition;
    const auto endPos = m_xmlContent.find("]]>", start);
    if (endPos == std::string_view::npos) {
      addError("Expected closing ']]>'.");
      m_currentPosition = m_xmlContent.size();
    } else {
      m_currentPosition = endPos;
    }
    domain::TreeNode& cdata{m_result.m_allNodes.emplace_back()};
    cdata.m_parent = parent;
    cdata.m_type = domain::NodeType::Cdata;
    cdata.m_content.setValue(m_xmlContent.substr(start, endPos - start));
    consume("]]>");
    return &cdata;
  }

  constexpr domain::TreeNode*
  parseProcessingInstruction(domain::TreeNode* parent) {
    domain::TreeNode& pi{m_result.m_allNodes.emplace_back()};
    pi.m_parent = parent;
    pi.m_type = domain::NodeType::ProcessingInstruction;
    pi.m_content.setName(readName());
    skipWhitespace();
    const auto start = m_currentPosition;
    const auto endPos = m_xmlContent.find("?>", start);
    if (endPos == std::string_view::npos) {
      addError("Expected closing '?>'.");
      m_currentPosition = m_xmlContent.size();
    } else {
      m_currentPosition = endPos;
    }
    pi.m_content.setValue(m_xmlContent.substr(start, endPos - start));
    consume("?>");
    return &pi;
  }

  constexpr domain::TreeNode* parseDoctype() {
    domain::TreeNode& dt{m_result.m_allNodes.emplace_back()};
    dt.m_parent = nullptr;
    dt.m_type = domain::NodeType::Doctype;
    dt.m_content.setName(readName());
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
        dt.m_content.setValue(
            m_xmlContent.substr(start, m_currentPosition - start));
        ++m_currentPosition;
        return &dt;
      }
      ++m_currentPosition;
    }
    addError("Expected closing '>' for DOCTYPE.");
    dt.m_content.setValue(m_xmlContent.substr(start));
    return &dt;
  }

  constexpr domain::TreeNode* parseDeclaration() {
    domain::TreeNode& decl{m_result.m_allNodes.emplace_back()};
    decl.m_parent = nullptr;
    decl.m_type = domain::NodeType::Declaration;
    decl.m_content.setName("xml");
    skipWhitespace();
    const auto start = m_currentPosition;
    const auto endPos = m_xmlContent.find("?>", start);
    if (endPos == std::string_view::npos) {
      addError("Expected closing '?>'.");
      m_currentPosition = m_xmlContent.size();
    } else {
      m_currentPosition = endPos;
    }
    decl.m_content.setValue(m_xmlContent.substr(start, endPos - start));
    consume("?>");
    return &decl;
  }

  constexpr domain::TreeNode* parseText(domain::TreeNode* parent) {
    const auto next = m_xmlContent.find('<', m_currentPosition);
    if (next == std::string_view::npos) {
      addError("Expected '<'.");
      m_currentPosition = m_xmlContent.size();
      return nullptr;
    }
    domain::TreeNode& textNode{m_result.m_allNodes.emplace_back()};
    textNode.m_parent = parent;
    textNode.m_type = domain::NodeType::Text;
    textNode.m_content.setValue(
        m_xmlContent.substr(m_currentPosition, next - m_currentPosition));
    m_currentPosition = next;
    return &textNode;
  }

  static constexpr void appendChild(domain::TreeNode& parent,
                                    domain::TreeNode* child) {
    child->m_parent = &parent;
    child->m_nextSibling = nullptr;
    if (parent.m_firstChild) {
      parent.m_lastChild->m_nextSibling = child;
    } else {
      parent.m_firstChild = child;
    }
    parent.m_lastChild = child;
    ++parent.m_numChildren;
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
    const auto ret = m_xmlContent.substr(start, m_currentPosition - start);
    expect(quoteChar); // skip closing quote
    return ret;
  }
};

} // namespace sde4::parser
