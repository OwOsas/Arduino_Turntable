// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to


//motor
#include <Adafruit_MotorShield.h>
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);

//Digital Pins
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; //LCD
int SW = 6; //Joystick

//Analog Pins
int VRx = A0;//Joystick
int VRy = A1;//Joystick

String menuList[] = { 
  "Item 1", 
  "Item 2", 
  "Item 3", 
  "Item 4", 
  "Item 5", 
  "Item 6", 
  "Item 7", 
  "Item 8"
};

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

int menuLength = 8;

//Value Setup
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int xPosition = 0;
int yPosition = 0;
int SW_state = 0;
int mapX = 0;
int mapY = 0;

int menuPos = 0;
int cursorPos = 0;
bool inMenu;
bool inCinematic;
bool inScan;
bool inControl;

void displayTwoLine(LiquidCrystal lcd, String line_one, String line_two){
  lcd.setCursor(1, 0);
  lcd.print(line_one);
  //Serial.println(line_one);
  lcd.setCursor(1, 1);
  lcd.print(line_two);
  //Serial.println(line_two);
}

void displayCursor(LiquidCrystal lcd, int cursorPos, int length){
  if(cursorPos == 0){
    lcd.setCursor(0, 0);
    lcd.print(">");
    lcd.setCursor(15, 1);
    lcd.write(byte(1));
  }
  else if(cursorPos == length-1){
    lcd.setCursor(0, 1);
    lcd.print(">");
    lcd.setCursor(15, 0);
    lcd.write(byte(0));
  }
  else{
    lcd.setCursor(0, 0);
    lcd.print(">");
    lcd.setCursor(15, 0);
    lcd.write(byte(0));
    lcd.setCursor(15, 1);
    lcd.write(byte(1));
  }

}

void joyStickMenuControl(){
  
}

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  //joyStick setup
  pinMode(VRx, INPUT);
  pinMode(VRy, INPUT);
  pinMode(SW, INPUT_PULLUP); 

  displayTwoLine(lcd, menuList[menuPos], menuList[menuPos+1]);
  displayCursor(lcd, cursorPos, menuLength);

  inMenu = true;

  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);
}

void loop() {
  Serial.print(menuPos);
  Serial.print(" | ");
  Serial.println(cursorPos);

  xPosition = analogRead(VRx);
  yPosition = analogRead(VRy);
  SW_state = digitalRead(SW);
  mapX = map(xPosition, 0, 1023, -512, 512);
  mapY = map(yPosition, 0, 1023, -512, 512);
  
//  Serial.print("X: ");
//  Serial.print(mapX);
//  Serial.print(" | Y: ");
//  Serial.print(mapY);
//  Serial.print(" | Button: ");
//  Serial.println(SW_state);
  if(inMenu){
    if(mapY > 100 && cursorPos+1 == menuLength-1){
      cursorPos++;
      lcd.clear();
      displayTwoLine(lcd, menuList[menuPos], menuList[menuPos+1]);
      displayCursor(lcd, cursorPos, menuLength);
    }
    else if(mapY > 100 && cursorPos+1 < menuLength-1){
      menuPos++;
      cursorPos++;
      lcd.clear();
      displayTwoLine(lcd, menuList[menuPos], menuList[menuPos+1]);
      displayCursor(lcd, cursorPos, menuLength);
    }
    else if(mapY < -100 && cursorPos+1 == menuLength){
      cursorPos--;
      lcd.clear();
      displayTwoLine(lcd, menuList[menuPos], menuList[menuPos+1]);
      displayCursor(lcd, cursorPos, menuLength);
    }
    else if(mapY < -100 && cursorPos > 0){
      cursorPos--;
      menuPos--;
      lcd.clear();
      displayTwoLine(lcd, menuList[menuPos], menuList[menuPos+1]);
      displayCursor(lcd, cursorPos, menuLength);
    }

  }



  if(SW_state == 0){
    displayTwoLine(lcd, menuList[menuPos], menuList[menuPos+1]);
  }





  
  delay(200);

  

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):

  
}