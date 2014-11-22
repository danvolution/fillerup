#include <pebble.h>
#include "marker_layer.h"
  
#define TICK_BIG_WIDTH 10
#define TICK_BIG_HEIGHT 3
#define TICK_SMALL_WIDTH 4
#define TICK_SMALL_HEIGHT 2

static void markerLayerUpdateProc(Layer *layer, GContext *ctx);

MarkerLayerData* CreateMarkerLayer(Layer* relativeLayer, LayerRelation relation) {
  MarkerLayerData* data = malloc(sizeof(MarkerLayerData));
  if (data != NULL) {
    data->layer = layer_create(GRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
    layer_set_update_proc(data->layer, markerLayerUpdateProc);
    AddLayer(relativeLayer, data->layer, relation);
  }
  
  return data;
}

void DrawMarkerLayer(MarkerLayerData* data, uint16_t hour, uint16_t minute) {

}

void DestroyMarkerLayer(MarkerLayerData* data) {
  if (data != NULL) {
    if (data->layer != NULL) {
      layer_destroy(data->layer);
      data->layer = NULL;
    }
    
    free(data);
  }
}

static void markerLayerUpdateProc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  
  for (int minute = 5; minute < 60; minute+=5) {
    if (minute == 15 || minute == 30 || minute == 45) {
      graphics_fill_rect(ctx, GRect(0, WATER_TOP(minute), TICK_BIG_WIDTH, TICK_BIG_HEIGHT), 0, GCornerNone);
  
    } else {
      graphics_fill_rect(ctx, GRect(0, WATER_TOP(minute), TICK_SMALL_WIDTH, TICK_SMALL_HEIGHT), 0, GCornerNone);      
    }
  }
}
