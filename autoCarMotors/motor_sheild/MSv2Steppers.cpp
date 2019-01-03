#include "MSv2Common.h"
#include <Wire.h> 
// the serial library?
#include <Regexp.h>
#include <Adafruit_MotorShield.h>
#include "MSv2Steppers.h"

#include <AccelStepper.h>
#include <MultiStepper.h>

//general pattern: MSv2Steppers_(shield I2C address)_(command)_(stepper motor number)_(parameter)

//You should have int params telling where parts of the string start and stop.

/*TESTS:
 * MSv2Steppers_60_move_1_+0FF_group_00
 * MSv2Steppers_60_move_2_+0FF_group_00
 * 
 * MSv2Steppers_execute_group_00 
 * 
*/

static const char STEPPER_PATTERN_START [] = "^MSv2Steppers_";
static const char MOVE_PATTERN [] = "^MSv2Steppers_[67]%x_move_[1-2]_[+-]"
                                    "%x%x%x_group_%x%x$";
static const uint8_t MOVE_AMOUNT_END = 27;
static const uint8_t MOVE_AMOUNT_SPLIT = 25;
static const uint8_t MOVE_AMOUNT_START = 24;
static const uint8_t STEPPER_INDEX = 21;
static const uint8_t GROUP_START = 34;
static const uint8_t GROUP_END = 36;



static const char EXE_PATTERN [] = "^MSv2Steppers_execute_group_%x%x$";

