#pragma once
#include "common.h"
  
typedef struct {
  InverterLayer* inverterLayer;
  int16_t lastUpdateMinute;
} WaterLayerData;

WaterLayerData* CreateWaterLayer(Layer* relativeLayer, LayerRelation relation);
void DrawWaterLayer(WaterLayerData* data, uint16_t hour, uint16_t minute);
void DestroyWaterLayer(WaterLayerData* data);