// aubatch.h

#ifndef AUBATCH_H
#define AUBATCH_H
#include "aubatch_scheduler.h"
#include "aubatch_utilities.h"
// global variables
struct Job *head_job_submitted;
struct Job *head_job_scheduled;
struct Job *head_job_completed;
enum Policy currentPolicy;
int policyChange;
pthread_mutex_t submitted_mutex;
pthread_cond_t submitted_full;
pthread_cond_t submitted_empty;
int submitted_buffer_size;
int submitted_size;
pthread_mutex_t scheduled_mutex;
pthread_cond_t scheduled_empty;
pthread_cond_t scheduled_full;
int scheduled_buffer_size;
int scheduled_size;
int hardquit;
int softquit;


#endif 