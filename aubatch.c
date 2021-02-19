/* COMP7500 aubatch main program
 * This is a pthreads based batch scheduling system
 * Author: Rolf Versluis
 * Date: 18 Feb 2021
 */


#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    struct Job* next;
};

// declare function to put a job in the queue
int schedule(struct Job*, struct Job*, struct Job*);
int dispatch(struct Job*, struct Job*);


int main(int argc, char *argv[]) {
    //create the main job queue by initializing the head and tail pointers
    struct Job* head = NULL;
    struct Job* tail = NULL;
    struct Job* current = NULL;

    //create the first job
    struct Job* first;
    first = (struct Job*)malloc(sizeof(struct Job));
    if (first == NULL) {
        return 1;
    }
    first->id = 1;
    first->name = "firstjob";
    first->priority = 0;
    first->cpu_time = 10;
    first->next = NULL;

    //current pointer at first job
    current = first;

    // call the scheduler thread - this puts jobs in the job queue   
    schedule(current, head, tail);

    // call the dispatcher thread - this takes jobs out of the job queue   
    dispatch(head, tail);
}

// define scheduler function to put a job at the tail of the queue
int schedule(struct Job* current, struct Job* head, struct Job* tail) {

    // need mutex before changing queue pointers
    //if empty queue put first at head
    if (head == NULL) {
        head = current;
    }
    //current tail next needs to point to new item
    if (tail != NULL) {
        tail->next = current;
    } 
    tail = current;   
    // need mutex after changing queue pointers
    return 0;
}

// define dispatcher function to pull a job out of queue to run it
int dispatch(struct Job* head, struct Job* tail) {
    struct Job* previous = NULL;
    struct Job* current = NULL;
    // FCFS
    current = head;

    //SJF - find shortest job, pull out of queue
    // traverse queue until shortest job found

    //Priority - find highest firstest job, pull out of queue, need previous

    // remove next job from queue and send to processor
    if (current == head) {
        head = head->next;
    } else {
        previous->next = current->next;
    }
    return 0;
}
