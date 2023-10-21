#CC specifying the compiler. Please use gcc or clang.
CC=gcc 

#CFLAGS specifying the arguments for compilation. Use at least the following: -Wall -Werror -pedantic -std=gnu18
CFLAGS=-Wall -Werror -pedantic -std=gnu18

#LOGIN specifying you login
LOGIN=pli

#SUBMITPATH specifying the path where you handin your files.
SUBMITPATH=~cs537-1/handin/$(LOGIN)/P3

#all is the first target and .PHONY, only runs wsh
all: wsh

#wsh is a target which depends on wsh.c and wsh.h and builds the wsh
wsh: wsh.c wsh.h
	$(CC) -o $@ wsh.c $(CFLAGS)

#run depends on wsh and executes wsh binary
run: wsh
	./wsh

#pack creates archive called login.tar.gz. The archive should contain wsh.h, wsh.c Makefile, and a README.md.
pack:
	tar -czf $(LOGIN).tar.gz wsh.h wsh.c Makefile README.md

#submit depends on pack target and copies login.tar.gz to the SUBMITPATH directory.
submit: pack
	cp $(LOGIN).tar.gz $(SUBMITPATH)

#my own function
remote_submit: pack
	scp $(LOGIN).tar.gz $(LOGIN)@best-linux.cs.wisc.edu:/home/$(LOGIN)/CS537/p3-shell
#my own function 
clean:
	rm -f wsh *.o *.tar.gz

#.PHONY objects
.PHONY: all 
