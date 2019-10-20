all : client

client : main.o client.o tools.o
		gcc main.o client.o tools.o -o client

main.o : main.c client.h
	gcc -c main.c

client.o : client.c client.h
	gcc -c client.c

tools.o: ../tools/tools.c
	gcc -c ../tools/tools.c

clean:
	rm *.o client