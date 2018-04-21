#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

// Defined Macros
#define   MAX_COUNT  3
#define TIMEOUT_TIME 10
#define GPS_TIMEOUT_TIME 6
#define MAX_GPS_TIMEOUT 8
#define	  TRUE	1
#define   FALSE 0

// Function Calls
void check_input(void);
void check_task(void);
void process_task(void);
void timeout(void);
void hackRF_start(void);
void GPS_child_process(void);
void get_coordinates(void);
void write_webpage(int);

//FIFO declarations
#define GPS_FILE_PATH "/var/www/html/interface/GPS_text_FIFO" //GPS_FILE_PATH FIFO is used to pipe the output of the GPS python script to the GPS_child_process function.
#define REQUEST_GPS_FILE_PATH "/var/www/html/interface/request_GPS_data_FIFO"
#define SEND_GPS_FILE_PATH "/var/www/html/interface/send_home_GPS_data_FIFO" //SEND_GPS_FILE_PATH FIFO is for sending the received GPS coordinates in the GPS_child_process function
// to the get_coordinates function. Also, the gps_timeout function uses this FIFO for sending in the invalid coordinates to increment the coord_timeout counter.
#define MAIN_FIFO_PATH "/var/www/html/interface/main_FIFO" //Main_FIFO_PATH is the pipe for the webpage to talk to the pi. The commands ID's, frequency parameters, bin width, 
//and location description all come through this FIFO. Also, this FIFO is used for the hackRF timeout to send a command saying it is stuck. Also, the hackRF child process
// uses MAIN_FIFO_PATH to write that the hackRF is done sweeping.

//Global Variables
char web_string[60] ={0};
char location_description[40] ={0};
char start_hackRF=0;
char get_gps_data=0;
char stop_hackRF=0;
char hackRF_running=0;
char continuous_mode=0;
char hackRF_done=0;
char timeout_active=0;
char coord_timeout = FALSE;
char use_location_flag;
int input_counter = 0;
int dummy = 0;
int f_lower_bound = 0;
int f_upper_bound = 0;
int bin_size =0;
int name_flag=0;
float lat=0.0;
float lon=0.0;
pid_t  pid_timeout;
pid_t  pid_gps;
pid_t  pid_hackRF;
pid_t  pid_gps_timeout;

int main(int argc, char **argv)
{

	
	
	
	system("pgrep python | xargs sudo kill -9"); //kill the pre existing GPS python programs running - we want only one running.
	printf("about to fork\n");
	pid_gps = fork();	//start timeout child
		if(pid_gps==FALSE){
			printf("right before gps child call\n");
			GPS_child_process();
		}
     else {
		 printf("right before parent main\n");
         while(1){	//Main while loop. 
		 check_input(); //Check input commands from the web
		 check_task(); //Verify which command is received through it's ID code
		 process_task(); //Do we have valid coordinates? If so, run hackRF.
	 }
     //printf("*** Parent is done ***\n");
	 }
	

	return 0;
}


    // if the received string is "one_shot", 
    void check_input(void){
	int fifo_file;
	printf("In check_input function\n");
	memset(web_string,0,sizeof(web_string));	//clear the storage string before data comes in
	if(coord_timeout){ //is coordinate timeout not zero? 
	//if it's not zero, timeout is occurring. Check web input quickly, then keep getting GPS data.
		printf("coord_timeout is true! \n");
		fifo_file = open(MAIN_FIFO_PATH, O_RDONLY | O_NONBLOCK, 0x0);
		sleep(1); // u sleep is microsecond sleep
		read(fifo_file, web_string, 60);
		printf("web_string is: %s\n",web_string);
		close(fifo_file);
		sscanf(web_string, "%d_%d_%d_%d", &dummy, &f_lower_bound, &f_upper_bound, &bin_size); 
	} else {
		printf("coord_timeout is false! \n");
		fifo_file = open(MAIN_FIFO_PATH, O_RDONLY, 0x0);
                while(web_string[0] == 0){
                    usleep(100); // u sleep is microsecond sleep
                    read(fifo_file, web_string, 60);
                }
   	        close(fifo_file); 
			sscanf(web_string, "%d_%d_%d_%d_%d_%s", &dummy, &f_lower_bound, &f_upper_bound, &bin_size, &name_flag, location_description); 
                //print the string received from the FIFO
                printf("printing received web string: %s\n",web_string);
                printf("command: %d,  f_low: %d,  f_high: %d, bin_size: %d,  name_flag: %d, location: %s", dummy, f_lower_bound, f_upper_bound, bin_size, name_flag, location_description);
				
	}
	
	}
	
