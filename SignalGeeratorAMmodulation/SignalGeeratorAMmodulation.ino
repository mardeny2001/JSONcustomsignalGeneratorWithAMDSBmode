#include <Arduino_JSON.h>
#include <EEPROM.h>

#define Set_Mode 4
#define MOD_mode 5

#define ADDR_FRE_EEPROM 0
#define ADDR_SP_EEPROM  4
#define ADDR_CH0_EEPROM 68
#define ADDR_CH1_EEPROM 132

#define FLOAT_ARRAY_SIZE 16

 
#define ch0_pin 25
#define ch1_pin 26

String Text_set;

float Freq=100;        // in Killo Hertz
float sample_period=5;   // in milli sec
float channel0[FLOAT_ARRAY_SIZE];
float channel1[FLOAT_ARRAY_SIZE];

/*
Valid Input:
{"Freq":15,"sample_period":0.3125,"ch0":[0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.1],"ch1":[0.0,0.2,0.4,0.5,0.6,0.8,0.9,1.0,0.0,0.17,0.32,0.433,0.4924,0.433,0.32,0.17]}*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  EEPROM.begin(4000);

  //Restore Last DATA

  EEPROM.get(ADDR_FRE_EEPROM,Freq);
  EEPROM.get(ADDR_SP_EEPROM,sample_period);

  Serial.println("################## Last Input Data ##################");
  Serial.println("Freq_Last ( kHz ): "+ (String)Freq);
  Serial.println("sample_period_Last (millisec ): " + (String)sample_period);
  Serial.print("Last Channel0  Data: ");
  for (int i = 0; i < FLOAT_ARRAY_SIZE; i++) {
    
    EEPROM.get(ADDR_CH0_EEPROM + (i * sizeof(float)), channel0[i]);
    Serial.print(channel0[i]);
    Serial.print(" ");

  }
  Serial.println();

  Serial.print("Last Channel1  Data: ");
  for (int i = 0; i < FLOAT_ARRAY_SIZE; i++) {
    
    EEPROM.get(ADDR_CH1_EEPROM + (i * sizeof(float)), channel1[i]);
    Serial.print(channel1[i]);
    Serial.print(" ");

  }
  Serial.println();
  Serial.println("#####################################################");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(Set_Mode) == HIGH){

    Serial.println("Set up Mode!");

    //Wait For JSON!
    while(!Serial.available()){
      if ( digitalRead(Set_Mode) == LOW ) {break;}
    }
    while(Serial.available()){

      Text_set = Serial.readString();
      JSONVar parsedText = JSON.parse(Text_set); // Rename variable here

      //Inteprate JSON DATA into signal generator data
      Freq = (float)(double)parsedText["Freq"]; 
      sample_period = (float)(double)parsedText["sample_period"]; 
      for (int i = 0 ; i < FLOAT_ARRAY_SIZE ; i++){
        channel0[i] = (float)(double)parsedText["ch0"][i]; 
        channel1[i] = (float)(double)parsedText["ch1"][i]; 
      }

      //Print incomming data for validation
      Serial.println("Freq ( kHz ): " + (String)Freq);
      Serial.println("Sample Period ( millisec ): " + (String)sample_period);
      Serial.print("Channel0: ");
      for (int i=0 ; i< 16 ; i++){
        Serial.print(channel0[i]);
        Serial.print(" ");
      }
      Serial.println();
      Serial.print("Channel1: ");
      for (int i=0 ; i< 16 ; i++){
        Serial.print(channel1[i]);
        Serial.print(" ");
      }
      Serial.println();
    }

    //Store Data into Memory
    EEPROM.put(ADDR_FRE_EEPROM, Freq);
    EEPROM.put(ADDR_SP_EEPROM, sample_period);
    for (int i = 0; i < FLOAT_ARRAY_SIZE; i++) {
      EEPROM.put(ADDR_CH0_EEPROM + (i * sizeof(float)), channel0[i]);
    }
    for (int i = 0; i < FLOAT_ARRAY_SIZE; i++) {
      EEPROM.put(ADDR_CH1_EEPROM + (i * sizeof(float)), channel1[i]);
    }
    EEPROM.commit();

    //Validate Storage
    float freq;
    float SP;
    EEPROM.get(ADDR_FRE_EEPROM,freq);
    EEPROM.get(ADDR_SP_EEPROM,SP);
    Serial.println("##################Validation of memory storage##################");
    Serial.println("Freq_Stored ( kHz ): "+ (String)freq);
    Serial.println("Sample Period Stored ( MilliSec. ): "+ (String)SP);
    Serial.print("Stored Channel0  Data: ");
    for (int i = 0; i < FLOAT_ARRAY_SIZE; i++) {
      float value;
      EEPROM.get(ADDR_CH0_EEPROM + (i * sizeof(float)), value);
      Serial.print(value);
      Serial.print(" ");
    }
    Serial.println();
    Serial.print("Stored Channel1  Data: ");
    for (int i = 0; i < FLOAT_ARRAY_SIZE; i++) {
      float value;
      EEPROM.get(ADDR_CH1_EEPROM + (i * sizeof(float)), value);
      Serial.print(value);
      Serial.print(" ");
    }
    Serial.println();
  }
  else{
    Serial.println("Modulation Mode!");

    //Check for Modulation Option digitalRead(MOD_mode)
    if ( digitalRead(MOD_mode) == HIGH){
      
      double sample_period_Carr = 1.0 / (2.0 * Freq);

      while (digitalRead(Set_Mode) == LOW && digitalRead(MOD_mode) == HIGH) {
        for (int i = 0; i < FLOAT_ARRAY_SIZE; i++) {

          for (int j = 0 ; j < 5 ; j++){
            double angle = 2.0 * M_PI * Freq * sample_period_Carr * j ;
            double signal0 =  (channel0[i]+1) * (cos(angle)) * 0.5;
            double signal1 =  (channel1[i]+1) * (cos(angle)) * 0.5;
            dacWrite(ch1_pin, 255/2 * ( signal1 + 1 ));
            dacWrite(ch0_pin, 255/2 * ( signal0 + 1 ));
            delayMicroseconds(sample_period_Carr * 1000);
          }
          
        }
}

    }
    else{

      while (digitalRead(Set_Mode) == LOW && digitalRead(MOD_mode) == LOW ){
        for (int i=0; i < FLOAT_ARRAY_SIZE; i++){
          
          dacWrite(ch1_pin, 255/2 *channel1[i]);
          dacWrite(ch0_pin, 255/2 *channel0[i]);
            
          delayMicroseconds(uint32_t(sample_period*1000));
            
        }
      }
      
    }
  }

}
