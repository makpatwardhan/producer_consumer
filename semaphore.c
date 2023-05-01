#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 10
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
#define NUM_ITEMS 20

int buffer[BUFFER_SIZE];
int in = 0, out = 0, count = 0;

sem_t empty, full;
pthread_mutex_t mutex;

void *producer(void *arg) {
    int i, item;
    long int thread_id = (long int)arg;
    for (i = 0; i < NUM_ITEMS; i++) {
        item = rand() % 100; // Generate a random item
        sem_wait(&empty); // Wait for an empty slot in the buffer
        pthread_mutex_lock(&mutex); // Acquire the mutex lock
        buffer[in] = item; // Put the item in the buffer
        printf("Producer %ld produced item %d\n", thread_id, item);
        in = (in + 1) % BUFFER_SIZE; // Update the buffer index
        count++; // Increment the item count
        pthread_mutex_unlock(&mutex); // Release the mutex lock
        sem_post(&full); // Signal that there's a full slot in the buffer
    }
    pthread_exit(NULL);
}

void *consumer(void *arg) {
    int i, item;
    long int thread_id = (long int)arg;
    for (i = 0; i < NUM_ITEMS / NUM_CONSUMERS; i++) {
        sem_wait(&full); // Wait for a full slot in the buffer
        pthread_mutex_lock(&mutex); // Acquire the mutex lock
        item = buffer[out]; // Take an item from the buffer
        printf("Consumer %ld consumed item %d\n", thread_id, item);
        out = (out + 1) % BUFFER_SIZE; // Update the buffer index
        count--; // Decrement the item count
        pthread_mutex_unlock(&mutex); // Release the mutex lock
        sem_post(&empty); // Signal that there's an empty slot in the buffer
    }
    pthread_exit(NULL);
}

int main() {
    int i;
    pthread_t producers[NUM_PRODUCERS], consumers[NUM_CONSUMERS];
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);
    for (i = 0; i < NUM_PRODUCERS; i++) {
        pthread_create(&producers[i], NULL, producer, (void *)(long int)i);
    }
    for (i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer, (void *)(long int)i);
    }
    for (i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    for (i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);
    return 0;
}

