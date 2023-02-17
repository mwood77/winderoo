# User Manual


### Understanding Winderoo's Blinking Status

- Most ESP32 dev boards have a primary LED that is always on. This is typically a RED led. This cannot be shut off via firmware. 
    - If it bothers your, you can cover it with electrical tape, de-solder it, or scratch the trace with an exacto knife.
- Most ESP32 dev boards have a secondary BLUE led, however some may be a different colour. Please use the 'blinking' status to deciper what Winderoo is trying to tell you.

| ESP32 Board (or optional external LED) | LED State | Meaning |
| :------------: | :------------: | :----------------------------------: |
| <img src="images/led-states/blue-off.png" height="300">  | secondary LED is not illuminated | Winderoo is operational. |
| <img src="images/led-states/blue-on.png" height="300">  | secondary LED is illuminated  | Winderoo's is ready for setup. Connect to the WiFi network called "Winderoo Setup" and add Winderoo to your WiFi network. |
| <img src="images/led-states/Winderoo-slow-blink.gif" height="300">   | slow blinking | Winderoo has successfully connected to your WiFi network. When the the **BLUE LED** stops blinking, you may access Winderoo's UI from your web browser. |
| <img src="images/led-states/Winderoo-fast-blink.gif" height="300"> | fast blinking | Winderoo is resetting, wait until the **BLUE LED** turns solid to begin WiFi setup. |
| <img src="images/led-states/Winderoo-snooze.gif" height="300">  | extremely slow blinking | Winderoo's winding capabilities have been turned 'OFF' via the software swtich, or an optional physical button. Winderoo will not wind until it has been turned to 'ON.' |