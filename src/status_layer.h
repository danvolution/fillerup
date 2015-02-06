#pragma once
#include "common.h"

typedef struct {
  TextLayer *textLayerBluetooth;
  TextLayer *textLayerBattery;
} StatusLayerData;

StatusLayerData* CreateStatusLayer(Layer *relativeLayer, LayerRelation relation);
void DrawStatusLayer(StatusLayerData *data, uint16_t hour, uint16_t minute);
void DestroyStatusLayer(StatusLayerData *data);
void UpdateBatteryStatus(StatusLayerData *data, BatteryChargeState charge_state);
void ShowBatteryStatus(StatusLayerData *data, bool show);
void UpdateBluetoothStatus(StatusLayerData *data, bool connected);
void ShowBluetoothStatus(StatusLayerData *data, bool show);