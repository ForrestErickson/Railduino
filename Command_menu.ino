//Command menu for Railduino
void commandmenu()  {
  Serial.println("\fRailduino Command Menu") ;
  Serial.println("F for Forward.");
  Serial.println("R for Reverse.");
  Serial.println("W for Wave motor forward and back.");
  Serial.println("R for Reverse.");
  Serial.println("H for Home the trolly.");
  Serial.println("G for motor and photos Go.");
  Serial.println("s for motor and photos sTOP.");
  Serial.println("S to set motor Speed.");
  Serial.println("T to report Time to travel rail.");
  Serial.println("L/l increment or decrement percent Length of rail to travel");
  Serial.println("E to set Exposure in seconds.");
  Serial.println("A to trigger Auto Focus on camera.");
  Serial.println("P to make Photo now!");
  Serial.println("I to set photo Interval.");
  Serial.println("N to set Number of photos during rail travel.");
  Serial.println("M to refresh the Menu.");
  Serial.println(""); //Leave space after menu.
}  


