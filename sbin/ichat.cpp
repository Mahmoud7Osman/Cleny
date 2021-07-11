#include <iostream>
#include <cstring>
#include <thread>

#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>

#include "../lib/include/colors.h"

using std::thread;



// Prototypes
void CreateRoom(struct sockaddr_in* address);
void JoinRoom(struct sockaddr_in* address);

char *GetMsg(void);
void ProcMsg(char *msg);
void SyncMsg(struct sockaddr_in* address);
void SendMsg(char *msg);

void CheckLen(char *msg);

char* GetName(struct sockaddr_in* address, int code);
char* GetPath(void);

void JoinRoom(struct sockaddr_in* address, int port);
void InitInfo(struct sockaddr_in* address, int sfd );

char* StrRep(char* str, char* src, char* trg);
// END


// Global Variables
char *id;

int sfd, client;

struct sockaddr_in trg={.sin_family=AF_INET, .sin_port=5566};
socklen_t addrsize=sizeof(struct sockaddr_in);
// END


// Program
int main(int argc, char *argv[]){
        sfd=socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr={.sin_family=AF_INET, .sin_port=5566 ,.sin_addr.s_addr=0};
	return 0;
}


void CreateRoom(struct sockaddr_in* address, int fd){
    printf ("%sCreating Room...%s\n", KCYN, KNRM);
    if (bind(sfd, (struct sockaddr*)&address, sizeof(struct sockaddr_in))==-1){
       printf ("%sError Binding %siChat%s Socket%s", KCYN, BGRN, BBLK, KNRM);
       exit(1);
    }
    listen(1);
    client=accept(sfd, (struct sockaddr*)&trg, (socklen_t*)&addrsize);
    if (client==-1){
     printf ("%sError Handling %siChat%s Client%s", KCYN, BGRN, BBLK, KNRM);
     exit(1);
    }
    InitInfo(address, client);
}
void InitInfo(struct sockaddr_in* address){
    char *id=getenv("ONLINE_ID");
    *(id+15)=NULL;
    if (*id==NULL){
      printf ("%sError Getting Your %siChat%s Online ID, Try export ONLINE_ID=[your name] then running\n%s", KCYN, BGRN, BBLK, KNRM);
      exit(1);
    }
    send(client, id, strlen(id));

}



void ProcMsg(char *msg){
   msg=StrRep(msg, "/blink", LNRM);
   msg=StrRep(msg, "/blred", LRED);
   msg=StrRep(msg, "/blblue", LBLU);
   msg=StrRep(msg, "/blblack", LBLK);
   msg=StrRep(msg, "/blgreen", LGRN);
   msg=StrRep(msg, "/blpurple", LPUR);
   msg=StrRep(msg, "/blwhite", LWHT);
   msg=StrRep(msg, "/blcyan", LCYN);

   msg=StrRep(msg, "/nrm", KNRM);
   msg=StrRep(msg, "/white", KWHT);
   msg=StrRep(msg, "/red", KRED);
   msg=StrRep(msg, "/blue", KBLU);
   msg=StrRep(msg, "/yellow", KYEL);
   msg=StrRep(msg, "/purple", KPUR);
   msg=StrRep(msg, "/cyan", KCYN);
   msg=StrRep(msg, "/green", KGRN);

   msg=StrRep(msg, "/bgred", BRED);
   msg=StrRep(msg, "/bgwhite", BWHT);
   msg=StrRep(msg, "/bgblue", BBLU);
   msg=StrRep(msg, "/bgcyan", BCYN);
   msg=StrRep(msg, "/bgpurple", BPUR);
   msg=StrRep(msg, "/bgblack", BBLK);
   msg=StrRep(msg, "/bgyellow", BYEL);

   msg=StrRep(msg, "/ublack", UBLK);
   msg=StrRep(msg, "/uwhite", UWHT);
   msg=StrRep(msg, "/ured", URED);
   msg=StrRep(msg, "/ugreen", UGRN);
   msg=StrRep(msg, "/ublue", UBLU);
   msg=StrRep(msg, "/ucyan", UCYN);
   msg=StrRep(msg, "/upurple", UPUR);


   return msg;
}
char* StrRep(char *str,const char *src,const char *trg){
    int tmp1=strlen(str),tmp2=strlen(src), tmp3=strlen(trg);

    int newsize=tmp1-tmp2+tmp3;
    char *newstr=(char*)malloc(newsize);

    for (int i=0; i<=tmp1; i++){
        if (strncmp((str+i), src, strlen(src))==0){
           strncat(newstr, str, i);
           strncat(newstr, trg, tmp3);
           strcat (newstr, str+i+tmp2);
        }
    }
    return newstr;
}



char* GetMsg(void){
  
}

// END
