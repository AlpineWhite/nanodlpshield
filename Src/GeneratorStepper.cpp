#include "GeneratorStepper.h"
#include "Config.h"

#include <wiringPi.h>
#include <math.h>
#include "TMCStepper.h"

// ---------------------------------------------------------------------------------
//                                  Setup functions
// ---------------------------------------------------------------------------------

//
// constructor for the stepper class
//
GeneratorStepper::GeneratorStepper()
{
  //
  // initialize constants
  /*
  Constants for calculation:
  Tref=2^24/fclk=2^24/12000000=t = (65536/46875)*sec ~=1.39810=t
  for accel ta^2 = 2^24/(fclk^2)=ta^2 = (33554432/2197265625)*sec^2/cyc ~=.15271=ta2
  */


  //
  stepPin = STEP_PIN;
  directionPin = DIR_PIN;
  stepsPerRevolution = STEPS_PER_REV*MICROSTEP_SET;
  stepsPerMillimeter = STEPS_PER_MM;
  currentPosition_InSteps = 0;
  desiredSpeed_InStepsPerSecond = DEFAULT_SPEED*STEPS_PER_MM;
  acceleration_InStepsPerSecondPerSecond = DEFAULT_ACCELERATION;
  currentStepPeriod_InUS = 0.0;
  neg_travel = 0;
  pos_travel = 165;
}

void GeneratorStepper::rampSettings(double vstart, double a1, double v1, double amax, double vmax, double dmax, double d1, double vstop)
{

}

//
// connect the stepper object to the IO pins
//  Enter:  stepPinNumber = IO pin number for the Step
//          directionPinNumber = IO pin number for the direction bit
//          enablePinNumber = IO pin number for the enable bit (LOW is enabled)
//            set to 0 if enable is not supported
//
void GeneratorStepper::connectToPins(byte stepPinNumber, byte directionPinNumber)
{
  //
  // remember the pin numbers
  //
  stepPin = stepPinNumber;
  directionPin = directionPinNumber;
  //
  // configure the IO bits
  //
  pinMode(stepPin, INPUT);
  pinMode(directionPin, INPUT);
  driver.begin();
  driver.rms_current(RMS_A, HOLD_MULT); // Set stepper current to 1105mA. The command is the same as command TMC2130.setCurrent(600, 0.11, 0.5);
  driver.en_pwm_mode(0);                //disable stealthchop
  driver.multistep_filt(0);             //normal multistep filtering
  driver.shaft(1);                      //motor direction ccw
  driver.small_hysteresis(0);           //step hysteresis set 1/16
  driver.stop_enable(0);                //no stop motion inputs
  driver.direct_mode(0);                // set normal operation
  driver.RAMPMODE(0);                   //ramp generator mode

}


// ---------------------------------------------------------------------------------
//                     Public functions with units in millimeters
// ---------------------------------------------------------------------------------


//
// set the number of steps the motor has per millimeter
//
void GeneratorStepper::setStepsPerMillimeter(float motorStepPerMillimeter)
{
  driver.microsteps(motorStepPerMillimeter);
}



//
// get the current position of the motor in millimeter, this functions is updated
// while the motor moves
//  Exit:  a signed motor position in millimeter returned
//
float GeneratorStepper::getCurrentPositionInMillimeters()
{
  return((float)currentPosition_InSteps / stepsPerMillimeter);

}



//
// set current position of the motor in millimeter, this does not move the motor
//
void GeneratorStepper::setCurrentPositionInMillimeters(float currentPositionInMillimeter)
{
  int32_t setPos = driver.XACTUAL();
  // need to set to hold mode not to move the motor
  driver.RAMPMODE(3);
  driver.XACTUAL(setPos);
  driver.RAMPMODE(0);
  //currentPosition_InSteps = (long) round(currentPositionInMillimeter *
			   // stepsPerMillimeter);
}



