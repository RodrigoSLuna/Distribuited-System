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
//Parametros passados como referencia.
string get_selfpath(){
	//Pega o Path que o programa esta rodando atualmente
	char buff[PATH_MAX];
	ssize_t len = ::readlink("/proc/self/exe",buff,sizeof(buff) -1 );
	if(len != -1){
		buff[len] = '\0';
		return string(buff);
	}


}
bool CreateDir( char *IP, int PORT){
	string PATH = "mkdir -p " + string("serv")+string(IP) + "/" +to_string(PORT);
	const int dir_err = system(PATH.c_str());
	if(dir_err == -1){
		puts("Erro ao criar o diretorio");
		return false;
	}
	return true;
}
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
}
void GetInfo( char* buffer, string &_IP,int &_port, int &key){
	string::size_type sz;		
	string parser(buffer);
	stringstream ss(parser);
	vector<string> v;
	while(ss>> parser){v.pb(parser);}
	_IP   = v[1];
	_port = stoi(v[2], &sz);
	key      = stoi(v[3], &sz);

}
void GetInfo( char* buffer, string &_IP,int &_port){
	cout <<"buffer "<< buffer;
	string::size_type sz;		
	string parser(buffer);
	stringstream ss(parser);
	vector<string> v;
	while(ss>> parser){v.pb(parser);}
	_IP   = v[1];
	_port = stoi(v[2], &sz);
}
void GetInfo(char *buffer, int &valor){
	string::size_type sz;		
	string parser(buffer);
	stringstream ss(parser);
	vector<string> v;
	while(ss>> parser){v.pb(parser);}
	valor = stoi(v[1], &sz);
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
	*id = val;
}
void GenerateKey(int *id, const char *buffer){
	int val = 0;
	int size = strlen(buffer);
	for(int i = 0; buffer[i] != '\0';i++){
		if( (buffer[i] >= 'a' and buffer[i] <= 'z') or (buffer[i]>='A' and buffer[i] <= 'Z') ){
			
			val += ( ( (buffer[i] - '0')*( (long long)pow(10,size) )%pw[m] )) %pw[m];
		}
		size--;
		while( val > pw[m] ) val -= pw[m];
	}
	*id = val;

}
class Peer{
	public:
	int tid; //Thread_Lists;
	pthread_t threads[NUMTHREADS];
	char IP[BUFSIZE];
	int PORT;
	string PATH;
	static int id;
	vector<int> FILES; // Possivelmente vai virar 1 descritor de arquivo!. 
	TSocket sock_connection, sock_peer;
	Peer *zero;
	static Peer *pre, *suc;
	Peer(){}
	Peer(char *IP,int PORT){	    
		memcpy(this->IP,IP,BUFSIZE);
		this->PORT = PORT;
		GeneratePeerId(&id,IP,PORT);
		if( !CreateDir(this->IP,this->PORT) ){
			cout << " NAO FOI POSSIVEL CRIAR DIRETORIO " << endl;
		}
		this->PATH = get_selfpath()+string(IP)+"/"+to_string(PORT)+"/";
		cout << this->PATH << endl;		
	}
	int join(Peer *A, char *buffer);
	int send(Peer *A, char *buffer);
	int receive(Peer *A, char *buffer);
	static void* HandleRequest( void* args );
	static int findsucessor(void *client, char *buffer);
	static int upload_file (void *client, char *buffer);
	static int receive_file (void *client, char *buffer);
	static int send_file (void *client, char *buffer);
	void Connection( Peer *ptr);
	static int atualizaPredecessor(void* args, char *buffer);
	void Create(){
		this->sock_connection = CreateServer(this->PORT);
	}
	~Peer(){}
};

