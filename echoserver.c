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
#include <arpa/inet.h>

#define BUFSIZE 2048
#define TIMEOUT 60
#define CLIENT_NUM 5


typedef struct sock_info
{
	int sock;
	struct sockaddr_in addr;
}sock_info;

int scan_request(char *rcv, int len);
void *communicate(void *client_socket_information);

int client_sock[CLIENT_NUM];
int broad_echo = 0;


/***************************main*******************************/
int main(int argc, char **argv) {
	int origin;
	int portnum, option_val = 0;
	int n, client_len;
	double real_time;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	struct hostent *client;
	char *client_ip_addr;
	sock_info client_socket_info;


	/* input : port */
	if (argc == 2)
	{
		broad_echo = 0;
		portnum = atoi(argv[1]);
	}
	else if (argc == 3)
	{
		if(memcmp("-b", argv[2], 2) == 0)
		{
			broad_echo = 1;
			portnum = atoi(argv[1]);
		}
		else
		{
			fprintf(stderr, "SYNTAX : echoserver <port> [-b]\n");
			return -1;
		}
	}
	else
	{
		printf("You have to exec %s as\n%s <port>\n\n", argv[0], argv[0]);
		return -1;
	}



	/* creating the socket - origin : 연결요청에 대해 새로운 소켓 생성 */
	origin = socket(AF_INET, SOCK_STREAM, 0);
	if (origin < 0)
	{
		perror("ERROR : Origin Socket is not well created!\n\n");
		return -1;
	}
	option_val = 1;
	setsockopt(origin, SOL_SOCKET, SO_REUSEADDR,
		(const void *)&option_val, sizeof(int));


	/************** server's address setting **************/
	memset((char *)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portnum);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/************** binding **************/
	if (bind(origin, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("ERROR : Origin Binding Failured!\n\n");
		return -1;
	}

	/************** listen for conneciton **************/
	if (listen(origin, 5) < 0)
	{
		perror("ERROR : Origin Listening Failured!\n\n");
		return -1;
	}

	/************** accept **************/
	client_len = sizeof(client_addr);
	int client_number = 0;
	while (client_number < CLIENT_NUM)
	{
		/* accept */
		client_sock[client_number] = accept(origin, (struct sockaddr *)&client_addr, &client_len);
		if (client_sock[client_number] < 0)
		{
			perror("ERROR : Accepting Error!\n\n");
			return -1;
		}

		pthread_t thread_id;

		client_socket_info.sock = client_sock[client_number];
		client_socket_info.addr = client_addr;

		if (pthread_create(&thread_id, NULL, communicate, (void *)&client_socket_info) < 0)
		{
			fprintf(stderr, "ERROR : cannot create PTHREAD!\n\n");
			return -1;
		}

		pthread_detach(thread_id);

		client_number++;

	} // while(1) : accept

	return 0;
}



void *communicate(void *client_socket_information)
{
	int client_sockk = (*(sock_info *)client_socket_information).sock;
	int n, time = 0, client_len;
	double real_time, result_time;
	struct sockaddr_in client_addr = (*(sock_info *)client_socket_information).addr;
	char *client_ip_addr;
	char rcv[BUFSIZE];
	char send[BUFSIZE];
	int keep_alive_val = 0, content_len = 0;
	char style_css[BUFSIZE];
	char content_length[32];
	int response_code;

	client_ip_addr = inet_ntoa(client_addr.sin_addr);	// a.b.c.d 형식으로 변환
	if (client_ip_addr < 0)
	{
		fprintf(stderr, "ERROR : inet_ntoa error!\n\n");
		return;
	}
	printf("Connection from : %s\nAccepted!\n----------------------------------\n", client_ip_addr);


	while(1)
	{
		/********************* request from client *********************/
		memset(rcv, 0, BUFSIZE);
		n = read(client_sockk, rcv, BUFSIZE);
		if (n < 0)
		{
			perror("ERROR : Read from client Error!\n\n");
			return;
		}
		else if (n > 0)
		{
			printf("From %s :\n\n%s\n", client_ip_addr, rcv);
			printf("--------------------------------------------------------\n");
			printf("--------------------------------------------------------\n");

			/********************* echo *********************/
			memset(send, 0, BUFSIZE);
			strcpy(send, rcv);
			/* -b option off */
			if(broad_echo == 0)
			{
				n = write(client_sockk, send, strlen(send));
				if (n < 0)
				{
					fprintf(stderr, "ERROR : Failured to echo\n\n");
					return;
				}
			}
			/* -b option on */
			else if(broad_echo == 1)
			{
				for(int num=0; (client_sock[num]>0 && num < CLIENT_NUM); num++)
				{
					n = write(client_sock[num], send, strlen(send));
					if (n < 0)
					{
						fprintf(stderr, "ERROR : Failured to echo\n\n");
						return;
					}
				}
			}
		} // read data exists
	} // while(1)
	return;
}




