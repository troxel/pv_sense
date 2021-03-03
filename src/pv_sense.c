#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/un.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include <stdint.h>

#include "ad_i2c.h"
#include "1wire.h"

// If changing EDS unit need to update this... 
#define BaseDeviceDir "/mnt/1wire/7E.303900001000/EDS0068/"

// --------------------------------
void SIGINT_HDL(int sig)
{
  close_ad();  
  exit(1);
}
// ------------------------------

int main(int argc, char *argv[])
{

    signal(SIGINT, SIGINT_HDL); 

    // --- Declare EDS data structure --
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
    // -----------------------------

    if (! setup_ad() ) { printf("Cannot open lvl AD\n"); exit(-1); }  

    while(1) {

        uint16_t lvl0 = read_lvl(0x1);
        printf("lvl0= %d ",lvl0);

        if ( read_eds(BaseDeviceDir, &pth) ) {

             for (int i=0;i<sizeof(pth.data);i++) {
                printf("%s=%5.2f ",pth.names[i],pth.data[i]);
            }
        }
   
        printf("\n");

        sleep(2);
    }

    return(1);

}

/*

       if ( read_eds(BaseDeviceDir, &pth) ) {
           
        } else {
            printf("cannot read owfs\n"); 
        }
*/
