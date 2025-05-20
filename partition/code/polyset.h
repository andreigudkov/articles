#pragma once

#include <set>
#include <unordered_set>
#include <tsl/hopscotch_set.h>

template<typename SetType>
class PolySet {
public:
  PolySet(int num_items, int num_subsets) :
      item_data_(new ItemData[num_items]),
      subset_data_(new SubsetData[num_subsets]) {}

  void Assign(int item, int subset) {
    int curr_subset = item_data_[item].subset;
    if (curr_subset != subset) {
      if (curr_subset != -1) {
        subset_data_[curr_subset].items.erase(item);
      }
      if (subset != -1) {
        subset_data_[subset].items.insert(item);
      }
      item_data_[item].subset = subset;
    }
  }

  typedef const SetType& SubsetView;
  const SubsetView ViewOf(int subset) const {
    return subset_data_[subset].items;
  }

  int SubsetOf(int item) const {
    return item_data_[item].subset;
  }

private:
  struct ItemData {
    ItemData() : subset(-1) {}
    int subset;
  };

  struct SubsetData {
    SetType items;
  };

  std::unique_ptr<ItemData[]> item_data_;
  std::unique_ptr<SubsetData[]> subset_data_;
};

typedef PolySet<std::set<int>>           PolyRbSet;
typedef PolySet<std::unordered_set<int>> PolyHashSet;
typedef PolySet<tsl::hopscotch_set<int>> PolyHopscotchSet;

