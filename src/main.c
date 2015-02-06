#include <pebble.h>
#include "common.h"
#include "marker_layer.h"
#include "hour_layer.h"
#include "water_layer.h"
#include "message_layer.h"
#include "status_layer.h"
  
#ifdef RUN_TEST
#include "test_unit.h"
#endif

#define KEY_CURRENT_VERSION 0
#define KEY_INSTALLED_VERSION 1
#define KEY_HOUR_VIBRATE 2
#define KEY_BLUETOOTH_VIBRATE 3
#define KEY_HOUR_VIBRATE_START 4
#define KEY_HOUR_VIBRATE_END 5
#define KEY_CLOCK_24_HOUR 6
#define KEY_REQUEST_SETUP_INFO 7
  
#define MESSAGE_SETTINGS_DURATION 1500
#define MESSAGE_BLUETOOTH_DURATION 5000
  
// Last version of settings that did not contain hour range configuration for hourly vibrate.
#define NO_HOUR_RANGE_VERSION 12
  
typedef struct {
  int32_t currentVersion;
  int32_t hourVibrate;
  int32_t hourVibrateStart;
  int32_t hourVibrateEnd;
  int32_t bluetoothVibrate;
} Settings;

static Window *_mainWindow = NULL;
static MarkerLayerData *_markerData = NULL;
static HourLayerData *_hourData = NULL;
static WaterLayerData *_waterData = NULL;
static MessageLayerData *_messageData = NULL;
static StatusLayerData *_statusData = NULL;
static Settings _settings;
static AppTimer *_messageTimer = NULL;

// Message window strings
static const char *_settingsReceivedMsg = "Settings received!";
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
static void battery_service_handler(BatteryChargeState charge_state);
static void inbox_received_callback(DictionaryIterator *iterator, void *context);
static void inbox_dropped_callback(AppMessageResult reason, void *context);
static void outbox_sent_callback(DictionaryIterator *values, void *context);
static void outbox_failed_callback(DictionaryIterator *failed, AppMessageResult reason, void *context);
static void loadSettings(Settings *settings);
static void saveSettings(Settings *settings);
static int32_t readPersistentInt(const uint32_t key, int32_t defaultValue);
static bool isHourInRange(int16_t hour, int16_t start, int16_t end);
static void sendSetupInfo();
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
  
  // Register battery service
  battery_state_service_subscribe(battery_service_handler);
  
  // Register AppMessage callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  
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
  _statusData = CreateStatusLayer(window_get_root_layer(_mainWindow), CHILD);
  _hourData = CreateHourLayer(window_get_root_layer(_mainWindow), CHILD);
  _waterData = CreateWaterLayer(window_get_root_layer(_mainWindow), CHILD);
  
  // Initialize Bluetooth status
  bool connected = bluetooth_connection_service_peek();
  ShowBluetoothStatus(_statusData, !connected);
  UpdateBluetoothStatus(_statusData, connected);
  
  // Initialize battery status
  BatteryChargeState batteryState = battery_state_service_peek();
  ShowBatteryStatus(_statusData, (batteryState.is_charging || batteryState.is_plugged));
  UpdateBatteryStatus(_statusData, batteryState);
  
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
  
  DestroyStatusLayer(_statusData);
  _statusData = NULL;
  
  DestroyMarkerLayer(_markerData);
  _markerData = NULL;
}

