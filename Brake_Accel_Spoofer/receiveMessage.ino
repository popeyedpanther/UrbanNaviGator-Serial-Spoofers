//======================================================================================
//============================Serial Message Parser Function============================
//======================================================================================

int parseReceivedMessage(String message)
{
  // Implemented for communications with the steering SmartMotor
  
  int indexForRemove;
  String numberString = "";

  indexForRemove = message.indexOf("\n");
  if(indexForRemove == -1){ indexForRemove = message.indexOf("\r"); } //Check for carriage return also
  
  if(indexForRemove == -1){ return -1;} // If still -1 then there was an error
  message.remove(indexForRemove);
  
  if(message.length() < 3 || message.length() > 7){ return -1;}   // Message length is incorrect, return -1 for error
  
  if(message.startsWith("p"))
  {
    return 0;
  }
  else if(message.equals("RUN"))
  {
    return 175; // Odd ball, will rehome and reset if it recieves. This to allow not having to shutoff the device to reset.
  }
  else if(message.equals("f=2"))
  {
    return 55;
  }
  else
  {
    return -2;
  }
}

//======================================================================================
//=================================Serial Event Function================================
//======================================================================================

void serialEvent() 
{
  while (Serial.available()) 
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    recievedMessage += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n' or inChar == '\r') 
    {
      stringComplete = true;
    }
  }
}
