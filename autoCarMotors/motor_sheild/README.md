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
- example, `MSv2Motors_60_speed_1_10`, will set the speed of the first DC motor on shield 0x60, to 10 (16).

### DC motor pattern end, direction

- The end of commands sent to control DC motors' direction must match this pattern:
  `direction_[1-4]_[0-2]$`

- The first part, `direction`, tells the program this command is for setting the motor's direction.
- The second part, `[1-4]`, tells which DC motor this command is for. Each shield can control up to 4 DC motors.

- The third and final part, `[0-2]` tells what direction the motor is going. [See here for Direction values](https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/using-dc-motors#run-the-motor-9-14)
  - If this parameter is 0, the direction is set to `RELEASE`
  - If this parameter is 1, the direction is set to `FORWARD`
  - If this parameter is 2, the direction is set to 'BACKWARD`

- example, `MSv2Motors_60_direction_1_1`, will set the direction of the first DC motor on shield 0x60 to 1 (FORWARD)

## Stepper motors pattern specifications

- Stepper motors are more complicated than DC motors. The code is based on the examples in the arduino library installed at: [Adafruit_Motor_Shield_V2_Library/examples/Accel_Multistepper/Accel_MultiStepper.ino](https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library/blob/master/examples/Accel_MultiStepper/Accel_MultiStepper.ino), but has been expanded and made dynamic.
- This code will require you to install the [AccelStepper library](https://www.airspayce.com/mikem/arduino/AccelStepper/classMultiStepper.html#a383d8486e17ad9de9f1bafcbd9aa52ee) if it's not yet installed.

- In summary, the code allows you to:
  - Set the number of steps each attached motors in a group should move (the destination).
    - This uses the functions from the AccelStepper library: `MultiStepper.addStepper` and `MultiStepper.moveTo`
  - Execute the instructions such that all attached motors arrive at their destinations at the same time.
    - This uses the function from the AccelStepper library: `MultiStepper.addStepper`

### Stepper motor pattern start:


- The beginning of all commands sent to control stepper motors must match this pattern:
  `^MSv2Steppers_`

- `MSv2Steppers` doesn't change, it just tells the program this command is for a stepper motor.
  - `^` is just an anchor saying the pattern must start here.

### Stepper motors pattern end, setting destination

- The end of commands sent to set stepper motors' final destination must obey this pattern:
  `[67]%x_move_[1-2]_[+-]%x%x%x_group_%x%x$`

- The first part, `[67]%x`; similar to DC motor commands, tells what motor shield the command is meant for. 
  - As above, valid values are hex numbers in the range 60 - 7F (96 - 127), where 70 is a broadcast address.

- The second part, `move`, just assures the program this is setting the direction

- The fourth part, `[+-]%x%x%x` is the number of steps for the motor to move.
  - The parameter can be be between -FFF to +FFF (-4095 - 4095)

- The fifth part, `group` tells the parser that the next parameter is a __group__.
  - Groups are groups of motors that can all move in unison when executed.

- The sixth and last part, `%x%x`, tells the group number.
  - __Currently you can only have up to 16 groups__ (00 - 0F), I left the extra value in because on devices with more memory, like the arduino mega, might need more than 16 groups, but for now I only allowed 16.

- Similar to DC motors, the `$` at the end is just an anchor that says the string must end at after these parts.
- For example, `MSv2Steppers_60_move_1_+0FF_group_00`, will add the first stepper motor on shield 0x60 to the 1st group (at position 0) if it hasn't been added yet, and tell it to move 255 steps in a positive direction.
- __note__: These commands will continue adding up until you execute them. If you send the above command twice it will move twice as far in the set time.

### Stepper motors pattern end, executing 

- The end of commands sent to move a group's motors to their destinations must match this pattern:
  `execute_group_%x%x$`

- This first part of this command, `execute`, tells the parser this is an execute command.
- The second part of this command, `group`, tells the parser that a group follows.

- The last part of this command, `%x%x`, tells the parser what group to execute.
  - These groups are the same groups set above in *setting destination*, they must be (00 - 0F) for memory purposes. 

- For example, `MSv2Steppers_execute_group_00` will move the motors in the first group, 00, to the destinations specified by the commands specified in *setting destination*
