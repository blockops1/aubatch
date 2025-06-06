/* COMP7500 aubatch main program
 * This is a pthreads based batch scheduling system.
 * The jobs are created as nodes that are moved from one queue to another.
 * There are three queues:
 * Submitted is for jobs waiting to be scheduled. Jobs are added to the tail.
 * Scheduled are jobs that are scheduled using the current policy.
 * The job head of the scheduled queue is removed and run.
 * When the job is done running, it gets placed at the tail of the completed queue.
 * The completed queue maintains a list of jobs run in order of execution. When
 * statistics are required, they are generated from the completed queue.
 *  
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
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
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
    //procTime = clock();
    global_job_id = 0;
    // this is the time the program starts running
    gettimeofday(&tv1, NULL);

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

    // start the parser running in a loop until done
    char *line = NULL; /* forces getline to allocate with malloc */
    size_t len = 0;    /* ignored when line = NULL */
    ssize_t read;
    printf("Welcome to BlockOps's batch job scheduler Version 1.0\n");
    printf("\nType 'help' to find more about AUbatch commands.\n");
    printf(">");

    while (softquit == 1 && (read = getline(&line, &len, stdin)) != -1)
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
    // if softquit wait for queues and running to stop
    int loop_counter = 0;
    while (running_job != NULL || scheduled_size > 0 || submitted_size > 0)
    {
        float time_to_go = time_left(&head_job_scheduled);
        if (loop_counter < 1)
        {
            printf("\nQuitting when no jobs left to run.\n");
            printf("Running job is %s, queue size is %d\n", running_job->name, scheduled_size);
            printf("Approximate time left is %f seconds.\n", time_to_go + running_job->cpu_time);
        }
        sleep(time_to_go);
        loop_counter++;
    }

    //this is the soft quit path, when all jobs are done running
    hardquit = 0;
    pthread_cond_signal(&submitted_empty);
    pthread_cond_signal(&scheduled_empty);
    pthread_cond_signal(&completed_empty);
    pthread_cond_signal(&submitted_full);
    pthread_cond_signal(&scheduled_full);
    pthread_cond_signal(&scheduled_empty);
    pthread_join(dispatch_tid1, returnval);
    pthread_mutex_destroy(&submitted_mutex);
    pthread_mutex_destroy(&scheduled_mutex);
    pthread_mutex_destroy(&completed_mutex);
    statisticsCompleted();
}

/// functions start

// parses the string provided, splits it into an array on spaces
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

// general command action function, chooses from array in aubatch.h
int commandAction(int n, char ***word_array)
// this function structure was provided by Dr. Qin
{
    int i, result = 0;
    for (i = 0; cmdtable[i].name; i++)
    {
        if (strcmp(*word_array[0], cmdtable[i].name) == 0)
        {
            result = cmdtable[i].func(n, *word_array);
        }
    }
    return result;
}

// prints the helpmenu from the array in aubatch.h
int cmd_helpmenu(int n, char **a)
// this function structure was provided by Dr. Qin
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
 * The quit command. There is a soft quit and hard quit function
 */
int cmd_quit(int nargs, char **args)
{
    int jobs_qty = submitted_size + scheduled_size;
    if (running_job != NULL)
        jobs_qty++;

    if (jobs_qty == 0 || (nargs == 2 && (strcmp(args[1], "force") == 0 || strcmp(args[1], "f") == 0)))
    {
        if (jobs_qty == 0)
        {
            printf("No jobs in queue, no jobs running, quitting gracefully.\n");
        }
        else
        {
            printf("Quitting and terminating queued and running jobs\n");
            cmd_queue_size(nargs, args);
            //print_job(running_job);
            printf("currently running job \nname: %s priority: %d cpu_time: %f starting_time: %f\n", running_job->name, running_job->priority, running_job->cpu_time, running_job->starting_time);
        }

        // this is the path if the "quit force" option is chosen. Shut it down now.
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
        printf("There are still jobs running. AUbatch will quit when jobs are done running.\n");
        //cmd_queue_size(nargs, args);
        softquit = 0;
        printf("Next time can type \"quit force\" or \"q f\" to force quit with jobs still running.\n");
        return 0;
    }
    return 0;
}

// prints the number of jobs in each queue
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

