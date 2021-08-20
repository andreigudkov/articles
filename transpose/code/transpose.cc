#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif

#include <immintrin.h>

#include <functional>
#include <chrono>
#include <random>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <memory>
#include <cstring>


/************ Utility *******************/

#define ensure(v) {if (!(v)) {throw std::runtime_error(#v);}}

inline uint64_t rdtscp() {
  uint32_t low, high, aux;
  asm volatile(".byte 0x0f,0x01,0xf9"
    : "=a" (low), "=d" (high), "=c" (aux));
  return low | (uint64_t(high) << 32);
}

// Marsaglia, G. (2003). Xorshift RNGs. Journal of Statistical Software, 8(14), 1–6.
// https://doi.org/10.18637/jss.v008.i14
inline uint64_t xorshift64(){
  static uint64_t x = 84531563975341225ULL;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  return x;
}


/************ Mat *******************/

class Mat {
public:
  Mat(int64_t n) :
    _n(n),
    _data(static_cast<uint8_t*>(std::aligned_alloc(64, n*n)))
  {
    ensure(_data);
  }

  ~Mat() {
    std::free(_data);
  }

  int64_t n() const { return _n; }

  const uint8_t* data() const { return _data; }
  uint8_t* data() { return _data; }

  uint8_t at(int64_t row, int64_t col) const { return _data[row*_n+col]; }
  uint8_t& at(int64_t row, int64_t col) { return _data[row*_n+col]; }

  Mat(const Mat&) = delete;
  Mat& operator=(const Mat&) = delete;

private:
  int64_t _n;
  uint8_t* _data;
};



void fill_random(Mat* mat) {
  int64_t off = 0;
  while (off+8 < mat->n()*mat->n()) {
    *(uint64_t*)(mat->data() + off) = xorshift64();
    *(uint64_t*)(mat->data() + off) = xorshift64();
    off += 4;
  }
  while (off < mat->n()*mat->n()) {
    mat->data()[off] = xorshift64();
    mat->data()[off] = xorshift64();
    off += 1;
  }
}
 
std::ostream& operator<<(std::ostream& out, const Mat& mat) {
  std::ostream tmp(out.rdbuf());
  tmp << std::hex;
  tmp << std::uppercase;

  for (int i = 0; i < mat.n(); i++) {
    if (i > 0) {
      tmp << std::endl;
    }
    for (int j = 0; j < mat.n(); j++) {
      if (j > 0) {
        tmp << ' ';
      }
      tmp << std::setw(2) << std::setfill('0') << (int)mat.at(i, j);
    }
  }
  
  return out;
}

void check_transpose(const Mat& mat1, const Mat& mat2) {
  ensure(mat1.n() == mat2.n());
  for (int64_t i = 0; i < mat1.n(); i++) {
    for (int64_t j = 0; j < mat1.n(); j++) {
      ensure(mat1.at(i, j) == mat2.at(j, i));
    }
  }
}


/******************* Naive *************************/

void transpose_Naive(const Mat& src, Mat* dst) {
  const int64_t n = src.n();
  for (int64_t r = 0; r < n; r++) {
    for (int64_t c = 0; c < n; c++) {
      dst->data()[n * c + r] = src.data()[n * r + c];
    }
  }
}

void transpose_NaiveSmall(const Mat& src, Mat* dst) {
  const int64_t n = src.n();
  for (int64_t r = 0; r < n; r++) {
    const uint8_t* src_ptr = src.data()  + r * n; // beginning of r-th row
    const uint8_t* src_ptr_end = src_ptr + n;
          uint8_t* dst_ptr0 = dst->data() + r + n*0; // beginning of r-th column
          uint8_t* dst_ptr1 = dst->data() + r + n*1;
    while (src_ptr + 2 <= src_ptr_end) {
      *dst_ptr0 = *(src_ptr + 0);
      *dst_ptr1 = *(src_ptr + 1);

      dst_ptr0 += 2*n;
      dst_ptr1 += 2*n;

      src_ptr += 2;
    }
    while (src_ptr < src_ptr_end) {
      *dst_ptr0 = *src_ptr;
      src_ptr += 1;
      dst_ptr0 += n;
    }
  }
}


/******************* Reverse *************************/

void transpose_Reverse(const Mat& src, Mat* dst) {
  const int64_t n = src.n();
  for (int64_t r = 0; r < n; r++) {
    for (int64_t c = 0; c < n; c++) {
      dst->data()[n * r + c] = src.data()[n * c + r];
    }
  }
}


