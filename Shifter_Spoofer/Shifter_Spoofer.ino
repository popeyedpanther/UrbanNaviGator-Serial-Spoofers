/*  Written by: Patrick Neal
 *  Email: neap@ufl.edu
 *  Last Updated: 2/21/18
 *  
 *  Used to test new Apollo interface code for the Urban NaviGator,
 *  specifically spoofing the shifting SmartMotor
 *  
 * Notes:
 *  Button Scheme:
 * 
 *              (N)
 *    (P)  (R)       (B) 
 *              (D) 
 *            
 *  P = Park            
 *  R = Reverse
 *  N = Neutral
 *  D = Drive
 *  B = toggle brake pedal
 *  
 *  The brake switch is handled through toggling. Press "B" to go from high->low or low->high, High = pedal is pressed.
 *           
 */

#include <LiquidCrystal.h>

// ------Initializing Variables--------
byte initialized = false;         // Has the "Smart Motor" been initialized?
byte gotRun = false;              // Run was recieved during the initialization    
byte alreadyRecieved = false;     // "h=1" has already been recieved, continue on and wait for brake
byte alreadyRecievedRUN = false;  // "RUN" has already been recieved, while doing main loop. Used to skip searching for "RUN" in the initialization step.


// -----Shifter Variables-----
int positionCode = 10;            // Stores what gear the tablet wants the vehicle to be in, default 10 because 10 should never been found naturally in the code
byte actuationCode = 0;           // Could be useless, could just compare position code numbers and achieve the same effect.

// ------Serial Variables------
boolean stringComplete = false;  // whether the string is complete
String recievedMessage;

// ------Brake Switch Variables-----
byte brakeState = true, prevBrake = true; // Logical false, does not follow actualy brake state where low = true

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//======================================================================================
//=================================Setup and Home=======================================
//======================================================================================
void setup() {

  // Setup the LCD screen.

  lcd.begin(16,2);  // Specify the size of the display (16 columns, 2 rows)
  
  //Initialize the serial
  Serial.begin(38400);
}

//======================================================================================
//======================================Main Loop=======================================
//======================================================================================
void loop() {

  byte brakePressed = false;
  int  buttonValue = 666;   // Use to store which button has been pressed

  if(!initialized){
    initialized = initializeShifter();  //Perform the initialization steps the tablet is looking for.
  }
  else{

    brakePressed = getBrake();
    
    //---------------------------------------------------------------------------
  
    if(stringComplete){
      positionCode = parseReceivedMessage(recievedMessage);

      if(brakePressed){
        switch(positionCode){
          case 0:
            lcd.setCursor(0, 0);
            lcd.print("Park");
            Serial.print("ACTUATING_TO_PARK\r");
            actuationCode = 1;
            break;
          case 255:
            lcd.setCursor(0, 0);
            lcd.print("Reverse");
            Serial.print("ACTUATING_TO_REVERSE\r");
            actuationCode = 2;
            break;
          case 128:
            lcd.setCursor(0, 0);
            lcd.print("Neutral");
            Serial.print("ACTUATING_TO_NEUTRAL\r");
            actuationCode = 3;
            break;
          case 1:
            lcd.setCursor(0, 0);
            lcd.print("Drive");
            Serial.print("ACTUATING_TO_DRIVE\r");
            actuationCode = 4;
            break;
          case 2:
            lcd.setCursor(0, 0);
            lcd.print("Regen");
            Serial.print("ACTUATING_TO_REGEN\r");
            actuationCode = 5;
            break;
          case 175:
            initialized = false;
            alreadyRecievedRUN = true;
            break;
          case 55:
            Serial.print("NORMAL_SHUTDOWN\r");
            break;
          default:
            Serial.print("UNKNOWN_COMMAND\r");
            break;
        }
      }
      recievedMessage.remove(0);
      stringComplete = false;
    }
 
    buttonValue = analogRead(0);
    if(buttonValue > 719 && buttonValue < 722 && actuationCode==1){
      Serial.print("PARK\r");
      lcd.clear();
      actuationCode = 0;
    }
    else if(buttonValue > 478 && buttonValue < 481 && actuationCode==2){
      Serial.print("REVERSE\r");
      lcd.clear();
      actuationCode = 0;
    }
    else if(buttonValue > 304 && buttonValue < 308 && actuationCode==3){
      Serial.print("NEUTRAL\r");
      lcd.clear();
      actuationCode = 0;
    }
    else if(buttonValue > 129 && buttonValue < 132 && actuationCode==4){
      Serial.print("DRIVE\r");
      lcd.clear();
      actuationCode = 0;
    }
  }
}

