//-----------------------------------------
// NAME: Sanjam Khera
// STUDENT NUMBER: 007866840
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Franklin Bristow
// ASSIGNMENT: assignment 2, QUESTION: question 1
// 
// REMARKS: Program implements a MLFQ Simulator, where each CPU is a thread and an additional thread
//          is used for process_boost.
//
//-----------------------------------------

#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>
#include <string.h>
#include <sys/syscall.h>
#include "MyQueue.h"

#define MAX_TASK_TYPES 4

// Lock for reader thread queue
pthread_mutex_t read_lock;
// Lock for done area
pthread_mutex_t done_lock;

// Dispatch Condition Variable
pthread_cond_t dispatch;

// Terminate Signal to end simulation
pthread_cond_t terminate_simulation;

// Declares an atomic unsigned integer variable to count ready cpu threads
_Atomic(int) cpu_ready;

// Declares an atomic unsigned integer as flag to see if EOF is reached
_Atomic(int) eof_reached;

// A dedicated thread to boost the queue is used
long priority_boost_time;

// Declaring the queue
priority_queue* ready_mlfq; 

// Index for the next empty space in the array
int done_index = 0;

// Done area
task* done[MAX_DONE_TASKS];

// Function to get thread ID
pid_t gettid(void)
{
    return syscall(SYS_gettid);  // syscall is used to call kernel functions
}

// Add a task to the done array
void add_to_done(task* done_task) 
{
    pthread_mutex_lock(&done_lock);

    if(done_index < MAX_DONE_TASKS) 
    {
        done[done_index++] = done_task;
    } 
    else 
    {
        printf("Error: done task array is full\n");
    }
    pthread_mutex_unlock(&done_lock);
}

// Calculate and print the average turnaround time per task type
void average_turnaround_time_per_type() 
{
    long sum_turnaround_time[MAX_TASK_TYPES] = {0};
    int num_tasks[MAX_TASK_TYPES] = {0};
    
    pthread_mutex_lock(&done_lock);
    
    for (int i = 0; i < done_index; i++) 
    {
        task* done_task = done[i];
        int type = done_task->task_type;
        
        if (type >= 0 && type < MAX_TASK_TYPES) 
        {
            long turnaround_time = diff(done_task->arrival_time, done_task->done_at_time);
            sum_turnaround_time[type] += turnaround_time;
            num_tasks[type]++;
        }
    }

    pthread_mutex_unlock(&done_lock);

    printf("Average turnaround time per type:\n");
    
    for (int i = 0; i < MAX_TASK_TYPES; i++) 
    {
        if (num_tasks[i] > 0) 
        {
            printf("Type %d: %ld usec\n", i, sum_turnaround_time[i] / num_tasks[i]);
        } 
        else 
        {
            printf("Type %d: 0 usec (no tasks of this type were done)\n", i);
        }
    }
}

// Calculate and print average response time per task type
void average_response_time_per_type() 
{
    long sum_response_time[MAX_TASK_TYPES] = {0};
    int num_tasks[MAX_TASK_TYPES] = {0};
    
    pthread_mutex_lock(&done_lock);
    
    for (int i = 0; i < done_index; i++) 
    {
        task* done_task = done[i];
        int type = done_task->task_type;
        
        if (type >= 0 && type < MAX_TASK_TYPES) 
        {
            sum_response_time[type] += done_task->response_time;
            num_tasks[type]++;
        }
    }

    pthread_mutex_unlock(&done_lock);

    printf("Average response time per type:\n");
    
    for (int i = 0; i < MAX_TASK_TYPES; i++) 
    {
        if (num_tasks[i] > 0) 
        {
            printf("Type %d: %ld usec\n", i, sum_response_time[i] / num_tasks[i]);
        } 
        else 
        {
            printf("Type %d: 0 usec (no tasks of this type were done)\n", i);
        }
    }
}


