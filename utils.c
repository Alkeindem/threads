#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"


// Input: char* (string) str: argument of the -c flag.
// Function: Validates the input and converts it to int.
// Output: int: -1 if input is not valid, int converted str if valid.
int validateCFlag(char* str)
{
	if (isNumeric(str) == -1)
		return -1;

	int num = atoi(str);

	if (num == 0)
		return -1;

	return num;
}

// Input: char* (string) str: argument of the -u flag.
// Function: Validates the input and converts it to int.
// Output: int: -1 if input is not valid, int converted str if valid.
void readKernelFile(char* str, double kernel[3][3])
{

	FILE* file;
	char c;
	char element[8];
	int strIndex = 0;

	file = fopen(str, "r");
	if (file == NULL)
	{
		printf("File could not be opened.\n");
		exit(1);
	}

	for(int j = 0; j < 3; j++)
	{
		int i = 0;

		for(c = fgetc(file); (c != EOF); c = fgetc(file))
		{

			if (c == '\n')
			{
				element[strIndex] = '\0';
				kernel[j][i] = atof(element);
				strIndex = 0;
				break;
			}

			else if (c == ' ')
			{
				element[strIndex] = '\0';
				kernel[j][i] = atof(element);
				strIndex = 0;
				i++;
			}

			else
			{	
				element[strIndex] = c;
				strIndex++;
			}
		}
	}

	kernel[2][2] = atof(element);

	fclose(file);
}

// Input: char* (string) str: argument of the -n flag.
// Function: Validates the input and converts it to int.
// Output: int: -1 if input is not valid, int converted str if valid.
int validateNFlag(char* str)
{
	if (isNumeric(str) == -1)
		return -1;

	int num = atoi(str);

	if ((num < 0) || (num > 100))
		return -1;

	return num;
}

// Input: char* (string) str: argument of the -h flag.
// Function: Validates the input and converts it to int.
// Output: int: -1 if input is not valid, int converted str if valid.
int validateHFlag(char* str)
{
	if (isNumeric(str) == -1)
		return -1;

	int num = atoi(str);

	if (num <= 0)
		return -1;

	return num;
}

// Input: char* (string) str: argument of the -t flag.
// Function: Validates the input and converts it to int.
// Output: int: -1 if input is not valid, int converted str if valid.
int validateTFlag(char* str)
{
	if (isNumeric(str) == -1)
		return -1;

	int num = atoi(str);

	if (num <= 0)
		return -1;

	return num;
}

// Input: char* (string) str.
// Function: Checks if the input is a number.
// Output: int: -1 if input is not a number, 1 if number.
int isNumeric(char* str)
{
	int len = strlen(str);

	for (int i = 0; i < len; i++)
	{
		if (isdigit(str[i]) == 0)
			return -1;
	}

	return 1;
}