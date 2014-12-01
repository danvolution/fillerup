//#define RUN_TEST true 

#include <pebble.h>
#include "common.h"
#include "marker_layer.h"
#include "hour_layer.h"
#include "water_layer.h"
  
#ifdef RUN_TEST
#include "test_unit.h"
#endif

static Window* _mainWindow = NULL;
static MarkerLayerData* _markerData = NULL;
static HourLayerData* _hourData = NULL;
static WaterLayerData* _waterData = NULL;

#ifdef RUN_TEST
static TestUnitData* _testUnitData = NULL;
#endif

static void init();
static void deinit();
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void timer_handler(struct tm *tick_time, TimeUnits units_changed);
static void drawWatchFace();

int main(void) {
  init();
  app_event_loop();
  deinit();
}

static void init() {
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
}

static void deinit() {
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
  DestroyWaterLayer(_waterData);
  _waterData = NULL;
  
  DestroyHourLayer(_hourData);
  _hourData = NULL;
  
  DestroyMarkerLayer(_markerData);
  _markerData = NULL;
}

static void timer_handler(struct tm *tick_time, TimeUnits units_changed) {
  drawWatchFace();
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

