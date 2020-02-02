# Forza-TCS
Hardware-based traction control system for the Forza Motorsport &amp; Forza Horizon games.

&nbsp;

## Intro
The Forza Motorsport and Forza Horizon games provide an interesting "data out" feature. This transmits realtime telemetry data via UDP.  

This project captures that data on an ESP32, reads the traction information from each tyre, and uses that to kill the cars throttle if too much (configurable) slip is detected.  

Requires modification of an Xbox Controller! I accept no responsability/liabilty for any destroyed hardware! Modifying the controller is reasonably tricky so proceed at your own risk.  

&nbsp;

## How it works
The main loop reads the telemetry data from the game. We look at the slip rate for each tyre and decide if the car is in a state of understeer or oversteer. If oversteering, and tyre slip rate is over a certain percentage, we kill power to the cars throttle by spoofing a signal to the Xbox controller.  

We don't kill power when understeering, since one of the best (read: most fun) ways to escape from understeer can be to apply a whole lot of throttle :-) 

&nbsp;

## Setup
This code is designed to run on [an ESP32 microcontroller with built in WiFi and OLED display](https://www.amazon.com/gp/product/B072HBW53G/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1)  


### Controller wiring
The objective here is to install 2x wires to "test points" on the controller board. [See here](https://github.com/richstokes/Forza-TCS/blob/master/images/solder.jpg)

- This has been tested on Xbox controller model 1708. Other controllers may need wiring differently. [This forum](http://www.acidmods.com/forum/index.php/topic,43981.0.html) has good information on Xbox controller boards.   
- There is a good video on how to dismantle the Xbox controller [here](https://www.youtube.com/watch?v=qRVTJpwjdZY)  
- You only need to take the back case off. No need to separate the boards inside the controller
- I recommend using a hot glue gun to stick the wires in place before soldering
- Be careful, the test points are very small and liable to break if subjected to a light breeze
- You can route the wires through the circle holes on the back of the controller for a reasonably neat install!  


1. Solder a wire from ESP32 ground to TP22 on the Xbox controller.  
2. Solder a wire from ESP32 pin 25 to TP67 on the Xbox controller.  


TP67 is the volts out signal from the triggers' hall sensor, which represents the amount of throttle applied. We use this test point to inject our own throttle signal.  

&nbsp;

### Configuring
Look for the `// CONFIG //` section of `ftcs.ino`:  

- Set your WiFi name and password
- Optionally change the UDP port
- `TCsensitivity` controls how much slip the program will allow before killing throttle
- `TCspeed` controls the speed above which traction control will kick in - you want a little slip at low speeds to avoid bogging down launches
- Flash the program to your ESP32


&nbsp;

## Usage
- Once connected OK, the display will show the IP address and port you need to enter in your Forza HUD "data out" settings
- The display will also show the amount of Forza packets it is processing per second. This should be around 60 since Forza sends telemetry data at 60 frames per second
- The "L" and "R" values on the display show the amount of slip at each rear wheel in real time


&nbsp;
&nbsp;

## Credits
Thank you to: 
- http://www.acidmods.com/forum/index.php/topic,44564.new.html
- https://www.youtube.com/watch?v=qRVTJpwjdZY
- https://github.com/richstokes/Forza-data-tools (This other project of mine is helpful for learning the Forza data format if you'd like to create something similar or learn more)