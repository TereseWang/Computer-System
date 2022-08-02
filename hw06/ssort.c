#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "float_vec.h"
#include "barrier.h"
#include "utils.h"

int
comparator(const void *a, const void *b){
  return *(const float*)a > *(const float*)b;
}
void
qsort_floats(floats* xs)
{
  qsort(xs->data, xs->size, sizeof(float), comparator);
}

floats*
sample(float* data, long size, int P)
{
  floats* xs = make_floats(0);
  for(int i = 0; i < 3 * (P - 1); i++) {
    floats_push(xs, data[rand() % size]);
  }
  qsort_floats(xs);
  floats* sample = make_floats(0);
  floats_push(sample, 0);
  for(int i = 0; i <  3 * (P - 1); i+=3) {
    floats_push(sample, xs->data[i]);
  }
  floats_push(sample, +INFINITY);
  free_floats(xs);
  return sample;
}

void
sort_worker(int pnum, float* data, long size, int P, floats* samps, long* sizes, barrier* bb)
{
  floats* xs = make_floats(0);
  // TODO: select the floats to be sorted by this worker
  for(int i = 0; i < size; i++){
    if(data[i] <= samps->data[pnum + 1] && data[i] > samps->data[pnum]) {
      floats_push(xs, data[i]);
    }
  }
  printf("%d: start %.04f, count %ld\n", pnum, samps->data[pnum], xs->size);
  sizes[pnum] = xs->size;
  barrier_wait(bb);
  qsort_floats(xs);
  long start = 0;
  for(int i = 0; i < pnum; i++){
    start+=sizes[i];
  }
  for(long i = start; i < start + sizes[pnum]; i++) {
    data[i] =  xs->data[i - start];
  }
  free_floats(xs);
}

void
run_sort_workers(float* data, long size, int P, floats* samps, long* sizes, barrier* bb)
{
  pid_t kids[P];
  (void) kids; // suppress unused warning

  for(int i = 0; i < P; ++i){
    if(!(kids[i] = fork())) {
      sort_worker(i, data, size, P, samps, sizes, bb);
      exit(0);
    }
  }

  for (int ii = 0; ii < P; ++ii) {
    int rv = waitpid(kids[ii], 0, 0);
    check_rv(rv);
  }
}

void
sample_sort(float* data, long size, int P, long* sizes, barrier* bb)
{
  floats* samps = sample(data, size, P);
  run_sort_workers(data, size, P, samps, sizes, bb);
  free_floats(samps);
}

int
main(int argc, char* argv[])
{
  alarm(120);

  if (argc != 3) {
    printf("Usage:\n");
    printf("\t%s P data.dat\n", argv[0]);
    return 1;
  }

  const int P = atoi(argv[1]);
  const char* fname = argv[2];

  seed_rng();

  int rv;
  struct stat st;
  rv = stat(fname, &st);
  check_rv(rv);

  const int fsize = st.st_size;
  if (fsize < 8) {
    printf("File too small.\n");
    return 1;
  }

  int fd = open(fname, O_RDWR);
  check_rv(fd);

  //From Nat tuck CS3650 Lecture 12 Notes save_array.c
  void* file = mmap(0, fsize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  (void) file; // suppress unused warning.

  long count = ((long*)file)[0];
  float* data = (float*)file + 2;

  long sizes_bytes = P * sizeof(long);
  //From Nat tuck CS3650 Lecture 12 Notes shared2.c
  long* sizes =  mmap(0, sizes_bytes, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

  barrier* bb = make_barrier(P);

  sample_sort(data, count, P, sizes, bb);

  free_barrier(bb);

  munmap(file, fsize);
  munmap(sizes, sizes_bytes);
  return 0;
}
