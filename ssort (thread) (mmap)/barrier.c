// Author: Nat Tuck
// CS3650 starter code

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

#include "barrier.h"

//referenced code from Nat Tuck CS3650 HW06
barrier*
make_barrier(int nn)
{
  int rv;
  barrier* bb = malloc(sizeof(barrier));
  assert(bb != 0);
  if ((long) bb == -1) {
    perror("malloc barrier");
    abort();
  }
  rv = pthread_cond_init(&(bb->condv), 0);
  if (rv == -1) {
    perror("pthread_cond_init(condv)");
    abort();
  }
  rv = pthread_mutex_init(&(bb->mutex), 0);
  if (rv == -1) {
    perror("pthread_mutex_init(mutex)");
    abort();
  }
  bb->count = nn;
  bb->seen  = 0;
  return bb;
}

void
barrier_wait(barrier* bb)
{
  //referenced from Nat Tuck Lecture notes 15-threads-pt2/condafor/par-stack.c
  int rv;
  rv = pthread_mutex_lock(&(bb->mutex));
  if (rv == -1) {
    perror("pthread_mutex_lock(mutex)");
    abort();
  }

  bb->seen+=1;
  int seen = bb->seen;

  if (seen < bb->count) {
    while (bb->seen < bb->count) {
      rv = pthread_cond_wait(&(bb->condv), &(bb->mutex));
      if(rv == -1) {
        perror("pthread_cond_wait(conv, mutex)");
        abort();
      }
    }
  }
  else {
    rv = pthread_cond_broadcast(&(bb->condv));
    if(rv == -1) {
      perror("pthread_cond_broadcast(condv)");
      abort();
    }
  }

  rv = pthread_mutex_unlock(&(bb->mutex));
  if(rv == -1) {
    perror("pthread_mutex_unlock(mutex)");
    abort();
  }
}

void
free_barrier(barrier* bb)
{
  free(bb);
}