int Peer:: receive(Peer*A, char *buffer){
	cout << "CONECTANDO COM: " <<  A->IP << " " << A->PORT << endl;	
	cout << "Nome do arquivo: " << buffer << endl;	
	char send[BUFSIZE];
	sprintf(send, "4 %s \n", buffer);
	
	TSocket sender = ConnectToServer(A->IP,A->PORT);	
	puts("Conectado...");		

	cout << "ENVIANDO NOME DO ARQUIVO QUE QUERO: " << buffer << endl;
	if(WriteN(sender,send, BUFSIZE) < 0 )
		ExitWithError("WriteN failed");
	
	//BUGADO
	/*
	cout << "Esperando confirmaçao do recebimento do nome do arquivo" << endl;
	if(ReadLine(sender,send,BUFSIZE) <0 )
		ExitWithError("#waitting back message failled");
	puts("Confirmou o recebimento");
	puts("Enviando conteúdo... ");
	char filename[BUFSIZE];	
	sprintf(filename,"%s%s",this->PATH.c_str(),buffer);
	cout <<"filename " << filename << endl;
	ifstream myfile( filename );
	string line;
	int number_of_lines = 0;
	
	while(getline(myfile,line)){
		cout << line << endl;
		number_of_lines++;
	}
	cout << "# lines " <<  number_of_lines << endl;
	//Envio a quantidade de linhas	
	memset(send,'\0',sizeof(send));
	sprintf(send,"%d \n",number_of_lines);	
	cout << "ENVIANDO: " << send;
	if(WriteN(sender, send, BUFSIZE) < 0 )
		ExitWithError("#lines WriteN failed");	
	puts("Conteudo enviado...");
	puts("Aguardando confirmação de recebimento");
	
	if(ReadN(sender,send,BUFSIZE) <0 )
		ExitWithError("#waitting back message failled");
	puts("Confirmou o recebimento");
	cout << send << endl;
	*/
	close(sender);
	cout << "Conexao fechada" << endl;
}
int Peer:: send(Peer*A, char *buffer){
	cout << "SEND" << endl;
	cout << "CONECTANDO COM: " <<  A->IP << " " << A->PORT << endl;	
	cout << "Nome do arquivo: " << buffer << endl;	
	char send[BUFSIZE];
	sprintf(send, "3 %s \n", buffer);
	
	TSocket sender = ConnectToServer(A->IP,A->PORT);	
	cout << "Conectado ... " << sender << endl;
	cout << "ENVIANDO NOME DO ARQUIVO: " << buffer << endl;
	if(WriteN(sender,send, BUFSIZE) < 0 )
		ExitWithError("WriteN failed");
	/*

	cout << "Esperando confirmaçao do recebimento do nome do arquivo" << endl;
	if(ReadN(sender,send,BUFSIZE) <0 )
		ExitWithError("#waitting back message failled");
	cout << send << endl;
	puts("Confirmou o recebimento");

	char buffer2[BUFSIZE] = "\0";

	sprintf(buffer2,"2 \n",BUFSIZE);	
	cout << "ENVIANDO: " << buffer2;
	if(WriteN(sender, buffer2, BUFSIZE) < 0 )
		ExitWithError("#lines WriteN failed");	
	puts("Conteudo enviado...");
	
	char filename[BUFSIZE];	
	sprintf(filename,"%s%s",this->PATH.c_str(),buffer);
	cout <<"filename " << filename << endl;
	ifstream myfile( filename );
	string line;
	int number_of_lines = 0;
	
	while(getline(myfile,line)){
		cout << line << endl;
		number_of_lines++;
	}
	cout << "# lines " <<  number_of_lines << endl;
	//Envio a quantidade de linhas	
	char buffer2[BUFSIZE] = "\0";

	sprintf(buffer2,"%d \n",number_of_lines);	
	cout << "ENVIANDO: " << send;
	if(WriteN(sender, buffer2, BUFSIZE) < 0 )
		ExitWithError("#lines WriteN failed");	
	puts("Conteudo enviado...");
	
	*/
	close(sender);
	cout << "Conexao fechada " << sender << endl;
}
int Peer:: send_file(void *client, char* buffer){
	Peer *A = (Peer*) client;
	//Recebo nome do arquivo, Pego o nome do arquivo
	string name;
	string parser(buffer);
	stringstream ss(parser);
	string filename;
	int i = 0;
	while(ss>>parser){
		if(i == 1) filename = parser;
		i++;
	}	
	cout << "Abre Arquivo e envia!" << endl;	
	cout << A->PATH+filename <<endl;


}	

