CC=gcc
CFLAGS=-Wall
INCLUDE=-L/usr/lib/
INSTALL_PATH=/usr/bin
APP_NAME=knx-logger

knx-logger: knx-logger.o csvparser.o
	$(CC) -o $(APP_NAME) knx-logger.o csvparser.o -leibclient $(INCLUDE)

csvparser.o: csvparser.c csvparser.h
	$(CC) $(CFLAGS) -c csvparser.c csvparser.h

knx-logger.o: knx-logger.c
	$(CC) $(CFLAGS) -c knx-logger.c

install:
	cp $(APP_NAME) $(INSTALL_PATH)

clean:
	rm *.o knx-logger
