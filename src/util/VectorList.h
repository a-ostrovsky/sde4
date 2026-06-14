#pragma once

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <deque>
#include <flat_map>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace sde4::util {

template <typename T> struct VectorListCompactResult;

template <typename T> class VectorList {
  static constexpr std::size_t ChunkSize{1024ull * 64};
  static constexpr std::size_t BitsPerWord{64};

  struct Chunk {
    std::vector<T> m_data{};
    std::vector<std::bitset<BitsPerWord>> m_deletedBits{};
    std::size_t m_aliveCount{0};
  };

  struct Position {
    std::size_t chunkIndex;
    std::size_t elementIndex;
  };

  enum class SoftDeleteOperation { Delete, Undelete };

  std::deque<Chunk> m_chunks{};
  std::size_t m_itemCount{};

  template <bool IsConst> class IteratorImpl {
    friend class VectorList;

    using ChunksPtr = std::conditional_t<IsConst, const std::deque<Chunk>*,
                                         std::deque<Chunk>*>;
    ChunksPtr m_chunks{};
    std::size_t m_chunkIndex{};
    std::size_t m_elementIndex{};

    constexpr void skipAliveForward() {
      while (m_chunkIndex < m_chunks->size()) {
        auto const& chunk = (*m_chunks)[m_chunkIndex];
        while (m_elementIndex < chunk.m_data.size()) {
          auto word = m_elementIndex / BitsPerWord;
          auto bit = m_elementIndex % BitsPerWord;
          if (!chunk.m_deletedBits[word].test(bit))
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
      skipAliveForward();
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
    it.skipAliveForward();
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
    it.skipAliveForward();
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
    ++m_chunks.back().m_aliveCount;
    ++m_itemCount;
    return ref;
  }

  constexpr void softDelete(const T* ptr) {
    const auto position = findChunkAndIndex(ptr);
    softDeleteOrUndelete(position, SoftDeleteOperation::Delete);
  }

  constexpr void softUndelete(const T* ptr) {
    const auto position = findChunkAndIndex(ptr);
    softDeleteOrUndelete(position, SoftDeleteOperation::Undelete);
  }

  constexpr std::size_t size() const { return m_itemCount; }

  constexpr VectorListCompactResult<T> compact() const {
    VectorList<T> compacted;
    std::flat_map<const T*, T*> oldToNew;

    for (const auto& chunk : m_chunks) {
      for (std::size_t i = 0; i < chunk.m_data.size(); ++i) {
        const std::size_t word = i / BitsPerWord;
        const std::size_t bit = i % BitsPerWord;
        if (!chunk.m_deletedBits[word].test(bit)) {
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
    const std::size_t wordsNeeded{(ChunkSize + BitsPerWord - 1) / BitsPerWord};
    m_chunks.back().m_deletedBits.resize(wordsNeeded, {});
  }

  constexpr Position findChunkAndIndex(const T* ptr) const {
    for (std::size_t i = 0; i < m_chunks.size(); ++i) {
      const auto& chunk = m_chunks[i];
      const T* begin = chunk.m_data.data();
      const T* end = begin + chunk.m_data.size();
      if (begin <= ptr && ptr < end) {
        return {i, static_cast<std::size_t>(ptr - begin)};
      }
    }
    throw std::invalid_argument("pointer not found in VectorList");
  }

  constexpr void softDeleteOrUndelete(Position position,
                                      SoftDeleteOperation op) {
    auto& chunk = m_chunks[position.chunkIndex];
    const std::size_t word = position.elementIndex / BitsPerWord;
    const std::size_t bit = position.elementIndex % BitsPerWord;
    if (op == SoftDeleteOperation::Delete &&
        !chunk.m_deletedBits[word].test(bit)) {
      chunk.m_deletedBits[word].set(bit);
      --chunk.m_aliveCount;
      --m_itemCount;
    } else if (op == SoftDeleteOperation::Undelete &&
               chunk.m_deletedBits[word].test(bit)) {
      chunk.m_deletedBits[word].reset(bit);
      ++chunk.m_aliveCount;
      ++m_itemCount;
    }
  }
};

template <typename T> struct VectorListCompactResult {
  VectorList<T> m_compacted{};
  std::flat_map<const T*, T*> m_oldToNew{};
};

} // namespace sde4::util
