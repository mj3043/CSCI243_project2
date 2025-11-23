# Makefile for interp project
# uses header.mak for CFLAGS
# @author Munkh-Orgil Jargalsaikhan
include header.mak

PROG = interp
SRCS = interp.c parser.c stack.c tree_node.c symtab.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(PROG)
