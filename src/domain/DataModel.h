#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace sde4::domain {

enum class NodeType : std::uint8_t {
  Element,
  Attribute,
  // TODO: For later
  // eCDATA,
  // eCOMMENT,
  // eDECLARATION,
  // eDOCTYPE,
  // ePI
};

struct XmlError {
  std::string m_message{};
  std::size_t m_position{};
};

struct TreeNode {
  std::vector<TreeNode*> m_children{};
  std::string_view m_name{};
  std::string_view m_value{};
  TreeNode* m_parent{};
  NodeType m_type{};
};

} // namespace sde4::domain
