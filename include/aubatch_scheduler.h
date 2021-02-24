// aubatch_scheduler.h
#ifndef AUBATCH_SCHEDULER_H
#define AUBATCH_SCHEDULER_H
struct Job;
enum Policy;
struct tParameter;

void *tBatchSchedule(void *);
void *tRunningSchedule(void *);
// declare function to put a job in the queue at correct place for batches
// change job at head of job queue
int batchSchedule(struct Job*, enum Policy); 
// declare function to put a job in the queue at correct place while running
// do not change job at head of job queue
int runningSchedule(struct Job*, enum Policy);
// declare function to pull a job off head of queue and run it
int batchReSortJobs(enum Policy);
int batchReSortedInsert(struct Job **,struct Job *, enum Policy);
int runningReSortJobs(enum Policy);
int runningReSortedInsert(struct Job **,struct Job *, enum Policy);
int batchSortedJobInsert(struct Job *, enum Policy);
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