// aubatch_scheduler.c

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include "aubatch_scheduler.h"

// function definitions

// thread to check the submitted queue for new jobs and place them in scheduled queue
void *tRunningSchedule(void *received_parameters)
{
    // this is a continually running thread. It checks the submitted queue for new jobs
    // and places them in the scheduled queue.
    struct Job *newjob = NULL;
    while (hardquit != 0)
    {
        pthread_mutex_lock(&submitted_mutex);
        if (submitted_size <= 0)
        {
            //nothing in the submit buffer, just wait
            //printf("submit queue is empty\n");
            pthread_cond_wait(&submitted_empty, &submitted_mutex); // wait until not empty
            if (hardquit == 0)
            {
                //printf("terminating schedule thread at point 1\n");
                pthread_exit(NULL);
            }
        }
        // read from submit queue and get pointer to item at head of queue
        submitSchedule(&newjob);
        //print_job(newjob);
        pthread_cond_signal(&submitted_full); //it is not full anymore
        pthread_mutex_unlock(&submitted_mutex);

        // write to scheduled queue
        pthread_mutex_lock(&scheduled_mutex);
        if (scheduled_size > scheduled_buffer_size)
        {
            //printf("scheduled queue is at max limit\n");
            pthread_cond_wait(&scheduled_full, &scheduled_mutex);
            if (hardquit == 0)
            {
                //printf("terminating schedule thread at point 2\n");
                pthread_exit(NULL);
            }
        }
        runningSchedule(newjob);
        pthread_cond_signal(&scheduled_empty);
        pthread_mutex_unlock(&scheduled_mutex);
    }
    if (hardquit == 0)
    {
        printf("terminating schedule thread at end\n");
        pthread_exit(NULL);
    }
    return 0;
}

// get new job from head of submit queue
int submitSchedule(struct Job **newjob)
{
    if (head_job_submitted == NULL)
        return 1; //tried to get a job when none available
    *newjob = head_job_submitted;
    head_job_submitted = (*newjob)->next;
    (*newjob)->next = NULL;
    submitted_size--;
    return 0;
}

// check policy, then put a new job at the correct place in scheduled queue
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
        runningReSortJobs(&head_job_scheduled, currentPolicy);
        policyChange = 1;
#ifdef DEBUG
        printf("policy change, time to sort, current job id: %d\n", head_job_scheduled->id);
#endif // DEBUG
    }
    // call an insert function, send it the job and policy. do not change running job
    runningSortedJobInsert(&head_job_scheduled, newjob, currentPolicy);
    scheduled_size++;
    return 0;
}

// insertion sort algorithm on linked list to put job in queue at correct location
int runningReSortJobs(struct Job **head_ref, enum Policy policy)
{
    // this is insertion sort 
    // Initialize sorted linked list. this uses global head queue
    // got this function from https://www.geeksforgeeks.org/insertion-sort-for-singly-linked-list/
    struct Job *sorted = NULL;
    // Traverse the given linked list and insert every
    // node to sorted
    printf("resorting jobs with new policy ");
    if (policy == FCFS) printf("FCFS\n");
    if (policy == SJF) printf("SJF\n");
    if (policy == Priority) printf("Priority\n");
    struct Job *current = *head_ref;
    struct Job *next = NULL;
    while (current != NULL)
    {
        // Store next for next iteration
        next = current->next;
        // have to set next to NULL or get all sorts of weird loops
        current->next = NULL;
        // insert current in sorted linked list
        //printf("inserting job name %s\n", current->name);
        //sleep(2);
        runningSortedJobInsert(&sorted, current, policy);
        current = next;
    }
    // Update head_ref to point to sorted linked list
    *head_ref = sorted;
    return 0;
}


/* function to insert a newjob in a list in correct place. This uses
 * the job queue with global variable head. 
 * refactor with comparator function because this is ugly
 */
int runningSortedJobInsert(struct Job **head_ref, struct Job *newjob, enum Policy policy)
{
    struct Job *current;
    /* Special case for the head end */
    if (*head_ref == NULL)
    {
        *head_ref = newjob;
        return 0;
    }

    // if head node needs to be replaced do this section
    if (policy == FCFS && (*head_ref)->arrival_time > newjob->arrival_time) {
        newjob->next = *head_ref;
        *head_ref = newjob;
        return 0;
    }
    if (policy == SJF && (*head_ref)->cpu_time > newjob->cpu_time) {
        newjob->next = *head_ref;
        *head_ref = newjob;
        return 0;
    }
    if (policy == Priority && (*head_ref)->priority > newjob->priority) {
        newjob->next = *head_ref;
        *head_ref = newjob;
        //printf("inserted at head of queue\n");
        return 0;
    }

    /* Locate the node before the point of insertion */
    current = *head_ref;
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

// print the details of a single job
int print_job(struct Job *testjob){
    printf("%d\t%s\t%d\t%f\t%f \n", testjob->id, testjob->name, testjob->priority, testjob->cpu_time, testjob->arrival_time);
    return 0;
}

// calculate approximate waiting time of a new job
double waiting_time(struct Job **head_ref, struct Job **newjob) {
    // traverse queue and find job, add up cpu_time, return time
    if (*head_ref == NULL) {
        //printf("head ref is null in waiting time");
        return 0;
    }
    double sum = 0;
    struct Job *current = *head_ref;
    while (current != NULL) {
        if (currentPolicy == FCFS && (*newjob)->arrival_time < current->arrival_time) break;
        if (currentPolicy == SJF && (*newjob)->cpu_time < current->cpu_time) break;
        if (currentPolicy == Priority && (*newjob)->priority < current->priority) break;
        sum += current->cpu_time;
        //printf("waiting time sum: %f", sum);
        current = current->next;
    }
    return sum;
}

// calculate how much time is left in the queues
double time_left(struct Job **head_ref) {
    // traverse queue and find job, add up cpu_time, return time
    if (*head_ref == NULL) {
        //printf("head ref is null in waiting time");
        return 0;
    }
    double sum = 0;
    struct Job *current = *head_ref;
    while (current != NULL) {
        sum += current->cpu_time;
        //printf("waiting time sum: %f", sum);
        current = current->next;
    }
    return sum;
}