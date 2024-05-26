#include <stdlib.h>
#include <string.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int min_delay = 8;
const int max_delay = 30;
const int steps=40;

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

const int endstopPinS = 11;
const int endstopPinL = 12;
const int endstopPinZ = 13;

double stopnieL = 90;
double stopnieS = 180;

char *eptr;
bool startedSending = false;
int textPos = 3;
String text = "Ready to work!";

int allCommands = 0;
int processedCommands = 0;
// variable that displays an animation when the input stream is empty twice in a row
int canDisplayLoop = false;
// bitmaps to draw percentage line

byte zero[] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000};
byte one[] = {
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000};
byte two[] = {
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000};
byte three[] = {
    B11100,
    B11100,
    B11100,
    B11100,
    B11100,
    B11100,
    B11100,
    B11100};
byte four[] = {
    B11110,
    B11110,
    B11110,
    B11110,
    B11110,
    B11110,
    B11110,
    B11110};
byte five[] = {
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111};

enum Status
{
  SUCCESS,
  ENDSTOP_L_N,
  ENDSTOP_L_P,
  ENDSTOP_S_N,
  ENDSTOP_S_P,
  ENDSTOP_Z_N,
  ENDSTOP_Z_P
};
// values of endstops
bool whichEndstopPressed[6] = {false, false, false, false, false, false};

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(50);
  pinMode(stepPinL, OUTPUT);
  pinMode(dirPinL, OUTPUT);
  pinMode(enPinL, OUTPUT);

  pinMode(stepPinS, OUTPUT);
  pinMode(dirPinS, OUTPUT);
  pinMode(enPinS, OUTPUT);

  pinMode(stepPinZ, OUTPUT);
  pinMode(dirPinZ, OUTPUT);
  pinMode(enPinZ, OUTPUT);

  pinMode(endstopPinL, INPUT_PULLUP);
  pinMode(endstopPinS, INPUT_PULLUP);
  pinMode(endstopPinZ, INPUT_PULLUP);

  lcd.begin();
  lcd.createChar(0, zero);
  lcd.createChar(1, one);
  lcd.createChar(2, two);
  lcd.createChar(3, three);
  lcd.createChar(4, four);
  lcd.createChar(5, five);
  lcd.clear();

  lcd.setCursor(2, 0);
  lcd.print("HELLO WORLD");
  delay(1000);
}

void loop()
{
  // reading input
  if (Serial.available())
  {
    // read the incoming byte:
    String str = Serial.readString();

    // reply if communication started
    if (str.indexOf("START") != -1)
    {
      lcd.clear();
      Serial.print("OK");
      startedSending = true;
      lcd.setCursor(3, 0);
      lcd.print("CONNECTED");
    }
    else if (str.indexOf("END") != -1)
    {
      startedSending = false;
    }
    else
    {
      if (allCommands == 0)
      {
        lcd.setCursor(3, 0);
        lcd.print("PROCESSING");
      }
      canDisplayLoop = false;
      str = removeSTART(str);
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
        // continue from previous position
        command = strtok(0, " ");
      }

      int moveS = 0;
      int moveL = 0;
      int moveZ = 0;
      if (splitStrings[0] == "commands")
        allCommands = splitStrings[1].toInt();

      int motorSpeed = 10;

      for (int i = cnt - 1; i >= 0; i--)
      {

        long moved = getLong(splitStrings, i);
        if (splitStrings[i][0] == 'F')
          motorSpeed = moved;
        else if (splitStrings[i][0] == 'L')
          moveL = moved;
        else if (splitStrings[i][0] == 'S')
          moveS = moved;
        else if (splitStrings[i][0] == 'Z')
          moveZ = moved;
      }

      if (allCommands != 0)
      {
        if (processedCommands < allCommands)
          ++processedCommands;
        else
        {
          allCommands = 0;
          processedCommands = 0;
        }
        updateProgressBar(processedCommands, allCommands, 1);
      }
      double alpha = moveL * 9 / 35;
      stopnieL += alpha;
      stopnieS += (moveS * 9 / 20 + alpha + 30 / 116) * 25 / 116;
      Status status = moveStepperSmooth(moveL, moveS, moveZ, true);
      switch (status)
      {
      case Status::SUCCESS:
        Serial.print("OK");
        break;
      case Status::ENDSTOP_L_N:
        Serial.print("ENDSTOP_L_N");
        break;
      case Status::ENDSTOP_L_P:
        Serial.print("ENDSTOP_L_P");
        break;
      case Status::ENDSTOP_S_N:
        Serial.print("ENDSTOP_S_N");
        break;
      case Status::ENDSTOP_S_P:
        Serial.print("ENDSTOP_S_P");
        break;
      case Status::ENDSTOP_Z_N:
        Serial.print("ENDSTOP_Z_N");
        break;
      case Status::ENDSTOP_Z_P:
        Serial.print("ENDSTOP_Z_P");
        break;
      }
    }
  }
  else if (!startedSending)
  {
    if (canDisplayLoop)
    {
      lcd.clear();
      lcd.setCursor(textPos, 1);
      if (textPos > 6)
      {
        lcd.print(text.substring(0, 16 - textPos));
        lcd.setCursor(0, 1);
        lcd.print(text.substring(16 - textPos, text.length()));
      }
      else
        lcd.print(text);
      --textPos;
      if (textPos == -1)
        textPos = 16;
      delay(700);
      processedCommands = 0;
    }
    canDisplayLoop = true;
  }

   delay(50);
}

