#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <glob.h>
#include "sh.h"

int sh( int argc, char **argv, char **envp )
{
	int pid; //updated from initial file
	char *prompt = calloc(PROMPTMAX, sizeof(char));
	char *commandline = calloc(MAX_CANON, sizeof(char));
	char *command, *arg, *commandpath, *p, *pwd, *owd;
	char **args = calloc(MAXARGS, sizeof(char*));
	char **enviro;
	int uid, i, status, argsct, go = 1;
	struct passwd *password_entry;
	char *homedir;
	struct pathelement *pathlist;
	char buff[MAXIMUM];
	char promptBuff[PROMPTMAX];

	uid = getuid();
	password_entry = getpwuid(uid);               /* get passwd info */
	homedir = password_entry->pw_dir;		/* Home directory to start
											   out with*/

	if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
	{
		perror("getcwd");
		exit(2);
	}
	owd = calloc(strlen(pwd) + 1, sizeof(char));
	memcpy(owd, pwd, strlen(pwd));
	prompt[0] = ' '; prompt[1] = '\0';

	/* Put PATH into a linked list */
	pathlist = get_path();

	while ( go )
	{
		/* print your prompt */
		printf("\n%s [%s]> ", prompt, pwd);
		/* get command line and process */
		if (fgets(buff, MAXIMUM, stdin) != NULL)
		{
			int len = strlen(buff);
			if(buff[len - 1] == '\n')
				buff[len-1] = 0;
			strcpy(commandline, buff);
		}

		int i = 0;
		char *tok = strtok(commandline, " ");
		command = tok;
		memset(args, '\0', MAXARGS*sizeof(char*));
		while (tok)
		{
			args[i] = tok;
			tok = strtok(NULL, " ");
			i++;
		}

		if (command != NULL)
		{
			/* check for each built in command and implement */
			if (strcmp(command, "exit") == 0) {
				printExec(command);
				break;
			}
			//which
			else if (strcmp(command, "which") == 0) {
				for (int i = 1; args[i] != NULL; i++) {
					commandpath = which(args[i], pathlist);
					printf("\n%s", commandpath);
					free(commandpath);
				}
			}
			//where
			else if (strcmp(command, "where") == 0) {
				for (int i = 1; args[i] != NULL; i++) {
					commandpath = where(args[i], pathlist);
					free(commandpath);
				}
			}

			//command is cd.
			else if(strcmp(command, "cd" ) == 0)
			{
				//No arguments: returns to home directory.
	  			printExec(command);
	  			if(args[1] == NULL)
				{
	    			strcpy(owd, pwd);
	    			strcpy(pwd, homedir);
	    			chdir(pwd);
	  			}
				//Goes back to previous directory.
				else if(strcmp(args[1], "-") == 0)
				{
	    			p = pwd;
	   				pwd = owd;
	    			owd = p;
	    			chdir(pwd);
	  			}
				//Too many arguments error.
				else if(args[1] != NULL && args[2] == NULL)
				{		
	    			if(chdir(args[1]) == -1)
	      				perror("Error ");
					else
					{
	      				memset(owd, '\0', strlen(owd));
	      				memcpy(owd, pwd, strlen(pwd));  
	      				getcwd(pwd, PATH_MAX+1);
	    			}	    
	  			}
			}

			//Checks for pwd command
			else if (strcmp(command, "pwd") == 0) {
				printExec(command);
				printf("\n%s\n", pwd);
			}
			//Checks for list command
			else if (strcmp(command, "list") == 0) {
				printExec(command);
				//if there are no arguments, list current dir. one per line.
          		if (args[1] == NULL)
            		list(pwd);
				//else: list all files for each directory given.
          		else
				{
            		int i = 1;
            		while (args[i])
					{
						//if there is no access error, list for that directory.
              			if (access(args[i], X_OK) == -1)
                			perror("\n Error: ");
              			else
						{
                			printf("\n%s\n", args[i]);
                			list(args[i]);
              			}
						//go to next argument (directory).
              			i++;
            		}	
          		}
			}
			//Checks if command is pid, then prints the process ID.
			else if (strcmp(command, "pid") == 0) {
				printExec(command);
				printf("\nPID: %d\n", getpid());
			}
			else if (strcmp(command, "kill") == 0) {
				if (args[1] == NULL)
					printf("\nNo Argument Given for %s", command);
				else if (args[2] == NULL) {
					int temp = -1;
					sscanf(args[1], "%d", &temp);
					if (temp != -1) {
						if (kill(temp, 15) == -1)
							perror("Error: ");
					} else 
						printf("\nInvalid PID");
				}
			}
			//Checks if it is prompt
			else if (strcmp(command, "prompt") == 0)
			{
				printExec(command);
				//When ran with no arguments, promts for a new prompt prefix string.
				if (args[1] == NULL)
				{
					printf("\n Input Prompt Prefix: ");
					//When given an argument, make that the new prefix prompt.
					if (fgets(promptBuff, PROMPTMAX, stdin) != NULL)
					{
						int len = strlen(promptBuff);
						if (promptBuff[len - 1] == '\n')
							promptBuff[len - 1] = 0;
						strtok(promptBuff, " ");
						strcpy(prompt, promptBuff);
					}
					else
						strcpy(prompt, args[1]);
				}
			}
			//Checks if printenv command.
			else if (strcmp(command, "printenv") == 0)
			{
				printExec(command);
				//When ran with no arguments, prints the whole environment.
				if (args[1] == NULL)
					printenv(enviro);
				//If one argument, call getenv(3) on it.
				else if (args[2] == NULL)
					printf("\n%s\n", getenv(args[1]));
				//two or more arguments, produce error message.
				else
					printf("\nprintenv: Too Many Arguments\n");
			}


			//Command is setenv
			else if (strcmp(command, "setenv") == 0)
			{
				printExec(command);
          		//If no arguments, print the whole environment.
				if (args[1] == NULL)
            		printenv(enviro);
				//Must make sure a second argument is given for PATH or HOME.
          		else if(args[2] == NULL && (strcmp(args[1], "PATH") == 0 || strcmp(args[1], "HOME") == 0))
            		printf("\nDo not set PATH or HOME to empty\n");
          		else if (args[2] == NULL)
				{
            		if (setenv(args[1], "", 1) == -1)
              			perror("Error: ");
          		}
				else if (args[3] == NULL)
				{
            		if (setenv(args[1], args[2], 1) == -1)
              			perror("Error: ");
            		else
					{
              			//Update linked list path directories.
						if (strcmp(args[1], "PATH") == 0)
						{
                			deletepath(&pathlist);
                			pathlist = NULL;
              			}
						//CD to new home.
              			if (strcmp(args[1], "HOME") == 0)
                			homedir = args[2];
            		}
          		}
				else printf("\nError setenv: Too Many Arguments\n");
      }
			else if (args[3] = NULL) {
				int sig = 0;
				int temp = -1;
				sscanf(args[1], "%d", &sig);
				sscanf(args[2], "%d", &sig);
				if (sig < 0 && temp != - 1) {
					if (sig == -1 && temp == getpid()) {
						deletepath(&pathlist);
						free(args);
						free(commandline);
						free(owd);
						free(prompt);
						free(pwd);
						pathlist = NULL;
					}
					if (kill (temp, abs(sig)) == -1)
						perror("Error: ");

				} else
					printf("\n Invalid arguments for %s", command);
			}
		}
		/*  else  program to exec */
		/* find it */
		/* do fork(), execve() and waitpid() */

		/* else */
		/* fprintf(stderr, "%s: Command not found.\n", args[0]); */
	}
	deletepath(&pathlist);
	free(args);
	free(commandline);
	free(owd);
	free(prompt);
	free(pwd);
	pathlist = NULL;
	exit(0);
	return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist )
{
	/* loop through pathlist until finding command and return it.  Return
	   NULL when not found. */
	char buff[MAXIMUM];
	while (pathlist)
	{
		snprintf(buff, MAXIMUM, "%s/%s", pathlist->element, command);
		if (access(buff, X_OK) == -1)
			pathlist = pathlist->next;
		else
		{
			int len = strlen(buff);
			char *ret = calloc(len+1, sizeof(char));
			strncpy(ret, buff, len);
			return ret;
		}
	}
	return NULL;
} /* which() */

