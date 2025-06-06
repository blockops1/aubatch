// aubatch_scheduler.h
#ifndef AUBATCH_SCHEDULER_H
#define AUBATCH_SCHEDULER_H

struct Job;
enum Policy;
struct tParameter;




void *tRunningSchedule(void*);
int submitSchedule(struct Job**);
int runningSchedule(struct Job*);
int runningReSortJobs(struct Job**, enum Policy);
int runningSortedJobInsert(struct Job **, struct Job *, enum Policy);
int printQueue(); 
int print_job(struct Job *);
double waiting_time(struct Job **, struct Job **);
double time_left(struct Job **);



// create a structure to hold job data
struct Job {
    int id;
    char* name;
    int priority; // 0 to 7, with lower numbers being higher priority
    double cpu_time; // this is measured in seconds
    double arrival_time;
    double starting_time;
    double finish_time;
    struct Job *next;
};

enum Policy
{
    FCFS = 0,
    SJF = 1,
    Priority = 2,
    Invalid = 3
};



//structure for calling scheduler thread
struct tParameter {
    struct Job* newjob;
    enum Policy policy;
};

//structure for calculating time
struct timeval  tv1, tv2;

// global variables
struct Job* head_job_submitted;
struct Job* head_job_scheduled;
struct Job* head_job_completed;
struct Job* running_job;
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
pthread_mutex_t completed_mutex;
pthread_cond_t completed_full;
pthread_cond_t completed_empty;
int completed_buffer_size;
int completed_size;
int hardquit;
int softquit;
//int global_job_id;
//int valid;
//const int MAXJOBS = 500;
//struct Job jobs[500];
//char jobnames[500][24];
#endif 