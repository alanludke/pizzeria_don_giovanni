#include "queue.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void queue_init(queue_t* q, size_t capacity) {
    memset(q, 0, sizeof(queue_t));
    q->capacity = capacity;
    q->buf = calloc(capacity, sizeof(void*));
    int err;
    err = pthread_mutex_init(&q->mtx, NULL); assert(!err);
    err = sem_init(&q->full, 0, 0);          assert(!err);
    err = sem_init(&q->empty, 0, capacity);  assert(!err);
}

void queue_destroy(queue_t* q) {
    assert(q->buf);
    pthread_mutex_lock(&q->mtx);
    free(q->buf);
    q->buf = 0;
    q->size = 0;
    q->begin = q->end = 0;
    pthread_mutex_unlock(&q->mtx);
    sem_destroy(&q->empty);
    sem_destroy(&q->full);
    pthread_mutex_destroy(&q->mtx);
}

void queue_push_back(queue_t* q, void* val) {
    assert(q->buf);

    sem_wait(&q->empty);
    pthread_mutex_lock(&q->mtx);
    q->buf[q->end] = val;
//    printf("buf[%d] <- %p  (%ld)\n", q->end, val, pthread_self());
    q->end = (q->end + 1) % q->capacity;
    ++q->size;
    sem_post(&q->full);
    pthread_mutex_unlock(&q->mtx);
}

void* queue_wait(queue_t* q) {
    assert(q->buf);
    sem_wait(&q->full);
    pthread_mutex_lock(&q->mtx);
    assert(q->size);
    void* front = q->buf[q->begin];
//    printf("buf[%d] -> %p  (%ld)\n", q->end, front, pthread_self());
    q->begin = (q->begin + 1) % q->capacity;
    --q->size;    
    sem_post(&q->empty);
    pthread_mutex_unlock(&q->mtx);

    return front;
}

int queue_empty(queue_t* q) {
    pthread_mutex_lock(&q->mtx);
    int empty = q->size == 0;
    pthread_mutex_unlock(&q->mtx);
    return empty;
}
