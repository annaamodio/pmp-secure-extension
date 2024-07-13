#include "cqueue.h"

void cqueue_init(cqueue_t *q) {
    q->head = -1;
    q->tail = -1;
}

bool cqueue_empty(cqueue_t *q) {
    return q->head == -1;
}

bool cqueue_full(cqueue_t *q){
    return (q->head == q->tail + 1) || (q->head == 0 && q->tail == CQUEUE_SIZE - 1);
}

int cqueue_enq(cqueue_t *q, uint8_t elem){
    
    if (cqueue_full(q))
        return -1;

    if (cqueue_empty(q))
        q->head = 0;

    q->tail = (q->tail + 1) % CQUEUE_SIZE;
    q->mem[q->tail] = elem;

    return 0;
}

int cqueue_deq(cqueue_t *q, uint8_t *elem){
    
    if (cqueue_empty(q) || !elem)
        return -1;

    *elem = q->mem[q->head];
    
    if (q->head == q->tail) {
        q->head = -1;
        q->tail = -1;
    }
    else
        q->head = (q->head + 1) % CQUEUE_SIZE;
    
    return 0;
}