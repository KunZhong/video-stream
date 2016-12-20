/*
* LED test program
*/
#include <core.h>
int button_pin = 7;
int buttonState = 0;
int switchChanged = 0;
void setup()
{
    pinMode(button_pin, INPUT);
}
void loop()
{
  buttonState = digitalRead(button_pin);  // set the LED on
  if(buttonState == HIGH && switchChanged == 0)
  {
		printf("button up!\n");
  		switchChanged = 1;
  }
  else if(buttonState == LOW && switchChanged == 1)
  {

		printf("button down!\n");
		switchChanged = 0;
  }
}
