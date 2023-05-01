#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


int *buffer;
int count = 0;
int NUM_ITEMS = 0;
int BUFFER_SIZE = 0;
int in = 0;
int out = 0;
int NUM_CONSUMERS = 0;

typedef struct {
  int count;
  pthread_mutex_t mutex;
  pthread_cond_t condition;
} semaphore_t;

semaphore_t empty,full;
pthread_mutex_t mutex;

void semaphore_init(semaphore_t *sem, int initial_count) {
  sem->count = initial_count;
  pthread_mutex_init(&sem->mutex, NULL);
  pthread_cond_init(&sem->condition, NULL);
}

void semaphore_wait(semaphore_t *sem) {
  pthread_mutex_lock(&sem->mutex);
  while (sem->count <= 0) {
    pthread_cond_wait(&sem->condition, &sem->mutex);
  }
  sem->count--;
  pthread_mutex_unlock(&sem->mutex);
}

void semaphore_signal(semaphore_t *sem) {
  pthread_mutex_lock(&sem->mutex);
  sem->count++;
  pthread_cond_signal(&sem->condition);
  pthread_mutex_unlock(&sem->mutex);
}


void *producer(void *arg) {
    int i, item;
    long int thread_id = (long int)arg;
    for (i = 0; i < NUM_ITEMS; i++) {
        item = rand() % 100; // Generate a random item
        semaphore_wait(&empty); // Wait for an empty slot in the buffer
        pthread_mutex_lock(&mutex); // Acquire the mutex lock
        buffer[in] = item; // Put the item in the buffer
        printf("Producer %ld produced item %d\n", thread_id, item);
        in = (in + 1) % BUFFER_SIZE; // Update the buffer index
        count++; // Increment the item count
        pthread_mutex_unlock(&mutex); // Release the mutex lock
        semaphore_signal(&full); // Signal that there's a full slot in the buffer
    }
    pthread_exit(NULL);
}

void *consumer(void *arg) {
    int i, item;
    long int thread_id = (long int)arg;
    for (i = 0; i < NUM_ITEMS / NUM_CONSUMERS; i++) {
        semaphore_wait(&full); // Wait for a full slot in the buffer
        pthread_mutex_lock(&mutex); // Acquire the mutex lock
        item = buffer[out]; // Take an item from the buffer
        printf("Consumer %ld consumed item %d\n", thread_id, item);
        out = (out + 1) % BUFFER_SIZE; // Update the buffer index
        count--; // Decrement the item count
        pthread_mutex_unlock(&mutex); // Release the mutex lock
        semaphore_signal(&empty); // Signal that there's an empty slot in the buffer
    }
    pthread_exit(NULL);
}


int main(int argc ,char *argv[])
{
    int i;
    pthread_t prod_threads[3], cons_threads[3];
    semaphore_t sem_t; 
    int *prod_ids = NULL ,*cons_ids = NULL;
    int Max_Sleep_Seconds, Total_Number_Items2Produce;
    int number_of_producer_threads, number_of_consumer_threads;

     if(argc != 6)
     {
     	printf("Usage: multiple_producer_consumer.out Number_Producers, Number_Consumers, Max_Sleep_Seconds, Total_Number_Items2Produce, Ring_Buffer_Size\n");
     	return 0;
     }
     number_of_producer_threads = atoi(argv[1]);
     number_of_consumer_threads = atoi(argv[2]);
     NUM_CONSUMERS = number_of_consumer_threads;
     BUFFER_SIZE = atoi(argv[5]);
     NUM_ITEMS = atoi(argv[4]);
     printf("The producer threads are %d\n" , number_of_producer_threads);
     printf("The number of consumer threads are %d\n", number_of_consumer_threads);
     printf("The buffer size is %d\n" ,  BUFFER_SIZE);
     printf("The number of items are %d\n", NUM_ITEMS);
     prod_ids = (int *)malloc(number_of_producer_threads * sizeof(int));
     if(prod_ids == NULL)
     {
     	  printf("Unable to create the producer threads");
     	  return 0;
     }
     
     cons_ids = (int *)malloc(number_of_consumer_threads * sizeof(int));     
     if(cons_ids == NULL)
     {
     	printf("Unable to create the consumer threads");
     	return 0;
     }
     buffer = (int *)malloc(BUFFER_SIZE*sizeof(int));
     if(buffer == NULL)
     {
     	printf("Unable to allocate the buffer");
     	return 0;
     }
    semaphore_init(&empty,0);
    semaphore_init(&full,0);
    // create producer threads
    for (i = 0; i < number_of_producer_threads; i++) {
        prod_ids[i] = i;
        pthread_create(&prod_threads[i], NULL, producer, (void *) &prod_ids[i]);
    }

    // create consumer threads
    for (i = 0; i < number_of_consumer_threads; i++) {
        cons_ids[i] = i;
        pthread_create(&cons_threads[i], NULL, consumer, (void *) &cons_ids[i]);
    }

    // join producer threads
    for (i = 0; i < number_of_producer_threads; i++) {
        pthread_join(prod_threads[i], NULL);
    }
   
    // join consumer threads	
    for (i = 0; i < number_of_consumer_threads; i++) {
        pthread_join(cons_threads[i], NULL);
    }

    free(prod_ids);
    free(cons_ids);
    free(buffer);
    
    return 0;
}

