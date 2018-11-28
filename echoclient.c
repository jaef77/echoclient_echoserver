#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define BUFSIZE 8192
#define TIMEOUT 120


typedef struct sock_info
{
	int sock;
	struct sockaddr_in addr;
}sock_info;

void *receive_tcp(void *server_socket_information);

int main(int argc, char **argv) {
	int sock, portnum;
	int n, time = 0;
	double real_time, result_time;
	struct sockaddr_in server_addr;
	struct hostent *server;
	char *hostname;
	char buf[BUFSIZE];
	sock_info server_socket_info;


	/* input : domain or address */
	if (argc != 3)
	{
		fprintf(stderr, "SYNTAX : echoclinet <host> <port>\n");
		return -1;
	}
	hostname = argv[1];
	portnum = atoi(argv[2]);

	/* creating the socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);	//IPv4
	if (sock < 0)
	{
		perror("ERROR : Socket is not well created!\n");
		return -1;
	}

	/* DNS */
	server = gethostbyname(hostname);
	if (server == NULL)
	{
		fprintf(stderr, "ERROR : There is no such a domain as %s\n", hostname);
		return -1;
	}

	memset((char *)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
	server_addr.sin_port = htons(portnum);

	/* connection */
	if (connect(sock, &server_addr, sizeof(server_addr)) < 0)
	{
		perror("ERROR : Connection Error\n\n");
		return -1;
	}
	printf("Connetion Success!\n");


	/* Send & Response */
	pthread_t thread_id_send, thread_id_rcv;
	server_socket_info.sock = sock;
	server_socket_info.addr = server_addr;

	if(pthread_create(&thread_id_rcv, NULL, receive_tcp, (void *)&server_socket_info) < 0)
	{
		fprintf(stderr, "ERROR : cannot create PTHREAD of receive!\n\n");
		return -1;
	}

	while(1)
	{
		/* request */
		memset(buf, 0, BUFSIZE);
		printf("\rMessage to send :");
		fgets(buf, BUFSIZE, stdin);
		n = write(sock, buf, strlen(buf));
		if (n < 0)
		{
			perror("ERROR : write function to socket");
			return -1;
		}
		printf("\n");
		sleep(1);
	}

	return 0;
} // main

void *receive_tcp(void *server_socket_information)
{
	int sock = (*(sock_info *)server_socket_information).sock;
	int n, time = 0;
	double real_time, result_time;
	struct sockaddr_in server_addr = (*(sock_info *)server_socket_information).addr;
	char buf[BUFSIZE];

	while(1)
	{
		memset(buf, 0, BUFSIZE);
		n = read(sock, buf, BUFSIZE);
		if (n < 0)
		{
			perror("ERROR : write function to socket");
			return -1;
		}
		else if (n > 0)
		{
			printf("\nResponse from server : ");
			for(int zz=0;zz<n;zz++)
				printf("%c", buf[zz]);
			printf("\rMESSAGE TO SEND : ");
		}
	} 
} // rcv_tcp