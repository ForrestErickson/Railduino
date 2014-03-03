/*Report the setup variables */
void report_setup()  {
  Serial.println("SYSTEM SETUP:");
  Serial.print("\tWe have N = ");
  Serial.print(number_photos);
  Serial.println(" remaining exposures to make.");
  Serial.print("\tWe have Exposure time of E = ");
  Serial.print(camera_exposure);
  Serial.println(" seconds.");
  Serial.print("\tWe have camera advance Interval time of I = ");
  Serial.print(camera_delay_interval);
  Serial.println(" seconds.");
  Serial.print("\tMotor speed is = ");
  Serial.print(speed_motor);
  Serial.println(" steps per second.");
}

