/* COMP7500 aubatch main program
 * This is a pthreads based batch scheduling system
 * Author: Rolf Versluis
 * Date: 18 Feb 2021
 */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>


// define constants for initializating data

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

struct Job* head;
enum Policy currentPolicy;
//int jobYime = 0;
pthread_mutex_t jobqueue_mutex;

void *tSchedule(void *);
// declare function to put a job in the queue at correct place
int schedule(struct Job*, enum Policy);
// declare function to pull a job off head of queue and run it
int dispatch();
int reSortJobs(enum Policy);
int reSortedInsert(struct Job **,struct Job *, enum Policy);
int sortedJobInsert(struct Job *, enum Policy);
int printQueue(); 

int main(int argc, char *argv[])
{
    //create the main job queue by initializing the head and tail pointers
    head = NULL;
    //struct Job* tail = NULL;
    struct Job *newjob = NULL;
    enum Policy policy = FCFS;
    currentPolicy = policy;

    if (pthread_mutex_init(&jobqueue_mutex, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    }
    //int success;
    //create the first job
    struct Job job1 = {0};
    job1.id = 1;
    job1.name = "job1";
    job1.priority = 1;
    job1.cpu_time = 10;
    job1.arrival_time = 0;
    job1.next = NULL;

    //create second job
    struct Job job2 = {0};
    job2.id = 2;
    job2.name = "job2";
    job2.priority = 0;
    job2.cpu_time = 20;
    job2.arrival_time = 20;
    job2.next = NULL;

    //create third job
    struct Job job3 = {0};
    job3.id = 3;
    job3.name = "job3";
    job3.priority = 2;
    job3.cpu_time = 5;
    job3.arrival_time = 30;
    job3.next = NULL;



    //current pointer at first job
    newjob = &job1;

    //call scheduler as a pthread job
    pthread_t schedule_tid1;
    struct tParameter schedule_param1 = {newjob, policy};
    struct tParameter* schedule_param1_ptr1 = &schedule_param1;

    if (pthread_create(&schedule_tid1, NULL, tSchedule, (void *)schedule_param1_ptr1)){
        perror("ERROR creating scheduling thread.\n");
    } 
    void* returnval;
      

    // call function for the scheduler thread - this puts jobs in the job queue
    //success = schedule(&head, newjob, policy);
    //printf("success value %d\n", success);
    /*#ifdef DEBUG
        if (head == NULL) printf("head is null\n");
        if (newjob == NULL) printf("newjob is null\n");
        printf("made it back from schedule\n");
        //printf("function main head id: %d\n", head->id);
        printf("function main newjob id: %d\n", newjob->id);
    #endif // DEBUG*/
    newjob = &job2;

    //call scheduler as a pthread job
    pthread_t schedule_tid2;
    struct tParameter schedule_param2 = {newjob, policy};
    struct tParameter* schedule_param2_ptr1 = &schedule_param2;

    if (pthread_create(&schedule_tid2, NULL, tSchedule, (void *)schedule_param2_ptr1)){
        perror("ERROR creating scheduling thread.\n");
    } 

    
    //schedule(&head, newjob, policy);
    newjob = &job3;
    
    //call scheduler as a pthread job
    pthread_t schedule_tid3;
    struct tParameter schedule_param3 = {newjob, policy};
    struct tParameter* schedule_param3_ptr1 = &schedule_param3;

    if (pthread_create(&schedule_tid3, NULL, tSchedule, (void *)schedule_param3_ptr1)){
        perror("ERROR creating scheduling thread.\n");
    } 
    pthread_join(schedule_tid1, &returnval);
    pthread_join(schedule_tid2, &returnval);
    pthread_join(schedule_tid3, &returnval);

    //schedule(&head, newjob, policy);

    printQueue(); 
    //reSortJobs(SJF);
    //printQueue(); 
    //reSortJobs(Priority);
    //printQueue(); 
    // call function for the dispatcher thread - this takes jobs out of the job queue
    //dispatch(head);
    pthread_mutex_destroy(&jobqueue_mutex); 
}

void *tSchedule(void * received_parameters) {
    struct tParameter* thread_parameter = (struct tParameter*) received_parameters;
    struct Job* newjob = (struct Job*)thread_parameter->newjob;
    enum Policy policy = thread_parameter->policy;
    pthread_mutex_lock(&jobqueue_mutex);
    schedule(newjob, policy);
    pthread_mutex_unlock(&jobqueue_mutex);
    return 0;
}

int schedule(struct Job* newjob, enum Policy policy)
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

    // if policy change, traverse queue and arrange all jobs properly. Do not move
    // the current job at head of queue
    if (policy != currentPolicy) {
        reSortJobs(policy);
        #ifdef DEBUG
            printf("policy change, time to sort\n");
        #endif // DEBUG        
    }
    // call an insert function, send it the job and policy
    sortedJobInsert(newjob, policy);
    return 0;
}

// define dispatcher function to pull a job off the head of queue and run it
int dispatch()
{
    // dispatcher runs until finished. Keeps a clock and counts every second?
    if (head == NULL)
        return 1;
    return 0;
}


int reSortJobs(enum Policy policy)
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
        reSortedInsert(&sorted, current, policy);
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
int reSortedInsert(struct Job** sorted,struct Job* newjob, enum Policy policy)
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

/* function to insert a newjob in a list when sorting for realtime. This uses
 * the job queue with global variable head
 */
int sortedJobInsert(struct Job* newjob, enum Policy policy)
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