//*****************************************************************************
//BT_SmartCar V4.0
//阿布拉機的3D列印與機器人
//http://arbu00.blogspot.tw/
//
//2017/5  Writen By Ashing Tsai
//
//******************************************************************************
#include <Timer.h>  
#include <Servo.h> 
#include <SoftwareSerial.h>

const int M1APin=3;
const int M1BPin=5;
const int M2APin=6;
const int M2BPin=11;


const int sig=2;
const int Eco=4;
const int ServoPin=13;
const int TracePinM=10;
const int TracePinR=9;
const int TracePinL=12;

int angleA=90;
int angle=90;
int angleR=30;
int angleL=170;
int angleM=90;
int  Keepflag=1;
unsigned long cm;
unsigned long time;
int speed=250;
int speed_now=250;
Servo servoA;
int Flag=0;

boolean TraceFlagL=false; 
boolean TraceFlagM=false; 
boolean TraceFlagR=false; 

int randNumber,RLFlag;
int TraceCount=0;
int TraceCountw=0;
int TraceRCount=0;
int TraceLCount=0;
const int BTPinR=7;
const int BTPinT=8;

char ModeSelect='A';  // 3(C):Trace mode, 2(B):auto mode 1(A):phone controll Q:Serial port mode
boolean DirectionFlag=true; 
boolean DirectionRFlag=false; // turn Right first;
boolean DirectionLFlag=false; // turn left first;
boolean startrun=true;


Timer t1,t2;
SoftwareSerial BT(BTPinR,BTPinT);    
String readString;       
char ch,Fisrtchar,speedchar;      
  
void setup() 
{ 
  BT.begin(9600);      
  Serial.begin(115200);
  Serial.println("press '0'~'9' : setup speed");
  Serial.println("press 'S' : stop");  
  Serial.println("press 'F' :  front");
  Serial.println("press 'B' :  back");   
  Serial.println("press 'R' : turn right");
  Serial.println("press 'L' : turn left"); 

   pinMode(sig,OUTPUT);
   pinMode(Eco,INPUT);
   pinMode(TracePinL,INPUT); 
   pinMode(TracePinM,INPUT); 
   pinMode(TracePinR,INPUT); 
   
  servoA.attach(ServoPin);
  ServoangleA2(angleM);
  delay(200);

   stop();
   Flag=0;

    t1.every(1, Check_BT);
      if (ModeSelect=='Q'){  //serial port debug mode
      t2.every(1, DebugMode);
   }

} 
void loop() 
{
   if (ModeSelect=='Q'){  //serial port debug mode
      t2.update();
   }else
   {
     t1.update();
    }

 if (ModeSelect=='A'){   //phone mode
  if (readString=="ST") { 
          stop(); 
        } else if (readString=="FW") { 
              ForwardPWM(); 
        } else if (readString=="BW") { 
              backwardPWM(); 
        } else if (readString=="RW") { 
              rightwardPWM();    
        } else if (readString=="LW") { 
              leftwardPWM();    
        }   
  }
    //===========================================
    //-Auto mode-----------------------------------------------
    //=========================================== 
   if (ModeSelect=='B'){  //Auto mode
           speed=speed_now;
        ForwardPWM();
        Checkhit();
        
    }//--------------------------------------------------------
    //===========================================
    //-Trace mode-----------------------------------------------
    //=========================================== 
   if (ModeSelect=='C'){  //Trace mode
      speed=map(speedchar,'0','9',110,200);
        TraceModeA();
    }//-------------------------------------------------------- 

} 
//Get Supersonic distance  CM
int ping(int sig)
{
   unsigned long cm,duration;
   //===================5us==================
   pinMode(sig,OUTPUT);
   digitalWrite(sig,LOW);
   delayMicroseconds(3);
   digitalWrite(sig,HIGH);
   delayMicroseconds(5);
   digitalWrite(sig,LOW);
   delayMicroseconds(3);
   //=====================================
   pinMode(Eco,INPUT);
   duration=pulseIn(Eco,HIGH);
   cm=duration/29/2;
   return cm;
}  
void Checkhit(void)
{
        cm=ping(sig);
        Serial.println(cm);

    if ((cm >0)&&(cm <=1000))
 {
   if (cm <=30)
   {
     Flag=Flag+1;
   }
 }
 if (Flag>=3)
    { Flag=0;
      SafeReturn();
    } 
}

