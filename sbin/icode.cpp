#include <iostream>
#include <cstring>
#include <netdb.h>
#include <arpa/inet.h>
#include "../lib/include/colors.h"
#include "../lib/include/smf.h"

#define RTD_CODE 0x1


using namespace std;

struct code{
     char    *ipaddr;
     char      *port;
     char   code[16];
};
struct code me;

void GetRoomCode(){
   int tmp, c;

   for (tmp=0; tmp<strlen(me.ipaddr); tmp++)
      me.code[tmp]=(int)me.ipaddr[tmp]-RTD_CODE;

   me.code[tmp]=0x5c;

   tmp++;
   c=tmp;

   for(tmp; tmp<22; tmp++)
      me.code[tmp]=(int)me.port[tmp-c]-RTD_CODE;

   (void) memset(me.code+strlen(me.ipaddr)+1+strlen(me.port), 0x00, 0x01);
}

int main(int argc,char** argv){

  if (argc < 3)
    return printf("(iChat) Usage: icode <IP Address> <Port Number>\n");

  me={.ipaddr=argv[1], .port=argv[2]};
  cout << "(iChat) Getting Room Code ...\n";
  GetRoomCode();
  cout << "Room Code: "<< KGRN << me.code << endl;
}
