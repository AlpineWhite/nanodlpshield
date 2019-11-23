#pragma once

//////// Z-axis parameters ///////////////
//////////////////////////////////////////

// Pin #s where motor is connected
const int DIR_PIN = 5;
const int STEP_PIN = 6;
const int ENABLE_PIN = 12;

// Steps in 1mm along Z axis
const int STEPS_PER_REV = 200; //Set steps/rev of motor. 200 for 1.8* 400 for .9* NEMA17
const int MICROSTEP_SET = 16; //Set microstepping of your Driver
const int LEAD_LEN = 2; // set leadscrew lead length
const float STEPS_PER_MM = STEPS_PER_REV*MICROSTEP_SET/LEAD_LEN; //calculate steps/mm
const float DEFAULT_SPEED = 6; // mm/s
const float DEFAULT_ACCELERATION = 20; // mm/s2


// Enable this option if the printer has hardware up/down button.
//#define SUPPORT_UP_DOWN_BUTTONS 1

#if SUPPORT_UP_DOWN_BUTTONS
// a single z-axis movement using HW buttons
const float MANUAL_MOVEMENT_MM = 0.1; //mm

// After a few single moves Z-axis will move with low speed/acceleration
const float LOW_SPEED = 2; // mm/s
const float LOW_ACCELERATION = 5; // mm/s2

// If pressing up or down button for >5 seconds z-axis will move faster
const float HIGH_SPEED = 8; // mm/s
const float HIGH_ACCELERATION = 20; // mm/s2

// Pin #s for up/down buttons
const int UP_BTN_PIN = 26;
const int DOWN_BTN_PIN = 19;

// Pull up/down modes for up/down buttons (see PUD_UP/PUD_DOWN/PUD_OFF values)
const int UP_BTN_PUD = 2; // PUD_UP
const int DOWN_BTN_PUD = 2; // PUD_UP

#endif //SUPPORT_UP_DOWN_BUTTONS

//Enable support of limit switches Set to 20 for bottom home, set to 21 for top home
const int Z_STOP_PIN = 20;

//Pull UP/DN for limit switches. (Both set to active low. Set PUD to 1 for active high)
const int Z_STOP_PUD = 2;

//Homing parameters. Direction 1 is up and DIrection -1 is down.
const long HOME_DIR = -1;
const float HOME_SPD = 10; // In mm/s
const long HOME_HEIGHT = 150; //Set max dist to travel in mm during homing

// Enable this option if the printer has hardware LED On/Off button
//#define SUPPORT_LED_ON_BUTTON 1
#if SUPPORT_LED_ON_BUTTON
const int LED_ON_BTN_PIN = 13;
const int LED_ON_BTN_PUD = 2; // PUD_UP
#endif //SUPPORT_LED_ON_BUTTON

// Pin number where UV LED is connected to
const int UV_LED_PIN = 17;

// Signaling LED pin
const int LED_PIN = 16;

// FAN pin
const int FAN_PIN = 18;
