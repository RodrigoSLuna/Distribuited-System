#include <pthread.h>
#include "structs.h"
#define MAX_NR_USERS 20
#define MAX_SIZE_NAME 40
#define watch(S1,S2) printf("DEBUG %s %s \n",S1,S2);
#define m 8
using namespace std;
const int INF = 1e6;
long long pw[m+1];
/*
Pedaço de codigo que nao deve estar aqui
*/
void init(){
	pw[0] = 1;
	for(int i = 1;i<=m;i++)
		pw[i] = pw[i-1]*2; 
}

void getIP(TSocket sock, char* A){
	unsigned int cliLen;
	struct sockaddr_in cliAddr;
	memset( (void *) &cliAddr, 0 ,sizeof( cliAddr ) );
	cliLen = sizeof(cliAddr);
	if( getpeername(sock, (struct sockaddr *) &cliAddr, &cliLen)){
		ExitWithError("Error in getting IP");
	}
	strcpy(A,inet_ntoa(cliAddr.sin_addr));
}

void GeneratePeerId(int *id, char *IP){
	int val = 0;
	int size = strlen(IP);
	for(int i = 0; IP[i] != '\0';i++){
		if(IP[i] >= '0' and IP[i] <= '9')	
			val += ( (IP[i] - '0')*pow(10,size));
		size--;
		while( val > pw[m] ) val -= pw[m];
	}
	printf("%lld\n",val);	
	*id = val;
}

class Peer{
public:
  int tid; //Thread_Lists;
  pthread_t threads[NUMTHREADS];
  char *IP;
  unsigned int PORT;
  int id;
  vector<int> FILES; // Possivelmente vai virar 1 descritor de arquivo!. 
  TSocket sock;
  Peer *pre, *suc;
  Peer(){}
  Peer(char *IP, unsigned int PORT){
    this->IP = IP;
    this->PORT = PORT;
    GeneratePeerId(&id,IP);
    printf("id: %d\n",id);
  }
  static void int join(void * client,char *buffer);
  static void* HandleRequest( void* args );
  void Connection();
  ~Peer(){}
};


int Peer::join(void* client, char *buffer){
	Peer *A = ( (Peer*)  );
	


	
}
void* Peer::HandleRequest( void* args){
	char buffer[BUFSIZE], usr_option;
	if(args == NULL) puts("Null");
	Peer *A = (Peer* ) args;

	if(ReadLine(A->sock, buffer , BUFSIZE ) < 0 ){
		printf("HandleRequest Message can't be read: %d, IP: %s\n",A->sock,A->IP);
		return NULL;
	}
		
	user_option = buffer[0];

	switch(user_option){
		case '1':
		ok = join(args, buffer);
	}
	


}
void Peer:: Connection(){
	while(true){
		if(tid == NUMTHREADS)
			ExitWithError("Number of threads is over");

		Peer *a = new Peer();
		int sock_aux = AcceptConnection( this -> sock );
		char IP[IPSIZE];
		getIP(sock_aux,IP);
		
		printf("\n|Connection Started\n");
		printf("|-IP: %s\n",a->IP);
		printf("sock: %d\n", a->sock);

		if(pthread_create(&threads[tid++], NULL, (THREADFUNCPTR) &Peer::HandleRequest,a))
			ExitWithError("pthread_create() failled");		
	}
}



int main(int argc, char ** argv){

  if (argc != 4 ){
   ExitWithError("Usage:peer0 <remote server IP>  client <remote server IP> <remote server Port> ");
  }
  init();
  Peer *node = new Peer(argv[2],atoi(argv[3]));
  // Entro na rede, a partir do no 0
  node->join(argv[1]); 

  return 0;
}
