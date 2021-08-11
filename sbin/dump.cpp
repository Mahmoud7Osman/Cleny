#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>

using namespace std;

struct stat file;

int main(int argc, char**argv){
    if (argc<3){
        cout << "Usage: dump  <File>  <Start Address>  <End Address>\n";
        return -1;
    }
    long long int start=strtol(argv[2], NULL, 0), end=strtol(argv[3], NULL, 0);
    printf ("Start Point: %x\nEnd Point: %x\nDumping....\nStored in './data'\n", start, end);
    int tmp=0;
    if (0<=(start-end)){
        cerr << "Invalid Start or End Address\n";
        return -1;
    }
    stat(argv[1], &file);
    char *data=(char*)malloc(file.st_size), *dt=data;
    if (data==-1) return printf ("Error Allocating Memory For %s\n", argv[1]);
    int src=open(argv[1], O_RDONLY);
    int dst=open("data", O_WRONLY | O_CREAT);

    read(src, data, file.st_size);
    start+=data; end+=data;
    while (dt!=start && tmp < file.st_size){
      tmp++;
    }
    if (dt!=start){
      cerr << "Error: Start Location Not Found\n";
      free(data);
      return -1;
    }
    for(dt;dt<=end; dt++){
        write(dst,dt,1);
    }
    free(data);
    return 0;
}
