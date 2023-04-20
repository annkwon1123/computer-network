#include <stdio.h> /* basic C header */
#include <stdlib.h> /* has macros sucha as EXIT_FAILURE */
#include <errno.h> 
#include <string.h> /* header to help with strings */
#include <sys/types.h> /* provides required data types */
#include <netinet/in.h> /* has the sockaddr_in structure */
#include <sys/socket.h> /* holds address familt and socket functions */ 
#include <sys/wait.h> 
#include <arpa/inet.h>

#define PORT 6560 /* port through which connection is to be madea */
#define BACKLOG 10 /* how many pendinf connections queue will hold */
#define MSG_SIZE 1024

int main() {
	int sockfd, new_fd; /* listen on soch_fd, new connection on new_fd */
	struct sockaddr_in my_addr; /* my address */
	struct sockaddr_in their_addr; /* connector address */
	int sin_size;

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT); /* short, network byte order */
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY allowws clients to connect to any one of the host's IP address */

	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	while(1) { /* main accept() loop */
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size)) == -1) {
			perror("accept");
			continue;
		}

		printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));
		
		char request[MSG_SIZE];
		int num_bytes = 0;
		while((num_bytes = recv(new_fd, request, MSG_SIZE-1, 0)) > 0) {
			request[num_bytes] = '\0';
			printf("%s", request);
		}
	}
}


