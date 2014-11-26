/*
  Wireless Testing - Remote Control
  As user inputs commands into terminal -> xBee, c-controller (containing this code) will
  receive the commands, which are WASD and transmit relative hex values to the m-controller
*/

#include <Wire.h>
#include <stdio.h>

int ID = 0x3;   //ID of the pod - 1, 2, 3
uint32_t incomingMsg;      // a variable to read incoming serial data into
const int frontSensor = 7; // front sensor
const int leftSensor = 5; // left sensor
const int rightSensor = 3; // right sensor
long front, left, right;  //used for output to serial - converted to inches
char bytes[11];

enum PODID{
  Pod1 = 0x01,
  Pod2 = 0x02,
  Pod3 = 0x03,
};

enum DIRECTION{
  FORWARD = 'w',
  LEFT    = 'a',
  RIGHT   = 'd',
  STOP    = 's',
};

enum FRONTSPEED{
  SLOW = 0x11,
  FAST = 0x13,
  HALT = 0x00,
};

enum MESSAGETYPE{
  MainHub_Path_InitialPathMsg   = 0x00,
  Pod_Path_ConfirmPathMsg       = 0x01,
  MainHub_Path_GoPathMsg        = 0x02,
  Pod_Path_ConfirmGoMsg         = 0x03,
  MainHub_Status_RequestStatus  = 0x04,
  Pod_Status_StatusInfo         = 0x05,
  EmergencyShutDown             = 0x0F,
};

FRONTSPEED front_speed;
DIRECTION direction = STOP;
MESSAGETYPE message_type;
PODID PodID;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  pinMode(frontSensor, INPUT);
  pinMode(leftSensor, INPUT);
  pinMode(rightSensor, INPUT);
  PodID = Pod1;
}

void updateMotors(){
  front = (pulseIn(frontSensor, HIGH))/146;
  right = (pulseIn(rightSensor, HIGH))/146;
  left  = (pulseIn(leftSensor, HIGH))/146;
  
  if(direction == STOP){
    Wire.beginTransmission(4);
    Wire.write(0x00);
    Wire.endTransmission();
  }

  else if (direction == FORWARD){  
    if(front <12 || right < 8 || left < 8 ){ // if object in front is less than 1 feet, then STOP
        //STAHP
        front_speed = HALT;
      }
    else if(front < 24 && front > 12){ // if between 1-2 feet, then go at MEDIUM speed
      // Slow slow slow
        front_speed = SLOW;
      }
    else{
        //go full speed ahead - well not really full speed
        front_speed = FAST;
      }
    Wire.beginTransmission(4);
    Wire.write(front_speed);
    Wire.endTransmission();
  }

  else if(direction == LEFT){
    Wire.beginTransmission(4);
    Wire.write(0x14);
    Wire.endTransmission();
  }

  else if(direction == RIGHT){
    Wire.beginTransmission(4);
    Wire.write(0x15);
    Wire.endTransmission();
  }
  else
    Serial.print("ERROR\n");
}

void checkAndSet_Msg(uint32_t incomingMsg){
  //Check receiver and sender bits
  Serial.print(incomingMsg);
  if ((incomingMsg>>28) & 0xC == 0xC){  // check first two bits for receiver ID - if it matches PodID then execute.
    Serial.print("HitHitHit");
    if((incomingMsg>>24) & 0x0F == MainHub_Path_InitialPathMsg){  // initial path message coming from main hub
      direction = FORWARD;
      Serial.print("Print 1");
    }
    if((incomingMsg>>24) & 0x0F == MainHub_Path_GoPathMsg){       // "GO" path message coming from main hub
      direction = RIGHT;
      Serial.print("Print 2");
    }
    if((incomingMsg>>24) & 0x0F == MainHub_Status_RequestStatus){ // "Request Status" message coming from main hub
      direction = LEFT;
      Serial.print("Print 3");
    }
    if((incomingMsg>>24) & 0x0F == EmergencyShutDown){    // EmergencyShutDown
      Serial.print("Emergency Shut Down!");
      direction = STOP;
    }
  }
}

void loop()
{
  updateMotors();
  
  Serial.flush();
  if (Serial.available() > 0) { // Receive Serial Message
    Serial.readBytes(bytes, 10);
    
    
    
    incomingMsg =0;
    Serial.print("Hit\r\n");
  
    for (int i =0 ;i<11; i++){
      if (bytes[i]==NULL)
        break;
      else
        incomingMsg = (incomingMsg*10)+(bytes[i] - '0');
    }
    for (int i = 0; i<11; i++){
      bytes[i]=NULL;
    }
    Serial.print(incomingMsg,HEX);
    Serial.println();
    //checkAndSet_Msg(incomingMsg);
    /*
    if (incomingMsg == 's') {    // stop
      direction = STOP;
    } 
    if (incomingMsg == 'w') {    // go forward at slow speed
      direction = FORWARD;
    }
    if (incomingMsg == 'a') {    // turn left
    	direction = LEFT;
    }
    if (incomingMsg == 'd') {    // turn right
    	direction = RIGHT;
    }
    */
  }
}