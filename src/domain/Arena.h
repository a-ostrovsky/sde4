#pragma once

#include "../util/VectorList.h"
#include "DataModel.h"
#include <string>

namespace sde4::domain {
struct Arena {
  sde4::util::VectorList<TreeNode> m_nodes{};
  std::string m_fileContent{};
};
} // namespace sde4::domain
