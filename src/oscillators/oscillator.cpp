#include "plugin.hpp"
struct TinyOscillator{
  private:
    float phase = 0.0f;
    float freq = 0.0f;
    float theta = 0.01f;
    float isStepEOC = false;

  public:
    enum OscillatorType {
      SIN,
      SAW,
      SQR,
      TRI
    };

    void step(float dt){
      //phase+= freq;
      phase+= freq / dt;
      if (phase >= 1.0f){
        phase -= 1.0f;
        isStepEOC = true;
      }
      else
        isStepEOC = false;
    }

    void reset(){
        phase = 0.f;
        isStepEOC = true;
    }

    void setTheta(float t){
      theta = t;
    }

    float getSin(){
      return sinf(2.0f * M_PI * phase) * 5.0f;
    }

    float getSaw(){
      return ((1+triInternal((2.f*phase-1.f)/4.f)*sqrInternal(phase/2.f))/2.f)*10.f-5.f;
    }

    float getTri(){
      return triInternal(phase)*5.f;
    }

    float triInternal(float x){
      return 1.f - 2.f*(acos((1.f-theta)*sin(2.f*M_PI*x)))/M_PI;
    }

    float getSqr(){
      return sqrInternal(phase)*5.f;
    }

    float sqrInternal(float x){
      return 2*atan(sin(2.0f*M_PI*x)/theta)/M_PI;
    }

    bool isEOC(){
      return isStepEOC;
    }

    void setPitch(float pitch){
      freq = dsp::FREQ_C4 * powf(2.0f, pitch);
    }


};
