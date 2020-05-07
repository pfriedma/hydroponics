/* This file contains pin definitions and constants, including:
   - Time (for lights, pump timing, etc)
   - Which pins connect to what hardware
   - Adjustment and scaling variables
   - Desired pH values, etc
*/


// Pin Definitions
const int lightPin = A2;
const int airPin = A3;
const int pumpPin = A4;
const int purgePin = A5;
const int buttonPin = 5;
const int phUpPin = 4;
const int phDownPin = 3;
const int phMeasurePin = A6;

//ADC Values
const float sysVoltage = 3.3;
const float adcRes = 4095.0;
const int analogSampleSize = 10;
const int discardFirstN = 1;

// Ph values 
const float idealPh = 7.0;
const float phThreshold = 1.0;
const float phSensorVoltageScalar = 3.5;




const unsigned long phUpdateFreq = 21600;

// Relay board is low = on
#define PINON LOW
#define PINOFF HIGH

// How long before doing maintence stuff like checking in with NTP
#define HEALTH_LIMIT 1000000

// Network 
const char hostname[16]="garden";


// Timing and schedule (relative to system timezone, so probably UTC)
#define LIGHT_ON_HOURS 10
#define LIGHT_ON_MINUTES 0
#define LIGHT_OFF_HOURS 0
#define LIGHT_OFF_MINUTES 30

// The fluid system (airation and fluid re-circulate)
// runs hourly at the OFFSET from the hour for RUN_LENGTH time.
// Airation will be offset BUFFER minutes in either direction.
// Airation also runs at the specified additional time and length
#define SYSTEM_RUN_OFFSET 0
#define SYSTEM_RUN_LENGTH 15
#define AIR_BUFFER 2
#define AIR_RUN_ADDL 30
#define AIR_RUN_ADDL_LEN 15

// The pump runs for this many seconds every minute
#define PUMP_PULSE_LENGTH 25

// How long should the system run for during a priming (seconds)
#define PRIME_LENGTH 600
