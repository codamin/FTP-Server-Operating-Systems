all : client

server : main.o client.o tools.o
		gcc main.o client.o tools.o -o client

main.o : main.c client.h
	gcc -c main.c

server.o : client.c client.h
	gcc -c server.c

tools.o: ../tools/tools.c
	gcc -c ../tools/tools.c

clean:
	rm *.o client
