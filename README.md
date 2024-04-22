About the Project
----------------
- The accompanying program implements a MLFQ scheduling policy, where each CPU is a thread. 
- Tasks enter the system at start_time as soon as the simulator starts running. 
- This project implements a start clock and do not need a buffer.  
- The "Scheduler" main thread initialize all CPU threads, Dispatcher notify the tasks are available. 
- A "ready" thread will wait for the next batch at process to enter the system. 
- Scheduling policy implemented by the scheduler decides which task gets implemented next. 
- Dispatcher will notify all threads. One at a time "CPU" threads will take the task and "run" it. Each thread will run one task at a time. 

- Running a task could mean two things. 
- Each task will have some attributes that associated to it and our program should update these tasks before putting them back in the queue. 
- If not done put it back in the queue, if not done before the time slice, put it back in the done area. 
- When there are no tasks left in the scheduler, ready threads has done reading all tasks from the file. 
- The simulator shuts down and prints out a report. 

## Files included 
1. README.md 
2. Simulator.c 
3. MyQueue.c (NOTE: It implements "task" and it's behavior as well) 
4. MyQUeue.h 
5. Makefile 
6. Report.md 
7. table.md 
8. AvgTimes (Directory) 

## Compile and Run 
Simply write ```make``` to compile all the files run as per the requirement of the assignment. 
