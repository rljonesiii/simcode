/****************************************************************
  File:         sort.c
  Author:       Rob Jones
  Purpose:      Modified bubble sort algorithm.
  Input:        array[first..last] of (double) numbers               
  Output:       array sorted in increasing order               
  Last Updated: 2/3/97
****************************************************************/

#include "sort.h"

void Sort(double *list, long first, long last)
{
  long   i, j;
  double tmp;

  for (i = first; i < last; i++) {
    for (j = i+1; j <= last; j++) {
      if (list[i] > list[j]) {
        tmp = list[i];
        list[i] = list[j];
        list[j] = tmp;
      }
    }
  }
}

