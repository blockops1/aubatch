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
extern float procTime;



#endif 