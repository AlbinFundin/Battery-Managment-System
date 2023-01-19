#include <BMS.h>
BMS BatteryControl;

void setup() 
{
    //set baud rate
    Serial.begin(9600);  
    BatteryControl.ExternalInterruptInit(BatteryControl.BMS_SAMPLING_1_HZ);

    //Setup
    BatteryControl.DisplayInit();
    BatteryControl.ClockInit();
    BatteryControl.SdcardInit();
    BatteryControl.PWMSetup();
    //Mode:
    BatteryControl.Charge(100);
    //BatteryControl.DichargeCharge
    
    }

ISR(INT1_vect)
{
  cli();  
  BatteryControl.WriteToFile();
  //update SOC
  BatteryControl.ReadFromFile();
  Serial.print("Charging");
  sei();
}



void loop() 
{
  BatteryControl.UpdateDisplay();
  BatteryControl.CheckAlarm();
  delay(400);
}


