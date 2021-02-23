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
    struct Job* head;
    struct Job* newjob;
    enum Policy policy;
};

enum Policy currentPolicy;

void *tSchedule(void *);
// declare function to put a job in the queue at correct place
int schedule(struct Job**, struct Job*, enum Policy);
// declare function to pull a job off head of queue and run it
int dispatch(struct Job **);
int sortJobs(struct Job **, enum Policy);
int sortedInsert(struct Job **, struct Job *, enum Policy);
int printQueue(struct Job **); 

int main(int argc, char *argv[])
{
    //create the main job queue by initializing the head and tail pointers
    struct Job *head = NULL;
    //struct Job* tail = NULL;
    struct Job *newjob = NULL;
    enum Policy policy = FCFS;
    currentPolicy = policy;
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
    struct tParameter schedule_param1 = {head, newjob, policy};
    struct tParameter* schedule_param1_ptr1 = &schedule_param1;

    if (pthread_create(&schedule_tid1, NULL, tSchedule, (void *)schedule_param1_ptr1)){
        perror("ERROR creating scheduling thread.\n");
    } 
    void* returnval;
    pthread_join(schedule_tid1, &returnval);
    head = (struct Job *)returnval;

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
    struct tParameter schedule_param2 = {head, newjob, policy};
    struct tParameter* schedule_param2_ptr1 = &schedule_param2;

    if (pthread_create(&schedule_tid2, NULL, tSchedule, (void *)schedule_param2_ptr1)){
        perror("ERROR creating scheduling thread.\n");
    } 
    pthread_join(schedule_tid2, &returnval);
    head = (struct Job *)returnval;
    
    //schedule(&head, newjob, policy);
    newjob = &job3;
    
    //call scheduler as a pthread job
    pthread_t schedule_tid3;
    struct tParameter schedule_param3 = {head, newjob, policy};
    struct tParameter* schedule_param3_ptr1 = &schedule_param3;

    if (pthread_create(&schedule_tid3, NULL, tSchedule, (void *)schedule_param3_ptr1)){
        perror("ERROR creating scheduling thread.\n");
    } 
    pthread_join(schedule_tid3, &returnval);
    head = (struct Job *)returnval;

    //schedule(&head, newjob, policy);

    printQueue(&head); 

    // call function for the dispatcher thread - this takes jobs out of the job queue
    //dispatch(head);
}

void *tSchedule(void * received_parameters) {
    struct tParameter* thread_parameter = (struct tParameter*) received_parameters;
    struct Job* head = (struct Job*)thread_parameter->head;
    struct Job* newjob = (struct Job*)thread_parameter->newjob;
    enum Policy policy = thread_parameter->policy;
    schedule(&head, newjob, policy);
    return head;
}

int schedule(struct Job** head, struct Job* newjob, enum Policy policy)
{
    // define scheduler function to put a job at the correct place in queue
    // head of queue is next job to run
    // scheduler is called whenever a job is created
    //SJF - find shortest job, pull out of queue
    //Priority - find highest firstest job, pull out of queue, need previous
    // traverse queue until shortest job found
    //if empty queue put first at head
    if (*head == NULL)
    {
        *head = newjob;
        return 0;
    }
    // if policy change, traverse queue and arrange all jobs properly
    if (policy != currentPolicy) {
        sortJobs(head, policy);
        #ifdef DEBUG
            printf("policy change, time to sort\n");
        #endif // DEBUG        
    }
    // call a general insert function, send it the queue and job
    sortedInsert(head, newjob, policy);
    return 0;
}

// define dispatcher function to pull a job off the head of queue and run it
int dispatch(struct Job **head)
{
    // dispatcher runs until finished. Keeps a clock and counts every second?
    if (*head == NULL)
        return 1;
    //struct Job *previous = *head;
    //struct Job *current = *head;
    return 0;
}


int sortJobs(struct Job **head, enum Policy policy)
{
    // this is insertion sort
    // Initialize sorted linked list
    struct Job *sorted = NULL;

    // Traverse the given linked list and insert every
    // node to sorted
    struct Job *current = *head;
    while (current != NULL)
    {
        // Store next for next iteration
        struct Job *next = current->next;

        // insert current in sorted linked list
        sortedInsert(&sorted, current, policy);

        // Update current
        current = next;
    }

    // Update head_ref to point to sorted linked list
    *head = sorted;
    return 0;
}

/* function to insert a newjob in a list. Note that this 
  function expects a pointer to head as this can modify the 
  head of the input linked list (similar to push())*/
int sortedInsert(struct Job** head, struct Job* newjob, enum Policy policy)
{
    struct Job* current;
    /* Special case for the head end */
    if (*head == NULL)
    {
        *head = newjob;
        return 0;
    }
    if (policy == FCFS && (*head)->arrival_time > newjob->arrival_time)
    {
        newjob->next = *head;
        *head = newjob;
        return 0;
    }
        if (policy == SJF && (*head)->cpu_time > newjob->cpu_time)
    {
        newjob->next = *head;
        *head = newjob;
        return 0;
    }
        if (policy == Priority && (*head)->priority > newjob->priority)
    {
        newjob->next = *head;
        *head = newjob;
        return 0;
    }


    /* Locate the node before the point of insertion */
    current = *head;
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

int printQueue(struct Job** head) {
    struct Job* current = *head;
    printf("Printing queue, current schedule is %d\n", currentPolicy);
    while (current != NULL)
    {
        printf("ID: %d\n", current->id);
        printf("Name: %s\n", current->name);
        printf("Priority: %d\n", current->priority);
        printf("CPU time: %d\n", current->cpu_time);
        printf("Arrival Time: %d\n", current->arrival_time);
        printf("Starting time: %d\n", current->starting_time);
        printf("Finish Time: %d\n\n", current->finish_time);
        // Update current
        current = current->next;
    }
    return 0;
}