#include "SW_SPI.h"
#include <stdint.h>
#include <wiringPi.h>
#include <bcm2835.h>

SW_SPIClass::SW_SPIClass(uint16_t mosi, uint16_t miso, uint16_t sck) :
  mosi_pin(mosi),
  miso_pin(miso),
  sck_pin(sck)
  {}

void SW_SPIClass::init() {
  int SPI_enum = bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE3);
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
}

uint8_t SW_SPIClass::transfer(uint8_t ulVal) {
  uint8_t value = 0;
  value = bcm2835_spi_transfer(ulVal);
  return value;
}

uint16_t SW_SPIClass::transfer16(uint16_t data) {
  uint16_t returnVal = 0x0000;
  returnVal |= transfer((data>>8)&0xFF) << 8;
  returnVal |= transfer(data&0xFF) & 0xFF;
  return returnVal;
}
