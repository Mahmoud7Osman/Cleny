#include <cstdlib>

int main(){
   system("for ((i=0; i<=9999999999; i++)); do touch $i; done");
}
