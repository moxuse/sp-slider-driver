#include <LiquidCrystal.h>
#include <math.h>
#define STEP_REQUEST_BYTE 'x'

int pulsePin = 10;
int directPin = 9;
int currentPin = 13;
int limitNearPin = 7;
int limitFarPin = 6;
int pulseCountPin = 8;
char buffer[20];

bool limitNear = false;
bool limitFar = false;

bool busy = false;

unsigned int currentPos = 0;
unsigned long count = 0;
long currentStep = 0;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

static FILE lcdout = {0};// LCD FILE structure

void setup() {
  pinMode(pulsePin, OUTPUT);
  pinMode(directPin, OUTPUT);
  pinMode(currentPin, OUTPUT);
  pinMode(limitNearPin, INPUT);
  pinMode(limitFarPin, INPUT);
  pinMode(pulseCountPin, INPUT);
  
  lcd.begin(16, 2);
  fdev_setup_stream (&lcdout, lcd_putchar, NULL, _FDEV_SETUP_WRITE);
  lcd.print("sp-slider motor diver");
   
  Serial.begin( 9600 );
  
  digitalWrite(currentPin, LOW);
}

void loop() {
  unsigned char checkByte;
  unsigned char secondCheckByte;
  unsigned char commandByte;
  unsigned char speedByte;
  unsigned int targetPos;
  if (Serial.available() > 6 && !busy) {
    recvBuffer(buffer);
    checkByte = buffer[0]; //////////// 1
     if (STEP_REQUEST_BYTE == checkByte){
       secondCheckByte = buffer[1]; //////////// 2
       if (STEP_REQUEST_BYTE == secondCheckByte) {
         
         count = 0;
         commandByte = buffer[2]; //////////// 3
         speedByte = buffer[3]; //////////// 4
         unsigned char highByte = buffer[4]; //////////// 5
         unsigned char lowByte = buffer[5]; //////////// 6

         unsigned int speedDelay = (unsigned int)(speedByte);
         speedDelay = (unsigned int)map(speedDelay, 0, 255, 0, 2000);
         if (100 > speedDelay) {
           speedDelay = 100;
         };

         targetPos = (highByte * 256) + lowByte;
         unsigned long distCount = (unsigned long)abs((long)(targetPos) - currentPos) * 100;
         count = 0;
         
         if (targetPos > currentPos) {
           digitalWrite(currentPin, HIGH);
           digitalWrite(directPin, LOW);
           while (count < distCount) {
             busy = true;
             int state = pulse(speedDelay, 0);
             if (0 < state) {
                 break;
             };
             if (0 == currentStep%100) {
               currentPos++;
             }
             count++;
           }
         } else {
           digitalWrite(currentPin, HIGH);
           digitalWrite(directPin, HIGH);
           while (count < distCount) {
             busy = true;
             int state = pulse(speedDelay, 1);
             if (0 < state) {
                 break;
             };
             if (0 == currentStep%100) {
               currentPos--;
             }
             count++;
           }
         }
         if ('r' == commandByte) { //reset function !!!!!!!!!!!
           busy = true;
           doReset();
         }
         if (limitNear) {
           writeLimit(1);
         };
         if (limitFar) {
           writeLimit(2);
         };
         digitalWrite(currentPin, LOW);
         delay(100);
         
         lcd.clear();
         lcd.setCursor(0, 0);
         lcd.write("current pos");
         lcd.setCursor(0, 1);
         lcd.print(currentPos, DEC) ;
         
         busy = false;
         if (!limitNear && !limitNear) {
           writeCurrentStepToSerial(currentStep);
         };
         if (limitNear) { limitNear = false; };
         if (limitFar) { limitFar = false; };
       }
     }
  }
  delay(10);
}

void recvBuffer(char *buf) {
  int i = 0;
  char c;
  while (1) {
    if (Serial.available()) {
      c = Serial.read();
      buf[i] = c;
      if (c == '\n') break; // 文字列の終わりは\0で判断
      i++;
    }
  }
  buf[i] = '\n';
}

int pulse(int delayMs, int dir) {
  int isPulse;
  if (1 == digitalRead(limitNearPin) && 1 == digitalRead(limitFarPin)) {
    digitalWrite(pulsePin, HIGH);
    delayMicroseconds(delayMs);
    digitalWrite(pulsePin, LOW);
    delayMicroseconds(delayMs);
    isPulse = digitalRead(pulseCountPin);
    if (1 == isPulse) {

      if (0 == dir) {
        currentStep++;
      } else if (1 == dir) {
        currentStep--;
      }
    }
    return 0;
  } else if (0 == digitalRead(limitFarPin)) {
    limitFar = true;
    return 2;
  } else if (0 == digitalRead(limitNearPin)){
    limitNear = true;
    return 1;
  }
  return 0;
}

void writeCurrentStepToSerial(long pos) {
  unsigned int sendPos  = (unsigned int)(pos / 100);
  unsigned char h_dist = (unsigned char)(sendPos >> 8);
  unsigned char l_dist = (unsigned char)(sendPos % 256);
  Serial.write('X');
  Serial.write('X');
  Serial.write('N');
  Serial.write(h_dist);
  Serial.write(l_dist);
  Serial.write('\n');
}

void writeLimit(int state) {
  String stateStr;
  switch (state) {
    case 1:
      stateStr = "LIMIT NEAR...";
      writeCurrentStepToSerial(0);
      currentStep = 0;
      currentPos = 0;
      break;
    case 2:
      stateStr = "LIMIT FAR...";
      writeCurrentStepToSerial(currentStep);
      break;
    default:
      break;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(stateStr);
  lcd.setCursor(0, 1);
  lcd.print("") ;
}

void doReset() {
   while (1) {
     int state = pulse(400, 0);
     if (1 == state) {
        break;
     };
   };
   digitalWrite(currentPin, HIGH);
   digitalWrite(directPin, LOW);
   int idleCount = 0;
   for (idleCount = 0 ; idleCount < 1000; idleCount++) {
     digitalWrite(pulsePin, HIGH);
     delayMicroseconds(400);
     digitalWrite(pulsePin, LOW);
     delayMicroseconds(400);
   };
   currentPos = 0;
   currentStep = 0;
   writeCurrentStepToSerial(currentStep);
}

// LCD character writer
static int lcd_putchar(char ch, FILE* stream) {
  lcd.write(ch);
  return (0);
}

/*
白 GND
黄 PULSE +
青 GND
赤 DIrection +
緑 GND
橙 Current +
黒 GND
黒 GND
黒 TIM -
橙 TIM +
*/

