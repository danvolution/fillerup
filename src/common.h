#pragma once
  
//#define RUN_TEST true 
//#define LOGGING_ON true
  
#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168
#define WATER_RISE_DURATION 500
  
#ifdef LOGGING_ON
  #define MY_APP_LOG(level, fmt, args...) APP_LOG(level, fmt, args)
#else
  #define MY_APP_LOG(level, fmt, args...)
#endif

// Convert from minute to Y coordinate
#define WATER_TOP(minute) (SCREEN_HEIGHT - (minute * 14 / 5))

typedef enum { CHILD, ABOVE_SIBLING, BELOW_SIBLING } LayerRelation;

typedef struct {
  BitmapLayer *layer;
  GBitmap *bitmap;
  uint32_t resourceId;
} BitmapGroup;

void AddLayer(Layer *relativeLayer, Layer *newLayer, LayerRelation relation);
void DestroyBitmapGroup(BitmapGroup* group);