#include "../../src/util/VectorList.h"
#include <doctest.h>

using sde4::util::VectorList;

TEST_CASE("VectorList is empty") {
  VectorList<int> list{};
  REQUIRE(list.size() == 0u);
  REQUIRE(list.begin() == list.end());
}

TEST_CASE("VectorList add item") {
  VectorList<int> list{};
  const int two = 2;
  list.push_back(1);   // T&&
  list.push_back(two); // const &;
  list.emplace_back(3);
  REQUIRE(list.size() == 3u);
  REQUIRE(*list.begin() == 1);
  REQUIRE(*std::next(list.begin()) == 2);
  REQUIRE(*std::next(std::next(list.begin())) == 3);
}

TEST_CASE("VectorList delete and undelete item") {
  VectorList<int> list{};
  const auto* ptr = &list.emplace_back(1);
  list.softDelete(ptr);
  list.softDelete(ptr); // double delete to test idempotency
  REQUIRE(list.size() == 0u);
  list.softUndelete(ptr);
  list.softUndelete(ptr); // double undelete to test idempotency
  REQUIRE(list.size() == 1u);
}

TEST_CASE("VectorList iteration skips deleted element") {
  VectorList<int> list{};
  list.emplace_back(1);
  auto* ptr = &list.emplace_back(2);
  list.emplace_back(3);

  list.softDelete(ptr);

  CHECK(*list.begin() == 1);
  CHECK(*std::next(list.begin()) == 3);
  CHECK(std::next(std::next(list.begin())) == list.end());
}

TEST_CASE("VectorList compact removes deleted elements and remaps pointers") {
  VectorList<int> list{};
  const int* ptr1 = &list.emplace_back(1);
  const int* ptr2 = &list.emplace_back(2);
  const int* ptr3 = &list.emplace_back(3);
  const int* ptr4 = &list.emplace_back(4);

  list.softDelete(ptr2);
  list.softDelete(ptr4);

  auto result = list.compact();

  REQUIRE(result.m_compacted.size() == 2u);
  CHECK(*result.m_oldToNew.at(ptr1) == 1);
  CHECK(*result.m_oldToNew.at(ptr3) == 3);
  CHECK(!result.m_oldToNew.contains(ptr2));
  CHECK(!result.m_oldToNew.contains(ptr4));
}
