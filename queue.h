#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <pthread.h>
#include <semaphore.h>

typedef struct {
    int capacity;
    int size;
    int begin, end;
    void** buf;
    pthread_mutex_t mtx;
    sem_t full, empty;
} queue_t;

extern void  queue_init(queue_t* q, size_t capacity);
extern void  queue_destroy(queue_t* q);
extern void  queue_push_back(queue_t* q, void* val);
extern void* queue_wait(queue_t* q);
extern  int  queue_empty(queue_t* q);

#endif /*__QUEUE_H__*/
