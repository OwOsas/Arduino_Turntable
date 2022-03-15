// include the library code:
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to


//motor
#include <Adafruit_MotorShield.h>
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);

//Digital Pins
int SW = 6; //Joystick
int returnSW = 5; //Return btn

//Analog Pins
int VRx = A0;//Joystick
int VRy = A1;//Joystick

String mainMenu[] = { 
  "3D Scan", 
  "Item 2", 
  "Item 3", 
  "Item 4", 
  "Item 5", 
  "Item 6", 
  "Item 7", 
  "Item 8"
};
int mainMenuLength = 8;

String scanMenu[] = { 
  "Step/Turn", 
  "Pause Time", 
  "START"
};
int scanMenuLength = 3;

byte upArrow[8] =
{
0b00000,
0b00000,
0b00000,
0b00000,
0b00100,
0b01110,
0b11111,
0b00000
};

byte downArrow[8] =
{
0b00000,
0b11111,
0b01110,
0b00100,
0b00000,
0b00000,
0b00000,
0b00000
};

byte leftArrow[] = {
  B00010,
  B00110,
  B01110,
  B11110,
  B01110,
  B00110,
  B00010,
  B00000
};

byte rightArrow[] = {
  B01000,
  B01100,
  B01110,
  B01111,
  B01110,
  B01100,
  B01000,
  B00000
};

int deadZone = 250;


//Value Setup
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int xPosition = 0;
int yPosition = 0;
int SW_state = 0;
int prev_SW_state = 1;
int SW_trigger = false;
int reSW_state = 0;
int prev_reSW_state = 1;
int reSW_trigger = false;
int mapX = 0;
int mapY = 0;

int menuPos;
int cursorPos;
bool inMenu;
bool inCinematic;
bool inScan;
bool inControl;
int stepPerTurn = 5;
int numberOfRotation = 1;
int pauseTime = 1000;
String currentMenu = "main";


void displayTwoLine(LiquidCrystal_I2C lcd, String line_one, String line_two){
  lcd.setCursor(1, 0);
  lcd.print(line_one);
  //Serial.println(line_one);
  lcd.setCursor(1, 1);
  lcd.print(line_two);
  //Serial.println(line_two);
}

void displayCursor(LiquidCrystal_I2C lcd, int cursorPos, int menuLength){
  if(cursorPos == 0){
    lcd.setCursor(0, 0);
    lcd.print(">");
    lcd.setCursor(15, 1);
    lcd.write(1);
  }
  else if(cursorPos == menuLength-1){
    lcd.setCursor(0, 1);
    lcd.print(">");
    lcd.setCursor(15, 0);
    lcd.write(0);
  }
  else{
    lcd.setCursor(0, 0);
    lcd.print(">");
    lcd.setCursor(15, 0);
    lcd.write(0);
    lcd.setCursor(15, 1);
    lcd.write(1);
  }
}

void dispalyValueControl(LiquidCrystal_I2C lcd, String name, int currentVal, String unit = ""){
  lcd.setCursor(0,0);
  lcd.print(name);
  lcd.setCursor(0,1);
  lcd.write(3);
  lcd.setCursor(15-unit.length(),1);
  lcd.print(unit);
  lcd.setCursor(1,1);
  lcd.print(currentVal);
  lcd.setCursor(15,1);
  lcd.write(4);
}

void joyStickMenuControl(LiquidCrystal_I2C lcd, int menuLength, String menu[]){
    if(mapY > deadZone && cursorPos+1 == menuLength-1){
      cursorPos++;
      lcd.clear();
      displayTwoLine(lcd, menu[menuPos], menu[menuPos+1]);
      displayCursor(lcd, cursorPos, menuLength);
      delay(200);
    }
    else if(mapY > deadZone && cursorPos+1 < menuLength-1){
      menuPos++;
      cursorPos++;
      lcd.clear();
      displayTwoLine(lcd, menu[menuPos], menu[menuPos+1]);
      displayCursor(lcd, cursorPos, menuLength);
      delay(200);
    }
    else if(mapY < -deadZone && cursorPos+1 == menuLength){
      cursorPos--;
      lcd.clear();
      displayTwoLine(lcd, menu[menuPos], menu[menuPos+1]);
      displayCursor(lcd, cursorPos, menuLength);
      delay(200);
    }
    else if(mapY < -deadZone && cursorPos > 0){
      cursorPos--;
      menuPos--;
      lcd.clear();
      displayTwoLine(lcd, menu[menuPos], menu[menuPos+1]);
      displayCursor(lcd, cursorPos, menuLength);
      delay(200);
    }
}

