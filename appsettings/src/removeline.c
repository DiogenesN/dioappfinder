#include <stdio.h>

void remove_line_number(char *textfile, int linenumber) {
	FILE *originalFile;
	FILE *newFile;
	const char *tempFilename = ".renamefiletemp~";
	int lineCount = 0;
	char line[1024];

	// Open the original file for reading
	originalFile = fopen(textfile, "r");
	if (originalFile == NULL) {
		perror("Error opening the original file");
		return;
	}

	// Open the new temporary file for writing
	newFile = fopen(tempFilename, "w");
	if (newFile == NULL) {
		perror("Error opening the new file");
		fclose(originalFile); // Close the original file
		return;
	}

	// Copy all lines except the second line
	while (fgets(line, sizeof(line), originalFile) != NULL) {
		lineCount++;
		if (lineCount != linenumber) {
			fputs(line, newFile);
		}
	}

	// Close both files
	fclose(originalFile);
	fclose(newFile);

	// Rename the temporary file to the original file
	if (rename(tempFilename, textfile) != 0) {
		perror("Error renaming the temporary file");
		return;
	}

	printf("line %d was removed from: %s\n", linenumber, textfile);
}
