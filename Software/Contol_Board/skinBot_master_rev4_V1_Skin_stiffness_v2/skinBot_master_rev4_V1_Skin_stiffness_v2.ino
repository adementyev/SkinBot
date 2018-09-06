//For teensy 3.6 board. It is for the skinBot board 4th revision (rev4) 
//Use 4V with the supply for motors.

//New Features///////////
/*
  1) Pressure is reversed from the previous board
  2) adding pressure sensing with FSR here
 */ 


/*******MOTOR POSITIONING (be careful: some servos can be reversed)     
 *         -
 *        - -  (gear)    
 *  1000--------1800
 *        - -
 *    left- -right side
 *        - - 
 *  1800--- ----1000
 * (gear)
 * 
 */

 //The new motor can only move between 1500 (Don't move it >1500, it will get stuck)

//BUTTON 1 - TEST SERVO MOTORS
//BUTTON 2 - CONTINOUS ADHESION TESTING
//BUTTON 3 - 
//BUTTON 4 - START WALKING SEQUECE

#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>b 
#include <Servo.h>

#define robot1 
//#define robot2


#define BUTTON_1        20
#define BUTTON_2        15
#define BUTTON_3        31
#define BUTTON_4        14
#define BUTTON_1_LED    2
#define BUTTON_2_LED    3
#define BUTTON_3_LED    4
#define BUTTON_4_LED    6
#define SERVO_VERTICAL_LEFT    36 
#define SERVO_HORIZONTAL_LEFT  35
#define SERVO_VERTICAL_RIGHT   38 
#define SERVO_HORIZONTAL_RIGHT 37 
#define OLED_RESET      16
#define LED_1           24
#define LED_2           25
#define LED_L           26
#define LED_R           27
#define PUMP_L_C        8
#define PUMP_R_C        7
#define LEFT_PRESSURE   32
#define RIGHT_PRESSURE  23
#define SOLENOID_L      34
#define SOLENOID_R      33 
#define POT_1           22
#define POT_2           21
#define MODE_MOTOR      30
#define MOTOR_PWM       29
#define MOTOR_DIR       28

#define SERVO_VERT_SKIN 17
#define SERVO_VERT_POS  A22
#define FSR_SENSOR A21

#define LEFT_ADHESION_PRESSURE   -23
#define RIGHT_ADHESION_PRESSURE  -23
#define HOLD_PRESSURE            -30

//#define DISPLAY 
 
//Adafruit_SSD1306 display(OLED_RESET);

Servo servo_ver_l; 
Servo servo_hor_l;
Servo servo_ver_r; 
Servo servo_hor_r;
Servo servo_ver_stiff;

IntervalTimer pressureTimer; 
boolean displayPressureData = true;


int counter = 0;
//float pressure = 0;
const float SUPPLY_VOLTAGE = 4.98;
const float ADC_BITS = 8192.0;


const int numReadings = 50; //This defines the running average filter size
//running average for left pressure sensor
float readings_l[numReadings];      // the readings from the analog input
int readIndex_l = 0;              // the index of the current reading
float total_l = 0.0;                  // the running total
float average_pressure_l = 0.0;                // the average
float pressure_l = 0.0;


//running average for right pressure sensor
float readings_r[numReadings];      // the readings from the analog input
int readIndex_r = 0;              // the index of the current reading
float total_r = 0.0;                  // the running total
float average_pressure_r = 0.0;                // the average
float pressure_r = 0.0;


 unsigned long currentMillisTime ; 
int currentVerticalLeft = 0;
int currentVerticalRight = 0;
int currentHorizontalLeft = 0; 
int currentHorizontalRight = 0; 

volatile int currentState = 0;
volatile bool starting = true; 
volatile bool time_pressure_read = false;

boolean servoFlag = false;  
int pressedState = 1;

boolean displayMoreData = true;
boolean delayRobotDebug = false; 
boolean calibrationSequence = true;


boolean rightPumpHolding = false;
boolean leftPumpHolding = false;

float count =0;

