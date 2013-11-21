/*Railduino
  Forrest Erickson
  Created Nov 16, 2013.
  Arduino controling a steper moter driging a linear rail camera mount.
  Camera control during rail movement

  Motor control with SEEED Studio motor sheild.
  Motor example AIRPAX A82743-M4 with 7.5 step angle.
  
  Schematic: 
  Drawings: 
*/

/*
Pin assignments
  A0 - A5          NC
  
  D0  RX
  D1  TX
  D2  Int.0        NC
  D3~ Int.1        NC
  D4               NC
  D5~              NC
  D6~              /Focus    ?ring?
  D7               /Shutter  ?tip?
  D8               MOTOR CONTROL
  D9~              NC
  D10~             NC
  D11~             MOTOR CONTROL
  D12              MOTOR CONTROL
  D13              MOTOR CONTROL
*/


#include <Stepper.h>
#include <String.h>  //Allows sting comparison.
#include <stdlib.h>  //Allows atoi ASCII to Interger

//Constants
const  int LED = 13;  // Pin assignement. The Arduino LED.  Also LED IN4 on the motor shield.
const  int nFOCUS = 6;  // Pin assignment. Make low to trigger auto focus.
const  int nSHUTTER = 7;  // Pin assignment. Make low to trigger open shutter.

const  int VERBOSE = 1; //User VEBRBOSE for development
const  int  HEARTBEAT = 1000;  //Period of heart beat in milliseconds used for LED.
const  int  FOCUS_DELAY = 10;  //mSec delay from focust to shutter release.

//Motor Setup
//const int stepsPerRevolution = int(360/7.5);  // For 7.5 AIRPAX degree / step motor.
//const int stepsPerRevolution = int(360);  // Big Inch degree / step motor.
const int stepsPerRevolution = int(360/1.5);  // Big Inch degree / step motor.

//Rail Setup
const int  THREADS_PER_INCH = 20;  //on 1/4x20 all thread.
const int  LENGTH_OF_TRAVEL = 1; //Inches 
const int  MAX_REVOLUSTIONS = LENGTH_OF_TRAVEL * THREADS_PER_INCH;
const int  MAX_STEPS  = MAX_REVOLUSTIONS * stepsPerRevolution;
int  length_percent = 10;  //Percent of the total rail length to travel.

//Camera Setup
int camera_delay_interval = 30;  //Seconds between closing of shutter and next camera shot.
int  camera_exposure = 30;  //Seconds of exposure
int  number_photos = 1;  //Default number of photos.
//long  number_photos = 1;  //Default number of photos.

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8,11,12,13);           
int advance = -1;  //Direction of stepper is counter clockwise to push trolly.

// Set serial port variables.  
  String inputString = "";         // a string to hold incoming data
  boolean stringComplete = false;  // whether the string is complete

  //unsigned long lastchange = millis();
  unsigned long lastchange = 0;  //Time since LED last changed.

//Initiliaze Hardware
void  setup()  {
  pinMode(LED, OUTPUT);
  digitalWrite(LED,HIGH);
  delay(50);
  toggleLED();
  pinMode(nFOCUS, INPUT);  //Set as input so high impedance.
  pinMode(nSHUTTER, INPUT); // Set as input so high impedance.
  delay(50);
  toggleLED();

 // set the speed in RMP and turn on pins to driver stepper shield.
  myStepper.setSpeed(24);
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);
  digitalWrite(9,HIGH);
  digitalWrite(10,HIGH);
  delay(50);
  toggleLED();

  Serial.begin(9600);
  if (VERBOSE)  {Serial.println("\r\n\fRailduino Setup Done");  }
  //  Serial.println (MAX_STEPS);
  delay(50);
  toggleLED();
  
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  }


