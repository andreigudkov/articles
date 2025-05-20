#pragma once

#include <memory>
#include <array>

template<int kChunkCapacity>
class ChunkTwine {
private:
  struct Chunk {
    Chunk() :
        next(nullptr),
        prev(nullptr),
        num_items(0) {}

    Chunk* next;
    Chunk* prev;
    std::array<int, kChunkCapacity> items;
    int num_items;
  };

  struct ItemData {
    ItemData() :
        chunk(nullptr),
        chpos(-1),
        subset(-1) {}

    Chunk* chunk;
    int chpos; // position inside Chunk.items
    int subset;
  };

  struct SubsetData {
    SubsetData() : back(nullptr) {}

    Chunk* back;
  };


public:
  class SubsetView {
  public:
    SubsetView(Chunk* chunk) : chunk_(chunk) {}

    class Iterator {
    public:
      using iterator_category = std::forward_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = int;
      using pointer           = int*;
      using reference         = int;

      Iterator(Chunk* chunk, int chpos) :
          chunk_(chunk), chpos_(chpos)
      {
        if (chunk_) {
          __builtin_prefetch(chunk_->next, 0/*read*/, 1);
        }
      }

      reference operator*() const {
        return chunk_->items[chpos_];
      }

      Iterator& operator++() {
        chpos_++;
        if (chpos_ >= chunk_->num_items) {
          chunk_ = chunk_->next;
          chpos_ = 0;
          if (chunk_) {
            __builtin_prefetch(chunk_->next, 0/*read*/, 1);
          }
        }
        return *this;
      }

      Iterator operator++(int) {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      bool operator== (const Iterator& that) const {
        return (this->chunk_ == that.chunk_) && (this->chpos_ == that.chpos_);
      }

      bool operator!= (const Iterator& that) const {
        return (this->chunk_ != that.chunk_) || (this->chpos_ != that.chpos_);
      }

    private:
      Chunk* chunk_;
      int chpos_;
    };

    Iterator begin() const { return Iterator(chunk_, 0); }
    Iterator end() const { return Iterator(nullptr, 0); }

  private:
    Chunk* chunk_;
  };

  SubsetView ViewOf(int subset) const {
    return SubsetView(subset_data_[subset].back);
  }

  ChunkTwine(int num_items, int num_subsets) :
    item_data_(new ItemData[num_items]),
    subset_data_(new SubsetData[num_subsets]),
    free_(nullptr)
  {
    // create chunks and add them to the pool of free chunks
    int num_chunks = num_subsets + (num_items-num_subsets)/kChunkCapacity;
    chunk_pool_.reset(new Chunk[num_chunks]);
    for (int i = 0; i < num_chunks; i++) {
      PushChunk(&free_, &chunk_pool_[i]);
    }
  }

  int SubsetOf(int item) const {
    return item_data_[item].subset;
  }

  void Assign(int item, int subset) {
    if (item_data_[item].subset == subset) {
      return;
    }

    if (item_data_[item].subset != -1) {
      ItemData& id = item_data_[item];
      Chunk* back_chunk = subset_data_[id.subset].back;
      int back_item = back_chunk->items[--back_chunk->num_items];
      if (back_chunk->num_items == 0) {
        PopChunk(&subset_data_[id.subset].back);
        PushChunk(&free_, back_chunk);
      }

      if (item != back_item) {
        // move back item to the position previously occupied by item
        id.chunk->items[id.chpos] = back_item;
        item_data_[back_item].chunk = id.chunk;
        item_data_[back_item].chpos = id.chpos;
      }

      item_data_[item].subset = -1;
      item_data_[item].chunk = nullptr;
      item_data_[item].chpos = -1;
    }

    if (subset != -1) {
      Chunk* chunk = subset_data_[subset].back;
      if (!chunk || (chunk->num_items == kChunkCapacity)) {
        chunk = PopChunk(&free_);
        chunk->num_items = 0;
        PushChunk(&subset_data_[subset].back, chunk);
      }
      int chpos = chunk->num_items++;
      chunk->items[chpos] = item;
      item_data_[item].subset = subset;
      item_data_[item].chunk = chunk;
      item_data_[item].chpos = chpos;
    }
  }

private:
  Chunk* PopChunk(Chunk** back) {
    Chunk* chunk = *back;
    *back = chunk->next;
    if (*back) {
      (*back)->prev = nullptr;
    }
    chunk->next = nullptr;
    return chunk;
  }

  void PushChunk(Chunk** back, Chunk* chunk) {
    chunk->next = *back;
    chunk->prev = nullptr;
    if (*back) {
      (*back)->prev = chunk;
    }
    *back = chunk;
  }

  std::unique_ptr<ItemData[]> item_data_;
  std::unique_ptr<SubsetData[]> subset_data_;
  std::unique_ptr<Chunk[]> chunk_pool_;
  Chunk* free_;
};