//
// set the maximum speed, units in millimeters/second, this is the maximum speed
// reached while accelerating
// Note: this can only be called when the motor is stopped
//  Enter:  speedInMillimetersPerSecond = speed to accelerate up to, units in
//          millimeters/second
/*
  time ref t = 1.34218 sec = 524288s/390625
  1/t = .74506/sec
  V = uSteps/t  -> uSteps * sec/1.34218
  1.34218*V = uSteps/sec

  example: 
  MICROSTEP_SET = 256
  1mm = 100 steps
  STEPS_PER_MM = 25600 usteps/mm
  for 1mm/s: STEPS_PER_MM * t = 34359.808 = V
  
*/
//
void GeneratorStepper::setSpeedInMillimetersPerSecond(float speedInMillimetersPerSecond)
{
  desiredSpeed_InStepsPerSecond = speedInMillimetersPerSecond * stepsPerMillimeter;
}



//
// set the rate of acceleration, units in millimeters/second/second
// Note: this can only be called when the motor is stopped
//  Enter:  accelerationInMillimetersPerSecondPerSecond = rate of acceleration,
//          units in millimeters/second/second
//
/*
  time ref ta2 = .0140737 sec^2
  A = uSteps / ta2
  .0140737 * A = uSteps / sec^2

*/
void GeneratorStepper::setAccelerationInMillimetersPerSecondPerSecond(
                      float accelerationInMillimetersPerSecondPerSecond)
{

    acceleration_InStepsPerSecondPerSecond =
      accelerationInMillimetersPerSecondPerSecond * stepsPerMillimeter;
}



//
// home the motor by moving until the homing sensor is activated, then set the
// position to zero, with units in millimeters
//  Enter:  directionTowardHome = 1 to move in a positive direction, -1 to move in
//             a negative directions
//          speedInMillimetersPerSecond = speed to accelerate up to while moving
//             toward home, units in millimeters/second
//          maxDistanceToMoveInMillimeters = unsigned maximum distance to move
//             toward home before giving up
//          homeSwitchPin = pin number of the home switch, switch should be
//             configured to go low when at home
//  Exit:   true returned if successful, else false
//
bool GeneratorStepper::moveToHomeInMillimeters(long directionTowardHome,
  float speedInMillimetersPerSecond, long maxDistanceToMoveInMillimeters,
  int homeLimitSwitchPin)
{
  return(moveToHomeInSteps(directionTowardHome,
                          speedInMillimetersPerSecond * stepsPerMillimeter,
                          maxDistanceToMoveInMillimeters * stepsPerMillimeter,
                          homeLimitSwitchPin));
}



//
// move relative to the current position, units are in millimeters, this function
// does not return until the move is complete
//  Enter:  distanceToMoveInMillimeters = signed distance to move relative to the
//          current position in millimeters
//
void GeneratorStepper::moveRelativeInMillimeters(float distanceToMoveInMillimeters)
{
  setupRelativeMoveInMillimeters(distanceToMoveInMillimeters);

  while(!processMovement())
    ;
}



//
// setup a move relative to the current position, units are in millimeters, no
// motion occurs until processMove() is called
// Note: this can only be called when the motor is stopped
//  Enter:  distanceToMoveInMillimeters = signed distance to move relative to the
//            currentposition in millimeters
//
void GeneratorStepper::setupRelativeMoveInMillimeters(float distanceToMoveInMillimeters)
{
  setupRelativeMoveInSteps((long) round(distanceToMoveInMillimeters *
			   stepsPerMillimeter));
}



//
// move to the given absolute position, units are in millimeters, this function
// does not return until the move is complete
//  Enter:  absolutePositionToMoveToInMillimeters = signed absolute position to
//            move toin units of millimeters
//
void GeneratorStepper::moveToPositionInMillimeters(
                      float absolutePositionToMoveToInMillimeters)
{
  setupMoveInMillimeters(absolutePositionToMoveToInMillimeters);

  while(!processMovement())
    ;
}



