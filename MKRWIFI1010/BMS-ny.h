#include <U8x8lib.h>
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <RTCZero.h>
#ifndef BMS_H
#define BMS_H

class BMS {
    public:
    RTCZero rtc;

    BMS(); //Setup constructor at start
    ~BMS();
    void Idle();
    void Charge(int DutyProcent); //P4
    void DisCharge(int DutyProcent); //P5
    void UpdateDutyReady();

    void Sample(uint16_t SampleRate,uint16_t DutyUpdateRate);
    bool ReadDuty();
    void CheckAlarm(); 
    void Interrupt();

    float BatteryVoltage(const uint8_t Pin); //A0-A5 Pin
    float ADCVoltage(const uint8_t Pin); //A0-A5 for Pin
    float ShuntCurrent();

    void DisplayInit();
    void UpdateDisplay();
    void SdcardInit();
    void FileSetup(char* Write,char* Read);
    void SOCInit();
    void ClockInit();

    bool NotIdle();
    void WriteToFile();
    void ReadFromFileDutySetup();
    void UpdateDuty();
    void ReadFromFile();
    void UpdateSOC();

    void MovingAvarageVoltage(const float& v1,const float& v2,const float& v3,const float& v4,const float& v5);
    void MovingAvarageCurrent(const float& c1,const float& c2,const float& c3,const float& c4,const float& c5);

    void Help();
    void Help2();
    void Help3();
    void Help4();

    private:
    U8X8_SH1106_128X64_NONAME_HW_I2C u8x8;
    bool ChargeMode;
    bool DischargeMode;
    File SdFile;
    int ReadBuffer[35];
    char FileNameWrite[8];
    char FileNameRead[8];
    uint16_t ReadLine;
    uint16_t m_Duty;
    float SOC;
    float m_Voltage;
    float m_Current;

};
#endif
