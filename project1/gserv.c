#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 6560
#define BACKLOG 10
#define MSG_SIZE 1024

int main() {
    int sock_fd, new_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    int sin_size;

    if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[ERR] socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("[ERR] bind");
        exit(1);
    }

    if (listen(sock_fd, BACKLOG) == -1) {
        perror("[ERR] listen");
        exit(1);
    }

    while(1) {
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sock_fd, (struct sockaddr *) &their_addr, &sin_size)) == -1) {
            perror("[ERR] accept");
            continue;
        }
        
        char request[MSG_SIZE];
        if(read(new_fd, request, MSG_SIZE) == -1) {
            perror("[ERR] read");
            exit(1);
        }

        int fd = 0;
        if((fd = open("/html.html", O_RDONLY)) == -1) {
            perror("[ERR] open");
            exit(1);
        }

        int num_bytes = 0;
        while((num_bytes = read(fd, request, MSG_SIZE)) > 0) {
            if(write(new_fd, request, num_bytes) == -1) {
                perror("[ERR] write");
                exit(1);
            }
        }
        close(fd);
        close(new_fd);
    }
}
