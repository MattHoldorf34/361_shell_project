/*
Thomas Oves and Matthew Holdorf
*/
#include "sh.h"
#include <signal.h>
#include <stdio.h>

void sig_handler(int signal); 

// Main function, runs shell function as well as signals
// Parameters: int- argc, char- **argv, char- **envp
// Returns: int, should be 0 if sh function is successful
int main( int argc, char **argv, char **envp )
{
	// Ctrl click signals
	signal(SIGINT, sig_handler);
	signal(SIGTSTP, sig_handler);
	signal(SIGTERM, SIG_IGN);
	signal(SIGQUIT, sig_handler);
	// Shell function returned
	return sh(argc, argv, envp);
}

// Signal handler function, prints when signals are ignored
// Parameters: int- signal
// Returns nothing
void sig_handler(int signal)
{
	printf("\n Ignored \n");
}
