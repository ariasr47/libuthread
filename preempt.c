#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "preempt.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

struct sigaction alarm_handler;

sigset_t signal_set; 

void
catch_alarm (int sig)
{
    uthread_yield();
}

void preempt_disable(void)
{
	/* TODO Phase 4 */
	sigprocmask(SIG_BLOCK, &signal_set, NULL);
}

void preempt_enable(void)
{
	/* TODO Phase 4 */
	sigprocmask(SIG_UNBLOCK, &signal_set, NULL);
}

void preempt_start(void)
{
	
	memset (&alarm_handler, 0, sizeof (alarm_handler));
	alarm_handler.sa_handler = catch_alarm;
	sigaction(SIGVTALRM, &alarm_handler, NULL);
	
	sigemptyset(&signal_set);
	sigaddset(&signal_set, SIGVTALRM);

	sigprocmask(SIG_BLOCK, &signal_set, NULL);

	/* Set an alarm to go off in a little while. */
	struct itimerval old, new;
	new.it_interval.tv_usec = 10000;
	new.it_interval.tv_sec = 0;
	new.it_value.tv_usec = 10000;
	new.it_value.tv_sec = 0;
	setitimer (ITIMER_VIRTUAL, &new, &old);
}

