#include <TimeLib.h>
#include <DS3232RTC.h>
#include <U8x8lib.h>
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <TimerOne.h>
#ifndef BMS_H
#define BMS_H

class BMS {
    public:
    enum ExternalSamplingRate
    {
        BMS_SAMPLING_1_HZ,
        BMS_SAMPLING_1024_HZ,
        BMS_SAMPLING_NONE_HZ  //Check cpp file to add more Sampling freqs
    };

    enum InternalSamplingRate
    {
        TIMER_SAMPLING_50_HZ,
        TIMER_SAMPLING_100_HZ,
        TIMER_SAMPLING_1000_HZ
    };

    BMS(); //Setup constructor at start
    ~BMS();
    void Idle();
    void PWMSetup();
    void DichargeCharge();
    void DischargeCharge(float DutyProcent);
    void DischargeCharge(float DutyProcent,uint8_t second,uint8_t minute); //Interrupt enable
    void Charge();
    void Charge(float DutyProcent);
    void Charge(float DutyProcent,uint8_t second,uint8_t minute); //Interrupt enable
    void CheckAlarm(); //Help function for interrupt 
    float BatteryVoltage(const uint8_t Pin); //A0-A5 Pin
    float ADCVoltage(const uint8_t Pin); //A0-A5 for Pin
    float ShuntCurrent();
    //void ADCInit(static const uint8_t Pin);
    void DisplayInit();
    void UpdateDisplay();
    void ClockInit();
    void SdcardInit();
    void SOCInit();
    void ExternalInterruptInit(enum ExternalSamplingRate Rate); //Enable external interrupt SQW from RTC
    void InternalInterruptInit();
    void InternalInterrupt(enum InternalSamplingRate Rate);
    void PWMDutySetup(float DutyProcent); //Set PWM duty as procent 
    float PWMFreqency();
    void CalibrateRTC(long int t); //Epoch & Unix time stamp
    uint16_t AmbientTemperature(); 
    void DigitalClockDisplay(); //Prints date and time
    bool NotIdle();
    void WriteToFile();
    void ReadFromFile();


    private:
    U8X8_SH1106_128X64_NONAME_HW_I2C u8x8;
    DS3232RTC ClockRTC;
    bool ChargeMode;
    bool DischargeMode;
    time_t Time;
    File SdFile;
    //time_t TimeFuture;
    char* BMSFile;
    char* BMSFileRead;
    char* BMSDir;
    uint16_t ReadLine;
    unsigned long StartMillis;
    unsigned long CurrentMillis;
};
#endif



