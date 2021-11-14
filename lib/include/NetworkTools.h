class NetworkTools{
	private:
		int  sfd;
		int  cfd;
		int port;

		size_t size;

		char*  txdata;
		char*  rxdata;
		char*  ipaddr;
		char*  domain;

		struct sockaddr_in server;
		struct sockaddr_in client;
		struct sockaddr_in    tmp;
		struct hostent*      host;
       	public:
		char* Name(){
			return domain;
		}
		char* GetHostByName(char* domain){
			host=gethostbyname(domain);
			if (host==NULL)
				return 0x00;
			ipaddr=(char*)malloc(16);
			inet_ntop(AF_INET, host->h_addr, ipaddr, 16);
			return ipaddr;
		}
		int Target(char* ip, int port){
			int inetpton=inet_pton(AF_INET, GetHostByName(ip), &client.sin_addr.s_addr);
			if (inetpton==-1)
				return -1;
			client.sin_port=htons(port);
			client.sin_family=AF_INET;
			free(ipaddr);
			return 0;
		}
};
