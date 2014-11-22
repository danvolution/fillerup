#pragma once
#include "common.h"

typedef struct {
  BitmapGroup leftHour;
  BitmapGroup middleHour;
  BitmapGroup rightHour;
} HourLayerData;

HourLayerData* CreateHourLayer(Layer* relativeLayer, LayerRelation relation);
void DrawHourLayer(HourLayerData* data, uint16_t hour, uint16_t minute);
void DestroyHourLayer(HourLayerData* data);