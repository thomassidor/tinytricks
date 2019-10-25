#include "../utility/SimplexNoise.hpp"
const int BUFFER_LENGTH = 512;
struct SimplexOscillator{
	float phase = 0.0f;
	float freq = 0.0f;
	float isStepEOC = false;
  float buffer[BUFFER_LENGTH];
  int bufferIndex = 0;
  float min = 0.f;
  float max = 0.f;


  SimplexNoise simp;

	SimplexOscillator(){
	    simp.init();
	}


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

	bool isEOC(){
		return isStepEOC;
	}

	void setPitch(float pitch){
		freq = 440.f * powf(2.0f, pitch);
	}

	float getOsc(float detaillevel, float x, float y, float z, float scale){
			float value = simp.SumOctave(detaillevel,x+phase,y,z,scale);
			return value*5.f;
	}

  float getNormalizedOsc(float detaillevel, float x, float y, float z, float scale){
    float value = getOsc(detaillevel, x, y, z, scale);

    int prevBufferIndex = bufferIndex-1;

    if(bufferIndex > BUFFER_LENGTH-1){
      bufferIndex = 0;
    }

    float bufferEndValue = buffer[bufferEndIndex];

    buffer[bufferIndex] = value;
    bufferIndex++;
  }

};
