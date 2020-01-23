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

Schematics and PCB V1.2 can be found [here](https://easyeda.com/p.cartwrig/nanodlpshieldintegrated_copy).

BOM:

MFR Part | MFR | PACKAGE | QTY 
---| --- | --- | ---   
WJ2EDGVC-5.08-2P | NGK | 5.08mm Pluggable 2-Pin Header | 2   
C27438 | BOOMELE | 8-Pin Female Header | 2   
C35165 | BOOMELE | 40-Pin Through-Hole 2-Row Header | 1  
WJ2EDGK-5.08-2P | NGK | 2-Position Pluggable Male Terminal 5.08mm |  2   
C116600 | LCSC | TO-220 Heatsink | 1   
C124378 | LCSC | 4-Pin Male 2.54mm Header | 20   
C132448 | LCSC | 2-Pin 2.54mm Jumper | 10   
IPP80N08S2L07AKSA1 | Infineon | TO-220 N-Channel MOSFET | 1   
ENB1CM471F12OTF3 | AISHI | 470uF 8x12mm 16V Capacitor | 1   
A2541HWV-3P | CJT | 3-Pin 2.54mm Female Header | 1   
HNDZ-1206 | Jiangsu | 12mmx6mm Buzzer | 1   
LKMD1151J121MF	| YMIN | 120uF 8x11.5mm 63V Capacitor | 1 
LL4148 | Guangdong Hottech | SOD-80 Diode | 1
AC1206FR-074K7L | Yageo/Vishay | 1206R 4.7k-Ohm | 11
RC1206FR-07120RL | Yageo/Vishay | 1206R 120-Ohm | 4
AC1206FR-070RL | Yageo/Vishay | 1206R 0-Ohm | 6
1206B105K101 | Walsin | 1206C 1uF | 1
DMN6075S-7 | Diodes Inc. | SOT-23 N-Channel MOSFET | 1
2N7002K-T1-GE3 | Vishay | SOT-23 2N7002 MOSFET| 1
TCC1206X7R104K500DT | CCTC | 1206C 100nF | 8
MP1584EN | Multiple | MP1584 5V Fixed Buck | 1

Cost savings are achieved by having all parts orderable from LCSC at the same time as the PCB is ordered.  Components were chosen so that all SMD can be ordered with pick-&-place services from JLCPCB.  Headers have minimum order quantities, so 4-pin headers are broken-down to populate smaller header locations.

Any of the components can be swapped for equivalents at the user's discretion if you have brand preferences.


# Software

***
NanoDlp Easy Installer throws dhcpcd5 package errors. This will cause the Pi to hang and fail after install. Please only use the advanced install with wget on the NanoDlp install page.
***

Software is based on [WiringPi](https://github.com/WiringPi/WiringPi) library which provides a Arduino-like interface to drive RasPi's GPIOs. WiringPi is no longer in development by the original author, and its creator will no longer support it.  All support is community-based going forward.

The [BCM2835](https://www.airspayce.com/mikem/bcm2835/) library is used to handle SPI communication to the TMC steppers, due to its inbuilt transfer() and transfernb() functions.

Additionally it uses
[SpeedyStepper](https://github.com/Stan-Reifel/SpeedyStepper) library with minor modifications to drive the stepper motor.
For TMC SPi control, a modified version of the [TMCStepper](https://github.com/teemuatlut/TMCStepper) library is included in the code with a GeneratorStepper class created on the SpeedyStepper template handling SPI calls to the ramp generator in the TMC5160 and TMC5130 ICs.

Installing prerequisites
```bash
sudo apt-get install cmake g++ wiringpi
```

Installing bcm2835 Library
```bash
sudo apt-get install html-xml-utils
mkdir -p bcm2835 && (wget -qO - `curl -sL http://www.airspayce.com/mikem/bcm2835 | hxnormalize -x -e | hxselect -s '\n' -c "div.textblock>p:nth-child(4)>a:nth-child(1)"` | tar xz --strip-components=1 -C bcm2835 )
cd bcm2835
./configure
make
sudo make install
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
        -> set TMC stepper support (in testing/development) options
        -> set the endstop pin and pull up/dn mode per active high or active low endstop
        -> set homing direction
        -> defined/undefined hardware button support

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
