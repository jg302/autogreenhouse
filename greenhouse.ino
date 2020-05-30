
#include <dht.h>

dht DHT;

#define DHT11_PIN 7

int thermistorPin = A0;
int windowPin = 4;
int Vo;
float R1 = 1000; // value of R1 on board
float logR2, R2, T, vOut, tempRangeHigh, tempRangeLow;
float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741; //steinhart-hart coeficients for thermistor
const int kelvin = 273.15;
float averagedTemp;
bool windowOpen;
bool pumpRunning;
long counter;

// Parameters

int targetTemp = 25;
//int targetHumidity = 50;
int testRate = 20;
int liveRate = 240; // 4mins
int sampleRate = 2;
bool testMode = true;

void setup(){
  Serial.begin(9600);
  pinMode(windowPin, OUTPUT);
}

int getInterval() {
  if (!testMode) {
    return liveRate;
  }
  return testRate;
}

void closeWindow() {
  Serial.println("close window");
  digitalWrite(windowPin, LOW);
  windowOpen = false;
}

void openWindow() {
  Serial.println("open window");
  digitalWrite(windowPin, HIGH);
  windowOpen = true;
}

int seconds(int value) {
  return value * 1000; 
}

float kelvinify(float value) {
  return value + kelvin;
}

float deKelvinify(float value) {
  return value - kelvin;
}

void loop()
{
  if (counter < seconds(getInterval())) {
  // DHT 11 stuff
  int chk = DHT.read11(DHT11_PIN);
  Serial.print("Humidity = ");
  Serial.println(DHT.humidity);

  // END DHT 11 stuff
  // Therminsor calculations
  Vo = analogRead(thermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0); //calculate resistance on thermistor
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); // temperature in Kelvin
  T = T - 273.15; //convert Kelvin to Celcius

  averagedTemp = (averagedTemp + ((T + DHT.temperature) * 0.5)) * 0.5;
  
  Serial.print("Average temperature: "); 
  Serial.print(averagedTemp);
  Serial.println(" c"); 

  // End temperature reading

  Serial.println("*****");
  counter = counter + seconds(sampleRate);
  }

  // Logic checking time
  if (counter >= seconds(getInterval())) {
    Serial.print("LOGIC TIEM, window open? ");
    Serial.println(windowOpen);

    delay(seconds(sampleRate));
    // It's too hot, open if not already open
    if ((averagedTemp >= targetTemp)){
      Serial.println("it's too hot");
      if (!windowOpen) {
      openWindow();
      }
    }
    // Too cold, close window if open
    if (averagedTemp < targetTemp) {
      Serial.println("it's too cold");
      if (windowOpen) {
        closeWindow();
      }
    }
    delay(seconds(sampleRate));
    counter = 0;
  }
  
  delay(seconds(sampleRate));
}

