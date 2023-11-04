//const byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //zcam controller mac address
//const byte client_ip[] = { 10, 98, 32, 6 };  //zcam controller IP address
const byte server_ip[] = { 10, 98, 32, 1 };  //zcam IP address
const unsigned int port = 80;


int startPosition = 250;
int newPosition = startPosition;
const unsigned int focus_step = 250/5; // for some reason it multiplies by 5 commands from pushbuttons here below
const unsigned int focus_range = 7500;  //max 32767  //(-32767,+32767): 65535 posizioni
unsigned int sensitivity = 1; //do not change for now: generates errors if different than 1

#include <Encoder.h>

//#include <SPI.h>         8
//#include <Ethernet.h>    
//EthernetClient client;

Encoder myEnc(2, 3); //(B,A)

// pushbutton variables
byte pin_btn[6] = {4, 6, 8, 10, 11, 12};
byte buttonEnabled[3] = {0, 0, 0};
byte buttonState[6];
byte lastButtonState[6] = {LOW, LOW, LOW, LOW, LOW, LOW};
unsigned long lastDebounceTime[6] = {0, 0, 0, 0, 0, 0};
const unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
bool pulsante_focus = LOW;
unsigned long t_pulsante = 10000000000;

int oldPosition;

void setup() {

  Serial.begin(115200);

  delay(500);
  /*
    Serial.println("ZCAM controller consolle - Arduino Uno:");
    Serial.print("elaborates http commands for ZCAM with IP ");
    Serial.print(String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]) + " on port " + String(port));
    Serial.println(" and sends them to PC-COM5");
    Serial.println("REMEMBER to start VSPE with Serial/TCPclient emulation on PC!");
    Serial.println();
  */
  /*// Per la connessione:
    Serial.println(String(client_ip[0]) + "." + String(client_ip[1]) + "." + String(client_ip[2]) + "." + String(client_ip[3]) + ":");
    Serial.print("connecting to ZCAM ");
    Serial.println(String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]) + " ");
    Ethernet.begin(mac, client_ip);
    if (client.connect(server_ip, port)) {
      Serial.println("--> connected");
    }
    else
    {
      Serial.println("--> connection failed");
    }
    client.stop();
  */

  //PUSHBUTTONS AND LEDs
  pinMode(pin_btn[0], INPUT);    //P1: REC
  pinMode(pin_btn[0] + 1, OUTPUT); //LED_REC
  pinMode(pin_btn[1], INPUT);    //P2: CAF continuous auto focus
  pinMode(pin_btn[1] + 1, OUTPUT); //LED_CAF
  pinMode(pin_btn[2], INPUT);    //P3: TRIGGER auto focus
  pinMode(pin_btn[2] + 1, OUTPUT); //LED_LAF
  pinMode(pin_btn[3], INPUT);   //P4
  pinMode(pin_btn[4], INPUT);   //P5
  pinMode(pin_btn[5], INPUT);   //P6
/*
  digitalWrite(pin_btn[0] + 1,HIGH);
  
    Serial.println("Focus start position:" + String(startPosition));
    //send_command("GET /ctrl/set?lens_focus_pos=" + String(startPosition));
    Serial.println();
  
  delay(1000);
  */
  Serial.println("GET /ctrl/set?caf=Off HTTP/1.1");  //
  Serial.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
  Serial.println("Connection: close");
  Serial.println(); // end HTTP header

  delay(250);

  Serial.println("GET /ctrl/set?live_caf=On HTTP/1.1");  //
  Serial.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
  Serial.println("Connection: close");
  Serial.println(); // end HTTP header

  delay(250);

  
  Serial.println("GET /ctrl/set?lens_focus_pos=" + String(startPosition) + " HTTP/1.1");
  Serial.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
  Serial.println("Connection: close");
  Serial.println(); // end HTTP header
  pulsante_focus = HIGH;
  t_pulsante = millis();

  delay(500);

  digitalWrite(pin_btn[0] + 1,HIGH);
  
  delay(1000);
  

  digitalWrite(pin_btn[1] + 1,HIGH);

  delay(1000);
  
  digitalWrite(pin_btn[2] + 1,HIGH);

  delay(2000);
  
  digitalWrite(pin_btn[0] + 1,LOW);
  digitalWrite(pin_btn[1] + 1,LOW);
  digitalWrite(pin_btn[2] + 1,LOW);

  //  while (Serial.available() > 0) {
  //    char risposta = Serial.read();
  //    Serial.print(risposta);
  //  }
  //  Serial.println();

  /*
    Serial.println("GET /ctrl/get?k=lens_focus_pos HTTP/1.1");  //HTTP/1.1
    Serial.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
    Serial.println("Connection: close");
    Serial.println(); // end HTTP header

    delay(10);

    while (Serial.available() > 0) {
      char risposta = Serial.read();
      Serial.print(risposta);
      //HTTP/1.1 200 Ok
      //Content-Length: 113
      //Cache-Control: no-cache, no-store, max-age=0, must-revalidate
      //Content-Type: application/json; charset=utf-8
      //Access-Control-Allow-Origin: *
      //Connection: close

      //{"code":0,"desc":"string","key":"lens_focus_pos","type":2,"ro":0,"value":-1914,"min":-32767,"max":32767,"step":1}
      delay(5);
    }
    Serial.println();
  */
  myEnc.write(startPosition);
  oldPosition = startPosition;
}

