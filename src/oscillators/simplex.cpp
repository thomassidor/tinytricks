#include <algorithm>
#include "../utility/SimplexNoise.hpp"

const int BUFFER_LENGTH = 1024;

struct SimplexOscillator{
	float phase = 0.0f;
	float freq = 0.0f;
	float isStepEOC = false;
  float buffer[BUFFER_LENGTH] = {0};
  int bufferIndex = 0;
  float min = 0.f;
  float max = 0.f;


  SimplexNoise simp;

	SimplexOscillator(){
	    simp.init();
	}

	bool reverse = false;
	void step(float dt){
		//phase+= freq;
		float delta = freq / dt;

		isStepEOC = false;
		if(!reverse){
			phase += delta;
			if (phase >= 0.5f)
				reverse = true;
		}
		else{
			phase -= delta;
			if(phase < 0){
				reverse = false;
				phase = -phase;
				isStepEOC = true;
			}
		}


		/*
		phase+= freq / dt;
		if (phase >= 1.0f){
			phase -= 1.0f;
			isStepEOC = true;
		}
		else{
			isStepEOC = false;
		}
		*/
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

  float getValue(float detaillevel, float x, float y, float z, float scale){
    return simp.SumOctave(detaillevel,x+phase,y,z,scale);
  }

	float getOsc(float detaillevel, float x, float y, float z, float scale){
			float value = getValue(detaillevel, x, y, z, scale);
			return value*5.f;
	}

  int t = 0;
  //float sum = 0.f;
  float getNormalizedOsc(float detaillevel, float x, float y, float z, float scale){

    float value = getValue(detaillevel, x, y, z, scale);

		if(bufferIndex >= BUFFER_LENGTH){
			bufferIndex = 0;
		}

  	buffer[bufferIndex] = value;
  	bufferIndex++;


		if(t++ % 256 == 0){
    	auto result = std::minmax_element(begin(buffer),end(buffer));
    	min = *result.first;
    	max = *result.second;
		}

    value = rescale(value,min,max,-1.f,1.f);
    return value*5.f;;

  }
};
