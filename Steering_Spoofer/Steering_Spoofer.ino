/*  Written by: Patrick Neal
 *  Email: neap@ufl.edu
 *  Last Updated: 2/21/18
 *  
 *  Used to test new Apollo interface code for the Urban NaviGator,
 *  specifically spoofing the steering SmartMotor
 *  
 * Notes:
 *  
 *           
 */

// ------Initializing Variables--------
byte initialized = false;         // Has the "Smart Motor" been initialized?
byte gotRun = false;              // Run was recieved during the initialization    
byte alreadyRecievedRUN = false;  // "RUN" has already been recieved, while doing main loop. Used to skip searching for "RUN" in the initialization step.

// ------Serial Variables------
boolean stringComplete = false;  // whether the string is complete
String recievedMessage;

void setup() {
  
  //Initialize the serial
  Serial.begin(38400);
}


void loop() {
  
  int temp;
  
  // put your main code here, to run repeatedly:
  if(!initialized){
    initialized = initializeSteerer();  //Perform the initialization steps the tablet is looking for.
  }
  else{
    if(stringComplete){ 
      temp = parseReceivedMessage(recievedMessage);
    }
    /*if(stringComplete){
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
          default:
            Serial.print("UNKNOWN_COMMAND\r");
            break;
        }
      }*/
      if(stringComplete){
        recievedMessage.remove(0);
        stringComplete = false;
      }
    }
}

byte initializeSteerer(){
  
  // This looks like the because of the way serialEvent works.
  if(alreadyRecievedRUN){
    gotRun = true;
    alreadyRecievedRUN = false;
  }
  else if(stringComplete && !gotRun){
    if(parseReceivedMessage(recievedMessage) == 175){
      gotRun = true;
    }
    recievedMessage.remove(0);
    stringComplete = false;
  }

  if(gotRun){
    
      Serial.print("HOMING\r");
    
      // Delay to represent homing
      delay(random(500,800));
    
      Serial.print("HOMING_COMPLETE\r");
    
      delay(200);
      
      Serial.print("READY\r");

      gotRun = false;
      
      return true;
      
  }
  else{
    return false;
  }
}

//======================================================================================
//=====================+======Serial Message Parser Function============================
//======================================================================================

int parseReceivedMessage(String message){
  // Implemented for communications with the steering SmartMotor
  
  int indexForRemove;
  String numberString;

  indexForRemove = message.indexOf("\n");
  if(indexForRemove == -1){ indexForRemove = message.indexOf("\r"); } //Check for carriage return also
  
  if(indexForRemove == -1){ return -1;} // If still -1 then there was an error
  message.remove(indexForRemove);
  
  if(message.length() < 3 || message.length() > 5){ return -1;}   // Message length is incorrect, return -1 for error

  Serial.println(message);
  
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
  else if(message.equals("c=32")){
    numberString = "";
    numberString.concat(message[2]);
    numberString.concat(message[3]);
    Serial.println(numberString.toFloat());
  }
  else{
    return -1;
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