//
// setup a move, units are in millimeters, no motion occurs until processMove() is
// called.  Note: this can only be called when the motor is stopped
//  Enter:  absolutePositionToMoveToInMillimeters = signed absolute position to move
//          to in units of millimeters
//
void GeneratorStepper::setupMoveInMillimeters(
                      float absolutePositionToMoveToInMillimeters)
{
 setupMoveInSteps((long) round(absolutePositionToMoveToInMillimeters *
   stepsPerMillimeter));
}



//
// Get the current velocity of the motor in millimeters/second.  This functions is
// updated while it accelerates up and down in speed.  This is not the desired
// speed, but the speed the motor should be moving at the time the function is
// called.  This is a signed value and is negative when motor is moving backwards.
// Note: This speed will be incorrect if the desired velocity is set faster than
// this library can generate steps, or if the load on the motor is too great for
// the amount of torque that it can generate.
//  Exit:  velocity speed in millimeters per second returned, signed
//
float GeneratorStepper::getCurrentVelocityInMillimetersPerSecond()
{
  return(getCurrentVelocityInStepsPerSecond() / stepsPerMillimeter);
}



// ---------------------------------------------------------------------------------
//                     Public functions with units in revolutions
// ---------------------------------------------------------------------------------


//
// set the number of steps the motor has per revolution
//
void GeneratorStepper::setStepsPerRevolution(float motorStepPerRevolution)
{
  stepsPerRevolution = motorStepPerRevolution;
}



//
// get the current position of the motor in revolutions, this functions is updated
// while the motor moves
//  Exit:  a signed motor position in revolutions returned
//
float GeneratorStepper::getCurrentPositionInRevolutions()
{
  return((float)currentPosition_InSteps / stepsPerRevolution);
}



//
// set current position of the motor in revolutions, this does not move the motor
//
void GeneratorStepper::setCurrentPositionInRevolutions(
   float currentPositionInRevolutions)
{
  currentPosition_InSteps = (long) round(
    currentPositionInRevolutions * stepsPerRevolution);
}



//
// set the maximum speed, units in revolutions/second, this is the maximum speed
// reached while accelerating.  Note: this can only be called when the motor is
// stopped
//  Enter:  speedInRevolutionsPerSecond = speed to accelerate up to, units in
//            revolutions/second
//
void GeneratorStepper::setSpeedInRevolutionsPerSecond(float speedInRevolutionsPerSecond)
{
  desiredSpeed_InStepsPerSecond = speedInRevolutionsPerSecond * stepsPerRevolution;
}



//
// set the rate of acceleration, units in revolutions/second/second
// Note: this can only be called when the motor is stopped
//  Enter:  accelerationInRevolutionsPerSecondPerSecond = rate of acceleration,
//            units inrevolutions/second/second
//
void GeneratorStepper::setAccelerationInRevolutionsPerSecondPerSecond(
                      float accelerationInRevolutionsPerSecondPerSecond)
{
    acceleration_InStepsPerSecondPerSecond =
       accelerationInRevolutionsPerSecondPerSecond * stepsPerRevolution;
}



//
// home the motor by moving until the homing sensor is activated, then set the
// position to zero, with units in revolutions
//  Enter:  directionTowardHome = 1 to move in a positive direction, -1 to move in
//             a negative directions
//          speedInRevolutionsPerSecond = speed to accelerate up to while moving
//             toward home, units in revolutions/second
//          maxDistanceToMoveInRevolutions = unsigned maximum distance to move
//             toward home before giving up
//          homeSwitchPin = pin number of the home switch, switch should be
//             configured to go low when at home
//  Exit:   true returned if successful, else false
//
bool GeneratorStepper::moveToHomeInRevolutions(long directionTowardHome,
  float speedInRevolutionsPerSecond, long maxDistanceToMoveInRevolutions,
  int homeLimitSwitchPin)
{
  return(moveToHomeInSteps(directionTowardHome,
                          speedInRevolutionsPerSecond * stepsPerRevolution,
                          maxDistanceToMoveInRevolutions * stepsPerRevolution,
                          homeLimitSwitchPin));
}



