struct WaveTableOscillatorWithRDP{

  static const unsigned int TABLE_SIZE = 2048;
  unsigned int TABLE_END = TABLE_SIZE;
  float lookuptable[TABLE_SIZE];

  float currentIndex = 0.f;
  float tableDelta = 0.f;

  WaveTableOscillator(){

  }

  void step(){
    //Adjusting currentIndex
    if ((currentIndex += tableDelta) > TABLE_END)
      currentIndex -= TABLE_END;
  }

  float getSample(){
    //Getting indexes for current place in table
    unsigned int index0 = (unsigned int) currentIndex;
    unsigned int index1 = index0 == (TABLE_END - 1) ? (unsigned int) 0 : index0 + 1;

    // How far are we from the index
    auto frac = currentIndex - (float) index0;

    //Getting the two samples in the table
    float value0 = lookuptable[index0];
    float value1 = lookuptable[index1];

    //Interpolating between the two
    float currentSample = value0 + frac * (value1 - value0);
  }

  void setFrequency(float pitch, float sampleRate){
    float frequency = 440.f * powf(2.0f, pitch);
    //auto tableSizeOverSampleRate = TABLE_SIZE / sampleRate;
    auto tableSizeOverSampleRate = TABLE_END / sampleRate;
    tableDelta = frequency * tableSizeOverSampleRate;
  }

  void simplify(float simplificationLevel){

  }

  void createLookuptable(float detailLevel, float scale, float xstart, float y, float z, float degradation){
    vector<Point> pointList;
    vector<Point> pointListOut;
    float x = xstart;
    for (unsigned int i = 0; i < TABLE_SIZE; i++) {
      x += 0.01f;
      float value = simp.SumOctave(detailLevel,x,y,z,scale);
      //baseLookuptable[i] = value;
      pointList.push_back(Point(i,value));
    }

    RamerDouglasPeucker(pointList, degradation, pointListOut);

    int size = pointListOut.size();
    for(int i = 0; i < size; i++){
      lookuptable[i] = pointListOut[i].second;
      //cout << pointListOut[i].first << "," << pointListOut[i].second << endl;
    }

    //cout << "Size: " << size << endl;

    TABLE_END = size;
    currentIndex = 0.f;
  }

};
