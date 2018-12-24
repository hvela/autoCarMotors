#include <Adafruit_MotorShield.h>
#include "MSv2Common.h"



Adafruit_MotorShield *shields [32];
// Initialized as all null
//https://stackoverflow.com/questions/2615071/c-how-do-you-set-an-array-of-pointers-to-null-in-an-initialiser-list-like-way
  // the above link described this initialization
  // shields holds pointer to the shield objects.
  // shields are addressed 0x60 to 0x7F for a total of 32 unique addresses.
  // In this array, [0] == address 0x60, [31] == address 0x7F

/*
 * converts a substring between A and B from message to a uint8_t
 * 
 * http://www.cplusplus.com/reference/cstring/strncpy/
 * 
 * Tested, it works
 */
uint8_t substr2num(char *message, int A, int B){
  char str[(B - A) + 1];
  strncpy(str, message + A, B - A);
  str[B-A] = '\0';
  return strtol(str, NULL, 16);
}

/** 
 * Checks if an I2C device is connected at shieldAddr. This effectively checks if a 
 * motor shield exists at that location.
 * 
 * Note, this only works when everything's imported for some reason.
 */
boolean shieldConnected(uint8_t shieldAddr){
  Wire.beginTransmission(shieldAddr);
  int end = Wire.endTransmission(true);
  //return shieldAddressValidator(shieldAddr); (A customization I added 
  //to the Adafruit_MotorShield.h library that doesn't work)
  return end == 0;
}

/*
 * Converts the message from the Serial port to its shield's int location 
 * in the shields array.
 * 
 * If a motor shield doesn't exist, it creates it before returning the int
 * 
 * Note: 0x70 is the broadcast
 * 
 * //https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/stacking-shields
 */
int getMotorShield(char *message){
// * https://stackoverflow.com/questions/45632093/convert-char-to-uint8-t-array-with-a-specific-format-in-c
// the above might help with the conversion

//pointers: https://stackoverflow.com/questions/28778625/whats-the-difference-between-and-in-c
   uint8_t addr = substr2num(message, 5,7);//make sure this is the right length
   //MSv2_60_speed_1_10
   if(addr < 96 || addr > 127){
     return -1;
   }else if(!shieldConnected(addr)){
     return -2;
   }else if(!shields[addr - 96]){
      //makes sure it's a null pointer before setting it
      //This describes the pointer magic here:
      //https://stackoverflow.com/questions/5467999/c-new-pointer-from-pointer-to-pointer#5468009
      shields[addr - 96] = new Adafruit_MotorShield;
      *shields[addr - 96] = Adafruit_MotorShield(addr);
      shields[addr - 96]->begin();
   }
   return (int)(addr - 96);
};