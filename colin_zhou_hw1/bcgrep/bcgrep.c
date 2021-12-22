#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

	char *str;
	size_t sizeStr = 50;

	str = (char*) malloc(sizeStr);

	if (argc == 1) {
		printf("%s", "wgrep: searchterm [file ...]\n");
		exit(1);
	}

	FILE *fp = fopen(argv[2], "r");

	if (fp == NULL) {
		printf("%s", "cannot open file\n");
		exit(1);
	}

	while(getline(&str, &sizeStr, fp) != -1) {
		if (strstr(str, argv[1]) != NULL) {
			printf("%s", str);
		}
	}

	fclose(fp);
	free(str);

	return 0;

}
