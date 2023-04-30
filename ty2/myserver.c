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


/* server socket�� port�� bind�ϴ� �Լ�
    �־��� port�� server socket�� bind�ϰ� �����ϸ� 0�� ��ȯ*/
int bind_listen_sock(int listen_sock, int port) {
    struct sockaddr_in sin;  /* 4byte�� IP�ּҰ� ����Ǵ� ����ü */

    sin.sin_family = AF_INET;   /* ���ͳ� �ּ� ü��� ���� */
    sin.sin_addr.s_addr = htonl(INADDR_ANY);    /* INADDR_ANY�� host byte �������� network byte ������ ��ȯ */
    sin.sin_port = htons(port); /* 2byte�� portnum */

    return bind(listen_sock, (struct sockaddr *)&sin, sizeof(sin));
}

/* http �������� �԰ݿ� ������ Ŭ���̾�Ʈ�� �������� �������� ������ �㵵���ϴ� header ä��� */
void fill_header(char *header, int status, long len, char *type) {
    char status_text[50];
    switch (status) {
        case 200:   /* ��û ���� ó�� */
            strcpy(status_text, "OK"); break;
        case 404:   /* ��û�� resource ã�� �� ���� */
            strcpy(status_text, "Not Found"); break;
        case 500:   /* �������� ó�� �߿� ���� �߻� */
        default:
            strcpy(status_text, "Internal Server Error"); break;
    }
    sprintf(header, HEADER_FMT, status, status_text, len, type);
}

/* �о���� html������ ó���ϱ� ���� �Լ��̴�. ���� Ȯ���ڸ� �����Ͽ� MIME Ÿ���� �����ϰ� 
*  filetype ���ۿ� �����Ͽ� �� ���������� � ������ �������� �˷��ֱ� ���ؼ� �ʿ��ϴ�. */
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

/* 404���� ó�� */
void handle_404(int accept_sock) {
    char header[BUF_SIZE];
    fill_header(header, 404, sizeof(NOT_FOUND_CONTENT), "text/html");

    write(accept_sock, header, strlen(header));
    write(accept_sock, NOT_FOUND_CONTENT, sizeof(NOT_FOUND_CONTENT));
}

/* 500���� ó��*/
void handle_500(int accept_sock) {
    char header[BUF_SIZE];
    fill_header(header, 500, sizeof(SERVER_ERROR_CONTENT), "text/html");

    write(accept_sock, header, strlen(header));
    write(accept_sock, SERVER_ERROR_CONTENT, sizeof(SERVER_ERROR_CONTENT));
}

/* Ŭ���̾�Ʈ�� ��û�� ���������� ó���ϱ� ���� �Լ��̴�.
*  request message�� �Ľ��ϰ� ������ �۾��� �����ϰ�, request�� header ���� ��,
*  ��û�� ���� ���� �����͸� �����ϴ� ������ �����Ѵ�. */
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

    int fd = open(local_uri, O_RDONLY); /* local_uri ����� ������ ���� �о���� ���� fd ���� */
    if (fd < 0) {
        perror("Failed to open file.\n");
        handle_500(accept_sock); return;
    }

    int file_len = st.st_size;
    char filetype[40];
    get_file(filetype, local_uri);
    fill_header(header, 200, file_len, filetype);   /* html ������ �о �ش� ������ ������ ���������� ���� ���� ��� */
    write(accept_sock, header, strlen(header)); /* ���� ���� ���� ������ ���� �������ϴ� http ���� �޼��� header ������ ��Ƽ� Ŭ���̾�Ʈ���� ����. */

    int cnt;
    while ((cnt = read(fd, buf, BUF_SIZE)) > 0)
        write(accept_sock, buf, cnt);
   
}


/* socket ��� ������ ������ ����.
 * socket����, bind, listen, accept(Ŭ���̾�Ʈ�� ���� connect��û ����), ������ �ۼ���
 */
int main(int cmdc, char **cmdv) {
	int port, pid;
    int listen_sock, accept_sock;

    struct sockaddr_in myaddr;
    socklen_t myaddr_len;

    /* ���̰� 2���� ������ portnum�� �Էµ��� ���� ���� */
   	if (cmdc < 2) {
       		printf("Usage: \n");
        	printf("\t%s {port}: runs mini HTTP server.\n", cmdv[0]);
        	exit(0);
    	}

    /* �Է¹��� portnum�� ���������� ��ȯ�Ͽ� ���� */
    port = atoi(cmdv[1]);
	printf("The server will listen to port: %d.\n", port);

    /* socket ���� �� ���� ��û�� ������ fd�� listen_sock�� ����
    *  �ּ� ü�� AF_INET���� ����, TCP�������� ���, �� �� �⺻ ���� */
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock < 0){
		perror("Failed to create.\n");
		exit(1);
	}
	/* ������ listen_sock�� portnum bind */
	if(bind_listen_sock(listen_sock, port) < 0){
		perror("Failed to bind.\n");
		exit(1);
	}

    /* socket ���� ��� ���� ��ȯ */
	if(listen(listen_sock, 10) < 0){
		perror("Failed to listen.\n");
		exit(1);
	}

    /* 
    *  ������ Ŭ���̾�Ʈ ��û�� ���� ó���� �ڽ� ���μ����� fork�Ͽ� ó���ϱ� ������
    *  ����� �� �θ� ���μ����� �����ϵ��� �����Ͽ� ���� �Ǵ� ���� �����Ѵ�. 
    */
	signal(SIGCHLD, SIG_IGN);

    while (1) {
        printf("waiting...\n");
        /* Ŭ���̾�Ʈ�� connect ��û�� ������ accept�� ���ؼ� ������ ����
        *  �����ϸ� ���ο� socket�� return�ϰ� �̸� ���� �����͸� �ۼ����Ѵ�. */
        accept_sock = accept(listen_sock, (struct sockaddr *)&myaddr, &myaddr_len); 
        if (accept_sock < 0) {
            perror("Failed to accept.\n");
            continue;
        }

        /* ��� ť�� �ִ� Ŭ���̾�Ʈ ��û�� �ڽ� ���μ������� ó���϶�� �����Ѵ�.
        *  http_handler�� ���ؼ� Ŭ���̾�Ʈ�� ���� ���� ��û�� �м��ϰ�, �ش��ϴ� ������ �о
        *  Ŭ���̾�Ʈ���� �����Ѵ�. ��, noot.html������ ������ �޾ƿͼ� ���������� ���� ���� �۾��� ����
        *  �ش� �۾��� ������ accept_sock�� �ݴ´�. 
        *  listen_sock�� accept_sock�� �ݾ��ִ� ������ �ڽ� ���μ����󼭴�. �θ� ���μ����� ����ؼ� ��û�� �ް� �ִ�. */
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


