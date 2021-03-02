/* COMP7500 aubatch main program
 * This is a pthreads based batch scheduling system
 * Author: Rolf Versluis
 * Date: 18 Feb 2021
 */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include "aubatch.h"

int splitInput(char *, void *);
size_t string_parser(const char *, char ***);

int main(int argc, char *argv[])
{
    //create the main job queue by initializing the head pointer
    head_job_submitted = NULL;
    head_job_scheduled = NULL;
    head_job_completed = NULL;
    struct Job *newjob = NULL;
    enum Policy policy = Priority;
    currentPolicy = policy;
    policyChange = 1;
    submitted_buffer_size = 10;
    submitted_size = 0;
    scheduled_buffer_size = 10;
    scheduled_size = 0;
    completed_buffer_size = 10;
    completed_size = 0;
    hardquit = 1;
    softquit = 1;
    procTime = 0;

    //int success;
    //create the first job
    struct Job job1 = {0};
    job1.id = 1;
    job1.name = "job1";
    job1.priority = 1;
    job1.cpu_time = 3;
    job1.arrival_time = 0;
    job1.next = NULL;

    //create second job
    struct Job job2 = {0};
    job2.id = 2;
    job2.name = "job2";
    job2.priority = 0;
    job2.cpu_time = 5;
    job2.arrival_time = 10;
    job2.next = NULL;

    //create third job
    struct Job job3 = {0};
    job3.id = 3;
    job3.name = "job3";
    job3.priority = 2;
    job3.cpu_time = 4;
    job3.arrival_time = 2;
    job3.next = NULL;

    if (pthread_mutex_init(&submitted_mutex, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&scheduled_mutex, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }
    if (pthread_mutex_init(&completed_mutex, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }

    //call scheduler as a pthread job - this will keep running until told to quit
    pthread_t schedule_tid1;
    void *schedule_param1_ptr1 = NULL;
    if (pthread_create(&schedule_tid1, NULL, tRunningSchedule, schedule_param1_ptr1))
    {
        perror("ERROR creating scheduling thread.\n");
    }

    //call dispatcher as a pthread job - this will keep running until told to quit
    pthread_t dispatch_tid1;
    void *dispatch_param1_ptr1 = NULL;
    if (pthread_create(&dispatch_tid1, NULL, tDispatcher, dispatch_param1_ptr1))
    {
        perror("ERROR creating dispatching thread.\n");
    }
    void *returnval = NULL;

    // start the parser running
    char *line = NULL; /* forces getline to allocate with malloc */
    size_t len = 0;    /* ignored when line = NULL */
    ssize_t read;
    printf("welcome to blockops job scheduled v0.12\n");
    printf("\nEnter string below, type 'help' for command options, [ctrl + d] to quit\n");
    printf(">");

    while ((read = getline(&line, &len, stdin)) != -1)
    {

        if (read > 0)
        {
            printf("\n  read %zd chars from stdin, allocated %zd bytes for line : %s\n", read, len, line);
            //char s[] = line;
            char **word_array = NULL;

            size_t n = string_parser(line, &word_array);

            for (size_t i = 0; i < n; i++)
                puts(word_array[i]);

            //after done using word_array
            for (size_t i = 0; i < n; i++)
            free(word_array[i]);
            free(word_array);
        }

        printf(">");
    }

    free(line); /* free memory allocated by getline */

    //current pointer at first job
    newjob = &job1;
    //printf("calling submit for job1\n");
    //pthread_mutex_lock(&submitted_mutex);
    //printf("got lock for job1\n");
    submitJob(newjob);
    //printf("returned from submit for job1\n");
    //pthread_cond_signal(&submitted_empty);
    //printf("Submitted Queue after job 1:\n");
    //printQueue(head_job_submitted);
    //pthread_mutex_unlock(&submitted_mutex);

    newjob = &job2;
    //printf("calling submit for job2\n");
    //pthread_mutex_lock(&submitted_mutex);
    submitJob(newjob);
    //pthread_cond_signal(&submitted_empty);
    //printf("Submitted Queue after job 2:\n");
    //printQueue(head_job_submitted);
    //pthread_mutex_unlock(&submitted_mutex);

    //policy = SJF;
    //policyChange = 0;
    //currentPolicy = policy;

    newjob = &job3;
    //pthread_mutex_lock(&submitted_mutex);
    submitJob(newjob);
    //pthread_cond_signal(&submitted_empty);
    //printf("Submitted Queue after job 3:\n");
    //printQueue(head_job_submitted);
    //pthread_mutex_unlock(&submitted_mutex);

    //printf("driver program sleep 30 seconds\n");
    //sleep(30);

    //quit the thread
    //pthread_cond_signal(&scheduled_full);
    pthread_cond_signal(&submitted_empty);
    hardquit = 0;
    pthread_join(schedule_tid1, returnval);
    //pthread_join(dispatch_tid1, returnval);
    pthread_mutex_destroy(&submitted_mutex);
    pthread_mutex_destroy(&scheduled_mutex);
    pthread_mutex_destroy(&completed_mutex);
    printf("\nSubmitted Queue:\n");
    printQueue(head_job_submitted);
    printf("\nScheduled Queue:\n");
    printQueue(head_job_scheduled);
    printf("\nCompleted Queue:\n");
    printQueue(head_job_completed);
    statisticsCompleted();
}

/// functions start

int splitInput(char *input, void *returnArray)
{
    return 0;
}

size_t string_parser(const char *input, char ***word_array)
// from https://stackoverflow.com/questions/43015843/parsing-a-string-in-c-to-individual-words
{
    size_t n = 0;
    const char *p = input;

    while (*p)
    {
        while (isspace((unsigned char)*p))
            ++p;
        n += *p != '\0';
        while (*p && !isspace((unsigned char)*p))
            ++p;
    }

    if (n)
    {
        size_t i = 0;

        *word_array = malloc(n * sizeof(char *));

        p = input;

        while (*p)
        {
            while (isspace((unsigned char)*p))
                ++p;
            if (*p)
            {
                const char *q = p;
                while (*p && !isspace((unsigned char)*p))
                    ++p;

                size_t length = p - q;

                (*word_array)[i] = (char *)malloc(length + 1);

                strncpy((*word_array)[i], q, length);
                (*word_array)[i][length] = '\0';

                ++i;
            }
        }
    }

    return n;
}