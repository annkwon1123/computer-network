#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define NOT_FOUND_CONTENT       "<h1>404 Not Found</h1>\n"
#define SERVER_ERROR_CONTENT    "<h1>500 Internal Server Error</h1>\n"
#define BUF_SIZE 1024
#define HEADER_FMT "HTTP/1.1 %d %s\nContent-Length: %ld\nContent-Type: %s\n\n"


/* server socket에 port를 bind하는 함수
    주어진 port로 server socket을 bind하고 성공하면 0을 반환*/
int bind_listen_sock(int listen_sock, int port) {
    struct sockaddr_in sin;  /* 4byte의 IP주소가 저장되는 구조체 */

    sin.sin_family = AF_INET;   /* 인터넷 주소 체계로 지정 */
    sin.sin_addr.s_addr = htonl(INADDR_ANY);    /* INADDR_ANY는 host byte 순서에서 network byte 순서로 반환 */
    sin.sin_port = htons(port); /* 2byte의 portnum */

    return bind(listen_sock, (struct sockaddr *)&sin, sizeof(sin));
}

/* http 프로토콜 규격에 맞춰져 클라이언트로 보내지는 데이터의 정보를 담도록하는 header 채우기 */
void fill_header(char *header, int status, long len, char *type) {
    char status_text[50];
    switch (status) {
        case 200:   /* 요청 성공 처리 */
            strcpy(status_text, "OK"); break;
        case 404:   /* 요청한 resource 찾을 수 없음 */
            strcpy(status_text, "Not Found"); break;
        case 500:   /* 서버에서 처리 중에 문제 발생 */
        default:
            strcpy(status_text, "Internal Server Error"); break;
    }
    sprintf(header, HEADER_FMT, status, status_text, len, type);
}

