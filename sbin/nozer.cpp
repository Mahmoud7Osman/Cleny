#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>

#include "../lib/fatal.h"
#include "../lib/done.h"

struct stat file;

int Run(char *pname, char *args){
    execve(pname, args, NULL);
}

char* LoadShellcode(char *fname, struct stat *f){

   char *sc=(char*)malloc(f->st_size);

   if ((int fd=open(fname, O_RDONLY))==-1){
      fatal("Error While Openning File.");
      exit(1);
   }

   char *sc=(char*)malloc(f->st_size);

   if ((int data=read(fd, sc, f->st_size))==-1){
      fatal("Error While Reading File");
      exit(1);
   }

   return sc;
}

int main(int argc, char **argv){

   stat(argv[1], file);

   char *shellcode=LoadShellcode(argv[1], &file);
   char *command  =(char*)malloc(file.st_size+sizeof(long int));
   long int ret=strtol(argv[1], NULL, 16);

   memset(command+file.st_size, ret, sizeof(ret));

   Run(argv[1]);

   free(command);
   free(shellcode);
}
