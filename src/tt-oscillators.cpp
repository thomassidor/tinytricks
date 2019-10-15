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
      freq = 440.f * powf(2.0f, pitch);
    }


};


struct TTOBase : Module {
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
  float prevPitch = 0.f;
  float prevTheta = 0.f;
  dsp::SchmittTrigger syncTrigger;

  void Initialize(){
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(TTOBase::FREQ_PARAM, -3.0f, 3.0f, 0.0f, "Value", " V");
    configParam(TTOBase::FREQ_FINE_PARAM, -0.5f, 0.5f, 0.0f, "Value", "");
    configParam(TTOBase::THETA_PARAM, 0.0001f, 0.1f, 0.01f, "Value", "");
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

    //Resetting if synced
    if(inputs[SYNC_INPUT].isConnected() && syncTrigger.process(inputs[SYNC_INPUT].getVoltage()))
      oscillator.reset();

    //Stepping
    oscillator.step(args.sampleRate);


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
        outputs[SYNC_OUTPUT].setVoltage(oscillator.isEOC() ? 1.f : 0.f);
  }
};




struct TTOBaseWidget : ModuleWidget {

	TTOBaseWidget(TTOBase *module) {
		setModule(module);

    addInput(createInput<PJ301MPort>(mm2px(Vec(3.567f,11.912f)), module, TTOBase::FREQ_CV_INPUT));

    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,27.541f)), module, TTOBase::FREQ_PARAM));
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.62f,42.329f)), module, TTOBase::FREQ_FINE_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(3.567f,51.607f)), module, TTOBase::FREQ_FINE_CV_INPUT));


    addInput(createInput<PJ301MPort>(mm2px(Vec(3.567f,89.308f)), module, TTOBase::SYNC_INPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.567f,100.201f)), module, TTOBase::SYNC_OUTPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.567f,113.06f)), module, TTOBase::OSC_OUTPUT));

    //Screws
    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));
	}
};


// Sine --------------------------------------------------------------------------------------------------------------
struct TTOSin : TTOBase{
  TTOSin():TTOBase(TinyOscillator::OscillatorType::SIN){
  }
};

struct TTOSinWidget : TTOBaseWidget {
	TTOSinWidget(TTOBase *module) : TTOBaseWidget(module) {
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/TTSIN.svg")));
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
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/TTSAW.svg")));
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.62f,65.811f)), module, TTOBase::THETA_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(3.567f,75.106f)), module, TTOBase::THETA_CV_INPUT));
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
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/TTSQR.svg")));
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.62f,65.811f)), module, TTOBase::THETA_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(3.567f,75.106f)), module, TTOBase::THETA_CV_INPUT));
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
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/TTTRI.svg")));
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.62f,65.811f)), module, TTOBase::THETA_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(3.567f,75.106f)), module, TTOBase::THETA_CV_INPUT));
	}
};
Model *modelTTTRI = createModel<TTOTri, TTOTriWidget>("TTTRI");