//
// move relative to the current position, units are in revolutions, this function
// does not return until the move is complete
//  Enter:  distanceToMoveInRevolutions = signed distance to move relative to the
//          current position in revolutions
//
void GeneratorStepper::moveRelativeInRevolutions(float distanceToMoveInRevolutions)
{
  setupRelativeMoveInRevolutions(distanceToMoveInRevolutions);

  while(!processMovement())
    ;
}



//
// setup a move relative to the current position, units are in revolutions, no
// motion occurs until processMove() is called.  Note: this can only be called
// when the motor is stopped
//  Enter:  distanceToMoveInRevolutions = signed distance to move relative to the
//          current position in revolutions
//
void GeneratorStepper::setupRelativeMoveInRevolutions(float distanceToMoveInRevolutions)
{
  setupRelativeMoveInSteps((long) round(distanceToMoveInRevolutions *
                           stepsPerRevolution));
}



//
// move to the given absolute position, units are in revolutions, this function
// does not return until the move is complete
//  Enter:  absolutePositionToMoveToInRevolutions = signed absolute position to
//          move to in units of revolutions
//
void GeneratorStepper::moveToPositionInRevolutions(
       float absolutePositionToMoveToInRevolutions)
{
  setupMoveInRevolutions(absolutePositionToMoveToInRevolutions);

  while(!processMovement())
    ;
}



//
// setup a move, units are in revolutions, no motion occurs until processMove() is
// called.  Note: this can only be called when the motor is stopped
//  Enter:  absolutePositionToMoveToInRevolutions = signed absolute position to
//          move to inunits of revolutions
//
void GeneratorStepper::setupMoveInRevolutions(
          float absolutePositionToMoveToInRevolutions)
{
 setupMoveInSteps((long) round(absolutePositionToMoveToInRevolutions *
                   stepsPerRevolution));
}



//
// Get the current velocity of the motor in revolutions/second.  This functions is
// updated while it accelerates up and down in speed.  This is not the desired
// speed, but the speed the motor should be moving at the time the function is
// called.  This is a signed value and is negative when motor is moving backwards.
// Note: This speed will be incorrect if the desired velocity is set faster than
// this library can generate steps, or if the load on the motor is too great for
// the amount of torque that it can generate.
//  Exit:  velocity speed in revolutions per second returned, signed
//
float GeneratorStepper::getCurrentVelocityInRevolutionsPerSecond()
{
  return(getCurrentVelocityInStepsPerSecond() / stepsPerRevolution);
}



// ---------------------------------------------------------------------------------
//                        Public functions with units in steps
// ---------------------------------------------------------------------------------


//
// set the current position of the motor in steps, this does not move the motor
// Note: This function should only be called when the motor is stopped
//    Enter:  currentPositionInSteps = the new position of the motor in steps
//
void GeneratorStepper::setCurrentPositionInSteps(long currentPositionInSteps)
{
  //this does not write any data. Equivalent to get.
  driver.RAMPMODE(3);
  int32_t xInt = driver.XACTUAL();
  driver.XACTUAL(currentPositionInSteps);
  float currentPosition_InSteps = (*(float*)(&xInt));
  currentPosition_InSteps = currentPositionInSteps;
  driver.RAMPMODE(0);
}



//
// get the current position of the motor in steps, this functions is updated
// while the motor moves
//  Exit:  a signed motor position in steps returned
//
long GeneratorStepper::getCurrentPositionInSteps()
{
  int32_t xInt = driver.XACTUAL();
  currentPosition_InSteps = xInt;
  return(currentPosition_InSteps);
}



