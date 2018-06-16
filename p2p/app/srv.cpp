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
/*

		int Peer::atualizaPredecessor(void *client, char *buffer ){
			puts("Atualiza predecessor");
			Peer *A = (Peer*) client;
			string parser(buffer);
			stringstream ss(parser);
			vector<string> v;
			while(ss>> parser){v.pb(parser);}
			string::size_type sz;

			char IP[BUFSIZE];
			int PORT;
			sprintf(IP, "%s",v[1].c_str());
			
			PORT = stoi(v[2], &sz );	
			cout << "CHECK AQUI:::: " << v[2] << " " << stoi(v[2], &sz);
			Peer *novo = new Peer(IP,PORT);
			pre = novo;
			
			//Checking
			cout <<"Checking"<< endl;
			cout << "Meu: "  << IP << " " << PORT << endl;
			cout << "Pred: " << pre->IP << " " << pre->PORT << endl;


	}
*/
//Parametros passados como referencia.
void GetInfo( char* buffer, string &suc_IP,int &suc_port,string &pre_IP, int &pre_port ){
	string::size_type sz;		
	string parser(buffer);
	stringstream ss(parser);
	vector<string> v;
	while(ss>> parser){v.pb(parser);}
	suc_IP   = v[1];
	suc_port = stoi(v[2], &sz);
	pre_IP   = v[3];
	pre_port = stoi(v[4], &sz); 	
	
	puts("FUNCAO GET INFO");
	cout << "Sucessor: " << endl;
	cout << suc_IP <<" " << suc_port << endl;
	cout << "Predecessor: " << endl;
	cout << pre_IP << " " << pre_port << endl;

}
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

void GeneratePeerId(int *id, const char *IP, int PORT){ int val = 0;
	int size = strlen(IP);
	for(int i = 0; IP[i] != '\0';i++){
		if(IP[i] >= '0' and IP[i] <= '9')	
			val += ( ( (IP[i] - '0')*( (long long)pow(10,size) )%pw[m] )) %pw[m];
			
		size--;
		while( val > pw[m] ) val -= pw[m];
	}
	PORT %= pw[m];
	val = (val+PORT)%pw[m];
	printf("%d %d\n",val,pw[m]);	
	*id = val;
}

class Peer{
	public:
	int tid; //Thread_Lists;
	pthread_t threads[NUMTHREADS];
	char IP[BUFSIZE];
	int PORT;
	static int id;
	vector<int> FILES; // Possivelmente vai virar 1 descritor de arquivo!. 
	TSocket sock_connection, sock_peer;
	Peer *zero;
	static Peer *pre, *suc;
	Peer(){}
	Peer(char *IP,int PORT){
	    
		memcpy(this->IP,IP,BUFSIZE);
		this->PORT = PORT;
		printf("Construtor: MEU IP: %s, MINHA PORTA: %d\n",this->IP,this->PORT);
		GeneratePeerId(&id,IP,PORT);
		printf("Construtor: id: %d\n",id);
	  }
	int join(Peer *A, char *buffer);
	static void* HandleRequest( void* args );
	static int findsucessor(void *client, char *buffer);
	void Connection( Peer *ptr);
	static int atualizaPredecessor(void* args, char *buffer);
	void Create(){
		this->sock_connection = CreateServer(this->PORT);
	}
	~Peer(){}
};


int Peer::join(Peer *A, char *buffer){
	cout << "CONECTANDO COM: " <<  A->IP << " " << A->PORT << endl;	
	TSocket sender = ConnectToServer(A->IP,A->PORT);	
	puts("Conectado...");
	cout << "Sender: "<< sender << endl;
	puts("Enviando minhas informações para a rede ...");
	if(WriteN(sender,buffer, BUFSIZE) < 0 )
		ExitWithError("WriteN failed");
	puts("Recebendo resposta da Rede p2p ....");
	if(ReadLine(sender,buffer, BUFSIZE) < 0 )
		ExitWithError("ReadLine failed");
	cout << "Recebi: "<< buffer;
	close(sender);
	cout << "Conexao fechada" << endl;
}
void* Peer::HandleRequest( void* args){	
	char buffer[BUFSIZE], usr_option;
	Peer *A = (Peer* ) args;	

	if(ReadLine(A->sock_peer, buffer , BUFSIZE ) < 0 ){
		printf("HandleRequest Message can't be read: %d, IP: %s\n",A->sock_peer,A->IP);
			return NULL;
	}

	usr_option = buffer[0];	
	printf("usr_option: %c\n",usr_option);
	switch(usr_option){
		case '1':	
			findsucessor(args,buffer);
			//Escrevo a resposta encontrada para o peer que me chamou
			if(WriteN(A->sock_peer, buffer, BUFSIZE) < 0 ){
				printf("Findsucessor failled\n");
				return NULL;
			}

		break;
	}
}
	