/*********************** Blocks ****************************/

void transpose_Blocks(const Mat& src, Mat* dst) {
  const int64_t n = src.n();
  const int64_t bsize = 64;
  for (int64_t rb = 0; rb < src.n()/bsize; rb++) {
    for (int64_t cb = 0; cb < src.n()/bsize; cb++) {
      const uint8_t* src_origin = src.data()  + (rb * n + cb) * bsize;
            uint8_t* dst_origin = dst->data() + (cb * n + rb) * bsize;
      for (int64_t r = 0; r < bsize; r++) {
        for (int64_t c = 0; c < bsize; c++) {
          dst_origin[r * n + c] = src_origin[c * n + r];
        }
      }
    }
  }
}

/*********************** BlocksPrf ****************************/

/* Compute origin of the 64-block next to (rb, cb) in row-major order */
inline const uint8_t* next_block(const Mat& src, int64_t rb, int64_t cb) {
  int64_t cb1 = cb + 1;
  int64_t rb1 = rb;
  if (cb1 == src.n()/64) {
    rb1 += 1;
    cb1 = 0;
  }
  return src.data() + (rb1*src.n() + cb1) * 64;
}

void transpose_BlocksPrf(const Mat& src, Mat* dst) {
  const int64_t n = src.n();
  const int64_t bsize = 64;
  for (int64_t rb = 0; rb < src.n()/bsize; rb++) {
    for (int64_t cb = 0; cb < src.n()/bsize; cb++) {
      const uint8_t* src_origin = src.data() + (rb*n+cb)*bsize;
            uint8_t* dst_origin = dst->data() + (cb*n+rb)*bsize;
      const uint8_t* prf_origin = next_block(src, rb, cb);
      for (int64_t r = 0; r < bsize; r++) {
        _mm_prefetch(prf_origin + r*n, _MM_HINT_NTA);
        for (int64_t c = 0; c < bsize; c++) {
          dst_origin[r * n + c] = src_origin[c * n + r];
        }
      }
    }
  }
}

/*********************** Vec32/Vec64 ***********************/

template<typename W> /* W is either u32 or u64 and defines word size */
void transpose_Vec_kernel(const uint8_t* src, uint8_t* dst, int64_t stride);

template<>
void transpose_Vec_kernel<uint32_t>(const uint8_t* src, uint8_t* dst, int64_t stride) {
  // load rows of src matrix
  uint32_t a0 = *((uint32_t*)(src+0*stride));
  uint32_t a1 = *((uint32_t*)(src+1*stride));
  uint32_t a2 = *((uint32_t*)(src+2*stride));
  uint32_t a3 = *((uint32_t*)(src+3*stride));

  // 2x2 block matrices
  uint32_t b0 = (a0 & 0x00ff00ffU) | ((a1 << 8) & 0xff00ff00U);
  uint32_t b1 = (a1 & 0xff00ff00U) | ((a0 >> 8) & 0x00ff00ffU);
  uint32_t b2 = (a2 & 0x00ff00ffU) | ((a3 << 8) & 0xff00ff00U);
  uint32_t b3 = (a3 & 0xff00ff00U) | ((a2 >> 8) & 0x00ff00ffU);

  // 4x4 block matrices
  uint32_t c0 = (b0 & 0x0000ffffU) | ((b2 << 16) & 0xffff0000U);
  uint32_t c1 = (b1 & 0x0000ffffU) | ((b3 << 16) & 0xffff0000U);
  uint32_t c2 = (b2 & 0xffff0000U) | ((b0 >> 16) & 0x0000ffffU);
  uint32_t c3 = (b3 & 0xffff0000U) | ((b1 >> 16) & 0x0000ffffU);

  // write to dst matrix
  *(uint32_t*)(dst + 0*stride) = c0;
  *(uint32_t*)(dst + 1*stride) = c1;
  *(uint32_t*)(dst + 2*stride) = c2;
  *(uint32_t*)(dst + 3*stride) = c3;
}


