/* Project10: Client
 * Shivani Kohli and James Anders */
/* Based on code by  DWB */
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 500

struct thread_args {
	int sockfd;
	struct sockaddr_in their_addr;
};

void receive_messages(void * pArgs);
void send_messages(void * pArgs);

int main (int argc, char *argv[])
{
  	int sockfd, myport, nread, their_addr_len;
  	struct sockaddr_in my_addr, their_addr;
  	char buf[BUF_SIZE];
	char server_ip[BUF_SIZE];
	struct thread_args tArgs;

	pthread_t receiveTID, sendTID;

  	if (argc != 4)
    {
      	fprintf (stderr, "Usage: %s <server IP address> <portnum> <name>\n", argv[0]);
      	exit (EXIT_FAILURE);
    }
	
	// Store IP and port parameters
	sprintf(server_ip, argv[1]);
  	myport = atoi (argv[2]);

  	sockfd = socket (AF_INET, SOCK_DGRAM, 0);

  	my_addr.sin_family = AF_INET;
  	my_addr.sin_port = htons (myport);
  	my_addr.sin_addr.s_addr = INADDR_ANY;
  	memset (&(my_addr.sin_zero), '\0', 8);

	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof (struct sockaddr))) {
		close(sockfd);
		fprintf(stderr, "Failed to bind socket\n");
		exit (EXIT_FAILURE);
	}
	else {
		printf("Binded on port %d\n", myport);	
	}

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons (myport);
	inet_aton(server_ip, &their_addr.sin_addr.s_addr);
	memset (&(their_addr.sin_zero), '\0', 8);	

	if (connect(sockfd, (struct sockaddr *) &their_addr, sizeof (struct sockaddr))) {
		close(sockfd);
		fprintf(stderr, "Failed to connect to server on port %d\n", myport);
		exit (EXIT_FAILURE);
	}
	else {
		printf("Connected to server on port %d\n", myport);	
	}

	char host[NI_MAXHOST], service[NI_MAXSERV];
	int result;

	sprintf(buf, "%s", argv[3]);
	sendto (sockfd, buf, BUF_SIZE, 0, (struct sockaddr *) &their_addr, sizeof(struct sockaddr_in));

	tArgs.sockfd = sockfd;
	tArgs.their_addr = their_addr;

	pthread_create(&receiveTID, NULL, receive_messages, (void *)&tArgs);
	pthread_create(&sendTID, NULL, send_messages, (void *)&tArgs);
	pthread_join(receiveTID, NULL);
	pthread_join(sendTID, NULL);
	
  return 0;
}

void receive_messages(void * pArgs) {
	struct thread_args *args = pArgs;
	int nread;
	char buf[BUF_SIZE];

	while (1) {
		nread = recvfrom (args->sockfd, buf, BUF_SIZE, 0, (struct sockaddr *) &args->their_addr, sizeof(struct sockaddr_in));
		printf("%s", buf);
	}
}


void send_messages(void * pArgs) {
	struct thread_args *args = pArgs;
	int nread;
	char buf[BUF_SIZE];
	char bexit[BUF_SIZE];
	char bclose[BUF_SIZE];
	sprintf(bexit,"exit\n");
	sprintf(bclose,"close\n");

	while (1) {
		fgets(buf, BUF_SIZE, stdin);
		if (!strcmp(buf, bclose)) {
			sprintf(buf,"HAS LEFT THE CONVERSATION\n");
			sendto(args->sockfd, buf, BUF_SIZE, 0, (struct sockaddr *) &args->their_addr, sizeof(struct sockaddr_in));
			close(args->sockfd);
			exit(1);
		}

		sendto (args->sockfd, buf, BUF_SIZE, 0, (struct sockaddr *) &args->their_addr, sizeof(struct sockaddr_in));
		
		if (!strcmp(buf, bexit)) {
			close(args->sockfd);
			exit(1);
		}
	}
}

