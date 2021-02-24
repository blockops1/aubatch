// aubatch_scheduler.h
#ifndef AUBATCH_SCHEDULER_H
#define AUBATCH_SCHEDULER_H
#include "aubatch_utilities.h"
#include "aubatch.h"
struct Job;
enum Policy;
struct tParameter;

// global variables
extern struct Job *head_job_submitted;
extern struct Job *head_job_scheduled;
extern struct Job *head_job_completed;
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


void *tRunningSchedule(void *);
int submitSchedule(struct Job*);
int runningSchedule(struct Job*);
int runningReSortJobs(enum Policy);
int runningReSortedInsert(struct Job **,struct Job *, enum Policy);
int runningSortedJobInsert(struct Job *, enum Policy);


// create a structure to hold job data
struct Job {
    int id;
    char *name;
    int priority; // 0 to 7, with lower numbers being higher priority
    int cpu_time; // this is measured in seconds
    int arrival_time;
    int starting_time;
    int finish_time;
    struct Job *next;
};

enum Policy
{
    FCFS = 0,
    SJF = 1,
    Priority = 2
};

//structure for calling scheduler thread
struct tParameter {
    struct Job* newjob;
    enum Policy policy;
};



#endif 