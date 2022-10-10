#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../base/defs.h"
#include "../base/Types.h"
#include "../headers/fontcli.h"
#include "../base/Base.h"
#include "../base/General.h"


int main(int argc, char** argv){
	if (argc == 1) return 1;

	File file(argv[1]);

	Syntax syntax(&file);

	syntax.scan();
}
