#include <Wire.h>

//#define DEBUG 1 // DEBUG variable must be commented in car usage, or arduino will start with 3s latency and fails to join I2C Bus
//#define TRACE 1
#define I2C_ADDRESS 88

int SW_DASH_LED;

#define ArduinoNano  1
#define ArduinoProMicro 2

#if defined(__AVR_ATmega328P__)
  //Code in here will only be compiled if an Arduino Uno (or older) is used.
  #define Hardware ArduinoNano
  #define HardwareName "ArduinoNano"
  //arduino pin allocation
  #define HandBrake 2 //D02
  #define RightTurn 3 //D03 PWM
  #define SeatBelt 4 //D04
  #define LowBeam 5 //D05 PWM
  #define HighBeam 6 //D06 PWM
  #define Doors 7 //D07
  #define Lambda 8 //D08
  #define LeftTurn 9 //D09 PWM
  #define Oil 10 //D10 PWM
  #define Battery 11 //D11 PWM
  #define Fuel 12 //D12  
#endif

#if defined(__AVR_ATmega32U4__)
  //Code in here will only be compiled if an Arduino Leonardo is used.
  #define Hardware ArduinoProMicro
  #define HardwareName "ArduinoProMicro"
  //arduino pin allocation
  #define HandBrake 18 //A0
  #define RightTurn 15 //D15
  #define SeatBelt 14 //D14
  #define LowBeam 16 //D16
  #define HighBeam 10 //D10 PWM
  #define Doors 4 //D04
  #define Lambda 5 //D05 PWM
  #define LeftTurn 6 //D06 PWM
  #define Oil 7 //D07 
  #define Battery 8 //D08
  #define Fuel 9 //D09 PWM  
#endif

//I2C Actions Infos
#define ALL_OFF 1
#define IGNITION_ON 2
#define IGNITION_OFF 3
#define ENGINE_ON 4
#define ENGINE_RUN 5
#define ENGINE_OFF 6
#define DOOR_ON 7
#define DOOR_OFF 8
#define L_TURN_ON 9
#define L_TURN_OFF 10
#define R_TURN_ON 11
#define R_TURN_OFF 12
#define HAZARD_ON 13
#define HAZARD_OFF 14
#define H_BEAM_ON 15
#define H_BEAM_OFF 16
#define L_BEAM_ON 17
#define L_BEAM_OFF 18
#define HAND_BRAKE_ON 19
#define HAND_BRAKE_OFF 20
#define MUSEUM_ON 21
#define MUSEUM_OFF 22
#define FLICKER_START 23
#define FLICKER_ON 24
#define FLICKER_OFF 25
#define FUEL_ON 26
#define FUEL_OFF 27
#define LAMBDA_ON 28
#define LAMBDA_OFF 29

byte red_pwm = 100;
byte green_pwm = 105;
byte blue_pwm = 125;

//manage turn signal
unsigned long lastTurnBlink = 0;
int defaultTurnDelay = 555;
int turnDelay = defaultTurnDelay;
int turnSignalStatus = 0; //0=off,1=leftSignal,2=rightSignal,3=hazard

unsigned long startupMillis;
unsigned long seatBeltMillis;

int lowFuelTime = 3600000; //time in millis
int seatBeltTime = 6000;

int messageSize = 0;

bool usbConnected = false;

#define LEDMAX 11        
#if Hardware == ArduinoNano   //id     0    1    2    3    4    5    6    7    8    9   10                           
  byte     SW_PIN       [LEDMAX] = {   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12}; // LED OUTPUT PIN
#else
  byte     SW_PIN       [LEDMAX] = {  18,  15,  14,  16,  10,   4,   5,   6,   7,   8,   9}; // LED OUTPUT PIN
#endif
byte     SW_PWM_MAX   [LEDMAX] = { 255, 105, 255, 105, 125, 255, 255, 105, 100, 100, 255}; // LED OUTPUT PWM Value ; 255 = Digital
boolean  SW_LED       [LEDMAX] = {}; // LED
boolean  SW_PREV_LED  [LEDMAX] = {}; // LED