//======================================================================================
//====================================Helper Functions=================================
//======================================================================================

byte getBrake(){
    static int counterOn = 0, counterOff = 0;
    int buttonValue = 666;
    
    //-----------------Check Brake Switch----------------------------------------
    buttonValue = analogRead(0);
    if(buttonValue == 0){
      counterOn++;
      if(counterOn > 10 && counterOn <= 11){
        brakeState = !brakeState;
      }
    }
    else{
      counterOn = 0;
    }

    if(prevBrake != brakeState){
      if(brakeState){
        // Brake is not pressed
        prevBrake = brakeState;
        lcd.setCursor(0,1);
        lcd.print("BRAKE_PEDAL_LOW ");
        Serial.print("BRAKE_PEDAL_LOW\r");
        return false;
      }
      else if(!brakeState){
        // Brake is pressed
        prevBrake = brakeState;
        lcd.setCursor(0,1);
        lcd.print("BRAKE_PEDAL_HIGH");
        Serial.print("BRAKE_PEDAL_HIGH\r");
        return true;
      }
    }
    else{
      return !brakeState;
    }
}


int parseReceivedMessage(String message){
  
  int indexForRemove;

  indexForRemove = message.indexOf("\n");
  if(indexForRemove == -1){ indexForRemove = message.indexOf("\r"); } //Check for carriage return also
  
  if(indexForRemove == -1){ return -1;} // If still -1 then there was an error
  message.remove(indexForRemove);
  
  if(message.length() < 3 || message.length() > 5){ return -1;}   // Message length is incorrect, return -1 for error

  if(message.equals("s=0")){
    return 0;
  }
  else if(message.equals("s=255")){
    return 255;
  }
  else if(message.equals("s=128")){
    return 128;
  }
  else if(message.equals("s=1")){
    return 1;
  }
  else if(message.equals("s=2")){
    return 2;
  }
  else if(message.equals("RUN")){
    return 175; // Odd ball, will rehome and reset if it recieves. This to allow not having to shutoff the device to reset.
  }
  else if(message.equals("h=1")){
    return 73;
  }
  else if(message.equals("f=2")){
    return 55;
  }
  else{
    return -1;
  }
}

byte initializeShifter(){
  
  byte brakePressed = false;
  static byte alreadyDisplayed = false;
  
  // This looks like the because of the way serialEvent works.
  if(alreadyRecievedRUN){
    gotRun = true;
    alreadyRecievedRUN = false;
  }
  else if(stringComplete && !gotRun){
    if(parseReceivedMessage(recievedMessage) == 175){
      gotRun = true;
      lcd.clear(); 
    }
    recievedMessage.remove(0);
    stringComplete = false;
  }

  if(gotRun){
    if(stringComplete && !alreadyRecieved){
      if(parseReceivedMessage(recievedMessage) == 73){
        alreadyRecieved = true; 
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Waiting Brake"); 
       }
       recievedMessage.remove(0);
       stringComplete = false;
    }
    else if(!alreadyRecieved && !alreadyDisplayed){
      lcd.setCursor(0, 0);
      lcd.print("Waiting h=1");
      alreadyDisplayed = true;
    }

    brakePressed = getBrake();  

    if(brakePressed && alreadyRecieved){
      lcd.clear();
      
      Serial.print("HOMING\r");
    
      // Simulate homing with potentiometer
      delay(2000);
    
      Serial.print("HOMING_COMPLETE\r");
    
      delay(200);
      
      Serial.print("READY\r");

      alreadyRecieved = false;
      gotRun = false;
      alreadyDisplayed = false;
      return true;
    }
    else{
      return false;
    }
  }
  else{
    lcd.setCursor(0, 0);
    lcd.print("Waiting RUN");
    return false;
  }
}

//======================================================================================
//=================================Serial Event Function================================
//======================================================================================

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    recievedMessage += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n' or inChar == '\r') {
      stringComplete = true;
    }
  }
}

