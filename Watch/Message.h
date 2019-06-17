#define ID_INDEX 0
#define SEVERITY_INDEX 2
#define LOCATION_INDEX 3
#define MESSAGE_INDEX 4

#define MAX_MESSAGE_AMT 100

// arrays to store the messages:
int message_id[MAX_MESSAGE_AMT] = {0};
int message_severity[MAX_MESSAGE_AMT] = {0};
String message_location[MAX_MESSAGE_AMT];
String message_message[MAX_MESSAGE_AMT];

int message_amt = 0; // how many messages are currently stored
int message_index = 0; // which message is currently being read by the user


// check if a message with a certain id exists in the storage of the watch
bool messageExists(int id) {
  for(int i = 0; i < message_amt; i++) {
    if(message_id[i] == id) return true;
  }
  return false;
}

// remove a message with a certain index from the watch, if it exists
void removeMessage(int index) {
  if(index >= message_amt || index < 0) return;
  
  for(int i = index; i < message_amt - 1; i++) {
    message_id[index] = message_id[index + 1];
    message_severity[index] = message_severity[index + 1];
    message_location[index] = message_location[index + 1];
    message_message[index] = message_message[index + 1];
  }
  message_amt--;
  if(message_index > 0) message_index--;
}

// remove a message with a certain id from the watch, if it exists
void removeMessageById(int id) {
  if(!messageExists(id)) return; // if we don't have this message, just return

  for(int i = 0; i < message_amt; i++) {
    if(message_id[i] == id) {
      removeMessage(i);
      return;
    }
  }
}

// store a new message from a string
// String format: [id];[patient_id];[severity];[location];[message]
int createMessage(const String& payload) {
  const char* message = payload.c_str();

  //we have to store the severity locally because it should be converted to an integer eventually
  char accumulator_severity[3];
  char accumulator_id[6];
  char accumulator_location[20];
  char accumulator_message[100];
  
  int delimiters_counter = 0;
  int prev_delimiters_counter = delimiters_counter;
  int index = 0;
  
  while(*message != '\0') {
    if(*message == ';') {
      delimiters_counter++;
      message++;
      continue;
    }

    if(delimiters_counter != prev_delimiters_counter) {
      index = 0;
    }
    
    switch(delimiters_counter) {
      case ID_INDEX: {
        accumulator_id[index] = *message;
        accumulator_id[index + 1] = '\0';
        index++;
        break;
      }
      case SEVERITY_INDEX: {
        accumulator_severity[index] = *message;
        accumulator_severity[index + 1] = '\0';
        index++;
        break;
      }
      case LOCATION_INDEX: {
        accumulator_location[index] = *message;
        accumulator_location[index + 1] = '\0';
        index++;
        break;
      }
      case MESSAGE_INDEX: {
        accumulator_message[index] = *message;
        accumulator_message[index + 1] = '\0';
        index++;
        break;
      }
    }
    prev_delimiters_counter = delimiters_counter;
    message++;
  }
  int new_message_id = String(accumulator_id).toInt();

  if(messageExists(new_message_id)) return message_amt - 1; //if we already have this message, just return
  
  message_id[message_amt] = new_message_id;
  message_severity[message_amt] = String(accumulator_severity).toInt();
  message_location[message_amt] = String(accumulator_location);
  message_message[message_amt] = String(accumulator_message);
  message_amt++;
  return message_amt - 1;
}

// function gets called when a new message is received
// creates a new message and shows it to the user
void messageCallback(const String& payload) {
  message_index = createMessage(payload);
}

// function gets called when another watch confirmed a message
// removes the message confirmed by another nurse
void confirmedCallback(const String& payload) {
  int id = payload.toInt();
  if(id == 0) return; // if the ID is 0, the payload was not an integer

  removeMessageById(id);
}


