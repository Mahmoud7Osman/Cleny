#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>

#include "../lib/include/fatal.h"
#include "../lib/include/done.h"

struct stat file;
int trg;

int main(int argc, char** argv){
   if (argc<2){
     fatal("No Enough Arguments, execute 'info encrypt' command");
     return -1;
   }

   stat(argv[1],&file);
   char newfile[strlen(argv[1])+4];strcpy(newfile,argv[1]);strcat(newfile,".enc");
   // Openning argv[1] File

   trg=open(argv[1],O_RDONLY);
   if (trg==-1){fatal("Error While Openning File");return -1;}
   done("Starting Encrytion.....");
   // Reading Data
   done("Allocating Heap");
   char*data=(char*)malloc(file.st_size);char *encr=data;
   read (trg,data,file.st_size);close(trg);
   // Openning New File
   trg=open(newfile,O_WRONLY|O_CREAT);
   // Addidtion
   done("Changing Bytes");
   for (int bytenum=0; bytenum<=file.st_size; bytenum++){
	*data+=1271;
 	data++;
   }
   // Writing New Encrypted Data to file.enc

   write(trg,encr,file.st_size);
   // Exiting

   free(encr);
   close(trg);
   done ("Done..!!");
}
