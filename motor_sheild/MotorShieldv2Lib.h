#ifndef MotorShieldv2lib
#define MotorShieldv2lib

#if (ARDUINO >=100)
  #include <arduino.h>
#else
  #include "WProgram.h"
#endif
 
#include <Wire.h> 
#include <SoftwareSerial.h>
// the serial library?
// hardware vs software serial https://forum.arduino.cc/index.php?topic=407633.0
// maybe you don't need serial?

#include <Adafruit_MotorShield.h>
#include "MotorShieldv2Lib.h"

//pass


using namespace std;

/*
 * The only real reason for this class is encapsulation. 
 * Everything in this class is static.
 * Consider making this not a class any more and just having the checkMessage function.
 * The argument against this is you need the Stream, but you could pass it to checkMessage
 */ 
class MotorShield{
  Adafruit_MotorShield AFMS;// need to make a .begin for this?
  private:
    Stream *ser;
      // see this for why ser must be a pointer https://stackoverflow.com/questions/4296276/c-cannot-declare-field-to-be-of-abstract-type
    
    static const MotorShield *shields [32];
    // Initialized as all null in the cpp file
    //https://stackoverflow.com/questions/2615071/c-how-do-you-set-an-array-of-pointers-to-null-in-an-initialiser-list-like-way
      // the above link described this initialization
      // shields holds pointer to the shield objects.
      // shields are addressed 0x60 to 0x7F for a total of 32 unique addresses.
      // In this array, [0] == address 0x60, [31] == address 0x7F
    
  public:
    MotorShield(String address, Stream *ptrSer);
    static boolean checkMessage(String message);
    static const char SHIELD_PATTERN_START [];
    static const char SPEED_PATTERN [];
    static const char DIR_PATTERN [];   
};
#endif