int joyStickValueControl(LiquidCrystal_I2C lcd, String name, int currentVal, int lowBound, int upBound, int step, String unit = ""){
  if(currentVal <= upBound && currentVal >= lowBound){
    if(mapX > deadZone && currentVal < upBound){
      currentVal++;
      lcd.clear();
      dispalyValueControl(lcd, name, currentVal, unit);
      delay(50);
    }
    else if(mapX < -deadZone && currentVal > lowBound){
      currentVal--;
      lcd.clear();
      dispalyValueControl(lcd, name, currentVal, unit);
      delay(50);
    }
    else if(mapY > deadZone){
      if(currentVal+step < upBound){
        currentVal += step;
        lcd.clear();
        dispalyValueControl(lcd, name, currentVal, unit);
        delay(50);
      }
      else{
        currentVal = upBound;
        lcd.clear();
        dispalyValueControl(lcd, name, currentVal, unit);
        delay(50);
      }
    }
    else if(mapY < -deadZone){
      if(currentVal-step > lowBound){
        currentVal -= step;
        lcd.clear();
        dispalyValueControl(lcd, name, currentVal, unit);
        delay(50);
      }
      else{
        currentVal = lowBound;
        lcd.clear();
        dispalyValueControl(lcd, name, currentVal, unit);
        delay(50);
      }
    }
  }
  return currentVal;
}

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  
  // set up the LCD's number of columns and rows:
  //joyStick setup
  pinMode(VRx, INPUT);
  pinMode(VRy, INPUT);
  pinMode(SW, INPUT_PULLUP); 
  pinMode(returnSW, INPUT_PULLUP); 

  displayTwoLine(lcd, mainMenu[menuPos], mainMenu[menuPos+1]);
  displayCursor(lcd, cursorPos, mainMenuLength);

  menuPos = 0;
  cursorPos = 0;

  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);
  lcd.createChar(3, leftArrow);
  lcd.createChar(4, rightArrow);

  AFMS.begin();
  myMotor->setSpeed(10000);  // 10 rpm

  myMotor->release();
}

void loop() {
  xPosition = analogRead(VRx);
  yPosition = analogRead(VRy);
  SW_state = digitalRead(SW);
  reSW_state = digitalRead(returnSW);
  mapX = -map(xPosition, 0, 1023, -512, 512);
  mapY = -map(yPosition, 0, 1023, -512, 512);


  Serial.print(reSW_state);
  Serial.print(" | ");
  Serial.print(SW_state);
  Serial.print(" | ");
  Serial.println(prev_SW_state);

  if(SW_state == 1 && prev_SW_state == 0 && SW_trigger == false){
    SW_trigger = true;
  }
  else{
    SW_trigger = false;
  }

  if(reSW_state == 1 && prev_reSW_state == 0 && reSW_trigger == false){
    reSW_trigger = true;
  }
  else{
    reSW_trigger = false;
  }

  if(currentMenu == "main"){
    joyStickMenuControl(lcd, mainMenuLength, mainMenu);
    if(SW_trigger){
      if(mainMenu[cursorPos] == "3D Scan"){
        cursorPos = 0;
        menuPos = 0;
        currentMenu = "scan";
        lcd.clear();
        displayTwoLine(lcd, scanMenu[menuPos], scanMenu[menuPos+1]);
        displayCursor(lcd, cursorPos, scanMenuLength);
      }
    }
  }
  else if(currentMenu == "scan"){
    joyStickMenuControl(lcd, scanMenuLength, scanMenu);
    Serial.println("SCANNNNNNN");
    
    if(SW_trigger){
      if(scanMenu[cursorPos] == "Step/Turn"){
        currentMenu = "stepPerTurn";
        lcd.clear();
        dispalyValueControl(lcd, "Step Per Turn", stepPerTurn, "steps");
      }
      else if(scanMenu[cursorPos] == "Pause Time"){
        currentMenu = "pauseTime";
        lcd.clear();
        dispalyValueControl(lcd, "Pause Time", pauseTime, "ms");
      }
      else if(scanMenu[cursorPos] == "START"){
        currentMenu = "START";
      }
    }
    if(reSW_trigger){
        Serial.println("RETRUNNNN");
        cursorPos = 0;
        menuPos = 0;
        currentMenu = "main";
        lcd.clear();
        displayTwoLine(lcd, mainMenu[menuPos], mainMenu[menuPos+1]);
        displayCursor(lcd, cursorPos, mainMenuLength);
    }
  }
  else if(currentMenu == "stepPerTurn"){
    stepPerTurn = joyStickValueControl(lcd, "Step Per Turn", stepPerTurn, 0, 200, 10, "step");
    if(reSW_trigger or SW_trigger){
      currentMenu = "scan";
      lcd.clear();
      displayTwoLine(lcd, scanMenu[menuPos], scanMenu[menuPos+1]);
      displayCursor(lcd, cursorPos, scanMenuLength);
      delay(100);
    }
  }
  else if(currentMenu == "pauseTime"){
    pauseTime = joyStickValueControl(lcd, "Pause Time", pauseTime, 0, 10000, 100, "ms");

    if(reSW_trigger or SW_trigger){
      currentMenu = "scan";
      lcd.clear();
      displayTwoLine(lcd, scanMenu[menuPos], scanMenu[menuPos+1]);
      displayCursor(lcd, cursorPos, scanMenuLength);
      delay(100);
    }

  }
  else if(currentMenu == "START"){
    int stepPerRotate = 200/stepPerTurn;
    int turnsLeft= stepPerTurn;
    float inProgress = true;
    while (turnsLeft > 0 && inProgress){
      displayTwoLine(lcd, "Job Progress", String(turnsLeft));
      delay(pauseTime);
      myMotor->step(stepPerRotate, FORWARD, DOUBLE);
      turnsLeft -= 1;
      lcd.clear();

      reSW_state = digitalRead(returnSW);

      if(reSW_state == 1 && prev_reSW_state == 0 && reSW_trigger == false){
        reSW_trigger = true;
      }
      else{
        reSW_trigger = false;
      }
      
      if (reSW_trigger)
      {
        inProgress = false;
      }
    }
    currentMenu = "scan";
    lcd.clear();
    displayTwoLine(lcd, scanMenu[menuPos], scanMenu[menuPos+1]);
    displayCursor(lcd, cursorPos, scanMenuLength);
    delay(100);
    myMotor->release();
  }


  prev_SW_state = SW_state;
  prev_reSW_state = reSW_state;

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):

  delay(200);
}