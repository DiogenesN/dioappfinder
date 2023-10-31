/*
 * this function is used insread of system()
 * credits to Eric Ma, systutorials.
 */

#include <spawn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "posixspawn.h"

extern char **environ;

void run_cmd(char *cmd) {
	pid_t pid;
	char *argv[] = {"sh", "-c", cmd, NULL};
	int status;
	printf("Run command in the background: %s\n", cmd);

	// Append "&" to the command
	size_t cmd_length = strlen(cmd);
	if (cmd_length > 0 && cmd[cmd_length - 1] != '&') {
		strcat(cmd, " &");
	}

	status = posix_spawn(&pid, "/bin/sh", NULL, NULL, argv, environ);

	if (status == 0) {
		printf("Child pid: %i\n", pid);
	}
	else {
		printf("posix_spawn: %s\n", strerror(status));
	}
}
