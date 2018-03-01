//void stateLoop(void)
//{
//  /* This is where all important actions are taken.
//   *
//   */
//
//  // Update ADC values and Digital Inputs
//  updateInputs();
//
//  // Updates the current state of the DBW board
//  updateState();
//
//  // Determine the drivers intent
//  updateDriverIntent();
//
//  // ============================= PERFORM PROPER ACTIONS BASED ON STATE ===========================
//  switch (State)
//  {
//
//    // ------------------------- ACTIVE PASS THROUGH --------------------------
//    case ACTIVE_PASS_THROUGH_STATE:
//
//      // check to see if we're coming out of Auto Paused.
//      if (previousState == AUTONOMOUS_STATE && pauseRequested)
//      {
//        // we should be in park at this point with driver's foot off the brake and throttle
//        // we need to ramp down the brakes back to zero before transitioning to active pass through.
//        pauseInEffect = true;
//      }
//
//      if (pauseInEffect)
//      {
//        // Decay efforts to zero
//        brakePercentEffort  = 0.995 * brakePercentEffort;
//        throttlePercentEffort = 0.995 * throttlePercentEffort;
//
//        setDACsFromPercentEffort((int)round(brakePercentEffort), (int)round(throttlePercentEffort)); // also sets brake lights
//
//        if (round(brakePercentEffort) <= 3) { pauseInEffect = false; } // Finished with ramp down to zero.
//      }
//      else // ramping from paused State no longer required, just perform active pass through function
//      {
//        desiredDAC1val = ai_C0;
//        desiredDAC2val = ai_C1;
//        desiredDAC3val = ai_A0;
//        desiredDAC4val = ai_A1; // Set to be the same ai_A0 because of toyota's ECUs
//        desiredDAC5val = ai_A2;
//        desiredDAC6val = ai_A3;
//
//        setDACsFromValues(desiredDAC1val, desiredDAC2val, desiredDAC3val, desiredDAC4val, desiredDAC5val, desiredDAC6val);
//      }
//
//      // Set DAC outputs Here
//      updateAnalogOutputs();
//
//    break;
//
//    // ------------------------------ AUTONOMOUS ------------------------------
//    case AUTONOMOUS_STATE:
//
//      setBlinkers();
//      if (pauseRequested)
//      {
//        if (previousState == ACTIVE_PASS_THROUGH_STATE)
//        { // if we just came out of active pass through with driver's feet off the pedals, we should start the ramp at zero% brake, zero% throttle
//          brakePercentEffort = 0.0;
//          throttlePercentEffort = 0.0;
//        }
//        // ramp up braking to 90%, ramp down throttle to 0%
//        brakePercentEffort  = 0.9997 * brakePercentEffort + 0.0003 * 90.0;  // 90% brakes
//        throttlePercentEffort = 0.9997 * throttlePercentEffort;       // 0% throttle
//
//        pauseInEffect = true;
//      }
//      else // no pause requested, full auto
//      {
//        // get both braking efforts and add them (in case driver wants more braking during auto)
//        brakePercentEffortTotal = brakePercentEffortDriver + brakePercentEffortTablet;
//        if (brakePercentEffortTotal > 99) { brakePercentEffortTotal = 99; }
//
//        if (pauseInEffect)
//        {
//          // pause was just released, we need to smoothly blend to the currently commanded braking and throttle efforts
//          brakePercentEffort  = 0.9995 * brakePercentEffort + 0.0005 * (double)brakePercentEffortTotal;
//          throttlePercentEffort = 0.9995 * throttlePercentEffort + 0.0005 * (double)throttlePercentEffortTablet;
//
//          if (abs((int)round(brakePercentEffort) - brakePercentEffortTotal) < 3){
//            // clear pauseInEffect flag when matched close enough
//            pauseInEffect = false;
//          }
//        }
//        else // pause not in effect
//        {
//          // this exponential filter is to smooth transitions between states
//          brakePercentEffort  = 0.97 * brakePercentEffort + 0.03 * (double)brakePercentEffortTotal;
//          throttlePercentEffort = 0.97 * throttlePercentEffort + 0.03 * (double)throttlePercentEffortTablet;
//        }
//      }
//      setDACsFromPercentEffort((int)round(brakePercentEffort), (int)round(throttlePercentEffort)); // also sets brake lights.
//
//      // Set DAC outputs Here
//      updateAnalogOutputs();
//
//    break;
//
//    // ------------------------------ ESTOPPED! ------------------------------------
//    case ESTOPPED_STATE:
//      // ramp up braking to 90%, ramp down throttle to 0%
//      brakePercentEffort  = 0.9995 * brakePercentEffort + 0.0005 * 90.0;
//      throttlePercentEffort = 0.995 * throttlePercentEffort;
//
//      setDACsFromPercentEffort((int)round(brakePercentEffort), (int)round(throttlePercentEffort)); // also sets brake lights.
//
//      // Set DAC outputs Here
//      updateAnalogOutputs();
//
//    break;
//
//    // ------------------------------ INACTIVE ------------------------------------
//    case INACTIVE_STATE:
//
//      // Check brake stroke sensors voltage level. This stuff is probably not needed. Could be though.
//      if(ai_C0 < 0.1 && ai_C1 < 0.1)
//      {
//        voltageLow = true;
//        if(prevVoltageLow != voltageLow){
//          prevVoltageLow = voltageLow;
//          jumpVoltages(false);
//        }
//      }
//      else
//      {
//        voltageLow = false;
//        if(prevVoltageLow != voltageLow){
//          prevVoltageLow = voltageLow;
//          jumpVoltages(true);
//        }
//      }
//
//      // Decide based on sensor that whether to pass voltages or trigger switch based on physical switch
//      if(brakePressed && voltageLow)
//      {
//        Dio_WriteBit(&brakeLights_C2, 1);
//        brakeLights = 1;
//      }
//      // If brake is not pressed when inactive and brakeLights are on, then toggle them off
//      else if(!brakePressed && brakeLights == 1 && voltageLow)
//      {
//        Dio_WriteBit(&brakeLights_C2, 0);
//        brakeLights = 0;
//      }
//      else if(!voltageLow)
//      {
//        desiredDAC1val = ai_C0;
//        desiredDAC2val = ai_C1;
//        desiredDAC3val = ai_A0;
//        desiredDAC4val = ai_A1; // Set to be the same ai_A0 because of toyota's ECUs
//        desiredDAC5val = ai_A2;
//        desiredDAC6val = ai_A3;
//
//        if(ai_A2 < ACCEL_ZERO_1) { desiredDAC5val = ACCEL_ZERO_1; }
//        if(ai_A3 < ACCEL_ZERO_2) { desiredDAC6val = ACCEL_ZERO_2; }
//
//        setDACsFromValues(desiredDAC1val, desiredDAC2val, desiredDAC3val, desiredDAC4val, desiredDAC5val, desiredDAC6val);
//      }
//
//    break;
//
//  } // End of switch case
//
//} // End of function
//
//void updateState(void)
//{
//  /* Logic of which state the myRIO is in.
//   * Notes: Autonomous state is used when in Tele-Op mode on tablet.
//   */
//
//  previousState = State;
//
//  if(ignitionSwitch)
//  {
//    if (estopPressed && autoSwitch)
//    {
//      State = ESTOPPED_STATE;
//    }
//    else if (autoSwitch && autoRequested)
//    {
//      State = AUTONOMOUS_STATE; // also includes sub-states, a) waiting for auto request and b) pause
//    }
//    // If the ignition is on and nothing else is triggering, then the state is APT (Man Active).
//    else
//    {
//      State = ACTIVE_PASS_THROUGH_STATE;
//    }
//  }
//  else
//  {
//    State = INACTIVE_STATE;
//  }
//
//  if(State != previousState){ setStateLCD(); }  // Only change LED when the state has changed. Saves function calls.
//
//} // End of function
//
//void setStateLED(void)
//{
//  /*
//   * This function is used to update the LEDs on the front of myRIO so that they
//   * represent the state that the DBW board is in currently. Reading the bit appears
//   * to be the only way to clear the LED after it has be turned on.
//   */
//  switch (State){
//
//    case 1: // Active Pass Through aka Manual Active
//      if (pauseRequested && pauseInEffect) // Pause requested and pause is in effect.
//      {
//        Dio_WriteBit(&LED1, 1);
//        Dio_ReadBit(&LED2);
//        Dio_WriteBit(&LED3, 1);
//      }
//      else if (pauseInEffect) // Pause is not requested but pause in effect.
//      {
//        Dio_WriteBit(&LED1, 1);
//        Dio_WriteBit(&LED2, 1);
//        Dio_ReadBit(&LED3);
//      }
//      else // Should be running in Man Active
//      {
//        Dio_WriteBit(&LED1, 1);
//        Dio_ReadBit(&LED2);
//        Dio_ReadBit(&LED3);
//      }
//    break;
//
//    case 2: // Autonomous
//
//      if (pauseRequested && pauseInEffect) // Pause requested and pause is in effect.
//      {
//        Dio_ReadBit(&LED1);
//        Dio_ReadBit(&LED2);
//        Dio_WriteBit(&LED3, 1);
//      }
//      else if (pauseInEffect) // Pause is not requested but pause in effect.
//      {
//        Dio_ReadBit(&LED1);
//        Dio_WriteBit(&LED2, 1);
//        Dio_WriteBit(&LED3, 1);
//      }
//      else // Should be running in autonomous
//      {
//        Dio_ReadBit(&LED1);
//        Dio_WriteBit(&LED2, 1);
//        Dio_ReadBit(&LED3);
//      }
//    break;
//
//    case 3: // E-Stopped
//      Dio_WriteBit(&LED1, 1);
//      Dio_WriteBit(&LED2, 1);
//      Dio_WriteBit(&LED3, 1);
//    break;
//
//    case 4: // Inactive State
//      Dio_ReadBit(&LED1);
//      Dio_ReadBit(&LED2);
//          Dio_ReadBit(&LED3);
//    break;
//
//  } // End of switch case
//} // End of function
