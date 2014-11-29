#include <pebble.h>
#include "water_layer.h"

static PropertyAnimation* _animation = NULL;

static void animationStoppedHandler(Animation *animation, bool finished, void *context);

WaterLayerData* CreateWaterLayer(Layer* relativeLayer, LayerRelation relation) {
  WaterLayerData* data = malloc(sizeof(WaterLayerData));
  if (data != NULL) {
    data->inverterLayer = inverter_layer_create(GRect(0, 0, 0, 0));
    AddLayer(relativeLayer, (Layer*) data->inverterLayer, relation);
    data->lastUpdateMinute = -1;
  }
  
  return data;
}

void DrawWaterLayer(WaterLayerData* data, uint16_t hour, uint16_t minute) {
  // Exit if this minute has already been handled.
  if (data->lastUpdateMinute == minute) {
    return;
  }
  
  // Remember whether first time called.
  bool firstDisplay = (data->lastUpdateMinute == -1); 
  data->lastUpdateMinute = minute;
  GRect newFrame = GRect(0, WATER_TOP(minute), SCREEN_WIDTH, (SCREEN_HEIGHT - WATER_TOP(minute)));
  
  if (minute == 0 || firstDisplay) {
    layer_set_frame((Layer*) data->inverterLayer, newFrame);

  } else if (_animation == NULL) {
    // Create the animation and schedule it.
    _animation = property_animation_create_layer_frame((Layer*) data->inverterLayer, NULL, &newFrame);
    animation_set_duration((Animation*) _animation, WATER_RISE_DURATION);
    animation_set_curve((Animation*) _animation, AnimationCurveLinear);
    animation_set_handlers((Animation*) _animation, (AnimationHandlers) {
      .started = NULL,
      .stopped = (AnimationStoppedHandler) animationStoppedHandler,
    }, NULL);

    animation_schedule((Animation*) _animation);
  }
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

static void animationStoppedHandler(Animation *animation, bool finished, void *context) {
  property_animation_destroy(_animation);
  _animation = NULL;
}