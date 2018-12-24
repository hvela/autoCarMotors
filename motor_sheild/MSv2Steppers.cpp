#include "MSv2Common.h"
#include <Wire.h> 
// the serial library?
#include <Regexp.h>
#include <Adafruit_MotorShield.h>
#include "MSv2Steppers.h"

#include <AccelStepper.h>
#include <MultiStepper.h>

//general pattern: MSv2Steppers_(shield I2C address)_(command)_(stepper motor number)_(parameter)

//maybe add a group of motors? You can only have 10 at a time.

static const char STEPPER_PATTERN_START [] = "^MSv2Steppers_";
static const char MOVE_PATTERN [] = "^MSv2Steppers_[67]%x_move_[0-1]_%x%x%x_group_[0-9]{0,2}$";
//The end matches longs https://stackoverflow.com/questions/11243204/how-to-match-a-long-with-java-regex

static const char EXE_PATTERN [] = "^MSv2Steppers_execute_group_[0-9]{0,2}$";

static const String NAME = "MSv2Steppers";

//************************************STEPPER MOTORS*******************************************


/**
 * A customized AccelStepper that takes an Adafruit_StepperMotor.
 * 
 */
class MyAccelStepper: public AccelStepper
{
   public:
       MyAccelStepper(Adafruit_StepperMotor *myStepper):AccelStepper(0,0,0,0,0,false)
       {
         //MyStepper(0, 0, 0, 0, 0, false);
         _myStepper = myStepper;
       }
   protected:
       void step0(long step) override{
          if(_myStepper == NULL){
            AccelStepper::step0(step);
          }else{
            (void)(step);
            if(speed() > 0){
              _myStepper->onestep(FORWARD, DOUBLE);
            }else{
              _myStepper->onestep(BACKWARD, DOUBLE);            
            }          
          }
       }
    private:
       Adafruit_StepperMotor *_myStepper;
};


/*
 * Each MultiStepper can contain up to 10 objects. This class wraps MultiStepper to
 * handle these objects
 */
class Steppers: public MultiStepper{
  public:
    Steppers(){
      curStepperIndex = 0;
      //steppersIndexes [10][2] = new uint8_t [10][2];
    }
    
    /**
     * Adds a stepper motor to act in unison with the other bound stepper motors 
     * 
     * params:
     * stepperNum: the number
     * with 200 steps
     */
    void addStepper(uint8_t stepperNumb, uint8_t shield, uint16_t steps_per_rev = 200){
      if(curStepperIndex >= 9){
        //error, too many shields
      }else if(stepperNumb != 0 and stepperNumb!=1){
         //Invalid Stepper motor
      }else if(!shieldConnected(shield)){
         // shield not connected.
      }else if(getSavedStepperIndex(shield, stepperNumb) != -1){
        //error, shield already added
      }else{  
        Adafruit_MotorShield AFMS = *shields[shield];//parsed out by getMotorShield?
        static Adafruit_StepperMotor *myStep = AFMS.getStepper(steps_per_rev, stepperNumb);
        
        MyAccelStepper curStepper(myStep);// create a MyAccelStepper named curStepper
        MultiStepper::addStepper(curStepper);//super class's method
        curStepperIndex +=1;
        steppersIndexes[curStepperIndex][0] = shield;
        steppersIndexes[curStepperIndex][1] = stepperNumb;
      }
    }

    /**
     * Finds the index of stepper motor in the class's array that's on this shield,
     * and is this stepper motor. 
     * 
     * Returns: the index in the array if the motor is in the array, otherwise it returns -1
    */
    uint8_t getSavedStepperIndex(uint8_t shield, uint8_t stepperNumb){
      for(int index = 0; index <= curStepperIndex; index++){
        if(steppersIndexes[index][0] == shield 
          && steppersIndexes[index][1] == stepperNumb){
          return index; 
        }
      }
      return -1;
    }

    /**
     * Tells the motor to move moveAmount ticks relative to the position it's in when it's executed.
     */
    void setToMove(uint8_t shield, uint8_t stepperNumb, long moveAmount){
       setToMove(getSavedStepperIndex(shield, stepperNumb), moveAmount);
    }

    /**
     * Tells the motor to move moveAmount ticks relative to the position it's in.
     */
    void setToMove(uint8_t index, long moveAmount){
       moves[index] += moveAmount;
    }

    /**
     * Calls moveTo with the stored possitions
     */
    void moveTo(){
      long * posArr = getPos_resetMoves();
      MultiStepper::moveTo(posArr);
      delete[] posArr;
      //delete[]: http://www.cplusplus.com/reference/new/operator%20new[]/
    }

    /**
     * Gets the positon array after finding the motor. Creates the motor if it doesn't
     * exist.
     */
    long getPosition(uint8_t shield, uint8_t stepperNumb){
      uint8_t index = getSavedStepperIndex(shield, stepperNumb);
      if(index > -1){
         return getPosition(index);
      }else{
        //Motor doesn't exist.
      }
    }
    
