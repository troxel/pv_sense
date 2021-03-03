#ifndef OWIRE_H
#define OWIRE_H

#include <unistd.h>
#include <stdint.h>
    
struct MRead_t {
    char ** names;
    float * data;          
};

int read_eds(char * basedir, struct MRead_t * mread);

#endif