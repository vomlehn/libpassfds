# Makefile for passfds

BUILD_DIR	= .
INSTALL_DIR	= .

INSTALL_LIB	= $(INSTALL_DIR)/lib
INSTALL_INC	= $(INSTALL_DIR)/include

CFLAGS	+= -g -ggdb
CFLAGS	+= -Werror -Wall -Wextra -Wno-unused-parameter
CFLAGS	+= -I.
CFLAGS	+= -fPIC

LDFLAGS	+= -L. -lpassfds

libpassfds.so: sendfds.o recvfds.o
	$(LD) -shared -o $@ $^
	
sendfds.o: sendfds.c passfds.h
	$(CC) $(CFLAGS) -c $(filter-out %.h,$^)

recvfds.o: recvfds.c passfds.h
	$(CC) $(CFLAGS) -c $(filter-out %.h,$^)

test-passfds.o: test-passfds.c passfds.h
	$(CC) $(CFLAGS) -c $(filter-out %.h,$^)

test-passfds: test-passfds.o libpassfds.so
	$(CC) -o $@ $(filter-out %.so,$^) $(LDFLAGS)

test:	test-passfds
	LD_LIBRARY_PATH=. ./$^

$(INSTALL_LIB)/libpassfds.so: libpassfds.so
	install -m 0644 -D $^ $@

$(INSTALL_INC)/passfds.h: passfds.h
	install -m 0644 -D $^ $@

install: $(INSTALL_LIB)/libpassfds.so \
	$(INSTALL_INC)/passfds.h

clean:
	rm -f test-passfds test-passfds.o libpassfds.so sendfds.o recvfds.o

.PHONE: clean test install