int Peer::findsucessor(void *client, char *buffer){
	Peer *A = (Peer* ) client;
	
	//Pego a msg na IDA, IP e PORTA para comparar.
	// Modularizar numa funcao, com apenas 1 chamada
	string parser(buffer);
	stringstream ss(parser);
	vector<string> v;
	while(ss>>parser){v.pb(parser);}
	string::size_type sz;
	char parser_IP[BUFSIZE];
	int parser_PORT;
	sprintf(parser_IP, "%s",v[1].c_str());
	parser_PORT = stoi(v[2], &sz );	
	
	int id_add, id_agora;
	//Pego o ID do peer que estou, e do peer que quero adicionar
	GeneratePeerId(&id_add, parser_IP,parser_PORT);
	GeneratePeerId(&id_agora, A->IP, A->PORT);
	
	//Crio o peer do no que estou adicionando, para passar para a funcao join do peer que estou no momento.	
	Peer *nxt_node = new Peer( parser_IP,parser_PORT );	
	if(A->suc != NULL)
	cout << "suc " << A->suc->IP << " " << A->suc->PORT << endl;
	cout << "buffer: " << buffer << endl;
	cout << "zero: " << A->zero->IP << endl;
	cout << "id_agora: " << id_agora << endl;
	cout << "id_add: " << id_add << endl;
	if(id_add>= id_agora) puts("1");
	//if(A->pre != NULL)
	//cout << "pre " << A->pre->IP << " " << A->pre->PORT << endl;
		
	// Se o id do peer que estou, tentando adicionar o peer novo
	// ou seja, id_agora > id_add, o Peer novo é pai do peer que estou no momento.
	puts("AQUIIIIIIIIIIIIIIIIIIIIIII");
	if(A->suc == NULL) puts("VAI DAR MERDA 1" );
	else
		cout << "suc: " << A->suc->IP << " " << A->suc->PORT << endl;

	if(A->pre == NULL) puts("VAI DAR MERDA 2" );
	else
		cout <<"pre: " << A->pre->IP  << " " << A->pre->PORT << endl;
	cout << "zero: " << A->zero->IP << " " << A->zero->PORT << endl;
	if( id_agora >= id_add ){
		puts("ENTROU AQUI 1");	
	// Envio a informacao, [SUC,PRE] 
		// Sucessor é o peer que estou e o predecessor e o peer
		//
		sprintf(buffer,"1 %s %d %s %d\n",A->IP,A->PORT, A->pre->IP, A->pre->PORT);	
	//	A->pre = nxt_node;
	}
	// Se o proximo PEER aponta para o zero ( ou seja nulo )
	// e o id_add > id_agora, entao id_agora é pai do peer add.
	// e o sucessor do peer add, é o zero.
	// SWAP entre os peers
	else if( strcmp(A->suc->IP, A->zero->IP) ==0 and A->suc->PORT == A->zero->PORT and id_add >= id_agora ){	
	//	suc = nxt_node;	
		puts("ENTROU AQUI 2");
		cout << "Cur: " << A->IP << " " << A->PORT << endl;
		cout << "Nxt: " << parser_IP <<" " << parser_PORT << endl;				
		sprintf(buffer,"2 %s %d  %s %d \n",A->zero->IP,A->zero->PORT,A->IP,A->PORT);			
	}
	// Se eu sou o zero, e tenho que adicionar o primeiro no seguinte
	else if( strcmp(A->suc->IP, A->zero->IP) == 0 and A->suc->PORT == A->zero->PORT and id_add <= id_agora){
		puts("Entrou AQUI 3 ");
		sprintf(buffer,"3 %s %d %s %d \n",A->zero->IP, A->zero->PORT, A->IP,A->PORT);
	} 
	/*
	// Se o sucessor é o zero
	//
	else if(A->suc->IP == A->zero->IP){
		printf("MEU ID: %d\n",id_agora);
		cout << "Entrou no meio " << endl;
		sprintf(buffer,"2 %s %d %s %d \n", A->IP,A->PORT, A->zero->IP, A->zero->PORT );
		cout << buffer << endl;
		pre = nxt_node;
	}
	
	else if( A->suc->IP != A->zero->IP and A->suc->id < id_add ){
		sprintf(buffer,"1 %s %d \n",parser_IP,parser_PORT);	
		A->join(suc, buffer);
	}
	*/
	else{
		puts("ENTROU ONDE NAO DEVERIA PORRA");
		sprintf(buffer,"1 %s %d \n",parser_IP,parser_PORT);
		A->join(suc,buffer);
	}
	//void GetInfo( char* buffer string &suc_IP,int &suc_port,string &pre_IP, int &pre_port ){
	puts("FIM DAS ALTERNATIVAS");
	string ret_suc_IP, ret_pre_IP;
	int    ret_suc_PORT, ret_pre_PORT;
	GetInfo( buffer, ret_suc_IP, ret_suc_PORT, ret_pre_IP, ret_pre_PORT );	
	puts("Debugging Return GetInfo findsucessor");
	cout << "Sucessor: " << ret_suc_IP << " " << ret_suc_PORT << endl;
	cout << "Predecessor: "<< ret_pre_IP << " " << ret_pre_PORT << endl; 

//ESSA DECISAO ESTA ERRADA!!! CORRIGIR DEVAGAR!	
	//Eu sou o sucessor do no, e preciso atualizar que o no e o meu predecessor

	// caso em que o primeiro tem que ser modificado
	if(ret_suc_IP == ret_pre_IP and ret_suc_PORT == ret_pre_PORT){
		A->suc = nxt_node;
		cout << "1 " << A->suc->IP << " " << A->suc->PORT << endl;
	}	
	else if( strcmp(A->IP,ret_suc_IP.c_str()) == 0 and A->PORT == ret_suc_PORT){	

		A->pre = nxt_node;
		cout <<"2 " << A->pre->IP << " " << A->pre->PORT << endl;	
	}
	// eu sou o predecessor daquele no, entao preciso atualizar o meu sucessor
	else if( strcmp(A->IP, ret_pre_IP.c_str()) == 0 and A->PORT == ret_pre_PORT){	
		A->suc = nxt_node;
		cout << "3 " << A->suc->IP << " " << A->suc->PORT << endl;
	}
}
void Peer:: Connection( Peer *ptr){
	while(true){
		if(tid == NUMTHREADS)
			ExitWithError("Number of threads is over");
	//	Peer *a = new Peer();
		puts("Esperando conexao");

		this->sock_peer = AcceptConnection( ptr->sock_connection );	
		
		printf("\n|Connection Started\n");
		printf("|-IP: %s\n",ptr->IP);
		printf("sock: %d\n", ptr->sock_peer);
		if(pthread_create(&threads[tid++], NULL, (THREADFUNCPTR) &Peer::HandleRequest,ptr))
			ExitWithError("pthread_create() failled");		
		
		printf("\n|Connection Closed\n");
		printf("|-IP: %s\n",ptr->IP);
		printf("sock: %d\n", ptr->sock_peer);
		if(ptr->suc != NULL )
			cout << ptr->suc->IP << " ---------------------  " << ptr->suc->PORT << endl;
		if(ptr->pre != NULL)
			cout << ptr->pre->IP << " _____________________ " << ptr->pre->PORT << endl;
	}
}


