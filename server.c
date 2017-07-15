/* Project10: Server
 * Shivani Kohli and James Anders */
/* Based on code by DWB */
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
#define MAX_CLIENTS 5

struct chat_client {
	int ID;
	char name[BUF_SIZE];
	struct sockaddr_in address;	
};

FILE *fp;

struct chat_client connected_clients[MAX_CLIENTS];

int main (int argc, char *argv[])
{
  	int sockfd, myport, nread, their_addr_len;
  	struct sockaddr_in my_addr, their_addr;
  	char buf[BUF_SIZE];

  	if (argc != 2)
    {
      	fprintf (stderr, "Usage: %s portnum\n", argv[0]);
      	exit (EXIT_FAILURE);
    }

  	myport = atoi (argv[1]);

  	sockfd = socket (AF_INET, SOCK_DGRAM, 0);

  	my_addr.sin_family = AF_INET;
  	my_addr.sin_port = htons (myport);
  	my_addr.sin_addr.s_addr = INADDR_ANY;
  	memset (&(my_addr.sin_zero), '\0', 8);

  	if (bind (sockfd, (struct sockaddr *) &my_addr, sizeof (struct sockaddr)))
    {
      	close (sockfd);
      	fprintf (stderr, "Failed to bind socket!\n");
      	exit (EXIT_FAILURE);
    }
  	else
    {
      	printf ("Server listening on port %d\n", myport);
    }

	fp = fopen("./chat_history.txt", "w+");
	

	char host[NI_MAXHOST], service[NI_MAXSERV];
	int result;
	int active_clients;
	active_clients = 0;

	while (1) {
  		their_addr_len = sizeof (struct sockaddr_in);
  		nread = recvfrom (sockfd, buf, BUF_SIZE, 0,
		    (struct sockaddr *) &their_addr, &their_addr_len);
  
		printf("buf: %s\n", buf);

		char bexit[BUF_SIZE];
		sprintf(bexit,"exit\n");

		if (!strcmp(buf, bexit)) {
			fclose(fp);
			close(sockfd);
			exit(1);
		}		

  		result = getnameinfo ((struct sockaddr *) &their_addr, their_addr_len,
			host, NI_MAXHOST, service, NI_MAXSERV,
			NI_NUMERICSERV);

		printf("addr: %lu\n", their_addr.sin_addr.s_addr);		

		int i;
		char sendername[BUF_SIZE];
		int senderID;
		struct chat_client client;
		int new_client;
		new_client = 1;
		for (i = 0; i < MAX_CLIENTS; i++) {
			client = connected_clients[i];
			if (client.address.sin_addr.s_addr == their_addr.sin_addr.s_addr) {
				// client exists in array
				new_client = 0;
				sprintf(sendername,"%s",client.name);
				senderID = client.ID;
				break;
			}
		}

		struct chat_client newguy;

		if (new_client) {
			// Add to array
			if (active_clients > 5) {
      			fprintf (stderr, "Too many clients\n");
			}
			else {
				newguy.address = their_addr;
				newguy.ID = active_clients;
				sprintf(newguy.name, "%s", buf);
				connected_clients[active_clients] = newguy;	
				active_clients++;		
				sprintf(sendername,"%s",newguy.name);
			}
		}

		// Broadcast message
		for (i = 0; i < active_clients; i++) {
			client = connected_clients[i];			
			char msg[BUF_SIZE];
			
			if (new_client) {
				sprintf(msg, "%s has entered ze chat room\n", buf);
				fprintf(fp, "%s has entered ze chat room\n", buf);
				if (sendto (sockfd, msg, BUF_SIZE, 0, (struct sockaddr *) &client.address,
	      			sizeof (struct sockaddr_in)) != nread)
    			{
      				fprintf (stderr, "Error sending response\n");
    			}	
			}
			else {
				if (client.ID != senderID) {
					sprintf(msg, "%s:%s", sendername, buf);
					fprintf(fp, "%s:%s", sendername, buf);
					if (sendto (sockfd, msg, BUF_SIZE, 0, (struct sockaddr *) &client.address,
		      			sizeof (struct sockaddr_in)) != nread)
	    			{
	      				fprintf (stderr, "Error sending response\n");
	    			}
				}			
			}		
		}		

  		if (result == 0)
    		printf ("Received %d bytes from %s:%s\n", (long) nread, host, service);
  		else
    		fprintf (stderr, "getnameinfo: %s\n", gai_strerror (result));
	}
  close(fp);
  close (sockfd);
  return 0;
}
