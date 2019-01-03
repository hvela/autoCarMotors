# Arduino MotorShield v2.3 hardware API

- Code in this directory allows a user to control an arduino's attached [motor shield v2.3](https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/overview) by passing it commands over the Serial interface (usb)

- Using these commands, the user can:
  - control DC motors
    - In the code, this works by passing Serial input that matches the specifications below to `checkMSv2Motors` from __MSv2Motors.cpp__
  - control Stepper motors
    - In the code, this works by passing Serial input that matches the specifications below to 'checkMSv2Steppers` from __MSv2Steppers.cpp__
  - control 1 servo (coming soon)
    - The code for this hasn't been written yet.
  - Learn what functionality this library has by sending "APIs" over the Serial port.
    - In the code, this works by  first adding the file's name, then passing Serial input to all attached libraries *check*... (checkMSv2Steppers and CheckMSv2Motors) functions.  These functions will then adding their names to the response. 

- This library was made with modularity in mind. `MSv2Motors` doesn't rely on `MSv2Steppers`, but they both rely on `MSv2Common`.

## DC motors pattern specifications

- All DC motor pattern have the same start, the speed and direction patterns have different endings.
- For more information on the underlying libraries used for setting DC motors see the [Adafruit Motor Shield V2 docs](https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/using-dc-motors)

### DC motor pattern start

- The beginning of all commands sent to control DC motors must match this pattern:
   `^MSv2Motors_[67]%x_`

- `MSv2Motors` doesn't change. This just tells the program this command is for a DC motor.
  - `^` is just an anchor saying the pattern must start here.

- The second part of this command ,`[67]%x`, corresponds to a valid I2C shield address.
  - Shield addresses must be a hex digit between 60 - 7F (96 - 127).
  - The default address is 60.
  - 70 is a broadcast address.
  - For more about shield addressing see [this tutorial](https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/install-headers)

### DC motor pattern end, speed

- To end of commands sent to control DC motors' speed must match this pattern:
  `speed_[1-4]_%x%x$`

- The first part, `speed`, tells the program this command is for setting the motor's speed.
- The second part, `[1-4]`, tells which DC motor this command is for. Each shield can control up to 4 DC motors.
- The third and final part, `%x%x`, tells how fast to set the motor. Values can be between 00 - FF.
  - For more info on how this works, see [the docs of the underlying function.](https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/using-dc-motors)
- `$` is just an anchor at end, saying the pattern must end here.
- example: `MSv2Motors_60_speed_1_10`

### DC motor pattern end, direction

- The end of commands sent to control DC motors' direction must match this pattern:
  `direction_[1-4]_[0-2]$`

- The first part, `direction`, tells the program this command is for setting the motor's direction.
- The second part, `[1-4]`, tells which DC motor this command is for. Each shield can control up to 4 DC motors.

- The third and final part, `[0-2]` tells what direction the motor is going. [See here for Direction values](https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/using-dc-motors#run-the-motor-9-14)
  - If this parameter is 0, the direction is set to `RELEASE`
  - If this parameter is 1, the direction is set to `FORWARD`
  - If this parameter is 2, the direction is set to 'BACKWARD`

- example: `MSv2Motors_60_direction_1_1`

## Stepper motors pattern specifications

- The beginning of all commands sent to control stepper motors must match this pattern:
  `^MSv2Steppers_`

- `MSv2Steppers` doesn't change, it just tells the program this command is for a stepper motor.
  - `^` is just an anchor saying the pattern must start here.

- Stepper motors are more complicated than DC motors. The 

### Stepper motors pattern end, setting 
