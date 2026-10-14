/*
*  qnsim.h  
*/
#ifndef QNSIM_H
#define QNSIM_H

void ProcessArrival(long i, double time, double *future_events);
void ServiceJob(long i, double time, double *future_events);

#endif /* QNSIM_H */
