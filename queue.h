#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

// Data structure to represent a queue
struct item
{
    void *value;     // array to store queue elements
};
struct queue
{
    struct item *data;
    int maxsize;    // maximum capacity of the queue
    int front;      // front points to the front element in the queue (if any)
    int rear;       // rear points to the last element in the queue
    int size;       // current capacity of the queue
};

struct queue* newQueue(struct queue *pt, int size);

int size(struct queue *pt);

int isEmpty(struct queue *pt);

int isFull(struct queue* pt);

//void* front(struct queue *pt);

void queue_push(struct queue *pt, void *x);

void* queue_pop(struct queue *pt);

void queue_free(struct queue *pt);

#endif