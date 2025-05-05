#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const int MY_SHELL_BUFF_SIZE = 1024;
const int MY_SHELL_TOK_SIZE = 64;
const char* MY_SHELL_TOK_DELIM = " \t\n\a\r";


int my_shell_cd(char **args);
int my_shell_help(char **args);
int my_shell_exit(char **args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &my_shell_cd,
  &my_shell_help,
  &my_shell_exit
};

int my_shell_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int my_shell_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "my_shell: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("my_shell");
    }
  }
  return 1;
}

int my_shell_help(char **args)
{
  int i;
  printf("Maksimka my_shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < my_shell_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int my_shell_exit(char **args)
{
  return 0;
}


char* my_shell_readline(){
	int position = 0;
	char* buffer = calloc(MY_SHELL_BUFF_SIZE, sizeof(char));
	int c;
	if(buffer == NULL){
		fprintf(stderr, "Bad allocation\n");
		abort();
	}

	while(1){
		c = getchar();
		if(c == EOF || c == '\n'){
			buffer[position] = '\0';
			return buffer;
		}else buffer[position++] = c;

		if(position >= MY_SHELL_BUFF_SIZE){
			fprintf(stderr, "Buffer size overheaped\n");
			abort();
		}
	}
}

char** my_shell_parse_line(char* line){
	int buf_size =MY_SHELL_TOK_SIZE, position = 0;
	char**  tokens = calloc(buf_size, sizeof(char));
	char* token;
	if(tokens == NULL){
		fprintf(stderr, "Failed to allocate tokens while parsing string\n");
		abort();
	}
	token = strtok(line, MY_SHELL_TOK_DELIM);
	while(token != NULL){
		tokens[position++] = token;
		if(position >= buf_size){
			fprintf(stderr, "Token buffer size overheaped\n");
			abort();
		}
		token = strtok(NULL, MY_SHELL_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}

int my_shell_launch(char** args){
	pid_t pid, wpid;
	int status;
	pid = fork();

	if(pid == 0){
		if(execvp(args[0], args) == -1){
			perror("my_shell");
		}
		abort();
	}
	else if(pid < 0)
		perror("my_shell");
	else{
		do {
    	wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

int my_shell_run(char** args){
	int i ;
	if(args == NULL)
		return 1;
	for(i = 0; i < my_shell_num_builtins(); ++i){
		if(strcmp(args[0], builtin_str[i]) == 0){
			return (*builtin_func[i])(args);
		}
	}
	return my_shell_launch(args);
}

void my_shell_loop(){
	char* line;
	char** arguments;
	int status;

	do{
		printf("$ ");
		line = my_shell_readline();
		arguments = my_shell_parse_line(line);
		status = my_shell_run(arguments);

		free(line);
		free(arguments);
	}while(status);
}


int main(int argc, char const *argv[])
{
	my_shell_loop();
	return 0;
}