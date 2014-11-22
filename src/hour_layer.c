#include <pebble.h>
#include "hour_layer.h"
  
#define NUMBER_TOP 41
#define LEFT_HOUR_LEFT 12
#define MIDDLE_HOUR_LEFT 43
#define RIGHT_HOUR_LEFT 76
#define NUMBER_HEIGHT 84
#define NUMBER_WIDTH 59

static uint32_t _hourResource[10] = { 
  RESOURCE_ID_IMAGE_0, RESOURCE_ID_IMAGE_1, RESOURCE_ID_IMAGE_2, 
  RESOURCE_ID_IMAGE_3, RESOURCE_ID_IMAGE_4, RESOURCE_ID_IMAGE_5, 
  RESOURCE_ID_IMAGE_6, RESOURCE_ID_IMAGE_7, RESOURCE_ID_IMAGE_8, 
  RESOURCE_ID_IMAGE_9 
};

static void drawHourGroup(BitmapGroup* group, uint16_t digit);
static uint16_t getHour(uint16_t hour);

HourLayerData* CreateHourLayer(Layer* relativeLayer, LayerRelation relation) {
  HourLayerData* data = malloc(sizeof(HourLayerData));
  if (data != NULL) {
    memset(data, 0, sizeof(HourLayerData));
    
    data->leftHour.layer = bitmap_layer_create(GRect(LEFT_HOUR_LEFT, NUMBER_TOP, NUMBER_WIDTH, NUMBER_HEIGHT));
    bitmap_layer_set_compositing_mode(data->leftHour.layer, GCompOpAnd);
    AddLayer(relativeLayer, (Layer*) data->leftHour.layer, relation);
    
    data->middleHour.layer = bitmap_layer_create(GRect(MIDDLE_HOUR_LEFT, NUMBER_TOP, NUMBER_WIDTH, NUMBER_HEIGHT));
    bitmap_layer_set_compositing_mode(data->middleHour.layer, GCompOpAnd);
    AddLayer(relativeLayer, (Layer*) data->middleHour.layer, relation);
    
    data->rightHour.layer = bitmap_layer_create(GRect(RIGHT_HOUR_LEFT, NUMBER_TOP, NUMBER_WIDTH, NUMBER_HEIGHT));
    bitmap_layer_set_compositing_mode(data->rightHour.layer, GCompOpAnd);
    AddLayer(relativeLayer, (Layer*) data->rightHour.layer, relation);
  }
  
  return data;
}

void DestroyHourLayer(HourLayerData* data) {
  if (data != NULL) {
    DestroyBitmapGroup(&data->leftHour);
    DestroyBitmapGroup(&data->middleHour);
    DestroyBitmapGroup(&data->rightHour);
    free(data);
  }
}

void DrawHourLayer(HourLayerData* data, uint16_t hour, uint16_t minute) {
  int16_t leftDigit = -1;
  int16_t middleDigit = -1;
  int16_t rightDigit = -1;
  uint16_t trueHour = getHour(hour);
  
  if (clock_is_24h_style() == true) {
    leftDigit = trueHour / 10;
    rightDigit = trueHour % 10;
    
  } else {
    if (trueHour < 10) {
      middleDigit = trueHour;
      
    } else {
      leftDigit = 1;
      rightDigit = trueHour % 10;
    }
  }
  
  if (leftDigit == -1) {
    layer_set_hidden((Layer*) data->leftHour.layer, true);
    
  } else {
    drawHourGroup(&data->leftHour, leftDigit);
  }
  
  if (middleDigit == -1) {
    layer_set_hidden((Layer*) data->middleHour.layer, true);
    
  } else {
    drawHourGroup(&data->middleHour, middleDigit);
  }
  
  if (rightDigit == -1) {
    layer_set_hidden((Layer*) data->rightHour.layer, true);
    
  } else {
    drawHourGroup(&data->rightHour, rightDigit);
  }
}

static void drawHourGroup(BitmapGroup* group, uint16_t digit) {
  if (group->resourceId != _hourResource[digit]) {
    if (group->bitmap != NULL) {
      gbitmap_destroy(group->bitmap);
      group->bitmap = NULL;
      group->resourceId = 0;
    }
    
    group->bitmap = gbitmap_create_with_resource(_hourResource[digit]);
    bitmap_layer_set_bitmap(group->layer, group->bitmap);
    group->resourceId = _hourResource[digit];
  }  
  
  layer_set_hidden(bitmap_layer_get_layer(group->layer), false);
}

static uint16_t getHour(uint16_t hour) {
  if (clock_is_24h_style() == true) {
    return hour;
  }
  
  int hour12 = hour % 12;
  return (hour12 == 0) ? 12 : hour12;
}