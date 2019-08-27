#include "gps.hpp"
#include "mast.hpp"

using namespace std;

extern cmast Mast;

int ublox_crc(unsigned char *ublox, int size)
{
    int i;
    unsigned char ck_a = 0;
    unsigned char ck_b = 0;

    for (i = 2 ; i < (size - 2) ; i++) {
        ck_a = ck_a + ublox[i];
        ck_b = ck_a + ck_b;
    }
    if (ck_a == ublox[size - 2] && ck_b == ublox[size - 1]) {
        return 0;
    }
    cout << "<*> Invalid u-blox CRC: " << int(ck_a) << " " << int(ck_b) << " - " << int(ublox[size-2]) << " " << int(ublox[size-1]) << endl;
    return 1;
}

int ublox_message(unsigned char ublox[], int size)
{
    // Check CRC
    if (ublox_crc(ublox, size)) {
        return 0;
    }

    switch (ublox[2]) {
        case (0x01):
            switch (ublox[3]) {
                case (0x3C): {// NAV-RELPOSNED
                    if (ublox[6] != 0x00 /* version*/ ||
                       (ublox[4] + (ublox[5] << 8)) != 40) {
                        cout << " *  U-Blox NAV RELPOSNED invalid version" << endl;
                        break;
                    }
                    struct NavRelposned *navrelposned = (struct NavRelposned *)&ublox[6];
                    if ((navrelposned->flags & 0x01) && /* GNSS fix */
                        (navrelposned->flags & 0x04)) { /* Rel pos valid */




                        cout << " *  U-Blox valid relative position" << endl;
                        cout << "E: " << navrelposned->relPosE << " " << int(navrelposned->relPosHPE) << " Acc " <<  navrelposned->accE<< endl;
                        cout << "N: " << navrelposned->relPosN << " " << int(navrelposned->relPosHPN) << " Acc " <<  navrelposned->accN<< endl;
                        cout << "D: " << navrelposned->relPosD << " " << int(navrelposned->relPosHPD) << " Acc " <<  navrelposned->accD<< endl;



                    }
                    break;
                }
                default:
                    cout << " *  U-Blox NAV Message " << int(ublox[3]) << endl;
            }
            break;
        case (0x02):
            cout << " *  U-Blox RXM Message" << endl;
            break;
        case (0x04):
            switch (ublox[3]) {
                case (0x04): {
                    string log_msg((char *)&ublox[5], size - 8);
                    cout << " *  U-Blox INF-DEBUG Message: " << log_msg << endl;
                    break;
                    }
                case (0x00): {
                    string log_msg((char *)&ublox[5], (int)ublox[4]);
                    cout << " *  U-Blox INF-ERROR Message: " << log_msg << endl;
                    break;
                    }
                case (0x02): {
                    string log_msg((char *)&ublox[5], (int)ublox[4]);
                    cout << " *  U-Blox INF-NOTICE Message: " << log_msg << endl;
                    break;
                    }
                case (0x03): {
                    string log_msg((char *)&ublox[5], (int)ublox[4]);
                    cout << " *  U-Blox INF-TEST Message: " << log_msg << endl;
                    break;
                    }
                case (0x01): {
                    string log_msg((char *)&ublox[5], (int)ublox[4]);
                    cout << " *  U-Blox INF-WARNING Message: " << log_msg << endl;
                    break;
                    }
                default:
                    cout << "<*> U-Blox unknown INF Message" << endl;
            }
            break;
        case (0x05):
            cout << " *  U-Blox ACK Message" << endl;
            break;
        case (0x06):
            cout << " *  U-Blox CFG Message" << endl;
            break;
        case (0x09):
            cout << " *  U-Blox UPD Message" << endl;
            break;
        case (0x0A):
            cout << " *  U-Blox MON Message" << endl;
            break;
        case (0x0B):
            cout << " *  U-Blox AID Message" << endl;
            break;
        case (0x0D):
            cout << " *  U-Blox TIM Message" << endl;
            break;
        case (0x10):
            cout << " *  U-Blox ESF Message" << endl;
            break;
        case (0x13):
            cout << " *  U-Blox MGA Message" << endl;
            break;
        case (0x21):
            cout << " *  U-Blox LOG Message" << endl;
            break;
        case (0x27):
            cout << " *  U-Blox SEC Message" << endl;
            break;
        case (0x28):
            cout << " *  U-Blox HNR Message" << endl;
            break;
        default:
            cout << "<*> U-Blox Unknown Message" << endl;
    }
    return size;
}

int nmea_crc(unsigned char *nmea, int size)
{
    int i;
    unsigned char crc = 0;
    char cal_crc[3];

    for(i = 1 ; i < (size - 3) ; i++) {
        crc = crc^nmea[i];
    }

    string rcv_crc((char *)&nmea[size-2], 2);
    snprintf(cal_crc, 3, "%X", crc);
    string str_crc(cal_crc);

    if(rcv_crc.compare(str_crc)) {
        cout << "<*> NMEA invalid CRC: " << rcv_crc << " " << cal_crc << endl;
        return 1;
    }
    return 0;
}

