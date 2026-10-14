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
#include <stdlib.h>
#include "circ_buf_queue.h"

void InitQueue(QueueType *q, int size)
/* 
*  initialize the (FIFO) queue 
*/
{
  q->contents = (ItemType *)calloc(size,sizeof(ItemType));
  if (q->contents == nullptr) {
    fprintf(stderr,"Error: unable to allocate queue!\n");
    exit(EXIT_FAILURE);
  }
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

bool Take(QueueType *q, int *c, double *t)
{
/*
*  removes the head of queue q and returns its contents in c and t;
*  returns false (leaving c and t unchanged) if the queue is empty.
*  (2026: tests the count rather than head != tail, which also holds
*  when the queue is completely full.)
*/
  ItemType *queue = q->contents;

  if (q->number == 0) {
    return false;
  }
  (*c)    = queue[q->head].index;
  (*t)    = queue[q->head].time;
  q->head = (q->head+1) % q->max;
  q->number--;
  return true;
}

int NumberInQueue(QueueType *q)
/* 
*  returns the number in the queue q 
*/
{
  return(q->number);
}
   
                 
