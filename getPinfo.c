// #include "types.h"
// #include "stat.h"
// #include "user.h"
// #include "proc.h"
// struct pstat pstat_var;
#include "types.h"
#include "stat.h"

// #include "defs.h"
#include "mmu.h"
#include "param.h"

// #include "memlayout.h"

// #include "x86.h"
#include "proc.h"
#include "user.h"

#define NULL ((void *)0)
// #include "user.h"
// #include <stdlib.h>

int
main(int argc, char *argv[])
{   
    struct pstat pstat_var;

    // cprintf("upar\n");
    // pstat_var = (struct pstat*)malloc(sizeof(struct pstat*));
    // cprintf("lower\n");
    getpinfo(&pstat_var);
    printf(1, "pid \t name \t\t state \t\t priority \t\t ticks \n");
    printf(1, "%d", pstat_var.state[1]);
    for(int i=0;i<64;i++){
        if(pstat_var.inuse[i] == 1){
        if(pstat_var.state[i] == SLEEPING)
            printf(1, "\n%d \t %s \t\tSLEEPING \t\t %d \t\t %d \n",pstat_var.pid[i],pstat_var.name[i], pstat_var.priority[i],pstat_var.ticks[i][pstat_var.priority[i]]);
        else if(pstat_var.state[i] == RUNNING)
            printf(1, "\n%d \t %s \t\tRUNNING \t\t %d \t\t %d \n",pstat_var.pid[i],pstat_var.name[i], pstat_var.priority[i],pstat_var.ticks[i][pstat_var.priority[i]]);
        else if(pstat_var.state[i] == RUNNABLE)
            printf(1, "\n%d \t %s \t\tRUNNABLE \t\t %d \t\t %d \n",pstat_var.pid[i],pstat_var.name[i], pstat_var.priority[i],pstat_var.ticks[i][pstat_var.priority[i]]);
        else if(pstat_var.state[i] == ZOMBIE)
            printf(1, "\n%d \t %s \t\tZOMBIE \t\t %d \t\t %d \n",pstat_var.pid[i],pstat_var.name[i], pstat_var.priority[i],pstat_var.ticks[i][pstat_var.priority[i]]);
        //else if(pstat_var.state[i] == UNUSED)
            //cprintf("%d \t %s \t\tUNUSED \t\t %d \t\t %d \n",pstat_var.pid[i],pstat_var.name[i], pstat_var.priority[i],pstat_var.ticks[i][pstat_var.priority[i]]);
        else if(pstat_var.state[i] == EMBRYO)
            printf(1, "\n%d \t %s \t\t EMBRYO \t\t %d \t\t %d \n",pstat_var.pid[i],pstat_var.name[i], pstat_var.priority[i],pstat_var.ticks[i][pstat_var.priority[i]]);

        }
    }

    exit();
}