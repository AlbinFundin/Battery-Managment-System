#include<BMS-ny.h>

BMS::BMS()
{
    ChargeMode = false;
    DischargeMode = false;
    ReadLine = 0;
    m_Current = 0;
    m_Voltage = 0;

    REG_GCLK_GENDIV = GCLK_GENDIV_DIV(1) |          
                    GCLK_GENDIV_ID(4);           
  while (GCLK->STATUS.bit.SYNCBUSY);              

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |          
                     GCLK_GENCTRL_GENEN |        
                     GCLK_GENCTRL_SRC_DFLL48M |   
                     GCLK_GENCTRL_ID(4);          
  while (GCLK->STATUS.bit.SYNCBUSY);              

   //PWM port D7 -
  PORT->Group[g_APinDescription[7].ulPort].PINCFG[g_APinDescription[7].ulPin].bit.PMUXEN = 1;
  PORT->Group[g_APinDescription[6].ulPort].PMUX[g_APinDescription[6].ulPin >> 1].reg = PORT_PMUX_PMUXO_F;

  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         
                     GCLK_CLKCTRL_GEN_GCLK4 |     
                     GCLK_CLKCTRL_ID_TCC0_TCC1;   
  while (GCLK->STATUS.bit.SYNCBUSY);             

  REG_TCC0_WAVE |= TCC_WAVE_POL(0xF) |         
                    TCC_WAVE_WAVEGEN_DSBOTH;    
  while (TCC0->SYNCBUSY.bit.WAVE);           

  REG_TCC0_PER = 1000;         // Set the frequency to 25kHz
  while (TCC0->SYNCBUSY.bit.PER);               
  
  // PWM 0 % duty
  REG_TCC0_CC3 = 0;         
  while (TCC0->SYNCBUSY.bit.CC3);              
  
  REG_TCC0_CTRLA |= TCC_CTRLA_PRESCALER_DIV1 |    
                    TCC_CTRLA_ENABLE;            
  while (TCC0->SYNCBUSY.bit.ENABLE);              
}

BMS::~BMS()
{
    REG_TCC0_CC3 = 0;  
    ChargeMode = false;
    DischargeMode = false;
}

void BMS::Idle()
{
    REG_TCC0_CC3 = 0;  
    ChargeMode = false;
    DischargeMode = false;
}

void BMS::Charge(int DutyProcent)
{
    DischargeMode = false;
    delay(500);
    
    ChargeMode = true;
    uint16_t temp = DutyProcent*10;
    REG_TCC0_CC3 = temp;         // TCC0 CC3 - on D7
    while (TCC0->SYNCBUSY.bit.CC3);  
}

void BMS::DisCharge(int DutyProcent)
{
    ChargeMode = false;
    delay(500);

    DischargeMode = true;
    uint16_t temp = DutyProcent*10;
    REG_TCC0_CC3 = temp;         // TCC0 CC3 - on D7
    while (TCC0->SYNCBUSY.bit.CC3);  
}

float BMS::BatteryVoltage(const uint8_t Pin)
{
    float AnalogValueGain = 0;
    float AnalogValue;

    //Gain/2
    float Gain = 1.99;

    AnalogValueGain = BMS::ADCVoltage(A1); 

    AnalogValue = AnalogValueGain*Gain;
    return AnalogValue;
}

float BMS::ADCVoltage(const uint8_t Pin)
{   
    uint16_t DigitalValue = 0;
    float AnalogValue = 0;
    const float Scale = 3.3/1024;
    DigitalValue = analogRead(Pin);
    AnalogValue = Scale*DigitalValue;
    return AnalogValue;
}

float BMS::ShuntCurrent()
{
    float Rshunt = 0.05; // 5 % tolerance, might need to adjust
    //float AnalogValueGain = 0;
    float Offset = 1.3;
    float AnalogValue = 0;
    float ShuntCurrent;
    AnalogValue = BMS::ADCVoltage(A2); 
    //Vout/Vin = Gain, 11
    
    // float Gain = 11;
    AnalogValue = AnalogValue - Offset;
    // AnalogValue = AnalogValueGain/Gain;

    AnalogValue = AnalogValue/11;

    //V = R*I
    ShuntCurrent = AnalogValue/Rshunt;

    //Minus DC offset
    return ShuntCurrent;
}

void BMS::DisplayInit()
{
    u8x8.begin();
    u8x8.setPowerSave(0);
}

void BMS::UpdateSOC()
{
    //1 Hz Sampling
    float DeltaTime = 1/3600;
    //Battery rated
    float RatedCharge = 1/1500;
    //I*delta
    if(ChargeMode)
    {
        SOC += (RatedCharge)*(DeltaTime*ShuntCurrent());
    }
    else if(DischargeMode)
    {
        SOC -= (RatedCharge)*(DeltaTime*ShuntCurrent());
    }

    else return;
}

