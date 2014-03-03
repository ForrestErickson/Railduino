//Motor turn to and fro for show and tell.
void wave()  {
  Serial.println("I say, Hello!");
  myStepper.step(advance*MAX_STEPS*0.5*(length_percent/100.0));
  toggleLED();
  delay(500);
  Serial.println("Good day to you!");
  myStepper.step(-1*advance*MAX_STEPS*(length_percent/100.0));
  delay(500);
  Serial.println("Good show!");
  myStepper.step(advance*MAX_STEPS*0.5*(length_percent/100.0));
  toggleLED();
  delay(500);
  Serial.println("Tooda Loo!");
}