int Peer:: receive_file(void *client, char* buffer){
	cout << "RECEIVE_FILE " << endl;
	Peer *A = (Peer*) client;
	//Recebo nome do arquivo, Pego o nome do arquivo
	string name;
	string parser(buffer);
	stringstream ss(parser);
	string filename;
	int i = 0;
	while(ss>>parser){
		if(i == 1) filename = parser;
		i++;
	}	
	cout << A->IP << " " << A-> PORT << " "<< A->sock_peer << endl;
	cout << A->PATH+filename <<endl;
	ofstream o(A->PATH+filename);
	o <<"TESTE " << endl;
	sprintf(buffer,"ok \n", BUFSIZE);
	/*
	cout << "->> Enviando que recebeu o nome\n" << buffer;	
	if(WriteN(A->sock_peer, buffer, BUFSIZE) < 0 )
		ExitWithError("WriteN COnfirmando recebimento NOME failed");	
	cout << "Recebendo Quantidade de linhas do arquivo " << endl;
	char buffer2[BUFSIZE];
	
	if(ReadN(A->sock_peer, buffer2, BUFSIZE) < 0 )
		ExitWithError("WriteN COnfirmando recebimento NOME failed");	
	cout << "buffer2: " << buffer2 << endl;
	cout << "Recebendo Quantidade de linhas do arquivo " << endl;
	cout << strlen(buffer2) << endl;
	cout << buffer2 << endl;
	*/
}
int Peer::join(Peer *A, char *buffer){
	cout << "CONECTANDO COM: " <<  A->IP << " " << A->PORT << endl;	
	cout << "ENVIANDO: " << buffer << endl;
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
	cout << "Conexao fechada "  << sender << endl;
}
void* Peer::HandleRequest( void* args){	
	char buffer[BUFSIZE], usr_option;
	Peer *A = (Peer* ) args;	
	
	if(ReadLine(A->sock_peer, buffer , BUFSIZE ) < 0 ){
		printf("HandleRequest Message can't be read: %d, IP: %s\n",A->sock_peer,A->IP);
			return NULL;
	}
	usr_option = buffer[0];	
	cout << "Requisitando: "<< buffer << " do socket " << A->sock_peer << endl;
	switch(usr_option){
		case '1':	
			findsucessor(args,buffer);
			//Escrevo a resposta encontrada para o peer que me chamou
			if(WriteN(A->sock_peer, buffer, BUFSIZE) < 0 ){
				printf("Findsucessor failled\n");
				return NULL;
			}
			break;
		case '2':
			upload_file(args,buffer);
			if(WriteN( A->sock_peer, buffer,BUFSIZE) < 0 ){
				printf("upload_file failled\n");
				return NULL;
			}
			break;
		case '3':
			receive_file(args,buffer);
			break;
		case '4':
			send_file(args,buffer);
			break;
	}
}
int Peer::upload_file(void*client , char*buffer ){
	Peer *A = (Peer* ) client;
	string IP;
	
	int id_busca, id_agora; // ID VAI SER O IDENTIFICADOR do peer que esta com o conteudo		
	string add_IP; // IP e PORTA que estao adicionando esse conteudo na rede
	int add_PORT;
	GetInfo(buffer,add_IP , add_PORT, id_busca);

	GeneratePeerId(&id_agora, A->IP, A->PORT);		
	
	if( id_agora >= id_busca ){	
		// Envio a informacao, PEER que deve ficar com o conteúdo	
		sprintf(buffer,"2 %s %d\n",A->IP,A->PORT);	
	
	}
	// Se o proximo PEER aponta para o zero ( ou seja nulo )
	// e o conteudo que quero adicionar e maior do que o valor atual, entao eu sou o peer que vai guardar esse conteudo	
	else if( strcmp(A->suc->IP, A->zero->IP) ==0 and A->suc->PORT == A->zero->PORT and id_busca >= id_agora ){	
		sprintf(buffer,"2 %s %d \n",A->IP,A->PORT);			
	}
	else if( strcmp(A->suc->IP, add_IP.c_str() ) == 0 and A->suc->PORT == add_PORT and id_busca >= id_agora ){	
		int id_suc;
		GeneratePeerId( &id_suc, A->suc->IP, A->suc->PORT);		
		if( id_suc >= id_busca)
			sprintf(buffer,"2 %s %d \n",A->suc->IP,A->suc->PORT);			
		else
			sprintf(buffer,"2 %s %d \n",A->IP,A->PORT);			
	} 
	else{	
		sprintf(buffer,"2 %s %d %d \n",add_IP.c_str(), add_PORT,id_busca);
		A->join(suc,buffer);
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
	//if(A->pre != NULL)
	//cout << "pre " << A->pre->IP << " " << A->pre->PORT << endl;
		
	// Se o id do peer que estou, tentando adicionar o peer novo
	// ou seja, id_agora > id_add, o Peer novo é pai do peer que estou no momento.
	if( id_agora >= id_add ){
		// Envio a informacao, [SUC,PRE] 
		// Sucessor é o peer que estou e o predecessor e o peer
		//
		sprintf(buffer,"1 %s %d %s %d\n",A->IP,A->PORT, A->pre->IP, A->pre->PORT);	
	}
	// Se o proximo PEER aponta para o zero ( ou seja nulo )
	// e o id_add > id_agora, entao id_agora é pai do peer add.
	// e o sucessor do peer add, é o zero.
	// SWAP entre os peers
	else if( strcmp(A->suc->IP, A->zero->IP) ==0 and A->suc->PORT == A->zero->PORT and id_add >= id_agora ){	
	//	suc = nxt_node;	
		sprintf(buffer,"2 %s %d  %s %d \n",A->zero->IP,A->zero->PORT,A->IP,A->PORT);			
	}
	// Se eu sou o zero, e tenho que adicionar o primeiro no seguinte
	else if( strcmp(A->suc->IP, A->zero->IP) == 0 and A->suc->PORT == A->zero->PORT and id_add <= id_agora){
		sprintf(buffer,"3 %s %d %s %d \n",A->zero->IP, A->zero->PORT, A->IP,A->PORT);
	} 

	//Passa a mensagem para o proximo peer
	else{
		sprintf(buffer,"1 %s %d \n",parser_IP,parser_PORT);
		A->join(suc,buffer);
	}
	//void GetInfo( char* buffer string &suc_IP,int &suc_port,string &pre_IP, int &pre_port ){
	string ret_suc_IP, ret_pre_IP;
	int    ret_suc_PORT, ret_pre_PORT;
	GetInfo( buffer, ret_suc_IP, ret_suc_PORT, ret_pre_IP, ret_pre_PORT );	

	//Eu sou o sucessor do no, e preciso atualizar que o no e o meu predecessor
	// caso em que o primeiro tem que ser modificado
	if(ret_suc_IP == ret_pre_IP and ret_suc_PORT == ret_pre_PORT){
		A->suc = nxt_node;
	}	
	else if( strcmp(A->IP,ret_suc_IP.c_str()) == 0 and A->PORT == ret_suc_PORT){	

		A->pre = nxt_node;
	}
	// eu sou o predecessor daquele no, entao preciso atualizar o meu sucessor
	else if( strcmp(A->IP, ret_pre_IP.c_str()) == 0 and A->PORT == ret_pre_PORT){	
		A->suc = nxt_node;
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
		void *retval;	
		pthread_join(threads[tid-1],&retval);
		printf("\n|Connection Closed\n");
		printf("|-IP: %s\n",ptr->IP);
		printf("sock: %d\n", ptr->sock_peer);
	}
}
void* HandleRequest_Menu(void* args){
	Peer *A = (Peer*)args;
	
	while(true){
		puts("\t\tSelecione uma das opções abaixo:");
		puts("\t\t2 --- UPLOAD FILE");
		puts("\t\t3 --- Download FILE");
		int op;
		scanf("%d",&op);
		char buffer[BUFSIZE];
		char file_name[BUFSIZE];
		if( op != 2 and op != 3)
			puts("\t\tOPÇÃO INVALIDA");	
		else if(op == 2) {
			puts("\t\tDigite nome do arquivo");			
	
			cin >> buffer;
			sprintf(file_name,"%s",buffer);	
			int key,my_id;
			GenerateKey(&key, buffer);

			GeneratePeerId( &my_id, A->IP, A->PORT );
			A->id = my_id;
			sprintf(buffer, "2 %s %d %d \n",A->IP, A->PORT,key);
			//cout << "Meu IP: " << A->IP << " MINHA PORTA " << A->PORT << endl;
			//cout << "Key: " << key << " my_id " << A->id << endl;
			// O peer que devera ficar com o arquivo, esta a esquerda do no
			if(key < A->id) {
				A->join(A->zero, buffer);		
			}
			// O peer que devera ficar com o arquivo, esta a direita do no
			else if( key > A->id and A->suc->IP != A->zero->IP and A->suc->PORT != A->zero->PORT ){
				A->join(A->suc, buffer);
			}
			else break;
		}
		else if(op == 3){
			puts("\t\tDigite nome do arquivo");			
			cin >> buffer;
			sprintf(file_name,"%s",buffer);	
			int key,my_id;
			GenerateKey(&key, buffer);

			GeneratePeerId( &my_id, A->IP, A->PORT );
			A->id = my_id;
			sprintf(buffer, "2 %s %d %d \n",A->IP, A->PORT,key);
			cout << "Meu IP: " << A->IP << " MINHA PORTA " << A->PORT << endl;
			cout << "Key: " << key << " my_id " << A->id << endl;
			// cout << "O peer que devera ficar com o arquivo, esta a esquerda do no
			if(key < A->id) {
				A->join(A->zero, buffer);		
			}
			// O peer que devera ficar com o arquivo, esta a direita do no
			else if( key > A->id and A->suc->IP != A->zero->IP and A->suc->PORT != A->zero->PORT ){
				cout << "ENTROU AQUI " << endl;
				cout << A->suc->IP << " " << A->suc->PORT << endl;
				A->join(A->suc, buffer);
			}
			else break;
		}	
		//	else{
		// Se nao eu fico com o conteudo
		//		sprintf(buffer, "3 %s %d \n",A->IP, A->PORT);	
		//	}		
			
			string con_IP;
			char parser_IP[BUFSIZE];
			int con_PORT;
			GetInfo(buffer, con_IP, con_PORT);		
			sprintf(parser_IP,"%s",con_IP.c_str());
			cout << "VAI SE CONECTAR COM: " << parser_IP << " " << con_PORT << endl;	
			Peer *receiver = new Peer(parser_IP, con_PORT);
			if(op == 2)
				A->send(receiver, file_name);		
			else if(op == 3)
				A->receive(receiver,file_name);
		
	}	
}
void* HandleRequest_Connection(void* args){
	Peer* A = (Peer *) args;
	puts("\tAguardando conexões");
	while( true ){ A->Connection(A);}
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
	pthread_t menu_thread,connection_thread; 
	if (argc == 5 ){ 
		zero = new Peer(argv[1],atoi(argv[2]));
  		node = new Peer(argv[3],atoi(argv[4]));
 	
		node->zero = zero; 
		node->join(node->zero,buffer);
		node->zero->id = 0;
		node->suc = zero;
	
		string ret_suc_IP, ret_pre_IP;
		int    ret_suc_PORT, ret_pre_PORT;
		GetInfo( buffer, ret_suc_IP, ret_suc_PORT, ret_pre_IP, ret_pre_PORT );	
		char IP_suc[BUFSIZE], IP_pre[BUFSIZE];
		sprintf(IP_suc, "%s",ret_suc_IP.c_str());	
		sprintf(IP_pre, "%s",ret_pre_IP.c_str());

		Peer *_pre = new Peer(IP_suc, ret_pre_PORT);
		Peer *_suc = new Peer(IP_pre, ret_suc_PORT);
		node->suc = _suc;
		node->pre = _pre;	
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
 //	node->Connection(ptr);
	if(pthread_create(&menu_thread,       NULL, HandleRequest_Menu,       (void*) ptr)){	
		ExitWithError("Menu failled ()");
	}
	if(pthread_create(&connection_thread,NULL, HandleRequest_Connection, (void*)ptr) ){
		ExitWithError("Connection Failled");
	}


	void* retval;
	pthread_join(menu_thread,&retval);
	pthread_join(connection_thread,&retval);
	return 0;
}