//
// setup a "Stop" to begin the process of decelerating from the current velocity to
// zero, decelerating requires calls to processMove() until the move is complete
// Note: This function can be used to stop a motion initiated in units of steps or
// revolutions
//
void GeneratorStepper::setupStop()
{
  //
  // move the target position so that the motor will begin deceleration now
  //
  //if (direction_Scaler > 0)
    //targetPosition_InSteps = currentPosition_InSteps + decelerationDistance_InSteps;
  //else
   // targetPosition_InSteps = currentPosition_InSteps - decelerationDistance_InSteps;
}



//
// set the maximum speed, units in steps/second, this is the maximum speed reached
// while accelerating
// Note: this can only be called when the motor is stopped
//  Enter:  speedInStepsPerSecond = speed to accelerate up to, units in steps/second
//
void GeneratorStepper::setSpeedInStepsPerSecond(float speedInStepsPerSecond)
{
  float calculatedV = speedInStepsPerSecond*t;
  uint32_t vMax = static_cast<unsigned int>(calculatedV + 0.5);
  driver.VMAX(vMax);
  driver.V1(round(vMax/10));
  driver.VSTART(t);
  driver.VSTOP(t+1);
  desiredSpeed_InStepsPerSecond = speedInStepsPerSecond;
}



//
// set the rate of acceleration, units in steps/second/second
// Note: this can only be called when the motor is stopped
//  Enter:  accelerationInStepsPerSecondPerSecond = rate of acceleration, units in
//          steps/second/second
//
void GeneratorStepper::setAccelerationInStepsPerSecondPerSecond(
                      float accelerationInStepsPerSecondPerSecond)
{
  float calculatedA = accelerationInStepsPerSecondPerSecond*ta2;
  uint32_t aMax = static_cast<unsigned int>(calculatedA + 0.5);
  driver.AMAX(aMax);
  driver.A1(round(aMax/10));
  driver.D1(round(aMax/5));
  driver.DMAX(aMax);
  acceleration_InStepsPerSecondPerSecond = accelerationInStepsPerSecondPerSecond;
}



//
// home the motor by moving until the homing sensor is activated, then set the
// position to zero with units in steps
//  Enter:  directionTowardHome = 1 to move in a positive direction, -1 to move in
//             a negative directions
//          speedInStepsPerSecond = speed to accelerate up to while moving toward
//             home, units in steps/second
//          maxDistanceToMoveInSteps = unsigned maximum distance to move toward
//             home before giving up
//          homeSwitchPin = pin number of the home switch, switch should be
//             configured to go low when at home
//  Exit:   true returned if successful, else false
/*
IMPLEMENTING A HOMING PROCEDURE
1. Make sure, that the home switch is not pressed, e.g. by moving away from the switch.
2. Activate position latching upon the desired switch event and activate motor (soft) stop upon
active switch. StallGuard based homing requires using a hard stop (en_softstop=0).
3. Start a motion ramp into the direction of the switch. (Move to a more negative position for a left
switch, to a more positive position for a right switch). You may timeout this motion by using a
position ramping command.
4. As soon as the switch is hit, the position becomes latched and the motor is stopped. Wait until the motor is in standstill again by polling the actual velocity VACTUAL or checking vzero or the standstill flag.
5. Switch the ramp generator to hold mode and calculate the difference between the latched position and the actual position. For StallGuard based homing or when using hard stop, XACTUAL stops exactly at the home position, so there is no difference (0).
6. Write the calculated difference into the actual position register. Now, homing is finished. A move to position 0 will bring back the motor exactly to the switching point. In case StallGuard was used for homing, read and write back RAMP_STAT to clear the StallGuard stop event event_stop_sg and release the motor from the stop condition.
HOMING

*/
bool GeneratorStepper::moveToHomeInSteps(long directionTowardHome,
  float speedInStepsPerSecond, long maxDistanceToMoveInSteps, int homeLimitSwitchPin)
{
  driver.en_softstop(0);  // hard stop mode
  if(Z_STOP_PIN == 26){   //Case for bottom endstop
  driver.pol_stop_l(0);
  driver.latch_l_active(1);
  driver.stop_l_enable(1);
  driver.RAMPMODE(2); //velocity mode to negative position
  while(!driver.vzero()){
    delay(10);
  }
  driver.RAMPMODE(3);
  moveToPositionInMillimeters(10);
  driver.pol_stop_l(0);
  driver.latch_l_active(1);
  driver.stop_l_enable(1);
  driver.RAMPMODE(2);
  }
  if(Z_STOP_PIN == 16){   //Case for top endstop
  driver.pol_stop_r(0);
  driver.latch_r_active(1);
  driver.stop_r_enable(1);
  driver.RAMPMODE(1); //velocity mode to positive position
  while(!driver.vzero()){
    delay(10);
  }
  driver.RAMPMODE(3);
  moveToPositionInMillimeters(10);
  driver.pol_stop_r(0);
  driver.latch_r_active(1);
  driver.stop_r_enable(1);
  driver.RAMPMODE(1);
  }
  driver.en_softstop(1); //enable gentle deceleration
  return true;
}