/* 읽어오는 html파일을 처리하기 위한 함수이다. 파일 확장자를 추출하여 MIME 타입을 결정하고 
*  filetype 버퍼에 저장하여 웹 브라우저에게 어떤 종류의 파일인지 알려주기 위해서 필요하다. */
void get_file(char *filetype, char *uri) {
    char *filename = strrchr(uri, '.');
    if (!strcmp(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (!strcmp(filename, ".jpg") || !strcmp(filename, ".jpeg"))
        strcpy(filetype, "image/jpeg");
    else if (!strcmp(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (!strcmp(filename, "png"))
	strcpy(filetype, "image/png");
    else if (!strcmp(filename, ".mp3"))
	strcpy(filetype, "audio/mp3");
    else if(!strcmp(filename, ".pdf"))
        strcpy(filetype, "application/pdf");
    else strcpy(filetype, "text/plain");

}

/* 404에러 처리 */
void handle_404(int accept_sock) {
    char header[BUF_SIZE];
    fill_header(header, 404, sizeof(NOT_FOUND_CONTENT), "text/html");

    write(accept_sock, header, strlen(header));
    write(accept_sock, NOT_FOUND_CONTENT, sizeof(NOT_FOUND_CONTENT));
}

/* 500에러 처리*/
void handle_500(int accept_sock) {
    char header[BUF_SIZE];
    fill_header(header, 500, sizeof(SERVER_ERROR_CONTENT), "text/html");

    write(accept_sock, header, strlen(header));
    write(accept_sock, SERVER_ERROR_CONTENT, sizeof(SERVER_ERROR_CONTENT));
}

/* 클라이언트의 요청을 본격적으로 처리하기 위한 함수이다.
*  request message를 파싱하고 적절한 작업을 수행하고, request의 header 생성 후,
*  요청에 대한 실제 데이터를 전송하는 과정을 실행한다. */
void http_handler(int accept_sock) {
    char header[BUF_SIZE];
    char buf[BUF_SIZE];

    if (read(accept_sock, buf, BUF_SIZE) < 0) {
        perror("Failed to read request.\n");
	    handle_500(accept_sock); return;
    }
    printf("%s\n", buf);

    char *method = strtok(buf, " ");
    char *uri = strtok(NULL, " ");
    if (method == NULL || uri == NULL) {
        perror("Failed to identify method, URI.\n");
        handle_500(accept_sock); return;
    }

    printf("Handling Request: method=%s, URI=%s\n", method, uri);
    
    char safe_uri[BUF_SIZE];
    char *local_uri;
    struct stat st;

    strcpy(safe_uri, uri);
    if (!strcmp(safe_uri, "/")) strcpy(safe_uri, "/noot.html");
    
    local_uri = safe_uri + 1;
    if (stat(local_uri, &st) < 0) {
        perror("No file found matching URI.\n");
        handle_404(accept_sock); return;
    }

    int fd = open(local_uri, O_RDONLY); /* local_uri 경로의 파일을 열고 읽어오기 위한 fd 생성 */
    if (fd < 0) {
        perror("Failed to open file.\n");
        handle_500(accept_sock); return;
    }

    int file_len = st.st_size;
    char filetype[40];
    get_file(filetype, local_uri);
    fill_header(header, 200, file_len, filetype);   /* html 파일을 읽어서 해당 파일의 내용을 웹페이지에 띄우기 위해 사용 */
    write(accept_sock, header, strlen(header)); /* 파일 내용 전송 이전에 먼저 보내야하는 http 응답 메세지 header 정보를 담아서 클라이언트에게 전송. */

    int cnt;
    while ((cnt = read(fd, buf, BUF_SIZE)) > 0)
        write(accept_sock, buf, cnt);
   
}


/* socket 사용 절차는 다음과 같다.
 * socket생성, bind, listen, accept(클라이언트로 부터 connect요청 받음), 데이터 송수신
 */
int main(int cmdc, char **cmdv) {
	int port, pid;
    int listen_sock, accept_sock;

    struct sockaddr_in myaddr;
    socklen_t myaddr_len;

    /* 길이가 2보다 작으면 portnum이 입력되지 않은 것임 */
   	if (cmdc < 2) {
       		printf("Usage: \n");
        	printf("\t%s {port}: runs mini HTTP server.\n", cmdv[0]);
        	exit(0);
    	}

    /* 입력받은 portnum을 정수형으로 변환하여 저장 */
    port = atoi(cmdv[1]);
	printf("The server will listen to port: %d.\n", port);

    /* socket 생성 후 연결 요청을 수신할 fd를 listen_sock에 저장
    *  주소 체계 AF_INET으로 지정, TCP프로토콜 사용, 그 외 기본 지정 */
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock < 0){
		perror("Failed to create.\n");
		exit(1);
	}
	/* 생성한 listen_sock에 portnum bind */
	if(bind_listen_sock(listen_sock, port) < 0){
		perror("Failed to bind.\n");
		exit(1);
	}

    /* socket 수신 대기 모드로 전환 */
	if(listen(listen_sock, 10) < 0){
		perror("Failed to listen.\n");
		exit(1);
	}

    /* 
    *  각각의 클라이언트 요청에 대한 처리를 자식 프로세스로 fork하여 처리하기 때문에
    *  종료된 후 부모 프로세스가 무시하도록 설정하여 좀비를 되는 것을 방지한다. 
    */
	signal(SIGCHLD, SIG_IGN);

    while (1) {
        printf("waiting...\n");
        /* 클라이언트의 connect 요청을 받으면 accept를 통해서 연결을 설정
        *  성공하면 새로운 socket을 return하고 이를 통해 데이터를 송수신한다. */
        accept_sock = accept(listen_sock, (struct sockaddr *)&myaddr, &myaddr_len); 
        if (accept_sock < 0) {
            perror("Failed to accept.\n");
            continue;
        }

        /* 대기 큐에 있던 클라이언트 요청을 자식 프로세스에게 처리하라고 지시한다.
        *  http_handler를 통해서 클라이언트로 부터 받은 요청을 분석하고, 해당하는 파일을 읽어서
        *  클라이언트에게 전송한다. 즉, noot.html파일의 내용을 받아와서 웹페이지에 띄우기 위한 작업을 수행
        *  해당 작업이 끝나면 accept_sock도 닫는다. 
        *  listen_sock와 accept_sock를 닫아주는 이유는 자식 프로세스라서다. 부모 프로세스는 계속해서 요청을 받고 있다. */
        pid = fork();
        if (pid == 0) { 
            close(listen_sock); 
	        http_handler(accept_sock); 
	        close(accept_sock);
            exit(0);
        }

        if (pid != 0)   { 
	    close(accept_sock); 
	}

        if (pid < 0)    { 
	    perror("Failed to fork.\n"); 
	}
	
	
    }

}


