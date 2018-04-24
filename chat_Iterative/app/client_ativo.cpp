#include <string>
#include "mysocket.h"
#include <iostream>
#include <time.h>

using namespace std;
char *nome;



int main(int argc, char** argv){
	TSocket cliSock;        /* server and client sockets */
	char *servIP;                /* server IP */
	unsigned short servPort;     /* server port */
	fd_set set;  /* file deion set */

	if (argc != 4) {
		ExitWithError("Usage: client <remote server IP> <remote server Port> <Nome>");
	}
	servIP   = argv[1];
	servPort = atoi(argv[2]);
	nome = argv[3];
	cliSock = ConnectToServer(servIP, servPort);
	puts("Conectado, envie msg de boas vindas");
	for(;;){
		FD_ZERO(&set);
		//configuro o select para ouvir I/O
		FD_SET(STDIN_FILENO,&set);

		//configuro o select para ouvir o socket
		FD_SET(cliSock,&set);

		int ret = select (FD_SETSIZE, &set, NULL, NULL, NULL);

	  	if (ret<0) {
	       WriteError("select() failed");
	       break;
   		}

		if( FD_ISSET(STDIN_FILENO,&set) ){
			if( write_message(cliSock,nome) == -1 ) break;
		}

		if( FD_ISSET(cliSock,&set) ){
			if( read_message(cliSock) == -1 ) break;
		}
	}
	close(cliSock);
	return 0;
}
