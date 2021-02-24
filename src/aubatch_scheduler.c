// aubatch_scheduler.c

#include "aubatch_scheduler.h"


#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

// function definitions

void *tRunningSchedule(void *received_parameters)
{
    // this is a continually running thread. It checks the submitted queue for new jobs
    // and places them in the scheduled queue.
    struct Job* newjob = NULL;
    //enum Policy policy;
    while (hardquit != 0)
    {
        pthread_mutex_lock(&submitted_mutex);
        if(submitted_size <= 0) {
            //nothing in the submit buffer, just wait
            printf("submit queue is empty");
            pthread_cond_wait(&submitted_empty, &submitted_mutex);
        }
        // read from submit queue and take item at head of queue
        submitSchedule(newjob);
        pthread_cond_signal(&submitted_full);
        pthread_mutex_unlock(&submitted_mutex);     

        // write to scheduled queue
        pthread_mutex_lock(&scheduled_mutex);
        if (scheduled_size >= scheduled_buffer_size)
        {
            printf("scheduled queue is at max limit");
            pthread_cond_wait(&scheduled_full, &scheduled_mutex);
        }
        runningSchedule(newjob);
        
        pthread_cond_signal(&scheduled_empty);
        pthread_mutex_unlock(&scheduled_mutex);
    }
    return 0;
}

int submitSchedule(struct Job* newjob) {
    // get new job from head of submit queue
    if (head_job_submitted == NULL) return 1;
    newjob = head_job_submitted;
    head_job_submitted = newjob->next;
    submitted_size--;
    return 0;
}

int runningSchedule(struct Job *newjob)
{
    // define scheduler function to put a job at the correct place in queue
    // head of queue is next job to run. This is for batch jobs
    // scheduler is called whenever a job is created
    //SJF - find shortest job, pull out of queue
    //Priority - find highest firstest job, pull out of queue, need previous
    // traverse queue until shortest job found
    //if empty queue put first at head

    if (head_job_scheduled == NULL)
    {
        head_job_scheduled = newjob;
        scheduled_size++;
        return 0;
    }

    // if policy change, traverse queue and arrange all jobs properly.
    // This is for batch scheduling, and does not change job at head of queue
    if (policyChange == 0)
    {
        runningReSortJobs(currentPolicy);
        policyChange = 1;
#ifdef DEBUG
        printf("policy change, time to sort, current job id: %d\n", head->id);
#endif // DEBUG
    }
    // call an insert function, send it the job and policy. do not change running job
    runningSortedJobInsert(newjob, currentPolicy);
    scheduled_size++;
    return 0;
}

int runningReSortJobs(enum Policy policy)
{
    // this is insertion sort but does not change job at head of queue
    // Initialize sorted linked list. this uses global head queue
    struct Job *sorted = NULL;

    // Traverse the given linked list and insert every
    // node to sorted
    struct Job *current = head_job_scheduled;
    while (current != NULL)
    {
        // Store next for next iteration
        struct Job *next = current->next;
        // insert current in sorted linked list
        runningReSortedInsert(&sorted, current, policy);
        // if next is null traverse sorted list to end, change tail next to NULL
        // this if section is a hack to fix tail->next not being assigned NULL
        // need to solve root of problem
        //if (next == NULL) {
        //    while (current != current->next) {
        //        current = current->next;
        //    }
        //    current->next = NULL;
        //}
        // Update current
        current = next;
    }
    // Update head_ref to point to sorted linked list
    head_job_scheduled = sorted;
    return 0;
}

/* function to insert a newjob in a list when sorting. Note that this 
  function expects a pointer to head as this can modify the 
  head of the input linked list (similar to push())*/
// need to fix new tail NULL pointer
int runningReSortedInsert(struct Job **sorted, struct Job *newjob, enum Policy policy)
{
    struct Job *current;
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
        if (current->next == NULL)
            newjob->next = NULL;
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
        if (current->next == NULL)
            newjob->next = NULL;
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
        if (current->next == NULL)
            newjob->next = NULL;
        current->next = newjob;
    }
    return 0;
}
/* function to insert a newjob in a list in correct place. This uses
 * the job queue with global variable head. It does not insert a job at the head
 * refactor with comparator function when time
 */
int runningSortedJobInsert(struct Job *newjob, enum Policy policy)
{
    struct Job *current;
    /* Special case for the head end */
    if (head_job_scheduled == NULL)
    {
        head_job_scheduled = newjob;
        return 0;
    }

    /* Locate the node before the point of insertion */
    current = head_job_scheduled;
    if (policy == FCFS)
    {
        while (current->next != NULL &&
               current->next->arrival_time < newjob->arrival_time)
        {
            current = current->next;
        }
        newjob->next = current->next;
        if (current->next == NULL)
            newjob->next = NULL;
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
        if (current->next == NULL)
            newjob->next = NULL;
        current->next = newjob;
    }
    if (policy == Priority)
    {
        while (current->next != NULL &&
               current->next->priority < newjob->priority)
        {
            current = current->next;
        }
        newjob->next = current->next; // why is this not working when next is NULL?
        if (current->next == NULL)
            newjob->next = NULL;
        current->next = newjob;
    }
    return 0;
}
