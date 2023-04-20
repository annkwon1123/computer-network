/*
    생성된 소켓 lsock(sd)에 주소 할당
    return bind() 값
*/
int bind_lsock(int lsock, int port) {
    struct sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(port);

    return bind(lsock, (struct sockaddr *)&sin, sizeof(sin));
}

int main(int argc, char **argv) {
    int port, pid;
    int lsock, asock;

    struct sockaddr_in remote_sin;
    socklen_t remote_sin_len;

    if (argc < 2) {
        printf("Usage: %s {port}\n",argv[0]);
        exit(0);
    }

    port = atoi(argv[1]);
    printf("[INFO] The server will listen to port: %d.\n", port);

    lsock = socket(AF_INET, SOCK_STREAM, 0);
    if (lsock < 0) {
        perror("[ERR] failed to create lsock.\n");
        exit(1);
    }

    if (bind_lsock(lsock, port) < 0) {
        perror("[ERR] failed to bind lsock.\n");
        exit(1);
    }
 
    printf("bind() success\n"); // 바인드 성공

    if (listen(lsock, 10) < 0) {
        perror("[ERR] failed to listen lsock.\n");
        exit(1);
    }

    printf("socket() success\n"); // 소켓 성공

    signal(SIGCHLD, SIG_IGN);

    while (1) {
        asock = accept(lsock, (struct sockaddr *)&remote_sin, &remote_sin_len);
        if (asock < 0) {
            perror("[ERR] failed to accept.\n");
            continue;
        }

        pid = fork(); // 멀티프로세스 생성 -> fork() 사용
        if (pid == 0) { 
            close(lsock); 
            http_handler(asock); 
            close(asock);
            exit(0);
    }

    if (pid != 0)    { close(asock); }
    if (pid < 0)    { perror("[ERR] failed to fork.\n"); }
    }
}