#pragma once

#include <memory>

class Carousel {
private:
  struct ItemData {
    int subset;
    int pos; // into ring_
  };

  struct SubsetData {
    int begin; // into ring_
    int size;
  };

  // fixed size array
  template<typename T>
  class Buffer {
  public:
    Buffer(int size) : data_(new T[size]), size_(size) {}
    const T& operator[](int index) const { return data_.get()[index]; }
    T& operator[](int index) { return data_.get()[index]; }
    int size() const { return size_; }
  private:
    std::unique_ptr<T[]> data_;
    int size_;
  };

public:
  struct SubsetView {
    class Iterator {
    public:
      using iterator_category = std::forward_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = int;
      using pointer           = int*;
      using reference         = int;

      Iterator(const Buffer<int>& ring, int pos) :
          ring_(ring), pos_(pos) {}

      reference operator*() const {
        return ring_[pos_];
      }

      Iterator& operator++() {
        pos_++;
        if (pos_ == ring_.size()) {
          pos_ = 0;
        }
        return *this;
      }

      Iterator operator++(int) {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      friend bool operator== (const Iterator& a, const Iterator& b) {
        return a.pos_ == b.pos_;
      }
      friend bool operator!= (const Iterator& a, const Iterator& b) {
        return a.pos_ != b.pos_;
      }

    private:
      const Buffer<int>& ring_;
      int pos_;
    };

    Iterator begin() const {
      return Iterator(ring_, sd_.begin);
    }

    Iterator end() const {
      int pos = sd_.begin + sd_.size;
      if (pos >= ring_.size()) {
        pos -= ring_.size();
      }
      return Iterator(ring_, pos);
    }

    SubsetView(const Buffer<int>& ring, const SubsetData& sd) :
        ring_(ring), sd_(sd) {}

  private:
    const Buffer<int>& ring_;
    const SubsetData& sd_;
  };

  SubsetView ViewOf(int subset) const {
    return SubsetView(ring_, subset_data_[subset]);
  }

  int SubsetOf(int item) const {
    return item_data_[item].subset;
  }

  Carousel(int num_items, int num_subsets) :
      item_data_(num_items),
      subset_data_(num_subsets),
      ring_(ComputeRingCapacity(num_items, num_subsets))
  {
    for (int i = 0; i < num_items; i++) {
      item_data_[i].subset = -1;
      item_data_[i].pos = -1;
    }

    int shift = ring_.size() / num_subsets;
    for (int s = 0; s < num_subsets; s++) {
      subset_data_[s].begin = shift * s;
      subset_data_[s].size = 0;
    }

    for (int pos = 0; pos < ring_.size(); pos++) {
      ring_[pos] = -1;
    }
  }

  void Assign(int item, int subset) {
    ItemData& id = item_data_[item];
    if (id.subset == subset) {
      return;
    }
    if (id.subset != -1) {
      Exclude(item, id.subset, id.pos);
    }
    if (subset != -1) {
      Include(item, subset);
    }
  }

private:
  void Exclude(int item, int subset, int pos) {
    int rpos = subset_data_[subset].begin + subset_data_[subset].size - 1;
    if (rpos >= ring_.size()) {
      rpos -= ring_.size();
    }
    if (rpos != pos) {
      ring_[pos] = ring_[rpos];
      ring_[rpos] = -1;
      item_data_[ring_[pos]].pos = pos;
    } else {
      ring_[pos] = -1;
    }
    subset_data_[subset].size--;
    item_data_[item].subset = -1;
    item_data_[item].pos = -1;
  }

  void Include(int item, int subset) {
    item_data_[item].subset = subset;

    if (subset_data_[subset].size == 0) {
      // rotate begin to point to an empty element
      int begin = subset_data_[subset].begin;
      while (ring_[begin] != -1) {
        begin++;
        if (begin >= ring_.size()) {
          begin -= ring_.size();
        }
      }
      subset_data_[subset].begin = begin;
    }

    while (true) {
      // find position for an item, and its current occupant
      int pos = subset_data_[subset].begin + subset_data_[subset].size;
      if (pos >= ring_.size()) {
        pos -= ring_.size();
      }
      int old_item = ring_[pos];

      // overwrite position with an item
      ring_[pos] = item;
      item_data_[item].pos = pos;
      subset_data_[subset].size++;
      if (old_item == -1) {
        break;
      }
      
      // prepare to relocate old_item
      int old_subset = item_data_[old_item].subset;
      subset_data_[old_subset].begin++;
      if (subset_data_[old_subset].begin >= ring_.size()) {
        subset_data_[old_subset].begin -= ring_.size();
      }
      subset_data_[old_subset].size--;
      item = old_item;
      subset = old_subset;
    }
  }

  static int ComputeRingCapacity(int num_items, int num_subsets) {
    // round up a to be multiple of b
    auto RoundUp = [](int a, int b) {
      return (a + (b - 1)) / b * b;
    };
    constexpr double kLoadFactor = 0.5;
    return RoundUp(int(std::ceil(num_items / kLoadFactor)), num_subsets);
  }

  Buffer<ItemData> item_data_;
  Buffer<SubsetData> subset_data_;
  Buffer<int> ring_;
};

