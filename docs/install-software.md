> [👈 Back to main page](../README.md)

# Software Installtion & Flashing your ESP32

> [!IMPORTANT]
> This project is an add-on to your already built [Open Source Watch Winder (OSWW)](https://github.com/mwood77/osww). 

- This project requires a microncontroller which replaces the Pi Pico used in the OSWW.
- The Raspberry Pi Pico is incompatible with this project and must be swapped.


# Getting Started

### Prerequisites
- Download [Visual Studio Code (VSCode)](https://code.visualstudio.com/download)
- Download [PlatformIO](https://platformio.org/install/ide?install=vscode)

## Flashing your microcontroller

1. Download this repository. You can do this via git or by downloading the repository as a ZIP.
    - If you downloaded the repository as a zip, uzip it before proceeding to step 2.
    <div align="center"><img src="images/download_directory.png" alt="how to download"></div>
1. Open the extracted folder (or cloned repository if using git) in Visual Studio Code
1. **IMPORTANT** -> if you're building Winderoo with an OLED screen attached, you must enable a build flag to tell PlatformIO to include additional libraries. To do this:
    - Navigate to the file `platformio.ini`
            <div align="center"><img src="images/platformio-ini.png" alt="how to download"></div>
    - In this file, you'll see the following block of code:
        ```yml
        build_flags =
	        -D OLED_ENABLED=false
        ```
    - Change `-D OLED_ENABLED=false` to `-D OLED_ENABLED=true`
    - PlatformIO will now compile Winderoo with OLED screen support
1. Select 'PlatformIO' (alien/insect looking button) on the workspace menu and wait for visual studio code to finish initializing the project
    <div align="center"><img src="images/platformIO.png" alt="platformIO button"></div>
1. Expand the main heading: **"esp32doit-devkit-v1"**:
    <div align="center"><img src="images/platformio_project_menu.png" alt="platformIO actions overview"></div>
1. Expand the "General" heading, then click **Upload**. You'll see a message if the code was uploaded successfully:
    <div align="center"><img src="images/code_uploaded.png" alt="code upload button"></div>
1. Now expand the "Platform" heading, then click **Upload Filesystem Image**. You'll see a message if the code was uploaded successfully:
    <div align="center"><img src="images/code_uploaded.png" alt="upload filesystem button"></div>
1. All done! Your microcontroller should now have 2 LEDs illuminated (see beneath). If it does, proceed to [Next steps](#next-steps). If not, try to upload the code & file system again.
    <div align="center"><img src="images/led_states/blue_on.png" alt="upload filesystem button" height="300"></div>
1. If you have a different LED state, compare it with this table: 
    - [Understanding Winderoo's LED Blink Status](user-manual.md#understanding-winderoos-led-blink-status)

## Next steps:

Ok, you've got 2 LEDs illuminated on your board. Great! Let's make sure the code works.

1. Get out your mobile device (phone/tablet) and navigate to the Wi-Fi / wireless network list. You'll want to find and join a WiFi network called "**Winderoo Setup**."
    - It may take a minute or so to connect, or look like it isn't doing anything. Hold tight, it is.
1. Once you've connected to "**Winderoo Setup**", a captive page will open called "WifiManger Winderoo Setup."
1. Tap "Configure WiFi"
1. Tap on the network you want to add Winderoo to, this is likely your home WiFi network
1. Enter your WiFi network's password then tap "Save." Wait until the captive page closes itself.
1. Re-join your home WiFi network
1. Open up a web browser and try to navigate to the following webpage:
    - [http://winderoo.local/](http://winderoo.local/)
1. If you see Winderoo's user interface, you're all done!
    - [Here is an overview of Winderoo's user interface](./user-manual.md)
