# User Manual

## Winderoo's User Interface

| v1.0.0 | |
| :---: |:---: |
|<img src="gui/overview_v1.0.0.png" height="600">  | This is winderoo's primary interace. From here you'll be able to change any settings you need. What follows is a break-down of what each feature does |


### Enable / Disable Winding
| UI Element | Function |
| :---: |:---: |
|<img src="gui/enable-disable.png" >  | This is a software switch, which will _completely enable or disable_ the winder's ability to wind. When the switch is set to **OFF**, Winderoo will not wind or begin winding at a your desired 'Cycle Start Time.' If you choose to attach an (optional) physical button, the code will automatically toggle this switch when you press the button. |

### Reset / Change WiFi Network
| UI Element | Function |
| :---: |:---: |
|<img src="gui/reset.png" >  | This is will open a pop-up window, which will ask you to confirm reset, and walk you through the reset proceedure. Use this to change the WiFi network Winderoo connects to. |

### Status Bar
| UI Element | Function |
| :---: |:---: |
|<img src="gui/status-bar_stopped.png" >  | Winder is not winding |
|<img src="gui/status-bar_winding.png" >  | Winder is winding |
|<img src="gui/status-bar_wifi-red.png" > | Wifi Status - this behaves as you'd expect |

### Control Buttons
| UI Element | Function |
| :---: |:---: |
|<img src="gui/control-button_start.png" >  | _begin_ winding immediately, using saved settings |
|<img src="gui/control-button_stop.png" >  | _stop_ winding immediately |

### Progress Indicator
> The progress indicator only appears while actively winding

| UI Element | Function |
| :---: |:---: |
|<img src="gui/progress-indicator_updating.png" >  | While animated, Winderoo's is attempting to update the progress indicator |
|<img src="gui/progress-indicator_status.png" >  | The progress of the current winding routine |

### Estimated Cycle Duration
| UI Element | Function |
| :---: |:---: |
|<img src="gui/estimated-cycle-duration.png" >  | An estimation of how long the your current winding routine will take, start to finish |

### Winder Control
| UI Element | Function |
| :---: |:---: |
|<img src="gui/winder-control_direction.png" >  | Which direction Winderoo should turn. The available options are 'Clockwise,' 'Both' (clockwise + counter-clockwise), and 'Counter-Clockwise' |
|<img src="gui/winder-control_rotations-per-day.png" >  | How many full rotations Winderoo should complete per routine |
|<img src="gui/winder-control_help.png" >  | Launches a new browser tab to an external, 3rd party website. This resource hosts winding parameters for most major watch manufacturers |

### Cycle Start Time
| UI Element | Function |
| :---: |:---: |
|<img src="gui/cycle-start-time.png" >  | Set which time you'd like Winderoo to begin winding at. **_Important!_** WInderoo will _always_ start at this time, even if you've already triggered a manual run with a [control button](#control-buttons). To stop this behaviour, see: [Enable / Disable Winding](#enable--disable-winding) |

### Save / Update Settings
| UI Element | Function |
| :---: |:---: |
|<img src="gui/save-button.png" >  | This will capture and save all settings (winding direction, rotations per day, cycle start time). If a winding routine is currenty running, it does not reset the current routine (it will update and finish accordingly). If your wish to make sure the routine is changed, manually stop, then start the routine. See [control button](#control-buttons). |


## Understanding Winderoo's LED Blink Status

- Most ESP32 dev boards have a primary RED LED that is always on. This is typically a RED led. This cannot be shut off via firmware. 
    - If it bothers your, you can cover it with electrical tape, de-solder it, or scratch the trace with an exacto knife.
- Most ESP32 dev boards have a secondary BLUE LED, however some may be a different colour. Please use the 'blinking' status to decipher what Winderoo is trying to tell you.

| ESP32 Board (or optional external LED) | LED State | Meaning |
| :------------: | :------------: | :----------------------------------: |
| <img src="images/led-states/blue-off.png" height="300">  | secondary LED is not illuminated | Winderoo is operational. |
| <img src="images/led-states/blue-on.png" height="300">  | secondary LED is illuminated  | Winderoo's is ready for setup. Connect to the WiFi network called "Winderoo Setup" and add Winderoo to your WiFi network. |
| <img src="images/led-states/Winderoo-slow-blink.gif" height="300">   | slow blinking | Winderoo has successfully connected to your WiFi network. When the the **BLUE LED** stops blinking, you may access Winderoo's UI from your web browser. |
| <img src="images/led-states/Winderoo-fast-blink.gif" height="300"> | fast blinking | Winderoo is resetting, wait until the **BLUE LED** turns solid to begin WiFi setup. |
| <img src="images/led-states/Winderoo-snooze.gif" height="300">  | extremely slow blinking | Winderoo's winding capabilities have been turned 'OFF' via the software swtich, or an optional physical button. Winderoo will not wind until it has been turned to 'ON.' |