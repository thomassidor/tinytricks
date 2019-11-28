#include <random>
#include <vector>
#include "plugin.hpp"
#include "shared/shared.cpp"
#include "widgets/curve-widget.cpp"
#include "oscillators/barebone.cpp"

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

  BareboneOscillator oscillator;

  const float RATE_MIN = -10.f;
  const float RATE_MAX = 10.f;
  float prevRate = 9000.f;

  const float SMOOTH_RATE_MIN = 0.f;
  const float SMOOTH_RATE_MAX = 1.f;
  float smoothRate = 0.f;
  float smoothFrac = 0.f;

  std::default_random_engine generator;
  std::piecewise_linear_distribution<float>* distributionLinear;
  std::piecewise_constant_distribution<float>* distributionConstant;
  std::vector<float> intervals;
  std::vector<float> weights;

  const float sensitivity = 0.01;

  bool isLiniearMode = true;

  dsp::SchmittTrigger trigTrigger;

  float fromOutValue = 5.f;
  float toOutValue = 5.f;
  float currentOutValue = 5.f;

  int tick = 0;

  CurveWidget* curve;

  std::vector<float> tmp;

  RANDOMWRANGLER() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    for (size_t i = 0; i < NUM_CURVE_POINTS; i++) {
      float initValue = RANDOM_MAX/(i+1);
      configParam(RANDOMWRANGLER::CURVE_PARAM + i, RANDOM_MIN, RANDOM_MAX, initValue, "Probability weight");
      weights.push_back(9000);
      tmp.push_back(9000);
    }

    configParam(RANDOMWRANGLER::RATE_PARAM, RATE_MIN, RATE_MAX, RATE_MAX, "Rate");
    configParam(RANDOMWRANGLER::SMOOTH_RATE_PARAM, SMOOTH_RATE_MIN, SMOOTH_RATE_MAX, SMOOTH_RATE_MIN, "Smoothing amout");
    configParam(RANDOMWRANGLER::LIN_SMOOTH_PARAM, 0.0f, 1.0f, 1.0f, "Smooth shape");

    initIntervals();
    //regenerateDistribution();
  }

  void onReset() override{
    processCurveParams(true);
  }

  void onRandomize() override{
    processCurveParams(true);
  }

  json_t *dataToJson() override {
		json_t *rootJ = json_object();
		// Mode
		json_object_set_new(rootJ, "isLiniearMode", json_boolean(isLiniearMode));

		AppendBaseJson(rootJ);
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		TinyTricksModule::dataFromJson(rootJ);

    //Mode
    json_t *isLiniearModeJ = json_object_get(rootJ, "isLiniearMode");
    if (isLiniearModeJ) isLiniearMode = json_is_true(isLiniearModeJ);
    curve->setMode(isLiniearMode);
    //processCurveParams(true);
  }

  void toggleMode(){
    isLiniearMode = !isLiniearMode;
    curve->setMode(isLiniearMode);
    regenerateDistribution();
  }

  void initIntervals(){
    intervals.reserve(NUM_CURVE_POINTS);
    for (size_t i = 0; i < NUM_CURVE_POINTS; i++) {
      float interval = (i/(float)(NUM_CURVE_POINTS-1))*RANDOM_MAX;
      intervals.push_back(interval);
    }
  }

  void regenerateDistribution(){
    //std::cout << "regenerating" << std::endl;
    if(isLiniearMode)
      distributionLinear = new std::piecewise_linear_distribution<float>(intervals.begin(),intervals.end(),weights.begin());
    else
      distributionConstant = new std::piecewise_constant_distribution<float>(intervals.begin(),intervals.end(),weights.begin());

    /*
    std::cout << "min: " << (*distributionLinear).min() << std::endl;
    std::cout << "max: " << (*distributionLinear).max() << std::endl;
    std::cout << "intervals : ";
    for (double x:(*distributionLinear).intervals()) std::cout << x << " ";
    std::cout << std::endl;
    std::cout << "densities : ";
    for (double x:(*distributionLinear).densities()) std::cout << x << " ";
    std::cout << std::endl;
    */
  }

  float getRandom(){
    if(isLiniearMode && distributionLinear != nullptr)
      return (*distributionLinear)(generator);
    else if(distributionConstant != nullptr)
      return (*distributionConstant)(generator);
    else
      return 0.f;
  }

  void updateCurve(){
    //Only update every 100 samples to increase performance
    if(tick%1000==0){
      tick = 0;
      processCurveParams(false);
    }
    tick++;
  }


  void processCurveParams(bool regenerate){
    bool dirty = false;
    for (size_t i = 0; i < NUM_CURVE_POINTS; i++) {
      float value = params[CURVE_PARAM + i].getValue();

      if(inputs[CURVE_CV_INPUT + i].isConnected())
        value += inputs[CURVE_CV_INPUT + i].getVoltage();
      value = clamp(value, 0.f, 10.f);

      if(abs(value-weights[i]) > sensitivity)
        dirty = true;

      tmp[i] = value;
    }
    if(dirty){
      curve->setPoints(tmp);
      if(regenerate){
        weights = tmp;
        regenerateDistribution();
      }
    }
  }

  void updateRate(){
    float rate = params[RATE_PARAM].getValue();
    if(inputs[RATE_CV_INPUT].isConnected())
      rate += inputs[RATE_CV_INPUT].getVoltage()*2.f;
    rate = clamp(rate, RATE_MIN, RATE_MAX);

    if(rate != prevRate){
      oscillator.setPitch(rate);
      //oscillator.reset();
      prevRate = rate;
    }
  }

  void updateSmoothRate(){
    smoothRate = params[SMOOTH_RATE_PARAM].getValue();
    if(inputs[SMOOTH_RATE_CV_INPUT].isConnected())
      smoothRate += inputs[SMOOTH_RATE_CV_INPUT].getVoltage()/10.f;
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

      if (params[LIN_SMOOTH_PARAM].getValue() != 1.f)
        frac = rescale(std::pow(50.f, frac), 1.f, 50.f, 0.f, 1.f);

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
    bool generate;
    if(inputs[TRIG_INPUT].isConnected())
      generate = trigTrigger.process(inputs[TRIG_INPUT].getVoltage());
    else
      generate = oscillator.isEOC();


    //Generating new value
    if(generate){
      processCurveParams(true);
      fromOutValue = toOutValue;
      toOutValue = getRandom();
      resetSmooth();
    }
    else{
      updateCurve();
    }

    //Getting smoothing to add
    currentOutValue = smooth(args.sampleTime);

    //Setting output
    outputs[NOISE_OUTPUT].setVoltage(currentOutValue-5.f);
  }
};



struct RANDOMWRANGLERWidget : TinyTricksModuleWidget {
  CurveWidget* curve;
  RANDOMWRANGLER* randModule;
  const float widgetSpacing = 10.807f;

  void appendContextMenu(Menu* menu) override {
		menu->addChild(new MenuEntry);
		menu->addChild(createMenuLabel("Mode"));

		struct LocalItem : MenuItem {
			RANDOMWRANGLER* module;
			void onAction(const event::Action& e) override {
				module->toggleMode();
			}
		};


		LocalItem* localItem = createMenuItem<LocalItem>("Constant distribution (instead of linear)");
		localItem->rightText = CHECKMARK(!randModule->isLiniearMode);
		localItem->module = randModule;
		menu->addChild(localItem);


		TinyTricksModuleWidget::appendContextMenu(menu);
	}

  RANDOMWRANGLERWidget(RANDOMWRANGLER* module) {
    if(module)
      randModule = module;

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


Model *modelRANDOMWRANGLER = createModel<RANDOMWRANGLER, RANDOMWRANGLERWidget>("RW");
