#include <stdlib.h>
#include <string.h>
#include <LiquidCrystal.h>

//LiquidCrystal lcd(A5, A4, A3, A2, A1, A0);
int sleep_ms=500;
int max_speed=100;
const int stepPinS=8;
const int dirPinS=9;
const int enPinS=10;

const int stepPinL=5;
const int dirPinL=6;
const int enPinL=7;

const int stepPinZ=2;
const int dirPinZ=3;
const int enPinZ=4;

char *eptr;

void setup() {  
  Serial.begin(9600);  
  
  pinMode(stepPinL,OUTPUT);
  pinMode(dirPinL,OUTPUT);
  pinMode(enPinL,OUTPUT);
    
  pinMode(stepPinS,OUTPUT);
  pinMode(dirPinS,OUTPUT);
  pinMode(enPinS,OUTPUT);
    
  pinMode(stepPinZ,OUTPUT);
  pinMode(dirPinZ,OUTPUT);
  pinMode(enPinZ,OUTPUT);

  //lcd.begin(16, 2); //Deklaracja typu

}
/*
long int getLong(char splitStrings[][10],int i ){
    int tsize=sizeof(splitStrings[i])/sizeof(splitStrings[i][0]);   
     char tab [tsize];
     strncpy(tab,&splitStrings[i][1],tsize);
     long ret=strtol(tab, &eptr,0);
     if(splitStrings[i][1]=='-'&&ret>0)
     ret*=-1;
     return ret;
  }*/

  long int getLong(String *splitStrings,int i ){
    int tsize=sizeof(splitStrings[i])/sizeof(splitStrings[i][0]);   
     char tab [tsize];
     strncpy(tab,&splitStrings[i][1],tsize);
     long ret=strtol(tab, &eptr,0);
     if(splitStrings[i][1]=='-'&&ret>0)
     ret*=-1;
     return ret;
  }
void loop() {
 
  if (Serial.available() > 0) {
   
    // read the incoming byte:
    String str=Serial.readString();//.toCharArray(line,50);

    if(str=="START")
       Serial.print("OK");
    else{     
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
          // Split the command in two values
         // char* separator = strchr(command, ':');
          /*if (separator != 0)
          {
              // Actually split the string in 2: replace ':' with 0
              *separator = 0;
              int servoId = atoi(command);
              ++separator;
              int position = atoi(separator);
      
              // Do something with servoId and position
          }*/
          // Find the next command in input string
          //if(command[0]!=NULL){
         // int tsize=sizeof(comand) / sizeof(comand[0])
         //  std::copy(std::begin(command), std::end(command), std::begin(splitStrings[cnt++]));
          splitStrings[cnt++]=command;
         // }
          command = strtok(0, " ");
      }


      /*
      char*str,*p=line;
      while(str=strtok(p,' ')){
        splitStrings[cnt++]=str;
        p=NULL;
        }*/
      /*
      for (i = 0; i <= str_len; i++) {
   
        if (line[i] == ' ') {
          ++cnt; //for next word
          j = 0; //for next word, init index to 0
        }else if(line[i] == '\0'){
          break;
          }
        else {
          splitStrings[cnt][j++] = line[i];
        }
      } */
  
      int motorSpeed=10;
      int moveS=0;
      int moveL=0;
      int moveZ=0;
      for(int i=cnt-1;i>=0;i--){
        long moved=getLong(splitStrings,i);
        if(splitStrings[i][0]=='F'){
          motorSpeed=moved;
      
        }else if(splitStrings[i][0]=='L'){
            moveL=moved;
           
        }else if(splitStrings[i][0]=='S'){
         // delay(30);
            moveS=moved;
        }else if(splitStrings[i][0]=='Z'){
            moveZ=moved;
        }
      }
   //  lcd.clear();
      // lcd.print(String(moveL)+" "+String(moveS));
      moveStepper(moveL,moveS,moveZ);

      Serial.print("OK");
    }
  }
} 


void moveStepper(int stepL,int stepS,int stepZ){
  int stepTMS=stepS;
  int stepTML=stepL;
  int stepTMZ=stepZ;
  
  if(stepS!=0){
    digitalWrite(enPinS,LOW);
    if(stepS>0)
       digitalWrite(dirPinS,HIGH);
     else{
      digitalWrite(dirPinS,LOW);
      stepTMS*=-1; 
      }
  }
  if(stepL!=0){
    digitalWrite(enPinL,LOW);
    if(stepL>0)
       digitalWrite(dirPinL,LOW);
     else{
      digitalWrite(dirPinL,HIGH);
     stepTML*=-1;
     }
  }
  if(stepZ!=0){
    digitalWrite(enPinZ,LOW);
    if(stepZ>0)
       digitalWrite(dirPinZ,HIGH);
     else{
      digitalWrite(dirPinZ,LOW);
     stepTMZ*=-1;
     }
  }

  double scaleL=1;
  double scaleS=1;
  double scaleZ=1;

  //biggest amount of steps in line
  double maxScale=1;
 
  if(stepTMS!=0)
    maxScale=stepTMS;
 // char minChar='s';
  if(maxScale<stepTML&&stepTML!=0){
    maxScale=stepTML;
   // minChar='l';
  }
  if(maxScale<stepTMZ&&stepTMZ!=0){
    maxScale=stepTMZ;
   // minChar='z';
  }
    // how often need to make step
  if(stepTMS!=0)
    scaleS=maxScale/stepTMS;
  if(stepTML!=0)
    scaleL=maxScale/stepTML;
  if(stepTMZ!=0)
    scaleZ=maxScale/stepTMZ;

  int currentStepZ=0;
  int currentStepS=0;
  int currentStepL=0;

  int liczZ=0;
  int liczS=0;
  int liczL=0;
  
  int iter=1;
  bool changeS;
  bool changeL;
  bool changeZ;
  
  while(currentStepZ<stepTMZ||currentStepS<stepTMS||currentStepL<stepTML){
    changeS=false;
    changeL=false;
    changeZ=false;
     if(currentStepS<stepTMS&&((int)floor(iter/scaleS))>liczS)
        changeS=true;
    if(currentStepL<stepTML&&((int)floor(iter/scaleL))>liczL)
        changeL=true;
    if(currentStepZ<stepTMZ&&((int)floor(iter/scaleZ))>liczZ)
        changeZ=true;
      
    if(changeS){
      digitalWrite(stepPinS,!digitalRead(stepPinS));    
      delayMicroseconds(sleep_ms);
      digitalWrite(stepPinS,!digitalRead(stepPinS));
      ++currentStepS;
      ++liczS;
     }
     if(changeL){
      digitalWrite(stepPinL,!digitalRead(stepPinL));
      delayMicroseconds(sleep_ms);
      digitalWrite(stepPinL,!digitalRead(stepPinL));
      ++currentStepL;
      ++liczL;
      }
     if(changeZ){
      digitalWrite(stepPinZ,!digitalRead(stepPinZ));    
      delayMicroseconds(sleep_ms);
      digitalWrite(stepPinZ,!digitalRead(stepPinZ));   
      ++currentStepZ;
      ++liczZ;
      } 
      delayMicroseconds(sleep_ms);
    ++iter;
    }

      
   // digitalWrite(enPinS,HIGH);
   // digitalWrite(enPinL,HIGH);
   // digitalWrite(enPinZ,HIGH);
  }