void setup() {

  unsigned int count=0;
  //#ifdef DEBUG    
    #if Hardware == ArduinoProMicro
      USBCON|=(1<<OTGPADE); //enables VBUS pad
      if ((USBSTA & (1 << VBUS)) != 0) //checks state of VBUS
      {  
        Serial.begin(115200);           // start serial for output
        while(!Serial && millis()<3000);
        if(Serial.availableForWrite())
        {
          usbConnected = true;
        }
      }
    #elif Hardware == ArduinoNano
      Serial.begin(115200);           // start serial for output
      usbConnected = true;
    #endif
  //#endif
  
  pinMode(SeatBelt, OUTPUT); //not used
  pinMode(Doors, OUTPUT); //linked to dome light
  pinMode(LeftTurn, OUTPUT); //linked to Left turn and warning
  pinMode(RightTurn, OUTPUT); //linked to Right turn and warning
  pinMode(HighBeam, OUTPUT); //linked to High Beam
  pinMode(LowBeam, OUTPUT); //linked to Low Beam
  pinMode(Lambda, OUTPUT); //not used
  pinMode(Oil, OUTPUT); //linked to engine startup
  pinMode(Battery, OUTPUT); //linked to engine startup
  pinMode(Fuel, OUTPUT); //randon long warning ?
  pinMode(HandBrake, OUTPUT);//linked to engine startup & run maybe

  Wire.begin(I2C_ADDRESS);                // join i2c bus with address #88
  Wire.setWireTimeout(5000, true); //concider wire transmission as timeout if communication is longer thant 5s
  Wire.clearWireTimeoutFlag();
  Wire.onReceive(receiveEvent); // register event

  startupMillis = millis();
  if(usbConnected)
  {
    Serial.print(F("Dashboard Extension System Started - "));
    Serial.print(HardwareName);
    Serial.println(" - 2021-2023 F²LAG Team ™");
  }

}

