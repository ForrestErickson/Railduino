/*Railduino
 Forrest Erickson
 Created Nov 16, 2013.
 Arduino controlling a stepper motor driving a linear rail camera mount.
 Camera control during rail movement for Auto Focus and Camera exposure.
 
 Motor control with SEEED Studio motor shield.
 Motor example AIRPAX A82743-M4 with 7.5 step angle.
 Motor during bench Development BigInch with 1.5 step angle.
 
 Schematic:
 Drawings:
 */

/*
Pin assignments
 A0               Auto / Serial Control  High if automatic advance.
 A1 - A5          NC
 
 D0  RX
 D1  TX
 D2  nNEAR_LIMIT  Near Limit Switch
 D3~ Near_LED     RED LED on near end.
 D4  nFAR_LIMIT   Far Limit Switch
 D5~ Far_LED      RED LED on far end.
 D6~              /Focus    ?ring?
 D7               /Shutter  ?tip?
 D8               MOTOR CONTROL
 D9~              Output Enable Bridge A, M1
 D10~             Output Enable Bridge B, M2
 D11~             MOTOR CONTROL
 D12              MOTOR CONTROL
 D13              MOTOR CONTROL
 */


#include <Stepper.h>
#include <stdlib.h>  //Allows atoi ASCII to Integer

//Constants
const char VERSION[] ="20140302";  //VERSION is printed at start up.
const  int nNEAR_LIMIT = 2;  // Switch goes low when we reach near limit.
const  int NEAR_LED = 3;  // LED on the motor end.
const  int nFAR_LIMIT = 4;  // Switch goes low when we reach far limit.
const  int FAR_LED = 5;  // LED on the far end.
const  int nFOCUS = 6;  // Pin assignment. Make low to trigger auto focus.
const  int nSHUTTER = 7;  // Pin assignment. Make low to trigger open shutter.

const  int VERBOSE = 1; //User VERBOSE for development
const  int  HEARTBEAT = 1000;  //Period of heart beat in milliseconds used for LED.
//const  int  FOCUS_DELAY = 250;  //mSec delay from focus to shutter release. too short
//750 ms missed some photos.
const  int  FOCUS_DELAY = 1000;  //mSec delay from focus to shutter release.

//Motor Setup
const int stepsPerRevolution = int(360/7.5);  // For 7.5 AIRPAX degree / step motor.
//const int stepsPerRevolution = int(360/1.5);  // Big Inch degree / step motor.
//int speed_motor = 48; //steps per second
int speed_motor = 32; //steps per second


//Rail Setup
const int  THREADS_PER_INCH = 13;  //on 1/2x13 all thread.
const int  LENGTH_OF_TRAVEL = 52; //Inches as measured Jan 3, 2014.
//const int  MAX_REVOLUSTIONS = LENGTH_OF_TRAVEL * THREADS_PER_INCH;
//const int  MAX_STEPS  = MAX_REVOLUSTIONS * stepsPerRevolution;
const long  MAX_REVOLUSTIONS = LENGTH_OF_TRAVEL * THREADS_PER_INCH;
const long  MAX_STEPS  = MAX_REVOLUSTIONS * stepsPerRevolution;
int  length_percent = 1;  //Percent of the total rail length to travel.

//Auto Advance Switch setup
const int AUTOSWITCHMID = 512;  //Switch is 10 bit A2D. 
int AutoSwitch = LOW;
int lastAutoSwitch = LOW;
int valAutoSwitch = LOW;


//Camera Default Setup
long camera_delay_interval =2;  //Seconds of stepper advancing. Seconds between closing of shutter and next camera shot.
long  camera_exposure = 30;  //Seconds of exposure
int  number_photos = 1000;  //Default number of photos.
unsigned long  exposure_finish_time = 0;  // Used in main loop to stop exposures in mills. Can count for 49 days.
unsigned long  next_exposure_starts = 0;  // Used in main loop to start next exposures in mills.
boolean  exposing  = 0;  //Boolean state variable, Nonzero for exposing (shutter is open).
boolean  traversing  = 0;  //Boolean state variable, Nonzero for normal advance. Set to 1 for going Home.
boolean  going = 0;  //Boolean state variable, Nonzero for going to make photos.

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8,11,12,13);           
int advance = 1;  //Direction of stepper is counter clockwise to push trolly.

