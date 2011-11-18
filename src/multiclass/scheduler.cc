/*
 * =====================================================================================
 *
 *       Filename:  test_sched.cc
 *
 *    Description:  A basic interval scheduler.

 *
 *        Version:  1.0
 *        Created:  11/17/2011 02:31:06 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "scheduler.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <signal.h>
#include <sys/time.h>

/*
 * I made this global so that the alarm handler
 * function can change the interval if necessary
 */
struct itimerval it;

void
start_scheduler(void (*s_handler)(int), unsigned int stepsize, unsigned int delaytime)
{
  char buffer[BUFSIZ];

  it.it_value.tv_sec     = delaytime;       /* start in 1 second */
  it.it_value.tv_usec    = 0;
  it.it_interval.tv_sec  = stepsize;       /* repeat every 5 seconds */
  it.it_interval.tv_usec = 0;

  if ( s_handler != NULL ) {
    signal(SIGALRM, s_handler); /* Install the handler */
  }
  setitimer(ITIMER_REAL, &it, NULL);/* turn on interval timer */
}
