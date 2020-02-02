/*
 *  https://github.com/richstokes/Forza-TCS
 *  Receives UDP Data Out Telemetry data from the Forza Motorsport games
 *  Sends throttle kill signal to Xbox controller
 *  Designed to run on https://www.amazon.com/gp/product/B072HBW53G/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1
 */

#include <WiFi.h>
#include <WiFiMulti.h>
#include "SSD1306Wire.h"  // display driver, using a SSD1306 OLED 128X64 monochrome display
#include "AsyncUDP.h"
#define outputPin 25 // PIN to use for sending throttle signal out - be careful with pin selection as it seems some may be shared with wifi and/or OLED display 

// CONFIG //
const char wifiName[] = "FZA"; // Your wireless name/SSID
const char wifiPW[] = "yourwifikeyhere"; // Your wireless password
const int udpPort = 9999; // UDP Port number to listen on
const float TCsensitivity = 0.95; // How sensitive the traction control is. Lower number = more sensitive. 
const float TCspeed = 2; // Traction control will kick in if above this speed (meters per second). You want to allow for wheelspin at lower speeds to avoid stalling/bogging down at launch
// END OF CONFIG //

// Counters
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 1000;  //the value is a number of milliseconds
int packetCount = 0; //used to track how many packets per second we are processing

// Variables for storing the text used by the display
String message1 = "Initializing..";
String message2 = "github.com/";
String message3 = "richstokes";
String message4 = "Forza TCS";
String carAttitude;
float trueSpeed;

// setting PWM properties
//const int freq = 5000;
//const int ledChannel = 0;
//const int resolution = 10; //Resolution 8, 10, 12, 15

WiFiMulti WiFiMulti; // initialize wifi
SSD1306Wire display(0x3c, 5, 4); // initialize display
AsyncUDP udp; // intialize UDP

void setup()
{
    startMillis = millis();  //initial start time
    Serial.begin(115200);
    
    // configure outputPin
    pinMode(outputPin,INPUT); // input is default state so might not need this
  
    display.init(); // display.flipScreenVertically();
    display.clear(); // clear the display
    message1 = "Connecting to WiFi..";
    drawText();

    WiFiMulti.addAP(wifiName, wifiPW); // Connect to wifi, should be the same network your Xbox/PC running Forza uses

    Serial.println();
    Serial.println();
    Serial.print("Waiting for WiFi... ");

    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    delay(500);
    
    Serial.println("");
    Serial.println("WiFi connected"); 
    const String ipAddress = WiFi.localIP().toString();
    message1 = ipAddress + ":" + String(udpPort);
    message2 = "Ready for data!";
    message3 = "Waiting";
    drawText();

    // Start UDP server
    if(udp.listen(udpPort)) {
        Serial.println("UDP Listening on: " + ipAddress + ":" + String(udpPort));
        udp.onPacket([](AsyncUDPPacket packet) {
          if (packet.length() > 310 && packet.length() < 325) { // Check we have a correctly-sized Forza packet. Note Horizon packets are 324 bytes, Motorsport packets are 311 bytes
            packetCount++; // update packet counter

            if (packet.length() == 324 ) {
//              Serial.println("Horizon packet");
              //Get speed (f32), Byte offset: 256:260
              char speed[4]; // four bytes in a float 32
              speed[0] = packet.data()[256];
              speed[1] = packet.data()[257];
              speed[2] = packet.data()[258];
              speed[3] = packet.data()[259];
              trueSpeed = *( (float*) speed ); // Convert byte array to floating point number
            } else { //Motorsport packet
                //Get speed (f32), Byte offset: 244:248
              char speed[4]; // four bytes in a float 32
              speed[0] = packet.data()[244];
              speed[1] = packet.data()[245];
              speed[2] = packet.data()[246];
              speed[3] = packet.data()[247];
              trueSpeed = *( (float*) speed ); // Convert byte array to floating point number
            }

            Serial.print("Speed: ");
            Serial.println(trueSpeed, DEC);
              
            // Get throttle position
            char throttle[1]; // one byte
            throttle[0] = packet.data()[303];
            Serial.print("TPS: ");
            Serial.println(throttle[0], DEC); //convert to number. Can also use HEX

            //Get TireCombinedSlipFrontLeft (f32),  Byte offset: 180:184
            char slipFrontLeft[4];
            slipFrontLeft [0] = packet.data()[180];
            slipFrontLeft [1] = packet.data()[181];
            slipFrontLeft [2] = packet.data()[182];
            slipFrontLeft [3] = packet.data()[183];
            float trueSlipFrontLeft = *( (float*) slipFrontLeft ); // Convert byte array to floating point number

            //Get TireCombinedSlipFrontRight (f32),  Byte offset: 184:188
            char slipFrontRight[4];
            slipFrontRight [0] = packet.data()[184];
            slipFrontRight [1] = packet.data()[185];
            slipFrontRight [2] = packet.data()[186];
            slipFrontRight [3] = packet.data()[187];
            float trueSlipFrontRight = *( (float*) slipFrontRight ); // Convert byte array to floating point number            

            // Get TireCombinedSlipRearLeft (f32), Byte offset: 188:192
            char slipRearLeft[4];
            slipRearLeft [0] = packet.data()[188];
            slipRearLeft [1] = packet.data()[189];
            slipRearLeft [2] = packet.data()[190];
            slipRearLeft [3] = packet.data()[191];
            float trueSlipRearLeft = *( (float*) slipRearLeft ); // Convert byte array to floating point number

            // Get TireCombinedSlipFrontRight (f32),  Byte offset: 192:196
            char slipRearRight[4];
            slipRearRight [0] = packet.data()[192];
            slipRearRight [1] = packet.data()[193];
            slipRearRight [2] = packet.data()[194];
            slipRearRight [3] = packet.data()[195];
            float trueSlipRearRight = *( (float*) slipRearRight ); // Convert byte array to floating point number

            // Get total slip rates, front and rear
            float totalSlipFront = trueSlipFrontLeft + trueSlipFrontRight;
            float totalSlipRear = trueSlipRearLeft + trueSlipRearRight;
//            Serial.println("TOTAL FRONT SLIP: " + String(totalSlipFront));
//            Serial.println("TOTAL REAR SLIP: " + String(totalSlipRear));
            // Concat both rear slip values to draw on screen display
            message2 = "L: " + String(trueSlipRearLeft) + " R: " + String(trueSlipRearRight);

            // Check cars attitude
            carAttitude = checkAttitude(totalSlipFront, totalSlipRear); // Might want to turn these into ints or otherwise reduce the precision but seems OK for now
//            Serial.println(carAttitude);
            message4 = carAttitude;
                        
            // Check if TCS should kick in
            if (trueSlipRearLeft + trueSlipRearRight > TCsensitivity && carAttitude == "Oversteer" && trueSpeed > TCspeed) {
              Serial.println("TRACTION CONTROL KICKED IN!");
//              display.invertDisplay(); // invert the display to show TCS kicking in
              // Kill throttle 
//              Serial.println("Sending 3.3Volts");
              pinMode(outputPin,OUTPUT);
              digitalWrite(outputPin, HIGH); // Output voltage here signals the controller that the throttle is NOT being pressed and so briefly kills power to the car
            } else {
//              display.normalDisplay();
//              Serial.println("Sending 0 volts");
              pinMode(outputPin,INPUT); // Switch mode back to input so we're not interfering with the signal
            }

            // RANDOM TESTING:
//            Serial.print(ipAddress +":9999", datastring, String(packet.length()) + ":" + String(packetCount));
//            Serial.write(packet.data(), packet.length());
//            Serial.println();
          } else {
            Serial.println("This doesn't look like a Forza packet");
            Serial.println(packet.length());
            packet.printf("Not a Forza packet?!", packet.length());
          }
        });
    }
}


