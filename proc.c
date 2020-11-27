#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"



// Defining the process queues and the corresponding maximum ticks
int clkPerPrio[4] ={1,2,4,8};
int q0 = -1;
int q1 = -1;
int q2 = -1;
int q3 = -1;
int *p0 = &q0;
int *p1 = &q1;
int *p2 = &q2;
int *p3 = &q3;
struct proc *q_0[64];
struct proc *q_1[64];
struct proc *q_2[64];
struct proc *q_3[64];

int flag0 = 0;
int flag1 = 0;
int flag2 = 0;

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  
  struct proc *p;
  char *sp;
  
  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
    {
      
      // Allocating new peocess a priority in initial ticks as 0 in all priority levels
      p->priority = 0;
      goto found;
    }
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  p->priority = 0;  //default
  p->myticks[0] = p->myticks[1] = p->myticks[2] = p->myticks[3] = 0;
  // q0++;
  (*p0)++;
  q_0[*p0] = p;
  // enQueue(Q_0,p);

  //cprintf("NEW PROCESS INITIALIZED PID : %d \t State : %d \t Q0 : %d !!\n",p->pid,p->state,*p0);
  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{   
  // Q_0 = createQueue();
  // Q_1 = createQueue();
  // Q_2 = createQueue();
  // Q_3 = createQueue();
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;
  

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;
  flag0=1;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

/*
BOOST
*/
void
Boost(void)
{ 
  // cprintf("Boosting\n");
  sti();
  acquire(&ptable.lock);
  //getpinfo(NULL);
  struct proc *p;
  // QNode *node;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    // int d = p->priority;
    if (p->priority!=0){
      //cprintf("%d\n",p->priority);
      
      p->priority = 0;
      // q0++;
      (*p0)++;
      q_0[*p0] = p;   
      // enQueue(Q_0,p);
    }
    flag0 = flag1 = flag2 = 0;
    p->myticks[0] = p->myticks[1] = p->myticks[2] = p->myticks[3] = 0;
  }

  for(int i=0;i<*p1;i++)
  {
    q_1[i] = NULL;
  }
  for(int i=0;i<*p2;i++)
  {
    q_2[i] = NULL;
  }
  for(int i=0;i<*p3;i++)
  {
    q_3[i] = NULL;
  }
  // Remove all process from other queues
  // q1=q2=q3=-1;
  *p1=*p2=*p3=-1;

  cprintf("\nBOOST DONE\n\n");
  release(&ptable.lock);
}


void
check_unused(struct proc **q_c,int *current)
{
  struct proc *p;  
  for(int i=0;i<*current+1;i++){
  
    p = q_c[i];
    
    if(p->state == UNUSED){
      int j;
      for(j=i;j<*current;j++){
	      q_c[j] = q_c[j+1];
      }    
      q_c[j] = NULL;
      (*current)--;
      
    }
  }
}