// Set queue time and total time spent by the task running at that priority level
void update_task_priority(task* curr_task) 
{
    //printf("In update task priority.\n");
    if (curr_task->q_time >= MAX_Q_TIME && curr_task->curr_priority > MIN_PRIORITY)
    {
        curr_task->curr_priority--;
        curr_task->q_time = 0;
    } 
    else
    {
        curr_task->q_time = curr_task->q_time + QUANTUM_LENGTH;
    }
}

// Function to set queue time and total time spent by the task running at that priority level.
void update_task_length(task* curr_task) 
{
    if (curr_task->task_length <= QUANTUM_LENGTH)
    {
        curr_task->task_length = 0;
    } 
    else
    {
        curr_task->task_length = curr_task->task_length - QUANTUM_LENGTH;
    }
}

// Check for IO
// Function is used by CPU thread 
// Returns true if a task requires I/O
bool io_processing(task* task_to_check) 
{
    // Generate a random number between 1 and 100
    int random_number = (rand() % 100) + 1;

    // Convert odds_of_IO to an integer between 0 and 100 for comparison
    int io_chance = (int) (task_to_check->odds_of_IO * 100);

    if (random_number >= io_chance) 
    {
        // Return true to indicate that the task did I/O
        return true;
    }
    
    // Return false to indicate that the task did not do I/O
    return false;
}

// Function to move all tasks to queue 4
// Functions is used to priority boost mlfq
void move_all_to_queue_4(priority_queue* pq) 
{
    for (int priority = MIN_PRIORITY; priority < MAX_PRIORITY; ++priority) 
    {
        task* deq_task;
        while ((deq_task = dequeue_priority(pq, priority)) != NULL) 
        {
            deq_task->curr_priority = MAX_PRIORITY;
            enqueue(pq, deq_task);
        }
    }
}


//------------------------------------------------------
// cpu_thread_function
//
// PURPOSE: This function acts as a thread routine simulating a CPU executing tasks.
// INPUT PARAMETERS:
//     arg: This parameter accepts a void pointer. In this case, it's unused.
// OUTPUT PARAMETERS:
//     This routine returns nothing but it modifies global variables such as cpu_ready, 
//     ready_mlfq and manages synchronization with other threads using mutex and conditions.
//     It also modifies the task properties such as task_length and done_at_time.
//------------------------------------------------------
void* cpu_thread_function(void *arg)
{
    (void) arg;

    while(1)
    {
        cpu_ready++;
        pthread_mutex_lock(&read_lock);

        while(is_empty(ready_mlfq) && !eof_reached)
        {
            pthread_cond_wait(&dispatch, &read_lock);
        }

        task* curr_task = dequeue(ready_mlfq);
        if (curr_task != NULL)
        {
            if (io_processing(curr_task))
            {   
                int random_io_time = (rand() % QUANTUM_LENGTH) + 1;
                if (curr_task->task_length < random_io_time)
                {
                    curr_task->task_length = 0;
                    curr_task->done_at_time = get_time();
                    add_to_done(curr_task);
                }
                else
                {
                    curr_task->task_length -= random_io_time;
                    enqueue_at_front(ready_mlfq, curr_task);
                }
   
            }
            else 
            {
                // Sleep for Quantum Length period of time
                usleep(QUANTUM_LENGTH);

                if (curr_task->task_length <= 0)
                {
                    curr_task->task_length = 0;
                    curr_task->done_at_time = get_time();
                    add_to_done(curr_task);
                }
                else 
                {
                    update_task_length(curr_task);
                    update_task_priority(curr_task);
                    enqueue(ready_mlfq, curr_task);
                }
            }
        }

        if (is_empty(ready_mlfq) && eof_reached) 
        {
            pthread_mutex_unlock(&read_lock);
            break;
        }
        pthread_mutex_unlock(&read_lock);

    }
    // Exit the thread
    pthread_exit(NULL);
}


