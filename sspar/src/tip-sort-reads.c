#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <inttypes.h>
#include <sys/stat.h>

inline static int64_t now() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ((int64_t)ts.tv_sec)*1000*1000*1000 + ts.tv_nsec;
}

inline static int64_t rnd64() {
  int64_t val = 0;
  for (int i = 0; i < 8; i++) {
    val = (val << 8) | (rand() & 0xff);
  }
  return val;
}

#define RECSIZE 4096
#define RECCOUNT 1000

int main(int argc, char* argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <filename> <seed> <winsize>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  const char* filename = argv[1];
  int seed = atoi(argv[2]);
  int winsize = atoi(argv[3]);


  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "open(): %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  struct stat st;
  if (fstat(fd, &st) == -1) {
    fprintf(stderr, "fstat(): %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  off_t filesize = st.st_size;

  // generate offsets
  int64_t offsets[RECCOUNT];
  srand(seed);
  for (int i = 0; i < RECCOUNT; i++) {
    offsets[i] = ((rnd64() & UINT64_MAX) % (filesize / RECSIZE)) * RECSIZE;
  }
  if (winsize > 1) {
    for (int first = 0; first < RECCOUNT; first += winsize) {
      int last = first + winsize;
      if (last > RECCOUNT) {
        last = RECCOUNT;
      }
      for (int i = first; i < last; i++) {
        for (int j = i+1; j < last; j++) {
          if (offsets[j] < offsets[i]) {
            int64_t tmp = offsets[i];
            offsets[i] = offsets[j];
            offsets[j] = tmp;
          }
        }
      }
    }
  }

  // read loop
  char buf[RECSIZE];
  int64_t ts_begin = now();
  for (int i = 0; i < RECCOUNT; i++) {
    //printf("offset: %"PRId64"\n", offsets[i]);
    if (pread(fd, buf, sizeof(buf), offsets[i]) != sizeof(buf)) {
      fprintf(stderr, "pread(): %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
  int64_t ts_end = now();
  printf("filesize winsize seconds\n");
  printf("%"PRId64" %d %f\n", filesize, winsize, (ts_end-ts_begin)/1E9);

  close(fd);

  exit(EXIT_SUCCESS);
  return 0;
}

