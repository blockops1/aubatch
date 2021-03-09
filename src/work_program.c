//work_program.c 
  
#include<stdio.h> 
#include <stdlib.h>
#include<unistd.h> 
  
int main(int argc, char *argv[]){ 
    //printf("Running program: ");
    //printf("job id:%s ", argv[1]);
    //printf(" starting time:%f ", argv[2]);
    //printf(" running time:%s ", argv[3]);
    //printf("\n"); 
    sleep(atof(argv[1]));
    return 0; 
} 