static void timer_handler(struct tm *tick_time, TimeUnits units_changed) {
  drawWatchFace();
  
#ifndef RUN_TEST
  // Check for hourly vibrate
  if ((units_changed & HOUR_UNIT) != 0 && _settings.hourVibrate == 1 &&
      isHourInRange(tick_time->tm_hour, _settings.hourVibrateStart, _settings.hourVibrateEnd)) {
    
    vibes_short_pulse();
  }
#endif
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *tuple = dict_read_first(iterator);
  
  // Check for setup info request from phone.
  if (tuple != NULL && tuple->key == KEY_REQUEST_SETUP_INFO) {
    MY_APP_LOG(APP_LOG_LEVEL_INFO, "Setup info request");
    sendSetupInfo();
    return;
  }

  while (tuple != NULL) {
    switch (tuple->key) {
      case KEY_CURRENT_VERSION:
        _settings.currentVersion = tuple->value->int32;
        MY_APP_LOG(APP_LOG_LEVEL_INFO, "Current version %i", (int) _settings.currentVersion);
        break;
      
      case KEY_HOUR_VIBRATE:
        _settings.hourVibrate = tuple->value->int32;
        MY_APP_LOG(APP_LOG_LEVEL_INFO, "Hourly vibrate %i", (int) _settings.hourVibrate);
        break;
      
      case KEY_HOUR_VIBRATE_START:
        _settings.hourVibrateStart = tuple->value->int32;
        MY_APP_LOG(APP_LOG_LEVEL_INFO, "Hourly vibrate start %i", (int) _settings.hourVibrateStart);
        break;
      
      case KEY_HOUR_VIBRATE_END:
        _settings.hourVibrateEnd = tuple->value->int32;
        MY_APP_LOG(APP_LOG_LEVEL_INFO, "Hourly vibrate end %i", (int) _settings.hourVibrateEnd);
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
  showMessage(_settingsReceivedMsg, MESSAGE_SETTINGS_DURATION);    
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
}

static void outbox_sent_callback(DictionaryIterator *values, void *context) {
  Tuple *tuple = dict_read_first(values);
  
  while (tuple != NULL) {
    switch (tuple->key) {
      case KEY_CLOCK_24_HOUR:
        // Record the most recently sent clock format
        persist_write_int(KEY_CLOCK_24_HOUR, tuple->value->int32);
        MY_APP_LOG(APP_LOG_LEVEL_INFO, "Successfully sent clock format %i to phone", (int) tuple->value->int32);
        break;
      
      case KEY_INSTALLED_VERSION:
        MY_APP_LOG(APP_LOG_LEVEL_INFO, "Successfully sent installed version %i to phone", (int) tuple->value->int32);
        break;
      
      default:
        MY_APP_LOG(APP_LOG_LEVEL_ERROR, "Key %i not recognized", (int) tuple->key);
        break;
    }

    tuple = dict_read_next(values);
  }
}

static void outbox_failed_callback(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  MY_APP_LOG(APP_LOG_LEVEL_INFO, "outbox_failed_callback");
}

static void bluetooth_service_handler(bool connected) {
  if (connected == false) {
    showMessage(_bluetoothDisconnectMsg, MESSAGE_BLUETOOTH_DURATION);
    if (_settings.bluetoothVibrate) {
      vibes_short_pulse(); 
    }
  }
  
  ShowBluetoothStatus(_statusData, !connected);
  UpdateBluetoothStatus(_statusData, connected);
}

static void battery_service_handler(BatteryChargeState charge_state) {
  ShowBatteryStatus(_statusData, (charge_state.is_charging || charge_state.is_plugged));
  UpdateBatteryStatus(_statusData, charge_state);
}

static void loadSettings(Settings *settings) {
  settings->currentVersion = readPersistentInt(KEY_CURRENT_VERSION, 0);
  settings->hourVibrate = readPersistentInt(KEY_HOUR_VIBRATE, 0);
  if (settings->currentVersion <= NO_HOUR_RANGE_VERSION && settings->currentVersion != 0) {
    // To maintain legacy behavior, set hour range to all day.
    settings->hourVibrateStart = 0;
    settings->hourVibrateEnd = 0;
      
  } else {
    settings->hourVibrateStart = readPersistentInt(KEY_HOUR_VIBRATE_START, 9);
    settings->hourVibrateEnd = readPersistentInt(KEY_HOUR_VIBRATE_END, 18);
  }
  
  settings->bluetoothVibrate = readPersistentInt(KEY_BLUETOOTH_VIBRATE, 1);
  
  MY_APP_LOG(APP_LOG_LEVEL_INFO, "Load settings: currentVersion=%i", (int) settings->currentVersion);
  MY_APP_LOG(APP_LOG_LEVEL_INFO, "Load settings: hourVibrate=%i, Start=%i, End=%i",
             (int) settings->hourVibrate, (int) settings->hourVibrateStart, (int) settings->hourVibrateEnd);
  
  MY_APP_LOG(APP_LOG_LEVEL_INFO, "Load settings: bluetoothVibrate=%i", (int) settings->bluetoothVibrate);
}

static void saveSettings(Settings *settings) {
  persist_write_int(KEY_CURRENT_VERSION, settings->currentVersion);
  persist_write_int(KEY_HOUR_VIBRATE, settings->hourVibrate);
  persist_write_int(KEY_HOUR_VIBRATE_START, settings->hourVibrateStart);
  persist_write_int(KEY_HOUR_VIBRATE_END, settings->hourVibrateEnd);
  persist_write_int(KEY_BLUETOOTH_VIBRATE, settings->bluetoothVibrate);
}

static int32_t readPersistentInt(const uint32_t key, int32_t defaultValue) {
  if (persist_exists(key)) {
    return persist_read_int(key);  
  }
  
  return defaultValue;
}

static bool isHourInRange(int16_t hour, int16_t start, int16_t end) {
  if (start == end) {
    return true;
    
  } else if ((end > start) && (hour >= start) && (hour < end)) {
    return true;
    
  } else if ((end < start) && ((hour >= start) || (hour < end))) {
    return true;
  }
  
  return false;
}

static void sendSetupInfo() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }
  
  Tuplet clock24Hour = TupletInteger(KEY_CLOCK_24_HOUR, clock_is_24h_style() ? 1 : 0);
  dict_write_tuplet(iter, &clock24Hour);
  Tuplet installedVersion = TupletInteger(KEY_INSTALLED_VERSION, INSTALLED_VERSION);
  dict_write_tuplet(iter, &installedVersion);
  dict_write_end(iter);
  app_message_outbox_send();
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
  DrawStatusLayer(_statusData, hour, minute);
  DrawHourLayer(_hourData, hour, minute);
  DrawWaterLayer(_waterData, hour, minute);
}
