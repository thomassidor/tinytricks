#include <random>
#include <vector>
#include "plugin.hpp"
#include "shared/shared.cpp"
#include "widgets/curve-widget.cpp"
#include "oscillators/oscillator.cpp"

struct RANDOMWRANGLER : TinyTricksModule {

  static const size_t NUM_CURVE_POINTS = 9;

  enum ParamIds {
    ENUMS(CURVE_PARAM, NUM_CURVE_POINTS),
    RATE_PARAM,
    SMOOTH_RATE_PARAM,
    LIN_SMOOTH_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    ENUMS(CURVE_CV_INPUT, NUM_CURVE_POINTS),
    TRIG_INPUT,
    RATE_CV_INPUT,
    SMOOTH_RATE_CV_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    NOISE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  const float RANDOM_MIN = 0.f;
  const float RANDOM_MAX = 10.f;

  TinyOscillator oscillator;

  const float RATE_MIN = -10.f;
  const float RATE_MAX = 10.f;
  float prevRate = 9000.f;

  const float SMOOTH_RATE_MIN = 0.f;
  const float SMOOTH_RATE_MAX = 1.f;
  float smoothRate = 0.f;
  float smoothFrac = 0.f;

  std::default_random_engine generator;
  std::piecewise_linear_distribution<float>* distribution;
  std::vector<float> intervals;
  std::vector<float> weights;

  dsp::SchmittTrigger trigTrigger;

  float fromOutValue = 5.f;
  float toOutValue = 5.f;
  float currentOutValue = 5.f;

  CurveWidget* curve;
  std::vector<float> curveParams;

  RANDOMWRANGLER() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    for (size_t i = 0; i < NUM_CURVE_POINTS; i++) {
      float initValue = RANDOM_MAX/(i+1);
      configParam(RANDOMWRANGLER::CURVE_PARAM + i, RANDOM_MIN, RANDOM_MAX, initValue, "Point");
      curveParams.push_back(initValue);
    }

    configParam(RANDOMWRANGLER::RATE_PARAM, RATE_MIN, RATE_MAX, RATE_MAX, "Rate");
    configParam(RANDOMWRANGLER::SMOOTH_RATE_PARAM, SMOOTH_RATE_MIN, SMOOTH_RATE_MAX, SMOOTH_RATE_MIN, "Smoothing amout");
    configParam(RANDOMWRANGLER::LIN_SMOOTH_PARAM, 0.0f, 1.0f, 1.0f, "Smooth shape");

    processCurveParams();
    //regenerateDistribution();
  }


  void regenerateDistribution(){
    //std::cout << "regenerating" << std::endl;
    intervals.clear();
    intervals.reserve(NUM_CURVE_POINTS);

    weights.clear();
    weights.reserve(NUM_CURVE_POINTS);

    size_t points =  curveParams.size();
    //std::cout << "points: " << points << std::endl;

    for (size_t i = 0; i < points; i++) {
      float interval = (i/(float)(points-1))*RANDOM_MAX;
      float weight = curveParams[i];

      //std::cout << "interval: " << interval << std::endl;
      //std::cout << "weight: " << weight << std::endl;

      intervals.push_back(interval);
      weights.push_back(weight);
    }

    distribution = new std::piecewise_linear_distribution<float>(intervals.begin(),intervals.end(),weights.begin());

    /*
    std::cout << "min: " << (*distribution).min() << std::endl;
    std::cout << "max: " << (*distribution).max() << std::endl;
    std::cout << "intervals : ";
    for (double x:(*distribution).intervals()) std::cout << x << " ";
    std::cout << std::endl;
    std::cout << "densities : ";
    for (double x:(*distribution).densities()) std::cout << x << " ";
    std::cout << std::endl;
    */
  }

  float getRandom(){
    return (*distribution)(generator);
  }

  void processCurveParams(){
    std::vector<float> tmp;
    bool dirty = false;
    for (size_t i = 0; i < NUM_CURVE_POINTS; i++) {
      float value = params[CURVE_PARAM + i].getValue();

      if(inputs[CURVE_CV_INPUT + i].isConnected())
        value += inputs[CURVE_CV_INPUT + i].getVoltage();
      value = clamp(value, 0.f, 10.f);

      if(value != curveParams[i])
        dirty = true;

      tmp.push_back(value);
    }
    if(dirty){
      curveParams = tmp;
      regenerateDistribution();
      curve->setPoints(curveParams);
    }
  }

