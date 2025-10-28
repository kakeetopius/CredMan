#--------------Directories---------------------------
OBJDIR = obj
BINDIR = bin
INCLUDEDIR = includes

#--------------Compiler options----------------------
CC = gcc
CFLAGS =  -Wall -g -I$(INCLUDEDIR) -MMD -MP
CLIENT_LIBS = -lsqlcipher
SERVER_LIBS = 

#----------------Source Files----------------------------
CLIENT_SRCS = src/client_main.c src/account.c src/db_access.c
SERVER_SRCS = 


#----------------Object Files-----------------------------
CLIIENT_OBJS = $(CLIENT_SRCS:src/%.c=$(OBJDIR)/%.o)
SERVER_OBJS  = $(SERVER_SRCS:src/%.c=$(OBJDIR)/%.o) 


#------------------Final Binaries--------------------------
CLIENT_BIN = cman
SERVER_BIN = cmand


#----------------Dependencies------------------------------
CLIENT_DEP = $(CLIIENT_OBJS:%.o=%.d)
SERVER_DEP = $(SERVER_OBJS:%.o=%.d)


#----------------------Make targets-------------------------
all: $(CLIENT_BIN) $(SERVER_BIN)


$(CLIENT_BIN): $(CLIIENT_OBJS) | $(BINDIR)
	@echo "Compiling client..........."
	$(CC) $(CLIIENT_OBJS) -o $(BINDIR)/$(CLIENT_BIN) $(CLIENT_LIBS)

$(SERVER_BIN): $(SERVER_OBJS) | $(BINDIR)
	@echo "Compiling server............"
	$(CC) $(SERVER_OBJS) -o $(BINDIR)/$(SERVER_BIN) $(SERVER_LIBS)

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
-include $(SERVER_DEP)
-include $(CLIENT_DEP)

# #---------------Target for individual binary files (for testing a module)-------------
# %: src/%.c db_structures.o | $(BINDIR)
# 	$(CC) $(CFLAGS) $< -o $(BINDIR)/$@ $(SERVER_LIBS)