void  loop()  {

  //Heart Beat
  if (HEARTBEAT/2 < (millis()-lastchange)){
    toggleLED();
 //  if (VERBOSE)  {Serial.println("Toggle LED"); }
    lastchange = millis();
  }

  //User interface on serial port.
  int  inByte;
//  if (0) {
  if (Serial.available() >0) {
    inByte = Serial.read();  //Add next char to string.
    
      switch (inByte)  {

        case 'f':
        case 'F':
        advance = -1;
        Serial.println("Set for Forward.") ;   
        break;
        
        case 'r':
        case 'R':
        advance = 1;
        Serial.println("Set for Reverse.") ;   
        break;
        
        case 'b':
        case 'B':
        Serial.println("Bump motor a step") ;  
        myStepper.step(advance*1);  //One step advance
        break;
        
        //Home the trolly by returning untill limit switch closes.
        case 'h':
        case 'H':
        Serial.println("Heading for Home.") ;  
        myStepper.step(1);  //TEMP One step back
        break;
        
        case 'g':
        case 'G':
        Serial.println("Motor is Go!") ;   
        go();
        Serial.println("Motor is now stoped.") ;   
        break;
        
        case 's':
        case 'S':
        Serial.println("This does nothing yet. Should be: Motor is Stopped.") ;   
        break;
        
        case 't':
        case 'T':
        Serial.println("This does nothing yet. Should be: Set for rail travel Time.") ;   
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

        case 'a':
        case 'A':
        Serial.println("Trigger camera Auto focus.") ; 
        digitalWrite(nFOCUS, LOW);
        delay(10);
        pinMode(nFOCUS, INPUT);
        break;

        case 'p':
        case 'P':
        Serial.println("Make Photo now.") ;   
        // Trigger auto focus
        digitalWrite(nFOCUS, LOW);
        delay(FOCUS_DELAY);
        pinMode(nFOCUS, INPUT);    //Make high impedance.
        // Release shutter
        digitalWrite(nSHUTTER, LOW);
        delay(10);
        pinMode(nSHUTTER, INPUT);  //Makd high impedance.
        break;

        case 'i':
        case 'I':
        Serial.println("Set up for photo interval.") ;   
        break;

        case 'n':
        case 'N':
        Serial.println("Set for number of photos to make < 500.") ; 
//        number_photos = get_number();
        number_photos = serial_get_int();
        Serial.print ("Number = ");
        Serial.println (number_photos);
        break;

        case 'm':  //M or m or NL prints menu.
        case 'M':
        case '\n':
        commandmenu();  
        break;

        case '\r':  //Swallow CR.
        break;

       default:
        Serial.println("Sorry, you did not pressed a command I know.");    
        Serial.print("You pressed a(n)--");    
        Serial.print(char(inByte));    
        Serial.println("--.");    

      }
  }
}  //End of loop()

//Functions go here.

//Motor turn and go back.
// MAX_REVOLUSTIONS takes us from one end to the other.
// MAX_STEPS takes us from one end to the other.
// A place holder for a real GO! function.
void go()  {
  if (advance) {
    Serial.println("counter clockwise");
  }  else Serial.println("clockwise");
 
  myStepper.step(advance*MAX_STEPS*(length_percent/100.0));
  toggleLED();
  delay(500);
  if (advance) {
    Serial.println("clockwise");
  }  else Serial.println("counter clockwise");
  myStepper.step(-1*advance*MAX_STEPS*(length_percent/100.0));
  delay(500);
}



//Command menu for Railduino
void commandmenu()  {
  Serial.println("\fRailduino Command Menu") ;
  Serial.println("F for Forward."); 
  Serial.println("R for Reverse."); 
  Serial.println("B for Bump motor a step."); 
  Serial.println("H for Home the trolly."); 
  Serial.println("G for motor and photos Go."); 
  Serial.println("S for motor and photos Stop."); 
  Serial.println("T to enter Time to travel rail."); 
  Serial.println("L/l increment or decrement percent Length of rail to travel"); 
  Serial.println("A to trigger Auto Focus on camera."); 
  Serial.println("P to make Photo now!"); 
  Serial.println("I to set photo Interval."); 
  Serial.println("N to set Number of photos during rail travel."); 
  Serial.println(""); //Leave space after manu.
  }  


//Toggle the LED on pin 13 for user feedback.
int valLED = LOW;  // variable to store LED state

void toggleLED()  {
  if (valLED == HIGH){
    digitalWrite(LED, LOW);
    valLED = LOW;
  } else {
    digitalWrite(LED, HIGH);
    valLED = HIGH;
  }  
}


//Get digit characters and combine as positive interger.
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
  } while ( (c != '\n') && (x <= 3000) ); // int in x.
  return (x);  // The number.
}
