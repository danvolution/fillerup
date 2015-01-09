#include <pebble.h>
#include "common.h"
#include "marker_layer.h"
#include "hour_layer.h"
#include "water_layer.h"
#include "message_layer.h"
  
#ifdef RUN_TEST
#include "test_unit.h"
#endif

#define KEY_CURRENT_VERSION 0
#define KEY_INSTALLED_VERSION 1
#define KEY_HOUR_VIBRATE 2
#define KEY_BLUETOOTH_VIBRATE 3
  
#define MESSAGE_SETTINGS_DURATION 3000
#define MESSAGE_BLUETOOTH_DURATION 8000
  
typedef struct {
  int32_t currentVersion;
  int32_t installedVersion;
  int32_t hourVibrate;
  int32_t bluetoothVibrate;
} Settings;

static Window *_mainWindow = NULL;
static MarkerLayerData *_markerData = NULL;
static HourLayerData *_hourData = NULL;
static WaterLayerData *_waterData = NULL;
static MessageLayerData *_messageData = NULL;
static Settings _settings;
static AppTimer *_messageTimer = NULL;

// Message window strings
static const char *_settingsReceivedMsg = "Settings received!";
static const char *_settingsReceivedNewVersionMsg = "Settings received.\nNew version available!";
static const char *_bluetoothDisconnectMsg = "Bluetooth connection lost!";

#ifdef RUN_TEST
static TestUnitData* _testUnitData = NULL;
#endif

static void init();
static void deinit();
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void timer_handler(struct tm *tick_time, TimeUnits units_changed);
static void bluetooth_service_handler(bool connected);
static void inbox_received_callback(DictionaryIterator *iterator, void *context);
static void inbox_dropped_callback(AppMessageResult reason, void *context);
static void loadSettings(Settings *settings);
static void saveSettings(Settings *settings);
static int32_t readPersistentInt(const uint32_t key, int32_t defaultValue);
static void showMessage(const char *text, uint32_t duration);
static void messageTimerCallback(void *callback_data);
static void drawWatchFace();

int main(void) {
  init();
  app_event_loop();
  deinit();
}

static void init() {
  loadSettings(&_settings);
  
#ifdef RUN_TEST
  _testUnitData = CreateTestUnit();
#endif
  
  // Create main Window element and assign to pointer
  _mainWindow = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(_mainWindow, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(_mainWindow, true);
  
#ifdef RUN_TEST
    tick_timer_service_subscribe(SECOND_UNIT, timer_handler);
#else
    tick_timer_service_subscribe(MINUTE_UNIT, timer_handler);
#endif
  
  // Register bluetooth service
  bluetooth_connection_service_subscribe(bluetooth_service_handler);
  
  // Register AppMessage callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  if (_messageTimer != NULL) {
    app_timer_cancel(_messageTimer);
    _messageTimer = NULL;
  }
  
#ifdef RUN_TEST
  if (_testUnitData != NULL) {
    DestroyTestUnit(_testUnitData);
    _testUnitData = NULL;
  }
#endif

  if (_mainWindow != NULL) {
    window_destroy(_mainWindow);
    _mainWindow = NULL;
  }
}

static void main_window_load(Window *window) {
  window_set_background_color(window, GColorWhite);
  
  // Fixed layers
  _markerData = CreateMarkerLayer(window_get_root_layer(_mainWindow), CHILD);
  _hourData = CreateHourLayer(window_get_root_layer(_mainWindow), CHILD);
  _waterData = CreateWaterLayer(window_get_root_layer(_mainWindow), CHILD);
  
  drawWatchFace();
}

static void main_window_unload(Window *window) {
  if (_messageData != NULL) {
    DestroyMessageLayer(_messageData);
    _messageData = NULL;
  }
  
  DestroyWaterLayer(_waterData);
  _waterData = NULL;
  
  DestroyHourLayer(_hourData);
  _hourData = NULL;
  
  DestroyMarkerLayer(_markerData);
  _markerData = NULL;
}