  void updateRate(){
    float rate = params[RATE_PARAM].getValue();
    if(inputs[RATE_CV_INPUT].isConnected())
      rate += inputs[RATE_CV_INPUT].getVoltage()*2.f;
    rate = clamp(rate, RATE_MIN, RATE_MAX);

    if(rate != prevRate){
      oscillator.setPitch(rate);
      oscillator.reset();
      prevRate = rate;
    }
  }

  void updateSmoothRate(){
    smoothRate = params[SMOOTH_RATE_PARAM].getValue();
    if(inputs[SMOOTH_RATE_CV_INPUT].isConnected())
      smoothRate += inputs[SMOOTH_RATE_CV_INPUT].getVoltage()*2.f;
    smoothRate = clamp(smoothRate, SMOOTH_RATE_MIN, SMOOTH_RATE_MAX);
  }


  void resetSmooth(){
    smoothFrac = 0.f;
    if(currentOutValue != toOutValue)
      fromOutValue = currentOutValue;
  }

  float smooth(float st){
    if(smoothRate == 0.f)
      return toOutValue;
    else{
      smoothFrac += st;

      float frac = smoothFrac / smoothRate;
      frac = clamp(frac,0.f,1.f);


      if (params[LIN_SMOOTH_PARAM].getValue() != 1.f) {
          frac = rescale(std::pow(50.f, frac), 1.f, 50.f, 0.f, 1.f);
      }


      float diff = toOutValue - fromOutValue;
      return fromOutValue + (diff*frac);
    }
  }

  void process(const ProcessArgs &args) override {
    //Updating values from knobs and CV
    updateRate();
    updateSmoothRate();

    //Stepping oscillator
    oscillator.step(args.sampleRate);

    //Determining whether to generate new value or not
    bool generate = false;
    if(inputs[TRIG_INPUT].isConnected())
      generate = trigTrigger.process(inputs[TRIG_INPUT].getVoltage());
    else
      generate = oscillator.isEOC();

    //Generating new value
    if(generate){
      processCurveParams();
      fromOutValue = toOutValue;
      toOutValue = getRandom();
      resetSmooth();
    }

    //Getting smoothing to add
    currentOutValue = smooth(args.sampleTime);

    //Setting output
    outputs[NOISE_OUTPUT].setVoltage(currentOutValue-5.f);
  }
};



struct RANDOMWRANGLERWidget : TinyTricksModuleWidget {
  CurveWidget* curve;
  const float widgetSpacing = 10.807f;
  RANDOMWRANGLERWidget(RANDOMWRANGLER *module) {
    setModule(module);

    //Top row
    for (size_t i = 0; i < 5; i++) {
      int index = i*2;
      addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(4.868f+i*widgetSpacing,10.312f)), module, RANDOMWRANGLER::CURVE_PARAM + index));
      addInput(createInput<TinyTricksPort>(mm2px(Vec(4.815f+i*widgetSpacing,19.545f)), module, RANDOMWRANGLER::CURVE_CV_INPUT + index));
    }

    //Curve widget
    curve = new CurveWidget();
    curve->box.pos = mm2px(Vec(8.868f, 33.396f));
    curve->box.size = mm2px(Vec(43.224f, 18.396f));
    addChild(curve);
    if(module)
      module->curve = curve;

    //Bottom row
    for (size_t i = 0; i < 4; i++) {
      int index = (i*2)+1;
      addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(10.271f+i*widgetSpacing,67.069f)), module, RANDOMWRANGLER::CURVE_PARAM + index));
      addInput(createInput<TinyTricksPort>(mm2px(Vec(10.218f+i*widgetSpacing,57.611f)), module, RANDOMWRANGLER::CURVE_CV_INPUT + index));
    }

    //Trigger
    addInput(createInput<TinyTricksPort>(mm2px(Vec(5.863f,87.153f)), module, RANDOMWRANGLER::TRIG_INPUT));

    //Rate
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(19.969f,87.153f)), module, RANDOMWRANGLER::RATE_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(19.915f,96.387f)), module, RANDOMWRANGLER::RATE_CV_INPUT));

    //Smooth rate
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(32.992f,87.153f)), module, RANDOMWRANGLER::SMOOTH_RATE_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(32.387f,96.387f)), module, RANDOMWRANGLER::SMOOTH_RATE_CV_INPUT));

    //Smooth shape
    addParam(createParam<CKSS>(mm2px(Vec(46.991f,87.565f)), module, RANDOMWRANGLER::LIN_SMOOTH_PARAM));


    //Output
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(26.427f,113.255f)), module, RANDOMWRANGLER::NOISE_OUTPUT));

    InitializeSkin("RW.svg");
  }
};


Model *modelRANDOMWRANGLER = createModel<RANDOMWRANGLER, RANDOMWRANGLERWidget>("RANDOMWRANGLER");
