/*
  DeAGOSTINI Back To The Furture Delorean Tire flame effect Control V2.00 By F²LAG Team (Original version by LsKaz)
  Customize Fire2012

    //Inputs
    Connect Arduino A04 to Mainboard SCL Pin
    Connect Arduino A05 to Mainboard SDA Pin

    //Outputs
    Connect Arduino D02 to WS2812 RGB strip LED signal in.
    Connect Arduino D07 to WS2812 RGB 12 RING LED signal in. LEFT
    Connect Arduino D08 to WS2812 RGB 12 RING LED signal in. RIGHT
    Connect Arduino D05 to Hover Mode Front LEDS in.
    Connect Arduino D06 to Hover Mode Rear LEDS in.
    Connect Arduino D09 to Hover Mode Central Front LEDS.
    Connect Arduino D10 to Hover Mode Central Centrer LEDS.
    Connect Arduino D11 to Hover Mode Central Rear LEDS.
*/

#include <Adafruit_NeoPixel.h>
#define NUM_OF_LED  12
#define BRIGHTNESS_MAX 255
#define BRIGHTNESS_MIN 2
#define CONTROL_PIN1 8 //RIGHT WHELL
#define CONTROL_PIN2 7 //LEFT WHEEL
#define MODE_H_INTERVAL 30 // ms
#define MODE_1_INTERVAL 70 // ms
#define MODE_2_INTERVAL 10 // ms
//#define FASTLED_ALLOW_INTERRUPTS 0
//#define FASTLED_INTERRUPT_RETRY_COUNT 0

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_OF_LED, CONTROL_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUM_OF_LED, CONTROL_PIN2, NEO_GRB + NEO_KHZ800);
unsigned long last_control_time = 0;
int r = 0;
int g = 0;
int b = 0;
int R_BR = BRIGHTNESS_MIN;
int R_INT = MODE_1_INTERVAL;
uint8_t led_index = 0;
uint8_t led_index2 = 0;
uint8_t i1 = 0;
uint8_t i2 = 0;
uint8_t i3 = 0;
uint8_t i4 = 0;


#include <FastLED.h>
#include <Wire.h>
int BR_DEF = 0;
int BR_CNT = 0;
int BR_MAX = 1;
int FluxC_Lv  = 0;
#define COLOR_ORDER GRB
#define CHIPSET     WS2812 // ← Set Chipset TYPE
#define LED_PIN     2      // ← Set LED signal data pin 
#define NUM_LEDS    72     // ← Set Number of LEDs 67
#define NUM_Tire    36     // ← Set Rear Tire Position
#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 60

#define HOVER_MOD_FRONT 5 //PWM D05
#define HOVER_MOD_BACK 6 //PWM D06
#define HOVER_MOD_CENTRAL_1 9 //PWM D09
#define HOVER_MOD_CENTRAL_2 10 //PWM D10
#define HOVER_MOD_CENTRAL_3 11 //PWM D11

//I2C Infos
#define FLAMES_HOVER_MOD_OFF 0 //reset
#define FLAMES_ON 1
#define FLAMES_OFF 2
#define HOVER_MODE_OFF 3
#define FLAMES_ON_HOVER_MODE_OFF 4
#define FLAMES_OFF_HOVER_MODE_OFF 5 //all Off
#define HOVER_MODE_ON 6
#define FLAMES_ON_HOVER_MODE_ON 7 //All On
#define FLAMES_OFF_HOVER_MODE_ON 8
#define MUSEUM_ON 9
#define MUSEUM_OFF 10
#define MUSEUM_ON_HOVER_MODE_OFF 12
#define MUSEUM_OFF_HOVER_MODE_OFF 13
#define MUSEUM_ON_HOVER_MODE_ON 15
#define MUSEUM_OFF_HOVER_MODE_ON 16

unsigned long hovermodLastTime = 0;
int hovermodWait = 80;

//byte hover_central_values [2] = {40,80,120};
byte hover_central_step = 37;
byte hover_central_mid = hover_central_step*2;
byte hover_central_max = 111;
byte hover_central_current [3] = {hover_central_max,hover_central_mid,hover_central_step};

bool flames_mode_activated=false;
bool hover_mode_activated=false;

#define DEBUG 1
#define I2C_ADDRESS 9
byte SW_LED_SEQ;

int messageSize = 0;

bool gReverseDirection = false;
CRGB leds[NUM_LEDS];

