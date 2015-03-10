#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <WProgram.h>
#include <Arduino.h>
#include <TinyDebugKnockBang.h>

const int pinPIR  = 4;                  // Pin PB4 (pin 3) for PIR sensor
const int pinOPTO = 0;                  // Pin PB0 (pin 5) for optocoupler
const int pinLDR = 2;                   // Pin PB2 (pin 7) for LDR sensor
const int pinLDR_VCC = 1;               // Pin PB1 (pin 6) to control LDR power supply
const int tiempoEncendido = 8;
int ldr_val;                            // Stores value read from LDR sensor

void setup() {

  pinMode(pinPIR, INPUT);               // Sets PIR pin as input
  pinMode(pinOPTO, OUTPUT);             // Sets optocoupler pin as output
  //pinMode(pinLDR, INPUT);               // Sets LDR pin as input
  pinMode(pinLDR_VCC, OUTPUT);          // Sets LDR power supply pin as output
  digitalWrite(pinLDR_VCC, HIGH);       // Turns off LDR power supply to reduce power consumption
  
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
  
  //sleep();                                 // Set the ATTINY85 in sleep mode
  
  if(digitalRead(pinPIR) == HIGH){         // Check if PIR has been triggered
    
    digitalWrite(pinLDR_VCC, HIGH);        // Turn on LDR power supply
    ldr_val = analogRead(pinLDR);          // Read value from LDR sensor
    
    if(ldr_val < 1023*0.5){                // Check if there is low luminosity
      digitalWrite(pinOPTO, HIGH);         // Turn light on
      digitalWrite(pinLDR_VCC, LOW);       // Turn off LDR power supply
    }else{
      digitalWrite(pinOPTO, LOW);          // Turn light off
    }
    //delay(2000);
  }else{
    digitalWrite(pinOPTO, LOW);            // Turn light off
    //delay(2000);
  }
}
