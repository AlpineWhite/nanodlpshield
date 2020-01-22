cmake_minimum_required(VERSION 3.15)
project (NanoDlpShield)

find_library(wiringPi_LIB wiringPi)
find_library(bcm2835_LIB bcm2835)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(Src)
include_directories(Src/source)

set(SOURCES
	Src/source/CHOPCONF.cpp
   Src/source/COOLCONF.cpp
	Src/source/DRV_CONF.cpp
	Src/source/DRV_STATUS.cpp
	Src/source/DRVCONF.cpp
	Src/source/DRVCTRL.cpp
	Src/source/DRVSTATUS.cpp
	Src/source/ENCMODE.cpp
	Src/source/GCONF.cpp
	Src/source/IHOLD_IRUN.cpp
	Src/source/PWMCONF.cpp
	Src/source/RAMP_STAT.cpp
	Src/source/SGCSCONF.cpp
	Src/source/SHORT_CONF.cpp
	Src/source/SMARTEN.cpp
	Src/source/SW_MODE.cpp
	Src/source/SW_SPI.cpp
	Src/source/SW_SPI.h
	Src/source/TMC2130_bitfields.h
	Src/source/TMC2130Stepper.cpp
	Src/source/TMC2160_bitfields.h
	Src/source/TMC2160Stepper.cpp
	Src/source/TMC2660_bitfields.h
	Src/source/TMC2660Stepper.cpp
	Src/source/TMC5130_bitfields.h
	Src/source/TMC5130Stepper.cpp
	Src/source/TMC5160_bitfields.h
	Src/source/TMC5160Stepper.cpp
	Src/source/TMC_MACROS.h
	Src/source/TMC_platforms.h
	Src/source/TMCStepper.cpp
	Src/TMCStepper.h
	Src/Config.h
	Src/GeneratorStepper.cpp
	Src/GeneratorStepper.h
	Src/HostPty.cpp
	Src/HostPty.h
	Src/NanoDLPShield.cpp
	Src/SpeedyStepper.cpp
	Src/SpeedyStepper.h
	)

add_executable(NanoDlpShield ${SOURCES})
target_link_libraries(NanoDlpShield ${wiringPi_LIB} ${bcm2835_LIB} util)
