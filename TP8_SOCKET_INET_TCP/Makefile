SRCDIR = ./src
INCDIR = ./include
OBJDIR = ./obj
BINDIR = ./bin
LOGDIR = ./logs

vpath %.c $(SRCDIR)
vpath %.h $(INCDIR)

CC = gcc
CFLAGS = -std=c18 -I$(INCDIR) -Wall -Wpedantic -Wextra -Wconversion -Wwrite-strings -Werror -fstack-protector-all -fpie -D_POSIX_C_SOURCE=700 -MMD -MP
LDFLAGS =

_OBJ = client.o server.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
DEP = $(OBJ:.o=.d) 
BIN = $(BINDIR)/server $(BINDIR)/client

-include $(DEP)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BINDIR)/%: $(OBJDIR)/%.o
	@mkdir -p $(BINDIR)
	$(CC) -o $@ $^ $(LDFLAGS)

all: $(BIN)

.PHONY: clean cleanlogs

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.d *~ core $(INCDIR)/*~
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)

cleanlogs:
	rm -rf $(LOGDIR)/*