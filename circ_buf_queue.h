/*
*  Module:   circ_buf_queue.h
*
*  Purpose:  Implements FIFO queue 
*/
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
int  Take(QueueType *q, int *c, double *t);
int  NumberInQueue(QueueType *q);

