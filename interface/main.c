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
#define TIMEOUT_TIME 20
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
#define GPS_FILE_PATH "/var/www/html/interface/GPS_text_FIFO"
#define REQUEST_GPS_FILE_PATH "/var/www/html/interface/request_GPS_data_FIFO"
#define SEND_GPS_FILE_PATH "/var/www/html/interface/send_home_GPS_data_FIFO"
#define MAIN_FIFO_PATH "/var/www/html/interface/main_FIFO"

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
         while(1){
		 check_input();
		 check_task();
		 process_task();
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
	//if it's not zero, timeout is occuring. Check web input quickly, then keep getting GPS data.
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
			sscanf(web_string, "%d_%d_%d_%d_%s", &dummy, &f_lower_bound, &f_upper_bound, &bin_size, location_description); 
                //print the string received from the FIFO
                printf("printing received web string: %s\n",web_string);
	}
	
	}
	
// look at the global string, web_command and decide what task to do next.
// sent value codes: 1=start one shot mode, 2=start continuous mode, 3=stop,
// 4=hackRF is done, 5=hackRF is stuck.
void check_task(void){
	
	printf("In check_task function\n");
	input_counter=0;
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
		
		
		//if location string is not empty
		if(location_description[0] != '\0') {
			use_location_flag = TRUE;
			//printf("something is in the location string! it works!");
			//printf("%s", location_description);
		}
		//set flag to use location as sweep file name as TRUE
	}
	if(web_string[input_counter]==50) //start continuous sweep mode?
	{
		
	    start_hackRF=TRUE;
	    get_gps_data=TRUE;
		continuous_mode = TRUE;
		coord_timeout = FALSE;
		printf("start continuous sweep mode condition true\n");
		
		
		//if location string is not empty
		if(location_description[0] != '\0') {
			use_location_flag = TRUE;
			//printf("something is in the location string! it works!");
			//printf("%s", location_description);
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
	
	}
	if(web_string[input_counter]==52) //if true, hackRF is done
	{
		if(timeout_active){
		kill(pid_timeout, SIGKILL);//kill timeout child
		timeout_active=FALSE;
		
		}
		if(continuous_mode==TRUE){
			start_hackRF=TRUE;
			get_gps_data=TRUE;
			printf("restarted hack_rf\n");
		}
		else {
			use_location_flag = FALSE;
			memset(location_description,0,sizeof(location_description));
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
			get_gps_data=TRUE; // get new GPS data
			coord_timeout=FALSE; 
			printf("hack rf ran when timout hit\n");
		}
		printf("hackRF stuck condition true\n");
	}
	
	
}

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

void timeout(void){
	int fifo_file;
	sleep(TIMEOUT_TIME);
	fifo_file = open(MAIN_FIFO_PATH, O_WRONLY, 0x0);
	write(fifo_file,"5",1);
	close(fifo_file);
	exit(EXIT_SUCCESS);
}

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
	if(use_location_flag != TRUE) {
	sprintf(system_command, "%s -a 0 -f %d:%d -1 32 -g 32 -w %d -r %f_%f.txt", &hack_command[0], f_lower_bound, f_upper_bound, bin_size, lat, lon);
	printf("checking system command: %s\n",system_command);
	system(system_command);
	}
	
	// hackRF system call for location description as text file title
	else {
		sprintf(system_command, "%s -a 0 -f %d:%d -1 32 -g 32 -w %d -r %s.txt", &hack_command[0], f_lower_bound, f_upper_bound, bin_size, location_description);
	printf("checking system command: %s\n",system_command);
	system(system_command);
	}
	//sleep(1); TESTING ONLY!!!
	fifo_file = open(MAIN_FIFO_PATH, O_WRONLY, 0x0); 
	write(fifo_file,"4",1);
	close(fifo_file);
	exit(EXIT_SUCCESS);
}


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
	//char file[60] = {GPS_FILE_PATH};
	
	
	sprintf(gps_system_call, "python gpsdData.py >> %s&", GPS_FILE_PATH);
	printf("system command is: %s\n",gps_system_call);
	system(gps_system_call);
	fifo_file = open(GPS_FILE_PATH, O_RDONLY, 0x0);
	while(1){
		num=0;
		num1=0;
		lat=0.0;
		lon=0.0;
		//printf("looking for gps input data in child\n");
	memset(input_string,0,sizeof(input_string));
	read(fifo_file, input_string, 1000);
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
	if(num>0 && num1>0 && lon>-128 && lat>25 && lon<-50 && lat<45) {
		//printf("found lon/lat in child\n");
		//printf("lat: %f... lon: %f... \n",lat, lon);
		//fifo_file1 = open(REQUEST_GPS_FILE_PATH, O_RDONLY | O_NONBLOCK, 0x0);
		//printf("child id %d\n", fifo_file1);
		//if(fifo_file1 >= 0){ //opened FIFO to read, parent opened to write request
			//printf("found request in child\n");
			//close(fifo_file1);
			sprintf(coord_string, "%f_%f", lat, lon);
			fifo_file2 = open(SEND_GPS_FILE_PATH, O_WRONLY | O_NONBLOCK, 0x0);
			write(fifo_file2, coord_string, 40);
			close(fifo_file2);
		//}
		
		//printf("Count: %d, lat: %f, lon: %f\n",num, lat, lon);
	}
	//else {
		//printf("no_data\n");
	close(fifo_file);
	fifo_file = open(GPS_FILE_PATH, O_RDONLY, 0x0);
}
exit(EXIT_SUCCESS);
}

void get_coordinates(void){
	//int fifo_file=0;
	int fifo_file1=0;
	char received_coordinates[40] = {0};
	//fifo_file = open(REQUEST_GPS_FILE_PATH, O_WRONLY, 0x0);
	//close(fifo_file);
	// start gps_timeout_child
	pid_gps_timeout = fork();
	if(pid_gps_timeout == FALSE){
		gps_timeout();
	}
	fifo_file1 = open(SEND_GPS_FILE_PATH, O_RDONLY, 0x0);
	read(fifo_file1, received_coordinates, 40);
	close(fifo_file1);
	sscanf(received_coordinates, "%f_%f", &lat, &lon);	//receive lat and lon value into floats
	if((lat == 0.0) && (lon == 0.0)){
		coord_timeout = coord_timeout + 1;
	}
	else{
		kill(pid_gps_timeout, SIGKILL);//kill gps_timeout
		coord_timeout = FALSE;
	}
}
