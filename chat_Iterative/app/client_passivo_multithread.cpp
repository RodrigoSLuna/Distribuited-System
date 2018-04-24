#include <string>
#include "mysocket.h"
#include "structs.h"
#include <iostream>
#include <time.h>
#include <map>
#include <queue>

using namespace std;

/*Variaveis globais*/

// map<string,int> nome_ip;
// map<int,_string> ip_nome;

set<TArgs*> conectados;
pthread_t threads[NUMTHREADS];
map<int, TBuffer> BUFFER;
int ativo;
unsigned int MyPort;
pthread_mutex_t mutex_actives = PTHREAD_MUTEX_INITIALIZER; // Fazer a secao critica de quem estara se conectando, saber a quantidade de ativos


int Register(TSocket srv ){
  puts("Faça o registro do seu nome");
  string username,cliPort,send= "";
  cin >> username>>cliPort;
  char buffer[BUFSIZE];
  send += "1 ";
  send += username;
  send += " ";
  send += cliPort;

  for(int i = 0;i<send.size();i++) buffer[i] = send[i];
  buffer[send.size()] = '\n';
  if( WriteN(srv, buffer, sizeof(buffer) ) < 0  ){
    ExitWithError(" WriteN() failed ");
  }

  if( ReadLine(srv,buffer,sizeof(buffer)) < 0 ){
    ExitWithError(" ReadLine() failed ");
  }

  close(srv);
  cout << buffer << endl;
  if( strcmp(buffer,"0") == 0 or strcmp(buffer,"0") == 1 or strcmp(buffer,"0") == 2  ){
    cout << cliPort << endl;
    MyPort = stoi(cliPort);
    printf("%u\n",MyPort);
    return 1;
  }
  return 0;
}
/*
Fazer a troca do dialogo ativo, pensar em como vai ficar!

*/
void show_conected_users(TSocket srv){

  pthread_mutex_lock(&mutex_actives);
  printf("Existe(m): %d usuarios conectados com voce\n",conectados.size());
  int i = 0;
  for(auto conexao : conectados){
    printf("%d %d\n",conexao->IP,conexao->PORT);
  }
  pthread_mutex_unlock(&mutex_actives);
}
void show_messages(){
  puts("Digite o IP como um numero");
  int ip;
  scanf("%d",&ip);

  printf("Mensagens recebidas de: %d ate o momento\n",ip);
  // Consumo as mensagens
  pthread_mutex_lock( &BUFFER[ip].mutex );

  while( !BUFFER[ip].messages.empty()){
    cout << BUFFER[ip].messages.front() << endl;
    BUFFER[ip].messages.pop();
  }
  pthread_mutex_unlock( &BUFFER[ip].mutex  ); // termino de escrever naquele buffer

}

/*Terminar de programar o 5.2*/

TArgs* findSocket(TSocket){
  puts("Digite o IP como um numero e a Porta");
  int ip,port;
  scanf("%d %d",&ip,&port);
  for(auto p: conectados){
    if( p->IP == ip and p->PORT == port ){
        return p;
    }
  }
  return NULL;
}

void chat(void *args){
  TSocket cliSock;
  cliSock = ( (struct TArgs *) args)->cliSock;
  fd_set set;  /* file description set */
  int ret; // retorno do select
  puts("Inicializou o chat");
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
    if (FD_ISSET(STDIN_FILENO, &set)) {
      scanf("%s", buffer);
      char MSG[BUFSIZE];
      snprintf(MSG,BUFSIZE,"%s \n",buffer);

      if (strncmp(MSG, "FIM\n", 3) == 0) break;
      else if( WriteN(cliSock, MSG, sizeof(buffer) ) < 0  ){
        ExitWithError(" WriteN() failed ");
      }
    }

    /* Read from srvSock */
    if (FD_ISSET(cliSock, &set)) {
      if(ReadLine(cliSock,buffer,BUFSIZE) < 0 ){ ExitWithError("ReadLine() failed");  }
      else printf("%s",buffer);
    }
  }
}


int select_users(TSocket srv){
  puts("Selecione uma das opções abaixo");
  puts("< 1 > Exibir ultimas mensagens recebidas");
  puts("< 2 > Iniciar Chat");
  int op;
  scanf("%d",&op);
  if(op == 1)
    show_messages();
  else if(op == 2){
    TArgs *args = findSocket(srv);
    if(args == NULL){
      return 0;
    }
    else{
      chat(( void *) args);
    }
  }
  return 1;
}


int options(TSocket srv){

  puts("Selecione uma das opções abaixo");
  puts("< 1 > Exibir usuários conectados");
  puts("< 2 > Selecionar usuário");
  puts("< 3 > Para finalizar sessão no servidor");
  puts("< 4 > Finalizar Aplicação");
  int op;
  scanf("%d",&op);
  printf("op:%d\n",op);
  if(op == 1){
    show_conected_users(srv);
    return 1;
  }
  else if(op == 2){
    if( select_users(srv) == 0 )
      return 0*puts("User not conected");
    return 1;
  }
  else if( op == 3 ){
    //Finalizar sessão no servidor
  }

  return 0;
}

