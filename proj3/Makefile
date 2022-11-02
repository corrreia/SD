# /* -------------------------------------------------------------
# * Grupo: 49
# * Membros: Miguel Pato, fc57102
# *          Tomás Correia, fc56372
# *          João Figueiredo, fc53524
# *
# */

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
