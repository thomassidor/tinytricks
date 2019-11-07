struct WaveTableOscillator{

  static const int WAVEFORM_COUNT = 3;
  static const int MAX_SAMPLE_COUNT = 2048;
  int TABLE_END = MAX_SAMPLE_COUNT;
  float lookuptables[WAVEFORM_COUNT][MAX_SAMPLE_COUNT] = {{0}};

  float currentIndex = 0.f;
  float tableDelta = 0.f;

	bool isStepEOC = false;

  bool mirror = false;
  bool reverse = false;

  float prevPitch = 90000.f;

  float phase = 0.f;
  float freq = 0.f;

  WaveTableOscillator(){

  }

  void step(){
    isStepEOC = false;

    if(mirror){
      if(!reverse){
        currentIndex += tableDelta;
        if (currentIndex >= TABLE_END/2.f)
          reverse = true;
      }
      else{
        currentIndex -= tableDelta;
        if(currentIndex < 0.f){
          reverse = false;
          currentIndex = 0.f;
          isStepEOC = true;
        }
      }
    }
    else{
      currentIndex += tableDelta;
      if (currentIndex >= TABLE_END){
        currentIndex -= TABLE_END;
        isStepEOC = true;
      }
    }
  }

  bool isEOC(){
		return isStepEOC;
	}

  float getSample(float y){

    //Getting indexes for current place in table
    int index0 = (int) currentIndex;
    int index1 = index0 == (TABLE_END - 1) ? (int) 0 : index0 + 1;

    // How far are we from the index
    float indexFrac = currentIndex - (float) index0;

    //Getting indexes for the levels based on y
    float frac = y * (WAVEFORM_COUNT-1);
    int level0 = floor(frac);
    int level1 = ceil(frac);
    float levelFrac = frac - (float) level0;

    //Getting the four samples in the table
    float Level0Value0 = lookuptables[level0][index0];
    float Level0Value1 = lookuptables[level0][index1];

    float Level1Value0 = lookuptables[level1][index0];
    float Level1Value1 = lookuptables[level1][index1];

    //Interpolating between the two
    float interpolatedValueForLevel0 = Level0Value0 + indexFrac * (Level0Value1 - Level0Value0);
    float interpolatedValueForLevel1 = Level1Value0 + indexFrac * (Level1Value1 - Level1Value0);

    float finalValue = interpolatedValueForLevel0 + levelFrac * (interpolatedValueForLevel1 - interpolatedValueForLevel0);

    return finalValue;
  }

  void addSampleToFrame(float sampleValue, float y){
    lookuptables[yToLevel(y)][(int)currentIndex] = sampleValue;
  }

  unsigned int yToLevel(float y){
    return floor(y*(WAVEFORM_COUNT-1));
  }

  void endFrame(){
    currentIndex = currentIndex + 1;
  }

  void startCapture(){
    reset();
  }

  void endCapture(){
    TABLE_END = currentIndex;
    reset();
  }

  void setPitch(float pitch, float sampleRate){
    if(pitch != prevPitch){
      float frequency = dsp::FREQ_C4 * powf(2.0f, pitch);
      auto tableSizeOverSampleRate = TABLE_END / sampleRate;
      tableDelta = frequency * tableSizeOverSampleRate;
      prevPitch = pitch;
    }
  }

  void setMirror(bool _mirror){
		mirror = _mirror;
		reset();
	}


  void reset(){
    currentIndex = 0.f;
  }


};