void* HandleRequest_Menu(void *args){
   TSocket srv;
   srv = ( (struct TArgs*) args)->cliSock;
   while( options(srv) ){;}

  pthread_exit(NULL);
}
//Funcao que ira LER do cliente e armazenar num buffer
void* HandleRequest_Read(void *args){
  TSocket cliSock;
  int Ip;
  //Quando aceita conexao pega Dados, Ip é identificador do usuário
  Ip = ( (struct TArgs*) args)->IP;
  cliSock = ( (struct TArgs*) args) -> cliSock;
  //Aceito as conexões
  //Bug com as mensagens recebidas, recebendo LIXO por algum motivo!.
  for(;;){
    char MSG[BUFSIZE];
    memset(MSG,'\0',BUFSIZE);
    if(ReadLine(cliSock, MSG,BUFSIZE-1) < 0 ) { ExitWithError("ReadLine() failed"); }
    string msg = string(MSG);
    pthread_mutex_lock( &BUFFER[Ip].mutex );
    printf("Armazenou em: %d, a msg: %s\n",Ip,MSG);
    BUFFER[Ip].messages.push(msg);  // Adiciono a conversa no buffer
    pthread_mutex_unlock( &BUFFER[Ip].mutex  ); // termino de escrever naquele buffer
  }
  puts("Encerro conexao");
  close(cliSock);
  //Encerro conexão
}
//Funcao que ira cuidar das conexões a partir de outras threads.
void* HandleRequest_Conection(void *args){
  struct TArgs *args_thread;
  int tid = 0;
  TSocket srv,cliSock;
  srv = ( (struct TArgs *) args)->cliSock;
  for(;;){
    if( tid == NUMTHREADS ){ ExitWithError("Number of Threads is over");}
    int retcode;
    unsigned int cliLen;
    struct sockaddr_in cliAddr;
    memset( (void*) &cliAddr, 0, sizeof ( cliAddr )  ); // Zero a estrutura de dados
    cliLen = sizeof(cliAddr);
    //Aceita a conexão
    cliSock = AcceptConnection(srv); // Esta errado preciso de uma thread apenas para isso!
    //Armazena a informação do cliente que acabou de se conectar
    retcode = getpeername(cliSock, (struct sockaddr *) &cliAddr, &cliLen); // IP da pessoa que acabou de se conectar
    /*Uma Thread para guardar e lidar com os requests*/
    /* Create separate memory for client argument */
    if ((args_thread = (struct TArgs *) malloc(sizeof(struct TArgs))) == NULL) { ExitWithError("malloc() failed"); }
    args_thread->cliSock = cliSock;
    //args_thread->IP = stoi( inet_ntoa(cliAddr.sin_addr) ); Retorna.

    pthread_mutex_lock(&mutex_actives);
    conectados.insert(args_thread);
    pthread_mutex_unlock(&mutex_actives);
    /* Create a new thread to handle the client requests */
    if (pthread_create(&threads[tid++], NULL, HandleRequest_Read, (void *) args_thread)) { ExitWithError("pthread_create() failed"); }
  }
}
int main(int argc, char** argv){
  /*Aceita a conexao!*/
  TSocket srv, cliSock,MySrv;        /* server and client sockets */
	char *servIP;                /* server IP */
	unsigned short servPort,cliPort;     /* server port */
  pthread_t conection_thread,main_thread;
  struct TArgs *args,*argSrv,*MyInfo;              /* argument structure for thread */
  void *retval;
  if (argc != 3) {
   ExitWithError("Usage: client <remote server IP> <remote server Port> ");
  }

  servIP   = argv[1];
  servPort = atoi(argv[2]);
  srv = ConnectToServer(servIP,servPort);

  // Envio meu nick e tento registrar, no servidor.
  while( !Register(srv)  ){ ; }

  MySrv = CreateServer( MyPort ); // Me coloco como um servidor
  // Roda eternamente
  if ((args = (struct TArgs *) malloc(sizeof(struct TArgs))) == NULL) { ExitWithError("malloc() failed"); }
  if ((argSrv = (struct TArgs *) malloc(sizeof(struct TArgs))) == NULL) { ExitWithError("malloc() failed"); }

  // Passo o meu servidor como argumento para a Thread
  argSrv->cliSock = MySrv;

  //Para a thread principal, irá ter a conexão com o server
  args->cliSock = srv;

  if( pthread_create(&conection_thread,NULL,HandleRequest_Conection, (void *) argSrv )){ ExitWithError("pthread_conection() failed"); }
  if( pthread_create(&main_thread,NULL, HandleRequest_Menu, (void *) args  )){ ExitWithError("pthread_main() failed"); }
  pthread_join(main_thread, &retval);
  pthread_join(conection_thread, &retval);
  return 0;
}
