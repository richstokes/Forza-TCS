# Forza-TCS
Hardware-based traction control system for the Forza Motorsport &amp; Forza Horizon games.

It uses a [M5Stack CoreS3](https://www.amazon.com/gp/product/B0C7G5GPGC/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) to receive telemetry data from the game. It then acts as a USB Keyboard and sends commands to the Xbox when traction is broken.
&nbsp;



## Arduino/M5 setup
- Follow [the docs](https://docs.m5stack.com/en/quick_start/cores3/arduino) to set up the Arduino IDE and install the libs
- Enable USB-OTG mode under the Ardunio tools menu


## Intro
The Forza Motorsport and Forza Horizon games provide an interesting "data out" feature. This transmits realtime telemetry data via UDP.  

This project captures that data on an ESP32, reads the traction information from each tyre, and uses that to kill the cars throttle if too much (configurable) slip is detected. In this case, it does so by applying the clutch. Ideal would be to kill the throttle instead, still thinking about a good way to do that!  

&nbsp;

## How it works
The main loop reads the telemetry data from the game. We look at the slip rate for each tyre and decide if the car is in a state of understeer or oversteer. If oversteering, and tyre slip rate is over a certain percentage, we kill power to the cars throttle by applying the cltuch.

&nbsp;

### Configuring
Look for the `// CONFIG //` section of `ftcs.ino`:  

- Set your WiFi name and password in `creds.h`
- Optionally change the UDP port
- `TCsensitivity` controls how much slip the program will allow before killing throttle
- `TCspeed` controls the speed above which traction control will kick in - you want a little slip at low speeds to avoid bogging down launches
- Flash the program to your Core S3


&nbsp;

## Usage
- Once connected OK, the display will show the IP address and port you need to enter in your Forza HUD "data out" settings
- The display will also show the amount of Forza packets it is processing per second. This should be around 60 since Forza sends telemetry data at 60 frames per second
- The "L" and "R" values on the display show the amount of slip at each rear wheel in real time

&nbsp;

## Credits
Thank you to: 
- http://www.acidmods.com/forum/index.php/topic,44564.new.html
- https://www.youtube.com/watch?v=qRVTJpwjdZY
- https://github.com/richstokes/Forza-data-tools (This other project of mine is helpful for learning the Forza data format if you'd like to create something similar or learn more)