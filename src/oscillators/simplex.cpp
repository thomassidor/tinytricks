#include <algorithm>
#include "../utility/SimplexNoise.hpp"

struct SimplexOscillator {
  static const int BUFFER_LENGTH = 2048;
  float phase = 0.0f;
  float freq = 0.0f;

  unsigned int tick = 0;

  bool isStepEOC = false;

  bool reverse = false;

  float buffer[BUFFER_LENGTH] = {0};
  int bufferIndex = 0;

  float min = -1.f;
  float max = 1.f;

  float mirror = false;

  SimplexNoise simp;

  SimplexOscillator() {
    simp.init();
  }

  void setMirror(bool _mirror) {
    mirror = _mirror;
    min = -1.f;
    max = 1.f;
    tick = 0;
    reset();
  }

  void step(float dt) {
    float delta = freq / dt;

    isStepEOC =  false;
    if (mirror) {
      if (!reverse) {
        phase += delta;
        if (phase >= 0.5f)
          reverse = true;
      }
      else {
        phase -= delta;
        if (phase < 0.f) {
          reverse = false;
          //phase = 0.f;
          phase = -phase;
          isStepEOC = true;
        }
      }
    }
    else {
      phase += delta;
      if (phase >= 1.0f) {
        //phase = 0.f;//-= 1.0f;
        phase -= 1.0f;
        isStepEOC = true;
      }
    }
  }

  void reset() {
    phase = 0.f;
    reverse = false;
  }

  bool isEOC() {
    return isStepEOC;
  }

  void setPitch(float pitch) {
    freq = dsp::FREQ_C4 * powf(2.0f, pitch);
  }

  float getValue(float detaillevel, float x, float y, float z, float scale) {
    return simp.SumOctaveSmooth(detaillevel, x + phase, y, z, scale);
  }

  float getOsc(float detaillevel, float x, float y, float z, float scale) {
    float value = getValue(detaillevel, x, y, z, scale);
    return value * 5.f;
  }


  float getNormalizedOsc(float detaillevel, float x, float y, float z, float scale) {

    float value = getValue(detaillevel, x, y, z, scale);

    if (bufferIndex >= BUFFER_LENGTH) {
      bufferIndex = 0;
    }

    buffer[bufferIndex] = value;
    bufferIndex++;


    if (tick % 256 == 0) {
      auto result = std::minmax_element(begin(buffer), end(buffer));
      min = *result.first;
      max = *result.second;
    }
    tick++;

    value = clamp(rescale(value, min, max, -1.f, 1.f), -1.f, 1.f);
    return value * 5.f;;

  }
};
