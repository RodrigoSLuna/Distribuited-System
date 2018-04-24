# October 2006
# Prof.: Silvana Rossetto
#------------------------------------------------------------

VERSION = 1.0
CLIENT_A = client_ativo.cpp
CLIENT_U = client_users.cpp
CLIENT_P = client_passivo.cpp
SERVER   = srv.cpp

PAS = pas
ATI = ati
USERS = users
SRV = serv
#-------------------------------------------------------------
# the rest of the file should not change
INC = ../include
SRC = ../src
OBJ = ../obj
BIN = ../bin

CC = g++ -std=c++11
CFLAGS = -I$(INC)
LFLAGS = -lpthread

RM = rm

# make all
all:  $(OBJ)/mysocket.o $(BIN)/$(SRV) $(BIN)/$(PAS) $(BIN)/$(ATI) $(BIN)/$(USERS)

$(OBJ)/mysocket.o: $(SRC)/mysocket.cpp $(INC)/mysocket.h $(INC)/structs.h
	$(CC) -c $(CFLAGS) -o $@ $(SRC)/mysocket.cpp

$(BIN)/$(ATI): $(OBJ)/mysocket.o $(CLIENT_A)
	$(CC) $(CFLAGS) -o $@ $(OBJ)/mysocket.o $(CLIENT_A) $(LFLAGS)

$(BIN)/$(USERS): $(OBJ)/mysocket.o $(CLIENT_U)
	$(CC) $(CFLAGS) -o $@ $(OBJ)/mysocket.o $(CLIENT_U) $(LFLAGS)

$(BIN)/$(PAS): $(OBJ)/mysocket.o $(CLIENT_P)
	$(CC) $(CFLAGS) -o $@ $(OBJ)/mysocket.o $(CLIENT_P) $(LFLAGS)

$(BIN)/$(SRV): $(OBJ)/mysocket.o $(SERVER)
	$(CC) $(CFLAGS) -o $@ $(OBJ)/mysocket.o $(SERVER) $(LFLAGS)


version:
	@echo $(VERSION) "(Draft Version!)"
# clean
clean:
	$(RM) -rf $(OBJ)/*.o  $(BIN)/*
