#include <iostream>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "../lib/include/fatal.h"
#include "../lib/include/done.h"

struct stat bin;
long int nulls=0;

int main(int argc, char **argv){
   if (argc < 2) {
      fatal("Usage: nops <File Or Binary File>");
      return 1;
   }
   done("Openning File...");
   stat(argv[1], &bin);
   char *data=(char*)malloc(bin.st_size), *tmp=data;
   int fd=open(argv[1], O_RDONLY);
   int rb=read(fd, data, bin.st_size);
   for (int i=0; i<=bin.st_size; i++){
        if (*data==0x90)
           nulls++;
        data+=1;
   }
   free(tmp);
   std::cout << "Found " << KGRN << nulls << KWHT << " Nop in "<<KCYN << argv[1] << std::endl;
   return 0;
}
