#ifndef __MAST_HPP
#define __MAST_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>

#define MAX_WIND_SPEED_DEPTH 600
#define MAX_WIND_DIRECTION_DEPTH 100

/**
 *  Global structures
 */
typedef struct gps_global_data {
    std::string longitude;
    std::string latitude;
    std::string time;
    float cog;
    float sog;
}gps_global_data_t;

typedef struct gyro_data {
    float roll;
    float pitch;
    float yaw;
}gyro_data_t;

typedef struct mast_axis {
    float current;
    float low;
    float high;
}mast_axis_t;

typedef struct wind_data {
    std::vector<float> speed;     // Windex speed queue
    std::vector<float> direction; // Windex direction queue
    float var_low;    // Wind variation low angle
    float var_high;   // Wind variation high angle
    float gust;       // Wind gust speed
    float TWA;        // True wind angle
    float TWS;        // True wind speed
    float TWD;        // True wind direction
    float AWA;        // Apparent wind angle
    float AWS;        // Apparent wind speed
    float AWD;        // Apparent wind direction
}wind_data_t;

class cmast {
        gps_global_data_t GpsData;
        gyro_data_t GyroData;
        mast_axis_t MastAxis[2];
        wind_data_t WindData;
    public:
        cmast();
        ~cmast();
        void update_wind(float direction, float speed);
        void update_speed(float cog, float sog) {
            cmast::GpsData.cog = cog;
            cmast::GpsData.sog = sog;
        }
        void update_position (std::string time, std::string longitude, std::string lattitude) {
            cmast::GpsData.time = time;
            cmast::GpsData.longitude = longitude;
            cmast::GpsData.latitude = lattitude;
        }
        float get_wind_var_low(void) {
            return cmast::WindData.var_low;
        }
        float get_wind_var_high(void) {
            return cmast::WindData.var_high;
        }
        std::string get_time(void) {
            return cmast::GpsData.time;
        }
        std::string get_longitude(void) {
            return cmast::GpsData.longitude;
        }
        std::string get_lattitude(void) {
            return cmast::GpsData.latitude;
        }
        float get_cog(void) {
            return cmast::GpsData.cog;
        }
        float get_sog(void) {
            return cmast::GpsData.sog;
        }
        float get_tws(void) {
            return cmast::WindData.TWS;
        }
        float get_twd(void) {
            return cmast::WindData.TWD;
        }
        float get_twa(void) {
            return cmast::WindData.TWA;
        }
        float get_aws(void) {
            return cmast::WindData.AWS;
        }
        float get_awa(void) {
            return cmast::WindData.AWA;
        }
        float get_awd(void) {
            return cmast::WindData.AWD;
        }
};
#endif
