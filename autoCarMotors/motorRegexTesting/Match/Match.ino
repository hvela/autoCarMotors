// Shows how to run three Steppers at once with varying speeds
//
// Requires the Adafruit_Motorshield v2 library 
//   https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library
// And AccelStepper with AFMotor support 
//   https://github.com/adafruit/AccelStepper

// This tutorial is for Adafruit Motorshield v2 only!
// Will not work with v1 shields

/*
 * Have one MultiStepper object with all attached stepper motors
 * connected to it. This will require you to save what position the stepper motors were in
 * in the MultiStepper array and tie it to the the shield and stepper number so you can  
 * access it.
 */

#include <Wire.h>
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>
#include <MultiStepper.h>
#include "MSv2Common.h"

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
          Serial.print("setCurrentPosition currentPosition=");
          Serial.println(AccelStepper::currentPosition());
       }
       /*
       ~MyAccelStepper(){
         delete _myStepper;
       }
       */
   protected:
       void step0(long step) override{
          if(_myStepper == NULL){
            AccelStepper::step0(step);
          }else{
            (void)(step);
            //Serial.print("step0 speed:");
            //Serial.println(speed());
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
      
      //steppersIndexes = new uint8_t [10][2];
    }
    
    ~Steppers(){
      delete[] steppersIndexes;
      delete[] stepperObjects;
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
      //Serial.print("getSavedStepperIndex, curStepperIndex=");
      //Serial.println(curStepperIndex);
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
         //Serial.print("addStepper stepper already exists at ");
         //Serial.println(getSavedStepperIndex(shield, stepperNumb));
         //Serial.print("addStepper step index: ");
         //Serial.println(curStepperIndex);
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
        //Serial.print("add stepper shield = ");
        //Serial.println(shield);
        steppersIndexes[curStepperIndex][1] = stepperNumb;
        moves[curStepperIndex] = 0;
        
        //Serial.print("add stepper stepperNumb = ");
        //Serial.println(stepperNumb);
        curStepperIndex++;
        //Serial.print("setCurrentPosition currentPosition=");
        //Serial.println(AccelStepper::currentPosition());
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
       /*
       Serial.print("setToMove moves[index] =");
       Serial.println(moves[index]);
       Serial.print("setToMove index =");
       Serial.println(index);//this is 255, which is wrong
       */
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
    uint8_t curStepperIndex;
    long moves [10];//see getPos_resetMoves() and moveTo()
    MyAccelStepper *stepperObjects [10];
    
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

        /*
        Serial.print("getPos_resetMoves Moves[");
        Serial.print(pRI);
        Serial.print("]= ");
        Serial.println(moves[pRI]);
        Serial.print("currentPosition = ");
        Serial.println(stepperObjects[pRI]->currentPosition());
        Serial.print("pRI = ");
        Serial.println(pRI);
        */
        //use the memcpy instead of this...
        posArr[pRI] = stepperObjects[pRI]->currentPosition() + moves[pRI];
        moves[pRI] = 0;//
      }
      
      return posArr;
    }
};

Steppers *myStep;

void setup()
{  
  Wire.begin();
  Serial.begin(9600);
  //delete myStep;
  //delete[] shields;
  /*
  for(int shieldsIndex = 0;  shieldsIndex < sizeof(shields)/sizeof(shields[0]); shieldsIndex ++){
    shields[shieldsIndex] = nullptr;
  }
  */
  /*
   * The uno's reset button properly resets the sketch, but the Motor shield's reset button doesn't
   */
  
  //delete[] groups;
  
  
  myStep = new Steppers();
  //myStep->addStepper(0x60, 1);
  myStep->addStepper(0x60, 2);
  myStep->addStepper(0x60, 1);
}

void loop()
{
  //myStep->setToMove(0x60, 1, 200);
  myStep->setToMove(0x60, 2, 100);
  myStep->setToMove(0x60, 1, 200);
  myStep->myMoveTo();
  
  delay(5000);
  Serial.println("ho");
  Serial.flush();
}
