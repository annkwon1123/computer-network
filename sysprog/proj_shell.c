#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_LEN 100
int main() {

	// 1. get the str (fgets)
	char str[MAX_LINE];
	
	printf("proj_shell> ");
	fgets(str, MAX_LINE, stdin);

	// 2. split the str (strtok)
	const char s[3] = " ";
	char *token;

	token = strtok(str, s);
	while(token != NULL) {
		printf("%s\n", token);
		token = strtok(NULL, s);

		// 3. compare the splited str (strcmp)
		if(strcmp(token, "exit")) {
			exit(1);
		}
		else if(strcmp(token, "&")) {
				
		}
	}
}

int mk_pros(char * token, char * tokens[]) {
	pid_t pid;
	int flag;
	flag = 0;

	pid = fork();

	if(pid < 0) {
		perror("fork");
	}
	else if(pid == 0) {
		execvp(token, tokens);
		flag = 1
	}
	else {
		if(flag == 0)
		waitpid(




