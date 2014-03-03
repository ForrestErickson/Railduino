
//Test state of limit switches.
//If limit reached then reverse motor by setting "advance" to opposite
void test_limit_switches()  {
  if (  !digitalRead(nFAR_LIMIT) )  {
    //  if (  !digitalRead(nFAR_LIMIT) && 0)  {
    Serial.println("Far limit switch reached");  //D4 switch
    advance = -1;  //Time to go reverse again.
  }

  if (  !digitalRead(nNEAR_LIMIT))  {
    Serial.println("Near limit switch reached");  //D2 switch
    advance = 1;  //Time to go forward again.
  }

}


