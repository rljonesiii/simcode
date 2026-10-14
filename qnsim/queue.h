/******************************************************************************
*                                                                             *
*  queue.h                                                                    *
*                                                                             *
*  Version:  30-April-97                                                      *
*                                                                             *
*******************************************************************************/
#ifndef QUEUE_H
#define QUEUE_H

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

typedef long queue_type;   /* a FIFO queue only needs to count its jobs */

/* FIFO QUEUE OPERATIONS (the item value is not stored) */

static inline void CreateQueue(queue_type *q) { *q = 0; }
static inline void DeleteQueue(queue_type *q) { *q = 0; }
static inline void FlushQueue(queue_type *q)  { *q = 0; }
static inline bool IsEmpty(queue_type *q)     { return *q == 0; }
static inline long Number(queue_type *q)      { return *q; }
static inline void Insert(queue_type *q, [[maybe_unused]] item_type v) { (*q)++; }
static inline void Remove(queue_type *q, [[maybe_unused]] item_type *v) { (*q)--; }

#endif

#endif /* QUEUE_H */

