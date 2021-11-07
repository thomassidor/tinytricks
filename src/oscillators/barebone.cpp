#include "plugin.hpp"
struct BareboneOscillator {
  float phase = 0.0f;
  float freq = 0.0f;
  float isStepEOC = false;

  void step(float dt) {
    //phase+= freq;
    phase += freq / dt;
    if (phase >= 1.0f) {
      phase = 0.f;
      isStepEOC = true;
    }
    else
      isStepEOC = false;
  }

  void reset() {
    phase = 0.f;
    isStepEOC = true;
  }


  bool isEOC() {
    return isStepEOC;
  }

  void setPitch(float pitch) {
    freq = dsp::FREQ_C4 * powf(2.0f, pitch);
  }
};
