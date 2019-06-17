#include <NTPClient.h>

#define LINE_HEIGHT 10
#define LINE_LENGTH 21
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define BATTERY_READ_PIN 36

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7200);


SSD1306 display(0x3c, 21, 22);

// where on the screen are we:
int pointer_x = 0;
int pointer_line = 0;


// reset the pointers back to 0
void reset_pointer() {
  pointer_x = 0;
  pointer_line = 0;
}

// move the pointers to the next line
void next_line() {
  pointer_x = 0;
  pointer_line++;
}

// draw text on the screen where the pointer is currently located.
// Wraps around the screen if the text doesn't fit on one line
void text(const String& str) {
  int text_width = display.getStringWidth(str.c_str());
  int text_length = str.length();

  bool we_have_to_go_deeper = false; // boolean to check if we have to make a recursive call to draw the next line
  String this_line = str; // variable to hold what should be drawn on the current line
  String new_line = ""; // variable to hold what should be drawn on the next line

  // Check to see if the text is too long, if so, split the text up so this_line contains the text which fits and new_line contains the rest.
  if (pointer_x + text_width > DISPLAY_WIDTH) {
    int trimmer;
    String trimmed_text;
    for (trimmer = 0; trimmer < text_width; trimmer++) {
      trimmed_text = str.substring(0, text_length - trimmer);
      if (display.getStringWidth(trimmed_text.c_str()) + pointer_x > DISPLAY_WIDTH) continue;
      else break;
    }

    // if the text doesn't fit, we have to make a recursive call
    we_have_to_go_deeper = true;
    this_line = trimmed_text;
    new_line = str.substring(text_length - trimmer, text_length);
  }

  //draw the current line
  display.drawString(pointer_x, (pointer_line * LINE_HEIGHT) + 16, this_line);
  // move the pointer to the end of the line
  pointer_x += display.getStringWidth(this_line.c_str());

  // if we have to make a recursive call
  if(we_have_to_go_deeper) {
    // draw the rest of the text on the next line:
    next_line();
    text(new_line);
  }
}

// draw which message is being shown on the screen
void show_message_index() {
  String text = String(message_index + 1) + "/" + String(message_amt);
  int text_length = display.getStringWidth(text.c_str());
  display.drawString(DISPLAY_WIDTH - text_length, 16, text);
}

// display the bar which tells the time
void show_status_bar(String formattedTime) {
  display.drawString(0, 0, formattedTime);
}

// update the screen with a message
void updateScreen(const String& location, const String& severity, const String& message) {
  reset_pointer();
  text("Location: " + location);
  next_line();
  text("Severity: " + severity);
  next_line();
  text("Message: " + message);
  show_message_index();
}

// update the full screen
void updateScreen() {
  show_status_bar(timeClient.getFormattedTime());
  if (message_amt == 0) {
    reset_pointer();
    text("No Messages");
    return;
  }

  String location = String(message_location[message_index]);
  String severity = String(message_severity[message_index]);
  String message = String(message_message[message_index]);

  updateScreen(location, severity, message);
}

