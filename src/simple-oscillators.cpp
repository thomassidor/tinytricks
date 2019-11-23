#include "plugin.hpp"
#include "shared/shared.cpp"
#include "oscillators/oscillator.cpp"

struct TTOBase : TinyTricksModule {
	enum ParamIds {
		FREQ_PARAM,
    FREQ_FINE_PARAM,
    THETA_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FREQ_CV_INPUT,
    FREQ_FINE_CV_INPUT,
    THETA_CV_INPUT,
    SYNC_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OSC_OUTPUT,
    SYNC_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

  TinyOscillator oscillator;
  TinyOscillator::OscillatorType oscType;
  float prevPitch = 900000.f; //Crude fix for making sure that oscillators oscillate upon module init
  float prevTheta = 900000.f; //Crude fix for making sure that oscillators oscillate upon module init
  dsp::SchmittTrigger syncTrigger;

  void Initialize(){
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(TTOBase::FREQ_PARAM, -3.0f, 3.0f, 0.0f, "Tuning");
    configParam(TTOBase::FREQ_FINE_PARAM, -0.5f, 0.5f, 0.0f, "Fine tuning");
    configParam(TTOBase::THETA_PARAM, 0.0001f, 0.1f, 0.01f, "Theta (smoothness)");
  }

	TTOBase() {
		Initialize();
	}

  TTOBase(TinyOscillator::OscillatorType t){
    oscType = t;
    Initialize();
  }


  void process(const ProcessArgs &args) override{

    //Setting the pitch
  	float pitch = params[FREQ_PARAM].getValue();
    if(inputs[FREQ_CV_INPUT].isConnected())
      pitch += inputs[FREQ_CV_INPUT].getVoltage();
    pitch += params[FREQ_FINE_PARAM].getValue();
    if(inputs[FREQ_FINE_CV_INPUT].isConnected())
      pitch += inputs[FREQ_FINE_CV_INPUT].getVoltage()/5.f;
  	pitch = clamp(pitch, -3.5f, 3.5f);

    if(pitch != prevPitch){
      oscillator.setPitch(pitch);
      prevPitch = pitch;
    }


    //Setting Theta
    float theta = params[THETA_PARAM].getValue();

    if(inputs[THETA_CV_INPUT].isConnected())
      theta += inputs[THETA_CV_INPUT].getVoltage()/100.f;

    theta = clamp(theta,0.0001f,0.1f);

    if(theta != prevTheta){
      oscillator.setTheta(theta);
      prevTheta = theta;
    }



    //Stepping
    oscillator.step(args.sampleRate);

    //Resetting if synced
    if(inputs[SYNC_INPUT].isConnected() && syncTrigger.process(inputs[SYNC_INPUT].getVoltage()))
      oscillator.reset();


    //Getting the value
    float value = 0.f;
    switch (oscType) {
      case TinyOscillator::OscillatorType::SIN:
        value = oscillator.getSin();
        break;

      case TinyOscillator::OscillatorType::SAW:
        value = oscillator.getSaw();
        break;

      case TinyOscillator::OscillatorType::SQR:
        value = oscillator.getSqr();
        break;

      case TinyOscillator::OscillatorType::TRI:
        value = oscillator.getTri();
        break;
    }
    //Setting output
  	outputs[OSC_OUTPUT].setVoltage(value);

    if(outputs[SYNC_OUTPUT].isConnected())
        outputs[SYNC_OUTPUT].setVoltage(oscillator.isEOC() ? 10.f : 0.f);
  }
};




struct TTOBaseWidget : TinyTricksModuleWidget {

	TTOBaseWidget(TTOBase *module) {
		setModule(module);

    addInput(createInput<TinyTricksPort>(mm2px(Vec(3.567f,22.402f)), module, TTOBase::FREQ_CV_INPUT));

    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,11.05f)), module, TTOBase::FREQ_PARAM));
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.62f,35.626f)), module, TTOBase::FREQ_FINE_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(3.567f,44.86f)), module, TTOBase::FREQ_FINE_CV_INPUT));


    addInput(createInput<TinyTricksPort>(mm2px(Vec(3.567f,84.325f)), module, TTOBase::SYNC_INPUT));
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.567f,99.804f)), module, TTOBase::SYNC_OUTPUT));

		addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.567f,113.016f)), module, TTOBase::OSC_OUTPUT));

	}
};


// Sine --------------------------------------------------------------------------------------------------------------
struct TTOSin : TTOBase{
  TTOSin():TTOBase(TinyOscillator::OscillatorType::SIN){
  }
};

struct TTOSinWidget : TTOBaseWidget {
	TTOSinWidget(TTOBase *module) : TTOBaseWidget(module) {
		InitializeSkin("TTSIN.svg");
	}
};
Model *modelTTSIN = createModel<TTOSin, TTOSinWidget>("TTSIN");

// Saw --------------------------------------------------------------------------------------------------------------
struct TTOSaw : TTOBase{
  TTOSaw():TTOBase(TinyOscillator::OscillatorType::SAW){
  }
};

struct TTOSawWidget : TTOBaseWidget {
	TTOSawWidget(TTOBase *module) : TTOBaseWidget(module) {
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.62f,61.225f)), module, TTOBase::THETA_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(3.567f,70.476f)), module, TTOBase::THETA_CV_INPUT));
		InitializeSkin("TTSAW.svg");
	}
};
Model *modelTTSAW = createModel<TTOSaw, TTOSawWidget>("TTSAW");


// Square --------------------------------------------------------------------------------------------------------------
struct TTOSqr : TTOBase{
  TTOSqr():TTOBase(TinyOscillator::OscillatorType::SQR){
  }
};

struct TTOSqrWidget : TTOBaseWidget {
	TTOSqrWidget(TTOBase *module) : TTOBaseWidget(module) {
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.62f,61.225f)), module, TTOBase::THETA_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(3.567f,70.476f)), module, TTOBase::THETA_CV_INPUT));
		InitializeSkin("TTSQR.svg");
	}
};
Model *modelTTSQR = createModel<TTOSqr, TTOSqrWidget>("TTSQR");


// Square --------------------------------------------------------------------------------------------------------------
struct TTOTri : TTOBase{
  TTOTri():TTOBase(TinyOscillator::OscillatorType::TRI){
  }
};

struct TTOTriWidget : TTOBaseWidget {
	TTOTriWidget(TTOBase *module) : TTOBaseWidget(module) {
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.62f,61.225f)), module, TTOBase::THETA_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(3.567f,70.476f)), module, TTOBase::THETA_CV_INPUT));
		InitializeSkin("TTTRI.svg");
	}
};
Model *modelTTTRI = createModel<TTOTri, TTOTriWidget>("TTTRI");
