#define P_GREEN  5
#define P_RED    6

#define I_CREATE 0x63
#define I_JOIN   0x6a
#define I_TEST   0x74

#define RTD_CODE 0x1

#include <ncurses.h>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <ctime>
#include <thread>

#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>

#include "../lib/include/colors.h"

using std::thread;


void  CreateRoom(void);
void  JoinRoom(int r);

char* GetMsg(void);

void  SyncMsg();
void  SendMsg(char* msg);
void  ProcessMsg(char* msg);
char* GetPath(void);

void  FindRoom(void);
void  InitRoom(void);

void  iTest(void);
void  iExit(void);

void  CreateRoomCode(void);

char* StrRep(char* str,const char* src,const char* trg);

unsigned long FindRoom(char* code);


struct user{
	char   ipaddr[16];
        char   about[100];
	char   status[10];
	char        *name;
        char     code[16];
};


struct user me={.name=getenv("ONLINE_ID")};
struct user you;

struct sockaddr_in  trg={.sin_family=AF_INET, .sin_port=htons(5566)};
struct sockaddr_in addr={.sin_family=AF_INET, .sin_port=htons(5566)};

socklen_t addrsize=sizeof(struct sockaddr_in);


WINDOW* about_me;
WINDOW*  details;
WINDOW*     iscr;


const char *aboutme="Programmer: Mahmoud Osman (MLT or MLTF18)\nGithub:     www.github.com/Mahmoud7Osman\nProject:    www.github.com/Mahmoud7Osman/Digle\n\n\nAbout me: Nothing But A C/C++/C#/Assembly Lover.\n";

int init;
int mnum=-1;
int y, x, cury, curx;
int sfd, client;

void print(char* str, int pair){
   wattron(details, COLOR_PAIR(pair));
   waddstr(details,str);

   wattroff(details,COLOR_PAIR(pair));
   waddstr(details,"\n");

   return;
}

void UpdateUpperScreen(){

   wbkgd(about_me, COLOR_PAIR(4));
   wbkgd(details,  COLOR_PAIR(3));

   waddstr(about_me, aboutme);

   wrefresh(about_me);
   wrefresh(details);

   wmove(about_me, 0, 0);
   wmove(details, 0, 0 );

   touchwin(about_me);
   touchwin(details );
}

void MessageCounter(){
    getyx(stdscr,cury, curx);
    mnum++;
    move(0, x-27);

    attron(COLOR_PAIR(2));
    printw("Messages in this Room:");
    attroff(COLOR_PAIR(2));
    printw(" %d", mnum);

    move(cury, curx);

    wrefresh(stdscr);
    wrefresh(iscr);
}

char* GetMsg(void){
    wmove(stdscr, y-2, 0);
    waddstr(stdscr, "->> ");

    char *msg=(char *)malloc(201);
    getnstr(msg, 200);

    waddstr(iscr, "\n[");

    wattron(iscr, COLOR_PAIR(1)); waddstr(iscr, me.name);
    wattroff(iscr, COLOR_PAIR(1)); waddstr(iscr, "] ");

    waddstr(iscr, msg);

    wrefresh(iscr);
    wclear(stdscr);

    return msg;
}

void iExit(){
   printw("Press Any Key To Exit...");
   refresh();
   getch();
   endwin();
   printf ("Thank You For Using iChat\n");
   exit(0);
}

void ProcessMsg(char *msg){
   msg++;
   switch (*msg){
     case 0x65:
        free(you.name);
        free(--msg);
        iExit();
     default:
        return;
   }
}

void CreateRoomCode(){
   for (int tmp=0; tmp<strlen(me.ipaddr); tmp++){
      me.code[tmp]=(int)me.ipaddr[tmp]-RTD_CODE;
   }
}

void FindRoom(){
   for (int byte=0; byte<strlen(you.code); byte++)
      you.ipaddr[byte]=*(you.code+byte)+RTD_CODE;

   int tmp=inet_pton(AF_INET, you.ipaddr, &trg.sin_addr.s_addr);
   if (tmp==-1)return (void)printw("Error Analyzing Room Code %s\n", you.code);
   sleep(2);
}

int main(int argc, char** argv){
   sfd=socket(AF_INET, SOCK_STREAM, 0);
   (void)setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, 0x00, 0x00);

   initscr();
   start_color();

   if (argc < 2 || *argv[1] != I_JOIN && *argv[1] != I_CREATE && *argv[1] != I_TEST){
      printf ("Create or Join or Test ? [c/j/t] ");
      while (init=getchar()){
        if (init!=I_CREATE && init!=I_JOIN && init!=I_TEST){
           continue;
        }
        break;
      }
   }

   else{
      init=*argv[1];
   }

   if (init==I_JOIN){
       client=sfd;

       printw ("Enter Room Code: ");
       refresh();
       getnstr(you.code, 15);

       FindRoom();
       JoinRoom(0);
   }
   else if (init==I_TEST){
      iTest();
   }
   else{
       CreateRoom();
   }
   getmaxyx(stdscr, y, x );

   you.name=(char*)malloc(16);

