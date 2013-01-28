#include <SPI.h>

// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin = 10;

void setup() {
  // set the slaveSelectPin as an output:
  //pinMode (slaveSelectPin, OUTPUT);
  // initialize SPI:
  SPI.begin(); 
  
  /*
  SPI.transfer(0x83);
  delay(1);
  SPI.transfer(1);
  delay(1);
  */
}

uint8_t letter = 'A';

void set_number(uint16_t val)
{
  SPI.transfer(0x88);
  delay(1);
  SPI.transfer(val & 0xFF);
  delay(1);
  SPI.transfer(val >> 8);
  delay(1);
}
void loop() {
  for (uint16_t i = 0; i < 0xffff; i++) {
    set_number(i);
    delay(10);
  }
  
  /*
  //SPI.transfer(0x81);
  //delay(1);
  
  SPI.transfer(letter++);
  delay(1000);
  SPI.transfer(letter++);
  delay(1000);
  SPI.transfer(letter++);
  delay(1000);
  
  if (letter >= 'X') letter = 'A';
  */
}

int digitalPotWrite(int address, int value) {
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin,LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 
}
