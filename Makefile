CC = gcc
CFLAGS =  -Wall -g -Iincludes
LIBS = 

SRCS = src/cred_manager.c src/secure.c src/account.c
OBJS = $(SRCS:src/%.c=obj/%.o)
INCLUDES = includes/account.h includes/main.h includes/secure.h
OBJDIR = obj

OUT = cman

all: $(OUT)

$(OUT): $(OBJS) $(INCLUDES)
	$(CC) $(OBJS) -o $(OUT) $(LIBS)

obj/%.o : src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	@mkdir $(OBJDIR)
