//======================================================================================
//============================Serial Message Parser Function============================
//======================================================================================

int parseReceivedMessage(byte* message)
{
  String crcString;
  // Check CRC
  for(int i = 0; i < RX_PACKET_SIZE-3; i++)
  {
    crcString.concat(message[i]);
  }
  
  if(((unsigned int)crcString.toInt() % CRC_DIVIDER) == message[RX_PACKET_SIZE-2])
  {
    autoRequested = message[0] == 128;
    leftBlinker = message[1] == 128;
    rightBlinker = message[2] == 128;     
    pauseRequested = message[3] == 128;
    brakePercentEffortTablet  = (int)message[4];
    throttlePercentEffortTablet   = (int)message[5];

    // Cap the brake and throttle efforts
    if(brakePercentEffortTablet > 99){ brakePercentEffortTablet = 99; }
    else if(brakePercentEffortTablet < 0) { brakePercentEffortTablet = 0;}
    if(throttlePercentEffortTablet > 99){ throttlePercentEffortTablet = 99; }
    else if(throttlePercentEffortTablet < 0) { throttlePercentEffortTablet = 0;}
    
    sendResponse(true);
  }
  else
  {
    // Did not pass the CRC test, data corrupted. Request another message.
    sendResponse(false);
  }
  messageComplete = false;
}

//======================================================================================
//=================================Serial Event Function================================
//======================================================================================
void serialEvent() 
{ 
  int counter = 0;
  
  while (Serial.available()) 
  {
    // get the new byte:
    int inByte = Serial.read();

    // Check if this is the beginning of a message
    if(inByte == 137)
    { 
      messageStarted = true; 
    }
    else if(messageStarted) // start adding to received message
    {
      if(messageComplete){ messageComplete = false; counter = 0; } // If we got another message before last was read. reset flags and overwrite.

      if(inByte == 127){ messageComplete = true; }
      else
      {
        receivedMessage[counter] = (byte)inByte;
        counter++;
      }
    }
  }
}


//======================================================================================
//=====================================Helper Functions==================================
//======================================================================================
void resetReceivedMessageBooleans()
{
  /*
   * This function flushes the last received data from the Tablet. Should only happen on
   * transitions out of Autonomous.
   */
  autoRequested = 0;
  pauseRequested = 0;
  brakePercentEffortTablet  = 0;
  throttlePercentEffortTablet   = 0;
  leftBlinker = 0;
  rightBlinker = 0;
}


void sendResponse(byte Correct)
{
  Serial.write(137);
  if(Correct){ Serial.write(128); }
  else{ Serial.write(64); }
  Serial.write(127);
}


