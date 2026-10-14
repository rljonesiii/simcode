/******************************************************************************
*                                                                             *
*  misc.h                                                                     *
*                                                                             *
*  Version:  30-April-97                                                      *
*            2026: C23 -- bool/true/false are now language keywords, so the   *
*            old FALSE/TRUE enum is gone; error() is [[noreturn]].            *
*                                                                             *
*******************************************************************************/

#ifndef MISC_H
#define MISC_H

#define max(x,y)   (((x) > (y)) ? (x) : (y))
#define min(x,y)   (((x) < (y)) ? (x) : (y))

/*
*  When a fatal error occurs call function 'error'
*  with one of the standard messages...
*/
#define MEM_ERR    "Error: out of memory.\n"
#define FIL_ERR    "Error: unable to access file.\n"
/*
*  or pass in your own.  error() prints the message to stderr and
*  exits with EXIT_FAILURE.
*/
[[noreturn]] void error(const char *message);

#endif /* MISC_H */
