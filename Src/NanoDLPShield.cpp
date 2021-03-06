#include "HostPty.h"
#include "SpeedyStepper.h"
#include "Config.h"

#include <wiringPi.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>

using namespace std;

#if defined(HAS_TMC_SPI)

#endif
SpeedyStepper stepper;  //Define SpeedyStepper motor as stepper

bool relativePositioning = true;  //Use relative positioning
unsigned long lastMovementMS = 0;

HostPty pty("/tmp/ttyNanoDLP");

void ptyWrite(const string & str)  //Write string to virtual console
{
    pty.write(str);
    cout << str << endl;
}

#if defined(HAS_2130)
//TMC2130Stepper driver = TMC2130Stepper(CS_PIN, R_SENSE, SW_MOSI, SW_MISO, SW_SCK); // Software SPI
#elif defined(HAS_2660)
//TMC2660Stepper driver = TMC2660Stepper(CS_PIN, R_SENSE, SW_MOSI, SW_MISO, SW_SCK);
#elif defined(HAS_5160)
//TMC5160Stepper driver = TMC5160Stepper(CS_PIN, R_SENSE, SW_MOSI, SW_MISO, SW_SCK);
#endif

#if SUPPORT_UP_DOWN_BUTTONS
void setSteperLowSpeed()
{
    stepper.setSpeedInMillimetersPerSecond(LOW_SPEED);
    stepper.setAccelerationInMillimetersPerSecondPerSecond(LOW_ACCELERATION);
}

void setSteperHighSpeed()
{
    stepper.setSpeedInMillimetersPerSecond(HIGH_SPEED);
    stepper.setAccelerationInMillimetersPerSecondPerSecond(HIGH_ACCELERATION);
}

void processBtnMovement(int btnPin, int direction = 1)
{
    // Try small movements first
    setSteperHighSpeed();
    for(int i=0; i<5; i++)
    {
        stepper.moveRelativeInMillimeters(MANUAL_MOVEMENT_MM * direction);
        delay(300);
        if(!isButtonPressed(btnPin))
            return;
    }

    // Then move at low speed for 3 sec
    setSteperLowSpeed();
    unsigned long startTime = millis();
    while(millis() < startTime + 3000)
    {
        if(stepper.motionComplete())
            stepper.setupRelativeMoveInMillimeters(1000 * direction);
        else
            stepper.processMovement();

        // Check if button released
        if(!isButtonPressed(btnPin))
        {
            stepper.setupStop();
            while(!stepper.processMovement())
                ;
            return;
        }
    }

    // Then move at high speed
    setSteperHighSpeed();
    stepper.setupRelativeMoveInMillimeters(1000 * direction);
    while(isButtonPressed(btnPin))
    {
        if(stepper.motionComplete())
            stepper.setupRelativeMoveInMillimeters(1000 * direction);
        else
            stepper.processMovement();
    }

    // Stop when button released
    stepper.setupStop();
    while(!stepper.processMovement())
        ;
}
#endif //SUPPORT_UP_DOWN_BUTTONS

bool isButtonPressed(int btnPin) //Check for button press (GPIO pulled low)
{
    return digitalRead(btnPin) == LOW;
}

void updateLastMovement() //Update time since last move command
{
    lastMovementMS = millis();
}

bool shouldDisableMotors() //If over 100S since last movement, motor is disabled
{
    return millis() - lastMovementMS > 100000; // 100 seconds
}

void processMotorOnCmd() //M17 enable motor driver
{
    updateLastMovement();
    digitalWrite(ENABLE_PIN, LOW);
}

void processMotorOffCmd() //M18 disable motor driver
{
    digitalWrite(ENABLE_PIN, HIGH);
}

void processLEDOnCmd() // M3 or M106 turn on UV LED
{
    digitalWrite(UV_LED_PIN, HIGH);
}

void processLEDOffCmd() // M5 or M107 turn off UV LED
{
    digitalWrite(UV_LED_PIN, LOW);
}

