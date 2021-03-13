#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/un.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <sys/file.h>

#include <stdint.h>

#include "ad_i2c.h"
#include "1wire.h"
#include "leak_gpio.h"


// If changing EDS unit need to update this... 
#define BaseDeviceDir "/mnt/1wire/7E.303900001000/EDS0068/"

// --------------------------------
void intHandler(int sig)
{ 
  printf("Stopping\n");  
  close_ad();
  close_gpio();  
  exit(0);
}
// ------------------------------

int main(int argc, char *argv[])
{

    signal(SIGINT, intHandler); 
    signal(SIGTERM, intHandler); 

    int pidfile = open("/run/pv_sense.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pidfile, LOCK_EX | LOCK_NB);
    if(rc) {
        if(EWOULDBLOCK == errno) {
            printf("\nAnother instance of pv_sense is running\n");
            printf("Kill it first before starting a new one.\n"); 
            printf("hint: >killall pv_sense.exe\n\n");
            exit(0);
        }
    }

    // --- Declare and initialize data structures --
    struct MRead_t pth;
        
    char * pth_names[] = {
            "pressure",
            "temperature",
            "humidity",
            "dew_point",
        };
    
    float data[4] = {0,0,0,0};

    pth.names = pth_names; 
    pth.data = data; 

    struct Leak_t leak; 
    char * leak_names[] = { "lvl0","lvl1","lvl2","lvl3" };
    uint8_t leak_gpio_num[4] = {6,24,23,25};
    uint8_t leak_state[4] = {0,0,0,0};

    leak.names = leak_names;
    leak.state = leak_state;
    leak.gpio_num = leak_gpio_num;
    // -----------------------------


    // --- Data file --
    FILE * dataf;
    dataf = fopen ("/var/data/pv","a");
    if (dataf == NULL) perror ("Error opening data file");
    // -----------------

    char pth_str[50]; 
    char leak_str[50]; 
    char dataline_str[150];    

    if (! setup_ad() ) { printf("Cannot open lvl AD\n"); return(1); }  
    if (! setup_leak(&leak) ) { printf("Cannot open leak gpio\n"); return(1); }      
 
    while(1) {

        uint16_t lvl0 = read_lvl(0x1);
        printf("lvl0=%d ",lvl0);
       
        if ( read_eds(BaseDeviceDir, &pth) ) {
             
            strcpy(pth_str,"");
            for (int i=0;i<sizeof(pth.data);i++) {
                printf("%s=%5.2f ",pth.names[i],pth.data[i]);
                sprintf(&pth_str[strlen(pth_str)], "%5.2f ", pth.data[i]);  // fancy strcat
            }
        }

	//Leak sensors are marked 1-4
	//Leak 1: at bottom position 0% fill capacity
	//Leak 2: at 0.5 inches up, ~5% fill capacity
	//Leak 3: at 0.75 inches up, ~10% fill capacity 
        //Leak 4: at 1.6 inches up, ~25% fill capacity
        if ( read_leak(&leak) ) {
            strcpy(leak_str,"");
            printf("Leaks: ");
            for (int i=0;i<sizeof(leak.state);i++) {

                printf("%1u ",leak.state[i]);
                sprintf(&leak_str[strlen(leak_str)], "%1u ", leak.state[i]);

            }
        }
   
        printf("\n");

        strcpy(dataline_str,"");
        sprintf(dataline_str,"%u %u %s %s", (unsigned)time(NULL), lvl0, pth_str, leak_str);
        fprintf(dataf,"%s\n",dataline_str);
        fflush(dataf);

        sleep(4);
    }

    return(1);

}

