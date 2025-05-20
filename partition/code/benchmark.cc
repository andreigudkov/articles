#include <vector>
#include <random>
#include <chrono>
#include <cstdio>

#include "polyset.h"
#include "item_twine.h"
#include "chunk_twine.h"
#include "carousel.h"

template<typename T>
class Ops {
public:
  void Run() {
    T partition(kNumItems, kNumSubsets);
    Assign(partition, SampleAssignCalls(kNumItems*10));
    Iterate(partition, SampleSubsets(kNumSubsets*100));
    Verify(partition);
  }

private:
  struct Call {
    int item;
    int subset;
  };

  std::vector<Call> SampleAssignCalls(int count) {
    std::default_random_engine rng(24741);
    std::uniform_int_distribution<int> item_dice(0, kNumItems-1);
    std::uniform_int_distribution<int> subset_dice(0, kNumSubsets-1);
    std::vector<Call> calls;
    calls.reserve(count);
    for (int i = 0; i < count; i++) {
      Call call;
      call.item = item_dice(rng);
      call.subset = subset_dice(rng);
      calls.push_back(call);
    }
    return calls;
  }

  std::vector<int> SampleSubsets(int count) {
    std::vector<int> subsets;
    std::uniform_int_distribution<int> subset_dice(0, kNumSubsets-1);
    std::default_random_engine rng(80956);
    for (int i = 0; i < count; i++) {
      subsets.push_back(subset_dice(rng));
    }
    return subsets;
  }

  void __attribute__((noinline)) Assign(T& partition, const std::vector<Call>& calls) {
    auto ts1 = std::chrono::high_resolution_clock::now();
    for (const Call& call : calls) {
      partition.Assign(call.item, call.subset);
    }
    auto ts2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = ts2 - ts1;

    std::printf("Assign(): %.3f sec | %.0f items/sec\n",
                elapsed.count(),
                std::floor(calls.size()/elapsed.count()));
  }

  void __attribute__((noinline)) Iterate(const T& partition, const std::vector<int>& subsets) {
    int64_t num_items = 0;
    auto ts1 = std::chrono::high_resolution_clock::now();
    for (int subset : subsets) {
      auto&& view = partition.ViewOf(subset);
      num_items += std::distance(view.begin(), view.end());
    }
    auto ts2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = ts2 - ts1;
    std::printf("Iterate(): %.3f sec | %.0f items/sec\n",
                elapsed.count(),
                std::floor(num_items/elapsed.count()));
  }

  void __attribute__((noinline)) Verify(const T& partition) {
    uint32_t checksum = 1;
    for (int subset = 0; subset < kNumSubsets; subset++) {
      checksum = checksum * 13;
      for (int item : partition.ViewOf(subset)) {
        checksum = checksum + (uint32_t)item; // order of items is unimportant
      }
    }
    std::printf("Verify(): checksum=%u\n", checksum);
  }

private:
  static constexpr int kNumItems = 1000000;
  static constexpr int kNumSubsets = 1000;
};


////////////////////////////////////////////////////////////////////////////////

/*
 * Given number of "widgets" each with its own height, group them into fixed
 * number of "columns" such that height of the highest column is minimized.
 * Brute-force enumeration.
 */
template<typename T>
class LayoutSolver {
public:
  void Run() {
    std::vector<int> widget_heights = SampleWidgetHeights();

    auto ts1 = std::chrono::high_resolution_clock::now();
    std::unique_ptr<T> layout = Solve(widget_heights);
    auto ts2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = ts2 - ts1;
    std::printf("Total time: %.3f sec\n", elapsed.count());

    Print(*layout, widget_heights);
  }

private:
  std::vector<int> SampleWidgetHeights() {
    std::vector<int> widget_heights(num_widgets_, 0);
    std::default_random_engine rng(321);
    std::uniform_int_distribution<int> dice(1, 1000);
    for (int w = 0; w < num_widgets_; w++) {
      widget_heights[w] = dice(rng);
    }
    return widget_heights;
  }

  void Print(const T& layout, const std::vector<int>& widget_heights) {
    int height = Evaluate(layout, widget_heights);
    std::printf("Height: %d\n", height);
    for (int c = 0; c < num_columns_; c++) {
      std::printf("Col[%d] = [", c);
      bool first = true;
      for (int w : layout.ViewOf(c)) {
        if (first) {
          first = false;
        } else {
          std::printf("+");
        }
        std::printf("%d", widget_heights[w]);
      }
      std::printf("]\n");
    }
  }

  int Evaluate(const T& layout, const std::vector<int>& widget_heights) {
    int highest_height = 0;
    for (int c = 0; c < num_columns_; c++) {
      int height = 0;
      for (int w : layout.ViewOf(c)) {
        height += widget_heights[w];
      }
      highest_height = std::max(highest_height, height);
    }
    return highest_height;
  }

