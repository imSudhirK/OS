#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include "zemaphore.h"

#define NUM_THREADS 3
#define NUM_ITER 10

zem_t sema_p[NUM_THREADS];     //------- declaration of semaphore variables

void *justprint(void *data)
{
  int thread_id = *((int *)data);
  for(int i=0; i < NUM_ITER; i++)
    {
      zem_down(&sema_p[thread_id]);   //----------decrement value
      printf("This is thread %d\n", thread_id);
      thread_id +=1;                  //----- increment thread index
      thread_id %= NUM_THREADS;
      zem_up(&sema_p[thread_id]);  //--------increment value and wake up specific thread
    }
  return 0;
}

int main(int argc, char *argv[])
{
   //---------------initialization
  zem_init(&sema_p[0], 1);   //------------first value to 1, control here
  for(int i=1; i < NUM_THREADS; i++){     //---------------CV are 0
    zem_init(&sema_p[i], 0);
  }

  //----given
  pthread_t mythreads[NUM_THREADS];
  int mythread_id[NUM_THREADS];

  for(int i =0; i < NUM_THREADS; i++)
    {
      mythread_id[i] = i;
      pthread_create(&mythreads[i], NULL, justprint, (void *)&mythread_id[i]);
    }

  for(int i =0; i < NUM_THREADS; i++)
    {
      pthread_join(mythreads[i], NULL);
    }

  return 0;
}
