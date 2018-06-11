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
LALALALALALA Pedaço de codigo que nao deve estar aqui
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

void GeneratePeerId(int *id, const char *IP){
	int val = 0;
	int size = strlen(IP);
	for(int i = 0; IP[i] != '\0';i++){
		if(IP[i] >= '0' and IP[i] <= '9')	
			val += ( ( (IP[i] - '0')*( (long long)pow(10,size) )%pw[m] )) %pw[m];
		
		size--;
		while( val > pw[m] ) val -= pw[m];
	}
	printf("%d %d\n",val,pw[m]);	
	*id = val;
}

class Peer{
public:
  int tid; //Thread_Lists;
  pthread_t threads[NUMTHREADS];
  char *IP;
  int PORT;
  static int id;
  vector<int> FILES; // Possivelmente vai virar 1 descritor de arquivo!. 
  TSocket sock;
  Peer *zero;
  static Peer *pre, *suc;
  Peer(){}
  Peer(char *IP,int PORT){
    this->IP = IP;
    this->PORT = PORT;
    GeneratePeerId(&id,IP);
    printf("id: %d\n",id);
  }
  int join();
  static void* HandleRequest( void* args );
  static int findsucessor(void *client, char *buffer);
  void Connection();
  void Create(){
 	this->sock = CreateServer(this->PORT);
  }
  ~Peer(){}
};


int Peer::join(){
	char buffer[BUFSIZE];
	
	if(this->zero == NULL ) puts("NULL");
	sprintf(buffer, "1 %s %d \n",this->IP, this->PORT);	
	
	printf("%s %d\n",this->zero->IP,this->zero->PORT);
	TSocket sender = ConnectToServer(this->zero->IP,this->zero->PORT);	
	puts("Conectado...");

	if(WriteN(sender,buffer, BUFSIZE) < 0 )
		ExitWithError("WriteN failed");
	
	close(sender);
	
}
void* Peer::HandleRequest( void* args){
	char buffer[BUFSIZE], usr_option;
	if(args == NULL) puts("Null");
	Peer *A = (Peer* ) args;

	if(ReadLine(A->sock, buffer , BUFSIZE ) < 0 ){
		printf("HandleRequest Message can't be read: %d, IP: %s\n",A->sock,A->IP);
		return NULL;
	}
		
	usr_option = buffer[0];	
	printf("usr_option: %c\n",usr_option);
	switch(usr_option){
		case '1':
			findsucessor(args,buffer);
			if(WriteN(A->sock, buffer, BUFSIZE) < 0 ){
				printf("Findsucessor failled\n");
				return NULL;
			}
	}	
}

int Peer::findsucessor(void *client, char *buffer){
	Peer *A = (Peer* ) client;
	string parser(buffer);
	stringstream ss(parser);
	vector<string> v;
	while(ss>>parser){v.pb(parser);}
	string::size_type sz;
	char IP[BUFSIZE];
	int PORT;
	sprintf(IP, "%s",v[1].c_str());
	PORT = stoi(v[2], &sz );	
	cout << "findsucessor "<< IP << " " << PORT << endl;
	int id1;
	GeneratePeerId(&id1, IP);
	printf("buscando Posicao para o ID: %d, meu id:%d\n",id1,id);
	//Adiciono no final da lista
	// Falta adicionar o pre, preciso avisar pro peer atualizar...
	if( suc == NULL and id1 > id ){	
		Peer *node = new Peer( IP,PORT );
		suc = node;
		cout << suc->IP << " " << suc->PORT << endl;
	}
	else if( suc != NULL and id1 > suc->id ){
		
	}
}

void Peer:: Connection(){
	while(true){
		if(tid == NUMTHREADS)
			ExitWithError("Number of threads is over");

		Peer *a = new Peer();
		puts("Esperando conexao");
		int sock_aux = AcceptConnection( this -> sock );
		char IP[IPSIZE];
		getIP(sock_aux,IP);
		a->IP = IP;
		a->sock = sock_aux;
		printf("\n|Connection Started\n");
		printf("|-IP: %s\n",a->IP);
		printf("sock: %d\n", a->sock);

		if(pthread_create(&threads[tid++], NULL, (THREADFUNCPTR) &Peer::HandleRequest,a))
			ExitWithError("pthread_create() failled");		
	}
}


Peer * Peer::suc;
Peer * Peer::pre;
int Peer::id;
int main(int argc, char ** argv){
  init();
  Peer *node, *zero;
  if (argc == 5 ){ 
	 zero = new Peer(argv[1],atoi( argv[2] ));
  	 node = new Peer(argv[3],atoi(argv[4]));
 	 node->zero = zero;
  	 node->join();
  }
  else if(argc == 3){
  	 node = new Peer(argv[1],atoi(argv[2]));
  	 node->id = 0;
  }
  else
	return 0*puts("Quantidade de parametros errados");
 node->Create(); 
 node->Connection();
 return 0;
}