char *where(char *command, struct pathelement *pathlist )
{
	char buff[MAXIMUM];
	char *ret;
	int flag = 0;
	while (pathlist) {
		snprintf(buff, MAXIMUM, "%s/%s", pathlist->element, command);
		if (access(buff, X_OK) == -1)
			pathlist = pathlist->next;
		else if (access(buff, X_OK) != -1 && flag == 0) {
			flag = 1;
			int len = strlen(buff);
			ret = calloc(len+1, sizeof(char));
			strncpy(ret, buff, len);
			printf("\n%s", ret);
			pathlist = pathlist->next;
		} else if (access(buff, X_OK) != -1) {
			printf("\n%s", buff);
			pathlist = pathlist->next;
		}
	}
	return ret;
	/* similarly loop through finding all locations of command */
} /* where() */

void list ( char *dir )
{
	/* see man page for opendir() and readdir() and print out filenames for
	   the directory passed */
	DIR *userdir = opendir(dir);
	struct dirent *file;
	if (userdir) {
		while ((file = readdir(userdir)) != NULL) 
			printf("%s\n", file->d_name);
	}
	closedir(userdir);
} /* list() */

void printenv(char ** envp) {
	int i = 0;
  while(envp[i]!=NULL){
    printf("%s\n",envp[i]);
    i++;
  }
}

void printExec(char * command) {
	printf("Executing %s\n", command);
}

int findWildcard(char wc, char **args) {
	int i = 0;
	char *j;
	while (args[i]) {
		j = strchr(args[i], wc);
		if (j != NULL)
			return i;
		i++;
	}
	return -1;
}
