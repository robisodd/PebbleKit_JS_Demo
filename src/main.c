#include <pebble.h>
bool js_ready = false;

static Window *window;
static TextLayer    *text_layer;
char text_buffer[100] = "";  // for text layer

#define KEY_MESSAGE     1
#define KEY_COMMAND     2
#define KEY_COUNT       3

#define COMMAND_STOP         100
#define COMMAND_START        101
#define COMMAND_COUNT        103

// ------------------------------------------------------------------------ //
//  AppMessage Functions
// ------------------------------------------------------------------------ //
char *translate_dictionaryresult(DictionaryResult result) {
  switch (result) {
    case DICT_OK: return "DICT_OK";
    case DICT_NOT_ENOUGH_STORAGE: return "DICT_NOT_ENOUGH_STORAGE";
    case DICT_INVALID_ARGS: return "DICT_INVALID_ARGS";
    case DICT_INTERNAL_INCONSISTENCY: return "DICT_INTERNAL_INCONSISTENCY";
    case DICT_MALLOC_FAILED: return "DICT_MALLOC_FAILED";
    default: return "UNKNOWN ERROR";
  }
}

char *translate_appmessageresult(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

static void send_command(uint8_t command) {
  printf("Sending Command: %d", command);
  if (!js_ready) {printf("Cannot send command: Javascript not ready"); return;}
  DictionaryIterator *iter;
  if (app_message_outbox_begin(&iter)) {printf("Cannot send command: Error Preparing Outbox"); return;}
  if (dict_write_uint8(iter, KEY_COMMAND, command)) {printf("Cannot send command: Failed to write uint8"); return;}
  dict_write_end(iter);
  app_message_outbox_send();
}

void send_message_to_phone(uint32_t key, char *str) {
  printf("Sending string: %s", str);
  if (!js_ready) {printf("Cannot send command: Javascript not ready"); return;}
  
  DictionaryIterator *iter;
  uint32_t result = (uint32_t)app_message_outbox_begin(&iter);
  if(result !=APP_MSG_OK) {
    printf("Error %s returned from app_message_outbox_begin (message key %d): %s", translate_appmessageresult(result), (int)key, str);
  } else if (!iter) {
    printf("Error creating iterator for the outbound message. (message key %d): %s", (int)key, str);
  } else {
    result = dict_write_cstring(iter, key, str);
    if(result != DICT_OK) {
      printf("Error %s returned from dict_write_cstring (message key %d): %s", translate_dictionaryresult(result), (int)key, str);
    } else {
      result = dict_write_end(iter);
      if(result==0) {
        printf("Dictionary size %d bytes returned from dict_write_end (message key %d): %s", (int)result, (int)key, str);
      } else {
        AppMessageResult result = app_message_outbox_send();
        if(result !=APP_MSG_OK) {
          printf("Error %s returned from app_message_outbox_send (message key %d): %s", translate_appmessageresult(result), (int)key, str);
        }
      }
    }
  }
}

static void appmessage_in_received_handler(DictionaryIterator *iter, void *context) {
  js_ready = true;  // PebbleKit JS is running
  
  Tuple *message_tuple = dict_find(iter, KEY_MESSAGE);
  if(message_tuple){
    printf("Received message: %s", message_tuple->value->cstring);
    strncpy(text_buffer, message_tuple->value->cstring, sizeof text_buffer);
    text_layer_set_text(text_layer, text_buffer);
  }
  
  Tuple *count_tuple = dict_find(iter, KEY_COUNT);
  if(count_tuple){
    printf("Received count: %d", (int)count_tuple->value->int16);
    snprintf(text_buffer, sizeof text_buffer, "Count = %d", (int)count_tuple->value->int16);
    text_layer_set_text(text_layer, text_buffer);
  }

  // If update GPS 32bit coords
//   Tuple *lat_tuple  = dict_find(iter, KEY_GPS_LAT);
//   Tuple *lon_tuple  = dict_find(iter, KEY_GPS_LON);
//   if(lat_tuple && lon_tuple) {
//     int32_t lat = lat_tuple->value->int32;
//     int32_t lon = lon_tuple->value->int32;
//     printf("Received GPS coords: (%d, %d)", (int)lat, (int)lon);
//   }
}

static void appmessage_out_failed_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  js_ready = false; // Can't communicate with PebbleKit JS
  printf("App Message Failed: %s", translate_appmessageresult(reason));
}


static void appmessage_in_dropped_handler(AppMessageResult reason, void *context) {
  js_ready = false; // Can't communicate with PebbleKit JS
  printf("App Message Failed: %s", translate_appmessageresult(reason));
}


static void app_message_init() {
  // Register message handlers
  app_message_register_inbox_received(appmessage_in_received_handler); 
  app_message_register_inbox_dropped(appmessage_in_dropped_handler); 
  app_message_register_outbox_failed(appmessage_out_failed_handler);
  
  // Init buffers
  // Size = 1 + 7 * N + size0 + size1 + .... + sizeN
  //app_message_open(1 + 7 * 2 + sizeof(int32_t) + CHUNK_SIZE, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
  
  //app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());  // <-- bad idea. lotsa ram eaten
  
  app_message_open(640, 640);  // <-- ought to be enough for anybody
}




// ------------------------------------------------------------------------ //
//  Button Functions
// ------------------------------------------------------------------------ //
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  //send_command(COMMAND_STOP);
  send_message_to_phone(KEY_MESSAGE, "Up pressed");
}

static void dn_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_command(COMMAND_COUNT);
}

static void sl_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_command(COMMAND_START);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,     up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, sl_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN,   dn_click_handler);
}

// ------------------------------------------------------------------------ //
//  Main Functions
// ------------------------------------------------------------------------ //
static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect root_frame = layer_get_frame(root_layer);
  
  text_layer = text_layer_create(GRect(0, (root_frame.size.h - 16) / 2, root_frame.size.w, 16));
  text_layer_set_text(text_layer, "Waiting for JavaScript...");
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(text_layer, GColorClear);
  layer_add_child(root_layer, text_layer_get_layer(text_layer));
  
  printf("Waiting for Javascript...");
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  app_message_init();
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
