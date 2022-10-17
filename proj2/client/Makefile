# -------------------------------------------------------------
# Grupo: 49
# Membros: Miguel Pato, fc57102
#          Tomás Correia, fc56372
#          João Figueiredo, fc53524
#--------------------------------------------------------------

# compile everything present in the directory /src
# and put the object files in the directory /obj
# and the executable in the directory /bin

# define the compiler to use
CC = gcc

# define the C compiler flags
CFLAGS = -g

# define the directories containing the source files
SRCDIR = source

# define the directories containing the object files
OBJDIR = object

# define the directories containing the executable
BINDIR = binary

# define the directories containing the header files
INCDIR = include

# define the source files
SRC = $(wildcard $(SRCDIR)/*.c)

# define the object files
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))

# define the executable file
BIN = $(BINDIR)/client

# define the header files
INC = $(wildcard $(INCDIR)/*.h)


# define the rules
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INC)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ $(INCDIR)/*~ $(SRCDIR)/*~ $(BINDIR)/*
	