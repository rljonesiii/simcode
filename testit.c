
#include "queue.h"


main()
{
  queue_type queue;
  item_type x;

  CreateQueue(&queue);

  printf("queue %s empty.\n",(IsEmpty(&queue) == TRUE)?"is":"is not");
  printf("put 2\n");Insert(&queue,2);
  printf("put 3\n");Insert(&queue,3);
  printf("put 1\n");Insert(&queue,1);
  printf("put 4\n");Insert(&queue,4);
  printf("queue %s empty.\n",(IsEmpty(&queue) == TRUE)?"is":"is not");
  printf("#queue = %ld\n",Number(&queue));

  Remove(&queue, &x); printf("took %f\n",x);
  Remove(&queue, &x); printf("took %f\n",x);

  printf("queue %s empty.\n",(IsEmpty(&queue) == TRUE)?"is":"is not");
  printf("#queue = %ld\n",Number(&queue));

  Remove(&queue, &x); printf("took %f\n",x);
  Remove(&queue, &x); printf("took %f\n",x);

  printf("queue %s empty.\n",(IsEmpty(&queue) == TRUE)?"is":"is not");
  printf("#queue = %ld\n",Number(&queue));

  printf("put 5\n");Insert(&queue,5);
  printf("put 2\n");Insert(&queue,2);
  printf("put 6\n");Insert(&queue,6);

  printf("queue %s empty.\n",(IsEmpty(&queue) == TRUE)?"is":"is not");

  Remove(&queue, &x); printf("took %f\n",x);
  Remove(&queue, &x); printf("took %f\n",x);
  Remove(&queue, &x); printf("took %f\n",x);

  printf("queue %s empty.\n",(IsEmpty(&queue) == TRUE)?"is":"is not");

  DeleteQueue(&queue);
}

