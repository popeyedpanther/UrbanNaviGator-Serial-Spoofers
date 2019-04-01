#ifdef USE_OLD_MESSAGING
   void sendMessage()
   {
     /*
     * This function sends values over UART to the Tablet. The message structure can be
     * seen in the myRIO DBW board Word documentation.
     *
     */
    byte writeData[TX_PACKET_SIZE]={0};
    uint16_t tempUInt16;
    byte analogCnt, i;
    double tempDoubleADC[8]={10.0}, tempDoubleDAC[8]={10.0};
    static double fiveVoltK      = 0.001220703;  //---------- Conversions between 12 bit ADC and real numbers
  
    //numbers2Array(ai_C0, ai_C1, ai_A0, ai_A1, ai_A2, ai_A3, tempDoubleADC);
    //numbers2Array(ao_C0, ao_C1, ao_A0, ao_A1, ao_B0, ao_B1, tempDoubleDAC);
  
    // Write information to Tablet
    writeData[0] = 65;  // ---------
    writeData[1] = 128; //  Header
    writeData[2] = 254; //
    writeData[3] = 68;  // ---------
    writeData[4] = (State == ESTOPPED_STATE) ? 170 : 0;   // eStop: 170 if the state is ESTOPPED_STATE
    writeData[5] = (State == AUTONOMOUS_STATE) ? 200 : 0; // Auto:  200 if the state is AUTONOMOUS_STATE
    writeData[6] = brakePressed == 1 ? 1 : 0;
    writeData[7] = writeData[6];
    // Use these to store throttle/brake effort
    writeData[8] = 0;// Brake Effort
    writeData[9] = 60; // Throttle Effort
  
    analogCnt = 0;
    for(i = 10; i < 25; i+=2){
      // Write the DAC values into message
      tempUInt16 = (uint16_t)(tempDoubleDAC[analogCnt]/fiveVoltK);
      writeData[i] = tempUInt16 & 0xFF;
      writeData[i+1] = (tempUInt16 >> 8) & 0xFF;
  
      // Write the ADC values into message, there are sent in reverse order
      tempUInt16 = (uint16_t)(tempDoubleADC[7-analogCnt]/fiveVoltK);
      writeData[i+16] = tempUInt16 & 0xFF;
      writeData[i+17] = (tempUInt16 >> 8) & 0xFF;
  
      analogCnt++;
    }
  
    writeData[42] = 78; // End Byte
  
    // Write data to serial.
    Serial.write(writeData, 43);
  }

#else
  void sendMessage()
  {
    byte sendMessageData[TX_PACKET_SIZE];
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

    sendMessageData[0] = 157;
    sendMessageData[1] = stateValue;
    sendMessageData[2] = pauseInEffect ? 128 : 64;
    sendMessageData[3] = (byte)brakePercentEffort;
    sendMessageData[4] = (byte)throttlePercentEffort;
   
    // Calculate CRC check
    for (int i = 0; i < 4; i++)
    {
      crcString.concat(sendMessageData[i]);
    }
  
    sendMessageData[5] = (unsigned int)crcString.toInt() % CRC_DIVIDER;
    sendMessageData[0] = 147;
    
    //Write data to serial
    Serial.write(157);
    Serial.write(sendMessageData, 5);
    Serial.write(147);
    
  }
#endif
