cmake_minimum_required(VERSION 2.8.9)
project (NanoDlpShield)

find_library(wiringPi_LIB wiringPi)
find_library(bcm2835_LIB bcm2835)
add_subdirectory(Src/source)
include(addTMCStepper.cmake)


set(SOURCES
    Src/NanoDLPShield.cpp 
    Src/HostPty.cpp
    Src/SpeedyStepper.cpp
    Src/GeneratorStepper.cpp
    )
add_executable(NanoDlpShield ${SOURCES})
target_link_libraries(NanoDlpShield ${wiringPi_LIB} util)
