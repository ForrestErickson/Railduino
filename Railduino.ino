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

//Initiliaze Hardware

void  setup()  {
  pinMode(LED, OUTPUT);
  digitalWrite(LED,HIGH);
  delay(50);
  toggleLED();
  delay(50);
  toggleLED();
  
  Serial.begin(9600);
  if (VERBOSE)  {Serial.println("\r\nRailduino Setup Done");  }
}


void  loop()  {
  //Heart Beat
  //delay(500);
  //toggleLED();
//  if (VERBOSE)  {Serial.println("Toggle LED"); }

  int  inByte;
//  char  serial;
//  String  serial;
  
  if (Serial.available() >0) {
    //Serial.println(Serial.read());  //Echo it.
    inByte = Serial.read();  //Add next char to string.
    
      switch (inByte)  {
        case 'a':
        Serial.println("You pressed an 'a'.") ;   
        break;
        case 'A':
        Serial.println("You pressed an 'A'.") ;   
        break;
        default:
        Serial.println("You did not pressed an 'a'.");    
        Serial.print("You pressed a ");    
        Serial.println(char(inByte));    

      }
  }
    
    toggleLED();
}

//Functions go here.

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

