#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const int ZYC_BUFF_SIZE = 1024;
const int ZYC_TOK_SIZE = 64;
const char* ZYC_TOK_DELIM = " \t\n\a\r";


int zyc_cd(char **args);
int zyc_help(char **args);
int zyc_exit(char **args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &zyc_cd,
  &zyc_help,
  &zyc_exit
};

int zyc_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int zyc_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "zyc: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("zyc");
    }
  }
  return 1;
}

int zyc_help(char **args)
{
  int i;
  printf("Maksimka zyc\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < zyc_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int zyc_exit(char **args)
{
  return 0;
}


char* zyc_readline(){
	int position = 0;
	char* buffer = calloc(ZYC_BUFF_SIZE, sizeof(char));
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

		if(position >= ZYC_BUFF_SIZE){
			fprintf(stderr, "Buffer size overheaped\n");
			abort();
		}
	}
}

char** zyc_parse_line(char* line){
	int buf_size =ZYC_TOK_SIZE, position = 0;
	char**  tokens = calloc(buf_size, sizeof(char));
	char* token;
	if(tokens == NULL){
		fprintf(stderr, "Failed to allocate tokens while parsing string\n");
		abort();
	}
	token = strtok(line, ZYC_TOK_DELIM);
	while(token != NULL){
		tokens[position++] = token;
		if(position >= buf_size){
			fprintf(stderr, "Token buffer size overheaped\n");
			abort();
		}
		token = strtok(NULL, ZYC_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}

int zyc_launch(char** args){
	pid_t pid, wpid;
	int status;
	pid = fork();

	if(pid == 0){
		if(execvp(args[0], args) == -1){
			perror("zyc");
		}
		abort();
	}
	else if(pid < 0)
		perror("zyc");
	else{
		do {
    	wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

int zyc_run(char** args){
	int i ;
	if(args == NULL)
		return 1;
	for(i = 0; i < zyc_num_builtins(); ++i){
		if(strcmp(args[0], builtin_str[i]) == 0){
			return (*builtin_func[i])(args);
		}
	}
	return zyc_launch(args);
}

void zyc_loop(){
	char* line;
	char** arguments;
	int status;

	do{
		printf("$ ");
		line = zyc_readline();
		arguments = zyc_parse_line(line);
		status = zyc_run(arguments);

		free(line);
		free(arguments);
	}while(status);
}


int main(int argc, char const *argv[])
{
	zyc_loop();
	return 0;
}