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
	int uid, i, status, argsct, go = 1;
	struct passwd *password_entry;
	char *homedir;
	struct pathelement *pathlist;
	char buff[MAXIMUM];

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
			else if (strcmp(command, "pwd") == 0) {
				printExec(command);
				printf("\n%s\n", pwd);
			}
			/*  else  program to exec */
			/* find it */
			/* do fork(), execve() and waitpid() */

			/* else */
			/* fprintf(stderr, "%s: Command not found.\n", args[0]); */
		}
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
} /* list() */

void printExec(char * command) {
	printf("Executing %s\n", command);
}