/*
MLFQ ROUND ROBIN IMPLEMENTATION
*/
void 
mlfq(struct proc **q_current,struct proc **q_next,int *current, int *next,struct cpu *c, int num1, int num2)
{

  // check_unused(q_current,current);
  struct proc *p;  
  
  for(int i=0;i<*current+1;i++){
    if(flag0)  return;
    if(flag1==1 && num1>=1)  return;
    if(flag2==1 && num2>=2)  return;

  // while(i<*current+1){
    p = q_current[i];
    // rerun:
    if(p->state != RUNNABLE){
      // cprintf("PState %d\n",p->state);
      continue;
    }
    check_unused(q_current,current);
    c->proc = p;

    // if(num1!=p->priority)cprintf("ERROR PROCPrio: %d \t REQPrio : %d \t PROCName : %s!!\n",p->priority,num1,p->name);
  
    switchuvm(p);
    p->state = RUNNING;

    swtch(&(c->scheduler), p->context);
    switchkvm();
  
    if(p->myticks[p->priority] == clkPerPrio[p->priority]){

      (*next)++;
      int d = p->priority;
      if (p->priority!=3){
        p->priority=p->priority+1;}    
      
      cprintf("process %s, %d going to priority %d from priority %d after ticks %d \n ",p->name,p->pid,p->priority,d,p->myticks[d]);      

      struct proc *t;
      for(t=ptable.proc;t<&ptable.proc[NPROC]; t++){
	      if(t->state == SLEEPING)
 		      cprintf("%s \t %d \t SLEEPING \t %d\n",t->name,t->pid,t->priority);
	      else if(t->state == RUNNING)
		      cprintf("%s \t %d \t RUNNING \t %d\n", t->name,t->pid,t->priority);
	      else if(t->state == RUNNABLE)
		      cprintf("%s \t %d \t RUNNABLE \t %d\n", t->name,t->pid,t->priority);
      }

      p->myticks[d] = 0;
	    q_next[*next] = p;

      int j;
      for(j=i;j<*current;j++){
	      q_current[j] = q_current[j+1];
        }     
	  
      i--;
      q_current[j] = NULL;
	    (*current)--;      
      
    }
    else if(p->state == RUNNABLE)
    {
      i--;
      //cprintf("process %s: pid :%d of priority: %d  after ticks: %d \n ",p->name,p->pid,p->priority,p->myticks[p->priority]);     
    }
    // else
    // {
    //    
    //   goto rerun;
    // }
    
    // i++;  
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){

    sti();
    
    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
   
    flag0 = flag1 = flag2 = 0;
    if(*p0!=-1){
      // cprintf("Queue : %d \t pointer : %d \t\n",q0,*p0);
      mlfq(q_0,q_1,p0,p1,c, 0,1);
    }

    // cprintf("QUEUE 0 COMPLETED !!\n");
    // cprintf("QUEUE 1 STARTED !!\n");

    if(*p1!=-1){
      mlfq(q_1,q_2,p1,p2,c, 1,2);
    }

    // cprintf("QUEUE 1 COMPLETED !!\n");
    // cprintf("QUEUE 2 STARTED !!\n");

    if(*p2!=-1){
      mlfq(q_2,q_3,p2,p3,c,2,3);
    }

    // cprintf("QUEUE 2 COMPLETED !!\n");
    // cprintf("QUEUE 3 STARTED !!\n");

    if(*p3!=-1){
      mlfq(q_3,q_3,p3,p3,c,3,3); 
    }
    c->proc = 0;
    
    // cprintf("QUEUE 3 COMPLETED !!\n");
    release(&ptable.lock);
  }
}



// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  if(myproc()->priority==0)
  {
    cprintf("boosting");
    flag0=1;
  }
  if(myproc()->priority==1)
  {
    cprintf("boosting");
    flag1=1;
  }
  if(myproc()->priority==2)
  {
    cprintf("boosting");
    flag2=1;
  }
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
    { 
      p->state = RUNNABLE;

      if(p->priority == 0){
        flag0 = 1;
      }
      else if(p->priority == 1){
        flag1 = 1;
      }
      else if(p->priority == 2){
        flag2 = 1;
      }
     

      // else if(p->priority == 1){
      //   // q1++;
      //   (*p1)++;
      //   for(int i=*p1;i>0;i--){
      //     q_1[i] = q_1[i-1];
      //   }
      //   q_1[0] = p;
      // }
      // else if(p->priority == 2){
      //   // (*p2)++;
      //   for(int i=*p2;i>0;i--){
      //     q_2[i] = q_2[i-1];
      //   }
      //   q_2[0] = p;
      // }
      // else{
      //   (*p3)++;
      //   for(int i=*p3;i>0;i--){
      //     q_3[i] = q_3[i-1];
      //   }
      //   q_3[0] = p;
      // }
    }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  int i=0;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;

      //pstat_xyz->inuse[i] = 0;
      release(&ptable.lock);      
      return 0;
    }
    i++;
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

