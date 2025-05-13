CC = gcc
CFLAGS =  -Wall -g -Iincludes
LIBS = -lcrypto 

SRCS = src/cred_manager.c src/secure.c
OBJS = $(SRCS:src/%.c=obj/%.o)

OBJDIR = obj

OUT = cman

all: $(OUT)

$(OUT): $(OBJS)
	$(CC) $(OBJS) -o $(OUT) $(LIBS)

obj/%.o : src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	@mkdir $(OBJDIR)