///Activate Flames when Musuem mode is on (infinity loop)?
bool museumFlames = false;

void setup() {

  Serial.begin(115200);

  pixels.begin();
  pixels.show(); // Initialize all pixels to 'off'
  pixels2.begin();
  pixels2.show(); // Initialize all pixels to 'off'

  pinMode(HOVER_MOD_FRONT, OUTPUT);
  pinMode(HOVER_MOD_BACK, OUTPUT);
  pinMode(HOVER_MOD_CENTRAL_1, OUTPUT);
  pinMode(HOVER_MOD_CENTRAL_2, OUTPUT);
  pinMode(HOVER_MOD_CENTRAL_3, OUTPUT);
  
  delay(1000); // sanity delay

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

  FastLED.setBrightness( BR_DEF );

  Wire.begin(I2C_ADDRESS);                // join i2c bus with address #9
  Wire.setWireTimeout(5000,true); //concider wire transmission as timeout if communication is longer thant 5s
  Wire.onReceive(receiveEvent); // register event
  
  randomSeed(analogRead(0));
  Serial.println("Flames & Hover Mode Extension System Started - 2021 F²LAG Team ™");
}



void loop()
{
  if(messageSize)//check if message as been sent
  {
    #ifdef DEBUG
      Serial.print(F("Led Seq : "));
      Serial.print(SW_LED_SEQ);
      Serial.println();
    #endif
    messageSize=0;
  }

  //Serial.println(BR_DEF);
  
  if ((SW_LED_SEQ==1) || (SW_LED_SEQ==4) || (SW_LED_SEQ==7) || (((SW_LED_SEQ==9) || (SW_LED_SEQ==12) || (SW_LED_SEQ==15))&&museumFlames)) {
     FluxC_Lv = 8;
     BR_DEF=BRIGHTNESS;
     R_BR = BRIGHTNESS_MAX;
     R_INT = MODE_2_INTERVAL;
     flames_mode_activated=true;
     Serial.println("Flames Activated");
  } else {
    FluxC_Lv = 0;
    flames_mode_activated=false;
    }

  if ((SW_LED_SEQ==6) || (SW_LED_SEQ==7) || (SW_LED_SEQ==8) || (SW_LED_SEQ==15) || (SW_LED_SEQ==16)) {    
    hover_mode_activated=true;
    analogWrite(HOVER_MOD_FRONT,hover_central_max);
    analogWrite(HOVER_MOD_BACK,hover_central_max);
    if(millis() - hovermodLastTime >= hovermodWait)
    { 
      hovermodLastTime=millis();
      analogWrite(HOVER_MOD_CENTRAL_1,hover_central_current[0]);
      analogWrite(HOVER_MOD_CENTRAL_2,hover_central_current[1]);
      analogWrite(HOVER_MOD_CENTRAL_3,hover_central_current[2]);
      hover_central_current[0]=next_value(hover_central_current[0]);
      hover_central_current[1]=next_value(hover_central_current[1]);
      hover_central_current[2]=next_value(hover_central_current[2]);
      
    }
     
    
  } else {
    analogWrite(HOVER_MOD_FRONT,0);
    analogWrite(HOVER_MOD_BACK,0);
    analogWrite(HOVER_MOD_CENTRAL_1,0);
    analogWrite(HOVER_MOD_CENTRAL_2,0);
    analogWrite(HOVER_MOD_CENTRAL_3,0);
    hover_central_current[0]=hover_central_max;
    hover_central_current[1]=hover_central_mid;
    hover_central_current[2]=hover_central_step;
    hover_mode_activated=false;
  }

  //Serial.println(BR_DEF);
  ring_loop();//activate ring loop
  
//  if (R_BR > BRIGHTNESS_MIN){R_BR--;} else {R_BR=BRIGHTNESS_MIN;}
  
  Fire2012(); // run simulation frame
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  if (BR_CNT++ > BR_MAX){
     BR_CNT=0;
     if (FluxC_Lv!=7 && FluxC_Lv!=8) {
        if (BR_DEF < 40) {BR_DEF=BR_DEF-1;BR_MAX=8;} else {BR_DEF=BR_DEF-20;BR_MAX=1;} 
        if (BR_DEF > 10 && BR_DEF < 40 && random8(10) == 1) {BR_DEF=BR_DEF+random8(10);}
     } 
     if (BR_DEF<=0){BR_DEF=0;}
//     Serial.println(BR_DEF);

     FastLED.setBrightness( BR_DEF );
  }

}

