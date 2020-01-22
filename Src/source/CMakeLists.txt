
set(SOURCES
    TMCStepper.cpp
    CHOPCONF.cpp
    COOLCONF.cpp
    DRV_CONF.cpp
    DRVCTRL.cpp
    DRVSTATUS.cpp
    ENCMODE.cpp
    GCONF.cpp
    IHOLD_IRUN.cpp
    PWMCONF.cpp
    RAMP_STAT.cpp
    SGCSCONF.cpp
    SMARTEN.cpp
    SW_MODE.cpp
    SW_SPI.cpp
    TMC2130Stepper.cpp
    TMC2160Stepper.cpp
    TMC2660Stepper.cpp
    TMC5130Stepper.cpp
    TMC5160Stepper.cpp)
add_library(TMCStepper ${SOURCES})