// look at the global string, web_command and decide what task to do next.
// sent value codes: 1=start one shot mode, 2=start continuous mode, 3=stop,
// 4=hackRF is done, 5=hackRF is stuck.
void check_task(void){
	
	printf("In check_task function\n");
	input_counter=0;
	//printf("%d",web_string[input_counter]);
	//while((web_string[input_counter] != 0) || (web_string[input_counter] != '\0')){
		//printf("Checking number: %d, %c\n", input_counter, web_string[input_counter]);
	if(web_string[input_counter]==49) //start one shot sweep mode?
	{
		//timeout();
		start_hackRF=TRUE;
		get_gps_data=TRUE;
		continuous_mode = FALSE;
		coord_timeout = FALSE;
		printf("start one shot mode condition true\n");
		printf("name_flag is: %d",name_flag);
		printf("before write to webpage");
		write_webpage(1);
		printf("after write to webpage");
		if(name_flag==2){	//webpage says we do not have GPS but have a description
				start_hackRF=TRUE;
				get_gps_data=FALSE;
				continuous_mode = FALSE;
				
			
		}
		if(name_flag==1){	//webpage says we have GPS and name description
				start_hackRF=TRUE;
				get_gps_data=TRUE;
				continuous_mode = FALSE;
				coord_timeout = FALSE;
		}
		if(name_flag==3){	//webpage says we have GPS but don't have description
				start_hackRF=TRUE;
				get_gps_data=TRUE;
				continuous_mode = FALSE;
				coord_timeout = FALSE;
		}
		//if location string is not empty
		//if(location_description[0] != '\0') {
			//use_location_flag = TRUE;
			//get_gps_data=FALSE;
			//coord_timeout = FALSE;
			//printf("something is in the location string! it works!");
			//printf("%s", location_description);
		//}
		//set flag to use location as sweep file name as TRUE
	}
	if(web_string[input_counter]==50) //start continuous sweep mode?
	{
		
	    start_hackRF=TRUE;
	    get_gps_data=TRUE;
		continuous_mode = TRUE;
		coord_timeout = FALSE;
		printf("start continuous sweep mode condition true\n");
		write_webpage(2);
		
		if(name_flag==2){	//webpage says we do not have GPS but have a description
				start_hackRF=TRUE;
				get_gps_data=FALSE;
				
				
			
		}
		if(name_flag==1){	//webpage says we have GPS and name description
				start_hackRF=TRUE;
				get_gps_data=TRUE;
				
				coord_timeout = FALSE;
		}
		if(name_flag==3){	//webpage says we have GPS but don't have description
				start_hackRF=TRUE;
				get_gps_data=TRUE;
				
				coord_timeout = FALSE;
		}
		//set flag to use location as sweep file name as TRUE
	}
	if(web_string[input_counter]==51) //stop hackRF?
	{
		if(hackRF_running){
			hackRF_running =FALSE;
			
			//system call?
			printf("Killed Hack_RF\n");
			kill(pid_hackRF, SIGKILL);	//kill hackRF
			system("pgrep hack | xargs sudo kill -INT");
			system("pgrep hack | xargs sudo kill -9");
			
		}
		if(timeout_active){
			kill(pid_timeout, SIGKILL);//kill timeout child
			printf("Killed Timeout");
			timeout_active=FALSE;
		}
		
		//the below two statements are for resetting or not resetting the location description string.
		use_location_flag = FALSE;
		memset(location_description,0,sizeof(location_description));
		//
		
		
		continuous_mode=FALSE;
		get_gps_data=FALSE;
			coord_timeout=FALSE;
			start_hackRF=FALSE;
		printf("stop hackRF condition true\n");
		write_webpage(3); //write to webpage that the sweep has been stopped.
	
	}
	if(web_string[input_counter]==52) //if true, hackRF is done
	{
		if(timeout_active){
		kill(pid_timeout, SIGKILL);//kill timeout child
		timeout_active=FALSE;
		
		}
		if(name_flag==2){	//webpage says we do not have GPS but have a description
				start_hackRF=TRUE;
				get_gps_data=FALSE;
				
				
			
		}
		if(name_flag==1){	//webpage says we have GPS and name description
				start_hackRF=TRUE;
				get_gps_data=TRUE;
				
				coord_timeout = FALSE;
		}
		if(name_flag==3){	//webpage says we have GPS but don't have description
				start_hackRF=TRUE;
				get_gps_data=TRUE;
				
				coord_timeout = FALSE;
		}
		printf("hackRF done condition true\n");
		
	}
	if(web_string[input_counter]==53) //if true, hackRF is stuck
	{
		if(hackRF_running==TRUE){
			timeout_active=FALSE;
			printf("\n\nHACK RF IS RESTARTING\n\n");
			//kill(0, sigkill);//kill(pid_hackRF, SIGTERM);//kill hackRF
			kill(pid_hackRF, SIGKILL);//kill hackRF
			system("pgrep hack | xargs sudo kill -INT");
			system("pgrep hack | xargs sudo kill -9");
			
			sleep(3);
			start_hackRF=TRUE; // if it get's stuck, restart it
			if(location_description[0] != '\0') {
			use_location_flag = TRUE;
			get_gps_data=FALSE;
			//printf("something is in the location string! it works!");
			//printf("%s", location_description);
		} else {
			get_gps_data=TRUE; // get new GPS data
		}
			coord_timeout=FALSE; 
			printf("hack rf ran when timout hit\n");
		}
		printf("hackRF stuck condition true\n");
		write_webpage(4); //write to webpage that the HackRF has stalled.
	}
	
	
}

