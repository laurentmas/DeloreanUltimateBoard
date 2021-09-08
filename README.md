
This is the repository of BTTF Delorean Ultimate Board.

Those sample code are shared to help you to implement additional extensibility for your Delorean Model.

They have been developped to work with our Ultimate Board.

Ultimate Board instruction Manual is available on this link : https://tinyurl.com/UltimateBoard

More information on Ultimate Board can be found here : https://www.facebook.com/groups/3DynamicsDesigns/permalink/1872951202854355/

Ultimate Board in Action video : https://www.youtube.com/watch?v=_GSnsW_yHhA&ab_channel=AdrianDinca

Instruction to upload code into Arduino can be found there : https://www.arduino.cc/en/Guide/Environment#uploading

## Arduino UI Configuration
Serial Monitor configuration must be set to 115200 baud to be able to see debug logs on the UI.

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

![Alt text](./DashBoard_bb.png?raw=true "Delorean Dashboard Extensibility Wiring")

# Flames and Hover Extensibility
This Extensibility allows to sync Ultimate Board with Led Strip Flames and delorean hovermod.

![Alt text](./Flames&Wheels_bb.png?raw=true "Flames and Hover Extensibility Wiring")