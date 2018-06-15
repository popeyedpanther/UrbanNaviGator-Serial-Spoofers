void sendMessage()
{
  byte sendMessageData[5];
  byte stateValue = 0;
  String crcString;
  
  // Gather information into writable format
  switch(State)
  {
    case MANUAL_STATE:
      stateValue = 64;
    break;
    case AUTONOMOUS_STATE:
      stateValue = 128;
    break;
    case ESTOPPED_STATE:
      stateValue = 255;
    break;
    case INACTIVE_STATE:
      stateValue = 32;
    break;
  }
  
  sendMessageData[0] = stateValue;
  sendMessageData[1] = pauseInEffect ? 128 : 64;
  sendMessageData[2] = (byte)brakePercentEffort;
  sendMessageData[3] = (byte)throttlePercentEffort;
 
  // Calculate CRC check
  for (int i = 0; i < 4; i++)
  {
    crcString.concat(sendMessageData[i]);
  }

  sendMessageData[4] = (unsigned int)crcString.toInt() % CRC_DIVIDER;
  
  //Write data to serial
  Serial.write(157);
  Serial.write(sendMessageData, 5);
  Serial.write(147);
  
}

