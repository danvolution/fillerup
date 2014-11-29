#pragma once

#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168
#define WATER_RISE_DURATION 500

// Convert from minute to Y coordinate
#define WATER_TOP(minute) (SCREEN_HEIGHT - (minute * 14 / 5))

typedef enum { CHILD, ABOVE_SIBLING, BELOW_SIBLING } LayerRelation;

typedef struct {
  BitmapLayer* layer;
  GBitmap* bitmap;
  uint32_t resourceId;
} BitmapGroup;

void AddLayer(Layer* relativeLayer, Layer* newLayer, LayerRelation relation);
void DestroyBitmapGroup(BitmapGroup* group);