#include <BMS-ny.h>

BMS BatteryControl;

const uint16_t SampleRate = 1024; // 16384 = 4096 kHz Sampling rate,1024 = 512 Hz Sampling rate
const uint16_t DutyUpdateRate = 1;

uint16_t ReadTime = 0;
const uint16_t Prescale = 32768/SampleRate;
void Interrupt();

//Moving avarage hack for display and SOC


float v1;
float v2;
float v3;
float v4;
float v5;

float c1;
float c2;
float c3;
float c4;
float c5;


void setup()
{
    Serial.begin(115200);
    BatteryControl.DisplayInit();
    delay(1000);
    BatteryControl.FileSetup("L5.csv","C1.txt"); //Write file, Read File -- Max 8 characters, 32 GB Sd-card
    delay(2000);
    BatteryControl.SdcardInit();  //MAX 8 characters, 32 GB Sd-card
    BatteryControl.ReadFromFileDutySetup();
    delay(2000);
    BatteryControl.DisCharge(5);
    delay(1000);
    //RTC
    BatteryControl.rtc.begin(true, 1,false,Prescale);            // initialize RTC: reset starting value, Mode 1 (16-bit counter)
    BatteryControl.rtc.enableCounter(1);
    BatteryControl.rtc.setPeriod(1);                            // set counter period
    BatteryControl.rtc.attachInterrupt(Interrupt); 
    delay(2000); 

     //pinMode(A0,OUTPUT);

    
}

void loop()
{
    BatteryControl.UpdateDisplay();
}

void Interrupt()                                 // interrupt when compare value is reached
{
  uint8_t source;
  source = BatteryControl.rtc.getIntSource();                  // check what caused the interrupt
  
  if (source == BatteryControl.rtc.INT_COMP0) 
  {
      
    BatteryControl.WriteToFile();
    ReadTime++;   
  }
  
  if((DutyUpdateRate > 0) && (source == BatteryControl.rtc.INT_COMP0)) //Read from File if(Read from file is active AND interrupt)
  {
    if(SampleRate < ReadTime)
    {
      
      //Voltage moving avarage
      v1 = BatteryControl.BatteryVoltage(A1);
      v2 = v1;
      v3 = v2;
      v4 = v3;
      v5 = v4;
      BatteryControl.MovingAvarageVoltage(v1,v2,v3,v4,v5);
       
      //Current moving avarage
      c1 = BatteryControl.ShuntCurrent();
      c2 = c1;
      c3 = c2;
      c4 = c3;
      c5 = c4;
      BatteryControl.MovingAvarageCurrent(c1,c2,c3,c4,c5);
      
      ReadTime = 0;


      Serial.print(BatteryControl.BatteryVoltage(A1));
      Serial.print(",");
      Serial.print(BatteryControl.ShuntCurrent());
      Serial.print('\n');
    
      //BatteryControl.Help3();
      BatteryControl.UpdateDuty();
    }
    else
    {
      ReadTime++;
    }
  }
} 


