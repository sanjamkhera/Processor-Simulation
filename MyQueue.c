//-----------------------------------------
// NAME: Sanjam Khera
// STUDENT NUMBER: 007866840
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Franklin Bristow
// ASSIGNMENT: assignment 2, QUESTION: question 1
// 
// REMARKS: Program implements a MLFQ Priority Queue and Tasks that are added to the queue
//
//-----------------------------------------

#include <time.h>
#include <string.h>
#include <unistd.h>
#include "MyQueue.h"

// Create a task
task* create_task(char* name, int type, int length, float odds) 
{
    task* new_task = (task*) malloc(sizeof(task));
    
    strncpy(new_task->task_name, name, MAX_NAME_LENGTH);
    new_task->task_type = type;
    new_task->task_length = length;
    new_task->odds_of_IO = odds;

    new_task->arrival_time = get_time();
    new_task->curr_priority = MAX_PRIORITY;
    new_task->done_at_time = 0;

    new_task->response_time = 0;
    new_task->q_time = 0;
    new_task->first_dequeue = true;
    return new_task;
}

// Create a queue
queue* create_queue() 
{
    queue* q = (queue*) malloc(sizeof(queue));
    q->front = NULL;
    q->rear = NULL;
    return q;
}

// Priority queue
priority_queue* create_priority_queue() {
    priority_queue* pq = (priority_queue*) malloc(sizeof(priority_queue));
    pq->queue_4 = create_queue();
    pq->queue_3 = create_queue();
    pq->queue_2 = create_queue();
    pq->queue_1 = create_queue();
    return pq;
}

//------------------------------------------------------
// enqueue
//
// PURPOSE: Adds a new task to a specific priority queue according to its current priority.
// INPUT PARAMETERS:
//     pq: A pointer to the priority_queue structure where the task is to be added.
//     new_task: A pointer to the task structure that is to be added to the queue.
// OUTPUT PARAMETERS:
//     The function does not return any value. However, it modifies the passed priority_queue by
//     adding a task to it.
//------------------------------------------------------
void enqueue(priority_queue* pq, task* new_task) 
{
    queue* queue_to_use;
    switch(new_task->curr_priority) 
    {
        case 4: queue_to_use = pq->queue_4; break;
        case 3: queue_to_use = pq->queue_3; break;
        case 2: queue_to_use = pq->queue_2; break;
        case 1: queue_to_use = pq->queue_1; break;
    }

    new_task->next = NULL;

    if(queue_to_use->rear != NULL)
        queue_to_use->rear->next = new_task;

    queue_to_use->rear = new_task;

    if(queue_to_use->front == NULL)
        queue_to_use->front = new_task;
}

//------------------------------------------------------
// dequeue
//
// PURPOSE: Removes and returns a task from the highest priority non-empty queue.
// INPUT PARAMETERS:
//     pq: A pointer to the priority_queue structure from which a task is to be dequeued.
// OUTPUT PARAMETERS:
//     The function returns a pointer to the dequeued task. It modifies the passed priority_queue by
//     removing a task from it.
//------------------------------------------------------
task* dequeue(priority_queue* pq) 
{
    queue* queue_to_use;
    task* task_to_dequeue;
    for (int priority = MAX_PRIORITY; priority >= MIN_PRIORITY; --priority) 
    {
        switch(priority) 
        {
            case 4: queue_to_use = pq->queue_4; break;
            case 3: queue_to_use = pq->queue_3; break;
            case 2: queue_to_use = pq->queue_2; break;
            case 1: queue_to_use = pq->queue_1; break;
        }
        if (queue_to_use->front != NULL) 
        {
            task_to_dequeue = queue_to_use->front;
            queue_to_use->front = queue_to_use->front->next;
            if (queue_to_use->front == NULL)
                queue_to_use->rear = NULL;
            if (task_to_dequeue->first_dequeue)
            {
                task_to_dequeue->response_time = get_time() - task_to_dequeue->arrival_time;
                task_to_dequeue->first_dequeue = false;
            }
            return task_to_dequeue;
        }
    }
    return NULL; // All queues are empty, return NULL
}


//------------------------------------------------------
// enqueue_at_front
//
// PURPOSE: Adds a new task at the front of the highest priority queue used for I/O Tasks..
// INPUT PARAMETERS:
//     pq: A pointer to the priority_queue structure where the task is to be added.
//     new_task: A pointer to the task structure that is to be added to the queue.
// OUTPUT PARAMETERS:
//     The function does not return any value. However, it modifies the passed priority_queue by
//     adding a task to it.
//------------------------------------------------------
void enqueue_at_front(priority_queue* pq, task* new_task) 
{
    queue* queue_to_use = pq->queue_4;  // Always use the highest priority queue

    new_task->next = queue_to_use->front;

    if(queue_to_use->rear == NULL)
        queue_to_use->rear = new_task;

    queue_to_use->front = new_task;
}

//------------------------------------------------------
// dequeue_priority
//
// PURPOSE: Removes and returns a task from a specific priority queue, used for priority boost.
// INPUT PARAMETERS:
//     pq: A pointer to the priority_queue structure from which a task is to be dequeued.
//     priority: The specific priority queue from which the task is to be dequeued.
// OUTPUT PARAMETERS:
//     The function returns a pointer to the dequeued task. It modifies the passed priority_queue by
//     removing a task from it.
//------------------------------------------------------
task* dequeue_priority(priority_queue* pq, int priority) 
{
    queue* queue_to_use;
    task* task_to_dequeue;
    switch(priority) 
    {
        case 3: queue_to_use = pq->queue_3; break;
        case 2: queue_to_use = pq->queue_2; break;
        case 1: queue_to_use = pq->queue_1; break;
        default: return NULL;
    }
    if (queue_to_use->front != NULL) 
    {
        task_to_dequeue = queue_to_use->front;
        queue_to_use->front = queue_to_use->front->next;
        if (queue_to_use->front == NULL)
            queue_to_use->rear = NULL;
        if (task_to_dequeue->first_dequeue)
        {
            task_to_dequeue->response_time = get_time() - task_to_dequeue->arrival_time;
            task_to_dequeue->first_dequeue = false;
        }
        return task_to_dequeue;
    }
    return NULL;
}

// Function to get current time in microseconds
long get_time() 
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    // Convert the time to microseconds
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

// Function to calculate time difference in microseconds
long diff(long start, long end)
{
    return end - start;
}

// Function to check if a priority queue is empty
bool is_empty(priority_queue* pq) 
{
    if (pq->queue_4->front != NULL) return false;
    if (pq->queue_3->front != NULL) return false;
    if (pq->queue_2->front != NULL) return false;
    if (pq->queue_1->front != NULL) return false;
    return true;
}
