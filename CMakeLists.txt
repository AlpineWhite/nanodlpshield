cmake_minimum_required(VERSION 2.8.9)
project (NanoDlpShield)

find_library(wiringPi_LIB wiringPi)
find_library(bcm2835_LIB bcm2835)

set(SOURCES
    Src/NanoDLPShield.cpp 
    Src/HostPty.cpp
    Src/SpeedyStepper.cpp
    )
add_executable(NanoDlpShield ${SOURCES})
target_link_libraries(NanoDlpShield ${wiringPi_LIB} util)
