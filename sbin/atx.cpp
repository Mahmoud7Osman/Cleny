#include <stdio.h>

int main(int argc, char*argv[]){
   for (int i=1; i<argc; i++){
     for (char *t=argv[i]; *t!=0;t++)
       printf ("%x ", *t);
   }
    printf ("\n");
}
