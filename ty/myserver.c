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

#define BUF_SIZE 1024
#define MYPORT 3490
#define HEADER_FMT "HTTP/1.1 %d %s\nContent-Length: %ld\nContent-Type: %s\n\n"

int bind_lsock(int lsock, int port) {
    struct sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(port);

    return bind(lsock, (struct sockaddr *)&sin, sizeof(sin));
}

void fill_header(char *header, int status, long len, char *type) {
    char status_text[40];
    switch (status) {
        case 200:
            strcpy(status_text, "OK"); break;
        case 404:
            strcpy(status_text, "Not Found"); break;
        case 500:
        default:
            strcpy(status_text, "Internal Server Error"); break;
    }
    sprintf(header, HEADER_FMT, status, status_text, len, type);
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


void http_handler(int asock) {
    char header[BUF_SIZE];
    char buf[BUF_SIZE];

    if (read(asock, buf, BUF_SIZE) < 0) {
        perror("Failed to read request.\n");
	return;
    }

    char *method = strtok(buf, " ");
    char *uri = strtok(NULL, " ");
    if (method == NULL || uri == NULL) {
        perror("Failed to identify method, URI.\n");
        return;
    }

    printf("Handling Request: method=%s, URI=%s\n", method, uri);
    
    char safe_uri[BUF_SIZE];
    char *local_uri;
    struct stat st;

    strcpy(safe_uri, uri);
    if (!strcmp(safe_uri, "/")) strcpy(safe_uri, "/index.html");
    
    local_uri = safe_uri + 1;
    if (stat(local_uri, &st) < 0) {
        perror("No file found matching URI.\n");
        return;
    }

    int fd = open(local_uri, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file.\n");
        return;
    }

    int ct_len = st.st_size;
    char ct_type[40];
    find_mime(ct_type, local_uri);
    fill_header(header, 200, ct_len, ct_type);
    write(asock, header, strlen(header));

    int cnt;
    while ((cnt = read(fd, buf, BUF_SIZE)) > 0){
        write(asock, buf, cnt);
    }
    printf("%s\n", buf);
}


int main(int argc, char **argv) {
	int port, pid;
    	int lsock, asock;

    	struct sockaddr_in remote_sin;
    	socklen_t remote_sin_len;

   	if (argc < 2) {
       		printf("Usage: \n");
        	printf("\t%s {port}: runs mini HTTP server.\n", argv[0]);
        	exit(0);
    	}

    	port = atoi(argv[1]);
	printf("The server will listen to port: %d.\n", port);

	lsock = socket(AF_INET, SOCK_STREAM, 0);
	if(lsock < 0){
		perror("Failed to create.\n");
		exit(1);
	}
	
	if(bind_lsock(lsock, port) < 0){
		perror("Failed to bind.\n");
		exit(1);
	}

	if(listen(lsock, 10) < 0){
		perror("Failed to listen.\n");
		exit(1);
	}

	signal(SIGCHLD, SIG_IGN);


    while (1) {
        printf("waiting...\n");
        asock = accept(lsock, (struct sockaddr *)&remote_sin, &remote_sin_len);
        if (asock < 0) {
            perror("Failed to accept.\n");
            continue;
        }

        pid = fork();
        if (pid == 0) { 
            close(lsock); http_handler(asock); close(asock);
            exit(0);
        }

        if (pid != 0)   { close(asock); }
        if (pid < 0)    { perror("Failed to fork.\n"); }
	
	
    }

}


