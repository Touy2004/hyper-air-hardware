#include "aqi.h"
#include <math.h>

const int BP_PM25[7]  = { 0, 12, 35, 55, 150, 250, 350 };
const int AQI_PM25[7] = { 0, 50, 100, 150, 200, 300, 400 };

const int BP_PM10[7]  = { 0, 54, 154, 254, 354, 424, 504 };
const int AQI_PM10[7] = { 0, 50, 100, 150, 200, 300, 400 };

int calculateAQI(float C, const int BP[], const int AQI_BP[], int size) {
  if (C < 0) C = 0;
  for (int i = 1; i < size; i++) {
    if (C <= BP[i]) {
      float Clow  = BP[i - 1];
      float Chigh = BP[i];
      int   Ilow  = AQI_BP[i - 1];
      int   Ihigh = AQI_BP[i];

      float aqi = (Ihigh - Ilow) * (C - Clow) / (Chigh - Clow) + Ilow;
      return (int)round(aqi);
    }
  }
  return AQI_BP[size - 1];  // max AQI
}
