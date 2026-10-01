# ⛳ Golf Ball Dispenser
 
**A wave-to-dispense golf ball dispenser built on an ESP32, made by Kaushik and Daniel.**
 
Wave a club (or a hand) in front of the sensor and a golf ball drops out, ready to hit. The remaining ball count is shown on a small display and can be updated from your phone.
 
<p align="center">
  <img width="725" height="676" alt="Circuit diagram of the golf ball dispenser" src="https://github.com/user-attachments/assets/001084fe-b140-46e9-b9bd-96bf11765db2" />
</p>
<p align="center"><em>Circuit diagram</em></p>

 
## Table of Contents
 
- [Overview](#overview)
- [How It Works](#how-it-works)
- [Hardware](#hardware)
- [Pin Connections](#pin-connections)
- [Software Setup](#software-setup)
- [Using the Control Panel](#using-the-control-panel)
- [Design Notes and Challenges](#design-notes-and-challenges)
- [Future Improvements](#future-improvements)
- [Credits](#credits)

 
## Overview
 
The main use case is **driving range practice**. We took inspiration from the wave-to-dispense mechanism at Topgolf and from portable helix-style ball dispensers. The result is a compact, servo-driven dispenser that:
 
- Detects motion with an **ultrasonic sensor** and dispenses one ball per trigger
- Shows **green** on the ESP32's onboard LED while dispensing and **red** when idle
- Tracks the **number of balls left** and shows it on a **uLCD display**
- **Remembers the count** across power cycles using the ESP32's non-volatile memory
- Hosts a simple **WiFi control panel** so the count can be set without reflashing
## How It Works
 
1. **Detect.** The ultrasonic sensor continuously measures distance. When something comes within trigger range, a dispense is queued.
2. **Dispense.** The servo sweeps from 0° to 190° in 10° steps, releasing one ball into the drop chamber, then returns to 0°. The count drops by one and is saved to non-volatile memory.
3. **Re-arm.** The sensor will not fire again until the object moves back out of range, so a single wave only dispenses a single ball.
4. **Display.** The uLCD shows the remaining count. When the bucket is empty, it shows **End** and the dispenser stops.
5. **Update.** On boot the ESP32 creates its own WiFi network and serves a small web page for setting the ball count.
## Hardware
 
| Component | Purpose |
|-----------|---------|
| ESP32 development board | Main controller, WiFi access point, onboard NeoPixel status LED |
| Ultrasonic sensor (TRIG/ECHO) | Detects the wave that triggers a dispense |
| Servo motor | Releases balls from the bucket into the drop chamber |
| 4D Systems uLCD (Goldelox, 128x128) | Displays the remaining ball count |
| Breadboard and jumper wires | Circuit connections |
| Ball bucket, mounted at a tilt | Lets balls roll toward the cutout and into the drop chamber |
 
## Pin Connections
 
| Signal | ESP32 Pin |
|--------|-----------|
| Servo signal | 3 |
| uLCD reset | 4 |
| Ultrasonic TRIG | 6 |
| Ultrasonic ECHO | 7 |
| Onboard NeoPixel LED | 8 |
| uLCD serial (RX / TX) | 21 / 22 |
 
See the circuit diagram above for the full wiring.
 
## Software Setup
 
### Requirements
 
- [Arduino IDE](https://www.arduino.cc/en/software) with ESP32 board support installed
- The following libraries:
  - `ESP32Servo`
  - `Adafruit NeoPixel`
  - `Goldelox_Serial_4DLib` (4D Systems serial library for the uLCD)
  - `WiFi`, `Preferences` (included with the ESP32 board package)
### Upload
 
1. Clone or download this repository.
2. Open the `.ino` sketch in the Arduino IDE.
3. Select your ESP32 board and port.
4. Click **Upload**.
### Network credentials
 
The sketch creates a WiFi network with a default name and password defined near the top of the code:
 
```cpp
const char *ssid = "yourAP";
const char *password = "12345678";
```
 
Change these before deploying if you want your own network name and password.
 
## Using the Control Panel
 
1. Power on the dispenser and wait a few seconds for the display to initialize.
2. On your phone or laptop, join the WiFi network named in the sketch (default: `yourAP`).
3. Open a browser and go to the IP address the ESP32 prints over Serial on boot (typically `192.168.4.1`).
4. Enter one of the following and press **Submit**:
| Input | Result |
|-------|--------|
| A number (for example `50`) | Sets the ball count to that number |
| `R` | Resets the count to 0 |
 
The page also shows the current count and the last message received. Because the count is stored in non-volatile memory, it stays accurate even after the dispenser is powered off.
 
## Design Notes and Challenges
 
- **Servo instead of a helix.** We originally designed the dispenser as an attachment for a standard range ball bucket, so a servo-driven drop chamber fit better than a helix conveyor.
- **Jamming.** We planned a 3D printed funnel to keep balls from jamming, but the printers available to us could not produce it. Instead, we mounted the bucket at a tilt so balls naturally roll down to the cutout and into the drop chamber.
- **Power.** We wanted the project to run on battery power but ran out of time. The change itself is simple, and it would make the project truly portable.
## Future Improvements
 
- Battery power for full portability
- Switch from WiFi to **Bluetooth** for the control interface, since WiFi can be unreliable out on the range
- **Swing feedback** from the sound of the shot (pure, shank, top, and so on)
- 3D printed drop chamber, breadboard casing, and funnel to minimize jamming
## Credits
 
Built by **Kaushik Vemulapalli** and **Daniel Ni** as a class project at Georgia Tech.
 
The servo code started from the [Sweep example](https://github.com/jkb-git/ESP32Servo/blob/master/examples/Sweep/Sweep.ino) in the ESP32Servo library, but has since been changed significantly.
