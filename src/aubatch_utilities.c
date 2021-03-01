
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include "aubatch_utilities.h"


int submitJob(struct Job* newjob)
{
    struct Job* current = NULL;
    // put new job in submit queue
    //pthread_mutex_lock(&submitted_mutex);
    if (submitted_size >= submitted_buffer_size)
    {
        printf("submitted queue is at max limit\n");
        //pthread_cond_wait(&submitted_full, &submitted_mutex);
    }
    if (head_job_submitted == NULL)
    {
        head_job_submitted = newjob;
        submitted_size++;
        printf("was null, submit queue size: %d\n", submitted_size);
    } else {
        current = head_job_submitted;
        //traverse to tail and add job
        while(current->next != NULL) {
            current = current->next;
        }
        current->next = newjob;
        submitted_size++;
        printf("found tail, submit queue size: %d\n", submitted_size);
    }
    //printQueue(head_job_submitted); 
    //pthread_cond_signal(&submitted_empty);
    //pthread_mutex_unlock(&submitted_mutex);
    return 0;
}

void *tDispatcher(void* received_parameters){
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
        runJob(&newjob);
        // write to completed queue
        pthread_mutex_lock(&completed_mutex);
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
        moveToCompleted(newjob);
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


// define dispatcher function to pull a job off the head of queue and run it
int dispatch()
{
    // dispatcher runs until finished. Keeps a clock and counts every second?
    if (head_job_scheduled == NULL)
        return 1;
    return 0;
}

int printQueue(struct Job* head)
{
    struct Job* current = head;
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