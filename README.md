<div align="center">  
  <h2>
    Winderoo
    <p><i>Supercharge your OSWW Build!</i></p>
  </h2>
</div>

<div align="center">
  <br>
  <img src="./docs/images/splash-image.jpg" alt="Winderoo - The Open Source Watch Winder" width="600">
  <br>
  <br>
  <p>
    Winderoo is open source firmware which adds additional functionality and a GUI to your OSWW.
  </p>
</div>

### What's an OSWW?
- OSWW is an open source modular watch winder, which you can read more about on [GitHub](https://github.com/mwood77/osww) or [Printables](https://www.printables.com/model/421249-osww-the-open-source-watch-winder).



### Key Features
* Settable RPD (rotations per day) between 100 - 960rpd
* Single direction (clockwise / anti-clockwise) or bi-directional winding.
* Settable start time. Have it wind while you're asleep, or while you're away, or anytime for that matter. 
* Manual cycle start / stop.
* 3 seconds of rest after 3 minutes of rotation, in every mode.
* Estimated cycle duration (how long it'll take to wind your watch).
* Cycle progress display (how far along the current winding routine is).
* Software or optional physical button to trigger ON/OFF state, so you can disable the winder completely.
* Simple setup. Flash the firmware and File System with a few clicks, then connect your phone (or other device) to the winder's setup wifi network & add it to your home network.
* There's no app required! You control it from a web browser. 
* Minimal electronics / programming experience required


### Winderoo Requires a Different Microcontroller
* **You must replace the Pi Pico, as spec'd in the OSWW build guide, with an ESP32. This project will not run on the Pi Pico!**

# User Manual
* [Click Here to see the user manual](./docs/user-manual.md)

# Installation
## Prerequisites
- [**Read this first**](./docs/prereqs.md)

## Build Instructions
1. [Bill of Materials (what you need to buy)](./docs/bom-requirements.md)
1. [Software Installation](./docs/install-software.md)
1. [Follow the OSWW Wiring Diagram](https://github.com/mwood77/osww/blob/main/instructions/wiring-diagram.md) but substitue the following connections:
    - Connect the ESP32's 5V Output pin to the 5V input of the L298N driver board (red wire)
    - Connect a ground from the ESP32 to the L298N driver board (black wire)
    - Connect the ESP32's `GPIO25` to `IN1` on the L298N driver board (yellow wire)
    - Connect the ESP32's `GPIO26` to `IN2` on the L298N driver board (blue wire)

## Specifications
The API has 4 endpoints which can be called:

### `status`
  ```
  GET     http://winderoo.local/api/status
  ```
- Provides the current state of Winderoo

#### Response Code 200
- Response Body
  ```json
  {
      "status": "Stopped",
      "rotationsPerDay": "300",
      "direction": "BOTH",
      "hour": "12",
      "minutes": "50",
      "durationInSecondsToCompleteOneRevolution": 8,
      "startTimeEpoch": 0,
      "currentTimeEpoch": 1680555863,
      "estimatedRoutineFinishEpoch": 0,
      "winderEnabled": "1",
      "timerEnabled": "1",
      "db": -67
  }
  ```

### `power`
  ```
  POST     http://winderoo.local/api/power
  ```
- Toggles the winder enable/disable state of Winderoo.

#### Response Code 204
- *No Response Body*

### `update`
  ```
  POST     http://winderoo.local/api/update
  ```
- Updates the state of Winderoo and writes settings to NVRAM
- Accepts the following Request Params:
  ```
    tpd= number
    hour= number
    minutes= number
    timerEnabled= number
    action= START | STOP
    rotationDirection= CW | CCW | BOTH
  ```

Example Request:
  - [`http://winderoo.local/api/update?action=STOP&rotationDirection=BOTH&tpd=300&hour=12&minutes=50`](http://winderoo.local/api/update?action=STOP&rotationDirection=BOTH&tpd=300&hour=12&minutes=50)


#### Response Code 204
- *No Response Body*

### `reset`

  ```
  GET     http://winderoo.local/api/reset
  ```
  - Triggers Winderoo'a reset countdown.
  - WiFi credentials are reset and cleared from NVRAM; Winderoo enters setup mode.

#### Response Code 200
- Response Body
  ```json
  {
      "status": "Resetting"
  }
  ```

### Front End / GUI
- [See this dedicated Readme](src/angular/osww-frontend/README.md)