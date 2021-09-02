#include <netdb.h>
#include <sys/socket.h>
#include <iostream>
#include <arpa/inet.h>

int main(int argc, char **argv){
   if (argc < 2)return 1;

   struct hostent* host=gethostbyname(argv[1]);
   char ipaddr[16];

   if (host==0x00)return 2;

   inet_ntop(AF_INET, host->h_addr, ipaddr, 16);
   printf ("%s\n", ipaddr);

   return 0;
}
