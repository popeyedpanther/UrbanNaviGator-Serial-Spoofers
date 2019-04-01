//======================================================================================
//============================Serial Message Parser Function============================
//======================================================================================
#ifdef USE_OLD_MESSAGING
  void parseReceivedMessage(byte* message)
  {
    if (message[0] == 55 && message[1] == 93 && message[2] == 200 && message[3] == 30)
    {
      if(message[15] == 88)
      {
        // Header was correct; read the rest
        autoRequested = message[0] == 128;
        brakePercentEffortTablet = (int)message[1];
        throttlePercentEffortTablet = (int)message[2];
        leftBlinker = message[3] == 128;
        rightBlinker = message[4] == 128;
        pauseRequested = message[5] == 128;
  
        // Cap the brake and throttle efforts
        if(brakePercentEffortTablet > 99){ brakePercentEffortTablet = 99; }
        else if(brakePercentEffortTablet < 0) { brakePercentEffortTablet = 0;}
        if(throttlePercentEffortTablet > 99){ throttlePercentEffortTablet = 99; }
        else if(throttlePercentEffortTablet < 0) { throttlePercentEffortTablet = 0;}
      }
    }
      
    messageComplete = false;
  }
#else
  void parseReceivedMessage(byte* message)
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
#endif
//======================================================================================
//=================================Serial Event Function================================
//======================================================================================
// This whole function code cause asynchronous issues. Replacement message data while it is being read.
// or not storing all the data if it does not all arrive in a timely manner.
void serialEvent() 
{ 
  int counter = 0;
  
  while (Serial.available()) 
  {
    // get the new byte:
    int inByte = Serial.read();

    #ifdef USE_OLD_MESSAGING
    // Check if this is the beginning of a message

    if(messageStarted) // start adding to received message
    {
      if(messageComplete){ messageComplete = false; counter = 0; } // If we got another message before last was read. reset flags and overwrite.
      // This could possible cause problems. The code could be in the process of storing the message as it is replaced.

      if(inByte == 88 && counter == RX_PACKET_SIZE)
      {
        receivedMessage[RX_PACKET_SIZE-1] = inByte;
        messageComplete = true;
        messageStarted = false; 
      }
      else
      {
        receivedMessage[counter] = (byte)inByte;
        counter++;
      }
    }
    else if(inByte == 55)
    { 
      messageStarted = true;
      receivedMessage[0] = inByte; 
    }


    #else

    if(messageStarted) // start adding to received message
    {
      if(messageComplete){ messageComplete = false; counter = 0; } // If we got another message before last was read. reset flags and overwrite.

      if(inByte == 127)
      { 
        messageComplete = true;
        messageStarted = false;
      }
      else
      {
        receivedMessage[counter] = (byte)inByte;
        counter++;
      }
    }
    // Check if this is the beginning of a message
    else if(inByte == 137)
    { 
      messageStarted = true; 
    }
    #endif
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

#ifndef USE_OLD_MESSAGING
void sendResponse(byte Correct)
{
  Serial.write(137);
  if(Correct){ Serial.write(128); }
  else{ Serial.write(64); }
  Serial.write(127);
}
#endif

