/* 
This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
It won't work with v1.x motor shields! Only for the v2's with built in PWM
control

For use with the Adafruit Motor Shield v2 
---->	http://www.adafruit.com/products/1438

Make sure your line separator is set to newline, not carrage return
*/

#include <Wire.h>
#include "MSv2Motors.h"
#include "MSv2Steppers.h"

String toWrite;
char *usb;
byte counter;
const static char* NAME = "motors";


void MyFunction(){
   if(Serial.available() > 0){
    //************read from serial**************
     while(Serial.available() > 0){
        char letter = Serial.read();
        if(letter == '\n')
          break;//handles the last '\n' char of arduino serial input
        usb[counter] = letter;
        counter ++;
        //the last character is a new line, destroying my regex.
        //how can I handle this?
        if(Serial.available() == 0){
          delay(10);
          //http://forum.arduino.cc/index.php?topic=22735.0
          //this waits for the next character if is hasn't yet arrived.
        }
     }
     usb[counter] = '\0';//terminates the string (replacing the newline)
  
     
     //***********do stuff with the serial input****************
     /*
     if(checkMSv2Motors(usb, &toWrite)){
       Serial.println(usb);
       //https://stackoverflow.com/questions/2229498/passing-by-reference-in-c
       Serial.println(toWrite);//passing the pointer
       //NAME + "_" + I wanted to add this, but they're different types
     }else 
     */
     if(checkMSv2Steppers(usb, &toWrite)){
       Serial.println(usb);
       Serial.println(toWrite);//passing the pointer
     }else if( strcmp(usb,"APIs") == 0 ){//tells what APIs are connected
       Serial.print( "APIs_");
       Serial.print(NAME);
       Serial.print("_");
       Serial.println(toWrite);
       //you can check something else here (another API's check...)
     }
     counter = 0;
     /*
      * note, this program assumes the entire message is in the buffer
      * when Serial.avaliable reads it, and that another message doesn't
      * get put into the buffer until after you've read this one.
      */
    toWrite.remove(0);
    Serial.flush();
   } 
}

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Adafruit Motorshield v2 - DC Motor test!");
  toWrite = "";
  counter = 0;
  usb = new char[50];
  Wire.begin();// super important for my motor shield library
  //char letter;

   //Serial.onReceive(MyFunction);
}

void loop() {
  MyFunction();
}

//TODO: Add a callback when Serial data is received
//https://playground.arduino.cc/Serial/AttachReceiveFunction