unsigned long lastchange = 0;  //Time since LED last changed.

//Initialize Hardware
void  setup()  {
  pinMode(NEAR_LED,OUTPUT);
  pinMode(FAR_LED,OUTPUT);
  digitalWrite(NEAR_LED,HIGH);
  digitalWrite(FAR_LED,HIGH);
  delay(50);
  toggleLED();

  //Setup Limit switchers as input.
  pinMode (nNEAR_LIMIT, INPUT);  //10K pull up. Switch closes at limit.
  pinMode (nFAR_LIMIT, INPUT);  //10K pull up. Switch closes at limit.

  //The focus and shutter will be an open drain to mimic a switch to ground.
  pinMode(nFOCUS, INPUT);  //Set as input for high impedance.
  pinMode(nSHUTTER, INPUT); // Set as input for high impedance.
  digitalWrite(nFOCUS, LOW);  // Set for pin low to sink current when low impedance.
  digitalWrite(nSHUTTER, LOW);
  delay(50);
  toggleLED();

  // set the speed in RPM and turn on pins to driver stepper shield.
  myStepper.setSpeed(speed_motor);
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);
  digitalWrite(9,HIGH);  //Enable output bridge A for M1
  digitalWrite(10,HIGH);  //Enable output bridge B for M2
  delay(50);
  toggleLED();

//  Serial.begin(38400);
  Serial.begin(9600);
  if (VERBOSE)  {
    Serial.print("\r\n\fRailduino Version ");  
  }
  if (VERBOSE)  {
    Serial.println(VERSION);  
  }
  if (VERBOSE)  {
    Serial.print("Setup Done.\n\rMax Steps = ");  
  }
  //  Serial.println (MAX_STEPS);
  delay(50);
  toggleLED();

  report_setup();
  toggleLED();
}

