all : server

server : main.o server.o tools.o
		gcc main.o server.o tools.o -o server

main.o : main.c server.h
	gcc -c main.c

server.o : server.c server.h
	gcc -c server.c

tools.o: ../tools/tools.c
	gcc -c ../tools/tools.c

clean:
	rm *.o server