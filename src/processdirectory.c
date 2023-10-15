#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "externvars.h"

// takes a /path/name.desktop as argument and getting the Name=, Exec=, Icon= strings
void process_desktop_file(const char *filename) {
	char buffer[256];
	char nameOfApp[256];
	char execOfApp[256];
	char iconOfApp[256];
	char *searchName = "Name";
	char *searchExec = "Exec";
	char *searchIcon = "Icon";
	int  counter_name = 0;
	int  counter_exec = 0;
	int  counter_icon = 0;
	
	FILE *textFile = fopen(filename, "r");

	if (textFile == NULL) {
		perror("Error opening file");
		return;
	}

	while (fgets(buffer, sizeof(buffer), textFile) != NULL) {
	char *pos = strchr(buffer, '='); // h0lds str after = sign
		if (strstr(buffer, searchName) != NULL && // if current line contains Name=
			strstr(buffer, "#") == NULL && // and the line doesn't start with #
			strstr(buffer, "Name[") == NULL && // and it doesn't start with Name[
			strstr(buffer, "GenericName") == NULL && // and it doesn't start with GenericName 
			counter_name < 1) { // and it finds only the first occurence of Name

			counter_name = counter_name + 1;
			if (pos != NULL) {
				strcpy(nameOfApp, pos + 1); // copies the contenr after = sign
				nameOfApp[strlen(nameOfApp) - 1] = '\0'; // Remove newline 
			}
		}
		if (strstr(buffer, searchExec) != NULL && \
			strstr(buffer, "#") == NULL && \
			strstr(buffer, "TryExec") == NULL && \
			strstr(buffer, "#Exec") == NULL && \
			counter_exec < 1) {

			counter_exec = counter_exec + 1;
			if (pos != NULL) {
				// Find the '%' character in the string
				// removes '%' and what comes after it because e.g. thunar %U won't run
				char *percentPos = strchr(pos, '%');
				char *quatePos = strchr(pos, '"');
				if (percentPos != NULL && quatePos == NULL) {
					// Truncate the string at the '%' character
					*percentPos = '\0';
				}
				
				//printf("pos + 1: %c\n", pos[1]);
				if (isspace(pos[1])) {
					pos++;
					strcpy(execOfApp, pos + 1);
				}
				else {
					strcpy(execOfApp, pos + 1);
					execOfApp[strlen(execOfApp) - 1] = '\0';
				}
			}
		}
		if (strstr(buffer, searchIcon) != NULL && \
			strstr(buffer, "#") == NULL && \
			counter_icon < 1) {

			counter_icon = counter_icon + 1;
			if (pos != NULL) {
				strcpy(iconOfApp, pos + 1);
				iconOfApp[strlen(iconOfApp) - 1] = '\0';
			}
		}
	}

	fclose(textFile);

	*names[strings_counter] = strdup(nameOfApp);
	*execs[strings_counter] = strdup(execOfApp);
	*icons[strings_counter] = strdup(iconOfApp);
}


// getting one .desktop file at a time from the given directory path
void process_directory(const char *directoryPath) {
	DIR *dirPath = opendir(directoryPath);
	if (dirPath == NULL) {
		perror("Error opening directory");
		path_error = 0;
		return;
	}

	struct dirent *listDirFiles;

	while ((listDirFiles = readdir(dirPath)) != NULL) {
			char filepath[333];

		if (listDirFiles->d_type == DT_REG && // DT_REG selecting only regular text files
			strcmp(listDirFiles->d_name + strlen(listDirFiles->d_name) - strlen(".desktop"), \
																			".desktop") == 0) {

			snprintf(filepath, sizeof(filepath), "%s/%s", directoryPath, listDirFiles->d_name);

			// calling the process_desktop_file func with /path/name.desktop
			process_desktop_file(filepath);
			
			// this is used as num in *names[num] = strdup(nameOfApp)
			strings_counter = strings_counter + 1;
		}
	}
	closedir(dirPath);
}
