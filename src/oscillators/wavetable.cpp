//#include "../utility/Ramer-Douglas-Peucker.cpp"

struct WaveTableOscillatorWithRDP{

  static const int TABLE_LEVELS = 3;
  static const int TABLE_SIZE = 2048;
  int TABLE_END = TABLE_SIZE;
  float lookuptables[TABLE_LEVELS][TABLE_SIZE] = {0};
  

  float currentIndex = 0.f;
  float tableDelta = 0.f;

  bool mirror = false;

  WaveTableOscillatorWithRDP(){

  }

  void step(){

    /*
    if(mirror){
      if(!reverse){
        //phase += delta;
        if (phase >= 0.5f)
          reverse = true;
      }
      else{
        phase -= delta;
        if(phase < 0.f){
          reverse = false;
          phase = -phase;
          isStepEOC = true;
        }
      }
    }
    else{
      phase += delta;
      if (phase >= 1.0f){
        phase -= 1.0f;
      }
    }
    */

    //Adjusting currentIndex
    if ((currentIndex += tableDelta) > TABLE_END)
      currentIndex -= TABLE_END;
  }


  unsigned int tick = 0;
  float getSample(float y){
    //Getting indexes for current place in table
    int index0 = (int) currentIndex;
    int index1 = index0 == (TABLE_END - 1) ? (int) 0 : index0 + 1;

    // How far are we from the index
    float indexFrac = currentIndex - (float) index0;

    //Getting indexes for the levels based on y
    float frac = y * (TABLE_LEVELS-1);
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

    /*
    if(tick++%100000==0){
      std::cout << "------------------" << std::endl;
      std::cout << "currentIndex: " << currentIndex << std::endl;
      std::cout << "indexFrac: " << indexFrac << std::endl;

      std::cout << " " << std::endl;
      std::cout << "y: " << y << std::endl;
      std::cout << "level0: " << level0 << std::endl;
      std::cout << "level1: " << level1 << std::endl;
      std::cout << "levelFrac: " << levelFrac << std::endl;

      std::cout << " " << std::endl;
      std::cout << "Level0Value0: " << Level0Value0 << std::endl;
      std::cout << "Level0Value1: " << Level0Value1 << std::endl;
      std::cout << "interpolatedValueForLevel0: " << interpolatedValueForLevel0 << std::endl;

      std::cout << " " << std::endl;
      std::cout << "Level1Value0: " << Level1Value0 << std::endl;
      std::cout << "Level1Value1: " << Level1Value1 << std::endl;
      std::cout << "interpolatedValueForLevel1: " << interpolatedValueForLevel1 << std::endl;

      std::cout << " " << std::endl;
      std::cout << "finalValue: " << finalValue << std::endl;
      std::cout << "done" << std::endl;
    }
    */

    return finalValue;
  }

  void addSampleToFrame(float sampleValue, float y){
    lookuptables[yToLevel(y)][(int)currentIndex] = sampleValue;
  }

  unsigned int yToLevel(float y){
    return floor(y*(TABLE_LEVELS-1));
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
    /*
    for (int l = 0; l < TABLE_LEVELS; l++){
        for (int i = 0; i < 15; i++) {
        std::cout << lookuptables[l][i] << ", ";
      }
      std::cout <<  std::endl;
    }
    */
  }

  void setPitch(float pitch, float sampleRate){
    float frequency = dsp::FREQ_C4 * powf(2.0f, pitch);
    auto tableSizeOverSampleRate = TABLE_END / sampleRate;
    tableDelta = frequency * tableSizeOverSampleRate;
  }

  void setMirror(bool _mirror){
		mirror = _mirror;
		reset();
	}


  void reset(){
    currentIndex = 0.f;
  }

  void simplify(float simplificationLevel){

  }

  /*
  void createlookuptables(float detailLevel, float scale, float xstart, float y, float z, float degradation){
    vector<Point> pointList;
    vector<Point> pointListOut;
    float x = xstart;
    for (unsigned int i = 0; i < TABLE_SIZE; i++) {
      x += 0.01f;
      float value = simp.SumOctave(detailLevel,x,y,z,scale);
      //baselookuptables[i] = value;
      pointList.push_back(Point(i,value));
    }

    //RamerDouglasPeucker(pointList, degradation, pointListOut);

    int size = pointListOut.size();
    for(int i = 0; i < size; i++){
      lookuptables[i] = pointListOut[i].second;
      //cout << pointListOut[i].first << "," << pointListOut[i].second << endl;
    }

    //cout << "Size: " << size << endl;

    TABLE_END = size;
    currentIndex = 0.f;
  }
  */

};
