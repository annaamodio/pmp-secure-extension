#ifndef CQUEUE_H_
#define CQUEUE_H_

// circular queues.

#include "stdint.h"
#include "stdbool.h"

#ifndef CQUEUE_SIZE
#define CQUEUE_SIZE 2048
#endif

typedef struct cqueue
{
    int head;
    int tail;
    uint8_t mem[CQUEUE_SIZE];
}cqueue_t;

void cqueue_init(cqueue_t *);
bool cqueue_empty(cqueue_t *);
bool cqueue_full(cqueue_t *);
int cqueue_enq(cqueue_t *, uint8_t);
int cqueue_deq(cqueue_t *, uint8_t *);

#endif