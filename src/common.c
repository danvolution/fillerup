#include <pebble.h>
#include "common.h"

void AddLayer(Layer* relativeLayer, Layer* newLayer, LayerRelation relation) {
  switch (relation) {
    case ABOVE_SIBLING:
      layer_insert_above_sibling(newLayer, relativeLayer);
    break;
    
    case BELOW_SIBLING:
      layer_insert_below_sibling(newLayer, relativeLayer);
      break;
    
    default:
      layer_add_child(relativeLayer, newLayer);      
      break;
  }
}

void DestroyBitmapGroup(BitmapGroup* group) {
  if (group != NULL) {
    if (group->bitmap != NULL) {
      gbitmap_destroy(group->bitmap);
      group->bitmap = NULL;
    }
    
    group->resourceId = 0;
    
    if (group->layer != NULL) {
      layer_remove_from_parent((Layer*) group->layer);
      bitmap_layer_destroy(group->layer);
      group->layer = NULL;
    }
  }
}
