#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "float_vec.h"
#include "barrier.h"
#include "utils.h"

typedef struct sample_worker {
    int pnum;
    float* data;
    long size;
    int P;
    floats* samps;
    long* sizes;
    barrier* bb;
    int fd;
}sample_worker;

//most of code is from previous homework
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
sort_worker(sample_worker* arg) {
    int pnum = arg->pnum;
    float* data = arg->data;
    long size = arg->size;
    floats* samps = arg->samps;
    long* sizes = arg->sizes;
    barrier* bb = arg->bb;

    floats* xs = make_floats(0);
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
run_sort_workers(float* data, long size, int P, floats* samps, long* sizes, barrier* bb, int fd)
{
    pthread_t threads[P];
    int rv;
    sample_worker arg[P];
    for (int ii = 0; ii < P; ++ii) {
      arg[ii].pnum = ii;
      arg[ii].data = data;
      arg[ii].size = size;
      arg[ii].P = P;
      arg[ii].samps = samps;
      arg[ii].sizes =  sizes;
      arg[ii].bb = bb;
      arg[ii].fd = fd;
      //referenced from Nat Tuck Lecture 15-threads-pt2/create.c
      rv = pthread_create(&(threads[ii]), 0, (void*)sort_worker, &(arg[ii]));
      check_rv(rv);
    }

    for (int ii = 0; ii < P; ++ii) {
        rv = pthread_join(threads[ii], 0);
        check_rv(rv);
    }
}

void
sample_sort(float* data, long size, int P, long* sizes, barrier* bb, int fd)
{
    floats* samps = sample(data, size, P);
    run_sort_workers(data, size, P, samps, sizes, bb, fd);
    free_floats(samps);
}

int
main(int argc, char* argv[])
{
    alarm(120);

    if (argc != 4) {
        printf("Usage:\n");
        printf("\t%s P data.dat\n", argv[0]);
        return 1;
    }

    const int P = atoi(argv[1]);
    const char* fname = argv[2];
    const char* oname = argv[3];

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
    int fd1 = open(oname, O_CREAT | O_RDWR | O_TRUNC, 0666);
    check_rv(fd1);


    long count;
    rv = read(fd, &count, 8);
    check_rv(rv);

    float* data = malloc(4 * count);
    rv = read(fd, data, count * 4);
    check_rv(rv);

    rv = lseek(fd1, 0, SEEK_SET);
    check_rv(rv);
    rv = write(fd1, &count, sizeof(long));
    check_rv(rv);

    long sizes_bytes = P * sizeof(long);
    long* sizes =  malloc(sizes_bytes);

    barrier* bb = make_barrier(P);
    sample_sort(data, count, P, sizes, bb, fd1);

    rv = lseek(fd1, sizeof(long), SEEK_CUR);
    check_rv(rv);
    rv = write(fd1, data, count * sizeof(float));
    check_rv(rv);

    rv = close(fd);
    check_rv(rv);
    rv = close(fd1);
    check_rv(rv);

    free_barrier(bb);
    free(data);
    free(sizes);
    return 0;
}
