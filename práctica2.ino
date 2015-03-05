#include "Wire.h"
#define DS1307_I2C_ADDRESS 0x68

const int pinBomba = 13;
const int pinLuz = 12;
const int pinHumedadTierra = A0;
const int pinLDR = A1;
const int UMBRAL_HUMEDAD_TIERRA = 700; //Por ejemplo, debera calibrarse
const int UMBRAL_LUZ = 512; //Por ejemplo, deberia calibrarse
const int tiempoRiego = 5; //5 segundos de riego. Podra calibrarse
const int primeraHoraLuz = 8; //8 de la mañana. NO PONER CEROS DELANTE!! PUEDE CONFUNDIRLO CON OCTAL
const int ultimaHoraLuz = 20; // 8 de la noche

void setup(){
  pinMode(pinBomba, OUTPUT);
  pinMode(pinLuz, OUTPUT);
}

void loop(){
  int horaActual = getHora();
  if(leerHumedadTierra() < UMBRAL_HUMEDAD_TIERRA){
    regar();
  }
  if((primeraHoraLuz < horaActual) && (horaActual < ultimaHoraLuz) && (leerCantidadLuz() < UMBRAL_LUZ)){
    encenderLuz();
  }else{
    apagarLuz();
  } 
}





void encenderLuz(){
  digitalWrite(pinLuz, HIGH);
}
void apagarLuz(){
  digitalWrite(pinLuz, LOW);  
}
int leerHumedadTierra(){
  return analogRead(pinHumedadTierra);
}

int leerCantidadLuz(){
  return analogRead(pinLDR);
}

void regar(){
  digitalWrite(pinBomba, HIGH);
  delay(tiempoRiego*1000);
  digitalWrite(pinBomba, LOW);
}

int getHora(){
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  return hour;

}
void getDateDs1307(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year)
{
  // Reset the register pointer
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);

  // A few of these need masks because certain bits are control bits
  *second     = bcdToDec(Wire.read() & 0x7f);
  *minute     = bcdToDec(Wire.read());
  *hour       = bcdToDec(Wire.read() & 0x3f);  // Need to change this if 12 hour am/pm
  *dayOfWeek  = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month      = bcdToDec(Wire.read());
  *year       = bcdToDec(Wire.read());
}
byte bcdToDec(byte val){
  return ( (val/16*10) + (val%16) );
}
