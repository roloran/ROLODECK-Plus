#ifndef _ROLODECK_GPS
#define _ROLODECK_GPS

// Undefine this to disable the use of the built-in GPS in T-Deck Plus devices
// Without GPS support, this firmware is backward compatible to plain T-Deck devices
#define ROLODECK_USE_GPS

// Use joystick button press for quick GPS CIRE sending?
// Usually only enabled for selected special operator personnel as it may be very confusing for regular users
// #define ROLODECK_GPS_QUICKCIRE

#include <Arduino.h>

void gps_setup(void);
void gps_loop(void);
void gps_displayInfo(void);
int gps_getAck(uint8_t *buffer, uint16_t size, uint8_t requestedClass, uint8_t requestedID);
static bool GPS_Recovery(void);

#endif
