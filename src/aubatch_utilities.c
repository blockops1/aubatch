
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

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
        //printf("submitted queue is at max limit\n");
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
    //printf("submit function:\n");
    //printQueue(head_job_submitted);
    pthread_cond_signal(&submitted_empty);
    pthread_mutex_unlock(&submitted_mutex);
    return 0;
}

// This is the dispatcher thread. It looks for jobs in the scheduler queue and runs them.
void *tDispatcher(void *received_parameters)
{
    struct Job *newjob = NULL;
    while (hardquit != 0)
    {
        pthread_mutex_lock(&scheduled_mutex);
        if (scheduled_size <= 0)
        {
            //nothing in the scheduled buffer, just wait
            //printf("scheduled queue is empty\n");
            pthread_cond_wait(&scheduled_empty, &scheduled_mutex); // wait until not empty
            if (hardquit == 0)
            {
                //printf("terminating dispatch thread at point 1\n");
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
            //printf("completed queue is at max limit\n");
            pthread_cond_wait(&completed_full, &completed_mutex);
            if (hardquit == 0)
            {
                //rintf("terminating dispatch thread at point 2\n");
                pthread_exit(NULL);
            }
        }
        //printf("sending job %d to moveToCompleted\n", newjob->id);
        moveToCompleted(&newjob);
        //printf("job %d back from moveToCompleted\n", newjob->id);
    }
    if (hardquit == 0)
    {
        //printf("terminating dispatch thread at end\n");
        pthread_exit(NULL);
    }
    return 0;
}

// this is the dipatcher. It returns the job at the head of the scheduled queue
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

// run the job based on its CPU time using fork and execv
int runJob(struct Job **newjob)
{
    // job runs until finished. Keeps a clock and counts every second?
    if (*newjob == NULL)
        return 1;
    //printf("new job arrival time: %f\n", (*newjob)->arrival_time);
    float wait = (*newjob)->arrival_time - process_time();
    //printf("wait time %f\n", wait);
    if (wait > 0)
    {
        printf("Waiting to run job id: %d waiting for %f seconds until job arrival time - cpu idle\n", (*newjob)->id, wait);
        running_job = &waiting_job;
        sleep(wait);
        running_job = NULL;
    }
    //while ((*newjob)->arrival_time > process_time())
    //    ;
    (*newjob)->starting_time = process_time();
    char cpu_time[20] = "";
    sprintf(cpu_time, "%f", (*newjob)->cpu_time);
    pid_t pid = fork();
    if (pid == -1)
    {
        printf("error, failed to fork()");
    }
    else if (pid > 0)
    {
        int status;
        running_job = *newjob;
        //printf("process time before waitpid %f\n", process_time());
        waitpid(pid, &status, 0);
        //printf("process time after waitpid %f\n", process_time());
        running_job = NULL;
        (*newjob)->finish_time = process_time();
    }
    else
    {
        // we are the child
        char *args[] = {"./work_program", cpu_time, NULL};
        execv(args[0], args);
        _exit(EXIT_FAILURE); // exec never returns
    }
    return 0;
}

// put a job at the tail of the completed queue
int moveToCompleted(struct Job **newjob)
{
    struct Job *current = NULL;
    // put new job in completed queue
    pthread_mutex_lock(&completed_mutex);
    if (completed_size >= completed_buffer_size)
    {
        //printf("completed queue is at max limit\n");
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

// print the jobs in the queue starting from the head, moving to the tail. 
int printQueue(struct Job *head)
{
    struct Job *current = head;
    int count = 0;
    printf("Printing queue, current policy is %d\n", currentPolicy);
    while (current != NULL)
    {
        printf("ID: %d  ", current->id);
        printf("Name: %s  ", current->name);
        printf("Priority: %d  ", current->priority);
        printf("CPU time: %f  ", current->cpu_time);
        printf("Arrival Time: %f  ", current->arrival_time);
        printf("Starting time: %f  ", current->starting_time);
        printf("Finish Time: %f  ", current->finish_time);
        if (current->next != NULL)
        {
            printf("Next Job: %d\n", current->next->id);
        }
        else
        {
            printf("Next Job: NULL\n\n");
        }
        // Update current
        current = current->next;
        count++;
        sleep(2);
    }
    return 0;
}

// this traverses the completed queue, generates and prints statistics for total completed jobs
int statisticsCompleted()
{
    struct Job *current = NULL;
    if (head_job_completed == NULL)
    {
        printf("\nNo jobs have been completed\n");
        return 0;
    }
    current = head_job_completed;
    pthread_mutex_lock(&completed_mutex);
    int completed = completed_size;
    double totalTurnaroundTime = 0;
    double totalCPUTime = 0;
    double waitingTime = 0;
    double totalWaitingTime = 0;
    double turnaroundTime = 0;
    double avgTurnaroundTime = 0;
    double avgCPUTime = 0;
    double avgWaitingTime = 0;
    double totalThroughput = 0;
    //traverse to tail gathering statistics
    printf("Individual Job Performance Report\n");
    while (current != NULL)
    {
        turnaroundTime = current->finish_time - current->arrival_time;
        totalTurnaroundTime += turnaroundTime;
        waitingTime = current->starting_time - current->arrival_time;
        totalWaitingTime += waitingTime;
        totalCPUTime += current->cpu_time;
        printf("name: %s priority: %d cpu time: %f arrival: %f start: %f finish %f wait:%f\n", current->name, current->priority, current->cpu_time, current->arrival_time, current->starting_time, current->finish_time, waitingTime);
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

// returns time since start of system running
double process_time()
{
    gettimeofday(&tv2, NULL);
    double time_now = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
    //printf("time now is %f\n", time_now);
    return time_now;
}

// deletes the completed queue. It is an ugly function, but it works
int delete_completed_queue() {
    // purpose is to delete all the items in the  completedqueue. 
    // this is used to clear the completed queue for another 
    // round of performance evaluations.
    // need to figure out how to gracefully free up memory from those jobs in queue
    if (head_job_completed == NULL)  return 0;
    pthread_mutex_lock(&completed_mutex);
    //struct Job *current = head_job_completed;
    //while (current != NULL) {
    //    current = current->next;
    //    free((struct Job *)current);
    //    completed_size--;
    //}
    completed_size = 0;
    head_job_completed = NULL;
    pthread_mutex_unlock(&completed_mutex);
    return 0;
}