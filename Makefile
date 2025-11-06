#--------------Directories---------------------------
OBJDIR = obj
BINDIR = bin
INCLUDEDIR = includes

#--------------Compiler options----------------------
CC = gcc
CFLAGS =  -Wall -g -I$(INCLUDEDIR) -MMD -MP
CLIENT_LIBS = -lsqlcipher

#----------------Source Files----------------------------
CLIENT_SRCS = src/client_main.c src/account.c src/db_access.c src/util.c


#----------------Object Files-----------------------------
CLIIENT_OBJS = $(CLIENT_SRCS:src/%.c=$(OBJDIR)/%.o)


#------------------Final Binaries--------------------------
CLIENT_BIN = cman

#----------------Dependencies------------------------------
CLIENT_DEP = $(CLIIENT_OBJS:%.o=%.d)

#----------------------Make targets-------------------------
all: $(CLIENT_BIN)

$(CLIENT_BIN): $(CLIIENT_OBJS) | $(BINDIR)
	@echo "Compiling client..........."
	$(CC) $(CLIIENT_OBJS) -o $(BINDIR)/$(CLIENT_BIN) $(CLIENT_LIBS)

$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	@echo "Removing binaries.............."
	@rm -rf $(OBJDIR) $(BINDIR)


#--------------Directories------------------
$(OBJDIR):
	@mkdir $(OBJDIR)

$(BINDIR):
	@mkdir $(BINDIR)


#----------------Directives------------------
#---they help to give extra dependencie(header files) for the binary objects
-include $(CLIENT_DEP)