int nmea_message(unsigned char *nmea, int size)
{

    // Check CRC
    if (nmea_crc(nmea, size)) {
        return 0;
    }

    std::string nmea_str((char *)(nmea), size);
    if (!nmea_str.compare(0, 6, "$GPGSV")) {
        /* Ignore */
    }
    else if (!nmea_str.compare(0, 6, "$GLGSV")) {
        /* Ignore */
    }
    else if (!nmea_str.compare(0, 6, "$GNRMC")) {
        /* Ignore */
    }
    else if (!nmea_str.compare(0, 6, "$GNVTG")) {
        int pos, next;
        float cog = 0;
        float sog = 0;

        pos = nmea_str.find_first_of(',', 7);
        if (pos < size && pos > 8){
            cog = stof(nmea_str.substr(7, pos - 7));
        }
        pos += 6;
        next = nmea_str.find_first_of(',', pos);
        if (next < size && next > pos){
            sog = stof(nmea_str.substr(pos, pos - next));
        }
        Mast.update_speed(cog, sog);
    }
    else if (!nmea_str.compare(0, 6, "$GNGGA")) {
        int pos, next;
        string hours, minutes, seconds;
        string long_deg, long_pri, long_sec, long_dir;
        string lat_deg, lat_pri, lat_sec, lat_dir;

        pos = nmea_str.find_first_of(',', 7);
        if (pos < size && pos > 8){
            hours = nmea_str.substr(7, 2);
            minutes = nmea_str.substr(9, 2);
            seconds = nmea_str.substr(11, 2);
        }
        pos++;
        next = nmea_str.find_first_of(',', pos);
        if (next < size && next > pos){
            long_deg = nmea_str.substr(pos, 2);
            long_pri = nmea_str.substr(pos+2, 2);
            long_sec = std::to_string(stoi(nmea_str.substr(pos+5, 3)) * 60 / 1000);
        }
        pos = next+1;
        next = nmea_str.find_first_of(',', pos);
        if (next < size && next > pos){
            long_dir = nmea_str.substr(pos, 1);
        }
        pos = next+1;
        next = nmea_str.find_first_of(',', pos);
        if (next < size && next > pos){
            lat_deg = nmea_str.substr(pos, 3);
            lat_pri = nmea_str.substr(pos+3, 2);
            lat_sec = std::to_string(stoi(nmea_str.substr(pos+6, 3)) * 60 / 1000);
        }
        pos = next+1;
        next = nmea_str.find_first_of(',', pos);
        if (next < size && next > pos){
            lat_dir = nmea_str.substr(pos, 1);
        }
        Mast.update_position( hours + ":" + minutes + ":" + seconds,
                              long_deg + "\u00B0" + long_pri + "'" + long_sec + "\"" + long_dir,
                              lat_deg + "\u00B0" + lat_pri + "'" + lat_sec + "\"" + lat_dir);
    }
    else if (!nmea_str.compare(0, 6, "$GNGSA")) {
        /* Ignore */
    }
    else if (!nmea_str.compare(0, 6, "$GNGLL")) {
        /* Ignore */
    }
    else {
        cout << "NMEA Message: " << nmea_str << endl;
    }

    return size;
}

void gthread (int fd)
{
    unsigned char buffer[2048];
    int n, i, y;

    cout << " *  GPS thread started" << endl;

    while (1) {

        n = read (fd, buffer, sizeof buffer);

        /* Check for communication error */
        if (n < 0) {
            cout << "[*] GPS communication error!"<< endl;
            break;
        }

        if (n) {
            for(i = 0 ; i < n ; i++) {
                if(buffer[i] == '$') { // NMEA start character
                    for(y = i ; y < n ; y++) {
                        if (buffer[y] == '*') {
                            break;
                        }
                    }
                    if (n <= (y + 2)) {
                        break;
                    }
                    i += nmea_message(&buffer[i], y + 2 - i + 1);
                }
                else if ( buffer[i] == 0xB5 && buffer[i + 1] == 0x62){ // u-blox start character
                    if (n <= (i + 6)) {
                        break;
                    }
                    int size = buffer[i+4] + (buffer[i+5] << 8);
                    if ((size + i + 8) > n) {
                        break;
                    }
                    i += ublox_message(&buffer[i], size + 8);
                }
            }
        }
    }
}

int gps::set_interface_attribs (int fd, int speed, int parity)
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
        tty.c_cc[VMIN]  = 0.1;            // read doesn't block
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

void gps::set_blocking (int fd, int should_block)
{
    struct termios tty;

    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0) {
        cout << "[*] Attributes not set" << endl;
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1000 : 0;
    tty.c_cc[VTIME] = 2;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0) {
        cout << "[*] Attributes not set" << endl;
    }
}

int gps::Initialize(char *portname) 
{

    cout << " *  Initializing gps " << portname << endl;

    /* open uart device */
    uart_fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (uart_fd < 0) {
        cout << "[*] Port " << portname << " not found" << endl;
        return -1;
    }

    /* set baudrate */
    set_interface_attribs (uart_fd, B230400, 0);  /* NMEA standart: 4800bps*/
    set_blocking (uart_fd, 1);                 

    /* create thread */
    gps_thread = thread(gthread, uart_fd); 

    return 0;
}