float LEFT_RELEASE_PRESSURE = 0; //-4 for the first robot
float RIGHT_RELEASE_PRESSURE = 0;  //-2 for the first robot

int ROBOT_ID = 2 ;

int rotationMagnitude = 70; //from 0 to 60
int rotationDirection = 1; // 0-left , 1-right, 2-off
int motorSpeed = 250;

int timeStartingPlungle;

void setup() {
  SerialUSB.begin(0);
  analogReadResolution(13);

  pinMode(BUTTON_1, INPUT_PULLUP); //0 means pushed
  pinMode(BUTTON_2, INPUT_PULLUP); //0 means pushed
  pinMode(BUTTON_3, INPUT_PULLUP); //0 means pushed
  pinMode(BUTTON_4, INPUT_PULLUP); //0 means pushed
  pinMode(BUTTON_1_LED, OUTPUT); 
  pinMode(BUTTON_2_LED, OUTPUT); 
  pinMode(BUTTON_3_LED, OUTPUT); 
  pinMode(BUTTON_4_LED, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_L, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(PUMP_L_C, OUTPUT); 
  pinMode(PUMP_R_C, OUTPUT);
  pinMode(SOLENOID_R, OUTPUT);
  pinMode(SOLENOID_L, OUTPUT);
  pinMode(MODE_MOTOR,OUTPUT);
  pinMode(MOTOR_PWM,OUTPUT);
  pinMode(MOTOR_DIR,OUTPUT);

  servo_hor_l.attach(SERVO_HORIZONTAL_LEFT); 
  servo_ver_l.attach(SERVO_VERTICAL_LEFT);
  servo_hor_r.attach(SERVO_HORIZONTAL_RIGHT);
  servo_ver_r.attach(SERVO_VERTICAL_RIGHT);
  servo_ver_stiff.attach(SERVO_VERT_SKIN);

  
  servo_ver_l.write(1000); //extend forward
  servo_ver_r.write(1000); //extend forward

  servo_ver_stiff.write(1500); //max in
  /*
  for (int i=1000; i<1500 ; i++) { 
        servo_ver_stiff.write(i); //about 1500 is the max inside
        delay(10);
        SerialUSB.print(analogRead(SERVO_VERT_POS));
        SerialUSB.print(" ,");
        SerialUSB.println(i);
  }
  */

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings_l[thisReading] = 0;
  }
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings_r[thisReading] = 0;
  }

    digitalWrite(BUTTON_1_LED, HIGH);
    digitalWrite(BUTTON_2_LED, HIGH);
    digitalWrite(BUTTON_3_LED, HIGH);
    digitalWrite(BUTTON_4_LED, HIGH);
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    digitalWrite(LED_L, HIGH);
    digitalWrite(LED_R, HIGH);
    digitalWrite(MODE_MOTOR,HIGH);

  


  for(int i=0; i<2; i++) {
    digitalWrite(LED_1, HIGH); 
    digitalWrite(LED_2, LOW); 
    digitalWrite(SOLENOID_L, HIGH); 
    delay(100);
    digitalWrite(LED_1, LOW); 
    digitalWrite(LED_2, HIGH);
    digitalWrite(SOLENOID_L, LOW);   
    delay(100);
  }

      for(int i=0; i<2; i++) {
    digitalWrite(LED_1, HIGH); 
    digitalWrite(LED_2, LOW); 
    digitalWrite(SOLENOID_R, HIGH); 
    delay(100);
    digitalWrite(LED_1, LOW); 
    digitalWrite(LED_2, HIGH);
    digitalWrite(SOLENOID_R, LOW); 
    delay(100);
  }

    #ifdef DISPLAY 
      //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
     // display.display();
     // delay(1000);
     // display.clearDisplay();
  
     // display.setTextSize(3);
     // display.setTextColor(WHITE);
     // display.setCursor(0,0);
    #endif

  pressureTimer.begin(readPressure, 10000); //0.01sec = 10 ms = 100 Hz

    turnOnLeftPump(); 
    turnOnRightPump();


attachInterrupt(BUTTON_3, isrService_button3, FALLING); // interrrupt 1 is data ready


}

