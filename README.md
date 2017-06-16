# LEDcontrol
This Tizen native App provides you a way to control a RGB-LED ambient light made of Arduino + WS2812b (aka Neopixel) + Bluetooth Module (like HC-05 or HC-06).

Each Bluetooth SPP/RFCOM message sent has the following structure :

- Byte 0: N/A (for future use, currently always 0x00)
- Byte 1: mode (0x00 = change color, 0x01 change effect)
- Byte 2: red value (unsigned char) or effect number (unsigned char)
- Byte 3: green value (unsigned char)
- Byte 4: blue value (unsigned char)
- Byte 5: end-of-message char ';'

(This app was inspired by Michael Fennel's LED-control on Android. More about it [here](https://github.com/fennel-labs/LED-control) and [here](https://play.google.com/store/apps/details?id=com.fennel.ledcontrol))

Here is a sample Arduino sketch :

```
#include <WS2812FX.h>
#include <SoftwareSerial.h>

#define LED_COUNT 72
#define LED_PIN 9

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
SoftwareSerial btmodule(11, 10); // RX, TX

void setup() {
  btmodule.begin(19200);

  ws2812fx.init();
  ws2812fx.setBrightness(255);
  ws2812fx.setSpeed(500);
  ws2812fx.setColor(255, 0, 0);
  ws2812fx.setMode(FX_MODE_CHASE_RAINBOW);
  ws2812fx.start();
  Serial.begin(115200);
}

void loop() {
  // data to read ?
  if (btmodule.available()) {
    // yes, get the cmd
    String commande = btmodule.readStringUntil(';');
    // have we 5 bytes (without the ";") ?
    if(commande.length() == 5) {
      // yes, it's a valid cmd
      // change color ?
      if(commande.charAt(1) == 0x00) {
        // yes, setting color
        ws2812fx.setColor((unsigned byte)commande.charAt(2),
                        (unsigned byte)commande.charAt(3),
                        (unsigned byte)commande.charAt(4));
        Serial.print("RGB to :");
        Serial.print((unsigned byte)commande.charAt(2));
        Serial.print(" ");
        Serial.print((unsigned byte)commande.charAt(3));
        Serial.print(" ");
        Serial.print((unsigned byte)commande.charAt(4));
        Serial.println("");
      // no.change effect ?
      } else if(commande.charAt(1) == 0x01) {
        // yes, setting effect without changing color
        ws2812fx.setMode(commande.charAt(2));
        Serial.print("Effect set to :");
        Serial.println((unsigned byte)commande.charAt(2));
      }
    }
  } else {
    ws2812fx.service();
  }
}
```

