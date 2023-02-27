
This is the repository of BTTF Delorean Ultimate Board.

Those sample code are shared to help you to implement additional extensibility for your Delorean Model.

They have been developped to work with our Ultimate Board.

Ultimate Board instruction Manual is available on this link : https://tinyurl.com/UltimateBoard

More information on Ultimate Board can be found here : https://www.facebook.com/groups/3DynamicsDesigns/permalink/1872951202854355/

Ultimate Board in Action video : https://www.youtube.com/watch?v=_GSnsW_yHhA&ab_channel=AdrianDinca

Instruction to upload code into Arduino can be found there : https://www.arduino.cc/en/Guide/Environment#uploading

Instruction to install Arduino library can be found there (We recommand using library manager): https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries

Information about Arduino Leonardo : https://docs.arduino.cc/retired/getting-started-guides/ArduinoLeonardoMicro

Useful link that helps troubleshooting arduino issue : https://www.instructables.com/5-Most-Common-Arduino-Nano-Clone-Problems-and-Thei/

## Arduino UI Configuration
Serial Monitor configuration must be set to 115200 baud to be able to see debug logs on the UI.

For Arduino Leonardo only (with custom dashboard electronic board):
 - For Normal mode, DEBUG variable must be commented when board is plugged only to the car. 
 - For debuging mode, DEBUG variable must be uncommented when mini usb is plugged to the computer //#define DEBUG 1.
 
DEBUG variable must be commented in car usage or arduino will start with 3s delay and fails to join Mainboard I2C Bus.


## Ultimate board and Extensibility Arduino Power Wiring
Do not forget to power both ultimate board and Extensibility Arduino as specified in the following schema.

If you forget to power ultimate board or extensibility ardunio then you may have issue with IC2 Communication.

The result will fail arduino's communication and keep Flux Capacitor in infinite loop without fluxing when required.


## Led & Resistor
In order to protect leds it is required to use resistor to lower current that the led get from the power source.
Depending on the led color and the led specification you need to adjust the resistor value.

We recommand using for 5V with 20mA the following resistor to protect your leds :
- Red : 220 Ohm
- Orange : 220 Ohm
- Green : 180 Ohm
- Blue : 120 Ohm
- White : 120 Ohm

On Arduino we can use PWM pin instead of resistor to lower current that led received to protect them.

On the following diagram led marked with '*' are placed on PWM pin and code use this feature.

Please check the forward voltage of the led you are using to adjust the resistor value or pwm value.


# Delorean Dashboard Extensibility
PWM value are set to deliver the following voltage :
- Red : 1,96V
- Green : 2,05V
- Blue : 2.45V

This Extensibility allows to sync Ultimate Board with Dashboard Modification.

### Delorean Dashboard Extensibility - Arduino Nano DIY
![Alt text](./DashBoard_bb.png?raw=true "Delorean Dashboard Extensibility Wiring")


### Delorean Dashboard Extensibility - Custom Arduino Pro Micro Board
![Alt text](./DashBoard-ProMicro_bb.png?raw=true "Delorean Dashboard Extensibility Board Wiring")


# Flames and Hover Extensibility
This Extensibility allows to sync Ultimate Board with Led Strip Flames and delorean hovermod.

This extensibility requires additional arduino libraries as :
- Adafruit NeoPixel (https://github.com/adafruit/Adafruit_NeoPixel)
- FastLED (https://github.com/FastLED/FastLED)

![Alt text](./Flames&Wheels_bb.png?raw=true "Flames and Hover Extensibility Wiring")
