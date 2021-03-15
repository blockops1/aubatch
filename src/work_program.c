//work_program.c 
  
#include<stdio.h> 
#include <stdlib.h>
#include<unistd.h> 
  
int main(int argc, char *argv[]){ 
    usleep((double)atof(argv[1]) * 1000000);
    return 0; 
} 