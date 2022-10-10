class File{
   public:
	int fd;
	char* name;
	char* data;
	char* EP;
	char* LP;
	size_t size;
	struct stat filestat;



	File(char* fn){
			fd=open(fn, O_RDONLY);

			if (fd == -1)
				fatalerror("read permission denied", NULL);

			fstat(fd, &filestat);
			name=fn;
			size=filestat.st_size;
			data=(char*)malloc(size);
			EP=data;
			LP=data+(size-1);
			if (data == NULL)
				fatalerror("memory allocation failed", NULL);

			read(fd, data, size);
	}

	void ResetDP(){
		data=EP;
	}

	~File(){
		free(EP);
	}
};


class Element{
 public:
 	char* open_tag        = NULL;
 	char* close_tag 	  = NULL;
 	const char* prefix    = NULL;
};


class ElementStack{
  public:
 	stack* TopPointer;
 	
  	ElementStack(){
		stack* base =(stack*) malloc(sizeof(stack));
		base->elementclass = new Element();
		base->elementclass->prefix = "\033[0m";
		TopPointer=base;
	}
  
	stack* top(){
		if (TopPointer->back)
			return TopPointer;
		return NULL;
	}
	
	void push(stack* newstack){
		stack* ns=(stack*)malloc(sizeof(stack));
		ns->back = TopPointer;
		TopPointer = ns;
		ns->elementclass = newstack->elementclass;
	}
	void StackDump(){
		for (stack* TP=TopPointer; TP->back!=NULL; TP=TP->back){
				printf("Stack Dump Layers: O: %s C: %s\n", TP->elementclass->open_tag, TP->elementclass->close_tag);
		}
		fflush(stdout);
	}
	void pop(){
		delete TopPointer->elementclass;
		stack* poped=TopPointer;
		TopPointer=TopPointer->back;
		free(poped);
	}
	void freestack(){
		for (TopPointer; TopPointer->back!=NULL; TopPointer=TopPointer->back){
			delete TopPointer->elementclass;
			free(TopPointer);
		}
	}
};

ElementStack Stack;

class Syntax{
   public:

	File* file;
	Syntax(File* f){
		file=f;
	}
	void scan(){
		for (file->data; file->data<file->LP; file->data++){
			if (*(file->data) == '<'){
				char** tag;
				size_t size=0;
				if (*(file->data+1) != '/'){
					stack* layer = (stack*)malloc(sizeof(stack));
					layer->elementclass = new Element();
					tag=&layer->elementclass->open_tag;
					Stack.push(layer);
				} 
				else {
					file->data++;
					tag=&Stack.TopPointer->elementclass->close_tag;
				}
				*tag=(char*)malloc(1);
				while (++file->data && file->data<=file->LP){
					if (*(file->data) == ' ' || *(file->data) == '<' || *(file->data) == '\n' || *(file->data) == '\r'){
						fatalerror("Illegal Character Detected inside tag name", file);
						file->data=file->LP;
						break;
					}
					else if(*(file->data) == '>'){
						break;
					}
					memset((*tag)+size, *file->data, 1);
					*tag      =  (char*) realloc(*tag, ++size);
				}
				
				memset(*(tag)+size, 0x00, 1);
				
				if (Stack.TopPointer->elementclass->close_tag){
					if (strcmp(Stack.TopPointer->elementclass->open_tag, Stack.TopPointer->elementclass->close_tag) != 0){
						Stack.freestack();
						fatalerror("one or more tags are not nested or closed correctly, Open/Close Tags doesn't match", NULL);
					}
					Stack.pop();
				}
			}
			else{
				enabler(Stack.TopPointer->elementclass->open_tag);
				printf("%c", *file->data);
			}
		}
		printf("\033[0;0m\n");
	}
};

void enabler(char* toenable){
	if (!toenable){
		Stack.TopPointer->elementclass->prefix="\033[0;37m";
		return;
	}
	for (int element=0; element < ELC - 1 ; element++){
		if (strcmp(toenable, elements[element]) != 0){
			continue;
		}

		printf("\033[0m%s", elements[++element]);

	}
}
