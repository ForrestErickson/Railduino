//Toggle the LEDs on near and far end for user feedback.
//For development toggle the near and far LEDs too.
//When not advancing both LEDs are on.
int valLED = LOW;  // variable to store LED state

void toggleLED()  {
  if (valLED == HIGH){
    if (going ==1)  {
      if (advance == 1)  {
        digitalWrite(FAR_LED, LOW);
      }
      else {
        digitalWrite(NEAR_LED, LOW);
      }
    }
    //    digitalWrite(LED, LOW);  //LED on pin 13.
    valLED = LOW;

  } 
  else {
    digitalWrite(NEAR_LED, HIGH);
    digitalWrite(FAR_LED, HIGH);
    //    digitalWrite(LED, HIGH);
    valLED = HIGH;
  }  
}