// lists the jobs, this is run just after a command line "run" command is entered
int cmd_list_jobs(int n, char **a)
{
    (void)n;
    (void)a;
    printf("Total number of jobs in the queue: %d\n", submitted_size + scheduled_size);
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
    //print_queue_job_info(head_job_completed);
    printf("\n");
    return 0;
}

// function to print summary of jobs in the queue, called from cmd_list_jobs
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

// checks to see if scheduling policy is changed. if so, calls for a reschedule of jobs
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
        pthread_mutex_lock(&scheduled_mutex);
        runningReSortJobs(&head_job_scheduled, currentPolicy);
        pthread_mutex_unlock(&scheduled_mutex);
    }
    return 0;
}

// commmand action to run a single job from the command line
int cmd_run_job(int nargs, char **args)
{
    //printf("back from commandAction\n");
    if ((strcmp(args[0], "run") == 0) || (strcmp(args[0], "r") == 0))
    {
        //printf("Entering run area\n");
        //valid = 0;
        float new_time = 0;
        int new_priority = 0;
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
        printf("Job %s was submitted.\n", newjob->name);
        printf("Total number of jobs in the queue: %d\n", submitted_size + scheduled_size);
        float wait_time = waiting_time(&head_job_submitted, &newjob) + waiting_time(&head_job_scheduled, &newjob);
        if (running_job != NULL)
        {
            wait_time += running_job->cpu_time;
        }
        printf("Expected waiting time: %f seconds.\n", wait_time);
        printf("Scheduling Policy: ");
        if (currentPolicy == FCFS)
            printf("FCFS.\n");
        if (currentPolicy == SJF)
            printf("SJF.\n");
        if (currentPolicy == Priority)
            printf("Priority.\n");
    }
    return 0;
}

// command to enter a large number of jobs at once, and keep running. good for seeing policy changes
int cmd_large_batch(int nargs, char **args)
{
    // generate jobs then submit them based on parameters
    // this does not delete the completed queue before running
    // batch <benchmark> <policy> <num_of_jobs> <priority_levels>
    //      <min_CPU_time> <max_CPU_time>
    int valid = 0;
    enum Policy policy_number = Invalid;
    int num_of_jobs = 0;
    //int priority_low = 0;
    int priority_high = 0;
    double min_cpu_time = 0;
    double max_cpu_time = 0;
    double arrival_rate = 0;
    if (nargs < 7 || nargs > 8)
    {
        printf("Incorrect command. Format is:\n");
        printf("batch <benchmark> <policy> <num_of_jobs> <priority_levels> <min_CPU_time> <max_CPU_time> <arrival_rate>\n");
        return -1;
    }
    if (strlen(args[1]) > 20)
    {
        printf("<benchmark> must be less than 20 characters.\n");
        valid = 1;
    }
    num_of_jobs = atoi(args[3]);
    if (num_of_jobs < 1 || num_of_jobs > MAXJOBS)
    {
        printf("Number of jobs must be a positive integer less than %d.\n", MAXJOBS);
        valid = 1;
    }
    priority_high = atoi(args[4]);
    if (priority_high < 0)
    {
        printf("Priority levels must be a non-negative integer.\n");
        valid = 1;
    }
    min_cpu_time = atof(args[5]);
    max_cpu_time = atof(args[6]);
    if (min_cpu_time <= 0 || max_cpu_time <= 0 || min_cpu_time > max_cpu_time)
    {
        printf("CPU time must be greater than 0, and max CPU time must not be less than min CPU time.\n");
        valid = 1;
    }
    if (nargs == 8)
    {
        arrival_rate = atof(args[7]);
    }
    if (arrival_rate < 0)
    {
        printf("arrival rate must be a non-negative number\n");
        return -1;
    }
    if (strcasecmp("fcfs", args[2]) == 0)
        policy_number = FCFS;
    if (strcasecmp("sjf", args[2]) == 0)
        policy_number = SJF;
    if (strcasecmp("priority", args[2]) == 0)
        policy_number = Priority;
    if (policy_number == Invalid)
    {
        printf("Policy must be one of: fcfs, sjf, priority.\n");
        valid = 1;
    }
    if (valid != 0)
    {
        printf("Incorrect command. Format is:\n");
        printf("test <benchmark> <policy> <num_of_jobs> <priority_levels> <min_CPU_time> <max_CPU_time>\n");
        return -1;
    }
    // set policy
    printf("creating batch of jobs, not clearing completed queue\n");
    currentPolicy = policy_number;
    double time_increment = (max_cpu_time - min_cpu_time) / num_of_jobs;
    for (int i = 0; i < num_of_jobs; i++)
    {
        // call cmd_run_job with loop for all the jobs
        struct Job *newjob;
        ++global_job_id;
        strcpy(jobnames[global_job_id], args[1]);
        jobs[global_job_id].id = global_job_id;
        jobs[global_job_id].name = jobnames[global_job_id];
        jobs[global_job_id].priority = i % priority_high;
        jobs[global_job_id].cpu_time = min_cpu_time + (time_increment * (i % 11));
        jobs[global_job_id].arrival_time = process_time();
        jobs[global_job_id].next = NULL;

        //current pointer at first job
        newjob = &jobs[global_job_id];
        print_job(newjob);
        //sleep(1);
        //printf("job %d arrived at about %f\n", job1.id, job1.arrival_time);
        submitJob(newjob);
    }
    printf("Approximate time left is %f\n", time_left(&head_job_scheduled) + time_left(&head_job_submitted));
    return 0;
}

