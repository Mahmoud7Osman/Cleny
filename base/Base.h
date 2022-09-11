class File{
   public:
	int fd;
	char* filename;
	char* data;
	char* EP;
	size_t size;
	struct stat filestat;



	File(char* fn){
			fd=open(fn, O_RDONLY);

			if (fd == -1)
				return;

			fstat(fd, &filestat);
			filename=fn;
			size=filestat.st_size;
			data=(char*)malloc(size);
			EP=data;

			if (data == NULL)
				return;

			read(fd, data, size);
	}

	~File(){
		free(EP);
	}
};

class Syntax{

};

class Tag{
	
};
