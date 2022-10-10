#include <glob.h>
#include "get_path.h"

int pid; //added extern
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void printenv(char **envp);
void execCommand(char *command, char **args, int status);
void printExec(char * command);
int findWildcard(char wc, char **args);
void runGlob(int index, char *commandpath, struct pathelement *pathlist, char **args, glob_t globIn, int status);

#define PROMPTMAX 32
#define MAXARGS 10
#define MAXIMUM 128
