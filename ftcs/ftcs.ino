// Remember to set USB mode (not upload mode) to USB-OTG under tools menu
// In this example we have bound the clutch to the Z key in the game settings
#include <M5CoreS3.h>
#include "USB.h"
#include "USBHIDKeyboard.h"
USBHIDKeyboard Keyboard;
#include <WiFi.h>
#include <WiFiMulti.h>
#include "AsyncUDP.h"
#include "creds.h"

// Configurables
const int udpPort = 9999;
const float maxAllowedSlip = 1.0; // Max slip allowed
// const float TCsensitivity = 0.95; // How sensitive the traction control is. Lower number = more sensitive.
const float TCsensitivity = 0.1;           // How sensitive the traction control is. Lower number = more sensitive.
const float TCspeed = 2;                   // Traction control will kick in if above this speed (meters per second). You want to allow for wheelspin at lower speeds to avoid stalling/bogging down at launch
const String actionString = "zzzzzzzzzzz"; // Action (keys to press) when traction is broken:
// End of Configurables

WiFiMulti WiFiMulti;
AsyncUDP udp; // intialize UDP

String slipMessage;
String carAttitude;
float trueSpeed;

// Wifi example https://github.com/m5stack/M5CoreS3/blob/main/examples/Advanced/WIFI/WiFiTCP/WiFiTCP.ino
void setup()
{
  int sum = 0;
  M5.begin(); // Init M5CoreS3
  // Reset the screen
  M5.Lcd.clear();
  M5.Lcd.fillScreen(WHITE);
  delay(50);
  M5.Lcd.fillScreen(RED);
  delay(50);
  M5.Lcd.fillScreen(GREEN);
  delay(50);
  M5.Lcd.fillScreen(BLUE);
  delay(50);
  M5.Lcd.fillScreen(BLACK);
  delay(50);
  M5.Lcd.setTextSize(2); // Configure text

  WiFiMulti.addAP(
      ssid,
      password);                  // Add wifi configuration information to creds.h
      "\nConnecting to WiFi..."); // Serial port output format string.
      while (WiFiMulti.run() !=
             WL_CONNECTED)
      {
        M5.lcd.print(".");
        delay(1000);
        sum += 1;
        if (sum == 8)
          M5.lcd.print("Connect failed!");
      }
      M5.lcd.println("\nWiFi connected");
      M5.lcd.print("IP address: ");
      M5.lcd.println(
          WiFi.localIP());
      delay(500);

      // Init USB keyboard / HID device
      Keyboard.begin();
      USB.begin();

      // Start UDP server
      if (udp.listen(udpPort))
      {
        M5.lcd.println("UDP listening on port " + String(udpPort));

        udp.onPacket([](AsyncUDPPacket packet)
                     {
                         if (packet.length() > 310 && packet.length() < 332) // packet length now 331 for motorsport?
                         {                                                   // Check we have a correctly-sized Forza packet. Note Horizon packets are 324 bytes, Motorsport packets are 311 bytes
                             if (packet.length() == 324)
                             {
                                //  USBSerial.println("Got Horizon packet");
                                 // Get speed (f32), Byte offset: 256:260
                                 char speed[4]; // four bytes in a float 32
                                 speed[0] = packet.data()[256];
                                 speed[1] = packet.data()[257];
                                 speed[2] = packet.data()[258];
                                 speed[3] = packet.data()[259];
                                 trueSpeed = *((float *)speed); // Convert byte array to floating point number
                             }
                            else
                             { // Motorsport packet
                                 // USBSerial.println("Got motorsport packet");
                                 // M5.lcd.println("Got a motorsport packet");
                                 // Get speed (f32), Byte offset: 244:248
                                 char speed[4]; // four bytes in a float 32
                                 speed[0] = packet.data()[244];
                                 speed[1] = packet.data()[245];
                                 speed[2] = packet.data()[246];
                                 speed[3] = packet.data()[247];
                                 trueSpeed = *((float *)speed); // Convert byte array to floating point number
                             }

                            //  USBSerial.print("Speed: ");
                            //  USBSerial.println(trueSpeed, DEC);

                             // Get data needed to calculate slip angles / attitude
                             // Get TireCombinedSlipFrontLeft (f32),  Byte offset: 180:184
                             char slipFrontLeft[4];
                             slipFrontLeft[0] = packet.data()[180];
                             slipFrontLeft[1] = packet.data()[181];
                             slipFrontLeft[2] = packet.data()[182];
                             slipFrontLeft[3] = packet.data()[183];
                             float trueSlipFrontLeft = *((float *)slipFrontLeft); // Convert byte array to floating point number

                             // Get TireCombinedSlipFrontRight (f32),  Byte offset: 184:188
                             char slipFrontRight[4];
                             slipFrontRight[0] = packet.data()[184];
                             slipFrontRight[1] = packet.data()[185];
                             slipFrontRight[2] = packet.data()[186];
                             slipFrontRight[3] = packet.data()[187];
                             float trueSlipFrontRight = *((float *)slipFrontRight); // Convert byte array to floating point number

                             // Get TireCombinedSlipRearLeft (f32), Byte offset: 188:192
                             char slipRearLeft[4];
                             slipRearLeft[0] = packet.data()[188];
                             slipRearLeft[1] = packet.data()[189];
                             slipRearLeft[2] = packet.data()[190];
                             slipRearLeft[3] = packet.data()[191];
                             float trueSlipRearLeft = *((float *)slipRearLeft); // Convert byte array to floating point number

                             // Get TireCombinedSlipFrontRight (f32),  Byte offset: 192:196
                             char slipRearRight[4];
                             slipRearRight[0] = packet.data()[192];
                             slipRearRight[1] = packet.data()[193];
                             slipRearRight[2] = packet.data()[194];
                             slipRearRight[3] = packet.data()[195];
                             float trueSlipRearRight = *((float *)slipRearRight); // Convert byte array to floating point number

                             // Get total slip rates, front and rear
                             float totalSlipFront = trueSlipFrontLeft + trueSlipFrontRight;
                             float totalSlipRear = trueSlipRearLeft + trueSlipRearRight;
                            //  USBSerial.println("TOTAL FRONT SLIP: " + String(totalSlipFront));
                            //  USBSerial.println("TOTAL REAR SLIP: " + String(totalSlipRear));

                             // Concat both rear slip values to draw on screen display
                             slipMessage = "L: " + String(trueSlipRearLeft) + " R: " + String(trueSlipRearRight);
                             M5.Lcd.setCursor(10, 100);
                             M5.Lcd.print(slipMessage);
                             M5.Lcd.setCursor(10, 120);
                             M5.Lcd.print(trueSpeed);

                            // Check for / react to oversteer
                            if (trueSlipRearLeft > maxAllowedSlip) { // todo: include && trueSpeed > TCspeed so it doenst kick in at too low a speed
                              M5.Lcd.setCursor(10, 160);
                              M5.Lcd.print("TCS ON!!!");
                              // Send the keyboard command
                              Keyboard.print(actionString);
                              // Todo: maybe switch for Keyboard.press/release? https://www.arduino.cc/reference/en/language/functions/usb/keyboard/
                              // // Keyboard.press(122);
                              // delay(200);
                              // Keyboard.release(122);
                            } else {
                              M5.Lcd.setCursor(10, 160);
                              // M5.Lcd.setTextColor(WHITE);
                              M5.Lcd.print("TCS OFF!!!");
                            }
                         }
                         else
                         {
                             M5.lcd.println("Error not a motorsport packet!?!");
                             M5.lcd.println(packet.length());
                             packet.printf("Not a Forza packet?!", packet.length());
                         } });
      }
}

void loop()
{
}

// checkAttitude looks for balance of the car
String checkAttitude(float totalSlipFront, float totalSlipRear)
{
  // Check attitude of car by comparing front and rear slip levels
  // If front slip > rear slip, means car is understeering
  if (totalSlipRear > totalSlipFront)
  {
    return "Oversteer";
  }
  else if (totalSlipFront > totalSlipRear)
  {
    return "Understeer";
  }
  else
  {
    return "Neutral";
  }
}