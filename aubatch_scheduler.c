// aubatch_scheduler.c

#include "aubatch_scheduler.h"

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

// global variables

struct Job* head;
enum Policy currentPolicy;
pthread_mutex_t jobqueue_mutex;


// function definitions

void *tBatchSchedule(void * received_parameters) {
    struct tParameter* thread_parameter = (struct tParameter*) received_parameters;
    struct Job* newjob = (struct Job*)thread_parameter->newjob;
    enum Policy policy = thread_parameter->policy;
    pthread_mutex_lock(&jobqueue_mutex);
    batchSchedule(newjob, policy);
    pthread_mutex_unlock(&jobqueue_mutex);
    return 0;
}


void *tRunningSchedule(void * received_parameters) {
    struct tParameter* thread_parameter = (struct tParameter*) received_parameters;
    struct Job* newjob = (struct Job*)thread_parameter->newjob;
    enum Policy policy = thread_parameter->policy;
    pthread_mutex_lock(&jobqueue_mutex);
    runningSchedule(newjob, policy);
    pthread_mutex_unlock(&jobqueue_mutex);
    return 0;
}

int batchSchedule(struct Job* newjob, enum Policy policy)
{
    // define scheduler function to put a job at the correct place in queue
    // head of queue is next job to run. This is for batch jobs
    // scheduler is called whenever a job is created
    //SJF - find shortest job, pull out of queue
    //Priority - find highest firstest job, pull out of queue, need previous
    // traverse queue until shortest job found
    //if empty queue put first at head
    
    if (head == NULL)
    {
        head = newjob;
        return 0;
    }

    // if policy change, traverse queue and arrange all jobs properly. 
    // This is for batch scheduling, and changes job at head of queue
    if (policy != currentPolicy) {
        batchReSortJobs(policy);
        #ifdef DEBUG
            printf("policy change, time to sort, current job id: %d\n", head->id);
        #endif // DEBUG   
        currentPolicy = policy;     
    }
    // call an insert function, send it the job and policy. can change head of queue
    batchSortedJobInsert(newjob, policy);
    return 0;
}


int runningSchedule(struct Job* newjob, enum Policy policy)
{
    // define scheduler function to put a job at the correct place in queue
    // head of queue is next job to run. This is for batch jobs
    // scheduler is called whenever a job is created
    //SJF - find shortest job, pull out of queue
    //Priority - find highest firstest job, pull out of queue, need previous
    // traverse queue until shortest job found
    //if empty queue put first at head
    
    if (head == NULL)
    {
        head = newjob;
        return 0;
    }

    // if policy change, traverse queue and arrange all jobs properly. 
    // This is for batch scheduling, and does not change job at head of queue
    if (policy != currentPolicy) {
        runningReSortJobs(policy);
        #ifdef DEBUG
            printf("policy change, time to sort, current job id: %d\n", head->id);
        #endif // DEBUG   
        currentPolicy = policy;     
    }
    // call an insert function, send it the job and policy. do not change running job
    runningSortedJobInsert(newjob, policy);
    return 0;
}



int batchReSortJobs(enum Policy policy)
{
    // this is insertion sort and changes job at head of queue
    // Initialize sorted linked list. this uses global head queue
    struct Job *sorted = NULL;

    // Traverse the given linked list and insert every
    // node to sorted
    struct Job *current = head;
    while (current != NULL)
    {
        // Store next for next iteration
        struct Job *next = current->next;
        // insert current in sorted linked list
        batchReSortedInsert(&sorted, current, policy);
        // if next is null traverse sorted list to end, change tail next to NULL
        if (next == NULL) {
            while (current != current->next) {
                current = current->next;
            }
            current->next = NULL;
        }
        // Update current
        current = next;
    }
    // Update head_ref to point to sorted linked list
    head = sorted;
    return 0;
}


/* function to insert a newjob in a list when sorting. Note that this 
  function expects a pointer to head as this can modify the 
  head of the input linked list (similar to push())*/
int batchReSortedInsert(struct Job** sorted,struct Job* newjob, enum Policy policy)
{
    struct Job* current;
    /* Special case for the head end */
    if (*sorted == NULL)
    {
        *sorted = newjob;
        return 0;
    }


    if (policy == FCFS && (*sorted)->arrival_time > newjob->arrival_time)
    {
        newjob->next = *sorted;
        *sorted = newjob;
        return 0;
    }
        if (policy == SJF && (*sorted)->cpu_time > newjob->cpu_time)
    {
        newjob->next = head;
        head = newjob;
        return 0;
    }
        if (policy == Priority && (*sorted)->priority > newjob->priority)
    {
        newjob->next = *sorted;
        *sorted = newjob;
        return 0;
    }

    
    /* Locate the node before the point of insertion */
    current = *sorted;
    if (policy == FCFS)
    {
        while (current->next != NULL &&
               current->next->arrival_time < newjob->arrival_time)
        {
            current = current->next;
        }
        newjob->next = current->next;
        current->next = newjob;
    }
        if (policy == SJF)
    {
        while (current->next != NULL &&
               current->next->cpu_time < newjob->cpu_time)
        {
            current = current->next;
        }
        newjob->next = current->next;
        current->next = newjob;
    }
        if (policy == Priority)
    {
        while (current->next != NULL &&
               current->next->priority < newjob->priority)
        {
            current = current->next;
        }
        newjob->next = current->next;
        current->next = newjob;
    }
    return 0;
}


