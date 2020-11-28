// TEST FILE, WHICH CREATES A CHILD AND A PARENT PROCESS
// BOTH PARENT AND THE CHILD HAVE CPU INTENSIVE JOBS 
// THE PARENT WAIT FOR THE CHILD TO FINISH BEFORE COMPLETING ITS EXECUTION

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(int argc, char *argv[]){
    int id = 0; 
    double x = 1;
    id = fork();
    if (id<0){
        printf(1,"%d failed in fork!\n",getpid());
        } 
    else if (id>0){
        while(x<1.2){
            x = x+0.000000005;
        }
        wait();
            
    }
    else{
        while(x<1.2){
            x = x+0.000000005;
        } 
    }
    exit();
}
