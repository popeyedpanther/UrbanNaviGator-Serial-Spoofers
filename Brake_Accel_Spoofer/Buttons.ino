// Make sure you are using the DFRobot LCD screen with the long buttons.
void updateButtons()
{
  int buttonValue = 0;
  static unsigned int counterOn = 0;
  
    buttonValue = analogRead(0);
    if(buttonValue > 820 && buttonValue < 830) // "Select" Button
    { //Should update E-stop
      counterOn++;
      if(counterOn > 10 && counterOn <= 11)
      {
        buttonPressed[0] = !buttonPressed[0];
        estopPressed = buttonPressed[0];
        displayDataChanged = true;
      }
    }
    else if(buttonValue > 620 && buttonValue < 630) // "Left" Button
    { //Should update Brake Switch
      counterOn++;
      if(counterOn > 10 && counterOn <= 11)
      {
        buttonPressed[1] = !buttonPressed[1];
        brakePressed = buttonPressed[1];
        displayDataChanged = true;
      }
    }
    else if(buttonValue > 200 && buttonValue < 210) // "Up" Button
    { // Should update Auto Switch
      counterOn++;
      if(counterOn > 10 && counterOn <= 11)
      {
        buttonPressed[2] = !buttonPressed[2];
        autoSwitch = buttonPressed[2];
        displayDataChanged = true;
      }
    }
    else if(buttonValue > 405 && buttonValue < 415) // "Down" Button
    { // Should Update Ignition Switch
      counterOn++;
      if(counterOn > 10 && counterOn <= 11)
      {
        buttonPressed[3] = !buttonPressed[3];
        ignitionSwitch = buttonPressed[3];
        displayDataChanged = true;
      }
    }
    else if(buttonValue == 0) // "Right" Button
    {
      counterOn++;
      if(counterOn > 10 && counterOn <= 11)
      {
        buttonPressed[4] = !buttonPressed[4];
        displayDataChanged = true;
      }
    }
    else
    {
      counterOn = 0;
    }
}