int cps()
{
    struct proc *p;
    sti();
    acquire(&ptable.lock);
    cprintf("name \t pid \t state \t\t priority \n");
    for(p=ptable.proc;p<&ptable.proc[NPROC]; p++){
	if(p->state == SLEEPING)
 		cprintf("%s \t %d \t SLEEPING \t %d\n",p->name,p->pid,p->priority);
	else if(p->state == RUNNING)
		cprintf("%s \t %d \t RUNNING \t %d\n",p->name,p->pid,p->priority);
	else if(p->state == RUNNABLE)
		cprintf("%s \t %d \t RUNNABLE \t %d\n",p->name,p->pid,p->priority);
 } 
release(&ptable.lock);
return 22;  
}


int
getpinfo(struct pstat* pstat)
{ 
  
  acquire(&ptable.lock);
  int i=0;
    struct proc *proc_pstat;  
    for(proc_pstat = ptable.proc; proc_pstat < &ptable.proc[NPROC]; proc_pstat++){
      
      if ((proc_pstat->state != UNUSED) && (proc_pstat->state != EMBRYO) && (proc_pstat->state!= ZOMBIE))
      {
        pstat->inuse[i] = 1;
        pstat->pid[i] = proc_pstat->pid;
        pstat->priority[i] = proc_pstat->priority;
        pstat->state[i] = proc_pstat->state;
        safestrcpy(pstat->name[i],proc_pstat->name, sizeof(proc_pstat->name));
        int j;
        for(j = 0; j < 4; ++j){
          pstat->ticks[i][j] = proc_pstat->myticks[j];
        }
        // cprintf("%d \t\t %d \n",pstat->pid[i], pstat->priority[i]);
      // cprintf("\n%d \t %s \t\tSLEEPING \t\t %d \t\t %d \n",pstat->pid[i],pstat->name[i], pstat->priority[i],pstat->ticks[i][pstat->priority[i]]);
      }
      
      i++;
    }
    
  //   cprintf("pid \t name \t\t state \t\t priority \t\t ticks \n");
  //   for(int i=0;i<64;i++){
  //       if(pstat->inuse[i] == 1){
  //         cprintf("i is %d",i);
  //       if(pstat->state[i] == SLEEPING)
  //           cprintf("\n%d \t %s \t\tSLEEPING \t\t %d \t\t %d \n",pstat->pid[i],pstat->name[i], pstat->priority[i],pstat->ticks[i][pstat->priority[i]]);
  //       else if(pstat->state[i] == RUNNING)
  //           cprintf("\n%d \t %s \t\tRUNNING \t\t %d \t\t %d \n",pstat->pid[i],pstat->name[i], pstat->priority[i],pstat->ticks[i][pstat->priority[i]]);
  //       else if(pstat->state[i] == RUNNABLE)
  //           cprintf("\n%d \t %s \t\tRUNNABLE \t\t %d \t\t %d \n",pstat->pid[i],pstat->name[i], pstat->priority[i],pstat->ticks[i][pstat->priority[i]]);
  //       else if(pstat->state[i] == ZOMBIE)
  //           cprintf("\n%d \t %s \t\tZOMBIE \t\t %d \t\t %d \n",pstat->pid[i],pstat->name[i], pstat->priority[i],pstat->ticks[i][pstat->priority[i]]);
  //       // else if(pstat->state[i] == UNUSED)
  //       //     cprintf("%d \t %s \t\tUNUSED \t\t %d \t\t %d \n",pstat->pid[i],pstat->name[i], pstat->priority[i],pstat->ticks[i][pstat->priority[i]]);
  //       else if(pstat->state[i] == EMBRYO)
  //           cprintf("\n%d \t %s \t\t EMBRYO \t\t %d \t\t %d \n",pstat->pid[i],pstat->name[i], pstat->priority[i],pstat->ticks[i][pstat->priority[i]]);

  //       }
  //   }
  // cprintf("address in %p\n",pstat);
  release(&ptable.lock);

  
  return 0;
}