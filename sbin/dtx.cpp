#include <stdio.h>
#include <stdlib.h>

int main(int argc, char*argv[]){
   for (int i=1; i<argc; i++)
        printf ("0x%x ", atoi(argv[i]));

    printf ("\n");
}
