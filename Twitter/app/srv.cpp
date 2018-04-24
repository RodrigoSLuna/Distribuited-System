#include <pthread.h>
#include "structs.h"
#define MAX_NR_USERS 20
#define MAX_SIZE_NAME 40
using namespace std;
class Node{
public: 
  char *IP;
  unsigned int PORT;
  TSocket sock;

  Node(){}
  Node(char *IP, unsigned int &PORT){
    this->IP = IP;
    this->PORT = PORT; 
  }
  ~Node(){}

};

class Client : public Node{
public:
  string UserLogin;
  Client(char *IP, unsigned int PORT ): Node( IP,PORT) { }
  Client(): Node() {}

  bool operator < ( const Client *o) const { return this->UserLogin < o->UserLogin; }
};

void getIP(TSocket sock, char* A){
  unsigned int cliLen;
  struct sockaddr_in cliAddr;
  memset((void *) &cliAddr, 0, sizeof(cliAddr));
  cliLen = sizeof(cliAddr);
  printf("sock: %d\n",sock);
  if (getpeername(sock, (struct sockaddr *) &cliAddr, &cliLen)) {
    ExitWithError("Error in getting IP");
  }
  strcpy(A,inet_ntoa(cliAddr.sin_addr));
}

// // //Making consts for the programmer cannot modify this pointer here.

class Server : public Node
{


public:
  pthread_t threads[NUMTHREADS];
  static map<char*, Client*> IpToClient;
  static set< Client *> clients; 
  static set< string >  logins_used;
  static map< string, Client* > name_refer;
  static map< string, set< pair< string , int >  > >followers;
  int tid; // Thread_ids;
  
  Server(){}
  Server(char *IP, unsigned int PORT) : Node( IP,PORT) { }
  ~Server(){}


  void Create(){
    this->sock = CreateServer(this->PORT);
  }
 static int ResponseOkUser(const TSocket &sock);
 static int addUser( void* client , char *buffer);
 static void ResponseExcepetionUser(const int &op, const TSocket &sock);
 static void* HandleRequest(void* args);
 static int Subscribe(void* client, char *buffer);
 static int Publish(void* client, char *buffer_args);
 static int DeleteProfile(void* client, char*buffer);
 void Connection();  
};



void Server::Connection(){
    void* retval;
    while( true ){
      if(tid == NUMTHREADS)
        ExitWithError("Number of threads is over");
    
      puts("Waitting Connection...");


      // Check if the client already exist!!!! cuz the client could change his name! or something else.
      // Estou criando um novo Client sempre... ruim <, para cada conexão!
      Client *a = new Client();
      
      int sock_aux = AcceptConnection( this->sock  );
      char IP[IPSIZE];
      getIP( sock_aux ,IP );

      if(IpToClient.find( IP ) != IpToClient.end() ){
        a = IpToClient[IP];
      }
      else{
        a->IP = IP;
        a->sock = sock_aux;
        IpToClient[IP] = a;
      }
      printf("\n|Connection Started\n");
      printf("|-IP: %s\n", a->IP);
      printf("sock: %d\n",a->sock);
      
      if(pthread_create(&threads[tid++], NULL, (THREADFUNCPTR) &Server::HandleRequest ,a) )
        ExitWithError("pthread_create() failed");
      }
    }

void Server:: ResponseExcepetionUser(const int &op, const TSocket &sock){
  if(op == 0)
    if(WriteN(sock, "0 \n", 4) < 0 )
    ExitWithError("WriteN() failed");
}

void* Server:: HandleRequest(void* args){
    char buffer[ BUFSIZE ],user_option;  
    if(args == NULL ) puts("NULL");
    Client *A = (Client* )args;

    if (ReadLine(A->sock, buffer, BUFSIZE) < 0) {
      printf("\nRequest message can't be read\n");
      return NULL;
    }
    user_option = buffer[0];
    int ok;
    switch(user_option){
      case '1':
        ok = addUser( args , buffer);
        if(ok == 1){
          printf("\n+\n+ %s\n+ ADD\n+\n", A-> IP);
          ResponseOkUser(A->sock);
        }
        else{
          printf("\n+\n+ %s\n+ CANNOT ADD\n+\n", A-> IP);
          ResponseExcepetionUser(ok, A->sock);
        }
        break;
      case '2':
        ok =  Subscribe( args, buffer );
        if(ok == 1){
          printf("\n+\n+ %s\n+ ADD\n+\n", A-> IP);
          ResponseOkUser(A->sock);
        }
        else{
          printf("\n+\n+ %s\n+ CANNOT FIND SUBSCRIBER\n+\n", A-> IP);
          ResponseExcepetionUser(ok, A->sock);
        }
        break;
      case '3':
        // O que quer dizer com codigo da subscricao, pensei que o nome era a chave primaria visto que so pode ter 1.
         break;
      case '4':
        ok = Publish( args, buffer );
        

        if(ok){
          printf("\n+\n+ %s\n+ PUBLISH\n+\n", A-> IP);
          ResponseOkUser(A->sock);
        }
        else{
          printf("\n+\n+ %s\n+ CANNOT PUBLISH\n+\n", A-> IP);
          ResponseExcepetionUser(ok, A->sock);

        }

        break;  
      case '5':
        ok = DeleteProfile(args,buffer);
        if(ok){
          printf("\n+\n+ %s\n+ DELETE\n+\n", A-> IP);
          ResponseOkUser(A->sock);
        }
        else{
          printf("\n+\n+ %s\n+ CANNOT DELETE\n+\n", A-> IP);
          ResponseExcepetionUser(ok, A->sock);

        }
      break;
      default:
      printf("\nInvalid option in user\n");
      if (WriteN(client, "0\n", 2) <= 0)
        ExitWithError("WriteN() in ResponseAllUsers failed");

    }
  } 