void readPressure(void) { 
    time_pressure_read = true;
}


void isrService_button3()
{
  cli();
  SerialUSB.println("Button3 interrupt");
  currentState = -4;
  digitalWrite(SOLENOID_L, HIGH); //Open pressure release
  digitalWrite(SOLENOID_R, HIGH); //Open pressure release
  leftPumpHolding = false; 
  rightPumpHolding = false;
  sei();
}

// the loop routine runs over and over again forever:
void loop() {
    //POT 2 is used to test the radius of rotation of the motor. 
    int temp = analogRead(POT_2); 
    if (temp>=4096){ 
      analogWrite(MOTOR_PWM,((temp/32)-(4096/32)));
      digitalWrite(MOTOR_DIR, LOW);
    }
    else if (temp<4096){ 
      analogWrite(MOTOR_PWM, (4096/32)-(temp/32)); 
      digitalWrite(MOTOR_DIR, HIGH);
    }

    
    temp = analogRead(POT_1); 
    if (temp>4500){ 
      rotationDirection = 0;
      rotationMagnitude = 60;
    }
    else if (temp<3500){ 
      rotationDirection = 1;
       rotationMagnitude = 60;
    }

    else if (temp>=3500 && temp<=4500) { 
      rotationMagnitude = 0;
    }
    

    if(calibrationSequence) { 
      count++;
      if(millis()>6000) {
        calibrationSequence=false;
        turnOffLeftPump();
        turnOffRightPump();
        LEFT_RELEASE_PRESSURE = average_pressure_l; 
        RIGHT_RELEASE_PRESSURE = average_pressure_r;
        SerialUSB.print("Calibration set:");
        SerialUSB.print(LEFT_RELEASE_PRESSURE);
        SerialUSB.print(","); 
        SerialUSB.println(RIGHT_RELEASE_PRESSURE);      
      }
    }
   //Test motor movements
   if(digitalRead(BUTTON_1)==0) {
      if(pressedState==1){ 
          currentState=0;
          SerialUSB.println("-------------------------------");
          SerialUSB.println("hor_l  to 1700");
          SerialUSB.println("ver_l  to 1000");
          SerialUSB.println("hor_r  to 1800"); 
          SerialUSB.println("ver_r  to 1500");
          
          starting=true; 
          //timer_active = true;
          servo_hor_l.write(1900); //extend forward
          servo_ver_l.write(1000); //extend forward
          servo_hor_r.write(1900); //extend forward
          servo_ver_r.write(1000); //level left and right cups the same height

          
          pressedState = 2;
          delay(600);
      }
      else if(pressedState==2) {
            currentState=0;
            SerialUSB.println("-------------------------------");
            SerialUSB.println("hor_l  to 1100");
            SerialUSB.println("ver_l  to 1500");
            SerialUSB.println("hor_r  to 1000"); 
            SerialUSB.println("ver_r  to 1500");
            
            starting=true; 
            servo_hor_l.write(900); //extend forward
            servo_ver_l.write(1500); //extend forward
            servo_hor_r.write(900); //extend forward
            servo_ver_r.write(1500); //level left and right cups the same height
            pressedState = 3;

            
            delay(600);
      }

      else if (pressedState==3) {
            servo_hor_l.write(1500); //extend forward
            servo_ver_l.write(1500); //extend forward
            servo_hor_r.write(1500); //extend forward
            servo_ver_r.write(1500); //level left and right cups the same height  
            SerialUSB.println("-------------------------------");
            SerialUSB.println("hor_l  to 1500");
            SerialUSB.println("ver_l  to 1500");
            SerialUSB.println("hor_r  to 1500"); 
            SerialUSB.println("ver_r  to 1500");
            pressedState = 1;
            delay(600);
      }   
      else if (pressedState==4) { 
        testSkin() ;
        pressedState = 1;
        
      }

   }
   
   //Make this for the attachment continous. What is the power requirements
   if(digitalRead(BUTTON_2)==0) {
      testSkin() ;
     // digitalWrite(PUMP_L_C, HIGH);
     // digitalWrite(PUMP_R_C, HIGH);    
     // digitalWrite(LED_L, HIGH);
     // digitalWrite(LED_R, HIGH);  
      //currentState= -1;
      delay(300);
   }

   //START
   if(digitalRead(BUTTON_4)==0) {
      currentState=6;
      SerialUSB.println("setting state to 0");
      starting=true; 
      
      //timer_active = true;
      delay(300);
   }
 
  switch(currentState) { 

    case -4: 
      displayState();
      turnOffLeftPump(); 
      turnOffRightPump();
      servo_ver_l.write(1500);
      servo_ver_r.write(1500);
      delay(50);
      digitalWrite(SOLENOID_L, LOW); //Close pressure release
      digitalWrite(SOLENOID_R, LOW); //Close pressure release
    break;

    case -3:
    break;
    
    case -2: 
          displayState();
            if (average_pressure_l > -20)  { 
              turnOnLeftPump();   
            }
            else turnOffLeftPump();

            if (average_pressure_r > -20) { 
              turnOnRightPump();
            }
            else turnOffRightPump();
      SerialUSB.println("Tryint to hold on...");
    break;
     
    case -1:
      displayState();
      digitalWrite(PUMP_L_C, HIGH);
      digitalWrite(PUMP_R_C, HIGH);
      digitalWrite(LED_L, HIGH);
      digitalWrite(LED_R, HIGH);
    break;
    
    case 0:
      displayState();
    break; 
    
    case 1:
      displayState();
      digitalWrite(PUMP_L_C, HIGH);
      digitalWrite(PUMP_R_C, HIGH);
      digitalWrite(LED_L, HIGH);
      digitalWrite(LED_R, HIGH);
      servo_hor_l.write(1000); //extend forward
      servo_hor_r.write(1000); //extend forward
      servo_ver_r.write(1000); //level left and right cups the same height
      servo_ver_l.write(1000); //level left and right cups the same height
      currentState=2;
    break;
    
    case 2:
      displayState();
      //wait for adhesion
      while((average_pressure_r >= RIGHT_ADHESION_PRESSURE) ) { 
          if(time_pressure_read) { 
            pressureRead();
            if (displayMoreData) SerialUSB.println(",,,looping right adhesion...");
          }
      }

       while( (average_pressure_l >= LEFT_ADHESION_PRESSURE)) { 
          if(time_pressure_read) { 
            pressureRead();
            if (displayMoreData) SerialUSB.println(",,,looping left adhesion...");
          }
      }
      //move to the next state
      currentState=3;   
    break;
    
    case 3:
      displayState();
    //keep right pump on and turn off the left one 
      digitalWrite(PUMP_L_C, LOW);
      digitalWrite(PUMP_R_C, HIGH);
      digitalWrite(SOLENOID_L, HIGH); //Open pressure release
      digitalWrite(LED_L, LOW);
      digitalWrite(LED_R, HIGH);
      currentState =4;
    break; 
    
   case 4:  
      displayState(); 
      //wait to realise left adhesion
      while( (average_pressure_l <= LEFT_RELEASE_PRESSURE)) { 
          if(time_pressure_read) { 
            pressureRead();
            if (displayMoreData) SerialUSB.println(",,,looping left release...");
          }
      }  
      currentState =5;
   break;
   
   case 5:
      displayState();
      //lift the robot up
      servo_ver_r.write(1000);
      servo_ver_l.write(1600);
      delay(50);
      currentVerticalLeft = 1600;
      currentVerticalRight = 1000;
      //pull the robot forward by retracting
      servo_hor_r.write(1800); //retract 
      currentState = 6;
    break;
    
    case 6:
      displayState();
      turnOnLeftPump(); 
      //turnOnRightPump();
      digitalWrite(SOLENOID_L, LOW); //Close pressure release
      currentState = 7; 
      if(delayRobotDebug) delay(3000);
    break;

    case 7: 
      //left attachment
      displayState();
      currentVerticalLeft = currentVerticalLeft-100;
      if (currentVerticalLeft<1000) { 
        currentVerticalLeft=1500;
        currentVerticalRight = currentVerticalRight+100;
      }

      if (currentVerticalRight>1500) { 
        currentVerticalRight = 1000;
      }


      servo_ver_l.write(currentVerticalLeft);
      servo_ver_r.write(currentVerticalRight);
      if (displayMoreData) SerialUSB.print("changing vertical left to: ");
      if (displayMoreData) SerialUSB.println(currentVerticalLeft);

      if(currentState==(-4))break;   
      else currentState = 8; 
      
    break;
      
    case 8: 
      displayState(); 
      turnOnLeftPump(); 
      //turnOnRightPump();

          if(currentState==(-4)){ 
        break; 
      }
      
      currentMillisTime = millis();
      while ((currentMillisTime+1000)>millis()) { 
        if(time_pressure_read) { 
            pressureRead();
            if(displayMoreData) SerialUSB.println(",,,looping left adhesion...");
        } //end if 
      }//end while

  

      if (average_pressure_l <= LEFT_ADHESION_PRESSURE) { 
        if(currentState==(-4))break;   
        else currentState = 9;  
      }
      else { 
        if(currentState==(-4))break;   
        else currentState =7;
      }
    break; 
     
    case 9:
      displayState(); 
      leftPumpHolding = true;
      //turnOnLeftPump(); 
      turnOffRightPump();
      rightPumpHolding = false;
      digitalWrite(SOLENOID_R, HIGH); //Open pressure release
      currentState = 10;
      if(delayRobotDebug) delay(3000);
    break;  

    case 10: 
      displayState();
      while( (average_pressure_r <= RIGHT_RELEASE_PRESSURE)) { 
          if(time_pressure_read) { 
              pressureRead();
              if(displayMoreData) SerialUSB.println(",,,looping right release...");
          }
      }  
      currentState =11;
      if(delayRobotDebug) delay(3000);
    break;

    //pull up with the left leg
    case 11: 
      displayState();
      servo_ver_l.write(900); 
      delay(300);
      servo_hor_l.write(2000); //retract
      delay(300);

      //Rotation test
      analogWrite(MOTOR_PWM,motorSpeed);
      if (rotationDirection==1) { 
        digitalWrite(MOTOR_DIR, LOW);
      }
      else if (rotationDirection==0) { 
        digitalWrite(MOTOR_DIR, HIGH);
      }
      delay(rotationMagnitude); 
      analogWrite(MOTOR_PWM,0);
      
      currentState = 12;   
      if(delayRobotDebug) delay(3000);
    break;

    case 12:
      displayState();

      servo_hor_r.write(2000);   
      
      delay(300);
      servo_ver_r.write(1500);
      delay(300);
      currentVerticalRight = 1500;
      currentState = 13;
      if(delayRobotDebug) delay(3000);
    break; 

    case 13: 
      displayState();
     // turnOnLeftPump();
      turnOnRightPump();
      digitalWrite(SOLENOID_R, LOW); //close pressure release
      currentState = 14;
      if(delayRobotDebug) delay(3000);
    break;

    case 14:
      displayState();
      //looping right attachment
      currentVerticalRight = currentVerticalRight-100;
      if (currentVerticalRight<1000) { 
        currentVerticalRight=1500;
        currentVerticalLeft = currentVerticalLeft+100;
      }

      if (currentVerticalLeft>1600) { 
        currentVerticalLeft = 1000;
      }
      servo_ver_r.write(currentVerticalRight);
      servo_ver_l.write(currentVerticalLeft);
      if(displayMoreData)SerialUSB.print(",,,changing vertical right to: ");
      if(displayMoreData)SerialUSB.println(currentVerticalRight);
      if(displayMoreData)SerialUSB.print(",,,changing vertical left to: ");
      if(displayMoreData)SerialUSB.println(currentVerticalLeft);

      if(currentState==(-4))break;   
      else currentState = 15;
    break; 

    case 15: 
      displayState();    
      currentMillisTime = millis();


      
      while ((currentMillisTime+1000)>millis()) { 
        if(time_pressure_read) { 
            pressureRead();
            //SerialUSB.println("looping right adhesion...");
        } //end if 
      }//end while

      if (average_pressure_r <= RIGHT_ADHESION_PRESSURE) { 
  
        if(currentState==(-4))break;   
        else currentState = 16;  
      }
      else { 
        if(currentState==(-4))break;   
        else  currentState =14;
      }
    break;

    case 16: 
      testSkin();
      displayState(); 
      turnOffLeftPump(); 
      leftPumpHolding = false;
      turnOnRightPump();
      rightPumpHolding = true;
      digitalWrite(SOLENOID_L, HIGH); //open pressure release
      currentState = 17;
      if(delayRobotDebug) delay(3000);
    break;

    case 17:
      displayState();
      while( (average_pressure_l <= LEFT_RELEASE_PRESSURE)) { 
          if(time_pressure_read) { 
              pressureRead();
              if(displayMoreData)SerialUSB.println(",,,looping left release...");
          }
      }  
      currentState =18; 
    break;

    case 18: 
      displayState();
      servo_ver_r.write(1000); 
      delay(500);

      analogWrite(MOTOR_PWM,motorSpeed);
      if (rotationDirection==1) { 
        digitalWrite(MOTOR_DIR, HIGH);
      }
      else if (rotationDirection==0) { 
        digitalWrite(MOTOR_DIR, LOW);
      }
      delay(rotationMagnitude); 
      analogWrite(MOTOR_PWM,0);
      

      servo_hor_r.write(800);
      delay(500);
      
      currentState = 19;   
    break;

    case 19: 
      displayState();
      servo_hor_l.write(800); 
      delay(300);
      servo_ver_l.write(1500);
      delay(300);
      currentVerticalLeft = 1500;
      currentState = 20;
      if(delayRobotDebug) delay(3000);
    break;

    case 20: 
        displayState();
        currentState = 6; 
        if(delayRobotDebug) delay(3000);
    break;
  }
  

  if(time_pressure_read) { 
      pressureRead();
  }

  if(leftPumpHolding) { 
    if (average_pressure_l > -23)  { 
        turnOnLeftPump();   
    }
    else turnOffLeftPump();
  }

  if(rightPumpHolding){ 
     if (average_pressure_r > - 23) { 
        turnOnRightPump();
     }
     else turnOffRightPump();
  }
}

