//-----------------------------------------
// NAME: Sanjam Khera
// STUDENT NUMBER: 007866840
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Franklin Bristow
// ASSIGNMENT: assignment 2, QUESTION: question 1
// 
// REMARKS: Program implements a interface for MLFQ Priority Queue and Tasks.
//
//-----------------------------------------

#ifndef MYQUEUE_H
#define MYQUEUE_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MAX_NAME_LENGTH 12
#define MAX_PRIORITY 4
#define MIN_PRIORITY 1
#define QUANTUM_LENGTH 50
#define MAX_DONE_TASKS 100
#define MAX_Q_TIME 200
#define LINE_BUFFER_SIZE 30

typedef struct Task task;

struct Task {
    char task_name[MAX_NAME_LENGTH];
    int task_type;
    int task_length;
    float odds_of_IO;

    long arrival_time;
    int curr_priority;
    long done_at_time;
    long response_time;
    long q_time;

    long enqueued_time;
    bool first_dequeue;
    task* next; 
};


// Queue Structure
typedef struct Queue 
{
    task* front;
    task* rear;
} queue;

// Priority Queue Structure
typedef struct Priority_queue
{
    queue* queue_1; // for priority 1 tasks
    queue* queue_2; // for priority 2 tasks
    queue* queue_3; // for priority 3 tasks
    queue* queue_4; // for priority 4 tasks
} priority_queue;

// Function Prototypes
task* create_task(char* name, int type, int length, float odds);
queue* create_queue();
priority_queue* create_priority_queue();
void enqueue(priority_queue* pq, task* new_task); 
task* dequeue(priority_queue* q);
void enqueue_at_front(priority_queue* pq, task* new_task);
long get_time();
long diff(long start, long end);
bool is_empty(priority_queue* pq);
task* dequeue_priority(priority_queue* pq, int priority);

#endif // MYQUEUE_H
