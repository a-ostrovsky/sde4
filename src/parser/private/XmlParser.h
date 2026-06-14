#pragma once

#include "../../domain/DataModel.h"
#include "../../util/VectorList.h"
#include <string_view>

namespace sde4::parser {

struct XmlParseResult {
  sde4::util::VectorList<sde4::domain::TreeNode> m_allNodes{};
  std::vector<sde4::domain::TreeNode*> m_rootNodes{};
  std::vector<sde4::domain::XmlError> m_errors{};
};

struct XmlParser {
  static XmlParseResult parse(std::string_view xmlContent);
};

} // namespace sde4::parser
