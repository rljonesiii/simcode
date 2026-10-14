/******************************************************************************
*                                                                             *
*  queue.h                                                                    *
*                                                                             *
*  Version:  30-April-97                                                      *
*                                                                             *
*******************************************************************************/
#ifndef _queue_h
#define _queue_h

#include "misc.h"

typedef double item_type;

#ifdef SJF_QUEUE /* SHORTEST JOB FIRST QUEUE */

/* QUEUE DATA STRUCTURES */

struct lst;

typedef struct lst {
  item_type  item;
  struct lst *next;
} list;

typedef struct {
  list *contents;    
  long number;     
} queue_type;

/* SJF QUEUE OPERATIONS */

void CreateQueue(queue_type *);
void DeleteQueue(queue_type *);
void FlushQueue(queue_type *);
bool IsEmpty(queue_type *);
long Number(queue_type *);
void Insert(queue_type *, item_type);
void Remove(queue_type *, item_type *);
list *First(queue_type *);
list *Next(list *);

#else /* ASSUME SIMPLE FIFO AS DEFAULT */

/* QUEUE DATA STRUCTURE */

typedef long queue_type;

/* FIFI QUEUE OPERATIONS */

#define CreateQueue(q)   (*q) = 0
#define DeleteQueue(q)   (*q) = 0
#define FlushQueue(q)    (*q) = 0
#define IsEmpty(q)       (((*q) == 0) ? TRUE : FALSE)
#define Number(q)        (*q)
#define Insert(q,v)      ((*q)++)
#define Remove(q,v)      ((*q)--)

#endif

#endif /* _queue_h */

