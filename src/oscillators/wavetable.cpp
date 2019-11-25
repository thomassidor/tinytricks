struct WaveTable{
  static const int WAVEFORM_COUNT = 3;
  static const int MAX_SAMPLE_COUNT = 2048;
  int WAVETABLE_SIZE = MAX_SAMPLE_COUNT;
  float lookuptable[WAVEFORM_COUNT][MAX_SAMPLE_COUNT] = {{0}};
  int recordingIndex = 0;

  float getSample(float y, float x){

    //Getting indexes for current place in table
    int index0 = (int) x;
    int index1 = index0 == (WAVETABLE_SIZE - 1) ? (int) 0 : index0 + 1;

    // How far are we from the index
    float indexFrac = x - (float) index0;

    //Getting indexes for the levels based on y
    float frac = y * (WAVEFORM_COUNT-1);
    int level0 = floor(frac);
    int level1 = ceil(frac);
    float levelFrac = frac - (float) level0;

    //Getting the four samples in the table
    float Level0Value0 = lookuptable[level0][index0];
    float Level0Value1 = lookuptable[level0][index1];

    float Level1Value0 = lookuptable[level1][index0];
    float Level1Value1 = lookuptable[level1][index1];

    //Interpolating between the two
    float interpolatedValueForLevel0 = Level0Value0 + indexFrac * (Level0Value1 - Level0Value0);
    float interpolatedValueForLevel1 = Level1Value0 + indexFrac * (Level1Value1 - Level1Value0);

    float finalValue = interpolatedValueForLevel0 + levelFrac * (interpolatedValueForLevel1 - interpolatedValueForLevel0);

    return finalValue;
  }

  void reset(){
    recordingIndex = 0.f;
  }

  void startCapture(){
    reset();
  }

  void endCapture(){
    WAVETABLE_SIZE = recordingIndex;
    reset();
  }

  void addSampleToFrame(float sampleValue, int waveId){
    lookuptable[waveId][recordingIndex] = sampleValue;
  }

  void endFrame(){
    recordingIndex = recordingIndex + 1;
  }
};

struct WaveTableOscillator{
  WaveTable* waveTable;

  float currentIndex = 0.f;
  float tableDelta = 0.f;

	bool isStepEOC = false;

  bool mirror = false;
  bool reverse = false;

  float prevPitch = 90000.f;

  float phase = 0.f;
  float freq = 0.f;

  WaveTableOscillator(){
    waveTable = new  WaveTable();
  }

  float getSample(float y){
    if(waveTable == nullptr)
      return 0.f;
    else
      return waveTable->getSample(y,currentIndex);
  }

  void step(){
    if(waveTable == nullptr)
      return;

    isStepEOC = false;

    if(mirror){
      if(!reverse){
        currentIndex += tableDelta;
        if (currentIndex >= waveTable->WAVETABLE_SIZE/2.f)
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
      if (currentIndex >= waveTable->WAVETABLE_SIZE){
        currentIndex -= waveTable->WAVETABLE_SIZE;
        isStepEOC = true;
      }
    }
  }

  bool isEOC(){
		return isStepEOC;
	}

  void setPitch(float pitch, float sampleRate){
    if(waveTable == nullptr)
      return;

    if(pitch != prevPitch){
      float frequency = dsp::FREQ_C4 * powf(2.0f, pitch);
      auto tableSizeOverSampleRate = waveTable->WAVETABLE_SIZE / sampleRate;
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
