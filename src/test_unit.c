#include <pebble.h>
#include "test_unit.h"

#define TEST_COUNT 1

typedef struct {
  time_t startTime;
  uint16_t stepSeconds;
  uint16_t stepCount;
} TestData;

static TestData _testData[TEST_COUNT];

TestUnitData* CreateTestUnit() {
  TestUnitData* data = malloc(sizeof(TestUnitData));
  if (data != NULL) {
    memset(data, 0, sizeof(TestUnitData));

    // Normal. Increment by 61 minutes so hour and minute changes.
    _testData[0].startTime = 1420070400; // Jan 1, 2015 00:00:00 GMT
    _testData[0].stepSeconds = 3660;
    _testData[0].stepCount = 60;
  }
  
  return data;
}

void DestroyTestUnit(TestUnitData* data) {
  if (data != NULL) {
    free(data);
  }
}

time_t TestUnitGetTime(TestUnitData* data) {
  // Check if we need to roll over to next test.
  if (data->stepIndex >= _testData[data->testIndex].stepCount) {
    data->stepIndex = 0;
    data->testIndex++;
    if (data->testIndex >= TEST_COUNT) {
      data->testIndex = 0;
    }
  }
  
  // Check if first step of TestData
  if (data->stepIndex == 0) {
    data->time = _testData[data->testIndex].startTime;
    
  } else {
    data->time += _testData[data->testIndex].stepSeconds;
  }
  
  data->stepIndex++;
    
  return data->time;
}