  std::unique_ptr<T> __attribute__((noinline)) Solve(const std::vector<int>& widget_heights) {
    std::unique_ptr<T> best_layout(new T(num_widgets_, num_columns_));
    int best_height = std::numeric_limits<int>::max();

    T layout(num_widgets_, num_columns_);
    Init(layout);
    do {
      int height = Evaluate(layout, widget_heights);
      if (height < best_height) {
        best_height = height;
        for (int w = 0; w < num_widgets_; w++) {
          best_layout->Assign(w, layout.SubsetOf(w));
        }
      }
    } while (Next(layout));
    return best_layout;
  }

  void Init(T& layout) {
    for (int w = 0; w < num_widgets_; w++) {
      layout.Assign(w, 0);
    }
  }

  bool Next(T& layout) {
    for (int w = 0; w < num_widgets_; w++) {
      int c = layout.SubsetOf(w);
      if (c + 1 < num_columns_) {
        layout.Assign(w, c + 1);
        return true;
      } else {
        layout.Assign(w, 0);
      }
    }
    return false;
  };

private:
  constexpr static int num_widgets_ = 16;
  constexpr static int num_columns_ = 3;
};


////////////////////////////////////////////////////////////////////////////////

/* Naive implementation of good old K-means clustering */
template<typename T>
class KMeans {
public:
  void Run() {
    constexpr int num_points = 1000000;
    constexpr int num_clusters = 5;
    constexpr int iters = 500;

    std::vector<double> points = SamplePoints(num_points);

    auto ts1 = std::chrono::high_resolution_clock::now();
    std::vector<double> centers = Clusterize(points, num_clusters, iters);
    auto ts2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = ts2 - ts1;

    std::printf("Total time: %.3f sec\n", elapsed.count());
    std::printf("Centers:");
    std::sort(centers.begin(), centers.end());
    for (double center : centers) {
      std::printf(" %.6f", center);
    }
    std::printf("\n");
  }

private:
  std::vector<double> SamplePoints(int num_points) {
    std::default_random_engine rng(7892);
    std::uniform_real_distribution<double> dice(0.0, 1.0);
    std::vector<double> points;
    points.reserve(num_points);
    for (int i = 0; i < num_points; i++) {
      points.push_back(dice(rng));
    }
    return points;
  }

  void RedistributePoints(const std::vector<double>& points,
                          const std::vector<double>& centers,
                          T& clusters)
  {
    for (size_t p = 0; p < points.size(); p++) {
      int best_c = 0;
      double best_dist = std::fabs(points[p] - centers[best_c]);
      for (size_t c = 1; c < centers.size(); c++) {
        double dist = std::fabs(points[p] - centers[c]);
        if (dist < best_dist) {
          best_c = c;
          best_dist = dist;
        }
      }
      clusters.Assign(p, best_c);
    }
  }


  void RecomputeCenters(const std::vector<double>& points,
                        const T& clusters,
                        std::vector<double>& centers)
  {
    for (size_t c = 0; c < centers.size(); c++) {
      double mean = 0.0;
      int count = 0;
      for (int p : clusters.ViewOf(c)) {
        mean += points[p];
        count += 1;
      }
      centers[c] = mean/count;
    }
  }

  std::vector<double> __attribute__((noinline)) Clusterize(
      const std::vector<double>& points,
      int num_clusters, int iters)
  {
    T clusters(points.size(), num_clusters);
    std::vector<double> centers(points.begin(), points.begin() + num_clusters);
    for (int iter = 0; iter < iters; iter++) {
      RedistributePoints(points, centers, clusters);
      RecomputeCenters(points, clusters, centers);
    }
    return centers;
  }
};

////////////////////////////////////////////////////////////////////////////////

/*
 * Find mapping of "data shards" to "servers" which equalizes per-server usage
 * as much as possible. Uses simple local random search.
 */
template<typename T>
class Balancer {
public:
  void Run() {
    constexpr int num_servers = 100;
    constexpr int avg_shards_per_server = 100;
    std::vector<int> shard_sizes = SampleShardSizes(num_servers * avg_shards_per_server);

    ShardMap shard_map(shard_sizes, num_servers);
    for (int i = 0; i < (int)shard_sizes.size(); i++) {
      shard_map.Assign(i, i%num_servers);
    }

    auto ts1 = std::chrono::high_resolution_clock::now();
    Balance(shard_map);
    auto ts2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = ts2 - ts1;

    std::printf("Total time: %.3f sec\n", elapsed.count());
    Print(shard_map);
  }

private:
  // mapping of shards to servers with tracking of per-server usage
  class ShardMap {
  public:
    ShardMap(const std::vector<int>& shard_sizes, int num_servers) :
      partition_(shard_sizes.size(), num_servers),
      shard_sizes_(shard_sizes),
      server_usages_(num_servers, 0)
    {
    }

    void Assign(int shard, int server) {
      int prev_server = partition_.SubsetOf(shard);
      if (prev_server != -1) {
        server_usages_[prev_server] -= shard_sizes_[shard];
      }
      server_usages_[server] += shard_sizes_[shard];
      partition_.Assign(shard, server);
    }

    int UsageOf(int server) const { return server_usages_[server]; }