static void timer_handler(struct tm *tick_time, TimeUnits units_changed) {
  drawWatchFace();
  
  // Check for hourly vibrate
  if (_settings.hourVibrate == 1 && (units_changed & HOUR_UNIT) != 0) {
    vibes_short_pulse();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *tuple = dict_read_first(iterator);

  while (tuple != NULL) {
    switch (tuple->key) {
      case KEY_CURRENT_VERSION:
        _settings.currentVersion = tuple->value->int32;
        MY_APP_LOG(APP_LOG_LEVEL_INFO, "Current version %i", (int) _settings.currentVersion);
        break;
      
      case KEY_INSTALLED_VERSION:
        _settings.installedVersion = tuple->value->int32;
        MY_APP_LOG(APP_LOG_LEVEL_INFO, "Installed version %i", (int) _settings.installedVersion);
        break;
      
      case KEY_HOUR_VIBRATE:
        _settings.hourVibrate = tuple->value->int32;
        MY_APP_LOG(APP_LOG_LEVEL_INFO, "Hourly vibrate %i", (int) _settings.hourVibrate);
        break;
      
      case KEY_BLUETOOTH_VIBRATE:
        _settings.bluetoothVibrate = tuple->value->int32;
        MY_APP_LOG(APP_LOG_LEVEL_INFO, "Bluetooth vibrate %i", (int) _settings.bluetoothVibrate);
        break;
      
      default:
        MY_APP_LOG(APP_LOG_LEVEL_ERROR, "Key %i not recognized", (int) tuple->key);
        break;
    }

    tuple = dict_read_next(iterator);
  }
  
  saveSettings(&_settings);
  
  if (_settings.currentVersion > _settings.installedVersion) {
    showMessage(_settingsReceivedNewVersionMsg, MESSAGE_SETTINGS_DURATION);

  } else {
    showMessage(_settingsReceivedMsg, MESSAGE_SETTINGS_DURATION);    
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
}

static void bluetooth_service_handler(bool connected) {
  if (connected == false) {
    showMessage(_bluetoothDisconnectMsg, MESSAGE_BLUETOOTH_DURATION);
    if (_settings.bluetoothVibrate) {
      vibes_short_pulse(); 
    }
  }
}

static void loadSettings(Settings *settings) {
  settings->currentVersion = readPersistentInt(KEY_CURRENT_VERSION, 0);
  settings->installedVersion = readPersistentInt(KEY_INSTALLED_VERSION, 0);
  settings->hourVibrate = readPersistentInt(KEY_HOUR_VIBRATE, 0);
  settings->bluetoothVibrate = readPersistentInt(KEY_BLUETOOTH_VIBRATE, 1);
  
  MY_APP_LOG(APP_LOG_LEVEL_INFO, "Load settings: currentVersion=%i, installedVersion=%i",
             (int) settings->currentVersion, (int) settings->installedVersion);
  
  MY_APP_LOG(APP_LOG_LEVEL_INFO, "Load settings: hourVibrate=%i, bluetoothVibrate=%i",
             (int) settings->hourVibrate, (int) settings->bluetoothVibrate);
}

static int32_t readPersistentInt(const uint32_t key, int32_t defaultValue) {
  if (persist_exists(key)) {
    return persist_read_int(key);  
  }
  
  return defaultValue;
}

static void saveSettings(Settings *settings) {
  persist_write_int(KEY_CURRENT_VERSION, settings->currentVersion);
  persist_write_int(KEY_INSTALLED_VERSION, settings->installedVersion);
  persist_write_int(KEY_HOUR_VIBRATE, settings->hourVibrate);
  persist_write_int(KEY_BLUETOOTH_VIBRATE, settings->bluetoothVibrate);
}

static void messageTimerCallback(void *callback_data) {
  _messageTimer = NULL;
  if (_messageData != NULL) {
    DestroyMessageLayer(_messageData);
    _messageData = NULL;
  }
}

static void showMessage(const char *text, uint32_t duration) {
  if (_messageTimer != NULL) {
    if (app_timer_reschedule(_messageTimer, duration) == false) {
      _messageTimer = NULL;
    }
  } 
  
  if (_messageTimer == NULL) {
    _messageTimer = app_timer_register(duration, messageTimerCallback, NULL);
  }
  
  if (_messageData == NULL) {
    _messageData = CreateMessageLayer(window_get_root_layer(_mainWindow), CHILD);
  }
  
  DrawMessageLayer(_messageData, text);
}

static void drawWatchFace() {
#ifdef RUN_TEST
  time_t now = TestUnitGetTime(_testUnitData); 
#else
  time_t now = time(NULL); 
#endif
  
  struct tm* localNow = localtime(&now);
  uint16_t hour = localNow->tm_hour;
  uint16_t minute = localNow->tm_min;
  
  DrawMarkerLayer(_markerData, hour, minute);
  DrawHourLayer(_hourData, hour, minute);
  DrawWaterLayer(_waterData, hour, minute);
}
