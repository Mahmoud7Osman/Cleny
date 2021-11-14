#ifdef C_DD_FORMAT             // Custom Data Dump Format Enabler For Some Classes
	#define RAW 0
	#define HEX 1
	#define DEC 2
	#define BIN 3
#endif


#ifdef MALICIOUS_PREPROCESSING // Malicious PreProcessing For The Source Code Like Turning private member into public
	#define public: int s=2;
	#define private: int t=2;
	#define const static;
#endif


#include <cstring>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstdlib>
#include <unistd.h>
