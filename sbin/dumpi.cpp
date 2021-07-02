#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <cstdio>
void DumpPidInstructions(pid_t pid){
        struct user_regs_struct regs;
        ptrace(PTRACE_ATTACH, pid, 0, 0);
        int status;
        waitpid(4921, NULL, NULL);
        long int i;
        while (ptrace(PTRACE_GETREGS, pid, 0, &regs) != -1){
             printf ("%x\n", regs.rip+i);
             i++;
        }
}


int main(){
  pid_t pid=4921;
  DumpPidInstructions(pid);
}
