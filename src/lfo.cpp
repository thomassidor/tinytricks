#include "plugin.hpp"
#include <cmath>

struct LowFrequencyOscillator {
    float phase = 0.0f;
    float pw = 0.5f;
    float freq = 1.0f;
    bool offset = false;
    bool invert = false;
    SchmittTrigger resetTrigger;
    LowFrequencyOscillator() {}
    void setPitch(float pitch) {
        pitch = fminf(pitch, 8.0f);
        freq = powf(2.0f, pitch);
    }
    void setPulseWidth(float pw_) {
        const float pwMin = 0.01f;
        pw = clamp(pw_, pwMin, 1.0f - pwMin);
    }
    void setReset(float reset) {
        if (resetTrigger.process(reset)) {
            phase = 0.0f;
        }
    }
    void setPhase(float p){
      phase = p;
    }
    void step(float dt) {
        float deltaPhase = fminf(freq * dt, 0.5f);
        phase += deltaPhase;
        if (phase >= 1.0f)
            phase -= 1.0f;
    }
    float sin() {
        if (offset)
            return 1.0f - cosf(2.0f*M_PI * phase) * (invert ? -1.0f : 1.0f);
        else
            return sinf(2.0f*M_PI * phase) * (invert ? -1.0f : 1.0f);
    }
    float tri(float x) {
        return 4.0f * fabsf(x - roundf(x));
    }
    float tri() {
        if (offset)
            return tri(invert ? phase - 0.5f : phase);
        else
            return -1.0f + tri(invert ? phase - 0.25f : phase - 0.75f);
    }
    float saw(float x) {
        return 2.0f * (x - roundf(x));
    }
    float saw() {
        if (offset)
            return invert ? 2.0f * (1.0f - phase) : 2.0f * phase;
        else
            return saw(phase) * (invert ? -1.0f : 1.0f);
    }
    float sqr() {
        float sqr = (phase < pw) ^ invert ? 1.0f : -1.0f;
        return offset ? sqr + 1.0f : sqr;
    }
    float light() {
        return sinf(2.0f*M_PI * phase);
    }
};
