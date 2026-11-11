ESP32 OLED CD Player

A DIY Bluetooth media controller powered by an ESP32, featuring a continuously spinning CD animation on a 128×64 SSD1306 OLED display.

Overview

This project turns an ESP32 into a wireless media controller. It uses a touch sensor instead of traditional buttons and displays a rotating CD animation between commands.

The ESP32 connects to a compatible device over Bluetooth and sends media control key events.

Features

- Animated spinning CD displayed on an SSD1306 OLED.
- Bluetooth media control using BLE Keyboard.
- Single touch: Play / Pause.
- Double touch: Next Track.
- Long press: Previous Track.
- On-screen feedback for every command.
- Automatic return to the CD animation after each command.
- Compact design using an ESP32 development board.

Hardware

- ESP32 DevKit V1
- SSD1306 OLED display (128×64, I2C)
- Three-pin digital touch sensor module
- Jumper wires
- USB power source

Wiring

Component| Pin| ESP32
OLED| SDA| GPIO 21
OLED| SCL| GPIO 22
Touch sensor| OUT| GPIO 27
OLED| VCC| Suitable supply voltage
OLED| GND| GND
Touch sensor| GND| GND
Touch sensor| VCC| Compatible supply voltage

Check your specific modules' voltage requirements before connecting them.

Software Requirements

Install the following Arduino libraries:

- ESP32 BLE Keyboard
- Adafruit GFX Library
- Adafruit SSD1306

The project uses the Arduino framework and the ESP32 Bluetooth Low Energy functionality.

How It Works

1. The ESP32 initializes the OLED display and Bluetooth keyboard.
2. A CD animation runs continuously on the display.
3. The touch sensor detects presses and distinguishes single taps, double taps, and long presses.
4. The ESP32 sends the corresponding media key when connected.
5. The display shows the executed command before returning to the animation.

Current Status

The main controller code implements the CD animation, touch gestures, and Bluetooth media commands.

Future improvements may include a custom enclosure, better animation effects, and project documentation with build photos.

Credits

Designed and built as a personal ESP32 electronics project.