void updateProgressBar(unsigned long count, unsigned long totalCount, int lineToPrintOn)
{
  int percent = count * 100 / totalCount;
  int currentCol = 0;
  lcd.setCursor(currentCol, 1);

  if (count >= totalCount)
  {
    for (int i = 0; i <= 19; i++)
    {
      lcd.setCursor(i, lineToPrintOn);
      lcd.write(5);
    }
    // delay(500);
    startedSending = false;
  }
  else
  {
    while (percent > 5 && currentCol < 19)
    {
      lcd.setCursor(currentCol, lineToPrintOn);
      lcd.write(5);
      percent -= 5;
      ++currentCol;
    }
    if (currentCol < 19)
    {
      lcd.setCursor(currentCol, lineToPrintOn);
      lcd.write(round(percent));
    }
  }
}

long int getLong(String *splitStrings, int i)
{
  int tsize = sizeof(splitStrings[i]) / sizeof(splitStrings[i][0]);
  char tab[tsize];
  strncpy(tab, &splitStrings[i][1], tsize);
  long ret = strtol(tab, &eptr, 0);
  if (splitStrings[i][1] == '-' && ret > 0)
    ret *= -1;
  return ret;
}

/**
 * Remove 'START' strings from line and return it
 */
String removeSTART(String line)
{
  int position = line.indexOf("START");
  while (position != -1)
  {
    String partBefore = line.substring(0, position);
    String partAfter = line.substring(position + 5);

    line = partBefore + partAfter;
    position = line.indexOf("START");
  }
  return line;
}

/**
 *Function to move stepper motor
 * @param stepL - steps of arm connected to base
 * @param stepS - steps of arm connected to tool
 * @param stepL - steps of Z axis
 * @param secure - bool value if use endstops
 * @return Status - result of moving arm/ interruptions
 */

Status moveStepperSmooth(int stepL, int stepS, int stepZ, bool secure){

  int stepTMS = stepS;
  int stepTML = stepL;
  int stepTMZ = stepZ;

  // Set directions and set absolute value to axis moves
  calculateDirectionAndAbsValue(stepTMS, stepS, dirPinS, enPinS);
  calculateDirectionAndAbsValue(stepTML, stepL, dirPinL, enPinL);
  calculateDirectionAndAbsValue(stepTMZ, stepZ, dirPinZ, enPinZ);

  // Calculate scale for every motor to perform moves together smoothly
  double scaleS = 1;
  double scaleL = 1;
  double scaleZ = 1;

  double maxScale = stepTMS;

  if (maxScale < stepTML)
    maxScale = stepTML;
  if (maxScale < stepTMZ)
    maxScale = stepTMZ;
  if (maxScale == 0)
    maxScale = 1;

  // how often need to make step
  if (stepTMS != 0)
    scaleS = maxScale / stepTMS;
  if (stepTML != 0)
    scaleL = maxScale / stepTML;
  if (stepTMZ != 0)
    scaleZ = maxScale / stepTMZ;

  // Current motors steps
  int currentStepS = 0;
  int currentStepL = 0;
  int currentStepZ = 0;

  int iter = 1;

  // Loop executing stepper motor move
  while (currentStepS < stepTMS || currentStepL < stepTML || currentStepZ < stepTMZ)
  {

    bool changeS = checkChange(stepTMS, currentStepS, scaleS, iter);
    bool changeL = checkChange(stepTML, currentStepL, scaleL, iter);
    bool changeZ = checkChange(stepTMZ, currentStepZ, scaleZ, iter);

    // interpolation
    
    int delayMS;
  
  /*
    if (iter < maxScale / 2)
      delayMS = (int)(max_delay - (iter / (maxScale / 2) * (max_delay - min_delay)));
    else
      delayMS = (int)(min_delay + ((iter - maxScale / 2) / (maxScale / 2) * (max_delay - min_delay)));
*/


  if(iter<=steps)
    delayMS = (int)(max_delay - (iter / steps * (max_delay - min_delay)));
  else if(maxScale-steps<iter)
    delayMS = (int)(min_delay + ((maxScale-iter)/steps* (max_delay - min_delay)));
  else
    delayMS = min_delay;

/*
    if (iter < maxScale / 2)
      delayMS = (int)max(min_delay, max_delay-(pow(iter/maxScale/4,2) *(max_delay-min_delay)));
    else
      delayMS =min(max_delay,min_delay+(pow(iter/maxScale/4,2)*(max_delay-min_delay)));
*/



   /* 
  int delayMS;
  if (iter < maxScale / 2) {
    double fraction = double(iter) / double(maxScale / 2);
    delayMS = (int)(max_delay - fraction * fraction * (max_delay - min_delay));
  } else {
    double fraction = double(iter - maxScale / 2) / double(maxScale / 2);
    delayMS = (int)(min_delay + fraction * fraction * (max_delay - min_delay));
  }
*/
    if(secure){
      Status endstopState= checkEndstops(endstopPinL, endstopPinS, endstopPinZ, stepL, stepS, stepZ);
      if(endstopState != Status::SUCCESS){
        return endstopState;
      }
    }

    // move motors one step
    if (changeS){
      digitalWrite(stepPinS, !digitalRead(stepPinS));
      ++currentStepS;
    }
    if (changeL){
      digitalWrite(stepPinL, !digitalRead(stepPinL));
      ++currentStepL;
    }
    if (changeZ){
      digitalWrite(stepPinZ, !digitalRead(stepPinZ));
      ++currentStepZ;
    }
    delay(delayMS);

    if (changeS){
      digitalWrite(stepPinS, !digitalRead(stepPinS));
      ++currentStepS;
    }
    if (changeL){
      digitalWrite(stepPinL, !digitalRead(stepPinL));
      ++currentStepL;
    }
    if (changeZ){
      digitalWrite(stepPinZ, !digitalRead(stepPinZ));
      ++currentStepZ;
    }
    delay(delayMS);
    ++iter;
  }

  return Status::SUCCESS;
}