//------------------------------------------------------
// priority_boost_thread
//
// PURPOSE: This function acts as a thread routine that boosts the priority of all tasks periodically.
// INPUT PARAMETERS:
//     arg: This parameter accepts a void pointer. In this case, it's unused.
// OUTPUT PARAMETERS:
//     This routine returns nothing but it manipulates global variables, ready_mlfq in particular,
//     by moving all the tasks to a certain queue (queue 4 in this case) and manages synchronization 
//     with other threads using mutex.
//------------------------------------------------------
void* priority_boost_thread(void *arg)
{
    (void) arg;
    while (1)
    {
        usleep(priority_boost_time);
        pthread_mutex_lock(&read_lock);

        // Use move_all_to_queue_4 function to transfer all tasks to queue 4
        move_all_to_queue_4(ready_mlfq);

        if (is_empty(ready_mlfq) && eof_reached) 
        {
            pthread_mutex_unlock(&read_lock);
            break;
        }

        pthread_mutex_unlock(&read_lock);
    }
    pthread_exit(NULL);
}

// MAIN Thread
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Required Inputs: ./programName <numOfCPUs> <time> <taskFile>\n");
        return EXIT_FAILURE;
    }

    int num_of_cpus = atoi(argv[1]);
    priority_boost_time = strtol((argv[2]), NULL, 10);
    char* task_file = argv[3];

    // Initialize MLFQ
    ready_mlfq = create_priority_queue();

    // CPU arrays
    pthread_t threads[num_of_cpus]; 
    pthread_t boost_thread;

    // Set flag
    eof_reached = 0;

    // Initialize the condition variables
    pthread_cond_init(&dispatch, NULL);
    pthread_cond_init(&terminate_simulation, NULL);

    // Initialize locks
    pthread_mutex_init(&read_lock, NULL);
    
    // Initialize all CPUs
    for(int i=0; i<num_of_cpus; i++)
    {
        pthread_create(&threads[i], NULL, cpu_thread_function, NULL);
    }

    // Busy waiting until all threads are ready
    while (cpu_ready < num_of_cpus);

    // Create boost thread
    pthread_create(&boost_thread, NULL, priority_boost_thread, NULL);

    char line[LINE_BUFFER_SIZE];
    FILE *file = fopen(task_file, "r");

    if (file == NULL) 
    {
        printf("Could not open file %s\n", task_file);
        return EXIT_FAILURE;
    }

    // Read file input
    while (fgets(line, sizeof(line), file))
    {
        char* token;
        char* task_name;
        int type;
        int length;
        float odds;
        int count = 0;

        token = strtok(line, " ");
        while (token) 
        {
            // Check if the token is "DELAY"
            if (count == 0 && strcmp(token, "DELAY") == 0) 
            {
                token = strtok(NULL, " ");
                int delay_time = atoi(token);
                usleep(delay_time);
            }
            if (count == 0) 
            {
                task_name = token;
            }
            else if (count == 1) 
            {
                type = atoi(token);
            }
            else if (count == 2) 
            {
                length = atoi(token);
            }
            else if (count == 3) 
            {
                odds = atof(token); // Convert the odds directly to float
            }
            count++;
            token = strtok(NULL, " ");
        }

        // Check if it was not a delay command
        if (count > 1)
        {
            task* new_task = create_task(task_name, type, length, odds / 100.0);
            pthread_mutex_lock(&read_lock);
            enqueue(ready_mlfq, new_task);
            pthread_mutex_unlock(&read_lock);
        }

        if (!is_empty(ready_mlfq)) {
            pthread_cond_broadcast(&dispatch);
        }
    }

    // Flag to terminate threads
    eof_reached = 1;
    pthread_cond_broadcast(&dispatch);
    fclose(file);

    // Wait for all threads to finish before exiting
    for(int i=0; i<num_of_cpus; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_join(boost_thread, NULL);

    // Print at the end
    average_turnaround_time_per_type();
    average_response_time_per_type();

    return 0;
}
