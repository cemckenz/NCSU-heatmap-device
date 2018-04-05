#include <stdio.h>
#include <stdlib.h>

float find_longitude(void)
{
	
	FILE* read_file;
	char data_array[500];
	float lon;
	int num = 0;
	char count = 0;
//open gpstext.txt
	read_file = fopen("/home/pi/projects/user_interface/gps_output.txt", "rb");
// for 10 tries (lines), read a line with fgets and look for longitude
	for(count = 0; count < 30; count++){
// read a line from text file and save as string
	fgets(data_array, 500, read_file);
// scan saved string for "longitude and a double/float
	num = sscanf(data_array, " longitude %f", &lon);
// if num is greater than 0 (meaning it found a longitude, print
// if no num found, show line it didn't find one in
	if(num > 0) printf("Count: %d, longitude: %f\n",num, lon);
	//else printf("no lat in string: %s\n", data_array);
}
	
	return lon;
}