Peer * Peer::suc;
Peer * Peer::pre;
int Peer::id = 0;
int main(int argc, char ** argv){
  init();
  Peer *node, *zero;
  char buffer[BUFSIZE];
  sprintf(buffer,"1 %s %s \n",argv[3], argv[4]);	  
 Peer *ptr; 
 if (argc == 5 ){ 
	zero = new Peer(argv[1],atoi(argv[2]));
  	node = new Peer(argv[3],atoi(argv[4]));
 	
	node->zero = zero; 
	node->join(node->zero,buffer);
	node->zero->id = 0;
	node->suc = zero;
	puts("ENTROU NA REDE");
	string ret_suc_IP, ret_pre_IP;
	int    ret_suc_PORT, ret_pre_PORT;
	GetInfo( buffer, ret_suc_IP, ret_suc_PORT, ret_pre_IP, ret_pre_PORT );	
	puts("Debugging Return GetInfo findsucessor");
	cout << "Sucessor: " << ret_suc_IP << " " << ret_suc_PORT << endl;
	cout << "Predecessor: "<< ret_pre_IP << " " << ret_pre_PORT << endl; 
	char IP_suc[BUFSIZE], IP_pre[BUFSIZE];
	sprintf(IP_suc, "%s",ret_suc_IP.c_str());	
	sprintf(IP_pre, "%s",ret_pre_IP.c_str());

	Peer *_pre = new Peer(IP_suc, ret_pre_PORT);
	Peer *_suc = new Peer(IP_pre, ret_suc_PORT);
	node->suc = _suc;
	node->pre = _pre;	
	if( node->pre == NULL )
		puts("NULL");
 	else
   		cout << "PREDECESSOR: " << node->pre->IP << " " << node->pre->PORT << endl;
  	cout << "MEU SUCESSOR: ";

  	if(node ->suc == NULL)
		puts("NULL");
  	else	
		cout << " SUCESSOR: "   << node->suc->IP << " " << node->suc->PORT << endl;
	cout << "POS - JOIN" << endl;


 }
  else if(argc == 3){
  	 node = new Peer(argv[1],atoi(argv[2]));
  	 node->id = 0;
	 node->zero = node;
	 node->zero->id = 0;
	 node->suc = node;
  }
  else
	return 0*puts("Quantidade de parametros errados");
 ptr = node;
 node->Create(); 
 node->Connection(ptr);
 return 0;
}
