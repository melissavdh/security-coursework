CC = gcc -Wall -g

client: cli_app.c cli_fragment.o cli_reliable.o cli_security.o cli_socket.o sec_lib.so
	$(CC) cli_app.c cli_fragment.o cli_reliable.o cli_security.o cli_socket.o sec_lib.so -o client

cli_fragment.o: cli_fragment.c
	$(CC) -c cli_fragment.c -o cli_fragment.o

cli_reliable.o: cli_reliable.c
	$(CC) -c cli_reliable.c -o cli_reliable.o

cli_security.o: cli_security.c
	$(CC) -c cli_security.c -o cli_security.o

cli_socket.o: cli_socket.c
	$(CC) -c cli_socket.c -o cli_socket.o

clean:
	rm -f *.o client core

submission:
	tar -cvf submission.tar *.c *.h *.so Makefile