/* ------------------------  MAIN Loop -------------------------------------*/
//Use  non blocking or minimaly blocking methods in the loop().
void  loop()  {

  //Heart Beat
  if (HEARTBEAT/2 < (millis()-lastchange)){
    toggleLED();
    lastchange = millis();
  }

  test_limit_switches();  //Check both switch status to see if limit is reached.

  //Read the auto advance switch
  valAutoSwitch= analogRead(AutoSwitch);
  if (valAutoSwitch > AUTOSWITCHMID) {
    number_photos = 2;  //Set for 2 bacause the next step decrements and we need at least one.
    if (lastAutoSwitch == LOW) {
      delay(10);
      lastAutoSwitch = HIGH;
      traversing = 0;  //Clear traversing of swtich set to auto.
      go();
      //      going = 1;  // Rail is going.
      //      exposing = 1;
    }
  } 
  else {
    if (lastAutoSwitch == HIGH) {
      nogo();
      number_photos =  0;                
    }
    lastAutoSwitch = LOW;
  }

//Traversing to home or far end.
  if (traversing == 1)  {
      stepperon();  //Turn motors back on.
      myStepper.step(advance);  //One step advance
  }



  /*Manage N photos.
   Asserts auto focus for a time, TBD before the shutter is released.
   Shutter released and held for the exposure time.
   Count down the number of photos until all are taken.
   */

  // If exposing and we reach the time to finish close the shutter.  Check if this is the last one.
  if (exposing)  {
    if (exposure_finish_time < millis())  {
      // close shutter, ?set state variable?
      pinMode(nSHUTTER,INPUT); // Make high impedance to stop photo.
      exposing = 0;  //Stop exposing.
      stepperon();  //Turn motors back on.
      number_photos = number_photos -1;  //Decrement number of photo remaining.
      if (number_photos < 1)  {
        nogo();   //Stop advancing.
        going = 0;  //Set state variable too.
        if (VERBOSE)  Serial.print("Done with this run at: ");
        if (VERBOSE)  Serial.println(exposure_finish_time);
      }
      if (VERBOSE)  Serial.print("Shutter closed at: ");
      if (VERBOSE)  Serial.println(exposure_finish_time);
      //Next exposure starts at camera_delay_interval + exposure_finish_time
      next_exposure_starts = (camera_delay_interval*1000) + exposure_finish_time - FOCUS_DELAY;
      Serial.print("Number of photos remaining: ");
      Serial.println(number_photos);
      if (VERBOSE && (number_photos > 0))  Serial.print("Next exposure at: ");
      if (VERBOSE && (number_photos > 0))  Serial.println(next_exposure_starts);    
      //We can start advancing the motor now. Set for TBD steps.
    }
  }  //checking to close shutter.

  //If not exposing, wait until the next exposure is to start, Then do it!
  if ((exposing != 1) && (going == 1)){
    if ((number_photos >0) && (next_exposure_starts < millis()))  {
      stepperoff();  // Turn off the motor outputs to save battery
      // Trigger auto focus before photo
      //        pinMode(nFOCUS, OUTPUT);  // Make low impedance
      //        Serial.print("Focusing! ");
      //        delay(FOCUS_DELAY);
      //        pinMode(nFOCUS, INPUT);    //Make high impedance.

      // Set new exposure finish time and Open shutter shutter,
      exposure_finish_time = (millis() + 1000*camera_exposure);
      if (VERBOSE)  Serial.print ("Exposing now at:");
      if (VERBOSE)  Serial.print (millis());
      if (VERBOSE)  Serial.print (". Finish will be time: ");
      if (VERBOSE)  Serial.println (exposure_finish_time);
      pinMode(nSHUTTER,OUTPUT); // Open shutter
      exposing = 1;  //Set the state variable.
    }
    else  {
      //Here is where we can step the rail.
//      if (VERBOSE)  Serial.print ("|");
      //        delay(5);
      myStepper.step(advance*1);  //One step advance
    }

  }  // Checking to open the shutter.

  //Get User input. User interface on serial port.
  int  inByte;
  //  if (0) {
  if (Serial.available() >0) {
    inByte = Serial.read();  //Add next char to string.

    switch (inByte)  {

    case 'f':
    case 'F':
      advance = 1;
      Serial.println("Set for Forward.") ;   
      break;

    case 'r':
    case 'R':
      advance = -1;
      Serial.println("Set for Reverse.") ;   
      break;

    case 'w':
    case 'W':
      Serial.println("Motor is Waving!") ;   
      wave();  //The old go() function. Useful for testing.
      Serial.println("Waving has now stopped.") ;   
      break;

    case 'b':
    case 'B':
      Serial.println("Bump motor a step") ;  
      myStepper.step(advance*1);  //One step advance
      break;

      //Home the trolly by returning until limit switch closes.
    case 'h':
    case 'H':
      Serial.println("Heading for Home.") ;  
      //Stop exposure. Close shutter, ?set state variable?
      pinMode(nSHUTTER,INPUT); // Make high impedance to stop photo.
      exposing = 0;  //Stop exposing.
      
      //Set direction to home, return to stepper moter end of rail.
      traversing = 1;  //State is going fast to home.
      advance = -1;
      break;

    case 'g':
    case 'G':
      go();
      break;

    case 'S':
      Serial.println("Enter motor Speed.") ;   
      speed_motor = serial_get_int();
      if (speed_motor <1) speed_motor = 1;
      myStepper.setSpeed(speed_motor);
      report_setup();
      break;

    case 's':
      stepperoff();  // Turn off the motor outputs to save battery
      going = 0;  // Stop Rail.
      exposing = 0;  //Stop Camera.
      traversing = 0;  //Stop going home or far.
      Serial.println("Stopped / Paused.");
      break;

    case 't':
    case 'T':
      Serial.println("This does nothing yet. Should be: Reports Time for rail travel.") ;   
      break;

    case 'L':
      length_percent = length_percent +10;
      if (length_percent >100) {
        length_percent = 100;
      }
      Serial.println("Set travel for ");
      Serial.print (length_percent);
      Serial.println(" % Length of rail.") ;
      break;

    case 'l':
      length_percent = length_percent - 10;
      if (length_percent <10) {
        length_percent = 10;
      }        
      Serial.println("Set travel for ");
      Serial.print (length_percent);
      Serial.println(" % Length of rail.") ;
      break;

    case 'e':
    case 'E':
        /* NON Blocking exposure function
       Kicks off the auto focus.
       Captures the exposure start time
       Sets the exposure finish time
       Opens the shutter. The main loop checks for end of exposure and closes the shutter.
       */
      Serial.println("Set Exposure time in seconds.") ;
      camera_exposure = serial_get_int();
      report_setup();
      break;

    case 'a':
    case 'A':
      Serial.println("Trigger camera Auto focus.") ;
      pinMode(nFOCUS, OUTPUT);
      delay(FOCUS_DELAY);
      pinMode(nFOCUS, INPUT);
      break;

    case 'p':
    case 'P':
      Serial.println("Make Photo now.") ;   
      // Trigger auto focus
      pinMode(nFOCUS, OUTPUT);
      delay(FOCUS_DELAY);
      pinMode(nFOCUS, INPUT);    //Make high impedance.

      /*        // Release shutter
       digitalWrite(nSHUTTER, LOW);
       delay(10);
       pinMode(nSHUTTER, INPUT);  //Make high impedance.
       */
      exposure_finish_time = (millis() + 1000*camera_exposure);
      if (VERBOSE)  Serial.print ("Exposing now at:");
      if (VERBOSE)  Serial.print (millis());
      if (VERBOSE)  Serial.print (". Finish will be time: ");
      if (VERBOSE)  Serial.println (exposure_finish_time);
      pinMode(nSHUTTER,OUTPUT); // Open shutter
      exposing = 1;  //Set the state variable.
      break;

    case 'i':
    case 'I':
      Serial.println("Set up for photo interval.") ;   
      camera_delay_interval = serial_get_int ();
      report_setup();
      break;

    case 'n':
    case 'N':
      Serial.println("Set for number of photos to make < 3000.") ;
      //        number_photos = get_number();
      number_photos = serial_get_int();
      report_setup();
      break;

    case 'm':  //M or m or NL prints menu.
    case 'M':
    case '\n':
      commandmenu();  
      report_setup();
      break;

    case '\r':  //Swallow CR.
      break;

    default:
      Serial.println("Sorry, you did not pressed a command I know.");    
      Serial.print("You pressed a(n)-- ");    
      Serial.print(char(inByte));    
      Serial.println(" --.");    

    }
  }  //Get User input.
}  //End of MAIN loop()