#if SUPPORT_LED_ON_BUTTON
void processLEDButon()
{
    digitalWrite(UV_LED_PIN, !digitalRead(UV_LED_PIN));
    delay(50);
    while(isButtonPressed(LED_ON_BTN_PIN))
        ;
}
#endif //SUPPORT_LED_ON_BUTTON

void setup()
{
    // General GPIO initialization
    if (wiringPiSetupGpio () == -1)
        throw std::runtime_error("Cannot initialize GPIO");

    // Init stepper motor
    stepper.connectToPins(STEP_PIN, DIR_PIN);
    stepper.setStepsPerMillimeter(STEPS_PER_MM);
    stepper.setSpeedInMillimetersPerSecond(DEFAULT_SPEED);
    stepper.setAccelerationInMillimetersPerSecondPerSecond(DEFAULT_ACCELERATION);
    pinMode(ENABLE_PIN, OUTPUT);
    processMotorOffCmd();

#if SUPPORT_UP_DOWN_BUTTONS
    // Init up/down buttons
    pinMode(UP_BTN_PIN, INPUT);
    pullUpDnControl(UP_BTN_PIN, UP_BTN_PUD);
    pinMode(DOWN_BTN_PIN, INPUT);
    pullUpDnControl(DOWN_BTN_PIN, DOWN_BTN_PUD);
#endif //SUPPORT_UP_DOWN_BUTTONS

    // Init UV LED MOSFET Pin as off
    pinMode(UV_LED_PIN, OUTPUT);
    digitalWrite(UV_LED_PIN, LOW);

#if SUPPORT_LED_ON_BUTTON
    // Init Led On/Off button
    pinMode(LED_ON_BTN_PIN, INPUT);
    pullUpDnControl(LED_ON_BTN_PIN, LED_ON_BTN_PUD);
#endif //SUPPORT_LED_ON_BUTTON

    // Signaling (general purpose) LED
    pinMode(LED_PIN, OUTPUT);

    // FAN PWM Range is 0-1024 and called via pwmWrite(int pin, int value)
    // Initialize as off
    pinMode(FAN_PIN, OUTPUT);
    pwmWrite(FAN_PIN, 0);

    //ENDSTOPS
    pinMode(Z_STOP_PIN, INPUT);
    pullUpDnControl(Z_STOP_PIN, Z_STOP_PUD);
}

bool checkMCommand(const char * buf, char prefix)
{
  const char * ptr = buf;
  ptr = strchr(ptr, prefix);
  if(ptr == NULL)
  {
    return false;
  } else {
    return true;
  }
}

int parseInt(const char * buf, char prefix, int value)
{
    const char * ptr = buf;

    while(ptr && *ptr)
    {
        if(*ptr == prefix)
            return atoi(ptr + 1);

        ptr = strchr(ptr, ' ');
        if(ptr == NULL)
            break;

        ptr++;
    }
    return value;
}

float parseFloat(const char * buf, char prefix, float value)
{
    const char * ptr = buf;

    while(ptr && *ptr)
    {
        if(*ptr == prefix)
            return atof(ptr + 1);

        ptr = strchr(ptr, ' ');
        if(ptr == NULL)
            break;

        ptr++;
    }
    return value;
}

void processMoveCmd(float position, float speed)
{
    if(speed != 0)
        stepper.setSpeedInMillimetersPerSecond(speed / 60);

    if(relativePositioning)
        stepper.moveRelativeInMillimeters(position);
    else
        stepper.moveToPositionInMillimeters(position);
}

void processPauseCmd(int duration)
{
    delay(duration);
}

