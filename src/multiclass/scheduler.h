#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <signal.h>

sighandler_t
start_scheduler(void (*s_handler)(int), unsigned int stepsize = 300, unsigned int delay = 1);

#endif
