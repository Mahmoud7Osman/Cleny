class File{
   public:
	int fd;
	char* name;
	char* data;
	char* EP;
	size_t size;
	struct stat filestat;



	File(char* fn){
			fd=open(fn, O_RDONLY);

			if (fd == -1)
				return;

			fstat(fd, &filestat);
			name=fn;
			size=filestat.st_size;
			data=(char*)malloc(size);
			EP=data;

			if (data == NULL)
				return;

			read(fd, data, size);
	}

	void ResetDP(){
		data=EP;
	}

	~File(){
		free(EP);
	}
};

class Syntax{
   public:

	File* file;
	Syntax(File* f){
		file=f;
	}
	void Divs(){
		for (int i; i<file->size; i++){
			if (*(file.data+i) == '<'){
				while (file.data++){
					if (*file.data == ' ' || *file.data == '<'){
						fatalerror("Illegal Character Detected Inside A Tag", &file);
						i=file->size;
						break;
					}
				}
			}
		}
	}
};

class Tag{

};
