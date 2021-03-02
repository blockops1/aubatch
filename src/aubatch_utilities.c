
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>


#include "aubatch_utilities.h"

int submitJob(struct Job *newjob)
{
    struct Job *current = NULL;
    // put new job in submit queue
    pthread_mutex_lock(&submitted_mutex);
    if (submitted_size >= submitted_buffer_size)
    {
        printf("submitted queue is at max limit\n");
        pthread_cond_wait(&submitted_full, &submitted_mutex);
    }
    if (head_job_submitted == NULL)
    {
        head_job_submitted = newjob;
        submitted_size++;
        printf("was null, submit queue size: %d\n", submitted_size);
    }
    else
    {
        current = head_job_submitted;
        //traverse to tail and add job - refactor to track submit job tail
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = newjob;
        submitted_size++;
        printf("found tail, submit queue size: %d\n", submitted_size);
    }
    //printf("submit function:\n");
    //printQueue(head_job_submitted);
    pthread_cond_signal(&submitted_empty);
    pthread_mutex_unlock(&submitted_mutex);
    return 0;
}

void *tDispatcher(void *received_parameters)
{
    struct Job *newjob = NULL;
    while (hardquit != 0)
    {
        pthread_mutex_lock(&scheduled_mutex);
        if (scheduled_size <= 0)
        {
            //nothing in the submit buffer, just wait
            printf("submit queue is empty\n");
            pthread_cond_wait(&scheduled_empty, &scheduled_mutex); // wait until not empty
            if (hardquit == 0)
            {
                printf("terminating dispatch thread at point 1\n");
                pthread_exit(NULL);
            }
        }
        // read from head of schedule queue and get pointer to item at head of queue
        submitDispatch(&newjob);
        pthread_cond_signal(&scheduled_full); //it is not full anymore
        pthread_mutex_unlock(&scheduled_mutex);
        // work the current job - get data and fill it out
        //printf("sending job %d to runJob\n", newjob->id);
        runJob(&newjob);
        //printf("job %d back from runJob\n", newjob->id);
        // write to completed queue
        if (completed_size > completed_buffer_size)
        {
            printf("completed queue is at max limit\n");
            pthread_cond_wait(&completed_full, &completed_mutex);
            if (hardquit == 0)
            {
                printf("terminating dispatch thread at point 2\n");
                pthread_exit(NULL);
            }
        }
        //printf("sending job %d to moveToCompleted\n", newjob->id);
        moveToCompleted(&newjob);
        //printf("job %d back from moveToCompleted\n", newjob->id);
    }
    if (hardquit == 0)
    {
        printf("terminating dispatch thread at end\n");
        pthread_exit(NULL);
    }
    return 0;
}

int submitDispatch(struct Job **newjob)
{
    // get new job from head of submit queue
    if (head_job_scheduled == NULL)
        return 1; //tried to get a job when none available
    *newjob = head_job_scheduled;
    head_job_scheduled = (*newjob)->next;
    (*newjob)->next = NULL;
    scheduled_size--;
    return 0;
}

// run the job based on its CPU time
int runJob(struct Job **newjob)
{
    // job runs until finished. Keeps a clock and counts every second?
    if (*newjob == NULL)
        return 1;
    int wait = (*newjob)->arrival_time - procTime;
    if (wait > 0) { 
        printf("waiting for %d seconds until job arrival time - cpu idle\n", wait);
        sleep(wait);
        procTime += wait;
    }
    (*newjob)->starting_time = procTime;
    printf("waiting for %d seconds while running job - cpu working\n", (*newjob)->cpu_time);
    sleep((*newjob)->cpu_time);
    procTime += (*newjob)->cpu_time;
    (*newjob)->finish_time = procTime;
    return 0;
}

int moveToCompleted(struct Job **newjob)
{
    struct Job *current = NULL;
    // put new job in completed queue
    pthread_mutex_lock(&completed_mutex);
    if (completed_size >= completed_buffer_size)
    {
        printf("completed queue is at max limit\n");
        pthread_cond_wait(&completed_full, &completed_mutex);
    }
    if (head_job_completed == NULL)
    {
        head_job_completed = *newjob;
        completed_size++;
        printf("was null, completed queue size: %d\n", completed_size);
    }
    else
    {
        current = head_job_completed;
        //traverse to tail and add job
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = *newjob;
        completed_size++;
        printf("found tail, completed queue size: %d\n", completed_size);
    }
    //printQueue(head_job_completed);
    pthread_cond_signal(&completed_empty);
    pthread_mutex_unlock(&completed_mutex);
    return 0;
}

int printQueue(struct Job *head)
{
    struct Job *current = head;
    int count = 0;
    printf("Printing queue, current policy is %d\n", currentPolicy);
    while (current != NULL)
    {
        printf("ID: %d\n", current->id);
        printf("Name: %s\n", current->name);
        printf("Priority: %d\n", current->priority);
        printf("CPU time: %d\n", current->cpu_time);
        printf("Arrival Time: %d\n", current->arrival_time);
        printf("Starting time: %d\n", current->starting_time);
        printf("Finish Time: %d\n", current->finish_time);
        if (current->next != NULL)
        {
            printf("Next Job: %d\n\n", current->next->id);
        }
        else
        {
            printf("Next Job: NULL\n\n");
        }
        // Update current
        current = current->next;
        count++;
    }
    return 0;
}

