// aubatch.h

#ifndef AUBATCH_H
#define AUBATCH_H

#include "aubatch_scheduler.h"
#include "aubatch_utilities.h"

// global variables
extern struct Job* head_job_submitted;
extern struct Job* head_job_scheduled;
extern struct Job* head_job_completed;
extern enum Policy currentPolicy;
extern int policyChange;
extern pthread_mutex_t submitted_mutex;
extern pthread_cond_t submitted_full;
extern pthread_cond_t submitted_empty;
extern int submitted_buffer_size;
extern int submitted_size;
extern pthread_mutex_t scheduled_mutex;
extern pthread_cond_t scheduled_empty;
extern pthread_cond_t scheduled_full;
extern int scheduled_buffer_size;
extern int scheduled_size;
extern int hardquit;
extern int softquit;


#endif 