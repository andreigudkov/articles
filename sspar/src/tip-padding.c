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

static int64_t rnd64() {
  int64_t val = 0;
  for (int i = 0; i < 8; i++) {
    val = (val << 8) | (rand() & 0xff);
  }
  return val;
}

#define RECSIZE 4096
#define PROBES (1*1000*1000)

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  srand(time(NULL));

  int fd = open(argv[1], O_WRONLY, 0666);
  if (fd == -1) {
    fprintf(stderr, "open(): %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  struct stat st;
  if (fstat(fd, &st) == -1) {
    fprintf(stderr, "fstat(): %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  off_t size = st.st_size;
  off_t records = size / RECSIZE;

  char buf[RECSIZE];
  int64_t total = 0;
  for (int i = 0; i < PROBES; i++) {
    buf[0] = (char) (rnd64() & 0xff);
    off_t offset = ((rnd64() & UINT64_MAX) % records) * RECSIZE;
    int64_t begin = now();
    if (pwrite(fd, buf, sizeof(buf), offset) != sizeof(buf)) {
      fprintf(stderr, "pwrite(): %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    int64_t end = now();
    total += end - begin;
    if (i % 100000 == 0) {
      printf("%d\n", i);
    }
  }
  printf("Total %f, bufsize %"PRId64"\n", total/1E9, sizeof(buf));

  int64_t begin = now();
  fsync(fd);
  int64_t end = now();
  printf("fsync %f\n", (end-begin)/1E9);

  close(fd);

  exit(EXIT_SUCCESS);
  return 0;
}