// deletes the completed queue so new statistics can be generated
int cmd_reset_queue(int nargs, char **args)
{
    // purpose is to delete the completed queue
    if ((submitted_size > 0 || scheduled_size > 0) || running_job != NULL)
    {
        printf("deleting completed queue not allowed while there are submitted or scheduled jobs.\n");
        return -1;
    }
    printf("deleting the completed queue\n");
    delete_completed_queue();
    return 0;
}

// this is a benchmarking command that will provide stats on a specific test from user
int cmd_test_benchmark(int nargs, char **args)
{
    // generate jobs then submit them based on parameters
    // test <benchmark> <policy> <num_of_jobs> <priority_levels>
    //       <min_CPU_time> <max_CPU_time> <arrival_rate>
    int valid = 0;
    enum Policy policy_number = Invalid;
    int num_of_jobs = 0;
    //int priority_low = 0;
    int priority_high = 0;
    double min_cpu_time = 0;
    double max_cpu_time = 0;
    double arrival_rate = 0;
    if (nargs < 7 || nargs > 8)
    {
        printf("Incorrect command. Format is:\n");
        printf("test <benchmark> <policy> <num_of_jobs> <priority_levels> <min_CPU_time> <max_CPU_time> <arrival_rate>\n");
        return -1;
    }
    if (strlen(args[1]) > 20)
    {
        printf("<benchmark> must be less than 20 characters.\n");
        valid = 1;
    }
    num_of_jobs = atoi(args[3]);
    if (num_of_jobs < 1 || num_of_jobs > MAXJOBS)
    {
        printf("Number of jobs must be a positive integer less than %d.\n", MAXJOBS);
        valid = 1;
    }
    priority_high = atoi(args[4]);
    if (priority_high < 0)
    {
        printf("Priority levels must be a non-negative integer.\n");
        valid = 1;
    }
    min_cpu_time = atof(args[5]);
    max_cpu_time = atof(args[6]);
    if (min_cpu_time <= 0 || max_cpu_time <= 0 || min_cpu_time > max_cpu_time)
    {
        printf("CPU time must be greater than 0, and max CPU time must not be less than min CPU time.\n");
        valid = 1;
    }
    if (nargs == 8)
    {
        arrival_rate = atof(args[7]);
    }
    if (arrival_rate < 0)
    {
        printf("arrival rate must be a non-negative number\n");
        return -1;
    }
    if (strcasecmp("fcfs", args[2]) == 0)
        policy_number = FCFS;
    if (strcasecmp("sjf", args[2]) == 0)
        policy_number = SJF;
    if (strcasecmp("priority", args[2]) == 0)
        policy_number = Priority;
    if (policy_number == Invalid)
    {
        printf("Policy must be one of: fcfs, sjf, priority.\n");
        valid = 1;
    }
    if (valid != 0)
    {
        printf("Incorrect command. Format is:\n");
        printf("test <benchmark> <policy> <num_of_jobs> <priority_levels> <min_CPU_time> <max_CPU_time> <arrival_rate>\n");
        return -1;
    }
    // set policy
    printf("Starting benchmark, deleting current completed queue\n");
    delete_completed_queue();
    global_job_id = 0;
    currentPolicy = policy_number;
    double time_increment = (max_cpu_time - min_cpu_time) / (num_of_jobs);
    for (int i = 0; i < num_of_jobs; i++)
    {
        // call cmd_run_job with loop for all the jobs
        struct Job *newjob;
        ++global_job_id;
        strcpy(jobnames[global_job_id], args[1]);
        jobs[global_job_id].id = global_job_id;
        jobs[global_job_id].name = jobnames[global_job_id];
        jobs[global_job_id].priority = i % priority_high;
        jobs[global_job_id].cpu_time = min_cpu_time + (time_increment * (i % num_of_jobs));
        jobs[global_job_id].arrival_time = process_time();
        jobs[global_job_id].next = NULL;

        //current pointer at first job
        newjob = &jobs[global_job_id];
        print_job(newjob);
        //sleep(1);
        //printf("job %d arrived at about %f\n", job1.id, job1.arrival_time);
        submitJob(newjob);
        usleep(arrival_rate * 1000000);
    }
    printf("done submitting jobs\n\n");
    printf("Approximate time left is %f\n", time_left(&head_job_scheduled) + time_left(&head_job_submitted));
    while ((submitted_size + scheduled_size > 0) || running_job != NULL)
    {
        //printf("waiting for jobs to finish\n");
        //printf("submitted size: %d\n", submitted_size);
        //printf("scheduled size: %d\n", scheduled_size);
        //printf(".");
        sleep(1);
    }
    //printf("benchmark completed\n");
    statisticsCompleted();
    return 0;
}

