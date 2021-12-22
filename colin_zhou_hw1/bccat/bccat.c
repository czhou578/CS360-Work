#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	char str[100];

	FILE *fp = fopen(argv[1], "r");

	if (argc == 1) {
		exit(0);
	}

	if (fp == NULL) {
		printf("cannot open file\n");
		exit(1);
	}

	if (fgets(str, 100, fp) == NULL) {
		printf("wcat: cannot open file\n");
		exit(1);
	}

	while (fgets(str, 100, fp) != NULL) {
		printf("%s", str);
	}

	fclose(fp);

	return 0;

}
