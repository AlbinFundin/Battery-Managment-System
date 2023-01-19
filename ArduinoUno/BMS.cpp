#include <BMS.h>

void PrintDigits(int digits);
#define TOP_VALUE 0X07FF;
#define PRESCALE 1;
constexpr float PWM_FREQUENCY = 60e3;  // Pwm kHz
constexpr uint16_t PERIOD = round(F_CPU / PWM_FREQUENCY); 

BMS::BMS()
{
    DDRB = B00000000;
    DDRB = B00000000;
    ChargeMode = false;
    DischargeMode = false;
}

BMS::~BMS()
{
    DDRB = B00000000;
    DDRB = B00000000;
    ChargeMode = false;
    DischargeMode = false;
}

void BMS::Idle()
{
    DDRB = B00000000;
    DDRB = B00000000;

    ChargeMode = false;
    DischargeMode = false;
}

void BMS::PWMSetup()
{
    DDRB  |= _BV(PB1);    // set pin as output
    TCCR1B = 0;           // stop timer
    TCCR1A = _BV(COM1A1)  // non-inverting PWN on OC1A
           | _BV(WGM11);  // mode 14: fast PWM, TOP = ICR1
    TCCR1B = _BV(WGM12)   
           | _BV(WGM13)   
           | _BV(CS10);   
    ICR1   = PERIOD - 1;  // period
    OCR1A  = PERIOD / 1.1;  // duty cycle
}

void BMS::Charge()
{
    //Turn off Discharge safety and wait 500 ms
    DDRB = B00000000;
    DischargeMode = false;
    delay(500);

    //Port 9 
    DDRB = B00000010;
    ChargeMode = true;
}

void BMS::Charge(float DutyProcent)
{
    //Turn off Discharge safety and wait 500 ms
    DDRB = B00000000;
    DischargeMode = false;
    delay(500);

    //Port 9 
    DDRB = B00000010;
    ChargeMode = true;
    BMS::PWMDutySetup(DutyProcent);
}

void BMS::Charge(float DutyProcent,uint8_t second,uint8_t minute)
{
    DDRB = B00000000;
    DischargeMode = false;
    delay(100);

    uint16_t Min_;
    Min_ = minute*60;
    Time = ClockRTC.get();
    Time = Time + second + Min_;
    
    //Port 9 
    DDRB = B00000010;
    ChargeMode = true;
    BMS::PWMDutySetup(DutyProcent);    
}

void BMS::CheckAlarm()
{   
   if(Time < ClockRTC.get())
   {
       ClockRTC.squareWave(ClockRTC.SQWAVE_NONE); 
   }
}

float BMS::BatteryVoltage(const uint8_t Pin)
{
    float AnalogValueGain = 0;
    float AnalogValue;
    float Gain = 1000;
    AnalogValueGain = BMS::ADCVoltage(5); //Pin 5
    //Ads620 gain 1000: Vout = 1000*(V2-V1)
    AnalogValue = AnalogValueGain/Gain;
    return AnalogValue;
}

float BMS::ADCVoltage(const uint8_t Pin)
{   
    uint16_t DigitalValue = 0;
    float AnalogValue = 0;
    const float Scale = 5/1023;
    DigitalValue = analogRead(Pin);
    AnalogValue = Scale*DigitalValue;
    return AnalogValue;
}

float BMS::ShuntCurrent()
{
    float Rshunt = 0.05; // 5 % tolerance, might need to adjust
    float AnalogValueGain = 0;
    float AnalogValue = 0;
    float ShuntCurrent;
    AnalogValueGain = BMS::ADCVoltage(4); //Read on pin 4
    //Vout/Vin = Gain, 24

    float Gain = 24;

    AnalogValue = AnalogValueGain/Gain;
    //V = R*I
    ShuntCurrent = AnalogValue/Rshunt;
    return ShuntCurrent;
}

void BMS::DisplayInit()
{
    u8x8.begin();
    u8x8.setPowerSave(0);
}

void BMS::UpdateDisplay()
{   char Temp_[3];

    Time = ClockRTC.get(); //Update time

    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.drawString(0,0,"Voltage: ");
    u8x8.drawString(0,1,"Current: "); 
    u8x8.drawString(0,2,"SOC %  : ");

    //Mode
    u8x8.drawString(0,3,"Mode   : ");
    if(ChargeMode)
    {
        u8x8.drawString(8,3,"Charge");
    }
    else if(DischargeMode)
    {
        u8x8.drawString(8,3,"D_Charge");
    }
    else if((!ChargeMode) && (!DischargeMode))
    
    {
        u8x8.drawString(8,3,"Idle   ");
    } 

    u8x8.drawString(0,4,"Time   : ");
    uint16_t Hour = hour();
    strcpy(Temp_, u8x8_u8toa(Hour, 2));
    u8x8.drawString(8,4,Temp_);

    u8x8.drawString(10,4,":");
    uint16_t Min = minute();
    strcpy(Temp_, u8x8_u8toa(Min, 2));
    u8x8.drawString(11,4,Temp_);

    u8x8.drawString(13,4,":");
    uint16_t Sec = second();
    strcpy(Temp_, u8x8_u8toa(Sec, 2));
    u8x8.drawString(14,4,Temp_);

    //Temp
    u8x8.drawString(0,5,"Temp   : ");
    uint16_t Temp = AmbientTemperature();

    strcpy(Temp_, u8x8_u8toa(Temp, 2));
    u8x8.drawString(8,5,Temp_);  
}

