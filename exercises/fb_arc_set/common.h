#ifndef COMMON
#define COMMON

#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0);

typedef struct {
  char from;
  char to;
} Edge;

typedef struct {
  Edge* edges;
  int size;
} EdgeList;

#define SHM_PATH "/11912007shm"  // full path: '/dev/shm/11912007shm'
#define MAX_THRESHOLD 1024       // maximum accepted solution size (>= 8)
#define BUF_SIZE 32              // circular buffer size

typedef struct {
  bool terminate;  // tell generators to terminate (no mutex needed)
  // dont use generatorCounter here because it is not atomic

  EdgeList buf[BUF_SIZE];  // circular buffer for solutions
  size_t write_index;      // index for next write into buffer
  size_t read_index;       // index for next read from buffer

  sem_t num_free;     // free space - used for alternating reads and writes
  sem_t num_used;     // used space - used for alternating reads and writes
  sem_t write_mutex;  // mutual exclusion for generator writes
} ShmStruct;

#endif