static const uint8_t EXE_GROUP_START = 27;
static const uint8_t EXE_GROUP_END = 29;

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
         _myStepper = myStepper;
       }
       void setCurrentPosition(){
          AccelStepper::setCurrentPosition(0);
       }
       
       ~MyAccelStepper(){
         delete _myStepper;
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
 * Each MultiStepper can contain up to 10 MyAccelStepper objects. 
 * This class wraps MultiStepper to handle these objects
 */
class Steppers: public MultiStepper{
  public:
    Steppers():MultiStepper()
    {
      curStepperIndex = 0;

      //Initialize all the steppers to 0s.
      //[[shield, stepperNumb], [shield, stepperNumb]...]
      for (int stepperInc = 0; stepperInc < 10; stepperInc ++){
        for (int stepperSubInc = 0; stepperSubInc < 2; stepperSubInc ++){
          steppersIndexes [stepperInc][stepperSubInc] = 0;
        }
      }
    }
    
    ~Steppers(){
      delete[] steppersIndexes;
      delete[] stepperObjects;
      delete[] moves;
      //curStepperIndex = 0;
    }
    

    /**
     * Finds the index of stepper motor in the class's array that's on this shield,
     * and is this stepper motor. 
     * 
     * Returns: the index in the array if the motor is in the array, otherwise it returns 255
     * 
     * works
    */
    uint8_t getSavedStepperIndex(uint8_t shield, uint8_t stepperNumb){
      for(int index = 0; index <= curStepperIndex; index++){
        if(steppersIndexes[index][0] == shield 
          && steppersIndexes[index][1] == stepperNumb){
          return index; 
        }
      }
      return 255;
    }
    
    /**
     * Adds a stepper motor to act in unison with the other bound stepper motors 
     * 
     * params:
     * stepperNum: the number
     * with 200 steps
     * 
     * Something's wrong with this function. Sometimes it crashes without printing
     */
    void addStepper(uint8_t shield, uint8_t stepperNumb, uint16_t steps_per_rev = 200){
      //Serial.println("here");
      if(curStepperIndex >= 9){
        //error, too many shields
        Serial.println("too many steppers to add another.");
      }else if(stepperNumb != 1 and stepperNumb!=2){
         //Invalid Stepper motor
         Serial.print("addStepper invalid stepper number " );
         Serial.println(stepperNumb);         
      }else if(getSavedStepperIndex(shield, stepperNumb) != 255){
         //stepper already added.
         //This will happen in most cases.
         Serial.print("addStepper stepper already exists at ");
         Serial.println(getSavedStepperIndex(shield, stepperNumb));
         Serial.print("addStepper step index: ");
         Serial.println(curStepperIndex);
      }else if(!shieldConnected(shield)){
        // shield not connected.
         //this is actually redundant in the current form.
         Serial.println("shield not connected");
      }else{
        Adafruit_MotorShield *AFMS = shields[getMotorShield(shield)];//parsed out by getMotorShield?
        Adafruit_StepperMotor *myStep = AFMS->getStepper(steps_per_rev, stepperNumb);
        stepperObjects[curStepperIndex] = new MyAccelStepper(myStep);// create a MyAccelStepper named curStepper
        stepperObjects[curStepperIndex]->setCurrentPosition();
        stepperObjects[curStepperIndex]->setMaxSpeed(200.0);
        stepperObjects[curStepperIndex]->setSpeed(100);
        MultiStepper::addStepper(*stepperObjects[curStepperIndex]);//super class's method
        steppersIndexes[curStepperIndex][0] = shield;
        steppersIndexes[curStepperIndex][1] = stepperNumb;
        moves[curStepperIndex] = 0;
        
        curStepperIndex++;
      }
    }


    /**
     * Tells the motor to move moveAmount ticks relative to the position it's in when it's executed.
     * 
     */
    void setToMove(uint8_t shield, uint8_t stepperNumb, long moveAmount){
       setToMove(getSavedStepperIndex(shield, stepperNumb), moveAmount);
    }

    /**
     * Tells the motor to move moveAmount ticks relative to the position it's in.
     */
    void setToMove(uint8_t index, long moveAmount){
       moves[index] += moveAmount;//added += back in
    }

    /**
     * Calls moveTo with the stored possitions
     * 
     * works
     */
    void myMoveTo(){
      //Serial.println("move to " + String(curStepperIndex));//getting printed as m?
      long * posArr = getPos_resetMoves();
      /*
      for(int myMoveToIndex = 0; myMoveToIndex < curStepperIndex; myMoveToIndex++){
        Serial.print("myMoveTo posArr[");
        Serial.print(myMoveToIndex);
        Serial.print("]=");
        Serial.println(posArr[myMoveToIndex]);
      }
      */
      MultiStepper::moveTo(posArr);
      MultiStepper::runSpeedToPosition();
      delete[] posArr;
      //delete[]: http://www.cplusplus.com/reference/new/operator%20new[]/
    }
 
  private:
  //Adafruit_MotorShield addresses are uint8_t
  //stepperNumb is uint8_T
    uint8_t steppersIndexes [10][2];//[[shield, stepper], [shield, stepper],...]
    //changing from uint8_t to boolean is pointless because they're both 8bits to the OS.
    uint8_t curStepperIndex = 0;
    long moves [10] = {};//see getPos_resetMoves() and moveTo()
    MyAccelStepper *stepperObjects [10] = {};
    
    /**
     * This will add a long[] array to the free store (heap). You MUST delete it with delete[].
     * 
     * works
     */
    long * getPos_resetMoves(){
      long * posArr = new long [curStepperIndex + 1];// need to put on "free store"
      //memcpy(posArr, moves, curStepperIndex + 1);
      //TODO: You might need this if the data isn't copied!!!
      for (int pRI=0; pRI < curStepperIndex; pRI++){        
        //use the memcpy instead of this...
        posArr[pRI] = stepperObjects[pRI]->currentPosition() + moves[pRI];
        moves[pRI] = 0;//
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
static Steppers *groups[16] = {};

/**
   * parses char *message: 
   *      "^MSv2Steppers_[67]%x_move_[0-1]_[\-\+]%x%x%x_group_%x%x$" - total len = 35
   * For these parameters: [uint8_t stepperNumb, long moveAmount, int group]
   * 
   * Then uses these parameters to tell the Steppers object how much to move when an execute
   * is received.
   * 
 */
void setToMove(char *message, int shieldInt, String *toWrite){
  uint8_t group = substr2num(message, GROUP_START, GROUP_END);
  if(group < 16){//group isn't too large.
    uint8_t stepperNumb = message[STEPPER_INDEX] - '0'; //conversion to number
    //Multiplying by 256 here because substr2num can only handle 2 digits at a time
    long moveAmount = ((message[MOVE_AMOUNT_START] - '0') * 256) + 
                      substr2num(message, MOVE_AMOUNT_SPLIT, MOVE_AMOUNT_END);
    //24 in the neg or positive.
    //' for char, " for strings!!!
    //https://stackoverflow.com/questions/7808405/comparing-a-char-to-a-const-char
    if(message[23] == '-')
      moveAmount *= -1;
    if(groups[group] == NULL){
      groups[group] = new Steppers();
    }
    groups[group]->addStepper(shieldInt, stepperNumb);
    //Adds the stepper if it doesn't exist.
    groups[group]->setToMove(shieldInt, stepperNumb, moveAmount);
    //Move all the stepper motors the required amount.
  }else{//group too large for the groups array
    toWrite->concat(": for memory purposes, only 16 groups allowed (0-15). ");
      //"An extra byte char is possible in the pattern for future expansion"
      //"but for now don't use it.");
    //return true;//kick out of this, because we have an error.
  }
}


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
  // converting to char array: https://www.arduino.cc/reference/en/language/variables/data-types/string/functions/tochararray/
  // regex from: https://github.com/nickgammon/Regexp also see the installed examples
  if(ms.Match(STEPPER_PATTERN_START) > 0){
    toWrite->concat(NAME);
    if(ms.Match(MOVE_PATTERN) > 0){
      uint8_t shieldInt = getMotorShield(message) + 96; 
      //you have to add 96 because getMotorShield returns the index in the array;
      //while code in this file expects the I2C address
      if(shieldInt < 0){//error
        if(shieldInt == -1){
          //set toWrite to an error message saying this isn't a valid number
          toWrite->concat(": That isn't a valid shield address.");
        }else if(shieldInt == -2){
          toWrite->concat(": Shield not attached."); 
        }else{
          //unknown error
        }
      }else{//shield connected.
        setToMove(message, shieldInt, toWrite);
      }
    }else if(ms.Match(EXE_PATTERN) > 0){
      //call run
      groups[substr2num(message, EXE_GROUP_START, EXE_GROUP_END)]->myMoveTo();
      toWrite->concat(": Move success");
    }else{
      toWrite->concat(": No matching command found.");
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
