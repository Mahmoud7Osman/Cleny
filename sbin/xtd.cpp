#include <stdio.h>
#include <stdlib.h>

int main(int argc, char*argv[]){
   for (int i=1; i<argc; i++)
        printf ("%d ", strtol(argv[i], NULL, 16));

    printf ("\n");
}
