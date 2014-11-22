#pragma once
#include "common.h"

typedef struct {
  Layer* layer;
} MarkerLayerData;

MarkerLayerData* CreateMarkerLayer(Layer* relativeLayer, LayerRelation relation);
void DrawMarkerLayer(MarkerLayerData* data, uint16_t hour, uint16_t minute);
void DestroyMarkerLayer(MarkerLayerData* data);