//---The process_task function first checks to see if we have valid coordinates through the get_coordinates function. If we don't have valid coordinates, declare a GPS timeout.
// If we do have valid coordinates, run the hackRF child process and the hackRF timeout function.
void process_task(void){
	printf("In check_process function\n");
	if(get_gps_data==TRUE){ // IF true, we are wanting GPS coordinates.
		printf("waiting to get gps data\n");
		get_coordinates();// get valid or invalid GPS data
		if(coord_timeout == FALSE){ //if so, received valid GPS coordinates! Move on
			get_gps_data = FALSE;
		} else {	//if we didn't receive valid GPS coordinates
			if(coord_timeout < MAX_GPS_TIMEOUT){	// Have we tried the maximum # of attempts before we say the GPS timed out?
				printf("GPS Timeout - Trying Again\n"); //If we haven't tried the maximum number of attempts, say we will retry finding GPS coordinates.
			} else {	//At this point, we tried the max amount of attempts to get GPS coordinates. Quit the sweep.
				printf("Max GPS Timeout Reached... Clearing Run\n");
				get_gps_data = FALSE;
				start_hackRF = FALSE;
				coord_timeout = FALSE;
			}
		}
	}
	if((start_hackRF==TRUE) && (coord_timeout == FALSE)){ // If we want to start hackRF and we got GPS coordinates
		start_hackRF=FALSE;	//Clear hackRF start since we are now engaged in starting hack RF
		timeout_active=TRUE;	//Start hackRF timeout function. Do not want hackRF running forever.
		printf("starting hackRF timeout function\n");
		pid_timeout = fork();	//start hack RF timeout child process
		if(pid_timeout==FALSE){	//If we successfully started hackRF timeout child, child runs the timeout function.
			timeout();
		}
		hackRF_running=TRUE;
		printf("starting hackrf function\n");
		pid_hackRF = fork();	//start timeout child
		if(pid_hackRF==FALSE){	//child process to run hackRF sweep command
			hackRF_start();
		}//start hackrf child
	}
}

