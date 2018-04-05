#include <stdio.h>
void find_latitude(){
	
	FILE* read_file;
//char data = 0;
char data_array[500]; //The array that will hold the shell script command
read_file = fopen("~/var/www/html/interface/gpstext.txt", "rb");

//data = getc(read_file);
fscanf(read_fil "%10[^\n]", data_array); //Put the shell command into data_array

//printf("data: %c\n", data);
printf("string: %s\n", &data_array[0]);
	
	
}
