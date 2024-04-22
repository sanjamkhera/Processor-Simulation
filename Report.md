# System Simulation and Performance Analysis

I ran a simulation with varying number of CPUs (1, 2, 4, 8) and varied the time allotment (S) for four different values (200µs, 800µs, 1600µs, and 3200µs), totaling to 12 runs, additional text files provided. Here, we'll take a look at the average turnaround and response time for each type of task for each of these combinations. 

## Impact of Time Allotment (S) on Turnaround Time and Response Time

The value of S, the time allotment, significantly affects both the turnaround time and the response time. As expected, in general, the average turnaround time increased as the value of S decreased. This is because a smaller time allotment means tasks take longer to complete their allocated cycles, leading to an increased turnaround time.

However, I am not too happy with the trend, seems like I was not able to achieve complete concurrency, the trend was not as consistent with the response time. Lowering the value of S didn't always result in higher response times. This inconsistency is likely because of my thread locking mechanism.

## Impact of Number of CPUs on Turnaround Time and Response Time

Increasing the number of CPUs tended to decrease both turnaround and response times. This is expected because with more CPUs, more tasks can be processed concurrently. This reduces the amount of time a task has to wait before it begins executing, which in turn reduces its response time. The reduced waiting time also contributes to a decreased turnaround time.

The impact of the number of CPUs wasn't consistent across all task types. It seems that the impact of the number of CPUs varies depending on the type of task and the fact that read_lock holds the queue and other resources wait around for it release so they can read the queue again.

## Correlation between S Value and Performance of Long-Running and I/O Tasks

Adjusting the S value in the system appeared to have a some impact on turnaround time or response time for long-running and I/O tasks specifically. The impact seemed to be correlated, with a lower S value generally leading to higher turnaround and response times.

These testings provided insights into how the system behaves under different conditions.
