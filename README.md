# NanoDLP Shield Firmware

This is a firmare for custom RaspberryPi shield to drive LCD 3d printer based on NanoDLP software.  I am not satisfied with the lack of flexibility of the ChituBoard in my Elegoo Mars, nor the ly proprietary Gcode/Firmware.

[NanoDLP](https://www.nanodlp.com/) is a web-based control for LCD or DLP 3D printer. It allows the user to upload
a model to print, slice it, and then drive 3D printer hardware (LCD, motors, shutters) in order to print the model.  Slicing can be performed on the host (typically a Pi) or by remote application on Windows/Linux/Mac which is then uploaded to the Pi. It provides full web control, a decent user interface, and supports up to ridiculous resolutions.

Unfortunately NanoDLP software is closed source and therefore has limited extension capabilities. Out of the box
it can drive Z-axis motor, display slices on LCD or DLP projector, can drive 2x16 LCD screen (or Nextion with beta firmware). But that is about it. If
you want more you have to write external scripts. Well, this is a good option, but it can't be used for low-latency
user input (e.g. adding more hardware buttons).

A good option is to delegate dealing with the hardware is by addition of an Arduino. Users can significantly extend hardware capabilities at the expense of hardware complexity and printer space.  This becomes highly detrimental in the case of more compact printers.

The aim of this project is to create a small all-in one extendable RaspberryPI shield (24V or 12V) that would include:
- Stepper Motor driver
- two Z-axis end stops (incl. optional pull-up or pull-down resistors)
- Powerful MOSFET to drive UV LED
- GPIO for user needs (buttons, driving external hardware components)
- 1-wire DS18B20 thermometer support (potentially, these are rather inaccurate at the temps we need)
- 2-pin Fan output with PWM speed support
- I2C and UART interfaces with integrated level converters (3.3V or 5V selectable)
- low power LED output for general illumination
- Buzzer
- Provide various voltage levels for external components (3.3V, 5V, input V pass-through)

This shield would take responsibility of driving all external hardware; replacing the Arduino and many external components.
Communication between the shield and NanoDLP software is implemented via pseudo-COM port, so that NanoDLP software treats it as an external Arduino.

# Hardware

Schematics and PCB V1.2 can be found [here](https://easyeda.com/editor#id=|54ad5a3a352e4a90804909cb4819c47c|c782efdc5bfa4d89bbee21e779a59103).

BOM:

MFR Part | MFR | QTY 
---| --- | --- 
BSS138 | ON Semiconductor | 4   
106990005 | Seeed Studio | 1   
MB 12	| Visaton | 1   
ERJ-UP8F4701V | Panasonic | 19  
CRCW12060000Z0EAC | Vishay	| 6   
2N7002 | ON Semiconductor | 1   
UVZ1H101MPD1TD	| Nichicon | 1   
90147-1108 | Molex | 2   
M20-9990345	| Harwin	| 8   
M20-9720345 | Harwin	| 1   
M20-9720245 | Harwin	| 2   
OQ025A000000G | Amphenol | 2   
M20-9730245	| Harwin | 2   
M20-9990445	| Harwin | 2   
URS1H101MPD1TD	| Nichicon | 1   
CRCW1206120RFKEAC	| Vishay | 4   
VJ1206Y104JXAMR | Vishay | 8   
UMK316BJ105KD-T | Taiyo Yuden	| 1   
FDLL4148-D87Z | ON Semiconductor	| 1   
76342-320HLF | FCI / Amphenol	| 1   
DMN3150L-7 | Diodes Incorporated |	1   
IRL2203NPBF | Infineon | 1   

Any of the components can be swapped for equivalents at the user's discretion if you have brand preferences.

Choice: Add 2x JST-XH 3-Pin and 1x JST-XH 4-Pin for Motor + E-stop connectors or substitute with bare pin headers.

# Software

***
NanoDlp Easy Installer throws dhcpcd5 package errors. This will cause the Pi to hang and fail after install. Please only use the advanced install with wget on the NanoDlp install page.
***

Software is based on WiringPi library which provides a Arduino-like interface to drive RasPi's GPIOs. WiringPi is no longer in development, and its creator will no longer support it.  I am in search of alternate C++/C implementations. Java is too slow for real-time interrupts. Tests on a Pi2 show Java capped at ~20kHz and C/C++ at closer to 4MHz GPIO square wave output.

Additionally it uses
[SpeedyStepper](https://github.com/Stan-Reifel/SpeedyStepper) library with minor modifications to drive stepper motor.

Installing prerequisites
```bash
sudo apt-get install cmake g++ wiringpi
```

# Building
-Install git on the Raspberry Pi.
-Git clone the repo:
```bash
   git clone https://github.com/AlpineWhite/nanodlpshield.git
```

-Use Nano to edit:
    -> Config.h
        -> steps/mm and leadscrew settings
        -> set speeds and accelerations
        -> set the endstop pin and pull up/dn mode per active high or active low endstop
        -> defined/undefined hardware button support
        -> defined/undefined buzzer enable

 create cmake dependencies with:
 ```bash
    cmake ~/nanodlpshield
 ```   
 build with:
 ```bash
    cmake --build ~/nanodlpshield
 ```  
 install with:
 ```bash
    cmake --install ~/nanodlpshield
 ```    
 Run the installed app with:
 ```bash
    cd ~/nanodlpshield
    ./NanoDlpShield
 ```
 and note the resulting name return, which should be:
     ```bash
     Name: /dev/pts/1
     ```
    (the number will increment if you restart the app without rebooting the pi)

 Open NanoDLP Web GUI, set the shield mode to USB/I2C and set the address to the value returned from 'Name:'.

 NanoDLP requires some extra setup for a 'through shield' implementation.  You can find a guide [for setting up pre/post print commands and resin profile GCode commands here.](https://www.nanodlp.com/forum/viewtopic.php?id=41)

# NOTE: I have to build each Gcode Manually. Current commands are:

 - G1
 - G4 (wait)
 - G28 (home)
 - G90
 - G91
 - M3 / M106 (UV LED On)
 - M106 P1 Snnnn (set fan headers to PWM value nnnn between 0 and 1023)
 - M5 / M107 (UV LED Off)
 - M107 P1 (turn all fans off)
 - M18 (disable motors)
 - M114 (get current position)
 - M300 Snnn (sound buzzer for Snnn seconds)


# Limitations:

 Since NanoDLP does not provide a way for RAMPS/Serial to write back, NanoDLP will not show the result of commands sent through the RAMPS terminal (primarily LED On/Off and current position if sent via Terminal).

 NanoDLP does execute the Gcode via the terminal, so any commands triggered via the NanoDLP UI or during printing will reflect properly.

# Planned additions:

 Configuration options for:
  - <strike>Max Height</strike>
  - <strike>Max Speeds</strike>
  - <strike>Motion parameters via Config.h</strike>
  - <strike>Max Accelerations</strike>
  - <strike>Implement PWM fan control</strike>
  - <strike>Fan control based on printing (fans on when UV LED is on and then on for 5 mins after finish of print)</strike> This can be done using NanoDLP GCode boxes now that PWM fan control is used.
  - Detailed setup instructions
  - TMC SPI control for motion.  This will probably take the longest.
