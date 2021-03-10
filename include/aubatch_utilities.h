//aubatch_utilities.h
#ifndef AUBATCH_UTILITIES_H
#define AUBATCH_UTILITIES_H
#include "aubatch_scheduler.h"

int submitJob(struct Job*);
void *tDispatcher(void*);
int runJob(struct Job**);
int submitDispatch(struct Job **);
int moveToCompleted(struct Job**);
int statisticsCompleted();
float process_time();

//int printQueue(); 

// global variables
extern struct Job *head_job_submitted;
extern struct Job *head_job_scheduled;
extern struct Job *head_job_completed;
extern struct Job* running_job;
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
extern pthread_mutex_t completed_mutex;
extern pthread_cond_t completed_full;
extern pthread_cond_t completed_empty;
extern int completed_buffer_size;
extern int completed_size;
extern int hardquit;
extern int softquit;
extern time_t procTime;

#endif