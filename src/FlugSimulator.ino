// http://blog.vinu.co.in/2012/07/tv-remote-controlled-computer-mouse-avr.html
// http://eleccelerator.com/tutorial-about-usb-hid-report-descriptors/

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>	/* for sei() */
#include <util/delay.h>		/* for _delay_ms() */
#include <avr/pgmspace.h>	/* required by usbdrv.h */
#include "usbdrv.h"

/* USB report descriptor, size must match usbconfig.h * /
PROGMEM char usbHidReportDescriptor[30] = {
  0x05, 0x01,		// USAGE_PAGE (Generic Desktop)
  0x09, 0x02,		// USAGE (Mouse)
  0xa1, 0x01,		// COLLECTION (Application)
  0x09, 0x01,		//   USAGE (Pointer)
  0xA1, 0x00,		//   COLLECTION (Physical)
  0x05, 0x01,		//     USAGE_PAGE (Generic Desktop)
  0x09, 0x30,		//     USAGE (X)
  0x09, 0x31,		//     USAGE (Y)
  0x09, 0x38,		//     USAGE (Wheel)
  0x15, 0x81,		//     LOGICAL_MINIMUM (-127)
  0x25, 0x7F,		//     LOGICAL_MAXIMUM (127)
  0x75, 0x08,		//     REPORT_SIZE (8)
  0x95, 0x03,		//     REPORT_COUNT (1)
  0x81, 0x06,		//     INPUT (Data,Var,Rel)
  0xC0,			//   END_COLLECTION
  0xC0,			// END COLLECTION
};
/* */

PROGMEM char usbHidReportDescriptor[46] = {
  0x05, 0x01,		// USAGE_PAGE (Generic Desktop)
  0x09, 0x04,		// USAGE (Joystick)
//  0x09, 0x05,		// USAGE (Gamepad)
  0xa1, 0x01,		// COLLECTION (Application)
  0xA1, 0x00,		//   COLLECTION (Physical)
  0x05, 0x09,           //     USAGE_PAGE (Button)
  0x19, 0x01,           //     USAGE_MINIMUM (Button 1)
  0x29, 0x03,           //     USAGE_MAXIMUM (Button 3)
  0x15, 0x00,           //     LOGICAL_MINIMUM (0)
  0x25, 0x01,           //     LOGICAL_MAXIMUM (1)
  0x95, 0x03,           //     REPORT_COUNT (3)
  0x75, 0x01,           //     REPORT_SIZE (1)
  0x81, 0x02,           //     INPUT (Data,Var,Abs)
  0x95, 0x01,           //     REPORT_COUNT (1)
  0x75, 0x05,           //     REPORT_SIZE (5)
  0x81, 0x03,           //     INPUT (Cnst,Var,Abs)
  0x05, 0x01,		//     USAGE_PAGE (Generic Desktop)
  0x09, 0x38,		//     USAGE (Wheele)
  0x15, 0x81,		//     LOGICAL_MINIMUM (-127)
  0x25, 0x7F,		//     LOGICAL_MAXIMUM (127)
  0x75, 0x08,		//     REPORT_SIZE (8)
  0x95, 0x01,		//     REPORT_COUNT (1)
  0x81, 0x06,		//     INPUT (Data,Var,Rel)
  0xC0,			//   END_COLLECTION
  0xC0,			// END COLLECTION
};

typedef struct {
  uchar buttonMask;
  char dWheel;
} 
report_t;

static report_t reportBuffer;
static uchar idleRate; 		/* repeat rate for keyboards, never used for mice */

// constants won't change. They're used here to
// set pin numbers:
const int button1Pin = 6;     // the number of the pushbutton pin
const int button2Pin = 7;     // the number of the pushbutton pin

const int ledPin =  13;      // the number of the LED pin

// variables will change:
int button1State = 0;         // variable for reading the pushbutton status
int button2State = 0;         // variable for reading the pushbutton status
int ledState = HIGH;

void switchLed()
{
  if(ledState == HIGH)
    ledState = LOW;
  else
    ledState = HIGH;
  digitalWrite(ledPin, ledState);
}

void sendMouseWheele() {
  while (!usbInterruptIsReady()) {
    // Note: We wait until we can send keystroke
    //       so we know the previous keystroke was
    //       sent.
  }
  usbSetInterrupt((unsigned char *)&reportBuffer, sizeof(reportBuffer));
}

void setup() {
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);

  digitalWrite(ledPin, ledState);

  uchar i;
  TCCR1B |= (1 << WGM12) | (1 << CS10);
  volatile uint16_t VIRTUAL_TIMER = 20000;
  wdt_enable(WDTO_1S);
  usbInit();
  usbDeviceDisconnect();
  i = 0;
  while (--i) {
    wdt_reset();
    _delay_ms(1);
  }
  usbDeviceConnect();
  sei();
}

void loop()
{
  wdt_reset();
  usbPoll();

  button1State = digitalRead(button1Pin);
  if (button1State == LOW)
  {
    switchLed();
    reportBuffer.dWheel = 1;
    reportBuffer.buttonMask = 1;
    sendMouseWheele();
    while(button1State == LOW)
    {
      delay(10);
      button1State = digitalRead(button1Pin);
    }
    reportBuffer.dWheel = 0;
    reportBuffer.buttonMask = 0;
    sendMouseWheele();
  }

  button2State = digitalRead(button2Pin);
  if (button2State == LOW)
  {
    switchLed();
    reportBuffer.dWheel = -1;
    reportBuffer.buttonMask = 2;
    sendMouseWheele();
    while(button2State == LOW)
    {
      delay(10);
      button2State = digitalRead(button2Pin);
    }
    reportBuffer.dWheel = 0;
    reportBuffer.buttonMask = 0;
    sendMouseWheele();
  }
}

