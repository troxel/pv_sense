#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "1wire.h"

char reading_fspec[100];

// ------------------------------------------------------------- 
int read_eds(char * basedevicedir, struct MRead_t * mread ) {

    FILE *fptr;

    if( access( basedevicedir, F_OK ) != 0 ) { return 0; }

    for( uint8_t i = 0; i < sizeof(mread->names); i++ ) {

        strcpy(reading_fspec,basedevicedir);
        strcat(reading_fspec, mread->names[i]);
        if( access( reading_fspec, F_OK ) != 0 ) { continue; }

        if ( (fptr = fopen (reading_fspec, "r" ) ) == NULL ) {
            printf("cannot open %s\n",reading_fspec); 
            continue;
        }

        fscanf(fptr, "%f", &(mread->data[i]) );
        fclose(fptr);
    }

    return 1; 
}