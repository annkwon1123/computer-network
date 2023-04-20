int socket (int domain, int type, int protocol); //Returns file descriptor or -1
int bind (int sockfd, struct sockaddr* myaddr, int addrlen);
int listen (int sockfd, int backlog);
int accept (int sockfd, struct sockaddr* cliaddr, int* addrlen); //Returns file descriptor or -1