template<>
void transpose_Vec_kernel<uint64_t>(const uint8_t* src, uint8_t* dst, int64_t stride) {
  // load rows of src matrix
  uint64_t a0 = *((uint64_t*)(src+0*stride));
  uint64_t a1 = *((uint64_t*)(src+1*stride));
  uint64_t a2 = *((uint64_t*)(src+2*stride));
  uint64_t a3 = *((uint64_t*)(src+3*stride));
  uint64_t a4 = *((uint64_t*)(src+4*stride));
  uint64_t a5 = *((uint64_t*)(src+5*stride));
  uint64_t a6 = *((uint64_t*)(src+6*stride));
  uint64_t a7 = *((uint64_t*)(src+7*stride));

  // 2x2 block matrices
  uint64_t b0 = (a0 & 0x00ff00ff00ff00ffULL) | ((a1 << 8) & 0xff00ff00ff00ff00ULL);
  uint64_t b1 = (a1 & 0xff00ff00ff00ff00ULL) | ((a0 >> 8) & 0x00ff00ff00ff00ffULL);
  uint64_t b2 = (a2 & 0x00ff00ff00ff00ffULL) | ((a3 << 8) & 0xff00ff00ff00ff00ULL);
  uint64_t b3 = (a3 & 0xff00ff00ff00ff00ULL) | ((a2 >> 8) & 0x00ff00ff00ff00ffULL);
  uint64_t b4 = (a4 & 0x00ff00ff00ff00ffULL) | ((a5 << 8) & 0xff00ff00ff00ff00ULL);
  uint64_t b5 = (a5 & 0xff00ff00ff00ff00ULL) | ((a4 >> 8) & 0x00ff00ff00ff00ffULL);
  uint64_t b6 = (a6 & 0x00ff00ff00ff00ffULL) | ((a7 << 8) & 0xff00ff00ff00ff00ULL);
  uint64_t b7 = (a7 & 0xff00ff00ff00ff00ULL) | ((a6 >> 8) & 0x00ff00ff00ff00ffULL);

  // 4x4 block matrices
  uint64_t c0 = (b0 & 0x0000ffff0000ffffULL) | ((b2 << 16) & 0xffff0000ffff0000ULL);
  uint64_t c1 = (b1 & 0x0000ffff0000ffffULL) | ((b3 << 16) & 0xffff0000ffff0000ULL);
  uint64_t c2 = (b2 & 0xffff0000ffff0000ULL) | ((b0 >> 16) & 0x0000ffff0000ffffULL);
  uint64_t c3 = (b3 & 0xffff0000ffff0000ULL) | ((b1 >> 16) & 0x0000ffff0000ffffULL);
  uint64_t c4 = (b4 & 0x0000ffff0000ffffULL) | ((b6 << 16) & 0xffff0000ffff0000ULL);
  uint64_t c5 = (b5 & 0x0000ffff0000ffffULL) | ((b7 << 16) & 0xffff0000ffff0000ULL);
  uint64_t c6 = (b6 & 0xffff0000ffff0000ULL) | ((b4 >> 16) & 0x0000ffff0000ffffULL);
  uint64_t c7 = (b7 & 0xffff0000ffff0000ULL) | ((b5 >> 16) & 0x0000ffff0000ffffULL);

  // 8x8 block matrix
  uint64_t d0 = (c0 & 0x00000000ffffffffULL) | ((c4 << 32) & 0xffffffff00000000ULL);
  uint64_t d1 = (c1 & 0x00000000ffffffffULL) | ((c5 << 32) & 0xffffffff00000000ULL);
  uint64_t d2 = (c2 & 0x00000000ffffffffULL) | ((c6 << 32) & 0xffffffff00000000ULL);
  uint64_t d3 = (c3 & 0x00000000ffffffffULL) | ((c7 << 32) & 0xffffffff00000000ULL);
  uint64_t d4 = (c4 & 0xffffffff00000000ULL) | ((c0 >> 32) & 0x00000000ffffffffULL);
  uint64_t d5 = (c5 & 0xffffffff00000000ULL) | ((c1 >> 32) & 0x00000000ffffffffULL);
  uint64_t d6 = (c6 & 0xffffffff00000000ULL) | ((c2 >> 32) & 0x00000000ffffffffULL);
  uint64_t d7 = (c7 & 0xffffffff00000000ULL) | ((c3 >> 32) & 0x00000000ffffffffULL);

  // write to dst matrix
  *(uint64_t*)(dst + 0*stride) = d0;
  *(uint64_t*)(dst + 1*stride) = d1;
  *(uint64_t*)(dst + 2*stride) = d2;
  *(uint64_t*)(dst + 3*stride) = d3;
  *(uint64_t*)(dst + 4*stride) = d4;
  *(uint64_t*)(dst + 5*stride) = d5;
  *(uint64_t*)(dst + 6*stride) = d6;
  *(uint64_t*)(dst + 7*stride) = d7;
}

