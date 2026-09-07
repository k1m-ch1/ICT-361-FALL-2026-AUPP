#pragma once

float asymNormalizedMap(float raw, float minLeft, float maxLeft, float minRight,
                        float maxRight) {
  // assume that minLeft < maxLeft < minRight < maxRight even without checking
  if (raw < maxLeft) {
    // this is the range
    return (raw - maxLeft) / (maxLeft - minLeft);
  } else if (raw > minRight) {
    return (raw - minRight) / (maxRight - minRight);
  } else {
    return 0;
  }
}

float mapFloat(float x, float x0, float x1, float y0, float y1) {
  double y = ((y1 - y0) / (x1 - x0)) * (x - x0) + y0;
  return y;
}
