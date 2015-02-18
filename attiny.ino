#include <avr/sleep.h>
#include <avr/interrupt.h>

const int pinPIR  = 3;
const int pinOPTO = 2;
const int tiempoEncendido = 8;
void setup() {

  pinMode(pinPIR, INPUT);
  pinMode(pinOPTO, OUTPUT);
} // setup

void sleep() {

  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT3);                   // Use PB3 as interrupt pin
  ADCSRA &= ~_BV(ADEN);                   // ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                                  // Enable interrupts
  sleep_cpu();                            // sleep

  cli();                                  // Disable interrupts
  PCMSK &= ~_BV(PCINT3);                  // Turn off PB3 as interrupt pin
  sleep_disable();                        // Clear SE bit
  ADCSRA |= _BV(ADEN);                    // ADC on

  sei();                                  // Enable interrupts
} // sleep

ISR(PCINT0_vect) {
  // This is called when the interrupt occurs, but I don't need to do anything in it
}

void loop() {
  sleep();
  if(digitalRead(pinPIR) == HIGH){
    digitalWrite(pinOPTO, HIGH);
    //delay(2000);
  }else{
    digitalWrite(pinOPTO, LOW);
    //delay(2000);
  }
}

