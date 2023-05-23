CFLAGS = -Wall -pthread

Load:
	@mkdir -p bin
	gcc $(CFLAGS) -o bin/client src/*.c src/client/*.c src/utils/*.c	
	gcc $(CFLAGS) -o bin/server src/*.c src/server/*.c src/utils/*.c

.PHONY: runclient runserver test clean

runclient: bin/client
	./bin/client

runserver: bin/server
	./bin/server

clean:
	rm -f bin/server
	rm -f bin/client
	@rm -rf bin