//
// move relative to the current position, units are in steps, this function does
// not return until the move is complete
//  Enter:  distanceToMoveInSteps = signed distance to move relative to the current
//            position in steps
//
void GeneratorStepper::moveRelativeInSteps(long distanceToMoveInSteps)
{
  setupRelativeMoveInSteps(distanceToMoveInSteps);

  while(!processMovement())
    ;
}



//
// setup a move relative to the current position, units are in steps, no motion
// occurs until processMove() is called.  Note: this can only be called when the
// motor is stopped
//  Enter:  distanceToMoveInSteps = signed distance to move relative to the current
//          position in steps
//
void GeneratorStepper::setupRelativeMoveInSteps(long distanceToMoveInSteps)
{
  setupMoveInSteps(currentPosition_InSteps + distanceToMoveInSteps);
}



//
// move to the given absolute position, units are in steps, this function does not
// return until the move is complete
//  Enter:  absolutePositionToMoveToInSteps = signed absolute position to move to
//            in units of steps
//
void GeneratorStepper::moveToPositionInSteps(long absolutePositionToMoveToInSteps)
{
  setupMoveInSteps(absolutePositionToMoveToInSteps);

  while(!processMovement())
    ;
}



//
// setup a move, units are in steps, no motion occurs until processMove() is called
// Note: this can only be called when the motor is stopped
//  Enter:  absolutePositionToMoveToInSteps = signed absolute position to move to in
//          units of steps
//
void GeneratorStepper::setupMoveInSteps(long absolutePositionToMoveToInSteps)
{
  driver.RAMPMODE(3); //set to holding mode
  driver.XTARGET(absolutePositionToMoveToInSteps);
}



//
// if it is time, move one step
//  Exit:  true returned if movement complete, false returned not a final target
//           position yet
//
bool GeneratorStepper::processMovement(void)
{
  //driver.X_COMPARE();
  driver.RAMPMODE(0); //switch to position mode, which executes the target destination
  while(!driver.position_reached()){
    delay(100);
  }
  return(true);
  driver.RAMPMODE(3); //return to hold mode
}



//
// Get the current velocity of the motor in steps/second.  This functions is updated
// while it accelerates up and down in speed.  This is not the desired speed, but
// the speed the motor should be moving at the time the function is called.  This
// is a signed value and is negative when the motor is moving backwards.
// Note: This speed will be incorrect if the desired velocity is set faster than
// this library can generate steps, or if the load on the motor is too great for
// the amount of torque that it can generate.
//  Exit:  velocity speed in steps per second returned, signed
//
float GeneratorStepper::getCurrentVelocityInStepsPerSecond()
{
  int32_t polledV = driver.VACTUAL();
  float CurrentVInSteps = polledV;
  return(CurrentVInSteps);
}



//
// check if the motor has competed its move to the target position
//  Exit:  true returned if the stepper is at the target position
//
bool GeneratorStepper::motionComplete()
{
  if(driver.position_reached()){
    return true;
  }else{
    return false;
  }
}

// -------------------------------------- End --------------------------------------