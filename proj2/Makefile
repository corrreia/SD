# 	make client-lib.o: Compilar os ficheiros client_stub.c, network-client.c,
# data.c e entry.c (e eventuais ficheiros adicionais “.c” criados na concretização
# do cliente), sem a aplicação interativa com o main(), criando uma biblioteca client-lib.o
# (Usar linker ld com opção -r: ld -r in1.o in2.o [...] -o clientlib.o);

# tree-client: Compilar todo o código que pertence ao cliente, criando a aplicação cliente
# tree-client;

# tree-server: Compilar todo o código que pertence ao servidor, criando a aplicação
# servidor tree-server;

# clean: Remover todos os ficheiros criados pelos targets acima

#server files are in server/
#client files are in client/

PROTOC_DIR = /usr/

CC = gcc
CFLAGS = -Wall -g -O2 -I${PROTOC_DIR}include/
LDFLAGS = ${PROTOC_DIR}lib/x86_64-linux-gnu/libprotobuf-c.a  #for debian/ubuntu (sudo apt-get install protobuf-compiler protobuf-c-compiler libprotobuf-c-dev)
#LDFLAGS = ${PROTOC_DIR}lib64/libprotobuf-c.so  #for fedora/redhat (sudo dnf install protobuf-compiler protobuf-c-compiler libprotobuf-c-devel

SERVER = server
CLIENT = client

SERVER_SRC = $(wildcard $(SERVER)/source/*.c)
CLIENT_SRC = $(wildcard $(CLIENT)/source/*.c)

SERVER_OBJ = $(patsubst $(SERVER)/source/%.c,$(SERVER)/object/%.o,$(SERVER_SRC))
CLIENT_OBJ = $(patsubst $(CLIENT)/source/%.c,$(CLIENT)/object/%.o,$(CLIENT_SRC))

SERVER_BIN = $(SERVER)/binary/tree-server
CLIENT_BIN = $(CLIENT)/binary/tree-client

SERVER_INC = $(wildcard $(SERVER)/include/*.h)
CLIENT_INC = $(wildcard $(CLIENT)/include/*.h)

all: client-lib.o tree-client tree-server

client-lib.o: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $(CLIENT)/object/client-lib.o $(CLIENT_OBJ) $(LDFLAGS)

tree-client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $(CLIENT_BIN) $(CLIENT_OBJ) $(LDFLAGS)

$(CLIENT)/object/%.o: $(CLIENT)/source/%.c $(CLIENT_INC)
	$(CC) $(CFLAGS) -c -o $@ $<

tree-server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $(SERVER_BIN) $(SERVER_OBJ) $(LDFLAGS)

$(SERVER)/object/%.o: $(SERVER)/source/%.c $(SERVER_INC)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(SERVER_OBJ) $(CLIENT_OBJ) $(SERVER_BIN) $(CLIENT_BIN) $(CLIENT)/object/client-lib.o
