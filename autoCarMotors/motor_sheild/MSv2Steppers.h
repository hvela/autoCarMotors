#ifndef MSv2Steppers
#define MSv2Steppers

#if (ARDUINO >=100)
  #include <Arduino.h>
#else
  #include "WProgram.h"
#endif

#include "MSv2Common.h"

using namespace std;
boolean checkMSv2Steppers(char *message, String *toWrite);

#endif
