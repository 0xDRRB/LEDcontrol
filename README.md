# LEDcontrol
This Tizen native App provides you a way to control a RGB-LED ambient light made of Arduino + WS2812b (aka Neopixel) + Bluetooth Module (like HC-05 or HC-06).

Each Bluetooth SPP/RFCOM message sent has the following structure :

- Byte 0: N/A (for future use, currently always 0x00)
- Byte 1: mode (for future use, currently always 0x00, static color)
- Byte 2: red (unsigned char)
- Byte 3: green (unsigned char)
- Byte 4: blue (unsigned char)
- Byte 5: end-of-message char ';'

(This app was inspired by Michael Fennel's LED-control on Android. More about it [here](https://github.com/fennel-labs/LED-control) and [here](https://play.google.com/store/apps/details?id=com.fennel.ledcontrol))

Here is a sample Arduino sketch :

```
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel ws = Adafruit_NeoPixel(1, 9, NEO_RGB + NEO_KHZ800);

SoftwareSerial btmodule(10, 11); // RX, TX

void setup() {
  Serial.begin(115200);
  while (!Serial);

  btmodule.begin(19200);

  ws.begin();
  ws.setPixelColor(0, ws.Color(0,0,0));
  ws.show();
}

void loop() {
  if (btmodule.available()) {
    String commande = btmodule.readStringUntil(';');
    if(commande.length() == 5) {
      if(commande.charAt(1) == 0x00) {
        ws.setPixelColor(0, ws.Color(commande.charAt(2),commande.charAt(3),commande.charAt(4)));
        ws.show();
      }
    }
  }
}
```

