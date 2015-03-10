
#include <SPI.h>
#include <Ethernet.h>
#include "plotly_streaming_ethernet.h"
#include "DHT.h"

// Plotly
#define nTraces 4
char *tokens[nTraces] = {"6mex9r8c94", "j4o35tdgn2", "vjkkh0drk2", "ttq8z5ek17"};
plotly graph("salvin", "hislwh0r0t", tokens, "filename", nTraces);
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte my_ip[] = { 199, 168, 222, 18 }; // google will tell you: "public ip address"

// Sensores
#define PIN_DHT 2          // Pin 2 para sensor de humedad/temperatura
#define DHT_TYPE DHT22     // Tipo de sensor de humedad/temperatura DHT 22 (AM2302)
#define LDR_PIN A0         // Pin analogico 0 para LDR
#define HUM_TIERRA_PIN A1  // Pin analogico 1 para sensor de humedad en tierra

// Actuadores
#define PIN_BOMBA 13       // Pin 13 para bomba de agua
#define PIN_LUZ 12         // Pin 12 para luz

// Inicializacion 
DHT dht(PIN_DHT, DHT_TYPE);

// Variables
float hum_amb = 0;         // Humedad ambiente
float temp = 0;            // Temperatura  
float hum_tierra = 0;      // Humedad en tierra
float luz = 0;             // Cantidad de luz

void startEthernet(){
  
    Serial.println("... Inicializando ethernet ...");
    if(Ethernet.begin(mac) == 0){
        Serial.println("... Fallo de configuracion Ethernet usando DHCP");
        // Si hay fallo no tiene sentido seguir
        // Intentar configurar usando direccion IP en lugar de DHCP:
        Ethernet.begin(mac, my_ip);
    }
    Serial.println("...Inicializacion ethernet terminada.");
    delay(1000);
}

void setup() {
  
  graph.maxpoints = 100;        // Puntos de la grafica
  
  Serial.begin(9600);           // Inicializacion de puerto serie
  
  Serial.println("-- e-Huerto -- \n");
  
  pinMode(PIN_BOMBA, OUTPUT);   // El pin de la bomba de agua como salida
  pinMode(PIN_LUZ, OUTPUT);     // El pin de la luz como salida
  
  digitalWrite(PIN_BOMBA, LOW); // Apagar la bomba de agua
  digitalWrite(PIN_LUZ, LOW);   // Apagar la luz
  
  dht.begin();                  // Inicializacion sensor de temperatura-humedad
  
  startEthernet();              // Inicializacion ethernet

  /* Si todo ha ido bien se inicia la grafica */
  bool success;                 
  success = graph.init();       
  if(!success){while(true){}}
  graph.openStream();

}

void loop() {
  
  // SENSOR HUMEDAD - TEMPERATURA AMBIENTE
  
  delay(2000);                     // Espera entre mediciones

  hum_amb = dht.readHumidity();    // Lee humedad ambiente
  temp = dht.readTemperature();    // Lee temperatura en Celsius
  
  /* Comprueba que se ha leido correctamente */
  if (isnan(hum_amb) || isnan(temp)) {
    Serial.println("¡Error al leer del sensor de humedad/temperatura!");
    return;
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
  } 
  
  if(hum_tierra > 499 && hum_tierra < 701) {
    Serial.print(" Tierra humeda");
    digitalWrite(PIN_BOMBA, LOW);        // Si la tierra esta humeda se apaga la bomba
  }
  
  if(hum_tierra < 501) {
    Serial.print(" Agua");
    digitalWrite(PIN_BOMBA, LOW);        // Si se detecta agua se apaga la bomba
  }
  Serial.println("");
  
  // SENSOR LUZ
  
  // Valor leido de 0 a 1023
  // 1023  mucha luz
  // 0     nada de luz
  
  luz = analogRead(LDR_PIN);             // Lee cantidad de luz
  
  /* Impresion por puerto serie */
  Serial.print("Luz: ");
  Serial.println(luz); 
  Serial.print(" \n");
  
  analogWrite(PIN_LUZ, (luz-1023));     // Encender la luz con intensidad adecuada a la luz ambiente
  
  // Grafica en Plotly
  graph.plot(millis(), temp, tokens[0]);        // Grafica de temperatura
  graph.plot(millis(), hum_amb, tokens[1]);     // Grafica de humedad ambiente
  graph.plot(millis(), hum_tierra, tokens[2]);  // Grafica de humedad en tierra
  graph.plot(millis(), luz, tokens[3]);         // Grafica de cantidad de luz

}
