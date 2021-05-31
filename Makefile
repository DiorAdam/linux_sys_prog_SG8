CC = gcc
SRC = src
OUTPUT = dist

build: $(SRC)/client.c $(SRC)/server.c
	if [ ! -d $(OUTPUT) ]; then mkdir $(OUTPUT); fi
	$(CC) $(SRC)/client.c -o $(OUTPUT)/client
	$(CC) -pthread $(SRC)/server.c -o $(OUTPUT)/server

clean:
	if [ -d $(OUTPUT) ]; then rm -rf $(OUTPUT); fi
