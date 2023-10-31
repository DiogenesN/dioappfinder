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
void process_desktop_file(const gchar *filename) {
	gchar buffer[256];
	gchar *searchName = "Name";
	gchar *searchExec = "Exec";
	gchar *searchIcon = "Icon";
	gint  counter_name = 0;
	gint  counter_exec = 0;
	gint  counter_icon = 0;
	gchar nameOfApp[256] = "Unknown Name";
	gchar execOfApp[256] = "No Exec";
	gchar iconOfApp[256] = "default-application";
	
	FILE *textFile = fopen(filename, "r");

	if (textFile == NULL) {
		perror("Error opening file");
		return;
	}

	while (fgets(buffer, sizeof(buffer), textFile) != NULL) {
	gchar *pos = strchr(buffer, '='); // h0lds str after = sign
		if (g_strrstr(buffer, searchName) != NULL && // if current line contains Name=
			g_strrstr(buffer, "#") == NULL && // and the line doesn't start with #
			g_strrstr(buffer, "Name[") == NULL && // and it doesn't start with Name[
			g_strrstr(buffer, "GenericName") == NULL && // and it doesn't start with GenericName 
			counter_name < 1) { // and it finds only the first occurence of Name

			counter_name = counter_name + 1;
			if (pos != NULL) {
				//prgintf("pos + 1: %c\n", pos[1]);
				if (isspace(pos[1])) {
					pos++;
					strcpy(nameOfApp, pos + 1); // copies the contenr after = sign
					nameOfApp[strlen(nameOfApp) - 1] = '\0'; // Remove newline 
				}
				else {
					strcpy(nameOfApp, pos + 1); // copies the contenr after = sign
					nameOfApp[strlen(nameOfApp) - 1] = '\0'; // Remove newline 
					
				}
			}
		}
		if (g_strrstr(buffer, searchExec) != NULL && \
			g_strrstr(buffer, "#") == NULL && \
			g_strrstr(buffer, "TryExec") == NULL && \
			g_strrstr(buffer, "#Exec") == NULL && \
			counter_exec < 1) {

			counter_exec = counter_exec + 1;
			if (pos != NULL) {
				// Find the '%' gcharacter in the string
				// removes '%' and what comes after it because e.g. thunar %U won't run
				gchar *percentPos = strchr(pos, '%');
				gchar *quatePos = strchr(pos, '"');
				if (percentPos != NULL && quatePos == NULL) {
					// Truncate the string at the '%' gcharacter
					*percentPos = '\0';
				}
				
				//prgintf("pos + 1: %c\n", pos[1]);
				if (isspace(pos[1])) {
					pos++;
					strcpy(execOfApp, pos + 1);
					execOfApp[strlen(execOfApp) + 1] = '\0';
				}
				else {
					strcpy(execOfApp, pos + 1);
					execOfApp[strlen(execOfApp) - 1] = '\0';
				}
			}
		}

		if (g_strrstr(buffer, searchIcon) != NULL && \
			g_strrstr(buffer, "#") == NULL && \
			counter_icon < 1) {

			counter_icon = counter_icon + 1;
			if (pos != NULL) {
				strcpy(iconOfApp, pos + 1);
				iconOfApp[strlen(iconOfApp) - 1] = '\0';
			}
		}
	}

	fclose(textFile);

	*names[strings_counter] = g_strdup(nameOfApp);
	*execs[strings_counter] = g_strdup(execOfApp);
	*icons[strings_counter] = g_strdup(iconOfApp);

	// popilating the paths with complete path to desktop file
	*paths[strings_counter] = g_strdup(filename);

	// getting rid of nasty whitespace that doesn't get the icon properly if file contains Icon = ...
	gchar nameOfAppNowhite[256];
	const gchar *ptrNameIndex = *names[strings_counter];

	while (isspace(*ptrNameIndex)) {
		ptrNameIndex++;
	}

	strcpy(nameOfAppNowhite, ptrNameIndex);

	// Store the name in the first column
	names_and_indexes[strings_counter][0] = (char *)malloc(strlen(nameOfApp) + 1);
	snprintf(names_and_indexes[strings_counter][0], strlen(nameOfApp) + 1, "%s", nameOfAppNowhite);

	// Store the index in the second column
	names_and_indexes[strings_counter][1] = (char *)malloc(sizeof(strings_counter));
	snprintf(names_and_indexes[strings_counter][1], sizeof(strings_counter), "%d", strings_counter);
}


// getting one .desktop file at a time from the given directory path
void process_directory(const gchar *directoryPath) {
	DIR *dirPath = opendir(directoryPath);
	if (dirPath == NULL) {
		perror("Error opening directory");
		path_error = 0;
		return;
	}

	struct dirent *listDirFiles;

	while ((listDirFiles = readdir(dirPath)) != NULL) {
			gchar filepath[333];

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