/**
* Function which will set direction for motor and change stepTM to absolute value
**/
void calculateDirectionAndAbsValue(int &stepTM, int &step, int dirPin, int enPin)
{
  if (step != 0)
  {
    // digitalWrite(enPin, LOW);
    if (step > 0)
      digitalWrite(dirPin, HIGH);
    else
    {
      digitalWrite(dirPin, LOW);
      stepTM *= -1;
    }
  }
}
/**
 * Function to check if there left steps to make for motor and if it need to be made at this moment
 * @param stepTM speps to make
 * @param currentStep current steps made
 * @param scale scale of this motor (how often motor make move relative to base motor)
 * @param iter current iteration
*/
bool checkChange(int stepTM, int currentStep, double scale, int iter)
{
  return (currentStep < stepTM && (int)floor(iter / scale) > currentStep);
}

/**
 * Function to check if endstop is triggered
 * @param endstopPinL pin on board connected to endstop L
 * @param endstopPinS pin on board connected to endstop S
 * @param endstopPinZ pin on board connected to endstop Z
 * @param currentStepL int - steps to made by motor L, 0 if no move, needed to specify move direction
 * @param currentStepS int - steps to made by motor S, 0 if no move, needed to specify move direction
 * @param currentStepZ int - steps to made by motor Z, 0 if no move, needed to specify move direction
 * @return Status of endstop or Status::SUCCESS if not triggered
 * !!! Be sure to check if endstops works properly with your motors !!!
 */
Status checkEndstops(int endstopPinL, int endstopPinS, int endstopPinZ, int currentStepL, int currentStepS, int currentStepZ)
{
  if (currentStepL != 0)
  {
    if (digitalRead(endstopPinL) == LOW)
    {
      if (currentStepL > 0 && !whichEndstopPressed[0])
      {
        whichEndstopPressed[1] = true;
        return Status::ENDSTOP_L_N;
      }
      else if (currentStepL < 0 && !whichEndstopPressed[1])
      {
        whichEndstopPressed[0] = true;
        return Status::ENDSTOP_L_P;
      }
    }else{
      whichEndstopPressed[0] = false;
      whichEndstopPressed[1] = false;
    }
  }

  if (currentStepS != 0)
  {
    if (digitalRead(endstopPinS) == LOW)
    {
      if (currentStepS > 0 && !whichEndstopPressed[2])
      {
        whichEndstopPressed[3] = true;
        return Status::ENDSTOP_S_P;
      }
      else if (currentStepS < 0 && !whichEndstopPressed[3])
      {
        whichEndstopPressed[2] = true;
        return Status::ENDSTOP_S_N;
      }
    }else{
      whichEndstopPressed[2] = false;
      whichEndstopPressed[3] = false;
    }
  }

  if (currentStepZ != 0)
  {
    if (digitalRead(endstopPinZ) == LOW)
    {
      if (currentStepZ > 0 && !whichEndstopPressed[4])
      {
        whichEndstopPressed[5] = true;
        return Status::ENDSTOP_Z_P;
      }
      else if (currentStepZ < 0 && !whichEndstopPressed[5])
      {
        whichEndstopPressed[4] = true;
        return Status::ENDSTOP_Z_N;
      }
    }else{
      whichEndstopPressed[4] = false;
      whichEndstopPressed[5] = false;
    }
  }
  return Status::SUCCESS;
}
