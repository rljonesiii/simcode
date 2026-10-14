/*
*  Module:   circ_buf_queue.c
*  
*  Author:   Rob Jones
*
*  Version:  3/31/97
*
*  Purpose:  Implements FIFO queue for each server
*/

#include <stdio.h>
#include "queue.h"

void InitQueue(QueueType *q, int size)
/* 
*  initialize the (FIFO) queue 
*/
{
  q->contents = (ItemType *)calloc(size,sizeof(ItemType));
  q->max      = size;
  q->number   = 0;
  q->head     = 0;
  q->tail     = 0;
}

void Put(QueueType *q, int c, double t)
/* 
*  inserts c and t into queue q 
*/
{
  ItemType *queue = q->contents;

  /* check for queue overflow */
  if ((q->number > 0) && (q->head == q->tail)) { 
    fprintf(stderr,"Error: queue overflow!\n");
    exit(1);
  } else {
    queue[q->tail].index = c;
    queue[q->tail].time  = t;
    q->tail = (q->tail+1) % q->max;
    q->number++;
  }
}

int Take(QueueType *q, int *c, double *t)
{
/*
*  returns the contents c and t the head of queue q 
*  (note: returns c = t = 0 if queue is empty)
*/
  ItemType *queue = q->contents;

  if (q->head != q->tail) {
    (*c)    = queue[q->head].index;
    (*t)    = queue[q->head].time;
    q->head = (q->head+1) % q->max;
    q->number--;
  }
}

int NumberInQueue(QueueType *q)
/* 
*  returns the number in the queue q 
*/
{
  return(q->number);
}
   
                 
