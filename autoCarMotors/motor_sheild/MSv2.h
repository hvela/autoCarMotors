#ifndef MSv2
#define MSv2

#if (ARDUINO >=100)
  #include <Arduino.h>
#else
  #include "WProgram.h"
#endif

using namespace std;

#include "MSv2Steppers.h"
#include "MSv2Common.h"
#include "MSv2Motors.h"

/*
 * Take a message and a Stream (Serial object)
 *   - the message was received from the stream
 *   - The stream will send a message back if it has to (error code...)
 */
boolean checkMSv2(char *message, String *toWrite);


#endif