void BMS::ClockInit()
{   
    ClockRTC.begin();
   // setSyncProvider(ClockRTC.get); //Sync clock with RTC module
}

void BMS::SdcardInit()
{
    const int Sdpin = 4; //Chip select pin 
    pinMode(Sdpin,OUTPUT);
    digitalWrite(Sdpin,HIGH);
    if(!SD.begin(Sdpin))
    {
      Serial.println("Could not open SD-card");
      return;
    }
    Serial.println("Card working");

    Serial.println("File name to write to: ");
  
    char DirName[10];
    const char* Divider = "/";
    const char* Txt = ".csv";

   // const char* 

    strcpy(DirName,u8x8_u8toa(day(),2));
    strcat(DirName,Divider);
    strcat(DirName,u8x8_u8toa(month(), 2));
    strcat(DirName,Txt);
    SD.mkdir(DirName);
    BMSDir = DirName;
    
}

void BMS::ExternalInterruptInit(enum ExternalSamplingRate Rate)
{       
    switch(Rate)
    {
        case BMS_SAMPLING_1_HZ:
        ClockRTC.squareWave(ClockRTC.SQWAVE_1_HZ);
        //Ardiuno interrupt setup
        EICRA |= (1<< ISC11) | (1<<ISC10); //Interrupt on raising edge, INT1 pin 3: (1,1) rising, (0,1) both rising annd falling
        EIMSK |= (1<<INT1);
        sei(); // Enable interrupt
        break;

        case BMS_SAMPLING_1024_HZ:
        ClockRTC.squareWave(ClockRTC.SQWAVE_1024_HZ);
        //Ardiuno interrupt setup
        EICRA |= (1<< ISC11) | (1<<ISC10); //Interrupt on raising edge, INT1 pin 3: (1,1) rising, (0,1) both rising annd falling
        EIMSK |= (1<<INT1);
        sei(); // Enable interrupt
        break;

        case BMS_SAMPLING_NONE_HZ:
        ClockRTC.squareWave(ClockRTC.SQWAVE_NONE);
        //Ardiuno interrupt setup
        EICRA |= (1<< ISC11) | (1<<ISC10); //Interrupt on raising edge, INT1 pin 3: (1,1) rising, (0,1) both rising annd falling
        EIMSK |= (1<<INT1);
        sei(); // Enable interrupt
        break;
    }   
}


//Internal interrupt not done
/*
void BMS::InternalInterruptInit()
{
    StartMillis = millis();
}
void BMS::InternalInterrupt(enum InternalSamplingRate Rate)
{
    switch(Rate)
    {
        case TIMER_SAMPLING_50_HZ:
        const int Test = 50;
        CurrentMillis = millis();
        if()

    }
    
}
*/
void BMS::PWMDutySetup(float DutyProcent)
{
    float Scale = DutyProcent/100; //Float procent scaling
    OCR1A = ICR1*Scale;
}

float BMS::PWMFreqency()
{
    float CpuFreq = 16000;
    uint16_t TOP = TOP_VALUE;
    uint8_t N = PRESCALE;
    float Frq = CpuFreq/(N*(1+TOP)); //Atmel 328p formula for PWM freqency(KHz)
    return Frq;
}

void BMS::CalibrateRTC(long int t)
{
    time_t TIMER;
    TIMER = t;
    ClockRTC.set(TIMER);
}

uint16_t BMS::AmbientTemperature()
{
    return (ClockRTC.temperature()/4)-2; 
}

void PrintDigits(int digits)
{
    
    Serial.print(':'); //Utility function for DigitalClockDisplay()
    if(digits < 10)
        Serial.print('0');
    Serial.print(digits);
}

void BMS::DigitalClockDisplay()
{
    Serial.print(hour(Time));
    PrintDigits(minute(Time));
    PrintDigits(second(Time));
    Serial.print(' ');
    Serial.print(day(Time));
    Serial.print(' ');
    Serial.print(month(Time));
    Serial.print(' ');
    Serial.print(year(Time));
    Serial.println();
    Serial.print("Time set: ");
    Serial.print(timeStatus());
    Serial.println();
}

bool BMS::NotIdle()
{
    return (ChargeMode || DischargeMode);
}

void BMS::WriteToFile()
{
    Serial.println("Tester");
    SdFile = SD.open(BMSDir);
    SdFile = SD.open(BMSFile,FILE_WRITE);
    if(!SdFile)
    {
        SdFile.print(BatteryVoltage(5));
        SdFile.print(",");
        SdFile.print(ShuntCurrent());
        SdFile.print(",");
    }
    else Serial.print("Error writing to file");
    SdFile.close();
}

void BMS::ReadFromFile()
{
    SdFile = SD.open(BMSFileRead,FILE_READ);
    if(!SdFile)
    {
        //TBD
    }
    else Serial.print("Error reading from file");
    SdFile.close();
}