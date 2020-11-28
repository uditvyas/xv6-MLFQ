# XV6 – MLFQ
## MLFQ RULES:
1. If Priority(A) > Priority(B), A runs (B doesn’t). 
2. If Priority(A) = Priority(B), A & B run in round-robin fashion using the time slice (quantum length) of the given queue.
3. When a job enters the system, it is placed at the highest priority (the topmost queue). 
4. Once a job uses up its time allotment at a given level (regardless of how many times it has given up the CPU), its priority is reduced (i.e., it moves down one queue). 
5. After some time period S, move all the jobs in the system to the topmost queue.

## List of modified files
* makefile
* proc.c
* proc.h
* syscall.c
* syscall.h
* sysproc.c
* trap.c
* trap.h
* user.h
* defs.h
* params.h

### Additions in Process Structures
### [Proc.h](https://github.com/uditvyas/xv6-MLFQ/blob/master/proc.h)
We add two more items to the ```struct proc``` for MLFQ scheduling
```C
int priority;                // Priority of the process
int myticks[4];              // No of ticks of the process in all the queues
```

We add a new structure ```struct pstat``` to facilitate the working of system call getPinfo.
```C
struct pstat{
  int inuse[NPROC]; // whether this slot of the process table is in use (1 or 0)
  int pid[NPROC];   // PID of each process
  int priority[NPROC];  // current priority level of each process (0-3)
  enum procstate state[NPROC];  // current state (e.g., SLEEPING or RUNNABLE) of each process
  int ticks[NPROC][4];  // number of ticks each process has accumulated at each of 4 priorities
  char name[NPROC][16]; // Name of the process
}
```
## [Proc.c](https://github.com/uditvyas/xv6-MLFQ/blob/master/proc.c)

In this file we declare the process queues for MLFQ. In our case, we have taken 4 priorities. So the Queues declared are ```q_1, q_2, q_3, q_4```. We have pointers to queue indices as ```p0, p1, p2, p3```. 
For each process queue, we define maximum permissible ticks as ```int clkPerPrio[4] ={1,2,4,8};```in the respective order. Where a tick is the smallest measurable time quantum of the system. In other words, a timer interrupt goes of after a duration of a tick.
The following are the functions defined in proc.c and declared in [defs.h](
https://github.com/uditvyas/xv6-MLFQ/blob/master/defs.h#L128)
#### [MLFQ function](https://github.com/uditvyas/xv6-MLFQ/blob/c80326de909811f20a783b5d6ffa526146f4d85f/proc.c#L434)
This file contains a generalised function MLFQ, which is used to implement the transition of a process from a higher priority queue to a lower priority queue.
The function takes in the input arguments as the current queue, the next queue and the pointers to their corresponding indices. Along with this, it takes two number, ```cur_q_id, next_q_id``` which represent the priority of the current queue and the next queue. 
The ```mlfq``` function is called in the ```scheduler``` function ([here](https://github.com/uditvyas/xv6-MLFQ/blob/master/proc.c#L508)) for every consecutive pairs of priority queues. 
#### [Boost function](https://github.com/uditvyas/xv6-MLFQ/blob/master/proc.c#L363)
This is a function to boost the priority of all the processes in the process table to the highest priority upon timer interrupt. 

## System Call : [int getpinfo(struct pstat*)](https://github.com/uditvyas/xv6-MLFQ/blob/master/getpinfo.c) 

In order to introduce a new system, we modify the following files:
* [sysproc.c](https://github.com/uditvyas/xv6-MLFQ/blob/master/sysproc.c#L98)
* [syscall.h](https://github.com/uditvyas/xv6-MLFQ/blob/master/syscall.h#L24)
* [syscall.c](https://github.com/uditvyas/xv6-MLFQ/blob/master/syscall.c#L136)
* [Makefile](https://github.com/uditvyas/xv6-MLFQ/blob/master/Makefile)
* [user.h](https://github.com/uditvyas/xv6-MLFQ/blob/master/user.h#L30)
* [defs.h](https://github.com/uditvyas/xv6-MLFQ/blob/master/defs.h#L131)

This system call returns basic information about each process: its process ID, how many timer ticks it has acquired at each level, which queue it is currently placed on (0, 1, 2, or 3), and its current procstate (e.g., ```SLEEPING, RUNNABLE, or RUNNING```)

## Testing Methods
We have included two files to test the MLFQ scheduler.
1. [foo.c](https://github.com/uditvyas/xv6-MLFQ/blob/master/foo.c) - This file generates a simple workload. While running this file, ensure that the logging paramter is changes from 0 to 1 in [params.h](https://github.com/uditvyas/xv6-MLFQ/blob/master/param.h#L14)
2. [getPinfo.c](https://github.com/uditvyas/xv6-MLFQ/blob/master/getPinfo.c) - This file creates a simple workload and returns the number of ticks spent by the process in each priority level. Ensure that logging is turned to 0 in params.h.
 



