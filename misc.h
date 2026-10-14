/******************************************************************************
*                                                                             *
*  mics.c                                                                     *
*                                                                             *
*  Version:  30-April-97                                                      *
*                                                                             *
*******************************************************************************/

#ifndef _misc_h
#define _misc_h

typedef enum {FALSE = 0, TRUE = 1} bool;

#define max(x,y)   (((x) > (y)) ? (x) : (y))
#define min(x,y)   (((x) < (y)) ? (x) : (y))

/*
*  When fatal error occurs call function 'error'
*  with stnd messages...
*/
#define MEM_ERR    0
#define FIL_ERR    1
/*
*  or pass in your own,
*  error will then call exit(0).
*/
void error(char *);

#endif /* _misc_h */