int Server::DeleteProfile(void* client, char *buffer){
  Client *A = ( (Client *) client); // making a cast for a object
    // Modularizar o parser, fazer uma funcao para isso!
    string parser(buffer_args);
    stringstream ss(parser); //stream that will parser
    vector<string> v;        // vector with the strings;
    while(ss >> parser){ v.pb(parser); }

    string UserLogin = v[1];

    if(logins_used.find(UserLogin) == logins_used.end())
      return 0;

    // Delete things here


}

int Server::Publish(void* client, char *buffer_args){
    Client *A = ( (Client *) client); // making a cast for a object
    // Modularizar o parser, fazer uma funcao para isso!
    string parser(buffer_args);
    stringstream ss(parser); //stream that will parser
    vector<string> v;        // vector with the strings;
    while(ss >> parser){ v.pb(parser); }

    string UserLogin = v[1];
    string MSG = "";
    for(int i = 2;i<v.size();i++) MSG += (i != 2? " " + v[i] : v[i] );
  

    if( logins_used.find(UserLogin) == logins_used.end() )
      return 0;

    

    char buffer[BUFSIZE];
    strcpy(buffer,MSG.c_str());
    // Possivel paralelizar essa parte do envio!
    for(auto user: followers[UserLogin]){

      //Abro uma conexao com cada um, e envio a MSG, posso fazer essa parte em paralelo
      Client *follow = name_refer[ user.st ];
      int PORT = user.nd;
      

      TSocket sender = ConnectToServer(follow-> IP ,PORT);
      if(WriteN(sender, buffer , BUFSIZE ) < 0){
        ExitWithError("WriteN Failed()");
      }
      close(sender);

    }
    return 1;

}

int Server:: Subscribe(void* client, char* buffer){
    Client *A = ( (Client *) client); // making a cast for a object
    string parser(buffer);
    stringstream ss(parser); //stream that will parser
    vector<string> v;        // vector with the strings;
    while(ss >> parser){ v.pb(parser); }
    
    if(v.size() != 3) 
      return 2;
    
    string follow = v[1];
    string port   = v[2];

    if(logins_used.find(follow) == logins_used.end())
      return 0;

    followers[follow].insert( mp( A->UserLogin, stoi(port) )) ;

    return 1;
}

int Server:: addUser( void* client , char *buffer){
    Client *A = ( (Client *) client); // making a cast for a object
    string parser(buffer);
    parser = parser.substr(2); // Avoid operation and the first space  

    int pos_space = parser.find(" ");
    if(pos_space == string::npos){
      return 2;
    }
    string name = parser.substr(0,pos_space);
    if(logins_used.find(name) != logins_used.end() )
      return 0;

    A->UserLogin = name;
    logins_used.insert(name);
    clients.insert(A);
    name_refer[name] = A;
    return 1;
}

int Server:: ResponseOkUser(const TSocket &sock){
  if(WriteN(sock, "1 \n", 4) < 0 )
    ExitWithError("WriteN() failed");
}


/*Undefined reference for the class*/
set<Client *> Server::clients; // Undefined reference solved here <
set< string > Server::logins_used;
map<string, Client*> Server::name_refer;
map< string, set< pair< string , int >  > > Server::followers;
map<char*, Client*> Server::IpToClient;
int main(int argc, char ** argv){

  if (argc != 3) {
   ExitWithError("Usage: client <remote server IP> <remote server Port> ");
  }

  Server *srv = new Server(argv[1], atoi(argv[2] ));
  srv->Create();
  srv->Connection();


  delete srv;
  return 0;
}