void testSkin() {

  int timePlunge = 0; 
  int timeReturn = 0;
  servo_ver_l.write(900);
  servo_ver_r.write(900);
  turnOnLeftPump();
  turnOnRightPump();
  delay(1000);
  servo_ver_l.write(1400);
  servo_ver_r.write(1400);
  delay(1000);
  
SerialUSB.println("-----PLUNGE TEST-----");
  //timeStartingPlungle = millis();
     for (int i=1500; i>1000 ; i--) { 
           servo_ver_stiff.write(i); //about 1500 is the max inside
           delay(3);
           SerialUSB.print(analogRead(SERVO_VERT_POS));
           SerialUSB.print(" ,");
           SerialUSB.print(i);
           SerialUSB.print(" ,");
           SerialUSB.print(analogRead(FSR_SENSOR)); 
           SerialUSB.print(" ,");
           SerialUSB.println(millis());
      }

SerialUSB.println("-----RETURN TEST-----");
      for (int i=1000; i<1500 ; i++) { 
            servo_ver_stiff.write(i); //about 1500 is the max inside
            delay(15);
            SerialUSB.print(analogRead(SERVO_VERT_POS));
            SerialUSB.print(" ,");
            SerialUSB.print(i);
            SerialUSB.print(" ,");
            SerialUSB.print(analogRead(FSR_SENSOR)); 
            SerialUSB.print(" ,");
            SerialUSB.println(millis());
      }
        turnOffLeftPump();
        turnOffRightPump();
          servo_ver_l.write(900);
          servo_ver_r.write(900);
          delay(1000);
  
}

