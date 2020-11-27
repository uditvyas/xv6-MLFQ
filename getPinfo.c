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




#define check(exp, msg) if(exp) {} else {       \
  printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg); \
  exit();}

int pow2[] = {80000000, 32, 16, 8};

int workload(int n) {
  int i, j = 0;
  for(i = 0; i < n; i++)
    j += i * j + 1;
  return j;
}

int
main(int argc, char *argv[])
{
  struct pstat st;

  sleep(10);

  int i, j, k, count = 0;
  for (i = 0; i <= 60; i++) {
    if (fork() == 0) {
      workload(4000000 * (i + 1));
      if (i == NPROC - 4) {
        sleep(100);
        check(getpinfo(&st) == 0, "getpinfo");

        // See what's going on...
         
        for(k = 0; k < NPROC; k++) {
          if (st.inuse[k]) {
            int m;
            printf(1, "pid: %d\n", st.pid[k]);
            if (st.pid[k] > 3) {
                printf(1,"state of pid: %d is %d\n",st.pid[k],st.state[k]);
                check(st.ticks[k][0] > 0, "Every process at the highest level should use at least 1 timer tick");
            }
            for (m = 0; m < 3; m++) {
                
              printf(1, "\t level %d ticks used %d\n", m, st.ticks[k][m]);
            }
          }
        }
        
        

        for(k = 0; k < NPROC; k++) {
          if (st.inuse[k]) {
            count++;
            check(st.priority[k] <= 3 && st.priority[k] >= 0, "Priority should be 0, 1, 2 or 3");
            for (j = 0; j < st.priority[k]; j++) {
              if (st.ticks[k][j] != pow2[j]) {
                printf(1, "#ticks at this level should be %d, \
                    when the priority of the process is %d. But got %d\n", 
                    pow2[j], st.priority[k], st.ticks[k][j]);
                exit();
              }
            }
            if (st.ticks[k][j] > pow2[j]) {
              printf(1, "#ticks at level %d is %d, which exceeds the maximum #ticks %d allowed\n", j, st.ticks[k][j], pow2[j]);
              exit();
            }
          }
        }
        check(count == NPROC, "Should have 64 processes currently in used in the process table.");
        printf(1, "TEST PASSED");
      }
    } else {
      wait();
      break;
    }
  }

  exit();
}












// int
// main(int argc, char *argv[])
// {
//     struct pstat st;

//     if(argc != 2){
//         printf(1, "usage: mytest counter");
//         exit();
//     }

//     int i, x, l, j;
//     int mypid = getpid();

//     for(i = 1; i < atoi(argv[1]); i++){
//         x = x + i;
//     }

//     getpinfo(&st);
//     for (j = 0; j < NPROC; j++) {
//         if (st.inuse[j] && st.pid[j] >= 2 && st.pid[j] == mypid) {
//             for (l = 0; l<4; l++) {
//                 printf(1, "level:%d \t ticks-used:%d\n", l, st.ticks[j][l]);
//             }
//         }
//     }
    
//     exit();
//     return 0;
// }








// {   
//     // char *temp = "hello";
//     struct pstat pstat;


//     // }
//     // cprintf("upar\n");
//     // pstat = (struct pstat*)malloc(sizeof(struct pstat*));
//     // cprintf("lower\n");
//     int i = 4;
//     i = getpinfo(&pstat);
//     printf(1, "pid \t name \t\t state \t\t priority \t\t ticks \n");
//     printf(1, "%d\n",i);
//     // printf(1, "%d", pstat.priority[1]);
//     // printf(1, "%d", pstat->priority[1]);

//     for(int i=0;i<64;i++){
//         if(pstat.inuse[i] == 1){
//         if(pstat.state[i] == SLEEPING)
//             printf(1, "\n%d \t %s \t\tSLEEPING \t\t %d \t\t %d \n",pstat.pid[i],pstat.name[i], pstat.priority[i],pstat.ticks[i][pstat.priority[i]]);
//         else if(pstat.state[i] == RUNNING)
//             printf(1, "\n%d \t %s \t\tRUNNING \t\t %d \t\t %d \n",pstat.pid[i],pstat.name[i], pstat.priority[i],pstat.ticks[i][pstat.priority[i]]);
//         else if(pstat.state[i] == RUNNABLE)
//             printf(1, "\n%d \t %s \t\tRUNNABLE \t\t %d \t\t %d \n",pstat.pid[i],pstat.name[i], pstat.priority[i],pstat.ticks[i][pstat.priority[i]]);
//         else if(pstat.state[i] == ZOMBIE)
//             printf(1, "\n%d \t %s \t\tZOMBIE \t\t %d \t\t %d \n",pstat.pid[i],pstat.name[i], pstat.priority[i],pstat.ticks[i][pstat.priority[i]]);
//         //else if(pstat.state[i] == UNUSED)
//             //cprintf("%d \t %s \t\tUNUSED \t\t %d \t\t %d \n",pstat.pid[i],pstat.name[i], pstat.priority[i],pstat.ticks[i][pstat.priority[i]]);
//         else if(pstat.state[i] == EMBRYO)
//             printf(1, "\n%d \t %s \t\t EMBRYO \t\t %d \t\t %d \n",pstat.pid[i],pstat.name[i], pstat.priority[i],pstat.ticks[i][pstat.priority[i]]);
//         }
//     }

//     exit();
// }