#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

int item_to_produce, curr_buf_size;  //--next item to be produced and dynamic size of buffer
int total_items, max_buf_size, num_workers, num_masters; //--, M, N, C, P


//----------------------lock, condition and global varibale declaration
pthread_mutex_t lockdown;
pthread_cond_t item_consumed, item_produced;  //-------- is item consumed/ produced
int item_to_consume; //-------this time this item need to be consumed

//given
int *buffer;   //--creating buffer list
void print_produced(int num, int master) {  //--printing
  printf("Produced %d by master %d\n", num, master);
}
void print_consumed(int num, int worker) {
  printf("Consumed %d by worker %d\n", num, worker);
}

//produce items and place in buffer
//modify code below to synchronize correctly
void *generate_requests_loop(void *data){  //-----function for masters
  int thread_id = *((int *)data);

  while(1){
	  pthread_mutex_lock(&lockdown);   //----------acquire lock and do something

	  if(item_to_produce >= total_items) {   //------------if masters are done producing
		  pthread_cond_broadcast(&item_consumed);
		  pthread_mutex_unlock(&lockdown);
		 break;
	  }

	  while(curr_buf_size >= max_buf_size) { //---------what if buffer is full
      pthread_cond_wait(&item_consumed, &lockdown);  //-- wait for item to be consumed
		  if(item_to_produce >= total_items) {  //----------masters job is done
			  pthread_cond_broadcast(&item_consumed);
			  pthread_mutex_unlock(&lockdown);
			 return 0 ;
		  }
		}
    //given
	  buffer[curr_buf_size++] = item_to_produce; //------- store item produced and increment buffer size
	  print_produced(item_to_produce, thread_id);
	  item_to_produce++;                         //----next time time item needs to be produced
    //----------------signal to workers
	  pthread_cond_signal(&item_produced);
	  pthread_mutex_unlock(&lockdown);
	}
  return 0;
}

//----------------------------worker threads functionality
//write function to be run by worker threads
//ensure that the workers call the function print_consumed when they consume an item
void *worker_threads_loop(void *data){
  int thread_id = *((int *)data);

  while(1){
	  pthread_mutex_lock(&lockdown);

	  if(curr_buf_size<=0 && item_to_produce >= total_items) { //----workers are also done
      pthread_cond_broadcast(&item_produced);
		  pthread_mutex_unlock(&lockdown);
		break;
	  }

	  while(curr_buf_size<=0){    //-----------------------buffer is empty
      pthread_cond_wait(&item_produced, &lockdown);   //---wait for items to be consumed
		  if(curr_buf_size<=0 && item_to_produce >= total_items) { //---workers job are also done
        pthread_cond_broadcast(&item_produced);
        pthread_mutex_unlock(&lockdown);
			  return 0 ;
		  }
	  }

//--------------------------------otherwise consume something LIFO
	item_to_consume = buffer[curr_buf_size-1];
	curr_buf_size= curr_buf_size-1;
	print_consumed(item_to_consume, thread_id);

  //-------------------signal to producers
	pthread_cond_signal(&item_consumed);
	pthread_mutex_unlock(&lockdown);
  }

  return 0;
}


int main(int argc, char *argv[])
{
  int *master_thread_id;
  pthread_t *master_thread;
  item_to_produce = 0;
  curr_buf_size = 0;

  //-----------------------declaration for workers threads
  int *worker_thread_id;
  pthread_t *worker_thread;

  //given
  int i;
  if (argc < 5) {  //--num argument passed did not met the requirment
    printf("./master-worker #total_items #max_buf_size #num_workers #masters e.g. ./exe 10000 1000 4 3\n");
	  exit(1);
  }
  else { //-- initializing 
	num_masters = atoi(argv[4]);
	num_workers = atoi(argv[3]);
	total_items = atoi(argv[1]);
	max_buf_size = atoi(argv[2]);
  }

  buffer = (int *)malloc (sizeof(int) * max_buf_size);
  //create master producer threads
  master_thread_id = (int *)malloc(sizeof(int) * num_masters);
  master_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_masters);
  for (i = 0; i < num_masters; i++)
	master_thread_id[i] = i;

  for (i = 0; i < num_masters; i++)
	pthread_create(&master_thread[i], NULL, generate_requests_loop, (void *)&master_thread_id[i]);

  //----------------------------------as master as workers
  //create worker consumer threads

  worker_thread_id = (int *)malloc(sizeof(int) * num_workers);
  worker_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_workers);
  for (i = 0; i < num_workers; i++)
	worker_thread_id[i] = i;

  for (i = 0; i < num_workers; i++)
	pthread_create(&worker_thread[i], NULL, worker_threads_loop, (void *)&worker_thread_id[i]);

  //wait for all threads to complete
  for (i = 0; i < num_masters; i++)
	{
	  pthread_join(master_thread[i], NULL);
	  printf("master %d joined\n", i);
	}

  //---------------------------------reaping up workers thread
  for (i = 0; i < num_workers; i++)
	{
	  pthread_join(worker_thread[i], NULL);
	  printf("worker %d joined\n", i);
	}

  /*----Deallocating Buffers---------------------*/
  free(buffer);
  free(master_thread_id);
  free(master_thread);

  //---------------------deallocating workers lists
  free(worker_thread_id);
  free(worker_thread);

  return 0;
}
