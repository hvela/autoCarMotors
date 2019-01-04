#include <Regexp.h>
//from here: https://github.com/nickgammon/Regexp installed in computer's Arduino library, so you'll have to do this

// https://www.youtube.com/watch?v=fE3Dw0slhIc - arduino libraries
#include <Wire.h> 
// the serial library?
#include <Adafruit_MotorShield.h>
#include "MSv2Common.h"
#include "MSv2Motors.h"


static const char SHIELD_PATTERN_START [] = "^MSv2Motors_[67]%x_";
static const char SPEED_PATTERN [] = "^MSv2Motors_[67]%x_speed_[1-4]_%x%x$";
//make sure you send hex bytes!
static const char DIR_PATTERN [] = "^MSv2Motors_[67]%x_direction_[1-4]_[0-2]$";

static const String NAME = "MSv2Motors";






//*************************************DC MOTORS**********************************



/*
 * gets the motor, then sets the speed. Speed is between 00 (0) and FF (255)
 * 
 * pattern: ^MSv2Motors_[67][0-9A-Fa-f]_speed_[1-4]_%x%x$
 *   - example: MSv2Motors_60_speed_1_10
 */
boolean setMotorSpeed(char *message, Adafruit_MotorShield *shield){
   uint8_t motorAddr = message[20] -'0';//make sure this is the right length
   uint8_t intSpeed = substr2num(message, 22, 24);//make sure this is the right length
   shield->getMotor(motorAddr)->setSpeed(intSpeed);
   return true;
}

/*
 * gets the motor, then sets the direction
 * 
 * see here: https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/library-reference#void-run-uint8-t-9-7
 * 
 * DIR_PATTERN: ^MSv2Motors_[67][0-9A-Fa-f]_direction_[1-4]_[0-2]$
 *   - example: MSv2Motors_60_direction_1_1
 */
boolean setMotorDir(char *message, Adafruit_MotorShield *shield){
   uint8_t motorAddr = message[24] - '0';
   switch(message[26]){
     case '0':
          shield->getMotor(motorAddr)->run(RELEASE); 
          break;
     case '1':
          shield->getMotor(motorAddr)->run(FORWARD);     
          break;
     case '2':
          shield->getMotor(motorAddr)->run(BACKWARD);
          break;
     default:
          return false;
   }
   return true;
}







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
boolean checkMSv2Motors(char *message, String *toWrite){
  MatchState ms;
  ms.Target(message);
  // converting to char array: https://www.arduino.cc/reference/en/language/variables/data-types/string/functions/tochararray/
  // regex from: https://github.com/nickgammon/Regexp also see the installed examples
  if(ms.Match(SHIELD_PATTERN_START) > 0){
    toWrite->concat(NAME);
    toWrite->concat(": ");
    //parse out which shield, set it as a variable
    int shieldInt = getMotorShield(message);
    if(shieldInt < 0){
       if(shieldInt == -1){
         //set toWrite to an error message saying this isn't a valid number
         *toWrite = String("That isn't a valid shield address.");
       }else if(shieldInt == -2){
         *toWrite = String("Shield not attached."); 
       }
    }else{
      if(ms.Match(SPEED_PATTERN) > 0){
        //parse out params
        //set speed on the shield
        if(setMotorSpeed(message, shields[shieldInt])){
          toWrite->concat("speed set success.");  
        }else{
          toWrite->concat("speed set fail.");
        }
      }else if(ms.Match(DIR_PATTERN) > 0){
        //set direction
        if(setMotorDir(message, shields[shieldInt])){
          toWrite->concat("direction set success.");
        }else{
          toWrite->concat("direction set failed.");
        }
      //ADD OTHER STUFF (SET SERVOS...)
        // note, people can put crap between the SHIELD_PATTERN_START and the parameter patterns, but this isn't really a problem
      }else{
        toWrite->concat("MotorShield: No matching command found.");
      }
    }
    return true;
  }else{
    return false;
  }
}
