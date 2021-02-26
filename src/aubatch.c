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
#include <unistd.h>
#include "aubatch.h"



int main(int argc, char *argv[])
{
    //create the main job queue by initializing the head pointer
    head_job_submitted = NULL;
    head_job_scheduled = NULL;
    head_job_completed = NULL;
    struct Job* newjob = NULL;
    enum Policy policy = FCFS;
    currentPolicy = policy;
    policyChange = 1;
    submitted_buffer_size = 3;
    submitted_size = 0;
    scheduled_buffer_size = 3;
    scheduled_size = 0;
    hardquit = 1;
    softquit = 1;


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

    if (pthread_mutex_init(&submitted_mutex, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    }
    if (pthread_mutex_init(&scheduled_mutex, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    }

    //call scheduler as a pthread job - this will keep running until told to quit
    pthread_t schedule_tid1;
    void* schedule_param1_ptr1 = NULL;
    if (pthread_create(&schedule_tid1, NULL, tRunningSchedule, schedule_param1_ptr1)){
        perror("ERROR creating scheduling thread.\n");
    } 
    void* returnval = NULL;
      

    //current pointer at first job
    newjob = &job1;
    submitJob(newjob);
    printf("job 1 submitted sleep 3 seconds\n");
    sleep(1);
    printf("Submitted Queue after job 1:\n");
    printQueue(head_job_submitted); 

    printf("sleep 3 seconds\n");
    sleep(1);


    newjob = &job2;
    submitJob(newjob);
    printf("job 2 submitted sleep 3 seconds\n");
    sleep(1);
    printf("Submitted Queue:\n");
    printQueue(head_job_submitted); 

    printf("sleep 3 seconds\n");
    sleep(1);


    policy = SJF;
    policyChange = 0;
    currentPolicy = policy;

    printf("sleep 3 seconds\n");
    sleep(1);
    printf("Scheduled Queue:\n");
    printQueue(head_job_scheduled); 

    newjob = &job3;
    submitJob(newjob);
    printf("sleep 3 seconds\n");
    sleep(1);
    printf("Submitted Queue:\n");
    printQueue(head_job_submitted); 

    printf("sleep 3 seconds\n");
    sleep(1);




    printf("sleep 10 seconds\n");
    sleep(10);

    //quit the thread
    //pthread_cond_signal(&scheduled_full);
    //pthread_cond_signal(&submitted_empty);
    //hardquit = 0;
    pthread_join(schedule_tid1, returnval);
    pthread_mutex_destroy(&submitted_mutex); 
    pthread_mutex_destroy(&scheduled_mutex); 
}

/// definitions start





