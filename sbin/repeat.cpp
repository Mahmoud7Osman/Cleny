#include <iostream>

int main(int argc, char** argv){
   if (argc < 3){
     std::cerr << "Usage: repeat <string> <times>\n";
     return 1;
   }

   int count=atoi(argv[2]);
   for (int repeat=0; repeat<count; repeat++)
     std::cout << argv[1];
   return 0;
}
