#include <stdio.h>
#include <dirent.h>
#include <string.h>

void select_sort( char names[1024][128], int size ) {
	int i, j, k;
	char temp[128] = { 0 };

	for (i=0; i<(size-1); i++) {
		k = i;
		for (j=i+1; j<size; j++) {
			if (strcmp(names[k], names[j]) > 0)
				k = j;
		}
		if (k != i) {
			strcpy(temp, names[k]);
			strcpy(names[k], names[i]);
			strcpy(names[i], temp);
		}
	}
}

int main() {
	DIR *d;
	struct dirent *dp;
	char names[1024][128] = { 0 };
	int i, j;

	d = opendir(".");
	for (dp = readdir(d), i=0; dp != NULL; dp = readdir(d), i++) {
		sprintf(names[i], "%-20s %s", dp->d_name, (dp->d_type == DT_DIR) ? "DIR" : "" );
	}
	closedir(d);
	select_sort(names, i);
	for (j=0; j<i; j++) {
		printf("%s\n", names[j]);
	}
	return 0;
}

