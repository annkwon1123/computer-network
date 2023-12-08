#include <stdio.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define SOCKET_NAME "user_socket"
#define PORTNUM 7500
#define MAX_PASSWD 100

int main(void) {

	int sd, len;
	char buf[256];
	struct sockaddr_in cli;

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	memset((char *) &cli, '\0', sizeof(cli));
	cli.sin_family = AF_INET;
	cli.sin_port = htons(PORTNUM);
	cli.sin_addr.s_addr = inet_addr("192.168.142.129");

	// strcpy(cli.sin_path, SOCKET_NAME);
	// len = sizeof(cli.sin_family) + strlen(cli.sin_path);

	if(connect(sd, (struct sockaddr *)&cli, len) < 0) {
		perror("connect");
		exit(1);
	}
	
	if(recv(sd, buf, sizeof(buf), 0 ) == -1) {
		perror("recv");
		exit(1);
	}
	
	char passwd_read[MAX_PASSWD];
	fgets(passwd_read, MAX_PASSWD, stdin);
	strcpy(buf, passwd_read);
	
	if(send(sd, buf, sizeof(buf), 0) == -1) {
		perror("send");
		exit(1);
	}

	close(sd);
	return 0;
}