//---This function is the timeout function for the hackRF. It is called upon receiving GPS coordinates and starting the hackRF child process. 
// This function sleeps for a set TIMEOUT_TIME, when that timeout time is complete, a "stuck" code, 5, is sent through the MAIN_FIFO_PATH to indicate that the
// hack RF is stalled.
void timeout(void){	
	int fifo_file;
	sleep(TIMEOUT_TIME);
	fifo_file = open(MAIN_FIFO_PATH, O_WRONLY, 0x0);
	write(fifo_file,"5",1);
	close(fifo_file);
	exit(EXIT_SUCCESS);
}

//---gps_timeout function is for sending an invalid GPS coordinate set to get_coordinates. Once a set amount of invalid coordinates is read, we will declare the GPS timed out.
void gps_timeout(void){
	int fifo_file;
	sleep(GPS_TIMEOUT_TIME);
	fifo_file = open(SEND_GPS_FILE_PATH, O_WRONLY, 0x0);
	write(fifo_file,"0.0_0.0",7);
	close(fifo_file);
	exit(EXIT_SUCCESS);
}

void hackRF_start(void){
	int fifo_file=0;
	// use lat/lon and hack_rf string to make system call!
	// call hack_rf system call
	
	char hack_command[50] = {"hackrf_sweep"};
	char system_command[500];
	//int binwidth=100000;
	//int f_lower_boundary=1;
	//int f_higher_boundary=6000;
	
	system("pgrep hack | xargs sudo kill -INT");
	
	// hackRF system call for GPS coordinates as text file title
	if(name_flag==3) {
	sprintf(system_command, "%s -a 0 -f %d:%d -1 32 -g 32 -w %d -r /home/pi/Desktop/SweepData/%f_%f.txt", &hack_command[0], f_lower_bound, f_upper_bound, bin_size, lat, lon);
	printf("checking system command: %s\n",system_command);
	system(system_command);
	}
	
	// hackRF system call for location description as text file title
	if(name_flag==2) {
		sprintf(system_command, "%s -a 0 -f %d:%d -1 32 -g 32 -w %d -r /home/pi/Desktop/SweepData/%s.txt", &hack_command[0], f_lower_bound, f_upper_bound, bin_size, location_description);
	printf("checking system command: %s\n",system_command);
	system(system_command);
	}
	if(name_flag==1) {
	sprintf(system_command, "%s -a 0 -f %d:%d -1 32 -g 32 -w %d -r /home/pi/Desktop/SweepData/%s_%f_%f.txt", &hack_command[0], f_lower_bound, f_upper_bound, bin_size, location_description, lat, lon);
	printf("checking system command: %s\n",system_command);
	system(system_command);
	}
	//sleep(1); TESTING ONLY!!!
	fifo_file = open(MAIN_FIFO_PATH, O_WRONLY, 0x0); //Write to the MAIN_FIFO_PATH so that check_task function sees that the hackRF is finished sweeping.
	write(fifo_file,"4",1);
	close(fifo_file);
	exit(EXIT_SUCCESS);
}


