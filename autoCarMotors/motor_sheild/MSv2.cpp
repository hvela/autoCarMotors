#include <Regexp.h>
//from here: https://github.com/nickgammon/Regexp installed in computer's Arduino library, so you'll have to do this

// https://www.youtube.com/watch?v=fE3Dw0slhIc - arduino libraries
#include <Wire.h> 
// the serial library?
#include <Adafruit_MotorShield.h>
#include "MSv2Common.h"
#include "MSv2Motors.h"
#include "MSv2Steppers.h"

static const String NAME = "MSv2";




//********************************MAIN********************************************

/*
 * motor shield signals are of the format "MSv2_shield number_then the command_parameters"
 * see the constants at the top for the commands
 * 
 * if the message is meant for a motor shield:
 *   - If the shield doesn't exist, create it and add it to shields
 *     - If there's not a shield connected with the corresponding address, throw an error
 *   - Run the function on the right shield
 *   - return true
 * else: 
 *   - return false
 *   
 *   Remember, you NEED to de-reference toWrite with this: https://stackoverflow.com/questions/2229498/passing-by-reference-in-c
 
*/
boolean checkMSv2(char *message, String *toWrite){
  if(strcmp(message,"APIs") == 0){
    //If this matches you still return false becasue it's not exclusively for this.
    if(toWrite->length() > 0){
      toWrite->concat("_");
    }
    toWrite->concat(NAME);
    return false;
  }else
    return checkMSv2Steppers(message, toWrite) || checkMSv2Motors(message, toWrite);
}