//Functions go here.

/* go() is the function which advances the motor and makes photos.
 The stop() function stops, actually pauses, the go() state.
 The home() command will take the trolly back to the starting point limited by a limit switch.
 
 At start:
 Capture start time.
 While length traveled is < length to travel
 Kick off focus. Blocks a short time.
 Then open the shutter for the exposure time. The open shutter function no longer blocks.
 */
void go()  {
  //  report_setup();
  if (number_photos >0) {
    stepperon();  //Turn motors back on.
    going = 1;  // Rail is going.
    Serial.println("Going!");
  }
  else {
    Serial.println("Going ignored! N < 1.");
  }
}  //end of go()

void nogo()  {
  going = 0;  // Rail is stopped.
  exposing = 0;  //Camera shutter closed.
  stepperoff();  //Turn motors back off.
  Serial.println("Stopping!");
}

//Stepper Motor On and Off Functions
//The delay is to allow the current to settle. This has not been measured.
void stepperon()
{
    digitalWrite(9,HIGH);  //Enable output bridge A
  digitalWrite(10,HIGH);  //Enable output bridge B
  delay(50);

}
void stepperoff()
{
    digitalWrite(9,LOW);  //Enable output bridge A
  digitalWrite(10,LOW);  //Enable output bridge B
  delay(50);

}

//--------------------------End Motor control--------------------------------------

//Get digit characters and combine as positive integer.
int serial_get_int ()  
{
  char c = '0' ;
  int  x = 0;
  do
  {
    if (Serial.available()>0)  {
      c= Serial.read();
      Serial.print (c);  //Echo to user's terminal.
      if ((c >= '0') && (c <= '9')) {
        x= 10*x + (c-'0');
      }
    }
  } 
  while ( (c != '\n') && (x <= 3000) ); // int in x.
  return (x);  // The number.
}

