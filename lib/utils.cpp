#pragma once

float asymNormalizedMap(float minLeft, float maxLeft, float minRight,
                        float maxRight, float raw) {
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
