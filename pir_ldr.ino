//**Includes **

#include <WProgram.h>
#include <Arduino.h>
#include <avr/io.h>     
#include <util/delay.h>   
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

//** Definitions **

#define PIN_PIR 4                         // Input from PIR
#define PIN_OPTO 0                        // Output for optocoupler
#define TIME_ON 2000                      // Time to have light on

#define LDR   PINB3 
#define LDR_PORT PINB
#define LDR_DDR  DDRB

#define OPTO   PB0  
#define OPTO_PORT PORTB
#define OPTO_DDR  DDRB

//** Function Prototypes **

void setup ();
void sleep ();
ISR(PCINT0_vect);
void loop ();
uint8_t get_adc ();

//**

int ldr_val;
int scaled_val;

void setup () 
{

  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_OPTO, OUTPUT);

  digitalWrite( PIN_OPTO, LOW );        // Turn off light

  LDR_DDR &= ~( 1 << LDR );             // set LDR pin to 0 for input
  LDR_PORT |= ( 1 << LDR );             // write 1 to enable internal pullup

  OPTO_DDR |= ( 1 << OPTO);             // set PB0 to "1" for output 
  OPTO_PORT &= ~( 1 << OPTO );          // turn the optocoupler off

  //********** PWM Config *********
 
  TCCR0A |= ( ( 1 << COM0A1 ) | ( 1 << WGM01 ) | ( 1 << WGM00 ) );  // non inverting fast pwm
  TCCR0B |= ( 1 « CS02 ) | ( 1 « CS00 );                            // Set maximum prescaler (1024) for minimum PWM frequency
  TCCR0B |= ( 1 << CS00 );              // start the timer
 
  //********** ADC Config **********
 
  ADMUX |= ( ( 1 << ADLAR ) | ( 1 << MUX1 ) | ( 1 << MUX0 ) );    // left adjust and select ADC3
  ADCSRA |= ( ( 1 << ADEN ) | ( 1 << ADPS2 ) | ( 1 << ADPS1 ) );  // ADC enable and clock divide 8MHz by 64 for 125khz sample rate
  DIDR0 |= ( 1 << ADC3D );                                        // disable digital input on analog input channel to conserve power

} // setup

void sleep () 
{
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

ISR (PCINT0_vect) {
  // This is called when the interrupt occurs, but I don't need to do anything in it
}

uint8_t get_adc ()
{
  ADCSRA |= ( 1 << ADSC );   // start the ADC Conversion
 
  while( ADCSRA & ( 1 << ADSC ));  // wait for the conversion to be complete

  return ~ADCH; // return the inverted 8-bit left adjusted adc val

}

void loop () 
{
  sleep ();

  if ( digitalRead(PIN_PIR) == HIGH )
  {

    ldr_val = get_adc ();                 // Read ADC value from LDR
    _delay_ms( 3 );                       // ADC conversion time

    scaled_val = ((ldr_val/266)*100);

    if ( scaled_val <= 2)                 // Checks luminosity (value read <= 0.1V)
    {
      // Turn on the light
      OCR0A = ldr_val;                    // Set PWM duty cycle
      // digitalWrite( PIN_OPTO, HIGH );     // Turn on light
      _delay_ms( TIME_ON );               // Keep light on for a while
    } 

    else 
    {
      // Turn off the light
      OCR0A = 0;                          // Turn off PWM
      // digitalWrite( PIN_OPTO, LOW );      // Turn off light
    }
  }
}
