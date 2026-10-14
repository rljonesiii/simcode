/******************************************************************************
*  Module:   queue.c                                                          *
*                                                                             *
*  Author:   Rob Jones                                                        *
*                                                                             *
*  Purpose:  CSci 726 Discrete-Event Simulation: PROJECT #3                   *
*            Provides the queue structure and operations for the simulator.   *
*                                                                             *
*  Version:  5-01-97                                                          *
*******************************************************************************/

#include <stdlib.h>
#include "queue.h"
#include "misc.h"

#ifdef SJF_QUEUE

list *free_nodes = NULL;


list *New()
/******************************************************************************
  Returns a new list structure (node). For more efficient memory management,
  a list of previously freed nodes is kept and used to later return new nodes.
  If the free nodes list is depleted then a new node is allocated in memory.
*******************************************************************************/
{
  list *ptr;

  if (free_nodes == NULL) {
    ptr = (list *)malloc(sizeof(list));
    if (ptr == NULL) {
      error(MEM_ERR);
    }
  } else {
    ptr        = free_nodes;
    free_nodes = free_nodes->next;
  }
  ptr->item = (item_type)0;
  ptr->next = NULL;
  return(ptr);
}

void Free(list *ptr)
/******************************************************************************
  Frees the node pointed to by 'ptr' and stores it in the free nodes list
  for later used.
*******************************************************************************/
{
  ptr->next  = free_nodes;
  free_nodes = ptr;
}

void CreateQueue(queue_type *q)
/******************************************************************************
  Initializes the queue structure to an empty state.
*******************************************************************************/
{
  q->contents = NULL;
  q->number   = 0;
}

void DeleteQueue(queue_type *q)
/******************************************************************************
  Deletes the queue contents and returns it to an empty state.
  The free nodes list is also deallocated and returned to the heap.
*******************************************************************************/
{
  list *ptr, *nxt;

  ptr = First(q); 
  while (ptr != NULL) {
    nxt = Next(ptr);
    free(ptr);
    ptr = nxt;
  }
  ptr = free_nodes; 
  while (ptr != NULL) {
    nxt = Next(ptr);
    free(ptr);
    ptr = nxt;
  }
  q->contents = NULL;
  free_nodes  = NULL;
  q->number   = 0;
}

void FlushQueue(queue_type *q)
/******************************************************************************
  Empties the queue and returns it to an empty state. 
  The freed contents are stored in the free nodes list for later use.
*******************************************************************************/
{
  list *ptr, *nxt;

  ptr = First(q);
  while (ptr != NULL) {
    nxt = Next(ptr);
    Free(ptr);
    ptr = nxt;
  }
  q->contents = NULL;
  q->number   = 0;
}

bool IsEmpty(queue_type *q)
/******************************************************************************
  Returns TRUE if queue is empty, FALSE otherwise.
*******************************************************************************/
{
  if (q->number == 0) {
    return(TRUE);
  } else {
    return(FALSE);
  }
}

long Number(queue_type *q)
/******************************************************************************
  Returns the number in the queue.
*******************************************************************************/
{
  return(q->number);
}

void Insert(queue_type *q, item_type v)
/******************************************************************************
  Inserts the item 'v' into the queue so that the queue is sorted in 
  increasing order of 'v'.  Thus the contents of this queue are removed
  with smallest 'v' first. 
*******************************************************************************/
{
  list *ptr, *nxt; 
  list *new_entry = New();

  new_entry->item = v;
  if (q->contents == NULL) { 
    q->contents = new_entry;
  } else if (v <= q->contents->item) { 
    new_entry->next = q->contents;
    q->contents     = new_entry;
  } else {
    for (ptr = q->contents; ptr != NULL; ptr = nxt) {
      nxt = ptr->next;
      if (nxt == NULL) {
        ptr->next = new_entry;
      } else if (v <= nxt->item) {
        new_entry->next = nxt;
        ptr->next       = new_entry;
        break;
      }
    }
  }
  q->number++;
}

void Remove(queue_type *q, item_type *v)
/******************************************************************************
  Removes the first entry in the queue and returns it in 'v'.
*******************************************************************************/
{
  list *ptr = q->contents;

  if (q->contents != NULL) {
    (*v) = q->contents->item;
    q->contents = q->contents->next;
    q->number--;
    Free(ptr);
  }
}

list *First(queue_type *q)
/******************************************************************************
  Returns a pointer to the first entry in the queue.
*******************************************************************************/
{
  return(q->contents);
}

list *Next(list *l)
/******************************************************************************
  Returns a pointer to the entry following the 'l' entry pointer. 
*******************************************************************************/
{
  return(l->next);
}

#endif
