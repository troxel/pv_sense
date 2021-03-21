#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/un.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <sys/file.h>
#include <ctype.h>
#include <libgen.h>

#include <stdint.h>

#include "ad_i2c.h"
#include "1wire.h"
#include "leak_gpio.h"

// If changing EDS unit need to update this... 
#define BaseDeviceDir "/mnt/1wire/7E.303900001000/EDS0068/"

char pid_fspec[] = "/run/pv_sense.pid";

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
    signal(SIGSTOP, intHandler); 
    signal(SIGHUP, intHandler); 
    signal(SIGKILL, intHandler); 
 
    // ---- Allow only one instance to run... kill off older instances ----        
    int pidfile_fd = open(pid_fspec, O_CREAT | O_RDWR, 0666);
    if( pidfile_fd <= 0 ) {
        perror("Error open : ");
        printf("Error open : file %s\n",pid_fspec);
        return(-1);
    }

    // -------------------------------------
    // Only run one instance of this program 
    // -------------------------------------
    while(1)
    {
        int rc = flock(pidfile_fd, LOCK_EX | LOCK_NB);
        //printf("file lock return --> %d\n",rc);
        if( rc == -1 ) {

            if(EWOULDBLOCK == errno) {
                // Another instance is running
                char pid_str[7];
                int rtn = read(pidfile_fd, pid_str, sizeof(pid_str) - 1);
                if ( rtn == - 1) {
                    printf("Cannot read PID File %s\n",pid_fspec);
                    exit(0);
                }

                char command[30];
                // kill older instances
                strcpy( command, "pkill -o pv_sense.exe" );
                printf("\nkilling process %s\n\n",command);
                system(command);
         
            }
            else {
                perror("Unanticipated flock Error: ");
            }
        }
        else {
            int pid = getpid();
            printf("Process starting pid = %d\n",pid);
            break; 
        }
        sleep(2);
    }

    // -------------------------------------
    // Options and defaults 
    // -------------------------------------
    unsigned int loop_sleep = 900; // seconds 
    uint8_t verbose_flag = 0;
    int copt; 
    while ( (copt = getopt (argc, argv, "vs:") ) != -1) {
        switch (copt)
            {
            case 'v':
                verbose_flag = 1;
                loop_sleep = 4; 
                break;
            case 's':
                loop_sleep = atoi(optarg);
                break;
            case '?':
                if (optopt == 's')
                fprintf (stderr, "Option -%c requires a sleep time argument.\n", optopt);
                else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                fprintf (stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
                return 1;
            default:
                abort();
            }
    }

    if ( !verbose_flag ) {
        printf("Running in quiet mode. Use -v option to print to stdout\n");
    }
    printf("Loop time is %d seconds. Use -s $num option to change\n\n",loop_sleep);

 
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
    dataf = fopen ("/var/log/pv","a");
    if (dataf == NULL) perror ("Error opening data file");
    // -----------------

    char pth_str[50]; 
    char pth_name_str[100]; 
    char leak_str[50]; 
    char dataline_str[150];    

    time_t tm_now;
    char tm_str[30];

    if (! setup_ad() ) { printf("Cannot open lvl AD\n"); return(1); }  
    if (! setup_leak(&leak) ) { printf("Cannot open leak gpio\n"); return(1); }      
 
    // ----------------------------------
    // ---------- MAIN Sense Loop ------------
    // ----------------------------------
    while(1) {

        uint16_t lvl0 = read_lvl(0x1);
     
        if ( read_eds(BaseDeviceDir, &pth) ) {
             
            strcpy(pth_str,"");
            strcpy(pth_name_str,"");
            for (int i=0;i<sizeof(pth.data);i++) {
                sprintf(&pth_name_str[strlen(pth_name_str)], "%s=%5.2f ", pth.names[i],pth.data[i]);  // fancy strcat
                sprintf(&pth_str[strlen(pth_str)], "%5.2f ", pth.data[i]);  
            }
        }

	    //Leak sensors are marked 1-4
	    //Leak 1: at bottom position 0% fill capacity
	    //Leak 2: at 0.5 inches up, ~5% fill capacity
	    //Leak 3: at 0.75 inches up, ~10% fill capacity 
        //Leak 4: at 1.6 inches up, ~25% fill capacity
        if ( read_leak(&leak) ) {
            strcpy(leak_str,"");           
            for (int i=0;i<sizeof(leak.state);i++) {

                //printf("%1u ",leak.state[i]);
                sprintf(&leak_str[strlen(leak_str)], "%1u ", leak.state[i]);

            }
        }
   
        tm_now = time(NULL);

        if ( verbose_flag ) {
            strftime(tm_str, 26, "%m/%d %H:%M:%S", localtime(&tm_now));
            printf("%s lvl0=%u %s Leaks=%s\n",tm_str, lvl0, pth_name_str, leak_str);
        }

        // --- Data file --
        FILE * dataf;
        dataf = fopen ("/var/log/pv","a");
        if (dataf == NULL) {
            perror ("Error opening data file");
        }
        else {
            strcpy(dataline_str,"");
            sprintf(dataline_str,"%u %u %s %s", (unsigned)tm_now, lvl0, pth_str, leak_str);
            fprintf(dataf,"%s\n",dataline_str);
            fclose(dataf);
        }

        sleep(loop_sleep);
    }

    return(1);

}

