/* creates initial config directory and file */

#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "configsgen.h"

void create_configs() {
	const char *HOME = getenv("HOME");

	if (HOME == NULL) {
        fprintf(stderr, "Unable to determine the user's home directory.\n");
        return;
    }

	const char  *configDirPath = "/.config/dioappfinder";
	const char  *nameOfConfig = "/dioappfinder.conf";
	char 		fullPath[256];
	char 		fullPathWithName[256];

    snprintf(fullPath, sizeof(fullPath), "%s%s", HOME, configDirPath);
    snprintf(fullPathWithName, sizeof(fullPathWithName), "%s%s%s", HOME, configDirPath, nameOfConfig);

	DIR *dioappfinder = opendir(fullPath);

	// cheks if the file already exists
	if (dioappfinder) {
		// directory exists
		closedir(dioappfinder);
		return;
	}
	else if (errno == ENOENT) {
    	// directory does not exist
		printf("Created initial config files in %s/.config/dioappfinder\n", HOME);
		mkdir(fullPath, 0755);
		
		//char *path  = "/home/diogenes/.local/share/applications";
		//char *path2 = "/usr/local/share/applications";
		char *path3 = "/usr/share/applications";

		FILE *confFile = fopen(fullPathWithName, "w+");
		//fprintf(confFile, "%s\n%s\n%s\n", path, path2, path3);
		fprintf(confFile, "%s\n", path3);
		fclose(confFile);
	}
	else {
    	perror("Access error!");
	}
	closedir(dioappfinder);
}
