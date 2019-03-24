#include <stdio.h>
#include <stdlib.h>	//to use system()
#include <string.h> //to use strcpy()

int getNumberOfLines(char *filename)
{
    char command[100];
    sprintf(command, "wc -l %s > temp.txt", filename);
    system(command);

    FILE *fp = fopen("temp.txt", "r");
    int total_len;
    char *x;
    fscanf(fp, "%d %s", &total_len, x);

    return total_len;
}

int main()
{
	int len = getNumberOfLines ("makefile.c");
    printf("%d" ,len);
	
	return 0;
}