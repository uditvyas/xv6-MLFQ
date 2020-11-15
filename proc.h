#ifndef PROC_H
#define PROC_H
#define NULL ((void *)0)
// #include "user.h"

// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
  
  int priority;                // Priority of the process
  int myticks[4];              // No of ticks of the process in all the queues
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap

struct pstat{
  int inuse[NPROC]; // whether this slot of the process table is in use (1 or 0)
  int pid[NPROC];   // PID of each process
  int priority[NPROC];  // current priority level of each process (0-3)
  enum procstate state[NPROC];  // current state (e.g., SLEEPING or RUNNABLE) of each process
  int ticks[NPROC][4];  // number of ticks each process has accumulated at each of 4 priorities
  char name[NPROC][16]; // Name of the process
};
/*Global Definition of queues and pointers to queues sizes*/
extern int q0;
extern int q1;
extern int q2;
extern int q3;
extern int *p0;
extern int *p1;
extern int *p2;
extern int *p3;
extern uint diff;

// #################################################################################
// typedef long Align;
// typedef union header Header;
// union header {
//   struct {
//     union header *ptr;
//     uint size;
//   } s;
//   Align x;
// };



// // static Header base;
// // static Header *freep;

// // void*
// // xv6_malloc(uint nbytes)
// // {
// //   Header *p, *prevp;
// //   uint nunits;

// //   nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
// //   if((prevp = freep) == 0){
// //     base.s.ptr = freep = prevp = &base;
// //     base.s.size = 0;
// //   }
// //   for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
// //     if(p->s.size >= nunits){
// //       if(p->s.size == nunits)
// //         prevp->s.ptr = p->s.ptr;
// //       else {
// //         p->s.size -= nunits;
// //         p += p->s.size;
// //         p->s.size = nunits;
// //       }
// //       freep = prevp;
// //       return (void*)(p + 1);
// //     }
// //     if(p == freep)
// //       if((p = xv6_morecore(nunits)) == 0)
// //         return 0;
// //   }
// // }

// // static Header*
// // xv6_morecore(uint nu)
// // {
// //   char *p;
// //   Header *hp;

// //   if(nu < 4096)
// //     nu = 4096;
// //   p = sbrk(nu * sizeof(Header));
// //   if(p == (char*)-1)
// //     return 0;
// //   hp = (Header*)p;
// //   hp->s.size = nu;
// //   xv6_free((void*)(hp + 1));
// //   return freep;
// // }

// // void
// // xv6_free(void *ap)
// // {
// //   Header *bp, *p;

// //   bp = (Header*)ap - 1;
// //   for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
// //     if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
// //       break;
// //   if(bp + bp->s.size == p->s.ptr){
// //     bp->s.size += p->s.ptr->s.size;
// //     bp->s.ptr = p->s.ptr->s.ptr;
// //   } else
// //     bp->s.ptr = p->s.ptr;
// //   if(p + p->s.size == bp){
// //     p->s.size += bp->s.size;
// //     p->s.ptr = bp->s.ptr;
// //   } else
// //     p->s.ptr = bp;
// //   freep = p;
// // }

// typedef struct QNode { 
// 	struct proc *process; 
// 	struct QNode* next; 
// }QNode; 

// // The queue, front stores the front node of LL and rear stores the 
// // last node of LL 
// struct Queue { 
// 	struct QNode *front, *rear; 
// }; 

// // A utility function to create a new linked list node. 
// struct QNode* newNode(struct proc* k) 
// { 
// 	struct QNode* temp = (struct QNode*)xv6_malloc(sizeof(struct QNode)); 
// 	temp->process = k; 
// 	temp->next = NULL; 
// 	return temp; 
// } 

// // A utility function to create an empty queue 
// struct Queue* createQueue() 
// { 
// 	struct Queue* q = (struct Queue*)xv6_malloc(sizeof(struct Queue)); 
// 	q->front = q->rear = NULL; 
// 	return q; 
// } 

// // The function to add a key k to q 
// void enQueue(struct Queue* q, struct proc *k) 
// { 
// 	// Create a new LL node 
// 	struct QNode* temp = newNode(k); 

// 	// If queue is empty, then new node is front and rear both 
// 	if (q->rear == NULL) { 
// 		q->front = q->rear = temp; 
// 		return; 
// 	} 

// 	// Add the new node at the end of queue and change rear 
// 	q->rear->next = temp; 
// 	q->rear = temp; 
// } 

// // Function to remove a key from given queue q 
// void deQueue(struct Queue* q) 
// { 
// 	// If queue is empty, return NULL. 
// 	if (q->front == NULL) 
// 		return; 

// 	// Store previous front and move front one node ahead 
// 	struct QNode* temp = q->front; 

// 	q->front = q->front->next; 

// 	// If front becomes NULL, then change rear also as NULL 
// 	if (q->front == NULL) 
// 		q->rear = NULL; 

// 	xv6_free(temp); 
// } 

// // Driver Program to test anove functions 
// // int main() 
// // { 
// // 	struct Queue* q = createQueue(); 
// // 	enQueue(q, 10); 
// // 	enQueue(q, 20); 
// // 	deQueue(q); 
// // 	deQueue(q); 
// // 	enQueue(q, 30); 
// // 	enQueue(q, 40); 
// // 	enQueue(q, 50); 
// // 	deQueue(q); 
// // 	printf("Queue Front : %d \n", q->front->key); 
// // 	printf("Queue Rear : %d", q->rear->key); 
// // 	return 0; 
// // } 

#endif /* PROC_H */
