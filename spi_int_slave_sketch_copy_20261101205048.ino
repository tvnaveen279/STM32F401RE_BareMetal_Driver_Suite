#include <SPI.h>

void setup() {
  // 1. Set MISO as OUTPUT so Arduino can drive data to STM32
  pinMode(MISO, OUTPUT);

  // 2. Configure SS (Pin 10) as INPUT with internal pull-up disabled (or connect Pin 10 directly to GND)
  pinMode(10, INPUT);

  // 3. Enable SPI in Slave Mode (MSTR bit remains 0) and enable SPI Interrupt
  SPCR |= _BV(SPE);
  SPCR |= _BV(SPIE);

  // 4. Preload the first byte into Data Register so it's ready for the first SCLK edge
  SPDR = 0xA5;
}

// SPI Interrupt Service Routine (triggers after 1 byte transfer completes)
ISR (SPI_STC_vect) {
  byte c = SPDR;  // Read dummy byte sent by STM32 (0xFF)
  SPDR = 0xA5;    // Preload 0xA5 immediately for the subsequent clock pulses
}

void loop() {
  // Empty loop
}