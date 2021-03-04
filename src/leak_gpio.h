
#ifndef GPIO_H
#define GPIO_H

struct Leak_t {
    char ** names;
    uint8_t * gpio_num;
    uint8_t * state;
};

// ---------------------------------
uint8_t setup_leak(struct Leak_t *leak_p);  
uint8_t read_leak(struct Leak_t *leak_p);  
void close_gpio();     

#endif