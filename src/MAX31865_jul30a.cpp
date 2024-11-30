//############     Libraries declaration     ############//
#include "Arduino.h"
#include "Wire.h"
#include <TM1637.h>
#include <Adafruit_MAX31865.h>

//############      Settings (pinout and probe mean number)     ############//
#define RREF (const float) 430.0                                 //Resistor value as MAX31865 reference    
#define RNOMINAL (const float) 100.0                            //PT100 nominal resistance at 0C
#define MEASURE_NUMBER (const int) 10                           //Number of measures to calc mean
#define MAX31865_RDY_PIN (const int) 21
#define MAX31865_CS_PIN (const int) 20      
#define DISPLAY_CLK_PIN (const int) 15
#define DISPLAY_DIO_PIN (const int) 16

bool readAvailable = 0;                                         //Flag to indicate a temp measure is available to read


Adafruit_MAX31865 tempProbe = Adafruit_MAX31865(MAX31865_CS_PIN);  //SPI instance with CS pin especified to manage temp probe
TM1637 disp(DISPLAY_CLK_PIN, DISPLAY_DIO_PIN);                  //Digital serial interface to manage display

//############     Function definition declaration     ############//
float calcMean(float* arr, int size);
void setupDisplay();
void setupMAX31865();
void dataAvailable();

void setup() {
  pinMode(MAX31865_RDY_PIN, INPUT);   //Define interrupt input pin
  attachInterrupt(digitalPinToInterrupt(MAX31865_RDY_PIN), dataAvailable, FALLING);
  setupDisplay();   //Startup display config
  setupMAX31865();   //Temp sensor start config
  delay(400);
  tempProbe.temperature(RNOMINAL, RREF);  //Dummy first reading (TESTTTTTT)

}

void loop() {
  static int a = 0;
  static float tempArray[MEASURE_NUMBER] = { 0 };                        //Array to store temp readings

  if (readAvailable) {                  
    readAvailable = 0;
    tempArray[a] = tempProbe.temperature(RNOMINAL, RREF);    //Fill array in pos 'a' with scaled temp reading
    a++;                                                  //Count un position in array for next iteration
    if (a == MEASURE_NUMBER) {                            //When array filled (position == array size)
      a = 0;
      float filteredTemp = calcMean(tempArray, MEASURE_NUMBER); //Calc filled temp array arithmetic  mean 
      disp.display(filteredTemp);
    }
  }
}

void setupDisplay() {
  disp.begin();
  disp.setBrightnessPercent(1);
  disp.display("LOAD");
  return;
}

void setupMAX31865() {
  tempProbe.begin(MAX31865_3WIRE);        //Could use 2/3/4 wire resistance measurement method (kelvin)
  tempProbe.enable50Hz(1);                //50 Hz filter avoid electrical noise from AC source
  return;
}


float calcMean(float* arr, int size){
  float sum = 0;
  float result = 0;
  if (size == 0) { return 0; }
  for (int i = 0; i < size; i++) {
    sum += arr[i];
  }
  result = (sum / size);
  return result;
}

void dataAvailable(){  //RAM stored funct to manage new temp available and init reading 
  readAvailable = 1;
}
