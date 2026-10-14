GetFeedback()
{
  SelectStream(2);
  if (Random() < BETA)
    return (1);
  else
    return (0);
}
  
  struct {
    double arrival;         /* next arrival time                        */
    double completion;      /* next completion time                     */
    double current;         /* current time                             */
    double next;            /* next (most imminent) event time          */
    double interval;        /* time width of batch                      */
  } t;
  long   *number;           /* number in the node                       */
  t.current    = 0.0;                           /* set the clock             */
  t.arrival    = t.current + GetInterarrival(); /* schedule first arrival    */
  t.completion = INFINITY;                      /* first event != completion */

      t.next = Min(t.arrival, t.completion);  /* next event time    */
      t.current = t.next;                     /* advance the clock  */

      if (t.current == t.arrival)  {          /* process an arrival */
        aindex++;
        /* store arrival time for this job */
        wait[aindex] = t.arrival;
        number++;
        t.arrival = t.current + GetInterarrival();
        if (number == 1)
          t.completion = t.current + GetService();
        } 
      else {                                  /* process a completion */
        if (GetFeedback())
          t.completion = t.current + GetService();
        else {
          index++;
          number--;
          /* store wait time for this job */
          wait[index] = t.completion - wait[index];

          /* Welford's algorithm to compute batch mean of wait times */
          aw += (wait[index] - aw)/(++j);

          if (number > 0)
            t.completion = t.current + GetService();
          else
            t.completion = INFINITY;
         }
      }
