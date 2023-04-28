#include <stdio.h> /* basic C header */
#include <stdlib.h> /* has macros sucha as EXIT_FAILURE */
#include <errno.h> 
#include <string.h> /* header to help with strings */
#include <sys/types.h> /* provides required data types */
#include <netinet/in.h> /* has the sockaddr_in structure */
#include <sys/socket.h> /* holds address familt and socket functions */ 
#include <sys/wait.h> 
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 6560 /* port through which connection is to be madea */
#define BACKLOG 10 /* how many pendinf connections queue will hold */
#define BUF_SIZE 1024
#define HEADER_FMT "HTTP/1.1 %d %s\nContent-Length: %ld\nContent-Type: %s\n\n"

void http_handler(int new_fd);
void fill_header(char *header, int status, long len, char *type);
void find_mime(char *ct_type, char *uri);

int main() {
	int sock_fd, new_fd; /* listen on sock_fd, new connection on new_fd */
	struct sockaddr_in my_addr; /* my address */
	struct sockaddr_in their_addr; /* connector address */
	socklen_t sin_size;

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("[ERR] socket\n");
		exit(1);
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT); /* short, network byte order */
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY allowws clients to connect to any one of the host's IP address */

	if (bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("[ERR] bind");
		exit(1);
	}

	if (listen(sock_fd, BACKLOG) == -1) {
		perror("[ERR] listen\n");
		exit(1);
	}
	char requests[BUF_SIZE];
	while(1) { /* main accept() loop */
		sin_size = sizeof(struct sockaddr_in);

		if ((new_fd = accept(sock_fd, (struct sockaddr *) &their_addr, &sin_size)) == -1) {
			perror("[ERR] accept\n");
			continue;
		}
	
		//printf("[INFO] server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));

		int pid = fork();
		if (pid == 0) {
			close(sock_fd); http_handler(new_fd); close(new_fd);
			exit(1);
		}
		if (pid > 0) {
			close(new_fd);
		}
		if (pid < 0) {
			perror("[ERR] fork\n");
		}
    }
	printf("%s", requests);
}
void http_handler(int new_fd) {
	char header[BUF_SIZE];
	char buf[BUF_SIZE];

	if(read(new_fd, buf, BUF_SIZE) == -1) {
		perror("[ERR] read request\n");
		return;
	}
	printf("%s", buf);

	char *method = strtok(buf, " ");
    char *uri = strtok(NULL, " ");
    if (method == NULL || uri == NULL) {
        perror("[ERR] Failed to identify method, URI.\n");
        return;
    }

	//printf("[INFO] Handling Request: method=%s, URI=%s\n", method, uri);
    char safe_uri[BUF_SIZE];
    char *local_uri;
    struct stat st;

	strcpy(safe_uri, uri);
    if (!strcmp(safe_uri, "/")) strcpy(safe_uri, "/html.html");
    
    local_uri = safe_uri + 1;
	if (stat(local_uri, &st) < 0) {
        perror("[WARN] No file found matching URI.\n");
        return;
    }

	int fd = open(local_uri, O_RDONLY);
    if(fd == -1) {
        perror("[ERR] open file\n");
    	write(new_fd, header, strlen(header));
        return;
    }

	int ct_len = st.st_size;
    char ct_type[40];
    find_mime(ct_type, local_uri);
	fill_header(header, 200, ct_len, ct_type);
    write(new_fd, header, strlen(header));

    int cnt;
    while ((cnt = read(fd, buf, BUF_SIZE)) > 0)
        write(new_fd, buf, cnt);
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


