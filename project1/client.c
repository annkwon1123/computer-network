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

	if((sockfd = socket (PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons (PORT);
	their_addr.sin_addr.s_addr = htonl (INADDR_ANY);

	if (connect (sockfd, (struct sockaddr*)&their_addr, sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}
	
	char message[MSG_SIZE];
	snprintf(message, MSG_SIZE, "GET / HTTP/1.1\r\n"
				"Host: %s:%d\r\n"
				"Connection: close\r\n"
				"\r\n", inet_ntoa(their_addr.sin_addr), PORT);
	
	if (send (sockfd, message, strlen(message), 0) == -1) {
		perror("send");
		exit(1);
	}
}
