	/**
	* Prepared by: Daniel Olaya
	* Student #: V00855054
	* Date: Jan, 2018
	* Professor: Zehui Zheng zehuizheng@uvic.ca
	* CSC360 - Operative Systems
	*/

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h> //getcwd()
#include <readline/readline.h>
#include <readline/history.h>

#define dirSize 256
#define cmdSize 256

struct node{ 
    pid_t pid; 
    char cmd[1024]; 
    char opt[1024];
    struct node* next; 
};

struct node *root, *temp;
int counter = 0;

void parse(char* input, char** next);
void executeCMD(char** args, char* opt, int isBg);
void changeDir(char** path);
void bgCmd(char** args);
void addNode(pid_t pid, char* args, char* opt);
void removeNode(pid_t pid);
void checkBg();
void getPrompt(char* dest);
void printList();

	//Helper method that checks if any of the child processes have changed status, 
	//removes those that have exited.
void checkBg(){
	pid_t ter; 
	if (counter > 0){
		ter = waitpid(0, NULL, WNOHANG);
		// printf("checkBg - pid:%ld\n", (long)ter);
		if (ter>0){
			removeNode(ter);
		}
	}
}

	// Removes node from linked list by pid
void removeNode(pid_t pid){
	assert(pid > 0);
	assert(counter > 0);
	struct node* trail;
	struct node* curr;

	if (counter == 1){
		// printf("\nREMOVING single node - PID:%ld\n",(long)pid); 
		free(root);
		root = NULL;
	}else{
		curr = root->next;
		for (trail = root ; (curr->pid != pid) && (curr->next != NULL) ; curr = curr->next, trail = trail->next){
			printf("IN LOOP - trail:%ld-%s-%s | curr:%ld-%s-%s\n", (long)trail->pid, trail->cmd, trail->opt, (long)curr->pid, curr->cmd, curr->opt);
		}
		printf("POST LOOP - trail:%ld-%s-%s | curr:%ld-%s-%s\n", (long)trail->pid, trail->cmd, trail->opt, (long)curr->pid, curr->cmd, curr->opt);
		if (curr->next == NULL){ 	//process to remove is at end of list	
			trail->next = NULL; 
		} else {
			trail->next = curr->next;	
		} 
		free(curr);
	}
	counter --;
}

	//Takes user input in string and parses to an array of pointers to string using white space delimiter
void parse(char* input, char** next){
		char *args[cmdSize]; //an array that stores pointers to a char

		char *temp = strtok(input, " \n");
		while (temp != NULL){
			*next = temp;
			next++;		// previous line and this could be represented as  *next++ = temp;
			temp = strtok(NULL, " \n");
		}
		*next = NULL;
}

	/** 
	* Executes command given by user using fork() and execvp() sys calls
	* The fork system call creates a new process. 
	* The new process created by fork() is copy of the current process except the returned value. 
	* The exex system call replaces the current process with a new program.
	* https://www.geeksforgeeks.org/fork-system-call/
	*/
void executeCMD(char** args, char* opt, int isBg){
	pid_t pid;
	int status;

	if (isBg == 1) { // bg process
		if ((pid = fork()) < 0) {	// fork a child process
			printf("*** ERROR: forking child process failed\n");
			exit(1);
		}
		if (pid != 0){
			addNode(pid, *args, opt);	
		}
		if (pid == 0) {	// child process
			if (execvp(*args, args) < 0){
				printf("*** ERROR: exec failed\n");
				exit(1);
			}
		}else {		// parent process
			usleep(20000);
		}
	}else { 	// Non bg process
		if ((pid = fork()) < 0) {	// fork a child process
			printf("*** ERROR: forking child process failed\n");
			exit(1);
		}
		if (pid == 0) {	// child process
			if (execvp(*args, args) < 0){
				printf("*** ERROR: exec failed\n");
				exit(1);
			}
		}
		else {		// parent process
			// sys call suspends execution of the calling proces until one of its children terminates wait() on success returns the process ID of the terminated child
			while (wait(&status) != pid);
		}
	}
}

	// Implements cd command, if path is '~', sets pwd to the home environment variable
void changeDir(char** path){
	if (!strcmp(*(path+1),"~")){
		// printf("~ HOME : %s\n", getenv("HOME"));
		char* home = getenv("HOME");
		chdir(home);
	}else{
		chdir(*(path+1));
	}
}

	//Method that removes first element from argument array (bg) and creates a string with all 
	//arguments used for bglist command
void bgCmd(char** args){
	//removes bg from array pointer list
	int i;
	for (i = 0; *(args+i) != '\0' ; i++ ){ 
		*(args+i) = *(args+i+1); 
	}
	*(args+i+1) = '\0';

	//creates single string with all optional arguments for printlist 
	char opt[cmdSize];
	memset(opt,'\0', cmdSize);
	for (i = 1 ; *(args+i) != '\0' ; i++){
		strcat(opt,*(args+i));
		strcat(opt, " ");
	}
	// printf("OPT STRING:%s\n", opt);
	executeCMD(args, opt, 1);
}

void addNode (pid_t pid, char* args, char* opt){
	struct node* new_node = (struct node*)malloc(sizeof(struct node));
	new_node->pid = pid;
	strcpy(new_node->cmd,args);
	strcpy(new_node->opt,opt);
	new_node->next = NULL;

	if (counter == 0){
		root = new_node;
		// printf("Added root - pid=%ld, cmd:%s, args:%s\n", (long)root->pid, root->cmd, root->opt);
	}
	else{
		for (temp = root ; temp->next != NULL ; temp = temp->next);
		temp->next = new_node;
		// printf("Added node - pid=%ld, cmd:%s, args:%s\n", (long)temp->pid, temp->cmd, temp->opt);
	}
	counter ++;
}

	//Helper method used for bglist command
void printList(){
	struct node *temp;
	int i=1;
	printf("\n----Running Processes (bg)----\n");
	for (temp = root ; temp != NULL ; temp = temp->next, i++)
	printf("%i-PID:%ld | CMD:%s |ARG:%s\n", i,(long)temp->pid, temp->cmd, temp->opt);
	printf("Total Background jobs:%d\n", counter);
	printf("----End of List (bg)----\n\n");
}

	// Gets current directory and creates the prompt char array
void getPrompt (char* dest){
	char cwd[dirSize];
	memset(dest, 0, dirSize);
	memset(cwd, 0, dirSize);
	strcat(dest, "SSI: ");
	getcwd(cwd, sizeof(cwd));
	// printf("Current working dir is:%s\n", cwd);
	strcat(dest, cwd);
	strcat(dest, " > ");
	// printf("dest:%s\n", dest);
}

int main(){
	printf("===Enter shell===\n");
	char prompt[dirSize];
	memset(prompt, 0, dirSize);

	getPrompt(prompt);
	// printf("prompt:%s\n", prompt);

	int bailout = 0;
	while (!bailout) {
		char* args[cmdSize]; //pointer to arrays of chars ie:[const char* example[]={"array","of","strings"}];
		char* input = readline(prompt);
		// printf("Main1 - input:%s\n", input);
		parse(input, args);

		if (!strcmp(*(args), "exit")) {
			bailout = 1;
		}  else if (! (strcmp(*(args), "cd")) ){
			char* newDir[dirSize];
			changeDir(args);
			getPrompt(prompt);
		}  else if (!(strcmp(*(args), "bg")) ){
			bgCmd(args);
		}  else if (!(strcmp(*(args), "bglist")) ){
			printList();
		} else {
			executeCMD(args, input, 0);
		}
		checkBg();
	}
	printf("===Exit shell===\n");
}