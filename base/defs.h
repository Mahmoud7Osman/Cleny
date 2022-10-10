#define ELC 82

class Element;

typedef struct stack{

	Element*        elementclass = NULL;
	struct stack*    back        = NULL;

}
stack;

class File;
class ElementStack;

void fatalerror(const char*, File*);
void DEBUG(const char*);

void enabler(char*);
