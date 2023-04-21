CFLAGS = -Wall -pthread

Load:
	gcc $(CFLAGS) -o bin/client src/*.c src/client/*.c	

.PHONY: runclient runserver test clean

runclient: bin/client
	./bin/client

runserver: bin/server
	./bin/server

client:
	rm -f bin/server
	rm -f bin/client
