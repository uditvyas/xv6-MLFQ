// #include "types.h"
// #include "stat.h"
// #include "user.h"
// #include "proc.h"
// struct pstat pstat_var;
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#define NULL ((void *)0)
// #include "user.h"
// #include <stdlib.h>

int
main(int argc, char *argv[])
{   
    struct pstat* pstat_var = NULL;

    // cprintf("upar\n");
    // pstat_var = (struct pstat*)malloc(sizeof(struct pstat*));
    // cprintf("lower\n");
    getpinfo(pstat_var);
    exit();
}