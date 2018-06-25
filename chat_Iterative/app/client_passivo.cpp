#include <string>
#include "mysocket.h"
#include "structs.h"
#include <iostream>
#include <time.h>
#include <map>
#include <queue>

using namespace std;

/*Variaveis globais*/
unsigned int MyPort;
char nome[BUFSIZE],cliPort[BUFSIZE];

int Register(TSocket srv ){
  puts("Faça o registro do seu nome");
  cin >> nome >>cliPort;
  char buffer[BUFSIZE] = {'\0'};
  sprintf(buffer, "1 %s %s \n",nome,cliPort);
  
  if( WriteN(srv, buffer, BUFSIZE ) < 0  ){
    ExitWithError(" WriteN() failed ");
  }

  if( ReadLine(srv,buffer, BUFSIZE ) < 0 ){
    ExitWithError(" ReadLine() failed ");
  }

  close(srv);
  if( strcmp(buffer,"0") == 0 or strcmp(buffer,"0") == 1 or strcmp(buffer,"0") == 2  ){
    MyPort = stoi(cliPort);
    return 1;
  }
  return 0;
}

void CloseConectionToServer(TSocket &srv){
  char buffer[BUFSIZE];
  sprintf(buffer,BUFSIZE,"3 %s \n",nome);  
  if( WriteN(srv, buffer,BUFSIZE ) < 0 ) { ExitWithError("WriteN() failed ");}

}

//Funcao que ira cuidar das conexões a partir de outras threads.
void HandleRequest_Conection(TSocket cliSock){
  //TSocket srv,cliSock;
  int ret; // retorno do select
  fd_set set;  /* file description set */
  //unsigned int cliLen;
  //char *ip;
  //struct sockaddr_in cliAddr; // Endereço de IP do cliente
  //srv = ((struct TArgs* )args)->cliSock; 
  //memset( (void*) &cliAddr, 0, sizeof ( cliAddr )  ); // Zero a estrutura de dados
  //cliLen = sizeof(cliAddr);

  //puts("Aguardando conexao de par ativo");
 
  //cliSock = AcceptConnection(srv);
  //int retcode = getpeername(cliSock, (struct sockaddr *) &cliAddr, &cliLen); // IP da pessoa que acabou de se conectar

  //printf("Conectado com: %s\n",cliAddr.sin_addr);
  for(;;){
    char buffer[BUFSIZE] = {'\0'};
    /* Initialize the file descriptor set */
    FD_ZERO(&set);
    /* Include stdin into the file descriptor set */
    FD_SET(STDIN_FILENO, &set);
    /* Include srvSock into the file descriptor set */
    FD_SET(cliSock, &set);

    ret = select (FD_SETSIZE, &set, NULL, NULL, NULL);

    if (ret<0) {
       WriteError("select() failed");
       break;
    }
    /* Read from stdin */
    if( FD_ISSET(STDIN_FILENO,&set) ){
      //if( write_message(cliSock, nome ) == -1 ) break;
	char MSG[BUFSIZE] = {'\0'}, buffer[BUFSIZE] = {'\0'};
	scanf("%[^\n]",MSG);	
	sprintf(buffer,"%s: %s \n",nome,MSG);
	if(WriteN(sock, buffer, BUFSIZE ) < 0 ){
		ExitWithError("1: WriteN() failed");
	}
    }
    //read from socket
    if( FD_ISSET(cliSock,&set) ){
      //if( read_message(cliSock) == -1 ) break;
	char buffer[BUFSIZE] = {'\0'};
	if(ReadLine(sock, buffer,BUFSIZE) < 0){
		ExitWithError("ReadLine() failed");
	}
	if(strcmp(buffer,"FIM") == 0){
		break;
	}
	else
		printf("Peer: %s \n",buffer);

    }
  }
  puts("Conexao encerrada");
  close(cliSock);
}
int main(int argc, char** argv){
  /*Aceita a conexao!*/
  TSocket srv, cliSock,MySrv;        /* server and client sockets */
  char *servIP;                /* server IP */
  unsigned short servPort,cliPort;     /* server port */
  pthread_t conection_thread,main_thread;
  struct TArgs *args,*argSrv,*MyInfo;              /* argument structure for thread */
  fd_set set;  /* file description set */
  int ret; // retorno do select
  

  if (argc != 3) {
   ExitWithError("Usage: client <remote server IP> <remote server Port> ");
  }

  servIP   = argv[1];
  servPort = atoi(argv[2]);
 
  srv = ConnectToServer(servIP,servPort);

 
  // Envio meu nick e  tento registrar, no servidor.
  while( !Register(srv)  ){ ; }

  if ((argSrv = (struct TArgs *) malloc(sizeof(struct TArgs))) == NULL) { ExitWithError("malloc() failed"); }

  // Passo o meu servidor como argumento para a Thread
  MySrv = CreateServer( MyPort );
  argSrv->cliSock = MySrv;
//  args->cliSock = srv;
/*
Argumentos do select para funcionar a troca de interacao
entre o servidor e meu stdin
*/
 fd_set	set; // descritor de arquivo do select
 int ret; // retorno do select
 //Crio um socket para aguardar a conexao


  while(1){
    TSocket cliSock;
    cliSock = AcceptConnection(MySrv);
    
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    FD_SET(cliSock, &set);
    ret = select(FD_SETSIZE, &set, NULL,NULL,NULL);

    if(ret < 0 ){
	WriteError("Select() failed");    
	break;
    } 

    if(FD_ISSET(cliSock, &set) ){
    	HandleRequest_Conection( cliSock );
    }
    if(FD_ISSET(STDIN_FILENO, &set)){
    	char buffer[BUFSIZE];
	scanf("%[^\n]",buffer);
	if(strcmp(buffer,"FIM") == 0 ){
		close(cliSock);	
  		srv = ConnectToServer(servIP,servPort);
		CloseConectionToServer( srv );
		break;
	} 
      
    } 
  }
  return 0;
}
