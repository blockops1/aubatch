
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>

#include "aubatch_utilities.h"
struct Job waiting_job = {0, "waiting", 0, 0, 0, 0, 0, NULL};

// this can be called from a driver program to put a job in submit queue
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
        //printf("was null, submit queue size: %d\n", submitted_size);
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
        //printf("found tail, submit queue size: %d\n", submitted_size);
    }
    printf("submit function:\n");
    printQueue(head_job_submitted);
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
        printf("sending job %d to runJob\n", newjob->id);
        runJob(&newjob);
        printf("job %d back from runJob\n", newjob->id);
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
        printf("sending job %d to moveToCompleted\n", newjob->id);
        moveToCompleted(&newjob);
        printf("job %d back from moveToCompleted\n", newjob->id);
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
    // get new job from head of scheduled queue
    // if arrival time is after current process_time, go to the next one in queue
    if (head_job_scheduled == NULL)
        return 1; //tried to get a job when none available

    *newjob = head_job_scheduled;
    struct Job *trailjob = head_job_scheduled;
    float nextJobTime = (*newjob)->arrival_time;
    // check if the job arrival time is not yet. If so, traverse until job found
    while ((*newjob) != NULL && (*newjob)->arrival_time > process_time())
    {
        if ((*newjob)->arrival_time < nextJobTime)
        {
            nextJobTime = (*newjob)->arrival_time;
        }
        trailjob = *newjob;
        (*newjob) = (*newjob)->next;
    }
    // pull job out of queue
    // if NULL, whole list was traversed, did not find any earlier job, will be waiting
    if ((*newjob) == NULL)
    {
        *newjob = head_job_scheduled;
    }
    // if this is true, head job arrival time <= process_time(), no traversing was done
    if ((*newjob) == head_job_scheduled)
    {
        head_job_scheduled = (*newjob)->next;
        (*newjob)->next = NULL;
    }
    // some job in the middle was found. Yank it out and run it
    if (trailjob != NULL && (*newjob) != NULL)
    {
        trailjob->next = (*newjob)->next;
        (*newjob)->next = NULL;
    }
    scheduled_size--;
    return 0;
}

// run the job based on its CPU time
int runJob(struct Job **newjob)
{
    // job runs until finished. Keeps a clock and counts every second?
    if (*newjob == NULL)
        return 1;
    printf("new job arrival time: %f\n", (*newjob)->arrival_time);
    float wait = (*newjob)->arrival_time - process_time();
    printf("wait time %f\n", wait);
    if (wait > 0)
    {
        //printf("Waiting to run job id: %d waiting for %f seconds until job arrival time - cpu idle\n", (*newjob)->id, wait);
        running_job = &waiting_job;
        sleep(wait - wait/10);
        running_job = NULL;
    }
    while ((*newjob)->arrival_time > process_time())
        ;
    (*newjob)->starting_time = process_time();
    //printf("");
    //char *id;
    //sprintf(id, "%d", (*newjob)->id);
    //char starting_time[17];
    //sprintf(starting_time, "%f", (*newjob)->starting_time);
    //char *cpu_time;
    //cpu_time = NULL;
    //sprintf(cpu_time, "%f", (*newjob)->cpu_time);
    //printf("%s", cpu_time);
    //printf("./workprogram %s %s %s\n", id, starting_time, cpu_time);
    //printf("Running job id: %d waiting for %d seconds while running job - cpu working\n", (*newjob)->id, (*newjob)->cpu_time);
    //pid_t parent = getpid();
   //pid_t pid = fork();
    //if (pid == -1)
    //{
    //    printf("error, failed to fork()");
    //}
    //else if (pid > 0)
    //{
    //    int status;
    //    running_job = *newjob;
    //    waitpid(pid, &status, 0);
    //    running_job = NULL;
    //}
    //else
    //{
        // we are the child
    //    char *args[] = {"./work_program", cpu_time, NULL};
    //    execv(args[0], args);
    //    _exit(EXIT_FAILURE); // exec never returns
    //}
    sleep((*newjob)->cpu_time);
    (*newjob)->finish_time = process_time();
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
        //printf("was null, completed queue size: %d\n", completed_size);
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
        //printf("found tail, completed queue size: %d\n", completed_size);
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
        printf("CPU time: %f\n", current->cpu_time);
        printf("Arrival Time: %f\n", current->arrival_time);
        printf("Starting time: %f\n", current->starting_time);
        printf("Finish Time: %f\n", current->finish_time);
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

// this traverses the completed queue, generates and prints statistics for total completed jobs
int statisticsCompleted()
{
    struct Job *current = NULL;
    if (head_job_completed == NULL)
    {
        printf("No jobs have been completed\n");
        return 0;
    }
    current = head_job_completed;
    pthread_mutex_lock(&completed_mutex);
    int completed = completed_size;
    float totalTurnaroundTime = 0;
    float totalCPUTime = 0;
    float waitingTime = 0;
    float totalWaitingTime = 0;
    float turnaroundTime = 0;
    float avgTurnaroundTime = 0;
    float avgCPUTime = 0;
    float avgWaitingTime = 0;
    float totalThroughput = 0;
    //traverse to tail gathering statistics
    printf("Individual Job Performance Report\n");
    while (current != NULL)
    {
        turnaroundTime = current->finish_time - current->arrival_time;
        totalTurnaroundTime += turnaroundTime;
        waitingTime = current->starting_time - current->arrival_time;
        totalWaitingTime += waitingTime;
        totalCPUTime += current->cpu_time;
        printf("id: %d cpu time:%f arrival: %f start: %f finish %f wait:%f\n", current->id, current->cpu_time, current->arrival_time, current->starting_time, current->finish_time, waitingTime);
        current = current->next;
    }
    pthread_mutex_unlock(&completed_mutex);
    avgTurnaroundTime = totalTurnaroundTime / completed;
    avgCPUTime = totalCPUTime / completed;
    avgWaitingTime = totalWaitingTime / completed;
    totalThroughput = 1 / avgTurnaroundTime;
    printf("\nTotal Performance Report\n");
    printf("Total number of job submitted: \t%d\n", completed);
    printf("Average turnaround time: \t%f seconds\n", avgTurnaroundTime);
    printf("Average CPU time: \t\t%f seconds\n", avgCPUTime);
    printf("Average waiting time: \t\t%f seconds\n", avgWaitingTime);
    printf("Throughput: \t\t\t%f jobs/second\n\n", totalThroughput);

    return 0;
}

float process_time()
{
    // returns time since start of system running
    float time_now = (clock() - procTime) / CLOCKS_PER_SEC;
    printf("time now is %f\n", time_now);
    return time_now;
}