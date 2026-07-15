#include "../../src/domain/DataModel.h"
#include "../../src/util/VectorList.h"
#include <doctest.h>

using sde4::domain::NodeType;
using sde4::domain::TreeNode;
using sde4::util::VectorList;

TEST_CASE("VectorList is empty") {
  VectorList<TreeNode> list{};
  REQUIRE(list.size() == 0u);
  REQUIRE(list.begin() == list.end());
}

TEST_CASE("VectorList add item") {
  VectorList<TreeNode> list{};
  TreeNode two{};
  two.m_type = NodeType::Attribute;
  list.push_back(TreeNode{});
  list.push_back(two);
  list.emplace_back();
  REQUIRE(list.size() == 3u);
  REQUIRE(list.begin()->m_type == NodeType::Element);
  REQUIRE(std::next(list.begin())->m_type == NodeType::Attribute);
  REQUIRE(std::next(std::next(list.begin()))->m_type == NodeType::Element);
}

TEST_CASE("VectorList iteration skips deleted element") {
  VectorList<TreeNode> list{};
  list.emplace_back();
  auto* ptr = &list.emplace_back();
  list.emplace_back();

  ptr->m_isDeleted = true;

  CHECK(list.begin()->m_type == NodeType::Element);
  CHECK(std::next(list.begin())->m_type == NodeType::Element);
  CHECK(std::next(std::next(list.begin())) == list.end());
}

TEST_CASE("VectorList compact removes deleted elements and remaps pointers") {
  VectorList<TreeNode> list{};
  TreeNode* ptr1 = &list.emplace_back();
  TreeNode* ptr2 = &list.emplace_back();
  TreeNode* ptr3 = &list.emplace_back();
  TreeNode* ptr4 = &list.emplace_back();
  ptr1->m_type = NodeType::Element;
  ptr2->m_type = NodeType::Attribute;
  ptr3->m_type = NodeType::Comment;
  ptr4->m_type = NodeType::Doctype;

  ptr2->m_isDeleted = true;
  ptr4->m_isDeleted = true;

  auto result = list.compact();

  REQUIRE(result.m_compacted.size() == 2u);
  CHECK(result.m_oldToNew.at(ptr1)->m_type == NodeType::Element);
  CHECK(result.m_oldToNew.at(ptr3)->m_type == NodeType::Comment);
  CHECK(!result.m_oldToNew.contains(ptr2));
  CHECK(!result.m_oldToNew.contains(ptr4));
}
