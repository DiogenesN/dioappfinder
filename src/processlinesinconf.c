#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "processdirectory.h"
#include "processlinesinconf.h"

// processing directories stated in config file
void parse_config() {
	const char *HOME = getenv("HOME");

	if (HOME == NULL) {
        fprintf(stderr, "Unable to determine the user's home directory.\n");
        return;
    }

	const char *configDirPath = "/.config/dioappfinder";
	const char *nameOfConfig = "/dioappfinder.conf";
	char 	   fullPathWithName[256];

	snprintf(fullPathWithName, sizeof(fullPathWithName), "%s%s%s", HOME, configDirPath, nameOfConfig);
	// Open the file for reading
	FILE *file = fopen(fullPathWithName, "r");

	if (file == NULL) {
		perror("Error opening the file");
		return;
	}

	char line[256];     // Buffer for reading lines

	// Read lines from the file and store them in the strings array
	while (fgets(line, sizeof(line), file) != NULL) {
		// Remove the newline character, if present
		char *newline = strchr(line, '\n');
		if (newline != NULL) {
			*newline = '\0';
		}
		int  counter_index = 0;
		char *strings[256];

		// Allocate memory for the string and copy it
		strings[counter_index] = strdup(line);

		// process directories found in config file
		process_directory(strings[counter_index]);
		counter_index++;

		if (counter_index >= 30) {
			break;  // Stop reading after 30 lines
		}

		// Free the allocated memory for each line
		for (int i = 0; i < counter_index; i++) {
			free(strings[i]);
		}
	}
	fclose(file);
}
