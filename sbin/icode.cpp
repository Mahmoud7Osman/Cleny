#include <iostream>
#include <cstring>

#define RTD_CODE 0x1


using namespace std;

struct code{
     char    *ipaddr;
     char   code[16];
};
struct code me;

void CreateRoomCode(){
   for (int tmp=0; tmp<strlen(me.ipaddr); tmp++){
      me.code[tmp]=(int)me.ipaddr[tmp]-RTD_CODE;
   }
}

int main(int argc,char** argv){

  if (argc < 2)
    return printf("Usage: icode <IP Address>\n");

  me={.ipaddr=argv[1]};

  CreateRoomCode();

  cout << me.code   << endl;
  cout << me.ipaddr << endl;
}
