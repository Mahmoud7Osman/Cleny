#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "base/Base.h"

int main(int argc, char** argv){
	File file(argv[1]);
	printf("%s", file.data);
}
