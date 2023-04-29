#include <stdio.h> /* C언어 기본 헤더 */
#include <stdlib.h> /* has macros sucha as EXIT_FAILURE */
#include <errno.h> 
#include <string.h> /* 문자열 헤더 */
#include <sys/types.h> /* provides required data types */
#include <netinet/in.h> /* has the sockaddr_in structure */
#include <sys/socket.h> /* holds address familt and socket functions */ 
#include <sys/wait.h> 
#include <sys/stat.h> /* stat()함수 */
#include <unistd.h> /* fork()함수: 프로세스 생성 */
#include <fcntl.h> /* open()함수: 파일 열기*/

#define BACKLOG 10 /* how many pendinf connections queue will hold */
#define BUF_SIZE 1024

#define HEADER_FMT "HTTP/1.1 %d %s\nContent-Length: %ld\nContent-Type: %s\n\n"

void http_handler(int new_fd); /* http 내용 받아 오고 쓰기 */
void write_content(int new_fd, char *header, int content); /* html 파일을 추가로 만들고 쓰기 */
void find_mime(char *ct_type, char *uri); /* 파일 형식 지정하기 */

int main(int argc, char **argv) {
	if (argc < 2) exit(0);
    int port = atoi(argv[1]); /* 포트 번호를 main()의 인자로 지정하기 */
	
	int sock_fd, new_fd; /* sock_fd는 서버의 소켓, new_fd는 클라이언트의 소켓 */
	struct sockaddr_in my_addr; /* my address */
	struct sockaddr_in their_addr; /* connector address */
	socklen_t sin_size;

	/* 소켓 만들기 */
	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("[ERR] socket\n");
		exit(1);
	}
	/* 소켓에 IP 주소와 포트 번호 할당하기 */
	my_addr.sin_family = AF_INET; /* 주소 체계 */
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* 내 장치의 IP 주소 */
	my_addr.sin_port = htons(port); /* 포트 번호 설정 */
	if (bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("[ERR] bind");
		exit(1);
	}
	/* 클라이언트 요청을 대기하기 */
	if (listen(sock_fd, BACKLOG) == -1) {
		perror("[ERR] listen\n");
		exit(1);
	}
	while(1) { /* 클라이언트 요청을 받아들이는 loop() */
		/* 클라이언트 요청 받아들이기 */
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sock_fd, (struct sockaddr *) &their_addr, &sin_size)) < 0) {
			perror("[ERR] accept\n");
			continue;
		}
		char header[BUF_SIZE];
		write_content(new_fd, header, port);
		/* 프로세스 생성하기 */
		int pid = fork();
		if (pid == 0) { /* 자식 프로세스 */
			close(sock_fd); 
			http_handler(new_fd);
			close(new_fd);
			exit(0);
		}
		if (pid != 0) {
			close(new_fd);
		}
		if (pid < 0) {
			perror("[ERR] fork\n");
		}
    }
}

void http_handler(int new_fd) {
	char header[BUF_SIZE];
	char buf[BUF_SIZE];
	/* buf에 데이터를 읽어오기, http 헤더 내용이 버퍼에 저장 */
	if(read(new_fd, buf, BUF_SIZE) == -1) {
		perror("[ERR] read request\n");
		write_content(new_fd, header, 5001); /* 500 에러: http 헤더를 읽어올 수 없음 */
		return;
	}
	printf("%s", buf); /* http 헤더 내용을 터미널에 출력하기 */
	/* http 헤더 내용에서 보낼 데이터 확인하기 */
	char *method = strtok(buf, " "); /* method = "GET" */
    char *uri = strtok(NULL, " "); /* uri = "html.html" */
    if (method == NULL || uri == NULL) {
        perror("[ERR] URI\n");
		write_content(new_fd, header, 5002); /* 500 에러: http 헤더 형식이 맞지 않음 */
        return;
    }
	/* uri에 맞는 파일 찾기 */
	char safe_uri[BUF_SIZE];
    char *local_uri;
    struct stat st;
	strcpy(safe_uri, uri);
    if (!strcmp(safe_uri, "/")) strcpy(safe_uri, "/html.html");
    local_uri = safe_uri + 1;
	if (stat(local_uri, &st) < 0) {
        perror("[WARN] URI\n"); 
        write_content(new_fd, header, 404); /* 404 에러: 찾고자 하는 파일이 없음 */
		return;
    }
	/* 보낼 소켓에 파일을 열기 */
    int fd = open(local_uri, O_RDONLY);
    if (fd < 0) {
        perror("[ERR] open file\n");
		write_content(new_fd, header, 5003); /* 500 에러: 파일을 열 수 없음 */
        return;
    }
	/* http 형식에 맞춘 헤더 넣기 */
	long ct_len = st.st_size; 
    char ct_type[40];
    find_mime(ct_type, local_uri);
	sprintf(header, HEADER_FMT, 200, "OK", ct_len, ct_type);
	write(new_fd, header, strlen(header));
	/* 파일 쓰기 */
    int cnt;
    while ((cnt = read(fd, buf, BUF_SIZE)) > 0)
        write(new_fd, buf, cnt);
}
void write_content(int new_fd, char *header, int ct_num) {
	char* content;
	long len;
	char* str = "";
	switch (ct_num) {
		case 404:
			content = "<h1>404 에러: 찾고자 하는 파일이 없음</h1>\n";
			len = sizeof(content);
			sprintf(header, HEADER_FMT, 404, "Not Found", len, "text/html"); 
			break;
		case 5001:
			content = "<h1>500 에러: http 헤더를 읽어올 수 없음</h1>\n";
			len = sizeof(content);
			sprintf(header, HEADER_FMT, 500, "Internal Server Error", len, "text/html"); 
			break;
		case 5002:
			content = "<h1>500 에러: http 헤더 형식이 맞지 않음</h1>\n";
			len = sizeof(content);
			sprintf(header, HEADER_FMT, 500, "Internal Server Error", len, "text/html"); 
			break;
		case 5003:
			content = "<h1>500 에러: 파일을 열 수 없음</h1>\n";
			len = sizeof(content);
			sprintf(header, HEADER_FMT, 500, "Internal Server Error", len, "text/html"); 
			break;
		default:
			sprintf(str, "%d", ct_num); // integer to string
			content = strcat("<h1>접속한 서버의 포트 번호: ", str);
			content = strcat(content, "</h1>\n");
			len = sizeof(content);
			sprintf(header, HEADER_FMT, 200, "Port Number", len, "text/html"); 
			break;
	}
	write(new_fd, header, strlen(header));
	write(new_fd, content, len);
}

void find_mime(char *ct_type, char *uri) {
    char *ext = strrchr(uri, '.');
    if (!strcmp(ext, ".html")) 
        strcpy(ct_type, "text/html");
    else if (!strcmp(ext, ".jpg") || !strcmp(ext, ".jpeg")) 
        strcpy(ct_type, "image/jpeg");
    else if (!strcmp(ext, ".png"))
        strcpy(ct_type, "image/png");
    else if (!strcmp(ext, ".css"))
        strcpy(ct_type, "text/css");
    else if (!strcmp(ext, ".js"))
        strcpy(ct_type, "text/javascript");
    else strcpy(ct_type, "text/plain");
}


