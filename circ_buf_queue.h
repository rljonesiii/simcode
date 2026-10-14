/*
*  Module:   circ_buf_queue.h
*
*  Purpose:  Implements FIFO queue 
*/
#ifndef CIRC_BUF_QUEUE_H
#define CIRC_BUF_QUEUE_H

typedef struct {
  int    index;        /* item index  */
  double time;         /* time placed */
} ItemType;

typedef struct {
  ItemType *contents;  /* queue contents  */
  int      max;        /* max queue size  */
  int      number;     /* number in queue */
  int      head;
  int      tail;
} QueueType;
/* 
*  Operations for Queue
*/
void InitQueue(QueueType *q, int size);
void Put(QueueType *q, int c, double t);
bool Take(QueueType *q, int *c, double *t);   /* false if q is empty */
int  NumberInQueue(QueueType *q);

#endif /* CIRC_BUF_QUEUE_H */