void loop()
{
  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)

  if (currentMillis - startMillis >= period/8) // for testing, in reality we probably dont want to call draw text so fast
    {
       drawText(); // Update the display
    }

    if (currentMillis - startMillis >= period)  //test whether the period has elapsed
    {
  //    For use with the serial plotter:
  //    Serial.println(packetCount);
  //    Serial.println("Packets per second: " + String(packetCount));
        message3 = String(packetCount) + " packets/sec";
  //    drawText(); // update the screen once every second
      
      packetCount = 0; // reset packet counter every second
      startMillis = currentMillis;
    }
}

void drawText() { // See https://github.com/ThingPulse/esp8266-oled-ssd1306
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, message1);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 12, message2);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 30, message3);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 48, message4);
    display.display();
}

// checkAttitude looks for balance of the car
String checkAttitude(float totalSlipFront, float totalSlipRear) { 
    // Check attitude of car by comparing front and rear slip levels
    // If front slip > rear slip, means car is understeering
  if (totalSlipRear > totalSlipFront) {
    return "Oversteer";
  } else if (totalSlipFront > totalSlipRear) {
    return "Understeer";
  } else {
    return "Balanced";
  }
}

// Original UDP Testing stuff
//            Serial.print("UDP Packet Type: ");
//            Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
//            Serial.print(", From: ");
//            Serial.print(packet.remoteIP());
//            Serial.print(":");
//            Serial.print(packet.remotePort());
//            Serial.print(", To: ");
//            Serial.print(packet.localIP());
//            Serial.print(":");
//            Serial.print(packet.localPort());
//            Serial.print(", Length: ");
//            Serial.print(packet.length());
//            Serial.print(", Data: ");

            // convert received buffer to String
            // (Type for packet.data() is uint8_t = byte) {aka unsigned char}
//            char buf[packet.length()+1] = {};
//            memcpy(buf, packet.data(), packet.length()); 
//            String datastring = String(buf);
