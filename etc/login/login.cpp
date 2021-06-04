#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <sys/stat.h>

#include "../../lib/include/fatal.h"

struct stat fs;
int main(void){
     stat("pass.dl", &fs);
     char *curr=(char *)malloc(fs.st_size), *pass=(char*)malloc(fs.st_size);
     FILE *fptr=fopen("pass.dl", "r");
     std::cout << "Password: ";
     std::cin  >> pass;
     fread((void*)curr, sizeof(pass), 1, fptr);
//   memset(curr+(fs.st_size-1),0x0,1);
     if (strcmp(curr, pass)==0){
        free(curr);
        free(pass);
	return 0;
     }
     free(pass);
     free(curr);
     return 1;
}

