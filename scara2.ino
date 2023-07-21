#include <stdlib.h>
#include <string.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,4);

int sleep_ms = 1000;
int max_speed = 100;
const int stepPinS = 8;
const int dirPinS = 9;
const int enPinS = 10;

const int stepPinL = 5;
const int dirPinL = 6;
const int enPinL = 7;

const int stepPinZ = 2;
const int dirPinZ = 3;
const int enPinZ = 4;

double stopnieL=90;
double stopnieS=180;

char *eptr;
bool startedSending=false;
int textPos=3;
String text="Ready to work!";

int allCommands=0;
int processedCommands=0;

byte zero[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
byte one[] = {
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000
};
byte two[] = {
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000
};
byte three[] = {
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100
};
byte four[] = {
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110
};
byte five[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

void updateProgressBar(unsigned long count, unsigned long totalCount, int lineToPrintOn){
   int percent=count*100/totalCount;
   int currentCol=0;
   lcd.clear();
   lcd.setCursor(currentCol,1);
   lcd.print("PRZETWORZONO");

   if(count>=totalCount){
      for(int i=0;i<=19;i++){
        lcd.setCursor(i,lineToPrintOn);
        lcd.write(5);
      }
      delay(1000);
      startedSending=false;
    }else{
       while(percent>5&&currentCol<19){
        lcd.setCursor(currentCol,lineToPrintOn);
        lcd.write(5);
        percent-=5;
        ++currentCol;
       }
        if(currentCol<19){
          lcd.setCursor(currentCol,lineToPrintOn);
          lcd.write(round(percent));   
        }
    }
 }


void setup() {
  Serial.begin(9600);

  pinMode(stepPinL, OUTPUT);
  pinMode(dirPinL, OUTPUT);
  pinMode(enPinL, OUTPUT);

  pinMode(stepPinS, OUTPUT);
  pinMode(dirPinS, OUTPUT);
  pinMode(enPinS, OUTPUT);

  pinMode(stepPinZ, OUTPUT);
  pinMode(dirPinZ, OUTPUT);
  pinMode(enPinZ, OUTPUT);

  lcd.begin();
  lcd.createChar(0, zero);
  lcd.createChar(1, one);
  lcd.createChar(2, two);
  lcd.createChar(3, three);
  lcd.createChar(4, four);
  lcd.createChar(5, five);
  lcd.clear();
  lcd.setCursor(7,1);
  lcd.print("ALOHA!");
  delay(1000);
}

long int getLong(String *splitStrings, int i ) {
  int tsize = sizeof(splitStrings[i]) / sizeof(splitStrings[i][0]);
  char tab [tsize];
  strncpy(tab, &splitStrings[i][1], tsize);
  long ret = strtol(tab, &eptr, 0);
  if (splitStrings[i][1] == '-' && ret > 0)
    ret *= -1;
  return ret;
}
void loop() {
  if (Serial.available()) {

    // read the incoming byte:
    String str = Serial.readString(); //.toCharArray(line,50);
    startedSending=true;
    if (str == "START") {
      Serial.print("OK");
     
    } else {

      int position = str.indexOf("START");
      if (position != -1) {
        String partBefore = str.substring(0, position);
        String partAfter = str.substring(position + 5);
      
        str = partBefore + partAfter;
      }
      
      int str_len = str.length() + 1;

      // Prepare the character array (the buffer)
      char line[str_len];

      // Copy it over
      str.toCharArray(line, str_len);

      String splitStrings[5];
      int i, j, cnt;
      j = 0;
      cnt = 0;
      String command = strtok(line, " ");
      while (command != 0)
      {
        splitStrings[cnt++] = command;
        command = strtok(0, " ");
      }
      if(splitStrings[0]=="commands"){
        allCommands=splitStrings[1].toInt();
      }else if(allCommands!=0){

      int motorSpeed = 10;
      int moveS = 0;
      int moveL = 0;
      int moveZ = 0;
      for (int i = cnt - 1; i >= 0; i--) {
        long moved = getLong(splitStrings, i);
        if (splitStrings[i][0] == 'F') {
          motorSpeed = moved;
        } else if (splitStrings[i][0] == 'L') {
          moveL = moved;
        } else if (splitStrings[i][0] == 'S') {
          // delay(30);
          moveS = moved;
        } else if (splitStrings[i][0] == 'Z') {
          moveZ = moved;
        }
      }
      ++processedCommands;
      double alpha=moveL*9/35;
      stopnieL+=alpha;
      stopnieS+=(moveS*9/20+alpha+30/116)*25/116;
      updateProgressBar(processedCommands,allCommands,2);
      }
      Serial.print("OK");
    }
  }else if(!startedSending){
      lcd.clear();
      lcd.setCursor(textPos,2);
      if(textPos>6){
        lcd.print(text.substring(0, 20-textPos));
        lcd.setCursor(0,2);
        lcd.print(text.substring(20-textPos,text.length()));
      }else  
        lcd.print(text);
      --textPos;
      if(textPos==-1)
        textPos=19;
      delay(700);
      processedCommands=0;
  }
  
    delay(100);
}


void moveStepper(int stepL, int stepS, int stepZ) {
  int stepTMS = stepS;
  int stepTML = stepL;
  int stepTMZ = stepZ;

  if (stepS != 0) {
    digitalWrite(enPinS, LOW);
    if (stepS > 0)
      digitalWrite(dirPinS, HIGH);
    else {
      digitalWrite(dirPinS, LOW);
      stepTMS *= -1;
    }
  }
  if (stepL != 0) {
    digitalWrite(enPinL, LOW);
    if (stepL > 0)
      digitalWrite(dirPinL, LOW);
    else {
      digitalWrite(dirPinL, HIGH);
      stepTML *= -1;
    }
  }
  if (stepZ != 0) {
    digitalWrite(enPinZ, LOW);
    if (stepZ > 0)
      digitalWrite(dirPinZ, HIGH);
    else {
      digitalWrite(dirPinZ, LOW);
      stepTMZ *= -1;
    }
  }

  double scaleL = 1;
  double scaleS = 1;
  double scaleZ = 1;

  //biggest amount of steps in line
  double maxScale = 1;

  if (stepTMS != 0)
    maxScale = stepTMS;
  // char minChar='s';
  if (maxScale < stepTML && stepTML != 0) {
    maxScale = stepTML;
    // minChar='l';
  }
  if (maxScale < stepTMZ && stepTMZ != 0) {
    maxScale = stepTMZ;
    // minChar='z';
  }
  // how often need to make step
  if (stepTMS != 0)
    scaleS = maxScale / stepTMS;
  if (stepTML != 0)
    scaleL = maxScale / stepTML;
  if (stepTMZ != 0)
    scaleZ = maxScale / stepTMZ;

  int currentStepZ = 0;
  int currentStepS = 0;
  int currentStepL = 0;

  int liczZ = 0;
  int liczS = 0;
  int liczL = 0;

  int iter = 1;
  bool changeS;
  bool changeL;
  bool changeZ;

  while (currentStepZ < stepTMZ || currentStepS < stepTMS || currentStepL < stepTML) {
    changeS = false;
    changeL = false;
    changeZ = false;
    if (currentStepS < stepTMS && ((int)floor(iter / scaleS)) > liczS)
      changeS = true;
    if (currentStepL < stepTML && ((int)floor(iter / scaleL)) > liczL)
      changeL = true;
    if (currentStepZ < stepTMZ && ((int)floor(iter / scaleZ)) > liczZ)
      changeZ = true;

    if (changeS) {
      digitalWrite(stepPinS, !digitalRead(stepPinS));
      delayMicroseconds(sleep_ms);
      digitalWrite(stepPinS, !digitalRead(stepPinS));
      ++currentStepS;
      ++liczS;
    }
    if (changeL) {
      digitalWrite(stepPinL, !digitalRead(stepPinL));
      delayMicroseconds(sleep_ms);
      digitalWrite(stepPinL, !digitalRead(stepPinL));
      ++currentStepL;
      ++liczL;
    }
    if (changeZ) {
      digitalWrite(stepPinZ, !digitalRead(stepPinZ));
      delayMicroseconds(sleep_ms);
      digitalWrite(stepPinZ, !digitalRead(stepPinZ));
      ++currentStepZ;
      ++liczZ;
    }
    delayMicroseconds(sleep_ms);
    ++iter;
  }
}
