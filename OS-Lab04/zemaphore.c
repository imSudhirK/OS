#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include "zemaphore.h"

//--------------------------initializes the specified zemaphore to the specified value
void zem_init(zem_t *s, int value) {
  s->counter = value;
  s->lockdown = PTHREAD_MUTEX_INITIALIZER;
  s->cond_var = PTHREAD_COND_INITIALIZER;
}

//--------------------------semaphores down function implementation
void zem_down(zem_t *s) {
  pthread_mutex_lock(&s->lockdown);   //----------acquire lock and do something
  s->counter -=1;                     //--------decrements the counter value of the zemaphore by one
  if(s->counter < 0){                //----------If the value is negative,
    pthread_cond_wait(&s->cond_var, &s->lockdown);  //-------thread blocks and is context switched out
  }
  pthread_mutex_unlock(&s->lockdown);
}

//-----------------increments the counter value of the zemaphore by one, and wakes up any one sleeping thread
void zem_up(zem_t *s) {
  pthread_mutex_lock(&s->lockdown);
  s->counter +=1;
  pthread_cond_signal(&s->cond_var);
  pthread_mutex_unlock(&s->lockdown);
}
