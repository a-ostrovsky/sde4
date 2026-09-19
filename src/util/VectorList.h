#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <deque>
#include <flat_map>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

namespace sde4::util {

template <typename T> struct VectorListCompactResult;

// VectorList stores elements in chunks and supports soft deletion via
// T::m_isDeleted. Soft-deleted elements are skipped when iterating.
template <typename T> class VectorList {
  static constexpr std::size_t ChunkSize{1024 * 64};

  static_assert(requires(T t) {
    { t.m_isDeleted } -> std::convertible_to<bool>;
    t.m_isDeleted = true;
  });

  struct Chunk {
    std::vector<T> m_data{};
  };

  std::deque<Chunk> m_chunks{};
  std::size_t m_itemCount{};

  template <bool IsConst> class IteratorImpl {
    friend class VectorList;

    using ChunksPtr = std::conditional_t<IsConst, const std::deque<Chunk>*,
                                         std::deque<Chunk>*>;
    ChunksPtr m_chunks{};
    std::size_t m_chunkIndex{};
    std::size_t m_elementIndex{};

    constexpr void advanceToNextAlive() {
      while (m_chunkIndex < m_chunks->size()) {
        auto const& chunk = (*m_chunks)[m_chunkIndex];
        while (m_elementIndex < chunk.m_data.size()) {
          if (!chunk.m_data[m_elementIndex].m_isDeleted)
            return;
          ++m_elementIndex;
        }
        ++m_chunkIndex;
        m_elementIndex = 0;
      }
    }

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = std::conditional_t<IsConst, const T*, T*>;
    using reference = std::conditional_t<IsConst, const T&, T&>;
    using iterator_category = std::forward_iterator_tag;

    constexpr IteratorImpl() = default;

    reference operator*() const {
      return (*m_chunks)[m_chunkIndex].m_data[m_elementIndex];
    }
    pointer operator->() const { return &**this; }

    constexpr IteratorImpl& operator++() {
      ++m_elementIndex;
      advanceToNextAlive();
      return *this;
    }
    constexpr IteratorImpl operator++(int) {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    friend bool operator==(const IteratorImpl& a, const IteratorImpl& b) {
      return a.m_chunks == b.m_chunks && a.m_chunkIndex == b.m_chunkIndex &&
             a.m_elementIndex == b.m_elementIndex;
    }
    friend bool operator!=(const IteratorImpl& a, const IteratorImpl& b) {
      return !(a == b);
    }
  };

public:
  using iterator = IteratorImpl<false>;
  using const_iterator = IteratorImpl<true>;

  iterator begin() {
    auto it = iterator{};
    it.m_chunks = &m_chunks;
    it.advanceToNextAlive();
    return it;
  }
  iterator end() {
    auto it = iterator{};
    it.m_chunks = &m_chunks;
    it.m_chunkIndex = m_chunks.size();
    return it;
  }

  const_iterator begin() const {
    auto it = const_iterator{};
    it.m_chunks = &m_chunks;
    it.advanceToNextAlive();
    return it;
  }
  const_iterator end() const {
    auto it = const_iterator{};
    it.m_chunks = &m_chunks;
    it.m_chunkIndex = m_chunks.size();
    return it;
  }

  constexpr void push_back(const T& value) { emplace_back(value); }

  constexpr void push_back(T&& value) { emplace_back(std::move(value)); }

  template <typename... Args> constexpr T& emplace_back(Args&&... args) {
    ensureChunk();
    T& ref = m_chunks.back().m_data.emplace_back(std::forward<Args>(args)...);
    ++m_itemCount;
    return ref;
  }

  constexpr std::size_t size() const { return m_itemCount; }

  constexpr VectorListCompactResult<T> compact() const {
    VectorList compacted;
    std::flat_map<const T*, T*> oldToNew;

    for (const auto& chunk : m_chunks) {
      for (std::size_t i = 0; i < chunk.m_data.size(); ++i) {
        if (!chunk.m_data[i].m_isDeleted) {
          const T& oldItem = chunk.m_data[i];
          T& newItem = compacted.emplace_back(oldItem);
          oldToNew[&oldItem] = &newItem;
        }
      }
    }

    return {std::move(compacted), std::move(oldToNew)};
  }

private:
  constexpr void ensureChunk() {
    if (!m_chunks.empty() &&
        m_chunks.back().m_data.size() < m_chunks.back().m_data.capacity()) {
      return;
    }

    auto& chunk = m_chunks.emplace_back();
    chunk.m_data.reserve(ChunkSize);
  }
};

template <typename T> struct VectorListCompactResult {
  VectorList<T> m_compacted{};
  std::flat_map<const T*, T*> m_oldToNew{};
};

} // namespace sde4::util
