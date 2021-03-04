#include <bcm2835.h>  
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <pigpio.h>

#include "leak_gpio.h"

// ---------------------------------
uint8_t setup_leak(struct Leak_t *leak_p)  
{ 
    if (gpioInitialise() < 0)
    {
      perror ("Error opening pigpio!");
      return 0;
    }
    
    for(int i=0; i<sizeof(leak_p->gpio_num); i++ ) {
      gpioSetMode(leak_p->gpio_num[i], PI_INPUT);
      gpioSetPullUpDown(leak_p->gpio_num[i], PI_PUD_OFF);  // Clear pull-ups/downs.
    }

    return 1;
}

// ---------------------------------
uint8_t read_leak(struct Leak_t *leak_p)  
{ 
    //Read data

    for(int i=0; i<sizeof(leak_p->gpio_num); i++ ) {
      leak_p->state[i] = gpioRead(leak_p->gpio_num[i]);
    }

    return 1; 
}


// ---------------------------------
void close_gpio() {    
  gpioTerminate();
} 