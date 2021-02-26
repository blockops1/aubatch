
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include "aubatch_utilities.h"

void *tDispatcher(void* received_parameters){
    return 0;
}

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