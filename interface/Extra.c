

#include <stdio.h>

int main(int argc, char **argv)
{
	
	
	return 0;
}

void  GPS_child_process(void)
{
     int   i;
	system("python gpsdData.py >> GPS_text_FIFO&");
    printf("   This line is from child, value = %d\n", i);
    fifo_file = open(FILE_PATH, O_RDONLY, 0x0);
	while(1){
		num=0;
		num1=0;
		lat=0.0;
		lon=0.0;
	memset(input_string,0,sizeof(input_string));
	found_data = read(fifo_file, input_string, 1000);
	ss = strstr(input_string, "latitude");
	if(ss==NULL) {
		
	}
	else{
		num = sscanf(ss+8, "%f", &lat);
		ss = strstr(input_string, "longitude");
		if(ss==NULL) {
		
	}
	else{
		num1 = sscanf(ss+9, "%f", &lon);
	}
	}
	
// if num is greater than 0 (meaning it found a latitude, print
// if no num found, show line it didn't find one in
	if(num>0 && num1>0 && lon!=0.0 && lat!=0.0) printf("Count: %d, lat: %f, lon: %f\n",num, lat, lon);
	//else {
		//printf("no_data\n");

	//printf("my strin: %s\n", input_string);
	//sleep(1);
	close(fifo_file);
	fifo_file = open(FILE_PATH, O_RDONLY, 0x0);
}
	return 0;
}
