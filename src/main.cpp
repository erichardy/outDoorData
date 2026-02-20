#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define WIRE Wire
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &WIRE);

#define DEBUG
// #undef DEBUG
#ifdef DEBUG
  #define debug(x) Serial.print(x)
  #define debugln(x) Serial.println(x)
#else
  #define debug(x)
  #define debugln(x)
#endif

int xl = 0;
float tempC = 0.0;
float jj = 6.3;

// related to DS18B20
#define DS18B20_PIN 4
#define TEMPERATURE_PRECISION 12
OneWire ds(DS18B20_PIN);
DallasTemperature sensors(&ds);
DeviceAddress insideThermometer;

/* Code de retour de la fonction getTemperature() */
enum DS18B20_RCODES {
  READ_OK,  // Lecture ok
  NO_SENSOR_FOUND,  // Pas de capteur
  INVALID_ADDRESS,  // Adresse reçue invalide
  INVALID_SENSOR  // Capteur invalide (pas un DS18B20)
};

// Motion sensor
const int MOTION_SENSOR_PIN = 19;
int motionSensorCurrentState   = LOW;  // current state of pin
int motionSensorPreviousState  = LOW;  // previous state of pin

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

float getExternalTemp() {
  sensors.requestTemperatures();
  return sensors.getTempC(insideThermometer);
}

int getMotionSensor(){
  motionSensorPreviousState = motionSensorCurrentState;
  return digitalRead(MOTION_SENSOR_PIN);
}

bool motionDetected(){
  motionSensorCurrentState = getMotionSensor();
  if ((motionSensorCurrentState == HIGH) && (motionSensorPreviousState == LOW)) {
    return 1;
  } else {
    return 0;
  }
}

/**
 * Fonction de lecture de la température via un capteur DS18B20.
 */
byte getTemperature(float *temperature, byte reset_search) {
  byte data[9], addr[8];
  // data[] : Données lues depuis le scratchpad
  // addr[] : Adresse du module 1-Wire détecté
  
  /* Reset le bus 1-Wire ci nécessaire (requis pour la lecture du premier capteur) */
  if (reset_search) {
    ds.reset_search();
  }
 
  /* Recherche le prochain capteur 1-Wire disponible */
  if (!ds.search(addr)) {
    // Pas de capteur
    return NO_SENSOR_FOUND;
  }
  
  /* Vérifie que l'adresse a été correctement reçue */
  if (OneWire::crc8(addr, 7) != addr[7]) {
    // Adresse invalide
    return INVALID_ADDRESS;
  }
 
  /* Vérifie qu'il s'agit bien d'un DS18B20 */
  if (addr[0] != 0x28) {
    // Mauvais type de capteur
    return INVALID_SENSOR;
  }
 
  /* Reset le bus 1-Wire et sélectionne le capteur */
  ds.reset();
  ds.select(addr);
  
  /* Lance une prise de mesure de température et attend la fin de la mesure */
  ds.write(0x44, 1);
  delay(800);
  
  /* Reset le bus 1-Wire, sélectionne le capteur et envoie une demande de lecture du scratchpad */
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
 
 /* Lecture du scratchpad */
  for (byte i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
   
  /* Calcul de la température en degré Celsius */
  *temperature = (int16_t) ((data[1] << 8) | data[0]) * 0.0625; 
  
  // Pas d'erreur
  return READ_OK;
}

void setup() {
  Serial.begin(115200);
  Serial.print("dans setup");
  // put your setup code here, to run once:
  // display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  display.display();
  delay(1000);
  display.setRotation(2); // vertical flip
  display.setTextSize(1.5);
  display.setTextSize(2);

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();
  display.clearDisplay();
  //
  pinMode(MOTION_SENSOR_PIN, INPUT);
  sensors.begin();
  delay(500);
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  if (!sensors.getAddress(insideThermometer, 0)){
    Serial.println("Unable to find address for Device 0");
  }
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();
  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
  delay(500);
  // END display
}

void loop() {
  float temperature;
   
  /* Lit la température ambiante à ~1Hz */
  if (getTemperature(&temperature, true) != READ_OK) {
    Serial.println(F("Erreur de lecture du capteur"));
    // return;
  }
  // motionSensorCurrentState = digitalRead(MOTION_SENSOR_PIN);
  display.clearDisplay();
  display.setCursor(0,0);
  tempC = getExternalTemp();
  // display.println(tempC);
  // display.print(jj);
  // display.print (" ");
  // display.println(motionSensorCurrentState);
  display.println(temperature);

  display.display();
  // jj -= .32;
  xl += 2;
  delay(1000);
}