byte next_value(byte current)
{
  if(current>hover_central_max)
  {
    return 0;
  } else {
    return (current+hover_central_step);
  }
}

// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 

#define COOLING  80

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.

#define SPARKING 90


void Fire2012()
{
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(6);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }
    if( random8() < SPARKING ) {
      int y = NUM_Tire + random8(6);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

  // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  int i=0;
  while(0 < Wire.available())       
  {
    SW_LED_SEQ = Wire.read();//get message sent from master   
  }
  //Serial.println();
  messageSize=howMany;
}

//--------------------------------------------------------------------
void ring_loop(){

  unsigned long now = millis();
    // 初回のみ、色を設定する - Set the color only for the first time
     R_BR = BR_DEF;
     
    if(now - last_control_time > R_INT)
    {
      if (hover_mode_activated) //check if hover mode is activated set static white color
      {
          r = 255;
          g = 160;
          b = 40;

          for(uint8_t i=0;i<NUM_OF_LED;i++)
          {
            pixels.setPixelColor(i, pixels.Color(r,g,b));
            pixels2.setPixelColor(i, pixels.Color(r,g,b));
          }

          pixels.setBrightness(BRIGHTNESS);
          pixels2.setBrightness(BRIGHTNESS);
       }
       else if(flames_mode_activated || R_BR!=0)
       {
         if (R_BR > 10)
         {
            r = 255;
            g = 160;
            b = 40;
            if (random(30)==1)
            {
               r = 255;
               g = 255;
               b = 255;
            }
              if (random(60)==1)
              {
               r = 50;
               g = 50;
               b = 255;
              }
         }
         else
         {
            r = 0;
            g = 0;
            b = 0;
         }
    
        //  if (R_BR > BRIGHTNESS_MIN){R_BR=R_BR - 5;}
        // if (R_BR < BRIGHTNESS_MIN){R_BR=BRIGHTNESS_MIN;}
          if (R_INT < MODE_1_INTERVAL){R_INT++;} else {R_INT=MODE_1_INTERVAL;}
          pixels.setBrightness(R_BR);
          pixels2.setBrightness(R_BR);
          
          for(uint8_t i=0;i<NUM_OF_LED;i++){
              pixels.setPixelColor(i, pixels.Color(0,0,0));
              pixels2.setPixelColor(i, pixels.Color(0,0,0));
          }
          i1 = led_index;
          i2 = led_index + 3;if (i2 >= NUM_OF_LED) {i2 = i2 - NUM_OF_LED;}
          i3 = led_index + 6;if (i3 >= NUM_OF_LED) {i3 = i3 - NUM_OF_LED;}
          i4 = led_index + 9;if (i4 >= NUM_OF_LED) {i4 = i4 - NUM_OF_LED;}
          pixels.setPixelColor(i1, pixels.Color(r,g,b));
          pixels.setPixelColor(i2, pixels.Color(r,g,b));
          pixels.setPixelColor(i3, pixels.Color(r,g,b));
          pixels.setPixelColor(i4, pixels.Color(r,g,b));
          
          led_index2 = NUM_OF_LED - led_index - 1;
          i1 = led_index2;
          i2 = led_index2 + 3;if (i2 >= NUM_OF_LED) {i2 = i2 - NUM_OF_LED;}
          i3 = led_index2 + 6;if (i3 >= NUM_OF_LED) {i3 = i3 - NUM_OF_LED;}
          i4 = led_index2 + 9;if (i4 >= NUM_OF_LED) {i4 = i4 - NUM_OF_LED;}
          pixels2.setPixelColor(i1, pixels.Color(r,g,b));
          pixels2.setPixelColor(i2, pixels.Color(r,g,b));
          pixels2.setPixelColor(i3, pixels.Color(r,g,b));
          pixels2.setPixelColor(i4, pixels.Color(r,g,b));
          
          led_index++;
          if(led_index >= NUM_OF_LED){
            led_index = 0;
          }
       } 
       else 
       {
          r = 0;
          g = 0;
          b = 0;
        
          for(uint8_t i=0;i<NUM_OF_LED;i++)
          {
            pixels.setPixelColor(i, pixels.Color(r,g,b));
            pixels2.setPixelColor(i, pixels.Color(r,g,b));
          }
      }

      pixels.show();
      pixels2.show();
 
      last_control_time = now;
    }
}
