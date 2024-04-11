#ifndef DEFINES
#define DEFINES

// Debug bits
// 0  - nodebug
// 1  - debug MAG values
// 2  - debug MAG raw readings
// 4  - debug MAG Compensation
// 8  - debug untilt acc values
// 16 - debug acc values
// 32 - debug alt
// 64 - debug az
// 128 - debug joystick
#define DEBUG_MAG_RAW   0x01
#define DEBUG_MAG_CAL    0x02
#define DEBUG_MAG_COMP   0x04
#define DEBUG_UNTILT_ACC 0x08
#define DEBUG_ALT_ACC    0x10
#define DEBUG_ALT        0x20
#define DEBUG_AZ         0x40
#define DEBUG_JOYSTICK   0x80

#define DegToRad 0.017453292F
#define RadToDeg 57.295779F

double AZ_CORR = 0;  // calculated in untilter.h tiltBackAcc()

#endif