//--- The GPS_child_process function starts the gpsdData.py file and outputs that GPS data into the GPS_FILE_PATH FIFO. Then, the GPS_FILE_PATH FIFO is opened and hangs at
// the open command until coordinates come through it. When valid coordinates are received, the GPS output is read into input_string. The input_string is searched for 
// latitude and longitude data. 
void GPS_child_process(void){
	int fifo_file;
	int fifo_file2;
	char input_string[1000] = {0};
	int num=0;
	int num1=0;
	char *ss;
	char coord_string[40]={0};
	float lat;
	float lon;
	char gps_system_call[60] = {0};

	
	
	sprintf(gps_system_call, "python gpsdData.py >> %s&", GPS_FILE_PATH);	//check and prepare the run GPS call
	printf("system command is: %s\n",gps_system_call);
	system(gps_system_call); //Run the GPS system call
	fifo_file = open(GPS_FILE_PATH, O_RDONLY, 0x0); //open and wait for data to come in from the GPS.
	while(1){	//This function stays in this while loop continuously in order to always be getting fresh GPS data.
		num=0;
		num1=0;
		lat=0.0;
		lon=0.0;
		//printf("looking for gps input data in child\n");
	memset(input_string,0,sizeof(input_string));	//clear the input_string of any garbage
	read(fifo_file, input_string, 1000);	//read data from the GPS_FILE_PATH FIFO into input_string
	ss = strstr(input_string, "latitude"); //Find the index position of where latitude is
	if(ss==NULL) {
		
	}
	else{	//when the index of latitude is found
		num = sscanf(ss+8, "%f", &lat);	//scan the value of latitude into the lat variable
		ss = strstr(input_string, "longitude");	//Find the index of the longitude value
		if(ss==NULL) {
			
		}
		else{
			num1 = sscanf(ss+9, "%f", &lon);	//read the longitude value into lon
		}
	}

	if(num>0 && num1>0 && lon>-128 && lat>25 && lon<-50 && lat<45) {	//This is a condition to make sure we have valid latitude and longitude coordinates for North America

			sprintf(coord_string, "%f_%f", lat, lon);	//prepare a string with latitude and longitude separated by _
			fifo_file2 = open(SEND_GPS_FILE_PATH, O_WRONLY | O_NONBLOCK, 0x0);	//Open the SEND_GPS_FILE_PATH FIFO to send lat and lon
			write(fifo_file2, coord_string, 40); //Send the coord_string containing the latitude_longitude
			close(fifo_file2);
			write_webpage(5);
		}

	close(fifo_file);	//Close the FIFO to refresh for next GPS coordinate
	fifo_file = open(GPS_FILE_PATH, O_RDONLY, 0x0);	//Open the GPS_FILE_PATH FIFO to receive next coordinate. Hang until the data comes in.
}
exit(EXIT_SUCCESS);
}

//--- The get_coordinates function reads in the received coordinates from the GPS_child_process. If they are valid, store them in global variables and kill this child process.
// if they are not valid, increment the coord_timeout. Once the coord_timeout variable is incremented a set amount, we will declare coordinate timeout.
void get_coordinates(void){
	int fifo_file1=0;
	char received_coordinates[40] = {0};
	// start gps_timeout_child
	pid_gps_timeout = fork();	//start the gps_timeout function.
	if(pid_gps_timeout == FALSE){
		gps_timeout();
	}
	fifo_file1 = open(SEND_GPS_FILE_PATH, O_RDONLY, 0x0);	//Open the SEND_GPS_FILE_PATH FIFO to get the received coordinates from the GPS_child_process function.
	read(fifo_file1, received_coordinates, 40);	//Read in the coordinates
	close(fifo_file1);
	sscanf(received_coordinates, "%f_%f", &lat, &lon);	//receive lat and lon value into floats that can be used.
	if((lat == 0.0) && (lon == 0.0)){	//If the latitude and longitude value are 0... increment coordinate timeout. 
		coord_timeout = coord_timeout + 1;
	}
	else{
		kill(pid_gps_timeout, SIGKILL);// the else means that the latitude and longitude values are valid, so kill the get_coordinates function and move onto sweeping.
		coord_timeout = FALSE;	//We got valid coordinates. Do not set coord_timeout as true.
	}
}

void write_webpage(int code){
	FILE *fp;
	printf("in write function!");
	printf("%d", code);
	if(code == 1){	//running single sweep
		printf("I should be writing");
		fp = fopen("/var/www/html/interface/test.txt", "w+");
		fputs("Running single sweep\n", fp);
		fclose(fp);
	}
	if(code == 2){	//running continuous sweep
		printf("I should be writing");
		fp = fopen("/var/www/html/interface/test.txt", "w+");
		fputs("Running continuous sweep\n", fp);
		fclose(fp);
	}
	if(code == 3){	//Sweep has been stopped
		printf("I should be writing");
		fp = fopen("/var/www/html/interface/test.txt", "w+");
		fputs("Stopped sweep\n", fp);
		fclose(fp);
	}
	if(code == 4){
		printf("I should be writing");
		fp = fopen("/var/www/html/interface/test.txt", "w+");
		fputs("One moment please - HackRF stalling\n", fp);
		fclose(fp);
	}
	if(code == 5){
		printf("I should be writing");
		fp = fopen("/var/www/html/interface/gps.txt", "w+");
		fputs("GPS connected\n", fp);
		fclose(fp);
	}
	
		
}   