void pressureRead() { 
      float sensorValue1 = analogRead(LEFT_PRESSURE);  
    // print out the value you read:
    float voltage = sensorValue1 * (3.336 / ADC_BITS);
    float voltageIn5Vrange = voltage * 2.0;
    pressure_l = ((voltageIn5Vrange/SUPPLY_VOLTAGE)-0.92)/0.007652; //in KPa
    float pressureError = 1.725*0.00762*SUPPLY_VOLTAGE;
    calculateAverageLeftSensor();
  
    //SerialUSB.print(average_pressure_l); //inKpa
    //SerialUSB.print(","); //inKpa
    //SerialUSB.println(pressure_l); //inKpa
    
    float sensorValue2 = analogRead(RIGHT_PRESSURE);  
    voltage = sensorValue2 * (3.336 / ADC_BITS);
    voltageIn5Vrange = voltage * 2.0;
    pressure_r = ((voltageIn5Vrange/SUPPLY_VOLTAGE)-0.92)/0.007652; //in KPa
    pressureError = 1.725*0.00762*SUPPLY_VOLTAGE;
    calculateAverageRightSensor();

    if (displayPressureData){
      SerialUSB.print("$,");
      SerialUSB.print(average_pressure_l); //inKpa
      SerialUSB.print(","); //inKpa
      SerialUSB.print(average_pressure_r); //inKpa
      SerialUSB.print(","); //inKpa
      SerialUSB.print(average_pressure_l*500); //inKpa
      SerialUSB.print(","); //inKpa
      SerialUSB.print(average_pressure_r*500); //inKpa

      SerialUSB.print(","); 
      SerialUSB.print(currentState);
      SerialUSB.print(",");
      SerialUSB.print(millis());
      SerialUSB.print(",");
      SerialUSB.println(analogRead(FSR_SENSOR));
    }




    time_pressure_read = false;
    //SerialUSB.println(digitalRead(BUTTON_1));
    //SerialUSB.println(digitalRead(BUTTON_2));
    //SerialUSB.println(digitalRead(BUTTON_3));
    //SerialUSB.println(digitalRead(BUTTON_4));
  
}

