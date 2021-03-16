// aubatch.h

#ifndef AUBATCH_H
#define AUBATCH_H

#include "aubatch_scheduler.h"
#include "aubatch_utilities.h"

size_t string_parser(const char *, char ***);
int commandAction(int, char ***);
int cmd_helpmenu(int, char **);
void showmenu(const char *, const char **);
int cmd_quit(int , char **);
int cmd_queue_size(int , char **);
int cmd_list_jobs(int , char **);
int print_queue_job_info(struct Job *);
int cmd_policy_change(int , char **);
int cmd_run_job(int , char **);
int cmd_test_benchmark(int, char **);
int cmd_large_batch(int, char **);
int cmd_reset_queue(int, char**);
int cmd_statistics(int, char **);
int cmd_performance(int, char **);



// global variables
extern struct Job* head_job_submitted;
extern struct Job* head_job_scheduled;
extern struct Job* head_job_completed;
extern struct Job* running_job;
extern enum Policy currentPolicy;
extern int policyChange;
extern pthread_mutex_t submitted_mutex;
extern pthread_cond_t submitted_full;
extern pthread_cond_t submitted_empty;
extern int submitted_buffer_size;
extern int submitted_size;
extern pthread_mutex_t scheduled_mutex;
extern pthread_cond_t scheduled_empty;
extern pthread_cond_t scheduled_full;
extern int scheduled_buffer_size;
extern int scheduled_size;
extern pthread_mutex_t completed_mutex;
extern pthread_cond_t completed_full;
extern pthread_cond_t completed_empty;
extern int completed_buffer_size;
extern int completed_size;
extern int hardquit;
extern int softquit;
int global_job_id;
int valid;
const int MAXJOBS = 500;
struct Job jobs[500];
char jobnames[500][24];


static struct
{
    // This command structure provided by Xiao Qin in commandline_parser.c
    const char *name;
    int (*func)(int nargs, char **args);
} cmdtable[] = {
    /* commands: single command must end with \n */
    {"", cmd_helpmenu},
    {"?", cmd_helpmenu},
    {"h", cmd_helpmenu},
    {"help", cmd_helpmenu},
    {"j", cmd_queue_size},
    {"jobs", cmd_queue_size},
    {"l", cmd_list_jobs},
    {"list", cmd_list_jobs},
    {"fcfs", cmd_policy_change},
    {"sjf", cmd_policy_change},
    {"priority", cmd_policy_change},
    {"r",		cmd_run_job },
    {"run",	cmd_run_job },
    {"q", cmd_quit},
    {"quit", cmd_quit},
    {"test", cmd_test_benchmark},
    {"reset", cmd_reset_queue},
    {"r", cmd_reset_queue},
    {"batch", cmd_large_batch},
    {"b", cmd_large_batch},
    {"statistics", cmd_statistics},
    {"s", cmd_statistics},
    {"performance", cmd_performance},
    {"p", cmd_performance},
    /* Please add more operations below. */
    {NULL, NULL}};

static const char *helpmenu[] = {
    "run <job> <time> <pri>: submit a job named <job>,",
    "\t\t\texecution time is <time>,",
    "\t\t\tpriority is <pri>.",
    "list: display the job status.",
    "fcfs: change the scheduling policy to FCFS.",
    "sjf: change the scheduling policy to SJF.",
    "priority: change the scheduling policy to priority.",
    "test <benchmark> <policy> <num_of_jobs> <priority_levels>\n     <min_CPU_time> <max_CPU_time> <arrival rate>",
    "batch <benchmark> <policy> <num_of_jobs> <priority_levels>\n     <min_CPU_time> <max_CPU_time> <arrival rate>",
    "quit: exit AUbatch",
    "reset: clear the completed queue",
    /* Please add more menu options below */
    NULL};

#endif 