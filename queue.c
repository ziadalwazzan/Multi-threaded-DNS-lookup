// https://www.techiedelight.com/circular-queue-implementation-c/
#include "queue.h"

// Utility function to initialize a queue
struct queue* newQueue(struct queue *pt, int size)
{
    pt->data = malloc(size * sizeof(struct item));
    pt->maxsize = size;
    pt->front = 0;
    pt->rear = -1;
    pt->size = 0;

    return pt;
}
 
// Utility function to return the size of the queue
int size(struct queue *pt) {
    return pt->size;
}
 
// Utility function to check if the queue is empty or not
int isEmpty(struct queue *pt) {
    return !size(pt);
}

// Queue is full when size becomes
// equal to the capacity
int isFull(struct queue* pt)
{
    return (pt->size == pt->maxsize);
}
 
// Utility function to add an element `x` to the queue
void queue_push(struct queue *pt, void *x)
{
    if (isFull(pt))
    {
        printf("Overflow\nProgram Terminated\n");
        exit(EXIT_FAILURE);
    }
 
    pt->rear = (pt->rear + 1) % pt->maxsize;    // circular queue
    pt->data[pt->rear].value = x;
    pt->size++;
 
    //printf("front = %d, rear = %d\n", pt->front, pt->rear);
}
 
// Utility function to dequeue the front element
void* queue_pop(struct queue *pt)
{
    if (isEmpty(pt))    // front == rear
    {
        printf("Underflow\nProgram Terminated\n");
        exit(EXIT_FAILURE);
    }
 
    //printf("Removing %d\t", front(pt));
    void *value = pt->data[pt->front].value;
    pt->data[pt->front].value = NULL;
    pt->front = (pt->front + 1) % pt->maxsize;  // circular queue
    pt->size--;
    return value;
    //printf("front = %d, rear = %d\n", pt->front, pt->rear);
}

void queue_free(struct queue *pt) 
{ 
    while(!isEmpty(pt)){
        void *temp;
        temp = queue_pop(pt);
        free(temp);
    }
    free(pt->data);
}