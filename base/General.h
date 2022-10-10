void fatalerror(const char* msg, File* file){
			printf("\033[0m");
			if (file == NULL){
				printf("\ncleny: %serror%s: %s\n", RED, RESET, msg);
				exit(1);
			}
				printf("\ncleny: %serror%s: %s\n\n", RED, RESET, msg);
				long b=0, a=0;

					if( file->data-30>file->EP )
						b=30;

					if( file->data-30<=file->EP )
						b=file->data-file->EP;

					if( file->data+30>file->LP )
						a=(long)(file->LP-file->data);

					if( file->data+30<file->LP )
						a=30;

				for (char* i=file->data-b; i<=file->data+a; i++){
					if (i == file->data){
						printf("%s%c%s", BRED, *i, RESET);
						continue;
					}
					printf("%c", *i);
				}
				printf("\n");
				exit(1);
}
void DEBUG(const char* msg){
	printf("\033[0m[DEBUG] %s\n", msg);
}
