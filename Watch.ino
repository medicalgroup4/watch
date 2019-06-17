#include <ESP32HTTPUpdateServer.h>
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
#define WIFI_SSID "davidhotspot"
#define WIFI_PASS "1234hoedjevan"

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

long batteryCheckMillis = 0; // keep time for the battery percentage check

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


  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true

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

  timeClient.begin();
  delay(2000);
  updateScreen();
}

void onConnectionEstablished()
{
  client.subscribe(MESSAGE_TOPIC, messageCallback);
  client.subscribe(CONFIRM_TOPIC, confirmedCallback);
  client.publish(CONNECT_TOPIC, "");
}



void loop() {
  if(millis() - batteryCheckMillis >= 5000) {
    percentage = calculateBatteryPercentage();
    batteryCheckMillis = millis();
  }
  
  if (millis() - prevMillis >= UPDATEINTERVAL) {
    timeClient.update();

    client.loop();

    detectButtons();

    if (left || right) {
      if(millis() - interactionMillis >= TOUCHWAITTIME) {
        button_interaction();
        triggered = true;
      }
      interactionMillis++;
    } else {
      interactionMillis = millis();
      triggered = false;
    }

    display.clear();
    updateScreen();
    performEffects();

    display.drawProgressBar(90, 0, 30, 10, percentage);
    display.display();

    prevMillis = millis();
  }
}

void button_interaction() {
  if (left && right && !triggered) {
    bothEffect = true;
    if(curr_message_amt > 0) {
      client.publish(CONFIRM_TOPIC, String(message_id[curr_message_index]));
      removeMessage(curr_message_index);
      Serial.println("sending delete");
    }    
  } else if (left && !triggered) {
    leftEffect = true;
    curr_message_index--;
    if (curr_message_index < 0) {
      curr_message_index = curr_message_amt - 1;
    }
  } else if (right && !triggered) {
    rightEffect = true;
    curr_message_index++;
    if (curr_message_index > curr_message_amt - 1) {
      curr_message_index = 0;
    }
  }
}

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
