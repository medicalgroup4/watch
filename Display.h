#include <WiFi.h>
#include <WiFiUDP.h>
#include <NTPClient.h>

#define LINE_HEIGHT 10
#define LINE_LENGTH 21
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define BATTERY_READ_PIN 36

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7200);

int percentage;


SSD1306 display(0x3c, 21, 22);

int pointer_x = 0;
int pointer_line = 0;

void reset_pointer() {
  pointer_x = 0;
  pointer_line = 0;
}

void next_line() {
  pointer_x = 0;
  pointer_line++;
}

void text(const String& str) {
  int text_width = display.getStringWidth(str.c_str());
  int text_length = str.length();

  bool we_have_to_go_deeper = false;
  String this_line = str;
  String new_line = "";
  if (pointer_x + text_width > DISPLAY_WIDTH) {
    int trimmer;
    String trimmed_text;
    for (trimmer = 0; trimmer < text_width; trimmer++) {
      trimmed_text = str.substring(0, text_length - trimmer);
      if (display.getStringWidth(trimmed_text.c_str()) + pointer_x > DISPLAY_WIDTH) continue;
      else break;
    }
    
    we_have_to_go_deeper = true;
    this_line = trimmed_text;
    new_line = str.substring(text_length - trimmer, text_length);
  }

  display.drawString(pointer_x, (pointer_line * LINE_HEIGHT) + 16, this_line);
  pointer_x += display.getStringWidth(this_line.c_str());
  if(we_have_to_go_deeper) {
    next_line();
    text(new_line);
  }
}

void show_message_index() {
  String text = String(curr_message_index + 1) + "/" + String(curr_message_amt);
  int text_length = display.getStringWidth(text.c_str());
  display.drawString(DISPLAY_WIDTH - text_length, 16, text);
}

void show_status_bar(String formattedTime, int percentage) {
  display.drawString(0, 0, formattedTime + "        " + String(percentage) + "%");
}

void updateScreen(const String& location, const String& severity, const String& message) {
  reset_pointer();
  text("Location: " + location);
  next_line();
  text("Severity: " + severity);
  next_line();
  text("Message: " + message);
  show_message_index();
}

void updateScreen() {
  show_status_bar(timeClient.getFormattedTime(), percentage);
  if (curr_message_amt == 0) {
    reset_pointer();
    text("No Messages!");
    return;
  }

  String location = String(message_location[curr_message_index]);
  String severity = String(message_severity[curr_message_index]);
  String message = String(message_message[curr_message_index]);

  updateScreen(location, severity, message);
}

int calculateBatteryPercentage() {
  float ADC = analogRead(BATTERY_READ_PIN) * (13.3 / 10.0);
  float voltage = ADC * (3.3 / 4095.0);

  Serial.println("ADC value = " + String(ADC));
  Serial.println("voltage = " + String(voltage));

  int return_value = (50/.6) * voltage - 250;
  if(return_value < 0) return_value = 0;
  if(return_value > 100) return_value = 100;
  
  return return_value;

}