void ForwardPWM() 
{
       analogWrite(M1APin,0);
       analogWrite(M1BPin,speed_now);
       analogWrite(M2APin,0);
       analogWrite(M2BPin,speed_now);
 }

void backwardPWM() 
{     
       analogWrite(M1APin,speed_now);
       analogWrite(M1BPin,0);
       analogWrite(M2APin,speed_now);
       analogWrite(M2BPin,0);   
}

void rightwardPWM() 
{
       analogWrite(M1APin,0);
       analogWrite(M1BPin,speed_now);
       analogWrite(M2APin,speed_now);
       analogWrite(M2BPin,0);
}

void leftwardPWM() 
{
       analogWrite(M1APin,speed_now);
       analogWrite(M1BPin,0);
       analogWrite(M2APin,0);
       analogWrite(M2BPin,speed_now);
}
void stop() 
{
   pinMode(M1APin,OUTPUT);
   pinMode(M1BPin,OUTPUT);
   pinMode(M2APin,OUTPUT);
   pinMode(M2BPin,OUTPUT);
   digitalWrite(M1APin,LOW);
   digitalWrite(M1BPin,LOW);
   digitalWrite(M2APin,LOW);
   digitalWrite(M2BPin,LOW);

     
}

int ORandom()
{

 // if analog input pin 7 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(7));
  
  // print a random number from 2 to 1
  randNumber = random(0, 2);
 // Serial.println(randNumber);
 // delay(50);  
  return (randNumber);
  
}

void TraceModeA()
 {

     TraceFlagL=digitalRead(TracePinL); //pin 13
       Serial.print("TraceFlagL="); 
       Serial.println(TraceFlagL);
     TraceFlagM=digitalRead(TracePinM); //pin 12
       Serial.print("TraceFlagM="); 
       Serial.println(TraceFlagM);
     TraceFlagR=digitalRead(TracePinR); //pin 11
       Serial.print("TraceFlagR="); 
       Serial.println(TraceFlagR);

 
     if ((TraceFlagR==0)&&(TraceFlagM==0)&&(TraceFlagL==1))       //如果右感測器壓到黑線試著右轉
  {   DirectionRFlag=true;   
      DirectionLFlag=false; 
    
  }else  if ((TraceFlagR==1)&&(TraceFlagM==0)&&(TraceFlagL==0))    {   //如果左感測器壓到黑線試著左轉
      DirectionLFlag=true;  
      DirectionRFlag=false;   
  }

      if (TraceFlagM==1)    {    //如果中間感測器壓到黑線直走
          TraceCount=0;  
          TraceLCount=0;
          TraceRCount=0;
          
           speed=speed_now;//speed_now;
           ForwardPWM();  

      }else{                             // 如果中間感測器沒壓到黑線
                    if ( DirectionRFlag==true){ 
                            speed=speed_now;//speed_now;
                    
                                for (int j=0;j<=20;j++)
                            {
                             rightwardPWM();  
              
                             TraceFlagL=digitalRead(TracePinL);
                             TraceFlagM=digitalRead(TracePinM); 
                             TraceFlagR=digitalRead(TracePinR);
                             if(TraceFlagM==1){
                                 break;
                                 }    
                            }

                           }else if ( DirectionLFlag==true){
                                speed=speed_now;//speed_now;
                                for (int l=0;l<=20;l++)
                            {
                             leftwardPWM();
  
                             TraceFlagL=digitalRead(TracePinL);
                             TraceFlagM=digitalRead(TracePinM); 
                             TraceFlagR=digitalRead(TracePinR);
                             if(TraceFlagM==1){
                                 break;
                                 }  
                            }
                    }
                    
      }
}

void SafeReturn_Random()
{
          speed=speed_now;
          Serial.println("SafeReturn"); 
          Flag=0;
          stop();
          delay(500);
          backwardPWM();
          delay(800);
         
          RLFlag=ORandom();  //Get random value
          // Serial.println(RLFlag);
        if (RLFlag==0x01){    //if 1 then turn right.
           rightwardPWM();
           delay(500);
        }else                 //if 0 turn left
        {  leftwardPWM();
           delay(500);
        }
 
 }

