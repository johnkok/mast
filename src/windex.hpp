#ifndef __WINDEX_HPP
#define __WINDEX_HPP

#include <thread>
#include <iostream>
#include <thread>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>

class windex {
        std::thread windex_thread;
        int uart_fd;
        /* static function */
        int set_interface_attribs (int fd, int speed, int parity);
        void set_blocking (int fd, int should_block);
    public:
        /* public function */
        int Initialize(char *);
    
};

#endif
