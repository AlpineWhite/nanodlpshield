#pragma once


////////   Buzzer Settings   /////////////
/*
Set the define to 1 (insatead of 0) to enable the buzzer.
The buzzer sounds at a fixed frequency, but the duration of the tone can be controlled by:
M300 Snnn

Snnn = tine to buzz in seconds

This will consume GPIO7.
*/ 

#define SUPPORT_BUZZER 0   
#if SUPPORT_BUZZER
const int BUZZ_PIN = 8;
#endif

//_______________________________________________________________________________________________________________________________________
//////// TMC SPI Settings ///////////////
//These are not yet implemented. Do not use them.
/*
Set the HAS_TMC_SPI define to 1 to enable SPi control for TMC steppers. 
GPIO 8 and 11 will be unavailable.
DO NOT INSTALL RESISTORS ON THESE IF YOU PLAN TO USE TMC STEPPERS
*/
#define HAS_TMC_SPI 1               
#if HAS_TMC_SPI
const int CS_PIN = 8;
const int SW_MOSI = 10;
const int SW_MISO = 9;
const int SW_SCK = 11;
const float R_SENSE = .075;          //Enter sense resistor value from step stick mfg datasheet
const long RMS_A = 1105;             //Enter current in mA
const float HOLD_MULT = .7;           //Set stantstill motor hold multiplier
// Select your stepper driver type by uncommenting it
//#define HAS_2130 1
//#define HAS_2660 1
//#define HAS_5130 1
#define HAS_5160 1
#define RAMP_MODE 1;
#ifdef RAMP_MODE
    const int X_COMP = 22;
    #endif
#endif

//________________________________________________________________________________________________________________________________________
//This is for the 1-wire thermal monitor.
/*
1-Wire thermal monitors can only be enabled on a single pin, GPIO4. 
Install a 4.7k SMD on the R10 pad to use a DS18B20
To enable the 1-Wire interface, open a shell and Enter
sudo nano /boot/config.txt
check if :
    dtoverlay=w1-gpio
exists at the end.  If it does not, add it.  Ctrl+O to write to the file, Ctl+X to exit.
Reboot the Pi.
*/
#define HAS_THERM 0
#if HAS_THERM
const int THERM_PIN = 4;
#endif

//_________________________________________________________________________________________________________________________________________
//////// Z-axis parameters ///////////////
/*

The following settings control universal pins on the motor driver, as well as motion parameters of your Z axis.

Acceleration and default speed is user-selectable.  5.5" printers from CHina have a default acceleration of 700
to 1000.  A higher acceleration gives less force on peel moves, but takes less time to peel / resume after each layer.
Too high of a default speed or acceleration can result in stretched/failed prints.

Recommend default values for 5/5" build plates, lower values for larger plates.
*/

// Pin #s where motor is connected. THese should not change.
const int DIR_PIN = 5;
const int VCC_PIN = 27;
const int STEP_PIN = 6;
const int ENABLE_PIN = 12;
const int STEPS_PER_REV = 200; //Set steps/rev of motor. 200 for 1.8* 400 for .9* NEMA17
const int MICROSTEP_SET = 16; //Set your chosen microstepping of your driver to match the jumpers ont he board.
const int LEAD_LEN = 2; // set leadscrew lead length in mm/rev
const float STEPS_PER_MM = STEPS_PER_REV*MICROSTEP_SET/LEAD_LEN; //calculate steps/mm. No user input.
const float DEFAULT_SPEED = 2; // Set the 'default' speed that will be used if no speed (G1 Zxx) is provided in mm/s 
const float DEFAULT_ACCELERATION = 600; // set default accelerations. 600-1000 is normal for a 5.5" build area (in mm/s2).

//___________________________________________________________________________________________________________________________________________
//////// Manual Movement BUttons ///////////////
/*
This section defines a set of manusl press-and-hold buttons for moving the build plate up and down in
staged accelerated rates.  Set the define to 1 to enable manual movement with the buttons.
The buttons consume GPIO pins 26 and 19 on the breakout header.
*/

// Enable this option if the printer has hardware up/down buttons.
#define SUPPORT_UP_DOWN_BUTTONS 0

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

//________________________________________________________________________________________________________________________________________
//////// Limit Switches ///////////////
/*

This section controls the limit switches.  You can have a limit switch at the top or the bottom for homing movements.
THe setting of the limit switch pins determines where the endstop lives.
Set Z_STOP_PIN to 26 for a limit switch connected to the bottom position on the shield.
Set Z_STOP_PIN to 16 for a limit switch connected to the top position on the shield.

The Z_STOP_PUD setting must compliment your endstops! For active high (3.3v when triggered) set to 1; for active low (GND when triggered), set to 2.
Consult your endstop switch documentation and set to:
*/

//Enable support of limit switches.
const int Z_STOP_PIN = 26;

//Pull UP/DN for limit switches. (Set to active low. Set PUD to 1 for active high)
const int Z_STOP_PUD = 2;

////////  Homing parameters. for the Z-axis ///////////////
/*
HOME_DIR is set automatically according to your selection of limit switch above.

HOME_SPD is the default speed while homing.  THe homing function is two-stage. 
First a pass at your default HOME_SPD then a second pass at 1mm/s for accurate homing.

If the HOME_HEIGHT parameter is smaller than your Z-axis length, then homing will fail.
For safety, set the HOME_HEIGHT value to your axis length +5mm
*/
const long HOME_DIR;
#if (Z_STOP_PIN == 26)
    HOME_DIR = -1;
#endif
#if (Z_STOP_PIN == 16)
    HOME_DIR = 1;
#endif
const float HOME_SPD = 10; // In mm/s
const long HOME_HEIGHT = 160; //Set max dist to travel in mm during homing


//________________________________________________________________________________________________________________________________________
//////// Manual UV Button ///////////////
/* 
This option toggles the UV LED output while a button on GPIO13 is pressed.
Enable this option if the printer has a hardware LED On/Off button.
*/
//#define SUPPORT_LED_ON_BUTTON 1
#if SUPPORT_LED_ON_BUTTON
const int LED_ON_BTN_PIN = 20;
const int LED_ON_BTN_PUD = 2; // PUD_UP
#endif //SUPPORT_LED_ON_BUTTON


//________________________________________________________________________________________________________________________________________
//////// Constants ///////////////

// Pin number where UV LED is connected to
const int UV_LED_PIN = 17;

// Signaling LED pin
const int LED_PIN = 21;

// FAN pin
const int FAN_PIN = 18;

