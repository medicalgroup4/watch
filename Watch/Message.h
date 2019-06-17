#define ID_INDEX 0
#define SEVERITY_INDEX 2
#define LOCATION_INDEX 3
#define MESSAGE_INDEX 4

#define MAX_MESSAGE_AMT 100

int message_id[MAX_MESSAGE_AMT] = {0};
int message_severity[MAX_MESSAGE_AMT] = {0};
String message_location[MAX_MESSAGE_AMT];
String message_message[MAX_MESSAGE_AMT];

int curr_message_amt = 0;
int curr_message_index = 0;



bool messageExists(int id) {
  for(int i = 0; i < curr_message_amt; i++) {
    if(message_id[i] == id) return true;
  }
  return false;
}

void removeMessage(int index) {
  if(index >= curr_message_amt || index < 0) return; // if the index does not exist, return
  for(int i = index; i < curr_message_amt - 1; i++) {
    message_id[index] = message_id[index + 1];
    message_severity[index] = message_severity[index + 1];
    message_location[index] = message_location[index + 1];
    message_message[index] = message_message[index + 1];
  }
  curr_message_amt--;
  if(curr_message_index > 0) curr_message_index--;
}

void removeMessageById(int id) {
  if(!messageExists(id)) return; // if we don't have this message, just return

  for(int i = 0; i < curr_message_amt; i++) {
    if(message_id[i] == id) {
      removeMessage(i);
      return;
    }
  }
}



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

  if(messageExists(new_message_id)) return curr_message_amt - 1; //if we already have this message, just return
  
  message_id[curr_message_amt] = new_message_id;
  message_severity[curr_message_amt] = String(accumulator_severity).toInt();
  message_location[curr_message_amt] = String(accumulator_location);
  message_message[curr_message_amt] = String(accumulator_message);
  curr_message_amt++;
  return curr_message_amt - 1;
}

void messageCallback(const String& payload) {
  curr_message_index = createMessage(payload);
}

void confirmedCallback(const String& payload) {
  int id = payload.toInt();
  if(id == 0) return; // if the ID is 0, the payload was not an integer

  removeMessageById(id);
}


