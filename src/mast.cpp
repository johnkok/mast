#include "graphics.hpp"
#include "mast.hpp"
#include "windex.hpp"
#include "gps.hpp"

using namespace std;

/* Global data */
cmast Mast;
extern graphics Graphics;

void cmast::update_wind(float direction, float speed)
{
    float min = 0;
    float max = 0;
    float var = 0;
    float gust = 0;

    WindData.speed.push_back(speed);
    WindData.direction.push_back(direction);
   
    // limit queue depth
    if (WindData.speed.size() > MAX_WIND_SPEED_DEPTH) {
        WindData.speed.erase(WindData.speed.begin()); // Used fo wind gust
    }
    // limit queue depth
    if (WindData.direction.size() > MAX_WIND_DIRECTION_DEPTH) {
        WindData.direction.erase(WindData.direction.begin()); // Used for direction variance
    }

    for (int i = 0 ; i < WindData.direction.size() ; i++) {
        var = direction - WindData.direction[i];

        if (var > 180)  { var = (var - 360); }
        if (var < -180) { var = (var + 360); }
        if (var > max)  { max = var; }
        if (var < min)  { min = var; }
    }

    min = direction - min;
    max = direction - max;
    // reshape
    if (min > 360) { min = min - 360; }
    if (max > 360) { max = max - 360; }
    if (min < 0)   { min = min + 360; }
    if (max < 0)   { max = max + 360; }

    // find wind gust value
    for (int i = 0 ; i < WindData.speed.size() ; i++) {
        if (WindData.speed[i] > gust) {
            gust = WindData.speed[i];
        }
    }

    // update data
    WindData.gust = gust;
    WindData.var_low = max;
    WindData.var_high = min;
    WindData.AWS = speed;
    WindData.AWD = direction;
}

/* mast constructor */
cmast::cmast(void)
{
    cout << " *  Mast initialized!" << endl;
}

/* mast destructor */
cmast::~cmast(void)
{
    cout << " *  Mast deinitialized!" << endl;
}

int main(int argc, char** argv)
{
    cout << " *  Starting " << argv[0] << " application" << endl;

    /* Initialize windex */
    windex Windex;
    Windex.Initialize("/dev/ttyUSB1");

    /* Initialize GPS */
    gps Gps;
    Gps.Initialize("/dev/ttyUSB0");

    /* Create display */
    Graphics.graphics_main(argc, argv);

    return 0; 
}
