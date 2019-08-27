#include "windex.hpp"
#include "mast.hpp"

using namespace std;

extern cmast Mast;

void wthread (int fd)
{
    char buffer[1024];
    unsigned char crc;
    int n, i;
    string data;

    cout << " *  Windex thread started" << endl;

    while (1) {

        n = read (fd, buffer, sizeof buffer);  // read up to 100 characters if ready to read

        /* Check for communication error */
        if (n < 0) {
            cout << "[*] Windex communication error!"<< endl;
            break;
        }
        /* Data exist */
        if (n < 7) {
            continue;
        }

        /* Validata packet */
        buffer[n] = 0;
        data = buffer;
        if (!data.compare(0, 6, "$WIMWV")) { // find wind NMEA message only
            /* calculate message CRC */
            for (i = 1, crc = 0 ; i < n ; i++) {
                if (buffer[i] == '*') {
                    break;
                }
                crc ^= buffer[i];
            }

            /* Sting to text */
            string rcv_crc = data.substr(data.find('*') + 1, 2);
            unsigned char crc_i;
            try {
                crc_i = stoi(rcv_crc, 0, 16);
            }
            catch (const exception& e) {
                cout << "<*> Invalid windex data" << endl;
                continue;
            }
            if (crc == crc_i) { /* check CRC */
                float direction;
                float speed;
                int end = data.find('*');
                int start = data.find(',');
                try {
                    data = data.substr(start + 1, end - start - 5);
                    direction = stof(data.substr(0, data.find(',')), 0);
                    data = data.substr(data.find(',') + 3, data.size() - data.find(',') - 3);
                    speed = stof(data.substr(0, data.find(',')), 0);
                }
                catch (const exception& e) {
                    cout << "<*> Invalid windex data" << endl;
                    continue;
                }

                /* set values */
                Mast.update_wind(direction, speed);
            }
        }
    }
}

int windex::set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0) {
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 10;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0) {
            cout << "[*]Attributes not set" << endl;
            return -1;
        }
        return 0;
}

void windex::set_blocking (int fd, int should_block)
{
    struct termios tty;

    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0) {
        cout << "[*] Attributes not set" << endl;
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0) {
        cout << "[*] Attributes not set" << endl;
    }
}

int windex::Initialize(char *portname) 
{

    cout << " *  Initializing windex " << portname << endl;

    /* open uart device */
    uart_fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (uart_fd < 0) {
        cout << "[*] Port " << portname << " not found" << endl;
        return -1;
    }

    /* set baudrate */
    set_interface_attribs (uart_fd, B4800, 0);  /* NMEA standart: 4800bps*/
    set_blocking (uart_fd, 1);                 

    /* create thread */
    windex_thread = thread(wthread, uart_fd); 

    return 0;
}


