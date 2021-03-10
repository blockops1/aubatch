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
#include <time.h>
#include "aubatch.h"

int main(int argc, char *argv[])
{
    //create the main job queue by initializing the head pointer
    head_job_submitted = NULL;
    head_job_scheduled = NULL;
    head_job_completed = NULL;
    enum Policy policy = Priority;
    currentPolicy = policy;
    policyChange = 1;
    submitted_buffer_size = MAXJOBS;
    submitted_size = 0;
    scheduled_buffer_size = MAXJOBS;
    scheduled_size = 0;
    completed_buffer_size = MAXJOBS;
    completed_size = 0;
    hardquit = 1;
    softquit = 1;
    procTime = time(NULL);
    global_job_id = 0;

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
    printf("Welcome to BlockOps's batch job scheduler Version 1.0\n");
    printf("\nType 'help' to find more about AUbatch commands.\n");
    printf(">");

    while ((read = getline(&line, &len, stdin)) != -1)
    {

        if (read > 1)
        {
            //printf("\n  read %zd chars from stdin, allocated %zd bytes for line : %s\n", read, len, line);
            //char s[] = line;
            char **word_array = NULL;

            size_t n = string_parser(line, &word_array);

            //this prints out the array
            //for (size_t i = 0; i < n; i++)
            //    puts(word_array[i]);

            // take action on the command here: send to command subroutine
            commandAction(n, &word_array);

            //after done using word_array
            for (size_t i = 0; i < n; i++)
                free(word_array[i]);
            free(word_array);
        }
        if (running_job > 0)
        {
            printf("job running: %s", running_job->name);
        }
        printf(">");
    }

    free(line); /* free memory allocated by getline */

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

int commandAction(int n, char ***word_array)
{
    int i, result = 0;
    for (i = 0; cmdtable[i].name; i++)
    {
        //printf("commandAction input: %s\n", *word_array[0]);
        //printf("commandAction table: %s\n", cmdtable[i].name);
        if (strcmp(*word_array[0], cmdtable[i].name) == 0)
        {
            //assert(cmdtable[i].func != NULL);
            //printf("commandAction match! %s, %s\n", *word_array[0], cmdtable[i].name);
            result = cmdtable[i].func(n, *word_array);
        }
    }
    return result;
}

int cmd_helpmenu(int n, char **a)
{
    (void)n;
    (void)a;
    //printf("cmd_helpmenu %s\n", a[0]);

    showmenu("AUbatch help menu", helpmenu);
    return 0;
}
/*
 * Display menu information
 */
void showmenu(const char *name, const char *x[])
{
    int i = 0;
    while (x[i] != NULL)
    {
        printf("%s\n", x[i]);
        i++;
    }
    printf("\n");
}

/*
 * The quit command.
 */
int cmd_quit(int nargs, char **args)
{
    int jobs_qty = submitted_size + scheduled_size;
    if (running_job != NULL)
        jobs_qty++;

    if (jobs_qty == 0 || (nargs == 2 && (strcmp(args[1], "force") == 0 || strcmp(args[1], "f") == 0)))
    {
        if (jobs_qty == 0) {
            printf("No jobs in queue, no jobs running, quitting gracefully.\n");
        } else {
        printf("Quitting and terminating queued and running jobs\n");
        cmd_queue_size(nargs, args);
        //print_job(running_job);
        printf("currently running job \nname: %s priority: %d cpu_time: %f starting_time: %f\n", running_job->name, running_job->priority, running_job->cpu_time, running_job->starting_time);
        }
        
        hardquit = 0;
        pthread_cond_signal(&submitted_empty);
        pthread_cond_signal(&scheduled_empty);
        pthread_cond_signal(&completed_empty);
        pthread_cond_signal(&submitted_full);
        pthread_cond_signal(&scheduled_full);
        pthread_cond_signal(&scheduled_empty);
        statisticsCompleted();
        //pthread_join(dispatch_tid1, returnval);
        pthread_mutex_destroy(&submitted_mutex);
        pthread_mutex_destroy(&scheduled_mutex);
        pthread_mutex_destroy(&completed_mutex);
        exit(0);
    }
    if (jobs_qty > 0)
    {
        printf("There are still jobs running. AUbatch will not quit while jobs are queued and running.\nTry again later.\n");
        cmd_queue_size(nargs, args);
        softquit = 0;
        printf("Type \"quit force\" or \"q f\" to force quit with jobs still running.\n");
        return 0;
    }
    return 0;
}

int cmd_queue_size(int n, char **a)
{
    (void)n;
    (void)a;
    if (running_job != NULL)
    {
        printf("Current running job name: %s\n", running_job->name);
    }
    printf("Submitted queue size: %d\n", submitted_size);
    //printQueue(head_job_submitted);
    printf("Scheduled queue size: %d\n", scheduled_size);
    //printQueue(head_job_scheduled);
    printf("Completed queue size: %d\n", completed_size);
    //printQueue(head_job_completed);

    printf("\n");
    return 0;
}

int cmd_list_jobs(int n, char **a)
{
    (void)n;
    (void)a;
    printf("Total number of jobs in the queue:: %d\n", submitted_size + scheduled_size);
    printf("Scheduling Policy: ");
    if (currentPolicy == FCFS)
        printf("FCFS-First Come First Served\n");
    if (currentPolicy == SJF)
        printf("SJF-Shortest Job First\n");
    if (currentPolicy == Priority)
        printf("Priority-Lowest Priority Jobs First\n");
    printf("Name\tCPU_Time\tPri\tArrival_time\tProgress\n");
    if (running_job != NULL)
    {
        printf("%s", running_job->name);
        printf("\t%f", running_job->cpu_time);
        printf("\t%d", running_job->priority);
        printf("\t%f\tRunning\n", running_job->arrival_time);
    }
    print_queue_job_info(head_job_submitted);
    print_queue_job_info(head_job_scheduled);
    print_queue_job_info(head_job_completed);
    printf("\n");
    return 0;
}

int print_queue_job_info(struct Job *head)
{
    struct Job *current = head;
    int count = 0;
    //printf("Printing queue, current policy is %d\n", currentPolicy);
    while (current != NULL)
    {
        printf("%s\t", current->name);
        //printf("%d  ", current->id);
        printf("%f\t", current->cpu_time);
        printf("%d\t", current->priority);
        printf("%f\t", current->arrival_time);
        if (head == head_job_submitted)
        {
            printf("Submit Queue\n");
        }
        if (head == head_job_scheduled)
        {
            printf("Scheduled Queue\n");
        }
        if (head == head_job_completed)
        {
            printf("Completed Queue\n");
        }
        // Update current
        current = current->next;
        count++;
    }
    return 0;
}

int cmd_policy_change(int nargs, char **args)
{
    char *policy_compare = NULL;
    if (currentPolicy == FCFS)
    {
        policy_compare = "fcfs";
        printf("FCFS is current policy\n");
    }
    if (currentPolicy == SJF)
    {
        policy_compare = "sjf";
        printf("SJF is current policy\n");
    }
    if (currentPolicy == Priority)
    {
        policy_compare = "priority";
        printf("Priority is current policy\n");
    }
    if (strcmp(args[0], policy_compare) == 0)
    {
        printf("Scheduling policy not changed\n");
    }
    else
    {
        if (strcmp(args[0], "fcfs") == 0)
        {
            currentPolicy = FCFS;
            printf("Changing Policy to FCFS\n");
        }
        if (strcmp(args[0], "sjf") == 0)
        {
            currentPolicy = SJF;
            printf("Changing Policy to SJF\n");
        }
        if (strcmp(args[0], "priority") == 0)
        {
            currentPolicy = Priority;
            printf("Changing Policy to Priority\n");
        }
        policyChange = 0;
    }
    return 0;
}

int cmd_run_job(int nargs, char **args)
{
    //printf("back from commandAction\n");
    if ((strcmp(args[0], "run") == 0) || (strcmp(args[0], "r") == 0))
    {
        //printf("Entering run area\n");
        //valid = 0;
        new_time = 0;
        new_priority = 0;
        if (nargs < 3 || nargs > 4)
        {
            printf("incorrect input. Usage: run <job> <time> <pri>\n");
            return 1;
        }
        if (nargs == 4)
        {
            new_priority = atoi(args[3]);
            if (new_priority < 0)
            {
                printf("<priority> must be a non-negative number\n");
                return 1;
            }
        }
        new_time = atof(args[2]);
        if (new_time < 0)
        {
            printf("<time> must be a positive number\n");
            return 1;
        }
        //printf("valid is %d\n", valid);
        // if valid == 0 create a new job

        struct Job *newjob;
        //char name[sizeof(word_array[1])];
        //strcpy(name, word_array[1]);
        ++global_job_id;
        strcpy(jobnames[global_job_id], args[1]);
        jobs[global_job_id].id = global_job_id;
        jobs[global_job_id].name = jobnames[global_job_id];
        jobs[global_job_id].priority = new_priority;
        jobs[global_job_id].cpu_time = new_time;
        jobs[global_job_id].arrival_time = process_time();
        jobs[global_job_id].next = NULL;

        //current pointer at first job
        newjob = &jobs[global_job_id];
        //print_job(newjob);
        //printf("job %d arrived at about %f\n", job1.id, job1.arrival_time);
        submitJob(newjob);
    }
    return 0;
}