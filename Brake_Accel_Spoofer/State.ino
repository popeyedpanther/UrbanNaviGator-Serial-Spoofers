void stateLoop(void)
{
  /* This is where all important actions are taken.
   *
   */

  static byte rampedFromZero = false;
  
  // Update ADC values and Digital Inputs
  //updateInputs();

  // Updates the current state of the DBW board
  updateState();

  // Determine the drivers intent
  //updateDriverIntent();

  // ============================= PERFORM PROPER ACTIONS BASED ON STATE ===========================
  switch (State)
  {

    // ------------------------- Manual --------------------------
    case MANUAL_STATE:
   
//      desiredDAC1val = ai_C0;
//      desiredDAC2val = ai_C1;
//      desiredDAC3val = ai_A0;
//      desiredDAC4val = ai_A1; // Set to be the same ai_A0 because of toyota's ECUs
//      desiredDAC5val = ai_A2;
//      desiredDAC6val = ai_A3;
//
//      setDACsFromValues(desiredDAC1val, desiredDAC2val, desiredDAC3val, desiredDAC4val, desiredDAC5val, desiredDAC6val);
//
//      // Set DAC outputs Here
//      updateAnalogOutputs();

    break;

    // ------------------------------ AUTONOMOUS ------------------------------
    case AUTONOMOUS_STATE:

      //setBlinkers();
      if (pauseRequested)
      {
        if (!rampedFromZero)
        { // if we just came out of active pass through with driver's feet off the pedals, we should start the ramp at zero% brake, zero% throttle
          brakePercentEffort = 0.0;
          throttlePercentEffort = 0.0;
          rampedFromZero = true;
        }
        // ramp up braking to 90%, ramp down throttle to 0%
        brakePercentEffort  = 0.9997 * brakePercentEffort + 0.0003 * 90.0;  // 90% brakes
        throttlePercentEffort = 0.9997 * throttlePercentEffort;       // 0% throttle

        pauseInEffect = true;
      }
      else // no pause requested, full auto
      {
        if(rampedFromZero){ rampedFromZero = false; } // Reset this variable for later use.
        
        // get both braking efforts and add them (in case driver wants more braking during auto)
        brakePercentEffortTotal = brakePercentEffortDriver + brakePercentEffortTablet;
        if (brakePercentEffortTotal > 99) { brakePercentEffortTotal = 99; }

        if (pauseInEffect)
        {
          // pause was just released, we need to smoothly blend to the currently commanded braking and throttle efforts
          brakePercentEffort  = 0.9995 * brakePercentEffort + 0.0005 * (double)brakePercentEffortTotal;
          throttlePercentEffort = 0.9995 * throttlePercentEffort + 0.0005 * (double)throttlePercentEffortTablet;
          
          // clear pauseInEffect flag when matched close enough
          if (abs((int)round(brakePercentEffort) - brakePercentEffortTotal) < 3){ pauseInEffect = false; }
        }
        else if(transitionToDriverEffort)
        {
          // Manual state is desired, blend efforts to driver's input
          brakePercentEffort  = 0.9995 * brakePercentEffort + 0.0005 * (double)brakePercentEffortDriver;
          throttlePercentEffort = 0.9995 * throttlePercentEffort;

          // Blending of efforts close enough, set completed variable.
          if (abs((int)round(brakePercentEffort) - brakePercentEffortDriver) < 3){ transitionToDriverEffortComplete = true; }
        }
        else
        {
          // Normal autonomous operation, this exponential filter is to smooth transitions between states
          brakePercentEffort  = 0.97 * brakePercentEffort + 0.03 * (double)brakePercentEffortTotal;
          throttlePercentEffort = 0.97 * throttlePercentEffort + 0.03 * (double)throttlePercentEffortTablet;
        }
      }
      //setDACsFromPercentEffort((int)round(brakePercentEffort), (int)round(throttlePercentEffort)); // also sets brake lights.

      // Set DAC outputs Here
      //updateAnalogOutputs();

    break;

    // ------------------------------ ESTOPPED! ------------------------------------
    case ESTOPPED_STATE:
      // ramp up braking to 90%, ramp down throttle to 0%
      brakePercentEffort  = 0.9995 * brakePercentEffort + 0.0005 * 90.0;
      throttlePercentEffort = 0.995 * throttlePercentEffort;

      //setDACsFromPercentEffort((int)round(brakePercentEffort), (int)round(throttlePercentEffort)); // also sets brake lights.

      // Set DAC outputs Here
      //updateAnalogOutputs();

    break;

    // ------------------------------ INACTIVE ------------------------------------
    case INACTIVE_STATE:

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
//      // Decide based on sensor whether to pass voltages or trigger brake lights based on physical switch
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

    break;

  } // End of switch case

} // End of function


void updateState(void)
{
  /*  
   *  Logic of which state the myRIO is in.
   *  Notes: Autonomous state is used when in Tele-Op mode on tablet.
   *  E-stop is always allowed, this could cause errors for the Toyota is used at inappropiate times
   *  Like if the state is INACTIVE but the switch is pressed. This will apply brakes.
   */

  updateDesiredState();

  if(checkTransitionRequirements())
  {
    if(State != desiredState){ displayDataChanged = true; } // Only change state display when state has changed.
    State = desiredState;  
  }
} // End of function


void updateDesiredState(void)
{
  /*  Logic of which state the myRIO is in.
   *  Notes: Autonomous state is used when in Tele-Op mode on tablet.
   *  E-stop is always allowed, this could cause errors for the Toyota is used at inappropiate times
   *  Like if the state is INACTIVE but the switch is pressed. This will apply brakes.
   */
  static byte previousDesiredState = 0;
   
  previousDesiredState = desiredState;

  if (estopPressed)
  {
    desiredState = ESTOPPED_STATE;
  }
  else if(ignitionSwitch)
  {
    if (autoSwitch && autoRequested)
    {
      desiredState = AUTONOMOUS_STATE; // also includes sub-states, a) waiting for auto request and b) pause
    }
    // If the ignition is on and nothing else is triggering, then the state is APT (Man Active).
    else
    {
      desiredState = MANUAL_STATE;
    }
  }
  else
  {
    desiredState = INACTIVE_STATE;
  }

  if(desiredState != previousDesiredState){ displayDataChanged = true; }  // Only change state display when state has changed.

} // End of function


byte checkTransitionRequirements()
{
  /*
   *  This function checks to make sure the prerequisites for the desired state transition 
   *  are completed before the state can transition.
   */

  static byte waitOneIteration = false;
   
  if(State == ESTOPPED_STATE && desiredState != ESTOPPED_STATE)
  {
    // Should return to Manual after E-stop has been released.
    resetReceivedMessageBooleans();
    if(!waitOneIteration) {waitOneIteration = true; return false;}
    else { waitOneIteration = false; return true; }
  }
  else if(State == AUTONOMOUS_STATE && desiredState == MANUAL_STATE)
  {
    if(transitionToDriverEffort = false){ resetReceivedMessageBooleans(); }
    transitionToDriverEffort = true;
    if(transitionToDriverEffortComplete)
    { 
      transitionToDriverEffort = false; 
      transitionToDriverEffortComplete = false; 
      return true; 
    }
    else{ return false; }
  }
  else{ return true; }
  
} // End of Function