bool parseGCommand(const char * cmd)
{
    int cmdID = parseInt(cmd, 'G', 0);
    switch(cmdID)
    {
        case 1: // G1 Move
        {
            float len = parseFloat(cmd, 'Z', 0);
            float speed = parseFloat(cmd, 'F', 0);
            processMotorOnCmd();
            processMoveCmd(len, speed);
            updateLastMovement();

            // NanoDLP waits for a confirmation that movement was completed
            ptyWrite("Z_move_comp");
            return true;
        }
        case 4: // G4 Pause
        {
            int duration = parseInt(cmd, 'P', 0);
            processPauseCmd(duration);
            return true;
        }
        case 28: // G28 Home
        {
          // Set direction, speed, travel, and endstop in Config.h
          stepper.moveToHomeInMillimeters(HOME_DIR, HOME_SPD, HOME_HEIGHT, Z_STOP_PIN);
          ptyWrite("Z_move_comp");
          updateLastMovement();
        }
        case 90: // G90 - Set Absolute Positioning
            relativePositioning = false;
            return true;

        case 91: // G91 - Set Relative Positioning
            relativePositioning = true;
            return true;

    }

    return false;
}

bool parseMCommand(const char * cmd)
{
    int cmdID = parseInt(cmd, 'M', 0);
    switch(cmdID)
    {

        case 3:// M3/M106 - UV LED On
        {
            processLEDOnCmd();
            return true;
        }

        case 106:
        {
            if(checkMCommand(cmd, 'P'))
            {
                float spd = parseFloat(cmd, 'S', 0);
                pwmWrite(FAN_PIN, spd);
            } else {
                processLEDOnCmd();
                return true;
            }
        }

        case 5: // M5/M107 - UV LED Off
        {
            processLEDOffCmd();
            return true;
        }

        case 107:
        {
            if(checkMCommand(cmd, 'P'))
            {
                pwmWrite(FAN_PIN, 0);
            } else {
            processLEDOffCmd();
            return true;
            }
        }

        case 17: // M17 - Motor on
        {
            processMotorOnCmd();
            return true;
        }

        case 18: // M18 - Motor off
        {
            processMotorOffCmd();
            return true;
        }

        case 114: // M114 - Get current position
        {
            float pos = stepper.getCurrentPositionInMillimeters();
            stringstream s;
            s << "Z:" << std::setprecision(2) << pos;
            ptyWrite(s.str());
            return true;
        }

        case 300:
        {   
            #if SUPPORT_BUZZER
            if(checkMCommand(cmd, 'S'))
            {
                long i = millis();
                float len = parseFloat(cmd, 'S', 0);
                long j = i+len;
                while(islessequal(i,j))
                {
                    digitalWrite(BUZZ_PIN,1);
                }
                digitalWrite(BUZZ_PIN,0);
            }
            #endif
            return true;
        }        
    }
    return false;
}

bool parseCommand(const char * cmd)
{
    switch(*cmd)
    {
    case 'G':
        return parseGCommand(cmd);

    case 'M':
        return parseMCommand(cmd);

    default:
        break;
    }

    return false;
}

int main(int argc, char** argv)
{
    setup();

    while(1)
    {
        //checkAlive();

        #if SUPPORT_UP_DOWN_BUTTONS
        if(isButtonPressed(UP_BTN_PIN))
        {
            processMotorOnCmd();
            processBtnMovement(UP_BTN_PIN, 1);
            updateLastMovement();
        }

        if(isButtonPressed(DOWN_BTN_PIN))
        {
            processMotorOnCmd();
            processBtnMovement(DOWN_BTN_PIN, -1);
            updateLastMovement();
        }
        #endif //SUPPORT_UP_DOWN_BUTTONS

        #if SUPPORT_LED_ON_BUTTON
        if(isButtonPressed(LED_ON_BTN_PIN))
            processLEDButon();
        #endif //SUPPORT_LED_ON_BUTTON
        
        string cmd;

        if(pty.receiveNextString(cmd))
        {
            cout << "Received line: " << cmd << endl;

            if(parseCommand(cmd.c_str()))
            {
                ptyWrite("ok");
            }
            else
            {
                string s("Invalid or unsupported command: ");
                s += cmd;
                ptyWrite(s);
            }
        }

        if(shouldDisableMotors() && !digitalRead(ENABLE_PIN))
            {
                processMotorOffCmd();
            } 
    }
    
}
