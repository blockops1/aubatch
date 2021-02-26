// aubatch_scheduler.h
#ifndef AUBATCH_SCHEDULER_H
#define AUBATCH_SCHEDULER_H

struct Job;
enum Policy;
struct tParameter;




void *tRunningSchedule(void*);
int submitSchedule(struct Job**);
int runningSchedule(struct Job*);
int runningReSortJobs(enum Policy);
int runningReSortedInsert(struct Job **,struct Job *, enum Policy);
int runningSortedJobInsert(struct Job *, enum Policy);
int printQueue(); 


// create a structure to hold job data
struct Job {
    int id;
    char* name;
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

// global variables
struct Job* head_job_submitted;
struct Job* head_job_scheduled;
struct Job* head_job_completed;
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