void loop() {

  if (messageSize) //check if message as been sent
  {
    #ifdef TRACE
        Serial.print(F("Dashboard Led Action : "));
        Serial.print(SW_DASH_LED);
        Serial.println();
    #endif

    messageSize = 0;

    if (startupMillis == 0) //if startup time was Finished and a new key is send then restart it
    {
      startupMillis = millis();
    }

    switch (SW_DASH_LED)
    {
      case ALL_OFF : //All Off
        stopAllLeds();
        lastTurnBlink = 0;
        turnSignalStatus = 0;
        seatBeltMillis = 0;
        startupMillis = 0;

        #ifdef DEBUG
                Serial.println("Stop All");
        #endif
        break;

      case IGNITION_ON : //ignition Key On
        SW_LED[getSWPos(Battery)] = true;
        #if Hardware == ArduinoNano
          analogWrite(Battery, red_pwm);
        #else
          digitalWrite(Battery, HIGH);
        #endif
        SW_LED[getSWPos(Oil)] = true;
        #if Hardware == ArduinoNano
          analogWrite(Oil, red_pwm);
        #else
          digitalWrite(Oil, HIGH);
        #endif
        SW_LED[getSWPos(HandBrake)] = true;
        digitalWrite(HandBrake, HIGH);
        SW_LED[getSWPos(SeatBelt)] = true;
        digitalWrite(SeatBelt, HIGH);

        #ifdef DEBUG
                Serial.println("Ignition On -> Battery On, Oil On, HandBrake On");
        #endif
        break;
      case IGNITION_OFF : //ignition Key Off
        SW_LED[getSWPos(Battery)] = false;
        digitalWrite(Battery, LOW);
        SW_LED[getSWPos(Oil)] = false;
        digitalWrite(Oil, LOW);
        SW_LED[getSWPos(HandBrake)] = false;
        digitalWrite(HandBrake, LOW);
        SW_LED[getSWPos(SeatBelt)] = false;
        digitalWrite(SeatBelt, LOW);
        seatBeltMillis = 0;
        startupMillis = 0;

        #ifdef DEBUG
                Serial.println("Ignition Off -> Battery Off, Oil Off, HandBrake Off");
        #endif
        break;

      case ENGINE_ON : //Engine Start
        delay(300);
        SW_LED[getSWPos(Battery)] = false;
        digitalWrite(Battery, LOW);
        SW_LED[getSWPos(Oil)] = false;
        digitalWrite(Oil, LOW);
        SW_LED[getSWPos(SeatBelt)] = true;
        digitalWrite(SeatBelt, HIGH);
        seatBeltMillis = millis();

        #ifdef DEBUG
                Serial.println((String)"Engine Start -> Battery Off, Oil Off, Seat Belt on for "+(6000/1000)+"s");
        #endif
        break;
      case ENGINE_RUN : //Engine Run
        SW_LED[getSWPos(Battery)] = false;
        digitalWrite(Battery, LOW);
        SW_LED[getSWPos(Oil)] = false;
        digitalWrite(Oil, LOW);
        SW_LED[getSWPos(HandBrake)] = false;
        digitalWrite(HandBrake, LOW);

        #ifdef DEBUG
                Serial.println("Engine Run -> HandBrake Off");
        #endif
        break;
      case ENGINE_OFF : //Engine Stop
        #ifdef DEBUG
                Serial.println("Engine Stop -> ...");
        #endif
        break;

      case DOOR_ON : //Door Open
        SW_LED[getSWPos(Doors)] = true;
        digitalWrite(Doors, HIGH);

        #ifdef DEBUG
                Serial.println("Door Opened");
        #endif
        break;
      case DOOR_OFF : //Door Closed
        //int doorId=;
        SW_LED[getSWPos(Doors)] = false;
        SW_PREV_LED[getSWPos(Doors)] = false;
        digitalWrite(Doors, LOW);

        #ifdef DEBUG
                Serial.println("Door Closed");
        #endif
        break;

      case L_TURN_ON : //Left Turn Signal On
        lastTurnBlink = 0;
        turnSignalStatus = 1; //0=off,1=leftSignal,2=rightSignal,3=hazard
        SW_LED[getSWPos(LeftTurn)] = true;
        #if Hardware == ArduinoNano
          analogWrite(LeftTurn, green_pwm);
        #else
          digitalWrite(LeftTurn, HIGH);
        #endif
        SW_LED[getSWPos(RightTurn)] = false;
        digitalWrite(RightTurn, LOW);

        #ifdef TRACE
                Serial.println("Left Turn Signal On");
        #endif
        break;
      case L_TURN_OFF : //Left Turn Signal Off
        SW_LED[getSWPos(LeftTurn)] = false;
        digitalWrite(LeftTurn, LOW);
        lastTurnBlink = 0;
        turnSignalStatus = 0; //0=off,1=leftSignal,2=rightSignal,3=hazard

        #ifdef TRACE
                Serial.println("Left Turn Signal Off");
        #endif
        break;
      case R_TURN_ON : //Right Turn Signal On
        lastTurnBlink = 0;
        turnSignalStatus = 2; //0=off,1=leftSignal,2=rightSignal,3=hazard
        SW_LED[getSWPos(RightTurn)] = true;
        #if Hardware == ArduinoNano
          analogWrite(RightTurn, green_pwm);
        #else
          digitalWrite(RightTurn, HIGH);
        #endif
        SW_LED[getSWPos(LeftTurn)] = false;
        digitalWrite(LeftTurn, LOW);

        #ifdef TRACE
                Serial.println("Right Turn Signal On");
        #endif
        break;
      case R_TURN_OFF : //Right Turn Signal Off
        lastTurnBlink = 0;
        turnSignalStatus = 0; //0=off,1=leftSignal,2=rightSignal,3=hazard
        SW_LED[getSWPos(RightTurn)] = false;
        digitalWrite(RightTurn, LOW);

        #ifdef TRACE
                Serial.println("Right Turn Signal Off");
        #endif
        break;
      case HAZARD_ON : //Hazard Signal On
        lastTurnBlink = 0;
        turnSignalStatus = 3; //0=off,1=leftSignal,2=rightSignal,3=hazard
        SW_LED[getSWPos(LeftTurn)] = true;
        #if Hardware == ArduinoNano
          analogWrite(LeftTurn, green_pwm);
        #else
          digitalWrite(LeftTurn, HIGH);
        #endif
        SW_LED[getSWPos(RightTurn)] = true;
        #if Hardware == ArduinoNano
          analogWrite(RightTurn, green_pwm);
        #else
          digitalWrite(RightTurn, HIGH);
        #endif

        #ifdef TRACE
                Serial.println("Warning/Hazard On");
        #endif
        break;
      case HAZARD_OFF : //Hazard Signal Off
        SW_LED[getSWPos(LeftTurn)] = false;
        digitalWrite(LeftTurn, LOW);
        SW_LED[getSWPos(RightTurn)] = false;
        digitalWrite(RightTurn, LOW);
        lastTurnBlink = 0;
        turnSignalStatus = 0; //0=off,1=leftSignal,2=rightSignal,3=hazard

        #ifdef TRACE
                Serial.println("Warning/Hazard Off");
        #endif
        break;

      case H_BEAM_ON : //High Beam ON
        SW_LED[getSWPos(HighBeam)] = true;
        #if Hardware == ArduinoNano
          analogWrite(HighBeam, blue_pwm);
        #else
          digitalWrite(HighBeam, HIGH);
        #endif

        #ifdef DEBUG
                Serial.println("High Beam ON");
        #endif
        break;
      case H_BEAM_OFF : //High Beam OFF
        SW_LED[getSWPos(HighBeam)] = false;
        digitalWrite(HighBeam, LOW);

        #ifdef DEBUG
                Serial.println("High Beam OFF");
        #endif
        break;
      case L_BEAM_ON : //Low Beam ON
        SW_LED[getSWPos(LowBeam)] = true;
        #if Hardware == ArduinoNano
          analogWrite(LowBeam, green_pwm);
        #else
          digitalWrite(LowBeam, HIGH);
        #endif

        #ifdef DEBUG
                Serial.println("Low Beam ON");
        #endif
        break;
      case L_BEAM_OFF : //Low Beam OFF
        SW_LED[getSWPos(LowBeam)] = false;
        digitalWrite(LowBeam, LOW);

        #ifdef DEBUG
                Serial.println("Low Beam OFF");
        #endif
        break;

      case HAND_BRAKE_ON : //HandBrake ON
        SW_LED[getSWPos(HandBrake)] = true;
        digitalWrite(HandBrake, HIGH);

        #ifdef DEBUG
                Serial.println("HandBrake ON");
        #endif
        break;
      case HAND_BRAKE_OFF : //HandBrake OFF
        SW_LED[getSWPos(HandBrake)] = false;
        digitalWrite(HandBrake, LOW);

        #ifdef DEBUG
                Serial.println("HandBrake OFF");
        #endif
        break;

      case MUSEUM_ON : //Museum ON
        SW_LED[getSWPos(LowBeam)] = true;
        #if Hardware == ArduinoNano
          analogWrite(LowBeam, green_pwm);
        #else
          digitalWrite(LowBeam, HIGH);
        #endif
        SW_LED[getSWPos(Battery)] = true;
        #if Hardware == ArduinoNano
          analogWrite(Battery, red_pwm);
        #else
          digitalWrite(Battery, HIGH);
        #endif
        SW_LED[getSWPos(Oil)] = true;
        #if Hardware == ArduinoNano
          analogWrite(Oil, red_pwm);
        #else
          digitalWrite(Oil, HIGH);
        #endif
        SW_LED[getSWPos(HandBrake)] = true;
        digitalWrite(HandBrake, HIGH);
        SW_LED[getSWPos(SeatBelt)] = false;
        digitalWrite(SeatBelt, LOW);
        seatBeltMillis = 0;
        startupMillis = 0;
        #ifdef DEBUG
                Serial.println("Museum ON : Low Beam On, Battery On, Oil On, HandBrake On");
        #endif
        break;
      case MUSEUM_OFF : //Museum OFF
        SW_LED[getSWPos(LowBeam)] = false;
        digitalWrite(LowBeam, LOW);
        SW_LED[getSWPos(Battery)] = false;
        digitalWrite(Battery, LOW);
        SW_LED[getSWPos(Oil)] = false;
        digitalWrite(Oil, LOW);;
        SW_LED[getSWPos(HandBrake)] = false;
        digitalWrite(HandBrake, LOW);
        #ifdef DEBUG
                Serial.println("Museum OFF : Low Beam Off, Battery Off, Oil Off, HandBrake Off, SeatBelt Off");
        #endif
        break;

      case FLICKER_START : //Flicker START
        for (int i = 0; i < LEDMAX; i++)
        {
          if (SW_LED[i])//if led is set as on
          {
            SW_PREV_LED[i] = true;
          }
          else
          {
            SW_PREV_LED[i] = false;
          }
        }

        #ifdef DEBUG
                Serial.println("Flicker START");
        #endif
        break;
      case FLICKER_ON : //Flicker ON
        SW_LED[getSWPos(HighBeam)] = false;
        digitalWrite(HighBeam, LOW);
        flickerAvailLeds(true);

        #ifdef TRACE
                Serial.println("Flicker ON");
        #endif
        break;
      case FLICKER_OFF : //Flicker OFF
        flickerAvailLeds(false);
        #ifdef TRACE
                Serial.println("Flicker OFF");
        #endif
        break;


        case FUEL_ON : //Fuel On
        SW_LED[getSWPos(Fuel)] = true;
        digitalWrite(Fuel, HIGH);

        #ifdef DEBUG
                Serial.println("Fuel On");
        #endif
        break;
      case FUEL_OFF : //Fuel Off
        SW_LED[getSWPos(Fuel)] = false;
        digitalWrite(Fuel, LOW);

        #ifdef DEBUG
                Serial.println("Fuel Off");
        #endif
        break;

        case LAMBDA_ON : //Lambda On
        SW_LED[getSWPos(Lambda)] = true;
        digitalWrite(Lambda, HIGH);

        #ifdef DEBUG
                Serial.println("Lambda On");
        #endif
        break;
      case LAMBDA_OFF : //Lambda Off
        SW_LED[getSWPos(Lambda)] = false;
        digitalWrite(Lambda, LOW);

        #ifdef DEBUG
                Serial.println("Lambda Off");
        #endif
        break;
        
    }
    SW_DASH_LED = 0;
  }

  //manage fuel tank led
  if ((startupMillis != 0) && (millis() - startupMillis >= lowFuelTime)) //start full tank is empty after 1h of running
  {
    SW_LED[getSWPos(Fuel)] = true;
    digitalWrite(Fuel, HIGH);
    startupMillis = 0;

    #ifdef DEBUG
        Serial.println("Empty Fuel ON");
    #endif
  }

  //manage seat belt alarm
  if ((seatBeltMillis != 0) && (millis() - seatBeltMillis >= seatBeltTime)) //stop seat belt alarm 7s after ingintion is started
  {
    SW_LED[getSWPos(SeatBelt)] = false;
    digitalWrite(SeatBelt, LOW);
    seatBeltMillis = 0;
    #ifdef DEBUG
        Serial.println("Seat Belt Alarm OFF");
    #endif
  }

}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  int i = 0;
  while (0 < Wire.available())
  {
    SW_DASH_LED = Wire.read();//get message sent from master
  }
  messageSize = howMany;
}

