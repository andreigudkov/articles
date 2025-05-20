#pragma once

#include <memory>

class ItemTwine {
private:
  struct ItemData {
    ItemData() :
        subset(-1),
        prev_item(-1),
        next_item(-1) {}

    int subset;
    int prev_item;
    int next_item;
  };

  struct SubsetData {
    SubsetData() :
        back_item(-1) {}
    int back_item;
  };

public:
  class SubsetView {
  public:
    SubsetView(ItemData* item_data, int back_item) :
        item_data_(item_data),
        back_item_(back_item) {}

    class Iterator {
    public:
      using difference_type   = std::ptrdiff_t;
      using value_type        = int;
      using pointer           = void;
      using reference         = const int&;
      using iterator_category = std::forward_iterator_tag;

      Iterator(ItemData* item_data, int curr_item) :
          item_data_(item_data), curr_item_(curr_item) {}

      reference operator*() const { return curr_item_; }

      bool operator==(const Iterator& that) {
        return this->curr_item_ == that.curr_item_;
      }

      bool operator!=(const Iterator& that) {
        return this->curr_item_ != that.curr_item_;
      }

      Iterator& operator++() {
        curr_item_ = item_data_[curr_item_].next_item;
        return *this;
      }

      Iterator operator++(int) {
        Iterator res = *this;
        ++(*this);
        return res;
      }

    private:
      ItemData* item_data_;
      int curr_item_;
    };
  
    Iterator begin() const { return Iterator(item_data_, back_item_); }
    Iterator end() const { return Iterator(item_data_, -1); }

  private:
    ItemData* item_data_;
    int back_item_;
  };

  ItemTwine(int num_items, int num_subsets) :
      item_data_(new ItemData[num_items]),
      subset_data_(new SubsetData[num_subsets]) {}

  void Assign(int item, int subset) {
    if (item_data_[item].subset == subset) {
      return;
    }
    if (item_data_[item].subset != -1) {
      RemoveItem(item);
    }
    if (subset != -1) {
      PushItem(item, subset);
    }
  }

  SubsetView ViewOf(int subset) const {
    return SubsetView(item_data_.get(), subset_data_[subset].back_item);
  }

  int SubsetOf(int item) const {
    return item_data_[item].subset;
  }

private:
  // add item to the back of given subset
  void PushItem(int item, int subset) {
    item_data_[item].prev_item = -1;
    item_data_[item].subset = subset;
    int back_item = subset_data_[subset].back_item;
    if (back_item != -1) {
      item_data_[item].next_item = back_item;
      item_data_[back_item].prev_item = item;
    } else {
      item_data_[item].next_item = -1;
    }
    subset_data_[subset].back_item = item;
  }

  // remove item from the subset it is currently assigned to
  void RemoveItem(int item) {
    int subset = item_data_[item].subset;
    int prev_item = item_data_[item].prev_item;
    int next_item = item_data_[item].next_item;
    if (prev_item != -1) {
      item_data_[prev_item].next_item = next_item;
    } else {
      subset_data_[subset].back_item = next_item;
    }
    if (next_item != -1) {
      item_data_[next_item].prev_item = prev_item;
    }
    item_data_[item].subset = -1;
  }

  std::unique_ptr<ItemData[]> item_data_;
  std::unique_ptr<SubsetData[]> subset_data_;
};

