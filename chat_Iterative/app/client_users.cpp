#include <string>
#include "mysocket.h"
#include "structs.h"
#include <iostream>
#include <time.h>
using namespace std;

void operations(){
  printf("\033[2J"); // Clear screen
  puts("\t\t < 2 > Para receber lista atualizada");
  puts("\t\t < 3 > Para receber lista atualizada a cada 10 segundos");
  puts("\t\t < 4 > Para para interromper execução");
}
void imprime_lista(Message a){
  std::vector<std::string> L = a.lista;
  printf("****** Onlines: %d ******* < Nome > **** < IP > **** < Porta > \n",L.size());
  for(int i = 0;i<L.size();i++){
    cout << L[i] << endl;
  }
}
void get_message(Message a, int op ,char *servIP, int &servPort ){
      TSocket sock;
      sock = ConnectToServer(servIP, servPort);
      char buffer[BUFSIZE];
      sprintf(buffer,"%d \n",op);
      if( WriteN(sock, buffer, 99 ) < 0  ){
        ExitWithError(" WriteN() failed ");
      }

      if( ReadLine(sock,buffer,99) < 0 ){
        ExitWithError(" 1ReadLine() failed ");
      }
      int num_onlines = stoi(buffer);
      for(int i = 0;i<num_onlines;i++){
        char buffer[BUFSIZE];
        string stringRead = "";
        if(ReadLine(sock, buffer, 99) < 0 ){
          puts("ReadLine Failed");
          break;
        }

        unsigned int len = strlen(buffer);
        stringRead.append(buffer,len);
        a.lista.pb(stringRead);
      }
      imprime_lista(a);
      close(sock);
}
int send_receive(Message a,char *servIP, int servPort){
   /* Write msg */
  operations();
  int op;
  scanf("%d",&op);
  // Me conecto apenas quando tiver a operacao

  if(op == 4) return -1;
  // Envia para o servidor o tipo da operacao
  if(op == 2){
    get_message(a,op,servIP,servPort);
  }
  /*
  else if(op == 3){
    // Pega a mensagem a cada 10 segundos;
    // Uma thread imprime, enquanto a outra thread fica esperando.
  }
  */
  return 1;
}

int main(int argc, char *argv[]) {
  TSocket sock;
  char *servIP;                /* server IP */
  unsigned short servPort;     /* server port */
  Message online;
  int n;

  if (argc != 3) {
    ExitWithError("Usage: client <remote server IP> <remote server Port>");
  }
  // To mantendo a conexao ligada... ruim
  servIP = argv[1];
  servPort = atoi(argv[2]);

  while( send_receive( online,servIP,servPort) != -1 ){;}
  return 0;
}
