#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{

char hack_command[500] = {"hackrf_sweep"};
	char hack_file_name[500] = {"FileName"};
	double number_0 = 100;
	double number_1 = 59;
	double number_2 = 22;
	char system_command[500];
	
	
	sprintf(system_command, "%s -f %s.%f.%f.%f.txt", &hack_command[0], &hack_file_name[0], number_0, number_1, number_2);
	printf("%s\n", &system_command[0]);

	return 0;
}
