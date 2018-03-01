/*  Written by: Patrick Neal
 *  Email: neap@ufl.edu
 *  Last Updated: 3/1/18
 *  
 *  Used to test new Apollo interface code for the Urban NaviGator,
 *  specifically spoofing the steering SmartMotor
 *  
 * Notes:
 *  
 *           
 */
 
#include <LiquidCrystal.h>

// ------Initializing Variables--------
byte initialized = false;         // Has the "Smart Motor" been initialized?
byte gotRun = false;              // Run was recieved during the initialization    
byte alreadyRecievedRUN = false;  // "RUN" has already been recieved, while doing main loop. Used to skip searching for "RUN" in the initialization step.

// ------Serial Variables------
boolean stringComplete = false;  // whether the string is complete
String recievedMessage;

// ------Steering Variables------
int currentSteeringPosition = 0, desiredSteeringPosition = 0;
int rotationDirection = 0;
byte newPositionSet = 0;
byte actuating = 0;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup() {
  //Setup the LCD display
  lcd.begin(16,2);  // Specify the size of the display (16 columns, 2 rows)
  
  //Initialize the serial
  Serial.begin(38400);
}


void loop() {
  
  int temp;
  unsigned long currentTime = 0, lastTime = 0;

  
  // put your main code here, to run repeatedly:
  if(!initialized){
    initialized = initializeSteerer();  //Perform the initialization steps the tablet is looking for.
  }
  else{
    if(stringComplete){ 
      temp = parseReceivedMessage(recievedMessage);

        switch(temp){
          case 122:
            lcd.clear();
            lcd.print("Move to " + String(desiredSteeringPosition));
            break;
          case 175:
            initialized = false;
            alreadyRecievedRUN = true;
            break;
          case 55:
            lcd.setCursor(0, 0);
            lcd.print("NORMAL_SHUTDOWN");
            Serial.print("NORMAL_SHUTDOWN\r");
            initialized = false;
            break;
          default:
            Serial.print("UNKNOWN_COMMAND\r");
            break;
        }
        
        recievedMessage.remove(0);
        stringComplete = false;
      }

    if(newPositionSet){
      rotationDirection = sign(currentSteeringPosition-desiredSteeringPosition);
      actuating = true;
      newPositionSet = false;
    }

    if(actuating){
      currentTime = micros();
      
      if(currentTime >= lastTime + 150000){
        
        currentSteeringPosition = currentSteeringPosition - rotationDirection*2;
        
        int error = currentSteeringPosition-desiredSteeringPosition;
        
        // Remove any overshoot
        if(rotationDirection < 0 && error > 0){
          // Motor was traveling Clock Wise
          currentSteeringPosition = desiredSteeringPosition;
          actuating = false;
        }
        else if(rotationDirection > 0 && error < 0){
          // Motor was traveling Counter Clock Wise
          currentSteeringPosition = desiredSteeringPosition;
          actuating = false;
        }
        else if(error == 0){
          actuating = false;
        }
        lcd.setCursor(0,1);
        lcd.print("c=" + String(currentSteeringPosition));
        Serial.print("c=" + String(currentSteeringPosition) +"\r");
        lastTime = currentTime;
      }
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
    lcd.clear();
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
    lcd.clear()
    lcd.print("Waiting RUN");
    return false;
  }
}

//======================================================================================
//============================Serial Message Parser Function============================
//======================================================================================

int parseReceivedMessage(String message){
  // Implemented for communications with the steering SmartMotor
  
  int indexForRemove;
  String numberString = "";

  indexForRemove = message.indexOf("\n");
  if(indexForRemove == -1){ indexForRemove = message.indexOf("\r"); } //Check for carriage return also
  
  if(indexForRemove == -1){ return -1;} // If still -1 then there was an error
  message.remove(indexForRemove);
  
  if(message.length() < 3 || message.length() > 7){ return -1;}   // Message length is incorrect, return -1 for error
  
  if(message.startsWith("p")){
    // Grab the characters after "p="
    for (int i = 2; i < message.length() ;i++){
      numberString.concat(message[i]);
    }
    
    desiredSteeringPosition = numberString.toInt();
    newPositionSet = true;
    return 122;
  }
  else if(message.equals("RUN")){
    return 175; // Odd ball, will rehome and reset if it recieves. This to allow not having to shutoff the device to reset.
  }
  else if(message.equals("f=2")){
    return 55;
  }
  else{
    return -2;
  }
}

// Implementation of the sign function. Returns the sign of the value given it it.
int sign(int val){

  if (val < 0) {return -1;}
  else { return 1;}
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

