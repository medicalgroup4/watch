#include <EspMQTTClient.h>

#include "WiFiLogo.h"
#include "SSD1306.h"
#include "Message.h"
#include "Display.h"

// once in how many milliseconds should everything update
#define UPDATEINTERVAL 50

// how sensitive are the capacitive buttons
#define TOUCHTHRESHOLD 30

// how much time does the user have to touch another capacitive button, after touching the first one
#define TOUCHWAITTIME 300

// wifi network to connect to
//#define WIFI_SSID "davidhotspot"
//#define WIFI_PASS "1234hoedjevan"

#define WIFI_SSID "Sitecom30EE9C"
#define WIFI_PASS "VTEXXJN6ETRT"

// broker to connect to
#define BROKER_ADDR "51.83.42.157"
// client name on the broker
#define CLIENT_NAME "Watch"

// different message topics
#define MESSAGE_TOPIC "watch/message"
#define CONNECT_TOPIC "watch/connected"
#define CONFIRM_TOPIC "watch/confirm"

// variables to check the capacitive touch buttons state
bool rightTest = false;
bool leftTest = false;
bool right = false;
bool left = false;

// variables to keep the animation state
bool leftEffect = false;
bool rightEffect = false;
bool bothEffect = false;
bool triggered = false;

// effect settings
int radius = 24;
int effectLimit = 6;
int leftPos = 0;
int rightPos = 0;

long prevMillis = 0; // keep time for the global update interval
long effectMillis = 0; // keep time for the animation interval

long interactionMillis = 0; // keep time for the button press interval

// Connect to the MQTT broker
EspMQTTClient client(
  WIFI_SSID,
  WIFI_PASS,
  BROKER_ADDR,
  CLIENT_NAME
);

void setup() {
  Serial.begin(115200);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.drawXbm(34, 16, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display.display();


  // connect to wifi to get the time:
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  client.enableDebuggingMessages();

  display.clear();
  text("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    text(".");
    Serial.print(".");
    delay(500);
    display.display();
  }

  String ip = WiFi.localIP().toString();

  reset_pointer();
  display.clear();
  next_line();
  text("Connected to " + String(WIFI_SSID));
  next_line();
  text("IP: " + ip);
  display.display();
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Connected with IP: ");
  Serial.println(ip);

  // start the time client
  timeClient.begin();
  delay(2000);
  updateScreen();
}

// gets called when we connect to the MQTT broker
void onConnectionEstablished()
{
  // subscribe to the topic to receive messages
  client.subscribe(MESSAGE_TOPIC, messageCallback);
  // subscribe to the topic to make sure confirmed messages also get deleted from this watch
  client.subscribe(CONFIRM_TOPIC, confirmedCallback);
  // send a connect message to receive all unconfirmed messages
  client.publish(CONNECT_TOPIC, "");
}



void loop() {
  // update every 50 milliseconds
  if (millis() - prevMillis >= UPDATEINTERVAL) {
    timeClient.update();

    client.loop();

    detectButtons();

    // if a left or right touch is detected, wait a little bit to give the user time to touch both buttons if they want to
    if (left || right) {
      if(millis() - interactionMillis >= TOUCHWAITTIME) {
        button_interaction();
        triggered = true; // variable to make sure the button trigger only happens once per touch
      }
      interactionMillis++;
    } else {
      interactionMillis = millis();
      triggered = false;
    }

    // update the screen
    display.clear();
    updateScreen();
    performEffects();
    display.display();

    prevMillis = millis();
  }
}


// if the button(s) is/are pressed
void button_interaction() {
  
  if (left && right && !triggered) { //if both buttons are pressed
    bothEffect = true; // trigger the correct animation

    // delete the current message, if there are messages stored:
    if(message_amt > 0) {
      client.publish(CONFIRM_TOPIC, String(message_id[message_index]));
      removeMessage(message_index);
      Serial.println("sending delete");
    }
  } else if (left && !triggered) { //if the left button is pressed
    leftEffect = true; // trigger the correct animation
    
    // go to the previous message:
    message_index--;
    if (message_index < 0) {
      message_index = message_amt - 1;
    }
  } else if (right && !triggered) { //if the right button is pressed
    rightEffect = true; // trigger the correct animation
    
    // go to the next message:
    message_index++;
    if (message_index > message_amt - 1) {
      message_index = 0;
    }
  }
}

// draw the animations
void performEffects() {
  if (leftEffect) {
    if (leftPos++ >= effectLimit) {
      leftPos = 0;
      leftEffect = false;
    }
    display.fillCircle(-radius + leftPos, 16 + radius, radius);
  }
  if (rightEffect) {
    if (rightPos++ >= effectLimit) {
      rightPos = 0;
      rightEffect = false;
    }
    display.fillCircle(DISPLAY_WIDTH + radius - rightPos, 16 + radius, radius);
  }
  if (bothEffect) {
    if (leftPos >= DISPLAY_WIDTH / 2 + radius) {
      leftPos = 0;
      rightPos = 0;
      bothEffect = false;
    }
    display.fillCircle(-radius + (leftPos += 8), 16 + radius, radius);
    display.fillCircle(DISPLAY_WIDTH + radius - (rightPos += 8), 16 + radius, radius);
  }
}

// test which buttons are being touched
void detectButtons() {
  Serial.println("Touchread T1: " + touchRead(T1));
  Serial.println("Touchread T2: " + touchRead(T2));
  
  if (touchRead(T1) < TOUCHTHRESHOLD) {
    if (leftTest)
      left = true;
      
    leftTest = true;
  } else {
    left = false;
    leftTest = false;
  }
  
  if (touchRead(T2) < TOUCHTHRESHOLD) {
    if (rightTest)
      right = true;
      
    rightTest = true;
  } else {
    right = false;
    rightTest = false;
  }
}
