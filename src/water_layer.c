#include <pebble.h>
#include "water_layer.h"

WaterLayerData* CreateWaterLayer(Layer* relativeLayer, LayerRelation relation) {
  WaterLayerData* data = malloc(sizeof(WaterLayerData));
  if (data != NULL) {
    data->inverterLayer = inverter_layer_create(GRect(0, 0, 0, 0));
    AddLayer(relativeLayer, (Layer*) data->inverterLayer, relation);
  }
  
  return data;
}

void DrawWaterLayer(WaterLayerData* data, uint16_t hour, uint16_t minute) {
  GRect waterFrame = GRect(0, WATER_TOP(minute), SCREEN_WIDTH, (SCREEN_HEIGHT - WATER_TOP(minute)));
  layer_set_frame((Layer*) data->inverterLayer, waterFrame);
}

void DestroyWaterLayer(WaterLayerData* data) {
  if (data != NULL) {
    if (data->inverterLayer != NULL) {
      inverter_layer_destroy(data->inverterLayer);
      data->inverterLayer = NULL;
    }
    
    free(data);
  }
}
