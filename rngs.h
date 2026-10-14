/* ----------------------------------------------------------------------- * 
 * Name            : rngs.h  (header file for the library file rngs.c)     * 
 * Author          : Steve Park & Dave Geyer                               * 
 * Language        : ANSI C                                                * 
 * Latest Revision : 10-21-96                                              * 
 * ----------------------------------------------------------------------- */

#if !defined( RNGS_H )
#define RNGS_H

double Random(void);
void   GetSeed(long *x);
void   PutSeed(long x);
void   PlantSeeds(long x);
void   SelectStream(int s);
void   TestRandom(void);

#endif
