#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FILE_PATH "/var/www/html/interface/main_FIFO"

char input_string[200] = {0};

int main(int argc, char **argv)
{
    int fifo_file;
    
    if(argc ==  2) {
        printf("New input! text: %s, size: %d\n", argv[1], strlen(argv[1]));
        
        fifo_file = open(FILE_PATH, O_WRONLY, 0x0);
        strcat(input_string, argv[1]);
        write(fifo_file, input_string, 200);
        
        close(fifo_file);
        
    } else {
        printf("No Function...Closing\n");
    }
    
    return 0;
}