void displayState() {
    #ifdef DISPLAY
     // display.clearDisplay(); 
     // display.setTextSize(5);
     // display.setCursor(0,0);
     // display.println(currentState);
     // display.setTextSize(1);
     // display.print(average_pressure_l);
     // display.setCursor(30,20);
     // display.display();
    #endif
  
}

void turnOnLeftPump(){ 
   digitalWrite(PUMP_L_C, HIGH);
   digitalWrite(LED_L, HIGH);
   SerialUSB.println("turing left pump on");
}

void turnOffLeftPump() { 
   digitalWrite(PUMP_L_C, LOW);
   digitalWrite(LED_L, LOW);
  
}

void turnOnRightPump() {
   digitalWrite(PUMP_R_C, HIGH);
   digitalWrite(LED_R, HIGH);
   SerialUSB.println("turing right pump on");

}

void turnOffRightPump(){ 
   digitalWrite(PUMP_R_C, LOW);
   digitalWrite(LED_R, LOW);
  
}

void calculateAverageLeftSensor() { 
  total_l = total_l - readings_l[readIndex_l];
  readings_l[readIndex_l] = pressure_l;
  total_l = total_l + readings_l[readIndex_l];
  readIndex_l = readIndex_l + 1;
  if (readIndex_l >= numReadings) {
    readIndex_l = 0;
  }
  average_pressure_l = (total_l / (float) numReadings);
}

void calculateAverageRightSensor() { 
  total_r = total_r - readings_r[readIndex_r];
  readings_r[readIndex_r] = pressure_r;
  total_r = total_r + readings_r[readIndex_r];
  readIndex_r = readIndex_r + 1;
  if (readIndex_r >= numReadings) {
    readIndex_r = 0;
  }
  average_pressure_r = (total_r / (float) numReadings);
}

void keepAdhesionLeft() { 
  
}

void stopAdhesionLeft() { 
  
}

void keepAdhesionRight() { 
}

void stopAdhesionRight() { 
}



