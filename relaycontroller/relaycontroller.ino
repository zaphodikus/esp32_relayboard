// https://forum.arduino.cc/t/esp32-c3-supermini-pinout/1189850
// A relay controller on GPIO pins for the ESP32 supermini
// LOW is relay energised, (led ON)
//
// Commands:
// =========
// DEMO places the target in demo cycle mode 1 relay at a time goes on/low
// R reads all 8 output states
// W nnnnnnnn writes all 8 outputs as either '0' or '1'
// ON<n> number 1-32000 blink on interval
// OFF<n> number 1-32000 blink off interval
//
// Replies:
// ========
// board responds with
// WERR for a "Wxxxx" write error
// WOK for a "Wnnnnnnnnn" write OK
// Rnnnnnnnn "R" where n=states read of all 8 outputs
// ENDD when leaving DEMO mode
//      any command other than DEMO will stop DEMO mode
// DEMO when acknowledging "DEMO"
// Snnnnnnnn when in demo mode it always sends the States of the I/O pins
//      in demo each pin gets cycled slowly!
// ON in response to ON<n>
// OFF in response to OFF<n>
//
#define LED_BUILTIN 8 // gpio 8=LED (internal next to the boot button)
#define RELAY_OFF HIGH
#define RELAY_ON  LOW
// GPIO 2,8,9 may prevent boot if pulled low during boot or flashing
//int iopins[8] = {5,6,7,10,20,21,0,1};
int iopins[8] = {21,4,3,20,1,0,10,7};

#define MAX_SERIAL_MESSAGE 200 // for faster recovery make this as small as needed for only a few full telegrams
bool demo_mode = true;
bool serial_complete = false;
String serial_message;
int on_blink = 100;
int off_blink = 400;

void setup() {
  // put your setup code here, to run once:
  for (int pin=0; pin< 8 ;pin ++)
    pinMode(pin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  serial_message.reserve(MAX_SERIAL_MESSAGE);

  Serial.begin(115200);
  Serial.println("INIT");
  for (int pin=0; pin< 8 ;pin ++){
    pinMode(iopins[pin], OUTPUT);
    digitalWrite(iopins[pin], HIGH); // relay off
  }
  delay(500);
}

void PollSerial(){
  // call as often as possible
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    //Serial.write('>');
    //Serial.write(inChar);
    if (serial_message.length() < MAX_SERIAL_MESSAGE)
      serial_message += inChar;   // discard any chars when we are full
    if (inChar == '\n') {
      serial_complete = true;
    }
  }
}

void HandleSerial_Write(String const message) {
  // Write state
  if (message.length()< 10){ // includes the carriage return
    Serial.println("WERR");
    return;
  }
  for (int pin=0; pin< 8 ;pin ++){
    pinMode(iopins[pin], OUTPUT);
    if (message[1+pin]=='0')
      digitalWrite(iopins[pin], LOW);
    else
      digitalWrite(iopins[pin], HIGH);
  }
  Serial.println("WOK");
}

void HandleSerial_Read(String const message) {
  // Read state
  String response = "R";
  for (int pin=0; pin< 8 ;pin ++){
    if (digitalRead(iopins[pin]) == HIGH)
      response+='1';
    else
      response+='0';
  }
  Serial.println(response);
}

void HandleSerial_DEMO(String const message) {
  // Read state
  Serial.println("DEMO");
  demo_mode = true;
}

void HandleSerial_ON(String const message) {
  // set blink on time
  String number = message.substring(2, message.length()-1);
  Serial.println("ON");
  on_blink = abs(atoi(number.c_str()));
}

void HandleSerial_OFF(String const message) {
  // set blink off time
  String number = message.substring(3, message.length()-1);
  Serial.println("OFF");
  off_blink = abs(atoi(number.c_str()));
}


// call only if serial_complete is true
void HandleSerialMessages(){
  // Grab everything up to the CR and then remove it from the buffer. 
  // Do this until there are no more messages
  //if (serial_complete){
    if (demo_mode)
    {
      Serial.println("ENDD");  // end demo mode
    }
    demo_mode = false; // kick out of demo mode if we detected a message
    // todo: call one of the above handlers
    if (serial_message == "DEMO\n")
      HandleSerial_DEMO(serial_message);
    if (serial_message[0] == 'R')
      HandleSerial_Read(serial_message);
    if (serial_message[0] == 'W')
      HandleSerial_Write(serial_message);
    if (serial_message[0] == 'O')
    {
      if (serial_message[1]== 'N')
        HandleSerial_ON(serial_message);
      if (serial_message[1]== 'F')
        HandleSerial_OFF(serial_message);
    }
    serial_complete = false;
    serial_message = ""; // todo: shift out up to last <CR>
    if (serial_message.lastIndexOf('\n') >=0)
      serial_message = serial_message.substring(serial_message.lastIndexOf('\n'));
    else
      serial_message = "";
  //}
}

void loop() {
  // put your main code here, to run repeatedly:
  if (demo_mode) {
    for (int pin=0; pin< 8 ;pin ++){
        digitalWrite(LED_BUILTIN, HIGH); // LED off

        pinMode(iopins[pin], OUTPUT);
        digitalWrite(iopins[pin], RELAY_ON);   // LOW - inverted
        PollSerial();
        delay(1000);                     // Wait for 1 second
        digitalWrite(iopins[pin], RELAY_OFF);   // HIGH = off inverted
        
        digitalWrite(LED_BUILTIN, LOW); // LED on
        PollSerial();
        delay(100);                     // Blink LED
        pinMode(iopins[pin], INPUT);
        String statusmessage = "S11111111";
        statusmessage[1+pin] = '0';
        Serial.println(statusmessage);
    }
  }else{
    // the non-demo mode
    PollSerial();
    digitalWrite(LED_BUILTIN, HIGH); // LED off
    delay(off_blink);                     
    digitalWrite(LED_BUILTIN, LOW); // LED on
    delay(on_blink);                     
  }
  if (serial_complete) {
    HandleSerialMessages();
  }
}
