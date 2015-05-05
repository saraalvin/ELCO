
#include <SPI.h>
#include "DHT.h"
#include "nRF24L01.h"
#include "RF24.h"


// Sensores
#define PIN_DHT 2          // Pin 2 para sensor de humedad/temperatura
#define DHT_TYPE DHT22     // Tipo de sensor de humedad/temperatura DHT 22 (AM2302)
#define LDR_PIN A0         // Pin analogico 0 para LDR
#define HUM_TIERRA_PIN A1  // Pin analogico 1 para sensor de humedad en tierra

// Actuadores
#define PIN_BOMBA 3        // Pin 3 para bomba de agua
#define PIN_LUZ 4          // Pin 4 para luz
#define LED_VERDE 5
#define LED_AMARILLO 6
#define LED_ROJO 7

// Inicializacion 
DHT dht(PIN_DHT, DHT_TYPE);

// Variables
float hum_amb = 0;         // Humedad ambiente
float temp = 0;            // Temperatura  
float hum_tierra = 0;      // Humedad en tierra
float luz = 0;             // Cantidad de luz
String mi_direccion = "H1";


int serial_putc( char c, FILE * ) 
{
  Serial.write( c );
  return c;
} 

//Debug
void printf_begin(void)
{
  fdevopen( &serial_putc, 0 );
}

//nRF24 Cableado utilizado. El pin 9 es CE y 10 a CSN/SS
//     CE       -> 9
//     SS       -> 10
//     MOSI     -> 11
//     MISO     -> 12
//     SCK      -> 13

RF24 radio(9,10);

const uint64_t pipes[6] = {
  0x65646f4e32LL,0x65646f4e31LL};


char b[30];
String str1, str2, str3, str4;
int msg[1];
String theMessage = "";
char rev[50]="";


void setup() {
  
  Serial.begin(57600);           // Inicializacion de puerto serie
  
  Serial.println("-- e-Huerto -- \n");
  
  pinMode(PIN_BOMBA, OUTPUT);   // El pin de la bomba de agua como salida
  pinMode(PIN_LUZ, OUTPUT);     // El pin de la luz como salida
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  
  digitalWrite(PIN_BOMBA, LOW); // Apagar la bomba de agua
  digitalWrite(PIN_LUZ, LOW);   // Apagar la luz
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_ROJO, LOW);
  
  dht.begin();                  // Inicializacion sensor de temperatura-humedad
  
  //nRF24 configuración
  radio.begin();
  radio.setChannel(0x4c);
  radio.setAutoAck(1);
  radio.setRetries(15,15);
  radio.setPayloadSize(32);
  radio.openReadingPipe(1,pipes[0]);
  radio.openWritingPipe(pipes[1]);
  radio.startListening();
  radio.printDetails(); //Debug

  radio.powerUp();  


}

void loop() {
  
  // SENSOR HUMEDAD - TEMPERATURA AMBIENTE
  
  delay(2000);                     // Espera entre mediciones

  hum_amb = dht.readHumidity();    // Lee humedad ambiente
  temp = dht.readTemperature();    // Lee temperatura en Celsius
  
  /* Comprueba que se ha leido correctamente */
  if (isnan(hum_amb) || isnan(temp)) {
    Serial.println("¡Error al leer del sensor de humedad/temperatura!");
    //return;
  }

  /* Impresion por puerto serie */
  Serial.print("Temperatura: "); 
  Serial.print(temp);
  Serial.println(" C ");
  
  Serial.print("Humedad ambiente: "); 
  Serial.print(hum_amb);
  Serial.println(" %");
  
  // SENSOR HUMEDAD TIERRA
  
  // 0  ~500     agua
  // 500~700     tierra humeda
  // 700~950     tierra seca
  
  hum_tierra = analogRead(HUM_TIERRA_PIN); // Lee humedad en tierra
  
  /* Impresion por puerto serie */
  Serial.print("Humedad de la tierra:");
  Serial.print(hum_tierra); 

  if(hum_tierra > 699) {
    Serial.print(" Tierra seca");
    digitalWrite(PIN_BOMBA, HIGH);       // Si la tierra esta seca se enciende la bomba
    
    digitalWrite(LED_ROJO, HIGH);
    digitalWrite(LED_AMARILLO, LOW);
    digitalWrite(LED_VERDE, LOW);
  } 
  
  if(hum_tierra > 499 && hum_tierra < 701) {
    Serial.print(" Tierra humeda");
    digitalWrite(PIN_BOMBA, LOW);        // Si la tierra esta humeda se apaga la bomba
    
    digitalWrite(LED_ROJO, LOW);
    digitalWrite(LED_AMARILLO, LOW);
    digitalWrite(LED_VERDE, HIGH);
  }
  
  if(hum_tierra < 501) {
    Serial.print(" Agua");
    digitalWrite(PIN_BOMBA, LOW);        // Si se detecta agua se apaga la bomba
    
    digitalWrite(LED_ROJO, LOW);
    digitalWrite(LED_AMARILLO, HIGH);
    digitalWrite(LED_VERDE, LOW);
  }
  Serial.println("");
  
  // SENSOR LUZ
  
  // Valor leido de 0 a 1023
  // 1023  mucha luz
  // 0     nada de luz
  
  luz = analogRead(LDR_PIN);             // Lee cantidad de luz
  
  /* Impresion por puerto serie */
  Serial.print("Luz: ");
  Serial.println((luz/1023)*100); 
  Serial.print(" \n");
  
  if (luz < 900) {
    digitalWrite(PIN_LUZ, HIGH);
  } else {
    digitalWrite(PIN_LUZ, LOW);
  }

  if (radio.available()){
    Serial.println("recibido datos");
    while (radio.available()) {                
      radio.read( &rev, sizeof(rev) );     
      Serial.print(rev);
      if(String(rev).equals("RP;H1;damedatos")){
         emite();
      }  
    }
    Serial.println();
  }
}


void emite(){
String dato="H1;RP;";
  
  str1 = String(luz);
  str2 = String(hum_amb);
  str3 = String(hum_tierra);
  str4 = String(temp);
  
  dato = dato + str1 + ";" + str2 + ";" + str3 + ";" + str4 + ";" ;
  
  Serial.println(dato);
  
  dato.toCharArray(b,30);

  radio.stopListening();
  Serial.println("Enviando datos...");
  bool ok = radio.write(&b,strlen(b));
  radio.startListening(); 
  delay(2000);
}