// about_me=newwin(10,50, 0, 0);
   send(client,  me.name, strlen(me.name), 0);
   recv(client, you.name, 16, 0);

   iscr=newwin(y-12, x-1, 10, 0);


   init_pair(1, COLOR_GREEN, COLOR_BLACK);
   init_pair(2, COLOR_BLUE, COLOR_BLACK );
   init_pair(3, COLOR_WHITE, COLOR_BLUE );
   init_pair(4, COLOR_BLACK, COLOR_WHITE);
   init_pair(5, COLOR_GREEN,  COLOR_BLUE);
   init_pair(6, COLOR_RED,    COLOR_BLUE);

   getyx(stdscr, cury, curx);


   MessageCounter();
   UpdateUpperScreen();

   move(20, 0);
   flushinp();

   scrollok(stdscr, TRUE);
   scrollok(iscr ,  TRUE);

   thread Sync(SyncMsg);
   wclear(stdscr);
   while (1){
         char *test=GetMsg();

         touchwin(iscr);

         if (*test==0x2f && *(test+1)!=0x2f)
            ProcessMsg(test);

         send(client, test, strlen(test), 0);

         getyx(stdscr, cury, curx);
         MessageCounter();
         UpdateUpperScreen();
         free (test);
   }
}


// Before iChat Start

void CreateRoom(){
    printw ("Enter Your IP Address to Start iChat On: ");
    refresh();
    getnstr(me.ipaddr, 15);
    CreateRoomCode();
    inet_pton(AF_INET, me.ipaddr ,&addr.sin_addr.s_addr);

    printw ("Room Code: ");
    addstr(me.code);
    printw("\nWaiting For Requests...\n");
    refresh();

    if (bind(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))==-1){
       printw ("Error Binding iChat Socket\n");
       refresh();
       iExit();
    }
    listen(sfd, 1);
    memset(&trg, 0x00, sizeof(struct sockaddr_in));
    client=accept(sfd, (struct sockaddr*)&trg, &addrsize);
    if (client==-1){
     printw ("Error While Accepting Join Request\n");
     refresh();
     iExit();
    }
    InitRoom();
}

void InitRoom(){
    if (me.name==0x00){
      printw ("Error Getting Your iChat Online ID, Try lset ONLINE_ID [your name] or type isetup\n");
      refresh();
      iExit();
    }

}


char* StrRep(char *str,const char *src,const char *trg){
    int tmp1=strlen(str),tmp2=strlen(src), tmp3=strlen(trg);

    int newsize=tmp1-tmp2+tmp3;
    char *newstr=(char*)malloc(newsize+1);
    *(newstr+newsize)=0x00;

    for (int i=0; i<=tmp1; i++){
        if (strncmp((str+i), src, strlen(src))==0){
           strncat(newstr, str, i);
           strncat(newstr, trg, tmp3);
           strcat (newstr, str+i+tmp2);
        }
    }
    free(str);
    return newstr;
}

void JoinRoom(int r){
      if (r==9)
        return;
      int tmp=connect(sfd, (struct sockaddr*)&trg, sizeof(trg));
      if (tmp==-1){
         printw ("Joining Room... (Request Number %d)\r", r+1);
         refresh();
         sleep(1);
         return JoinRoom(r+1);
      }
      else{
         printf("\n");
         InitRoom();
         return;
      }

}
void SyncMsg(){
  char msg[200];

  while (recv(client, msg ,200 , 0)){
    waddstr(iscr,"\n[");
    wattron(iscr,COLOR_PAIR(2)); waddstr(iscr, you.name);
    wattroff(iscr, COLOR_PAIR(2)); waddstr(iscr, "] ");
    waddstr(iscr, msg);
    wrefresh(iscr);
    bzero(msg, 200);
    wmove(stdscr, y-2, 4);
  }
}
void iTest(){
   getmaxyx(stdscr, y, x);
   iscr=newwin(y-12, x-1, 10, 0);

   init_pair(1, COLOR_GREEN, COLOR_BLACK);
   init_pair(2, COLOR_BLUE, COLOR_BLACK );
   init_pair(3, COLOR_WHITE, COLOR_BLUE );
   init_pair(4, COLOR_BLACK, COLOR_WHITE);
   init_pair(5, COLOR_GREEN,  COLOR_BLUE);
   init_pair(6, COLOR_RED,    COLOR_BLUE);

   getmaxyx(stdscr, y, x );
   getyx(stdscr, cury, curx);


   MessageCounter();
   UpdateUpperScreen();

   move(20, 0);
   flushinp();

   scrollok(stdscr, TRUE);
   scrollok(iscr,   TRUE);

   while (1){
         char *test=GetMsg();

         touchwin(iscr);

         if (*test==0x2f && *(test+1)!=0x2f)
            ProcessMsg(test);

         getyx(stdscr, cury, curx);
         MessageCounter();
         UpdateUpperScreen();
         free (test);
   }
};