void BMS::UpdateDisplay()
{   char Temp_[3];

    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.drawString(0,0,"Voltage: ");
    String Voltage = String(m_Voltage);
    //snprintf(Temp_,sizeof Temp_,".3f",Voltage);
    //strcpy(Temp_, u8x8_u8toa(Voltage, 2));
    //u8x8.drawString(8,0,Voltage);
    u8x8.setCursor(8,0);
    u8x8.print(Voltage);


    u8x8.drawString(0,1,"Current: "); 
    String Current = String(m_Current);
    //strcpy(Temp_, u8x8_u8toa(Current, 2));
    //u8x8.drawString(8,1,Temp_);
    u8x8.setCursor(8,1);
    u8x8.print(Current);


    u8x8.drawString(0,2,"SOC %  : ");
     uint16_t SOC_temp = 0;
    strcpy(Temp_, u8x8_u8toa(SOC_temp, 2));
    u8x8.drawString(8,2,Temp_);

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
    uint16_t Hour = rtc.getHours();
    strcpy(Temp_, u8x8_u8toa(Hour, 2));
    u8x8.drawString(8,4,Temp_);

    u8x8.drawString(10,4,":");
    uint16_t Min = rtc.getMinutes();
    strcpy(Temp_, u8x8_u8toa(Min, 2));
    u8x8.drawString(11,4,Temp_);

    u8x8.drawString(13,4,":");
    uint16_t Sec = rtc.getSeconds();
    strcpy(Temp_, u8x8_u8toa(Sec, 2));
    u8x8.drawString(14,4,Temp_);

    //Temp //TDO
    u8x8.drawString(0,5,"Temp   : ");
    uint16_t Temp = 20;

    strcpy(Temp_, u8x8_u8toa(Temp, 2));
    u8x8.drawString(8,5,Temp_);  
}

void BMS::SdcardInit()
{
    const int Sdpin = 6; //Chip select pin 
    pinMode(Sdpin,OUTPUT);
    digitalWrite(Sdpin,HIGH);
    if(!SD.begin(Sdpin))
    {
      Serial.println("Could not open SD-card");
      return;
    }
    Serial.println("Sd-card working");

    if(SD.exists(FileNameWrite))
    {
       Serial.println("File already exists"); 
    }
    else
    {
    SdFile = SD.open(FileNameWrite,FILE_WRITE);
    SdFile.println("Voltage,Current,SOC");
    }
    SdFile.close();    
}

void BMS::FileSetup(char* Write,char* Read)
{
    strcat(FileNameWrite,Write);
    strcat(FileNameRead,Read);
}

bool BMS::NotIdle()
{
    return (ChargeMode || DischargeMode);
}

void BMS::WriteToFile()
{  
    float V1,C1;
    V1 = BatteryVoltage(A1);
    C1 = ShuntCurrent();
    Serial.print(V1);
    Serial.print(",");
    Serial.print(C1);
    Serial.print('\n');
    SdFile = SD.open(FileNameWrite,FILE_WRITE);
    if(SdFile)
    {
        
        SdFile.print(V1);
        SdFile.print(",");
        SdFile.print(ShuntCurrent());
        
        //Sdfile.print(SOC);
        SdFile.print("\n");

    }
    else Serial.print("Error writing to file");
    SdFile.close();
}

void BMS::ReadFromFileDutySetup()
{
    ReadLine = 0;
    SdFile = SD.open(FileNameRead,FILE_READ); 
    if(SdFile)
    {
        uint16_t temp = 0;
        while(SdFile.available())
        {
            ReadBuffer[temp] = SdFile.parseInt();
            if(ReadBuffer[temp] == -1)
            {
                break;
            }
            else temp++;
        }
    }  

}

void BMS::UpdateDuty()
{ 
    if(ReadBuffer[ReadLine] == -1)
    {
        ReadLine = 0;

        uint16_t DutyProcent = ReadBuffer[ReadLine];
        uint16_t temp = DutyProcent*10;
        REG_TCC0_CC3 = temp;         // TCC0 CC3 - on D7
        while (TCC0->SYNCBUSY.bit.CC3);  
        ReadLine++;
    }
    else
    {
    uint16_t DutyProcent = ReadBuffer[ReadLine];
    uint16_t temp = DutyProcent*10;
    REG_TCC0_CC3 = temp;         // TCC0 CC3 - on D7
    while (TCC0->SYNCBUSY.bit.CC3);  
    ReadLine++;
    }
}

void BMS::ReadFromFile()
{
    uint16_t temp = 0;
    SdFile = SD.open(FileNameRead,FILE_READ);
    if(SdFile)
    {   
       
        SdFile.seek(ReadLine); //2 bytes per line
        temp = SdFile.parseInt();
        if(temp == -1)
        {
            ReadLine = 0;
            SdFile.seek(ReadLine); 
            temp = SdFile.parseInt();
            Serial.println(temp);

            ReadLine++; //2 bytes per line
            ReadLine++;

        }
        else 
        {
        Serial.println(temp);
        ReadLine++; //2 bytes per line
        ReadLine++;
        }   
    }
    else Serial.print("Error reading from file");
    SdFile.close();
}

void BMS::MovingAvarageVoltage(const float& v1,const float& v2,const float& v3,const float& v4,const float& v5)
{
    m_Voltage = (v1+v2+v3+v4+v5)/5;
}

void BMS::MovingAvarageCurrent(const float& c1,const float& c2,const float& c3,const float& c4,const float& c5)
{
    m_Current = (c1+c2+c3+c4+c5)/5;
}


void BMS::Help()
{
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.drawString(0,0,"Voltage: ");
    u8x8.drawString(0,1,"Current: "); 
    u8x8.drawString(0,2,"SOC %  : ");
}

void BMS::Help2()
{
    Serial.println(FileNameRead);
}

void BMS::Help3()
{
    pinMode(A6,OUTPUT);

    digitalWrite(A6,!(digitalRead(A6)));
}

void BMS::Help4()
{
    
}
