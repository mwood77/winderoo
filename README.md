[![Product Links](https://github.com/mwood77/winderoo/actions/workflows/selenium-web.yml/badge.svg)](https://github.com/mwood77/winderoo/actions/workflows/selenium-web.yml)
[![Markdown Link Checker](https://github.com/mwood77/winderoo/actions/workflows/lint-markdown.yml/badge.svg)](https://github.com/mwood77/winderoo/actions/workflows/lint-markdown.yml)

<div align="center">  
  <h2>
    Winderoo
    <p><i>Open source firmware for microcontolled watch winders</i></p>
  </h2>
</div>

<div align="center">
  <a href="https://www.youtube.com/watch?v=BJKpY6Zp8BI" target="_blank"><strong>👉 See Winderoo in action on youtube 👈</strong></a>
  <br>
  <br>
  <img src="./docs/images/splash-image.jpg" alt="Winderoo - The Open Source Watch Winder" width="600">
  <br>
  <br>
  <p>
    Winderoo is open source firmware which adds <i>smart functionality</i> to your <a href="https://github.com/mwood77/osww" target="_blank">OSWW build</a> or microcontroller equipped watch winder. <i>Smart functionality</i> includes a Web UI (fully-translated in 5 languages!), OLED screen support, start timer, and more!
  </p>
</div>

> [!NOTE]
> OSWW is an open source modular watch winder, which you can read more about on [GitHub](https://github.com/mwood77/osww) or [Printables](https://www.printables.com/model/421249-osww-the-open-source-watch-winder).

### Key Features
* Settable RPD (rotations per day) between 100 - 960rpd
* Single direction (clockwise / anti-clockwise) or bi-directional winding.
* Settable start time. Have it wind while you're asleep, or while you're away, or anytime for that matter. 
* Manual cycle start / stop.
* 3 seconds of rest after 3 minutes of rotation, in every mode.
* Estimated cycle duration (how long it'll take to wind your watch).
* Cycle progress display (how far along the current winding routine is).
* Software or optional physical button to trigger ON/OFF state, so you can disable the winder completely.
* OLED screen support
* Simple setup. Flash the firmware and File System with a few clicks, then connect your phone (or other device) to the winder's setup wifi network & add it to your home network.
* There's no app required! You control it from a web browser.
* Minimal electronics / programming experience required
* Web UI is fully tranlated into 5 langauges (more are welcome!)


### Winderoo Requires a Different Microcontroller
* **You must replace the Pi Pico, as spec'd in the OSWW build guide, with an ESP32. This project will not run on a Pi Pico!**

# User Manual
* [Click Here to see the user manual](./docs/user-manual.md)

## Prerequisites
> [!IMPORTANT]
> This project is an add-on to your already built [Open Source Watch Winder (OSWW)](https://github.com/mwood77/osww)

#### Download and install the following on your computer:
1. [Visual Studio Code](https://code.visualstudio.com/)
1. [PlatformIO](https://platformio.org/install/ide?install=vscode)
    - Note: the "extensions" button has changed since Platformio has created their install guide. You can access the extions pane with the following key combinations:
        - Windows: Ctrl+Shift+X
        - macOS: Command+Shift+X
1. You may or may not need these drivers, but some 'knock off' ESP32 dev boards require them.
    - [CP210x Drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads) 
    - If the CP210x drivers don't work:  [CH340 Drivers](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers) 


## Build Instructions
1. [Bill of Materials (what you need to buy)](./docs/bom-requirements.md)
1. [Software Installation](./docs/install-software.md)
1. [Winderoo Wiring Diagram](./docs/wiring-diagram.md)


## Contributing
This project welcomes contributions. Please follow the regular git workflow; fork + PR to contribute.

### Translations / Localization
Winderoo has multi-language support, and we welcome adding more langauges.

To add another language:
1. Duplicate one of the current localizations, and translate the text:
    - `src/angular/osww-frontend/src/assets/i18n/` stores the current localizations.
1. Add a new langauge menu item here to enable your langauge:
    - `src/angular/osww-frontend/src/app/header/header.component.html`
1. Open a Pull Request

### Frontend
- Angular with Angular Material
- [Frontend Readme](./src/angular/osww-frontend/README.md)

### Backend 
- C++ based API with mDns, a web server, local file system, and a bunch of other things.

### API Specification

The API has 6 endpoints. You can explore them in the attached Open API definition.
- [Open API Definition](./openapi.yml)