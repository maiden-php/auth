### THIS IS JUST A TEMPLATE: HAVE TO CREATE YOUR OWN MAKEFILE for this assignment

# This illustrates how you can set the value of a defined constant 
# on the compile line.
PORT=30001
CFLAGS = -DPORT=$(PORT) -g -Wall -std=gnu99

all: rcopy_client rcopy_server

# The variable $@ has the value of the target. 
rcopy_server: rcopy_server.o ftree_server.o hash_functions.o
	gcc ${CFLAGS} -o $@ ftree_server.o rcopy_server.o hash_functions.o

rcopy_client: rcopy_client.o ftree_client.o hash_functions.o
	gcc ${CFLAGS} -o $@ ftree_client.o rcopy_client.o hash_functions.o

.c.o:
	gcc  ${CFLAGS}  -c $<

clean:
	rm *.o rcopy_client rcopy_server
