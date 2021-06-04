#define DP "PODGL"

#include "../lib/include/fatal.h"
#include "../lib/include/done.h"
#include "../lib/include/colors.h"

#include <cstring>
#include <iostream>

#include <unistd.h>

using namespace std;

int error(){
   fatal("Usage: password <New Password>");
   return 1;
}

int main(int argc, char *argv[]){
     if (argc < 2) return error();
     const char *pwd=getenv(DP);
     chdir(pwd);
     FILE *pf=fopen("etc/login/pass.dl","w");
     fwrite(argv[1], strlen(argv[1]), 1, pf);
     return fclose(pf);
}