    /**
     * gets the possition array that will be passed to moveTo after memcpy 
     */
     long getPosition(uint8_t index){
       if(index < 10){
          return stepperObjects[index]->currentPosition();//return a copy of this instead of the real thing.
       }else{
          //error, indexOutOfBounds
       }
     }
 
  private:
  //Adafruit_MotorShield addresses are uint8_t
  //stepperNumb is uint8_T
    uint8_t steppersIndexes [10][2];//[[shield, stepper], [shield, stepper],...]
    unsigned char curStepperIndex;
    long moves [10];//you pass this to moveTo. Before you do, you have to resize it with memcpy
    AccelStepper *stepperObjects [10];//replace positions with this so you can have overlapping groups
        
    /**
     * This will add a long[] array to the free store (heap). You MUST delete it with _____.
     * The array this returns will be passed to moveTo
     */
    long * getPos_resetMoves(){
      long * posArr = new long [curStepperIndex + 1];// need to put on "free store"
      for (int i=0; i < curStepperIndex; i++){
        posArr[i] = stepperObjects[i]->currentPosition() + moves[i];
        moves[i] = 0;
      }
      return posArr;
    }
};


//********************************MAIN********************************************

/*
 * 151473214816 is the number of unique combinations the 64 possible stepper motors can be 
 * combined into sets of 10 motors for Steppers objects. (combinations 64 choose 10 
 * without reuses or repeats)
 * 
 * Because That number isn't realistic, I set the number of possible groups to 100
 * which is more than enough for 99% of projects.
 */
Steppers *groups[16];



/*
 * Motor shield Stepper signals are of the format 
 * "MSv2Stepper_shield number_then the command_parameters"
 * see the constants at the top for the commands
 * 
 *   
 *   Remember, you NEED to de-reference toWrite with this: https://stackoverflow.com/questions/2229498/passing-by-reference-in-c
 
*/
boolean checkMSv2Steppers(char *message, String *toWrite){
  MatchState ms;
  ms.Target(message);
  Serial.println(message);
  // converting to char array: https://www.arduino.cc/reference/en/language/variables/data-types/string/functions/tochararray/
  // regex from: https://github.com/nickgammon/Regexp also see the installed examples
  if(ms.Match(STEPPER_PATTERN_START) > 0){
    //parse out which shield, set it as a variable
    Serial.println("match");//only works on the first one?
    if(ms.Match(MOVE_PATTERN) > 0){
      int shieldInt = getMotorShield(message);
      //parse out params
      if(shieldInt < 0){
        if(shieldInt == -1){
          //set toWrite to an error message saying this isn't a valid number
          *toWrite = String(NAME + ": That isn't a valid shield address.");
        }else if(shieldInt == -2){
          *toWrite = String(NAME + ": Shield not attached."); 
        }else{
          //unknown error
        }
      }else{
        /**
         * Parameters: char *message, [uint8_t stepperNumb, long moveAmount, int group]
         * returns nothing, but overwrites the above parameters with the parsed crap
         * 
         * parsing: "^MSv2Steppers_[67]%x_move_[0-1]_[\-\+]%x%x%x_group_%x%x$" - total len = 35
         */
        uint8_t group = substr2num(message, 34, 36);
        if(group < 16){       
          uint8_t stepperNumb = message[22] - 48; //48 is '0'.
          //TODO: change from substr2num. it returns a uint8_t, which is too
          long moveAmount = (substr2num(message, 24, 25) * 256) + substr2num(message, 25, 27);
          //24 in the neg or positive.
          //' for char, " for strings!!!
          //https://stackoverflow.com/questions/7808405/comparing-a-char-to-a-const-char
          if(*(message+23) == '-')
            moveAmount *= -1;
          
          if(groups[group] == NULL){
            groups[group] = new Steppers();
          }
          groups[group]->addStepper(stepperNumb, shieldInt);// Add some error checking here...
          groups[group]->setToMove(shieldInt, stepperNumb, moveAmount);
        }else{
          *toWrite = String(NAME + ": for memory purposes, only 16 groups allowed (0-15). "
                                    "An extra byte char is possible in the pattern for future expansion"
                                    "but for now don't use it.");
          return true;//kick out of this, because we have an error.
        }
      }
    }else if(ms.Match(EXE_PATTERN) > 0){
      //call run
      groups[substr2num(message, 26, 29)]->moveTo();
      *toWrite = String(NAME + ": Move success");
    }else{
      *toWrite = String(NAME + ": No matching command found.");
    }
    return true;
  }else{
    if(ms.Match(API_PATTERN) > 0){
      //If this matches you still return false becasue it's not exclusively for this.
      if(toWrite->length() > 0){
        toWrite->concat("_");
      }
      toWrite->concat(NAME);
    }
    return false;
  }
}
