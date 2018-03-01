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

#define MANUAL_STATE      1
#define AUTONOMOUS_STATE  2
#define PAUSED_STATE      3
#define ESTOPPED_STATE    4
#define INACTIVE_STATE    5

// ---Serial Communication Parameters---
#define BAUDRATE        115200
#define TX_PACKET_SIZE  43
#define HEADER_SIZE     4
#define RX_PACKET_SIZE  20
#define CRC_DIVIDER     255

// ---Loop Timers----
// The timing is approximately microseconds
#define STATE_TIMER 1000    // Time in microseconds between each call of the state loop (~1000Hz)
#define SEND_TIMER 100000 // Time in microseconds between each call of sendMessage (~10Hz)

//======================================================================================
//===============================Global Variables=======================================
//======================================================================================
int State = INACTIVE_STATE;       // start in the inactive State, ignition is off
int previousState = INACTIVE_STATE;   // stores the previous State

bool pauseInEffect = false;               // whether or not we are finished with a pause ramp down.
bool receiveMessages = true;          // Value used to set the recieveMessage loop, set to false to stop the recieveMessage thread.

// ---Received From Tablet---
int autoRequested = 0;                  // 0 if no auto requested from tablet, 1 if auto requested from tablet
int pauseRequested = 0;                 // 0 if no pause requested from tablet, 1 if pause requested from tablet
int brakePercentEffortTablet  = 0;        // variable holds the current braking effort commanded by the tablet
int throttlePercentEffortTablet   = 0;    // variable holds the current throttle effort commanded by the tablet
// leftBlinker is set from Tablet
// rightBliner is set from Tablet

// ---Percent Effort---
int brakePercentEffortDriver  = 0;      // variable holds the current braking effort determined from the sensors
int throttlePercentEffortDriver = 0;    // variable holds the current throttle effort determined from the sensors
int brakePercentEffortTotal   = 0;      // brake effort used to set the DACs in auto.

double brakePercentEffort     = 0;    // these are the global % effort variables used to smooth transitions between states
double throttlePercentEffort  = 0;    // ""

// ------Serial Variables------
boolean stringComplete = false;  // whether the string is complete
String recievedMessage;

char  buttonNames[5] = {'E', 'B', 'A', 'I', ' '};
// Index:    0         1            2                3            4
//        E-stop  Brake Switch  Auto Switch   Ignition Switch   Unset
byte buttonPressed[5] = {0};

String stateStrings[5] = {"Manual", "Auto", "Paused", "Estopped", "Inactive"};
byte displayDataChanged = false;

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

  if(currentTime >= lastStateTime + STATE_TIMER)
  {
    //State();
    lastStateTime = currentTime;
  }

  if(currentTime >= lastSendTime + SEND_TIMER)
  {
    
    //sendMessage();
    lastSendTime = currentTime;
  }

  if(displayDataChanged){ updateLCD(); }
}

