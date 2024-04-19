#include <LiquidCrystal.h>

// Initial configuration
#define BUTTON_PIN 2
#define BUTTON_START_STOP 3


// LCD pins
#define rs 12
#define en 11
#define d4 4 
#define d5 5
#define d6 6
#define d7 7

// LCD cursor coordinates
int cursorT0Active = 0;
int cursorT1Active = 9;
int t0TimeS = 1;
int t1TimeS = 10;
int t0TimePoints = 3;
int t1TimeMs = 4;
int t1TimePOints = 12;
int t0TimeMs = 13;
int cursorIncrementS = 2;
int cursorIncrementPoints = 6;
int  cursorIncrementMs = 7;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7); // Init LCD handler

long actualTime_0, actualTime_1, timeReference; // Variables to store each player remaining time counter and last time reading in ms
long *activeTimePointer = &actualTime_0; // Pointer to the active player time pointer, initialized with player 0 counter
int activeIncrement; // Int with the time increment in ms
bool playRoutineFlag = false, previousPlayRoutineFlag; // Boolean flags to control play/plause and transitions
bool changeActiveTimeFlag = false; // Boolean flag to indicate that the player counter must be switched
bool activeTime = false; // boolean to see wich player counter is active: player0 = false , player1 = true


void setupInitialClockConfig(long initialTime_0=3000, long initialTime_1=3000, long increment=3000);

void setup() {
  // put your setup code here, to run once:

  //Initialize Serial Communiction
  Serial.begin(9600);lcd.clear();

  // Initialize lcd
  initLCD();

  // Initialize Clock Buttons and attactch it's interrupts
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(BUTTON_PIN, HIGH);
  pinMode(BUTTON_START_STOP, INPUT_PULLUP);
  digitalWrite(BUTTON_START_STOP, HIGH);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), changeActiveTimeInterrupt, FALLING); // Switch active time interrupt
  attachInterrupt(digitalPinToInterrupt(BUTTON_START_STOP), startStopInterrupt, FALLING); // Play/Pause interrupt

  setupInitialClockConfig(); // Initialize the clock with default config - Variables can be passed here!
}

void loop() {
  
  if (playRoutineFlag) {
    // if we are in play routine
    if (!previousPlayRoutineFlag) {
      // and previously we where not, enter on play routine
      enterPlayRoutine();
    }

   
    subtractTimeDelta(); // if it's not the first play routine cycle, subtract the elapsed time from time active time counter
    checkForLoss(); // verify if the game was lost by time for the active time counter

    printStatistics(); // print the cycle status
  } else {
    // if we are not on play routine, hence, paused
    if (previousPlayRoutineFlag) {
      // and whe where before
      Serial.println("Paused...");
      previousPlayRoutineFlag = playRoutineFlag; // And our previous play routine starts to be false
    }
  }

  // independetly of the active play routine, if changeActiveTime Flag is on, call the changeTimeRoutine
  if (changeActiveTimeFlag) {
      changeActiveTime();
    }
}


void initLCD() {
  // Simple initial print function
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("best chess clock");
  delay(1500);
  //lcd.clear();
}

void enterPlayRoutine (){
  // Enter the play routine, starting the time reference and putting the previous play routine as the actual
  timeReference = millis();
  previousPlayRoutineFlag = playRoutineFlag;
}

void setupInitialClockConfig (long initialTime_0=3000, long initialTime_1=3000, long increment=3000){
  // Starts both players time counters and increments with the default or provided times
  actualTime_0 = initialTime_0;
  actualTime_1 = initialTime_1;
  activeIncrement = increment;

  // Print initial statistices
  printStatistics();
  return;
}

void checkForLoss() {
  // Verify if the active time counter have runned out of time
  if (*activeTimePointer <= 0) {
    lcd.clear();
    // iF active time pointer value is equal or minor than zero, verify wich player lost and prints it
    if (activeTime) {
      Serial.println("Player 1 is a LOOOOOOOSER!");
      lcd.print("player 1 loser");
    } else {
      Serial.println("Player 0 is a LOOOOOOOSER!");
      lcd.print("player 2 looser");
    }
    // Put the time as 0 to not display a negative value and stops the game
    *activeTimePointer = 0;
    playRoutineFlag = false;
  }
}

void subtractTimeDelta() {
  // Calculate the time between now and the last measure
  long newMeasure = millis();
  long timeDelta = newMeasure - timeReference;

  //Set the new time reading reference as now
  timeReference = newMeasure;

  // And subtracts the time delta from the active counter
  *activeTimePointer -= timeDelta;
}

void changeActiveTime() {
  // Takes the active time pointer, add the increment to its time and change the active pointer and it's flags

  if (playRoutineFlag) {
    // If in play routine, add increment, to avoid incrementing time when paused and not started
    *activeTimePointer += activeIncrement;
  }

  // set the new time counter based on wich counter is active: if 0 is active, set the new as 1 and vice-versa
  if (activeTime) {
    activeTimePointer = &actualTime_0;
  } else {
    activeTimePointer = &actualTime_1;
  }

  // Set the new active time and put the changeTimeFlag to false
  activeTime = !activeTime;
  changeActiveTimeFlag = false;
}

void printStatistics(){
  // Prints statistics in a single line on Serial Monitor
  float actualTime_0_seconds = actualTime_0 / 1000.0; // Convert to seconds
  float actualTime_1_seconds = actualTime_1 / 1000.0; // Convert to seconds
  lcd.setCursor(0, 1);
  Serial.print("Time 0: "); Serial.print(actualTime_0_seconds, 2); Serial.print("s ");
  lcd.print(actualTime_0_seconds);
  //lcd.clear();
  Serial.print("| Time 1: "); Serial.print(actualTime_1_seconds, 2); Serial.print("s |");
  lcd.print(" | "); lcd.print(actualTime_1_seconds);
  //lcd.clear();
  Serial.print("Increment: "); Serial.print(activeIncrement / 1000.0, 2); Serial.println("s");
}

void startStopInterrupt() {
  // Switch the value of the play routine
  playRoutineFlag = !playRoutineFlag;
}

void changeActiveTimeInterrupt() {
  // Indicates tha the active time counter must be switched
  changeActiveTimeFlag = true;
}