// command to show the current completed queue statistics
int cmd_statistics(int nargs, char **args)
{
    // print statistics of current completed queue
    statisticsCompleted();
    return 0;
}

// generates a performance evaluation of 9 tests, writes results to screen and disc
int cmd_performance(int nargs, char **args)
{
    // single command will conduct automated performance evaluation
    // create a batch and run it, write it to disc in csv format
    // 3 job profiles for each method
    // 25 jobs of time distribution 1 to 10 seconds
    // 3 priority levels
    // arrival rate .1, .5, 1 second
    // create data directory
    printf("Starting performance test - 9 sets of data generated.\n");
    printf("A file of the results is available in ./data/performance_data.csv\n");
    struct stat st = {0};

    if (stat("./data", &st) == -1)
    {
        mkdir("./data", 0744);
    }
    // create and open file for writing in data directory
    FILE *fptr;
    fptr = fopen("./data/performance_data.csv", "w");
    fprintf(fptr, "testij,policy,arrival_rate,throughput,response_time_mean,max_response_time,min_response_time,response_deviation\n");
    if (fptr == NULL)
    {
        printf("Error - can't open file");
        exit(1);
    }
    // let's go! make data and write to file
    enum Policy policy_number = Invalid;
    float max_cpu_time = 3.0;
    float min_cpu_time = 1.0;
    int priority_high = 3;
    float arrival_rate = 0;
    int num_of_jobs = 25;
    char perf_test[24] = "";
    double response_time = 0;
    double response_time_mean = 0;
    double response_time_total = 0;
    double max_response_time = 0;
    double min_response_time = 1000000;
    double earliest_arrival_time = 10000000;
    double latest_finish_time = 0;
    struct Job *current = NULL;
    int num_jobs_run = 0;
    double response_times[num_of_jobs];
    double response_deviation_total = 0;
    double response_deviation_mean = 0;
    double response_deviation = 0;
    double throughput = 0;

    for (int i = 0; i < 3; i++)
    {
        if (i == 0)
            arrival_rate = 0.1;

        if (i == 1)
            arrival_rate = 0.5;

        if (i == 2)
            arrival_rate = 1.0;

        for (int j = 0; j < 3; j++)
        {
            if (j == 0)
                policy_number = FCFS;
            if (j == 1)
                policy_number = SJF;
            if (j == 2)
                policy_number = Priority;
            delete_completed_queue();
            global_job_id = 0;
            currentPolicy = policy_number;
            double time_increment = (max_cpu_time - min_cpu_time) / (num_of_jobs - 1);
            sprintf(perf_test, "test%d%d", i, j);
            if (currentPolicy == FCFS)
            {
                printf("FCFS ");
            }
            if (currentPolicy == SJF)
            {
                printf("SJF ");
            }
            if (currentPolicy == Priority)
            {
                printf("Priority ");
            }
            printf("& arrival rate: %f \n", arrival_rate);
            printf("ID\tname\tpri\tcpu\t\tarrival\n");
            for (int i = 0; i < num_of_jobs; i++)
            {
                // call cmd_run_job with loop for all the jobs
                struct Job *newjob;
                ++global_job_id;
                strcpy(jobnames[global_job_id], perf_test);
                jobs[global_job_id].id = global_job_id;
                jobs[global_job_id].name = jobnames[global_job_id];
                jobs[global_job_id].priority = i % priority_high;
                jobs[global_job_id].cpu_time = max_cpu_time - (time_increment * (i % (num_of_jobs)));
                jobs[global_job_id].arrival_time = process_time();
                jobs[global_job_id].next = NULL;

                //current pointer at first job
                newjob = &jobs[global_job_id];
                print_job(newjob);

                submitJob(newjob);
                usleep(arrival_rate * 1000000);
            }
            // when jobs are all done, get statistics and write to disc
            // average repsonse time, throughput, max response time, min response time,
            // response time standard deviation
            //
            // first wait for jobs to be done
            printf("done submitting jobs\n\n");
            printf("Approximate time left is %f\n", time_left(&head_job_scheduled) + time_left(&head_job_submitted));
            while ((submitted_size + scheduled_size > 0) || running_job != NULL)
            {
                sleep(1);
            }
            printf("jobs are done running, starting calculations\n");
            // we have a queue of complete jobs - get the information
            current = head_job_completed;
            response_time_total = 0;
            response_time = 0;
            max_response_time = 0;
            min_response_time = 1000000;
            earliest_arrival_time = 10000000;
            latest_finish_time = 0;
            num_jobs_run = -1;
            while (current != NULL)
            {
                print_job(current);
                num_jobs_run++;
                if (current->finish_time > latest_finish_time)
                    latest_finish_time = current->finish_time;
                if (current->arrival_time < earliest_arrival_time)
                    earliest_arrival_time = current->arrival_time;
                response_time = current->finish_time - current->arrival_time;
                response_times[num_jobs_run] = response_time;
                if (response_time < min_response_time)
                    min_response_time = response_time;
                if (response_time > max_response_time)
                    max_response_time = response_time;
                current = current->next;
            }
            // now we have the data needed to calculate performance
            response_time_mean = 0;
            response_time_total = 0;
            for (int i = 0; i <= num_jobs_run; i++)
            {
                response_time_total += response_times[i];
            }
            response_time_mean = response_time_total / (num_jobs_run + 1);
            response_deviation_total = 0;
            for (int i = 0; i <= num_jobs_run; i++)
            {
                response_deviation_total += (response_times[i] - response_time_mean) * (response_times[i] - response_time_mean);
            }
            response_deviation_mean = response_deviation_total / (num_jobs_run + 1);
            response_deviation = sqrt(response_deviation_mean);
            throughput = (num_jobs_run + 1) / (latest_finish_time - earliest_arrival_time);
            // now write all the data to disc for this job
            // format is job name, throughput, avg resp time, max resp time, min resp time, resp time std deviation
            fprintf(fptr, "test%d%d,", i, j);
            printf("test%d%d ", i, j);
            if (currentPolicy == FCFS)
            {
                fprintf(fptr, "FCFS,");
                printf("FCFS ");
            }
            if (currentPolicy == SJF)
            {
                fprintf(fptr, "SJF,");
                printf("SJF ");
            }
            if (currentPolicy == Priority)
            {
                fprintf(fptr, "Priority,");
                printf("Priority ");
            }
            fprintf(fptr, "%f,", arrival_rate);
            printf("arrival rate: %f ", arrival_rate);
            fprintf(fptr, "%f,%f,%f,%f,%f\n", throughput, response_time_mean, max_response_time, min_response_time, response_deviation);
            printf("throughput: %f jobs/sec mean response: %f sec max: %f sec min response: %f sec std deviation: %f sec\n", throughput, response_time_mean, max_response_time, min_response_time, response_deviation);
            printf("\n\n");
        }
    }
    printf("performance test is complete\n");
    fclose(fptr);
    return 0;
}
