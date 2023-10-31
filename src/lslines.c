#include <stdio.h>

// returns the number of lines in a text file

int lslines(const char *filename) {
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		perror("Error opening the file");
		return -1;  // Return -1 to indicate an error
	}

	int lineCount = 0;
	char ch;
	while ((ch = fgetc(file)) != EOF) {
		if (ch == '\n') {
			lineCount++;
		}
	}

	fclose(file);
	return lineCount;
}
