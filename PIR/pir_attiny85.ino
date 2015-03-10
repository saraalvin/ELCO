#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <WProgram.h>
#include <Arduino.h>

const int pinPIR  = 4;                   // Pin PB4 (pin 3) for PIR sensor
const int pinOPTO = 0;                   // Pin PB0 (pin 5) for optocoupler
const int tiempoEncendido = 8;           
void setup() {

  pinMode(pinPIR, INPUT);                // Sets PIR pin as input
  pinMode(pinOPTO, OUTPUT);              // Sets optocoupler pin as output
} // setup

void sleep() {

  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT4);                   // Use PB3 as interrupt pin
  ADCSRA &= ~_BV(ADEN);                   // ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                                  // Enable interrupts
  sleep_cpu();                            // sleep

  cli();                                  // Disable interrupts
  PCMSK &= ~_BV(PCINT4);                  // Turn off PB3 as interrupt pin
  sleep_disable();                        // Clear SE bit
  ADCSRA |= _BV(ADEN);                    // ADC on

  sei();                                  // Enable interrupts
} // sleep

ISR(PCINT0_vect) {
  // This is called when the interrupt occurs, but I don't need to do anything in it
}

void loop() {
  
  sleep();   // Sets the ATTINY85 in sleep mode
  
  if(digitalRead(pinPIR) == HIGH){         // Check if PIR has been triggered
    digitalWrite(pinOPTO, HIGH);           // Turn light on
    //delay(2000);
  }else{                                   // If PIR hasn't been triggered
    digitalWrite(pinOPTO, LOW);            // Turn light off
    //delay(2000);
  }
}
