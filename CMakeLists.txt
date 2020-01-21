cmake_minimum_required(VERSION 2.8.9)
project (NanoDlpShield)

find_library(wiringPi_LIB wiringPi)
find_library(bcm2835_LIB bcm2835)


set(SOURCES
    Src/NanoDLPShield.cpp 
    Src/HostPty.cpp
    Src/SpeedyStepper.cpp
    Src/GeneratorStepper.cpp
    Src/source/TMCStepper.cpp
    src/Source/CHOPCONF.cpp
    src/Source/COOLCONF.cpp
    src/Source/DRV_CONF.cpp
    src/Source/DRVCTRL.cpp
    src/Source/DRVSTATUS.cpp
    src/Source/ENCMODE.cpp
    src/Source/GCONF.cpp
    src/Source/IHOLD_IRUN.cpp
    src/Source/PWMCONF.cpp
    src/Source/RAMP_STAT.cpp
    src/Source/SGCSCONF.cpp
    src/Source/SMARTEN.cpp
    src/Source/SW_MODE.cpp
    src/Source/SW_SPI.cpp
    src/Source/TMC2130Stepper.cpp
    src/Source/TMC2160Stepper.cpp
    src/Source/TMC2660Stepper.cpp
    src/Source/TMC5130Stepper.cpp
    src/Source/TMC5160Stepper.cpp
    )
add_executable(NanoDlpShield ${SOURCES})
target_link_libraries(NanoDlpShield ${wiringPi_LIB} ${bcm2835_LIB} util)