void SafeReturn()
{
  int cmR,cmL;
  cmR=0;
  cmL=0;

  Serial.println("SafeReturn"); 
  Flag=0;
      
  backwardPWM();
  delay(300);
  stop();
         
 ServoangleA2(angleR);
 delay(300);

  cm=ping(sig);
 if ((cm >0)&&(cm <=1500))
 {  
    for(int i=1;i<=5;i++)
  {
  cmR=(cmR+ping(sig))/i;
   // Serial.print(cmR);
  //Serial.print('\n');
  }
 }
  Serial.print("cmR=");
  Serial.println(cmR);
  
  ServoangleA2(angleL);
  delay(300);
  
  cm=ping(sig);
 if ((cm >0)&&(cm <=1500))
 {  
    for(int i=1;i<=5;i++)
  {
  cmL=(cmL+ping(sig))/i;
 
  }
 }
  Serial.print("cmL=");
  Serial.println(cmL);
  ServoangleA2(angleM);
  delay(300);
  
     if (cmR>cmL){
       rightwardPWM();
       delay(500);
    }else
    {
       leftwardPWM();
       delay(500);
    }

 }
void Check_BT()
{
  
//BT+========================
  if(readString.length()>0) {  
    readString="";
  }
  while(BT.available()) {  
        delay(1);   
        ch = BT.read();
        readString+=ch;
  }
     //  Serial.println(readString);
     // delay(1000);

         if (readString[0]=='A'&&readString[1]=='A'){
             ModeSelect='A'; 
             stop();
              speed=speed_now;
         }else if(readString[0]=='B'&&readString[1]=='B'){
             ModeSelect='B';
             stop();
                 speed=speed_now;
         }else if(readString[0]=='C'&&readString[1]=='C'){
             ModeSelect='C';
             startrun=true;
             stop();  
                 speed=speed_now;
         }
       //  Serial.println(ModeSelect);
       //  delay(1000);

   if (readString[0]=='P'){
       speedchar=readString[1];
            if (ModeSelect=='C'){  //Trace mode
             speed=map(speedchar,'0','9',110,200);
             }else{
             speed=map(speedchar,'0','9',150,255);
             }
       speed_now=speed;
         // Serial.println(readString);
         // delay(500);
    }
    
//BT-========================

}

void DebugMode()
{
    String readString2="";
   if(Serial.available())
  {

        while(Serial.available()) {  
        delay(1);   
         char key = Serial.read();
        readString2+=key;
        } 
        char key=readString2[0];
        Serial.print("key=");
        Serial.println(key);
     if(key>='0' && key<='9')
    {
      speed=map(key,'0','9',150,255);
       Serial.print("speed=");
       Serial.println(speed);
    }  
    if(key=='S'||key=='s')
    {
     stop(); 
    }  
    else if(key=='F'||key=='f')
    {
      stop();
      delay(100);
      ForwardPWM();

    }  
    else if(key=='B'||key=='b')
    {
     stop();
     delay(100);
     backwardPWM(); 
    }  
    else if(key=='R'||key=='r')
    {
     rightwardPWM();   
    }  
    else if(key=='L'||key=='l')
    {
     leftwardPWM(); 
    } else if(key=='T'||key=='t')
    {
   
     TraceFlagL=digitalRead(TracePinL);
     Serial.println(TraceFlagL);
     TraceFlagM=digitalRead(TracePinM);
     Serial.println(TraceFlagM);
     TraceFlagR=digitalRead(TracePinR);
     Serial.println(TraceFlagR);
     delay(1000);
    } else if(key=='G'||key=='g')
    {
      for (int i=0;i<=5;i++){
           cm=ping(sig);
           Serial.print("Ultrasonic detected Distance=");
           Serial.print(cm);
           Serial.println("cm");
      }
    }     
    
  }     

//Debug mode-=============================

}

void ServoangleA2(int angle)
{
                 servoA.write(angle);
                 delay(200);  //200ms
}



 
