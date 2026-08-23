#esp32\_relayboard

##An esp32 relay board driver.
5VDC / USB powered, including Python driver code

This code is intended for and documents a simple ESP32 supermini with 8 isolated relays. How I used power over USB to drive up to 2 relays at a time maximum load in an 8 relay bank. This is a simple veroboard-able project because it is really just 8 pins for the relay driver transistors and 2 pins for power.

Note, I'm driving LOW for the relays, it just happened that way, so you may need to read the ESP spec sheet and then also play with which relay board you do buy to get the driver signal right for you. I also have this power budget of only being able to drive 3 relays before the USB power budget (300mA?) seems to cause a USB bus power problem. Regardless that can be overcome by using a separate relay power supply source. With that one gotch abotu the drive direction out of the way I hope that's the dragons and other smoke sources covered.

##web
ESP32 supermini pinout notes [SoftCircuitry](https://softcircuitry.blogspot.com/2026/08/esp32-c3-supermini.html)
Arduino [Testing ideas thread](https://forum.arduino.cc/t/how-to-create-a-simple-test-f-w-over-monitor/1456745)

