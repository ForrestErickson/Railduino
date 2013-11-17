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

#include <Stepper.h>
#include <String.h>  //Allows sting comparison.

//Constants
const  int LED = 13;  // The Arduino LED.  Also LED IN4 on the motor shield.
const  int VERBOSE = 1; //User VEBRBOSE for development
const  int  HEARTBEAT = 1000;  //Period of heart beat in milliseconds
//Initiliaze Hardware

void  setup()  {
  pinMode(LED, OUTPUT);
  digitalWrite(LED,HIGH);
  delay(50);
  toggleLED();
  delay(50);
  toggleLED();
  
  Serial.begin(9600);
  if (VERBOSE)  {Serial.println("\r\n\fRailduino Setup Done");  }
}

//unsigned long lastchange = millis();
unsigned long lastchange = 0;

void  loop()  {

  //Heart Beat
  if (HEARTBEAT/2 < (millis()-lastchange)){
    toggleLED();
    lastchange = millis();
  }
 

//  if (VERBOSE)  {Serial.println("Toggle LED"); }

  int  inByte;
//  char  serial;
//  String  serial;
  
//  if (0) {
  if (Serial.available() >0) {
    //Serial.println(Serial.read());  //Echo it.
    inByte = Serial.read();  //Add next char to string.
    
      switch (inByte)  {

        case 'f':
        case 'F':
        Serial.println("Set for Forward.") ;   
        break;
        
        case 'r':
        case 'R':
        Serial.println("Set for Reverse.") ;   
        break;
        
        case 'g':
        case 'G':
        Serial.println("Mortor is Go!") ;   
        break;
        
        case 's':
        case 'S':
        Serial.println("Motor is Stoped.") ;   
        break;
        
        case 't':
        case 'T':
        Serial.println("Set for rail travel Time.") ;   
        break;
        
        case 'l':
        case 'L':
        Serial.println("Set travel for percent Length of rail.") ;   
        break;

        case 'a':
        case 'A':
        Serial.println("Trigger camera Auto focus.") ;   
        break;

        case 'p':
        case 'P':
        Serial.println("Make Photo now.") ;   
        break;

        case 'i':
        case 'I':
        Serial.println("Set up for photo interval.") ;   
        break;

        case 'n':
        case 'N':
        Serial.println("Set for number of photos to make.") ;   
        break;

        case 'm':
        case 'M':
        case '\n':
//        case '\r':
        commandmenu();  
        break;

        case '\r':
        break;

       default:
        Serial.println("Sorry, you did not pressed a command I know.");    
        Serial.print("You pressed a(n)--");    
        Serial.print(char(inByte));    
        Serial.println("--.");    

      }
  }
}

//Functions go here.

//Command menu for Railduino
void commandmenu()  {
  Serial.println("\fRailduino Command Menu") ;
  Serial.println("F for Forward."); 
  Serial.println("R for Reverse."); 
  Serial.println("G for motor and photos Go."); 
  Serial.println("S for motor and photos Stop."); 
  Serial.println("T to enter Time to travel rail."); 
  Serial.println("L for percent Length of rail to travel"); 
  Serial.println("A to trigger Auto Focus on camera."); 
  Serial.println("P to make Photo now!"); 
  Serial.println("I to set photo Interval."); 
  Serial.println("N to set Number of photos during rail travel."); 
  Serial.println(""); //Leave space after manu.
  }  


//const int LED  = 13;  //Build in LED.
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

