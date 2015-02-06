#include <pebble.h>
#include "status_layer.h"
  
static char _batteryText[20];
static char _bluetoothConnected[] = "Connected";
static char _bluetoothDisconnected[] = "Disconnected";

StatusLayerData* CreateStatusLayer(Layer *relativeLayer, LayerRelation relation) {
  StatusLayerData *data = malloc(sizeof(StatusLayerData));
  if (data != NULL) {
    memset(data, 0, sizeof(StatusLayerData));
    
    data->textLayerBattery = text_layer_create(GRect(107, 0, 36, 34));
  	text_layer_set_font(data->textLayerBattery, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  	text_layer_set_text_alignment(data->textLayerBattery, GTextAlignmentRight);
    text_layer_set_background_color(data->textLayerBattery, GColorClear);
    AddLayer(relativeLayer, (Layer*) data->textLayerBattery, relation);
    
    data->textLayerBluetooth = text_layer_create(GRect(10, 0, 96, 34));
  	text_layer_set_font(data->textLayerBluetooth, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  	text_layer_set_text_alignment(data->textLayerBluetooth, GTextAlignmentLeft);
    text_layer_set_background_color(data->textLayerBluetooth, GColorClear);
    AddLayer(relativeLayer, (Layer*) data->textLayerBluetooth, relation);
  }
  
  return data;
}

void DrawStatusLayer(StatusLayerData *data, uint16_t hour, uint16_t minute) {
  
}

void DestroyStatusLayer(StatusLayerData *data) {
  if (data != NULL) {
    if (data->textLayerBattery != NULL) {
      text_layer_destroy(data->textLayerBattery);
      data->textLayerBattery = NULL;
    }
    
    if (data->textLayerBluetooth != NULL) {
      text_layer_destroy(data->textLayerBluetooth);
      data->textLayerBluetooth = NULL;
    }
    
    free(data);
  }
}

void UpdateBatteryStatus(StatusLayerData *data, BatteryChargeState charge_state) {
  snprintf(_batteryText, sizeof(_batteryText), "%d%% ", charge_state.charge_percent);
  text_layer_set_text(data->textLayerBattery, _batteryText);
}

void ShowBatteryStatus(StatusLayerData *data, bool show) {
  layer_set_hidden((Layer*) data->textLayerBattery, (show == false));
}

void UpdateBluetoothStatus(StatusLayerData *data, bool connected) {
  text_layer_set_text(data->textLayerBluetooth, connected ? _bluetoothConnected : _bluetoothDisconnected);  
}

void ShowBluetoothStatus(StatusLayerData *data, bool show) {
  layer_set_hidden((Layer*) data->textLayerBluetooth, (show == false));
}