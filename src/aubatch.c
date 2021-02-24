/* COMP7500 aubatch main program
 * This is a pthreads based batch scheduling system
 * Author: Rolf Versluis
 * Date: 18 Feb 2021
 */

#include "aubatch_scheduler.h"
#include "aubatch_utilities.h"
#include "aubatch.h"

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

    if (pthread_create(&schedule_tid1, NULL, tBatchSchedule, (void *)schedule_param1_ptr1)){
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

    if (pthread_create(&schedule_tid2, NULL, tBatchSchedule, (void *)schedule_param2_ptr1)){
        perror("ERROR creating scheduling thread.\n");
    } 
    pthread_join(schedule_tid1, &returnval);
    pthread_join(schedule_tid2, &returnval);
    printQueue(); 
    //schedule(&head, newjob, policy);
    newjob = &job3;
    policy = SJF;
    
    //call scheduler as a pthread job
    pthread_t schedule_tid3;
    struct tParameter schedule_param3 = {newjob, policy};
    struct tParameter* schedule_param3_ptr1 = &schedule_param3;

    if (pthread_create(&schedule_tid3, NULL, tBatchSchedule, (void *)schedule_param3_ptr1)){
        perror("ERROR creating scheduling thread.\n");
    } 

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

/// definitions start