void loop() {
  /////////////// PUSHBUTTONS/////////////////////////////////////////////////////////////////////////////////////////
  //P1 REC
  byte reading1 = digitalRead(pin_btn[0]);    // read the state of the switch into a local variable:
  // check to see if you just pressed the button  (i.e. the input went from LOW to HIGH), and you've waited long enough since the last press to ignore any noise:
  if (reading1 != lastButtonState[0]) {  // If the switch changed, due to noise or pressing:
    lastDebounceTime[0] = millis(); // reset the debouncing timer
  }
  if ((millis() - lastDebounceTime[0]) > debounceDelay) {  // whatever the reading is at, it's been there for longer than the debounce delay, so take it as the actual current state:
    // if the button state has changed:
    if (reading1 != buttonState[0]) {
      buttonState[0] = reading1;
      if (buttonState[0]) {   // only toggle if the new button state is HIGH
        buttonEnabled[0] = 1 - buttonEnabled[0];
        digitalWrite(pin_btn[0] + 1, buttonEnabled[0]);
        if (buttonEnabled[0]) {  //if buttonEnabled[0]==1 so True then REC ON e LED_REC HIGH
          Serial.println("GET /ctrl/rec?action=start HTTP/1.1");  //HTTP/1.1
          Serial.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
          Serial.println("Connection: close");
          Serial.println(); // end HTTP header
          //send_command("GET /ctrl/rec?action=start");  //HTTP/1.1
        }
        else  {                 //otherwise buttonEnabled[0]==0 so False then REC OFF e LED_REC LOW
          Serial.println("GET /ctrl/rec?action=stop HTTP/1.1");  //HTTP/1.1
          Serial.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
          Serial.println("Connection: close");
          Serial.println(); // end HTTP header
          //send_command("GET /ctrl/rec?action=stop");  //HTTP/1.1
        }
        delay(10);
      }
    }
  }
  lastButtonState[0] = reading1;

  //P2  CAF continuous auto focus
  byte reading2 = digitalRead(pin_btn[1]);    // read the state of the switch into a local variable:
  // check to see if you just pressed the button  (i.e. the input went from LOW to HIGH), and you've waited long enough since the last press to ignore any noise:
  if (reading2 != lastButtonState[1]) {  // If the switch changed, due to noise or pressing:
    lastDebounceTime[1] = millis(); // reset the debouncing timer
  }
  if ((millis() - lastDebounceTime[1]) > debounceDelay) {  // whatever the reading is at, it's been there for longer than the debounce delay, so take it as the actual current state:
    // if the button state has changed:
    if (reading2 != buttonState[1]) {
      buttonState[1] = reading2;
      if (buttonState[1]) {   // only toggle if the new button state is HIGH
        buttonEnabled[1] = 1 - buttonEnabled[1];
        digitalWrite(pin_btn[1] + 1, buttonEnabled[1]);

        if (buttonEnabled[1]) {  //if buttonEnabled[1]==1 so True then CAF ON e LED_CAF HIGH
          //Serial.println("GET /ctrl/set?caf=on");  //HTTP/1.1
          Serial.println("GET /ctrl/set?caf=On HTTP/1.1");
          Serial.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
          Serial.println("Connection: close");
          Serial.println(); // end HTTP header
          //digitalWrite(pin_btn[1] + 1, HIGH);
        }
        else  {                 //otherwise buttonEnabled[1]==0 so False then CAF OFF e LED_CAF LOW
          //Serial.println("GET /ctrl/set?caf=off");  //HTTP/1.1
          Serial.println("GET /ctrl/set?caf=Off HTTP/1.1");
          Serial.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
          Serial.println("Connection: close");
          Serial.println(); // end HTTP header
          //digitalWrite(pin_btn[1] + 1, LOW);
        }
        delay(10);
      }
    }
  }
  lastButtonState[1] = reading2;

  if (millis() - t_pulsante >= 1000) {
    digitalWrite(pin_btn[2] + 1, LOW);
    t_pulsante = 10000000000;
  }

  //P3 TRIGGER AUTO-FOCUS
  byte reading3 = digitalRead(pin_btn[2]);    // read the state of the switch into a local variable:
  // check to see if you just pressed the button  (i.e. the input went from LOW to HIGH), and you've waited long enough since the last press to ignore any noise:
  if (reading3 != lastButtonState[2]) {  // If the switch changed, due to noise or pressing:
    lastDebounceTime[2] = millis(); // reset the debouncing timer
  }
  if ((millis() - lastDebounceTime[2]) > debounceDelay) {  // whatever the reading is at, it's been there for longer than the debounce delay, so take it as the actual current state:
    // if the button state has changed:
    if (reading3 != buttonState[2]) {
      buttonState[2] = reading3;
      if (buttonState[2]) {        // only toggle if the new button state is HIGH
        Serial.println("GET /ctrl/af HTTP/1.1");
        Serial.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
        Serial.println("Connection: close");
        Serial.println(); // end HTTP header

        delay(10);

        digitalWrite(pin_btn[2] + 1, HIGH);
        t_pulsante = millis();
      }
    }
  }
  lastButtonState[2] = reading3;


  //P4 FOCUS -
  byte reading4 = digitalRead(pin_btn[3]);    // read the state of the switch into a local variable:
  // check to see if you just pressed the button  (i.e. the input went from LOW to HIGH), and you've waited long enough since the last press to ignore any noise:
  if (reading4 != lastButtonState[3]) {  // If the switch changed, due to noise or pressing:
    lastDebounceTime[3] = millis(); // reset the debouncing timer
  }
  if ((millis() - lastDebounceTime[3]) > debounceDelay) {  // whatever the reading is at, it's been there for longer than the debounce delay, so take it as the actual current state:
    // if the button state has changed:
    if (reading4 != buttonState[3]) {
      buttonState[3] = reading4;
      if (buttonState[3]) {        // only toggle if the new button state is HIGH

        newPosition -= focus_step;

        if (abs(newPosition) > focus_range) { 
          newPosition = newPosition / abs(newPosition) * focus_range;
        }
        if (newPosition != oldPosition) {
          pulsante_focus = HIGH;

          Serial.println("GET /ctrl/set?mf_drive=" + String(oldPosition - newPosition) + " HTTP/1.1");
          //Serial.println("GET /ctrl/set?lens_focus_pos=" + String(newPosition) + " HTTP/1.1");
          Serial.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
          Serial.println("Connection: close");
          Serial.println(); // end HTTP header

          digitalWrite(pin_btn[2] + 1, HIGH);
          delay(10);
          t_pulsante = millis();
        }
      }
    }
  }
  lastButtonState[3] = reading4;


  //P5 FOCUS MED
  byte reading5 = digitalRead(pin_btn[4]);    // read the state of the switch into a local variable:
  // check to see if you just pressed the button  (i.e. the input went from LOW to HIGH), and you've waited long enough since the last press to ignore any noise:
  if (reading5 != lastButtonState[4]) {  // If the switch changed, due to noise or pressing:
    lastDebounceTime[4] = millis(); // reset the debouncing timer
  }
  if ((millis() - lastDebounceTime[4]) > debounceDelay) {  // whatever the reading is at, it's been there for longer than the debounce delay, so take it as the actual current state:
    // if the button state has changed:
    if (reading5 != buttonState[4]) {
      buttonState[4] = reading5;
      if (buttonState[4]) {        // only toggle if the new button state is HIGH
        Serial.println("GET /ctrl/set?lens_focus_pos=" + String(startPosition) + " HTTP/1.1");  //HTTP/1.1
        Serial.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
        Serial.println("Connection: close");
        Serial.println(); // end HTTP header

        newPosition = startPosition;
        pulsante_focus = HIGH;
        //oldPosition = newPosition;

        digitalWrite(pin_btn[2] + 1, HIGH);

        delay(10);
        t_pulsante = millis();
      }
    }
  }
  lastButtonState[4] = reading5;

  //P6 FOCUS +
  byte reading6 = digitalRead(pin_btn[5]);    // read the state of the switch into a local variable:
  // check to see if you just pressed the button  (i.e. the input went from LOW to HIGH), and you've waited long enough since the last press to ignore any noise:
  if (reading6 != lastButtonState[5]) {  // If the switch changed, due to noise or pressing:
    lastDebounceTime[5] = millis(); // reset the debouncing timer
  }
  if ((millis() - lastDebounceTime[5]) > debounceDelay) {  // whatever the reading is at, it's been there for longer than the debounce delay, so take it as the actual current state:
    // if the button state has changed:
    if (reading6 != buttonState[5]) {
      buttonState[5] = reading6;
      if (buttonState[5]) {        // only toggle if the new button state is HIGH

        newPosition += focus_step;
        if (abs(newPosition) > focus_range) {
          newPosition = newPosition / abs(newPosition) * focus_range;
          
        }
        if (newPosition != oldPosition) {
          Serial.println("GET /ctrl/set?mf_drive=" + String(oldPosition - newPosition) + " HTTP/1.1");
          //Serial.println("GET /ctrl/set?lens_focus_pos=" + String(newPosition) + " HTTP/1.1");
          Serial.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
          Serial.println("Connection: close");
          Serial.println(); // end HTTP header

          pulsante_focus = HIGH;
          digitalWrite(pin_btn[2] + 1, HIGH);

          delay(10);

          t_pulsante = millis();
        }
      }
    }
  }
  lastButtonState[5] = reading6;

  if (pulsante_focus) {  //if focus position has been changed with a pushbutton, update encoder value
    pulsante_focus = LOW;
    /* if (abs(newPosition) > focus_range) {   // filters any variation bringing out of range
       newPosition = newPosition / abs(newPosition) * focus_range;
      }*/
    myEnc.write(newPosition);
    oldPosition = newPosition;
  }
  else {                 //if no pushbutton was used, read encoder value
    newPosition = myEnc.read() * sensitivity;
    if (abs(newPosition) > focus_range) {  
      newPosition = newPosition / abs(newPosition) * focus_range;
      myEnc.write(newPosition);
    }
    int movement=0;
    if (newPosition != oldPosition) { //if focus value has changed send focus command
      movement=(oldPosition - newPosition)/3;
      movement+=(oldPosition - newPosition)/abs(oldPosition - newPosition);
      Serial.println("GET /ctrl/set?mf_drive=" + String(movement) + " HTTP/1.1");
      //Serial.println("GET /ctrl/set?lens_focus_pos=" + String(newPosition) + " HTTP/1.1");
      Serial.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
      Serial.println("Connection: close");
      Serial.println(); // end HTTP header

      digitalWrite(pin_btn[2] + 1, HIGH);
      delay(10);
      digitalWrite(pin_btn[2] + 1, LOW);

      oldPosition = newPosition;
      delay(90);
    }

  }
}

/* for TCP-IP 
  void //send_command(String x) {
  if (client.connect(server_ip, port)) {
  client.println(x); 
  client.println("Host: " + String(server_ip[0]) + "." + String(server_ip[1]) + "." + String(server_ip[2]) + "." + String(server_ip[3]));
  client.println("Connection: close");
  client.println(); // end HTTP header
  Serial.println();
  client.stop();
  }
  else
  {
  Serial.println(" --> connection failed");
  }
  }
*/
