/* Unit tests for queue.c / queue.h.  Built twice: once as the default FIFO
 * counter (macros) and once with -DSJF_QUEUE (sorted linked list). */
#include "minitest.h"
#include "queue.h"

static void starts_empty(void)
{
  queue_type q;
  CreateQueue(&q);
  CHECK(IsEmpty(&q) == true);
  CHECK(Number(&q) == 0);
}

static void insert_and_remove_track_count(void)
{
  queue_type q;
  item_type  v = 0;
  CreateQueue(&q);
  Insert(&q, 2.0);
  Insert(&q, 3.0);
  Insert(&q, 1.0);
  CHECK(Number(&q) == 3);
  CHECK(IsEmpty(&q) == false);
  Remove(&q, &v);
  CHECK(Number(&q) == 2);
  Remove(&q, &v);
  Remove(&q, &v);
  CHECK(Number(&q) == 0);
  CHECK(IsEmpty(&q) == true);
}

static void flush_empties_queue(void)
{
  queue_type q;
  CreateQueue(&q);
  Insert(&q, 1.0);
  Insert(&q, 2.0);
  FlushQueue(&q);
  CHECK(Number(&q) == 0);
  CHECK(IsEmpty(&q) == true);
  Insert(&q, 7.0);                 /* still usable after a flush */
  CHECK(Number(&q) == 1);
  DeleteQueue(&q);
  CHECK(Number(&q) == 0);
}

#ifdef SJF_QUEUE

static void removes_shortest_job_first(void)
{
  queue_type q;
  item_type  in[]   = {5.0, 1.0, 3.0, 1.0, 9.0, 0.5, 3.0};
  item_type  want[] = {0.5, 1.0, 1.0, 3.0, 3.0, 5.0, 9.0};
  item_type  v;
  int        i, ok = 1;
  CreateQueue(&q);
  for (i = 0; i < 7; i++) Insert(&q, in[i]);
  for (i = 0; i < 7; i++) { Remove(&q, &v); if (v != want[i]) ok = 0; }
  CHECK(ok);
  CHECK(IsEmpty(&q) == true);
  DeleteQueue(&q);
}

static void traversal_is_sorted(void)
{
  queue_type q;
  list      *p;
  double     prev = -1.0;
  int        n = 0, ok = 1;
  CreateQueue(&q);
  Insert(&q, 4.0); Insert(&q, 2.0); Insert(&q, 8.0); Insert(&q, 6.0);
  for (p = First(&q); p != NULL; p = Next(p)) {
    if (p->item < prev) ok = 0;
    prev = p->item;
    n++;
  }
  CHECK(ok);
  CHECK(n == 4);
  DeleteQueue(&q);
}

static void remove_from_empty_is_harmless(void)
{
  queue_type q;
  item_type  v = 123.0;
  CreateQueue(&q);
  Remove(&q, &v);
  CHECK(v == 123.0);
  CHECK(Number(&q) == 0);
}

static void nodes_are_recycled_after_flush(void)
{
  queue_type q;
  item_type  v;
  int        i;
  CreateQueue(&q);
  for (i = 0; i < 100; i++) Insert(&q, (double)(100 - i));
  FlushQueue(&q);
  for (i = 0; i < 100; i++) Insert(&q, (double)i);
  Remove(&q, &v);
  CHECK(v == 0.0);
  CHECK(Number(&q) == 99);
  DeleteQueue(&q);
}

#endif

int main(int argc, char **argv)
{
  MT_MAIN_BEGIN(argc, argv)
  RUN(starts_empty);
  RUN(insert_and_remove_track_count);
  RUN(flush_empties_queue);
#ifdef SJF_QUEUE
  RUN(removes_shortest_job_first);
  RUN(traversal_is_sorted);
  RUN(remove_from_empty_is_harmless);
  RUN(nodes_are_recycled_after_flush);
  MT_MAIN_END("test_queue (SJF)")
#else
  MT_MAIN_END("test_queue (FIFO)")
#endif
}