int runningReSortJobs(enum Policy policy)
{
    // this is insertion sort but does not change job at head of queue
    // Initialize sorted linked list. this uses global head queue
    struct Job *sorted = NULL;

    // Traverse the given linked list and insert every
    // node to sorted
    struct Job *current = head;
    while (current != NULL)
    {
        // Store next for next iteration
        struct Job *next = current->next;
        // insert current in sorted linked list
        runningReSortedInsert(&sorted, current, policy);
        // if next is null traverse sorted list to end, change tail next to NULL
        if (next == NULL) {
            while (current != current->next) {
                current = current->next;
            }
            current->next = NULL;
        }
        // Update current
        current = next;
    }
    // Update head_ref to point to sorted linked list
    head = sorted;
    return 0;
}

/* function to insert a newjob in a list when sorting. Note that this 
  function expects a pointer to head as this can modify the 
  head of the input linked list (similar to push())*/
int runningReSortedInsert(struct Job** sorted,struct Job* newjob, enum Policy policy)
{
    struct Job* current;
    /* Special case for the head end */
    if (*sorted == NULL)
    {
        *sorted = newjob;
        return 0;
    }
    
    /* Locate the node before the point of insertion */
    current = *sorted;
    if (policy == FCFS)
    {
        while (current->next != NULL &&
               current->next->arrival_time < newjob->arrival_time)
        {
            current = current->next;
        }
        newjob->next = current->next;
        current->next = newjob;
    }
        if (policy == SJF)
    {
        while (current->next != NULL &&
               current->next->cpu_time < newjob->cpu_time)
        {
            current = current->next;
        }
        newjob->next = current->next;
        current->next = newjob;
    }
        if (policy == Priority)
    {
        while (current->next != NULL &&
               current->next->priority < newjob->priority)
        {
            current = current->next;
        }
        newjob->next = current->next;
        current->next = newjob;
    }
    return 0;
}

/* function to insert a newjob in a list when sorting for batches. This uses
 * the job queue with global variable head
 */
int batchSortedJobInsert(struct Job* newjob, enum Policy policy)
{
    struct Job* current;
    /* Special case for the head end */
    if (head == NULL)
    {
        head = newjob;
        return 0;
    }

    if (policy == FCFS && (head)->arrival_time > newjob->arrival_time)
    {
        newjob->next = head;
        head = newjob;
        return 0;
    }
        if (policy == SJF && (head)->cpu_time > newjob->cpu_time)
    {
        newjob->next = head;
        head = newjob;
        return 0;
    }
        if (policy == Priority && (head)->priority > newjob->priority)
    {
        newjob->next = head;
        head = newjob;
        return 0;
    }

    /* Locate the node before the point of insertion */
    current = head;
    if (policy == FCFS)
    {
        while (current->next != NULL &&
               current->next->arrival_time < newjob->arrival_time)
        {
            current = current->next;
        }
        newjob->next = current->next;
        current->next = newjob;
    }
        if (policy == SJF)
    {
        while (current->next != NULL &&
               current->next->cpu_time < newjob->cpu_time)
        {
            current = current->next;
        }
        newjob->next = current->next;
        current->next = newjob;
    }
        if (policy == Priority)
    {
        while (current->next != NULL &&
               current->next->priority < newjob->priority)
        {
            current = current->next;
        }
        newjob->next = current->next;
        current->next = newjob;
    }
    return 0;
}


/* function to insert a newjob in a list when sorting for realtime. This uses
 * the job queue with global variable head. It does not insert a job at the head
 */
int runningSortedJobInsert(struct Job* newjob, enum Policy policy)
{
    struct Job* current;
    /* Special case for the head end */
    if (head == NULL)
    {
        head = newjob;
        return 0;
    }

    /* Locate the node before the point of insertion */
    current = head;
    if (policy == FCFS)
    {
        while (current->next != NULL &&
               current->next->arrival_time < newjob->arrival_time)
        {
            current = current->next;
        }
        newjob->next = current->next;
        current->next = newjob;
    }
        if (policy == SJF)
    {
        while (current->next != NULL &&
               current->next->cpu_time < newjob->cpu_time)
        {
            current = current->next;
        }
        newjob->next = current->next;
        current->next = newjob;
    }
        if (policy == Priority)
    {
        while (current->next != NULL &&
               current->next->priority < newjob->priority)
        {
            current = current->next;
        }
        newjob->next = current->next;
        current->next = newjob;
    }
    return 0;
}

