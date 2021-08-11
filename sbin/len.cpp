#include <cstring>
#include <iostream>

int main(int argc, char**argv){
   if (argc < 2) return printf ("Error: No Arguments\n");
   return printf ("%d\n", strlen(argv[1]))-1;
}
