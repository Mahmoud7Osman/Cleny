#include <iostream>
#include <cstring>

#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include <stdlib.h>
#include <stdio.h>


// Functions ProtoTypes
int InjectProcess(pid_t pid, char *sc, int size);
//

int main(int argc, char **argv){

}


int InjectProcess(pid_t pid, char *sc, int size){
    struct user_regs_struct regs;
    ptrace(PTRACE_ATTACH , pid,0,0);
    ptrace(PTRACE_GETREGS, pid,0,regs);
    for (int tmp=0; tmp<=size; tmp++){
        ptrace(PTRACE_POKETEXT, pid, regs.rip+tmp, sc+tmp);
    }
    ptrace(PTRACE_DETACH, pid, 0, 0);
    return 0;
}
