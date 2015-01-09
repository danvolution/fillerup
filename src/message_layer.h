#pragma once
#include "common.h"

typedef struct {
  Layer *borderLayer;
  TextLayer *textLayer;
} MessageLayerData;

MessageLayerData* CreateMessageLayer(Layer *relativeLayer, LayerRelation relation);
void DrawMessageLayer(MessageLayerData *data, const char *text);
void DestroyMessageLayer(MessageLayerData *data);