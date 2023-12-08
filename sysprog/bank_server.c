#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORTNUM 7500

int main(void) {

	char buf[256];
	struct sockaddr_in ser, cli;
	int sd, nsd;
	int len = sizeof(cli);

	memset((char *) &ser, '\0', sizeof(ser));
	ser.sin_family = AF_INET;
	ser.sin_port = htons(PORTNUM);
	ser.sin_addr.s_addr = inet_addr("192.168.0.1");

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	if(bind(sd, (struct sockaddr *)&ser, sizeof(ser))) {
		perror("bind");
		exit(1);
	}

	if(listen(sd, 5)) {
		perror("listen");
		exit(1);
	}
	
	int num;
	char pwnum[10];

	while(1) {

		if((nsd = accept(sd, (struct sockaddr *)&cli, &len)) == -1) {
			perror("accept");
			exit(1);
		}
		
		srand((unsigned int)time(NULL));
		num = rand();
		itoa(num, pwnum, 10);
		
		sprintf(buf, "%s", inet_ntoa(cli.sin_addr));
		

		if(send(nsd, buf, strlen(buf) + 1, 0) == -1) {
			perror("send");
			exit(1);
		}
		
		if (recv(nsd, buf, sizeof(buf), 0) == -1) {
			perror("recv");
			exit(1);
		}

		if (strcmp(buf, pwnum) == 0) {
			sprintf(buf, "%s", "Valid Security Number"); 
		
			if(send(nsd, buf, sizeof(buf), 0) == -1) {
				perror("recv");
				exit(1);
			}		
		}
		close(nsd);
	}
	close(sd);
	return 0;
}