template<typename W>
void transpose_Vec(const Mat& src, Mat* dst) {
  const int64_t n = src.n();
  const int64_t bsize = 64;
  
  // iterate over 64x64 block matrices
  for (int64_t rb = 0; rb < n/bsize; rb++) {
    for (int64_t cb = 0; cb < n/bsize; cb++) {
      const uint8_t* srcb_origin = src.data()  + (rb*n+cb)*bsize;
            uint8_t* dstb_origin = dst->data() + (cb*n+rb)*bsize;
      const uint8_t* prfb_origin = next_block(src, rb, cb);

      // iterate over sizeof(W)xsizeof(W) block matrices inside 64x64 block
      for (size_t rw = 0; rw < 64/sizeof(W); rw++) {
        // preload sizeof(W) rows of the next 64x64 block
        for (size_t i = rw*sizeof(W); i < (rw+1)*sizeof(W); i++) {
          _mm_prefetch(prfb_origin + i*n, _MM_HINT_NTA);
        }
        for (size_t cw = 0; cw < 64/sizeof(W); cw++) {
          const uint8_t* srcw_origin = srcb_origin + (cw*n + rw)*sizeof(W);
                uint8_t* dstw_origin = dstb_origin + (rw*n + cw)*sizeof(W);
          // use vector kernel to transpose sizeof(W)xsizeof(W) matrix
          transpose_Vec_kernel<W>(srcw_origin, dstw_origin, n);
        }
      }
    }
  }
}

void transpose_Vec32(const Mat& src, Mat* dst) {
  transpose_Vec<uint32_t>(src, dst);
}

void transpose_Vec64(const Mat& src, Mat* dst) {
  transpose_Vec<uint64_t>(src, dst);
}


/************************** Vec256/Vec256Buf ********************************/

/**
 * Generate AVX shuffle mask: [A0 A1 B0 B1 ...] -> [A1 A0 B1 B0 ...],
 * where A_i, B_i has size of (2^logsize) octets.
 */
__m256i make_shuffle_mask(int logsize) {
  int size = (1 << logsize);
  uint8_t mask[32];
  for (int i = 0; i < 32; i++) {
    // Coordinate geometry: (lane, block, index inside block)
    int src_lane = i / 16;
    int src_block = (i % 16) / (2 * size);
    int src_index = (i % 16) % (2 * size);
    
    int dst_index = (src_index + size) % (2 * size);
    int dst_block = src_block;
    int dst_lane = src_lane;

    mask[i] = (dst_lane * 16) + (dst_block * (2 * size)) + dst_index;
  }
  return _mm256_loadu_si256((const __m256i*) mask);
}

/**
 * Generate AVX interleave mask:
 *   src1: [A0 A1 B0 B1 ...]
 *   src2: [A2 A3 B2 B3 ...]
 *   res:  [A0 A3 B0 B3 ...]
 * where A_i, B_i has size of (2^logsize) octets.
 */
__m256i make_blendv_mask(int logsize) {
  uint8_t mask[32];
  for (int i = 0; i < 32; i++) {
    if ((i / (1 << logsize)) % 2 == 0) {
      mask[i] = 0x00;
    } else {
      mask[i] = 0xff;
    }
  }
  
  return _mm256_loadu_si256((const __m256i*) mask);
}

static const __m256i SHUFFLE_MASK[] = {
  make_shuffle_mask(0),
  make_shuffle_mask(1),
  make_shuffle_mask(2),
  make_shuffle_mask(3),
};

static const __m256i BLENDV_MASK[] = {
  make_blendv_mask(0),
  make_blendv_mask(1),
  make_blendv_mask(2),
  make_blendv_mask(3),
  make_blendv_mask(4),
};

#include "transpose_Vec256_kernel.h"

