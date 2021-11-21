
#include "queue.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct QueueStruct {
    int front, rear, capacity;
    void **array;
    pthread_mutex_t lock;
    sem_t write;
    sem_t read;
} Queue;


Queue *queue_alloc(int size) {
    void *array = malloc(size * sizeof(void*));
    Queue *queue = malloc(sizeof(Queue));
    int num2 = sem_init(&queue->read, 0, 0); // Get
    int num1 = sem_init(&queue->write, 0, size); // Put
    queue->front = 0;
    queue->rear = 0;
    queue->capacity = size;
    //queue->size = 0;
    queue->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    queue->array = array;
    return queue;
}

void queue_free(Queue *queue) {
    pthread_mutex_destroy(&queue->lock);
    sem_destroy(&queue->write);
    sem_destroy(&queue->read);
    free(queue->array);
    free(queue);
}

void queue_put(Queue *queue, void *item) {
    sem_wait(&queue->write); // wait for a space
    pthread_mutex_lock(&queue->lock);
    queue->array[queue->rear] = item;
    queue->rear = (queue->rear + 1) % queue->capacity;
    pthread_mutex_unlock(&queue->lock);
    sem_post(&queue->read);
}



void *queue_get(Queue *queue) {
    sem_wait(&queue->read);
    pthread_mutex_lock(&queue->lock);
    void *item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    pthread_mutex_unlock(&queue->lock);
    sem_post(&queue->write);
    return item;
}

