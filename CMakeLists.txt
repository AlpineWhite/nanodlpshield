cmake_minimum_required(VERSION 2.8.9)
project (NanoDlpShield)

find_library(wiringPi_LIB wiringPi)
find_library(bcm2835_LIB bcm2835)

add_library(
    Src/TMCStepper
    Src/source/CHOPCONF.cpp
    Src/source/COOLCONF.cpp
    Src/source/DRV_CONF.cpp
    Src/source/DRVCTRL.cpp
    Src/source/DRVSTATUS.cpp
    Src/source/ENCMODE.cpp
    Src/source/GCONF.cpp
    Src/source/IHOLD_IRUN.cpp
    Src/source/PWMCONF.cpp
    Src/source/RAMP_STAT.cpp
    Src/source/GCSCONF.cpp
    Src/source/SMARTEN.cpp
    Src/source/SW_MODE.cpp
    Src/source/SW_SPI.cpp
    Src/source/TMC2130Stepper.cpp
    Src/source/TMC2160Stepper.cpp
    Src/source/TMC2660Stepper.cpp
    Src/source/TMC5130Stepper.cpp
    Src/source/TMC5160Stepper.cpp)


set(SOURCES
    Src/NanoDLPShield.cpp 
    Src/HostPty.cpp
    Src/SpeedyStepper.cpp
    Src/GeneratorStepper.cpp)

add_subdirectory(Src/source)
add_executable(NanoDlpShield ${SOURCES})
target_link_libraries(NanoDlpShield ${wiringPi_LIB} ${bcm2835_LIB} TMCStepper TMC2130Stepper TMC2660Stepper TMC5130Stepper TMC5160Stepper util)
