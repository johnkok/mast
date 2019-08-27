#ifndef __GPS_HPP
#define __GPS_HPP

#include <thread>
#include <thread>
#include <iostream>
#include <thread>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sstream>

class gps {
        std::thread gps_thread;
        int uart_fd;
        /* static function */
        int set_interface_attribs (int fd, int speed, int parity);
        void set_blocking (int fd, int should_block);
    public:
        /* public function */
        int Initialize(char *);
    
};

typedef struct NavRelposned {
    unsigned char version;
    unsigned char reserved1;
    unsigned short int refStationId;
    unsigned int iTOW;
    int relPosN;
    int relPosE;
    int relPosD;
    char relPosHPN;
    char relPosHPE;
    char relPosHPD;
    unsigned char reserved2;
    unsigned int accN;
    unsigned int accE;
    unsigned int accD;
    unsigned int flags;
}__attribute__((packed)) NavRelposned_t;

#endif // __GPS_HPP