    const typename T::SubsetView ViewOf(int server) const { return partition_.ViewOf(server); }

    int num_servers() const { return (int)server_usages_.size(); }

  private:
    T partition_;
    const std::vector<int>& shard_sizes_;
    std::vector<int> server_usages_;
  };

  std::vector<int> SampleShardSizes(int num_shards) const {
    std::vector<int> shard_sizes;
    shard_sizes.reserve(num_shards);
    std::default_random_engine rng(77654);
    std::uniform_int_distribution<int> dice(1, 5000);
    for (int shard = 0; shard < num_shards; shard++) {
      shard_sizes.push_back(dice(rng));
    }
    return shard_sizes;
  }

  void Print(const ShardMap& shard_map) {
    int lo = 0;
    int hi = 0;
    for (int server = 0; server < shard_map.num_servers(); server++) {
      if (shard_map.UsageOf(server) < shard_map.UsageOf(lo)) {
        lo = server;
      }
      if (shard_map.UsageOf(server) > shard_map.UsageOf(hi)) {
        hi = server;
      }
    }
    std::printf("Lowest usage: %d (at %d)\n", shard_map.UsageOf(lo), lo);
    std::printf("Highest usage: %d (at %d)\n", shard_map.UsageOf(hi), hi);
  }

  void __attribute__((noinline)) Balance(ShardMap& shard_map) {
    std::default_random_engine rng(49136);
    std::uniform_int_distribution<int> dice(0, shard_map.num_servers()-1);
    for (int iter = 0; iter < 1000000; iter++) {
      int src = dice(rng);
      int dst = dice(rng);
      if (shard_map.UsageOf(src) > shard_map.UsageOf(dst)) {
        while (TryBalance(shard_map, src, dst));
      }
    }
  }

  bool TryBalance(ShardMap& shard_map, int src, int dst) {
    int best_diff = shard_map.UsageOf(src) - shard_map.UsageOf(dst);
    int best_shard = -1;

    std::vector<int> shards(shard_map.ViewOf(src).begin(), shard_map.ViewOf(src).end());
    for (int shard : shards) {
      shard_map.Assign(shard, dst);
      int diff = shard_map.UsageOf(src) - shard_map.UsageOf(dst);
      if ((diff >= 0) && (diff < best_diff)) {
        best_diff = diff;
        best_shard = shard;
      }
      shard_map.Assign(shard, src);
    }

    if (best_shard != -1) {
      shard_map.Assign(best_shard, dst);
      return true;
    }

    return false;
  }
};
 

////////////////////////////////////////////////////////////////////////////////

class Unit {
public:
  Unit(const std::string& bench_name,
       const std::string& impl_name,
       std::function<void()> runner) :
    bench_name_(bench_name),
    impl_name_(impl_name),
    runner_(runner)
  {
  }

  const std::string& bench_name() const { return bench_name_; }
  const std::string& impl_name() const { return impl_name_; }
  std::function<void()> runner() const { return runner_; }

private:
  std::string bench_name_;
  std::string impl_name_;
  std::function<void()> runner_;
};

template<typename T>
void Register(const std::string& impl_name, std::vector<Unit>& units) {
  units.emplace_back(
    "ops",
    impl_name,
    [](){Ops<T>().Run();}
  );
  units.emplace_back(
    "layout",
    impl_name,
    [](){LayoutSolver<T>().Run();}
  );
  units.emplace_back(
    "kmeans",
    impl_name,
    [](){KMeans<T>().Run();}
  );
  units.emplace_back(
    "balancer",
    impl_name,
    [](){Balancer<T>().Run();}
  );
}

int main(int argc, char* argv[]) {
  ::setlinebuf(stdout);

  if (argc > 3) {
    std::printf("Usage: %s [<bench> [<impl>]]\n", argv[0]);
    return 1;
  }
  std::string bench_name;
  std::string impl_name;
  if (argc > 1) {
    bench_name = std::string(argv[1]);
    if (argc > 2) {
      impl_name = std::string(argv[2]);
    }
  }

  std::vector<Unit> units;
  Register<PolyRbSet>("PolyRbSet", units);
  Register<PolyHashSet>("PolyHashSet", units);
  Register<PolyHopscotchSet>("PolyHopscotchSet", units);
  Register<ItemTwine>("ItemTwine", units);
  Register<ChunkTwine<123>>("ChunkTwine", units);
  Register<Carousel>("Carousel", units);

  if (!bench_name.empty()) {
    units.erase(
      std::remove_if(
        units.begin(),
        units.end(),
        [&](const Unit& unit){return unit.bench_name() != bench_name;}),
      units.end()
    );
  }
  if (!impl_name.empty()) {
    units.erase(
      std::remove_if(
        units.begin(),
        units.end(),
        [&](const Unit& unit){return unit.impl_name() != impl_name;}),
      units.end()
    );
  }

  for (const Unit& unit : units) {
    std::printf("[%s] %s\n", unit.impl_name().c_str(), unit.bench_name().c_str());
    unit.runner()();
    std::printf("\n");
  }

  return 0;
}


