// Make sure you are using the DFRobot LCD screen with the long buttons.
void updateButtons()
{
  int buttonValue = 0;
  static int counterOn = 0;
  
    buttonValue = analogRead(0);
    if(buttonValue > 823 && buttonValue < 828) // "Select" Button
    { //Should update E-stop
      counterOn++;
      if(counterOn > 10 && counterOn <= 11)
      {
        buttonPressed[0] = !buttonPressed[0];
        displayDataChanged = true;
      }
    }
    else if(buttonValue > 623 && buttonValue < 626) // "Left" Button
    { //Should update Brake Switch
      counterOn++;
      if(counterOn > 10 && counterOn <= 11)
      {
        buttonPressed[1] = !buttonPressed[1];
        displayDataChanged = true;
      }
    }
    else if(buttonValue > 204 && buttonValue < 208) // "Up" Button
    { // Should update Auto Switch
      counterOn++;
      if(counterOn > 10 && counterOn <= 11)
      {
        buttonPressed[2] = !buttonPressed[2];
        displayDataChanged = true;
      }
    }
    else if(buttonValue > 407 && buttonValue < 411) // "Down" Button
    { // Should Update Ignition Switch
      counterOn++;
      if(counterOn > 10 && counterOn <= 11)
      {
        buttonPressed[3] = !buttonPressed[3];
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

