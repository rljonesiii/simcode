/* ----------------------------------------------------------------------- * 
 * Name            : rngs.h  (header file for the library file rngs.c)     * 
 * Author          : Steve Park & Dave Geyer                               * 
 * Language        : ANSI C                                                * 
 * Latest Revision : 10-21-96                                              * 
 * ----------------------------------------------------------------------- */

#if !defined( _RNGS_ )
#define _RNGS_

double Random(void);
void   GetSeed(long *x);
void   PutSeed(long x);
void   PlantSeeds(long x);
void   SelectStream(int s);
void   TestRandom(void);

#endif
