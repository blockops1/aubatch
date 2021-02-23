#include "aubatch_scheduler.h"
#include "aubatch_utilities.h"

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


// define dispatcher function to pull a job off the head of queue and run it
int dispatch()
{
    // dispatcher runs until finished. Keeps a clock and counts every second?
    if (head == NULL)
        return 1;
    return 0;
}


int printQueue() {
    struct Job* current = head;
    int count = 0;
    printf("Printing queue, current policy is %d\n", currentPolicy);
    while (current != NULL && count < 3)
    {
        printf("ID: %d\n", current->id);
        printf("Name: %s\n", current->name);
        printf("Priority: %d\n", current->priority);
        printf("CPU time: %d\n", current->cpu_time);
        printf("Arrival Time: %d\n", current->arrival_time);
        printf("Starting time: %d\n", current->starting_time);
        printf("Finish Time: %d\n", current->finish_time);
        if (current->next != NULL) {
        printf("Next Job: %d\n\n", current->next->id);
        } else {
            printf("Next Job: NULL\n\n");
        }
        // Update current
        current = current->next;
        count++;
    }
    return 0;
}