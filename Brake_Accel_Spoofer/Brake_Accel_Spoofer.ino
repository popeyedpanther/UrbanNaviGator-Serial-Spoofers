/*  Written by: Patrick Neal
 *  Email: neap@ufl.edu
 *  Last Updated: 2/21/18
 *  
 *  Used to test new interface code for the Urban NaviGator,
 *  specifically spoofing the drive by wire board (myRIO)
 *  
 * Notes:
 *  Button Scheme:
 * 
 *              (A)
 *    (E)  (R)       (B) 
 *              (I) 
 *            
 *  E = E-Stop            
 *  B = Brake Switch
 *  A = Auto Switch
 *  I = Ignition Switch
 *  U = not implemented
 *  
 *  All buttons are handled through toggling. Press button to toggle between on and off. The LCD should display the current state.
 *           
 */

#include <LiquidCrystal.h>

#define MANUAL_STATE      1
#define AUTONOMOUS_STATE  2
#define ESTOPPED_STATE    3
#define INACTIVE_STATE    4

#define USE_OLD_MESSAGING

// ---Serial Communication Parameters---
#define BAUDRATE        115200
#ifdef USE_OLD_MESSAGING
  #define TX_PACKET_SIZE  43
  #define RX_PACKET_SIZE  20 //Message is oversized to allow more information to have been sent. This is legacy.
#else
  #define TX_PACKET_SIZE  7
  #define RX_PACKET_SIZE  9
  #define CRC_DIVIDER     256
#endif


// ---Loop Timers----
// The timing is approximately microseconds
#define STATE_TIMER 1000    // Time in microseconds between each call of the state loop (~1000Hz)
#define SEND_TIMER 100000 // Time in microseconds between each call of sendMessage (~10Hz)

//======================================================================================
//===============================Global Variables=======================================
//======================================================================================
int State = INACTIVE_STATE;           // start in the inactive State, ignition is off
int previousState = INACTIVE_STATE;   // stores the previous State
int desiredState = INACTIVE_STATE;    // stores the desired state based on state transition logic

bool pauseInEffect = false;               // whether or not we are finished with a pause ramp down.
bool receiveMessages = true;          // Value used to set the recieveMessage loop, set to false to stop the recieveMessage thread.

// ---Received From Tablet---
byte autoRequested = 0;                  // 0 if no auto requested from tablet, 1 if auto requested from tablet
byte pauseRequested = 0;                 // 0 if no pause requested from tablet, 1 if pause requested from tablet
int brakePercentEffortTablet  = 0;        // variable holds the current braking effort commanded by the tablet
int throttlePercentEffortTablet   = 0;    // variable holds the current throttle effort commanded by the tablet
byte leftBlinker = 0;
byte rightBlinker = 0;

// ---Percent Effort---
int brakePercentEffortDriver  = 0;      // variable holds the current braking effort determined from the sensors
int throttlePercentEffortDriver = 0;    // variable holds the current throttle effort determined from the sensors
int brakePercentEffortTotal   = 0;      // brake effort used to set the DACs in auto.

double brakePercentEffort     = 0;    // these are the global % effort variables used to smooth transitions between states
double throttlePercentEffort  = 0;    // ""

// ------Serial Variables------
byte messageStarted = false;
byte messageComplete = false;  // whether the string is complete
byte receivedMessage[RX_PACKET_SIZE];

// ------Booleans------
byte estopPressed = false;
byte brakePressed = false;
byte ignitionSwitch = false;
byte autoSwitch = false;

byte transitionToDriverEffort = false;
byte transitionToDriverEffortComplete = false;

char  buttonNames[5] = {'E', 'B', 'A', 'I', ' '};
// Index:    0         1            2                3            4
//        E-stop  Brake Switch  Auto Switch   Ignition Switch   Unset
byte buttonPressed[5] = {0};

String stateStrings[4] = {"Manual", "Auto", "Estopped", "Inactive"};
byte displayDataChanged = true;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//======================================================================================
//========================================Setup=========================================
//======================================================================================
void setup() 
{
  //Setup the LCD display
  lcd.begin(16,2);  // Specify the size of the display (16 columns, 2 rows)
  
  //Initialize the serial
  Serial.begin(115200);
}

//======================================================================================
//=========================================Loop=========================================
//======================================================================================
void loop() 
{
  static unsigned long lastStateTime = 0, lastSendTime = 0;
  unsigned long currentTime = 0;

  updateButtons();

  if(messageComplete){ parseReceivedMessage(receivedMessage); }

  currentTime = micros();
  if(currentTime >= (lastStateTime + STATE_TIMER))
  {
    stateLoop();
    lastStateTime = currentTime;
  }

  if(currentTime >= (lastSendTime + SEND_TIMER))
  {
    
    sendMessage();
    lastSendTime = currentTime;
  }

  if(displayDataChanged){ updateLCD(); }
}