void transpose_Vec256(const Mat& src, Mat* dst) {
  const int64_t n = src.n();
  for (int64_t rb = 0; rb < n/64; rb++) {
    for (int64_t cb = 0; cb < n/64; cb++) {
      const uint8_t* src_origin = src.data()  + (rb*n+cb)*64;
      const uint8_t* prf_origin = next_block(src, rb, cb);
            uint8_t* dst_origin = dst->data() + (cb*n+rb)*64;

      transpose_Vec256_kernel(src_origin,             dst_origin,             prf_origin,        n, n);
      transpose_Vec256_kernel(src_origin + 32*n,      dst_origin + 32,        prf_origin + n*16, n, n);
      transpose_Vec256_kernel(src_origin + 32,        dst_origin + 32*n,      prf_origin + n*32, n, n);
      transpose_Vec256_kernel(src_origin + 32*n + 32, dst_origin + 32*n + 32, prf_origin + n*48, n, n);
    }
  }
}

void transpose_Vec256Buf(const Mat& src, Mat* dst) {
  const int64_t n = src.n();
  uint8_t buf[64*64] __attribute__ ((aligned (64)));
  for (int64_t rb = 0; rb < n/64; rb++) {
    for (int64_t cb = 0; cb < n/64; cb++) {
      const uint8_t* src_origin = src.data()  + (rb*n+cb)*64;
      const uint8_t* prf_origin = next_block(src, rb, cb);
            uint8_t* dst_origin = dst->data() + (cb*n+rb)*64;

      transpose_Vec256_kernel(src_origin,         buf,          prf_origin,      n, 64);
      transpose_Vec256_kernel(src_origin+32*n,    buf+32,       prf_origin+n*16, n, 64);
      transpose_Vec256_kernel(src_origin+32,      buf+32*64,    prf_origin+n*32, n, 64);
      transpose_Vec256_kernel(src_origin+32*n+32, buf+32*64+32, prf_origin+n*48, n, 64);

      for (int row = 0; row < 64; row++) {
        __m256i lane0 = *(const __m256i*)(buf + 64*row);
        __m256i lane1 = *(const __m256i*)(buf + 64*row + 32);
        _mm256_stream_si256((__m256i*)(dst_origin + n*row), lane0);
        _mm256_stream_si256((__m256i*)(dst_origin + n*row + 32), lane1);
      }
    }
  }
}

/**********************************************************/

const std::unordered_map<std::string, std::function<void(const Mat&, Mat*)>> functions = {
  {"Naive",     transpose_Naive},
  {"Reverse",   transpose_Reverse},
  {"Blocks",    transpose_Blocks},
  {"BlocksPrf", transpose_BlocksPrf},
  {"Vec32",     transpose_Vec32},
  {"Vec64",     transpose_Vec64},
  {"Vec256",    transpose_Vec256},
  {"Vec256Buf", transpose_Vec256Buf}
};


void run(int64_t n, const std::string& algo, int64_t volume, bool markers, bool verify) {
  auto function = functions.at(algo);

  int nr_mats = std::max<int>(volume/(n*n), 1);
  
  std::vector<std::unique_ptr<Mat>> srcs;
  std::vector<std::unique_ptr<Mat>> dsts;
  for (int i = 0; i < nr_mats; i++) {
    srcs.push_back(std::make_unique<Mat>(n));
    dsts.push_back(std::make_unique<Mat>(n));
  }

  for (int i = 0; i < nr_mats; i++) {
    fill_random(srcs[i].get());
  }

  for (int i = 0; i < nr_mats; i++) {
    explicit_bzero(dsts[i]->data(), n*n);
  }
  
  if (markers) {
    printf("PERF-BEGIN\n");
    fflush(stdout);
  }
  int64_t ts1 = rdtscp();
  for (int i = 0; i < nr_mats; i++) {
    function(*srcs[i], dsts[i].get());
  }
  int64_t ts2 = rdtscp();
  if (markers) {
    printf("PERF-END\n");
    fflush(stdout);
  }

  if (verify) {
    for (int i = 0; i < nr_mats; i++) {
      check_transpose(*srcs[i], *dsts[i]);
    }
  }

  printf("%15s %7ld %10.6f %d\n",
    algo.c_str(),
    n,
    double(ts2-ts1)/nr_mats/n/n, // cycles per element
    nr_mats
  );
  fflush(stdout);
}


int main(int argc, char* argv[]) {
  if (argc != 6) {
    printf("Usage: %s <n> <algorithm> <volume> <markers> <verify>\n", argv[0]);
    return 1;
  }
  int64_t n = std::stoll(argv[1]);
  std::string algorithm(argv[2]);
  int64_t volume = std::stoll(argv[3]);
  int markers = atoi(argv[4]);
  int verify = atoi(argv[5]);

  run(n, algorithm, volume, markers, verify);

  return 0;
}