//stop all leds that are on
void stopAllLeds()
{
  for (int i = 0; i < LEDMAX; i++)
  {
    if (SW_LED[i])//if led is set as on
    {
      #ifdef TRACE
            Serial.println((String)"Stop Led Pin " + SW_PIN[i] + " [" + i + "]");
      #endif
      SW_LED[i] = false;//set led disabled
      int pinId = SW_PIN[i];
      digitalWrite(pinId, LOW);
    }
  }
  SW_DASH_LED = 0;
}

//flicker all leds that are on just before the sequence
void flickerAvailLeds(bool ledStatus)//ledStatus 0= Off; 1 = On
{
  int rightTurnPin = getSWPos(RightTurn);
  int leftTurnPin = getSWPos(LeftTurn);

  for (int i = 0; i < LEDMAX; i++)
  {
    if ((i != rightTurnPin) && (i != leftTurnPin)) //check if not turn signal
    {
      if (SW_PREV_LED[i] && ledStatus) //if led is set as on
      {
        SW_LED[i] = true;//set led enable
        int pinId = SW_PIN[i];        
        #if Hardware == ArduinoNano
          analogWrite(pinId, SW_PWM_MAX[i]);
        #else
          digitalWrite(pinId, HIGH);
        #endif
      }
      if (SW_LED[i] && !ledStatus)
      {
        //SW_PREV_LED[i]= true;//keep the info of previous status
        SW_LED[i] = false;//set led disable
        digitalWrite(SW_PIN[i], LOW);
      }
    }
  }
}

//get position on the pinId on the SW_PIN table
int getSWPos(int pinId)
{
  for (int i = 0; i < (sizeof(SW_PIN) / sizeof(SW_PIN[0])); i++) {
    //check if pinId is in SW Pin
    if (pinId == SW_PIN[i])
    {
      #ifdef TRACE
            Serial.println((String)"PinId : " + pinId);
            Serial.println((String)"SW Id : " + i);
      #endif
      return i;
    }
  }
}
