all: echoclient echoserver

echoclient: echoclient.c
	gcc -pthread -o echoclient echoclient.c

echoserver: echoserver.c
	gcc -pthread -o echoserver echoserver.c

clean:
	rm echoclient echoserver

