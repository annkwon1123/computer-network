/*
    주어진 매개 변수를 기준으로 HTTP 헤더 형식 지정
*/
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

/*
    uri로부터 content type 찾기
*/
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

/*
    handler for not found(404)
*/
void handle_404(int asock) {
    char header[BUF_SIZE];
    fill_header(header, 404, sizeof(NOT_FOUND_CONTENT), "text/html");
    write(asock, header, strlen(header));
}

/*
    handler for internal server error(500)
*/
void handle_500(int asock) {
    char header[BUF_SIZE];
    fill_header(header, 500, sizeof(SERVER_ERROR_CONTENT), "text/html");
    write(asock, header, strlen(header));
}

/*
    main http handler
    요청된 리소스를 열고 전송
    failure에 대한 에러 호출
*/
void http_handler(int asock) {
    char header[BUF_SIZE];
    char buf[BUF_SIZE];
    char safe_uri[BUF_SIZE];
    char *local_uri;
    struct stat st;

    if (read(asock, buf, BUF_SIZE) < 0) {
        perror("[ERR] Failed to read request.\n");
        handle_500(asock); return;
    }

    printf("%s",buf); // 버퍼에 읽어들인 내용 모두 출력

    char *method = strtok(buf, " ");
    char *uri = strtok(NULL, " ");

    strcpy(safe_uri, uri);
    if (!strcmp(safe_uri, "/")) strcpy(safe_uri, "/index.html"); // '/'라면 자동으로 index.html을 match
    
    local_uri = safe_uri + 1;
    if (stat(local_uri, &st) < 0) {
        handle_404(asock); return;
    }

    int fd = open(local_uri, O_RDONLY);
    if (fd < 0) {
        handle_500(asock); return;
    }

    int ct_len = st.st_size;
    char ct_type[40];
    find_mime(ct_type, local_uri);
    fill_header(header, 200, ct_len, ct_type);
    write(asock, header, strlen(header));

    int cnt;
    while ((cnt = read(fd, buf, BUF_SIZE)) > 0)
        write(asock, buf, cnt);

}