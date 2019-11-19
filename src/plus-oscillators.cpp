#include "plugin.hpp"
#include "shared/shared.cpp"
#include "oscillators/oscillator.cpp"


const int OSC_COUNT = 3;
struct TTOBasePlus : TinyTricksModule {
	enum ParamIds {
		FREQ_PARAM,
    FREQ_FINE_PARAM,
    THETA_PARAM,
		DETUNE_PARAM,
		HARDSYNC2_PARAM,
		HARDSYNC3_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FREQ_CV_INPUT,
    FREQ_FINE_CV_INPUT,
    THETA_CV_INPUT,
		DETUNE_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OSC_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		HARDSYNC2_LIGHT,
		HARDSYNC3_LIGHT,
		NUM_LIGHTS
	};

  TinyOscillator* oscillators;
  TinyOscillator::OscillatorType oscType;
	dsp::SchmittTrigger hardsync2Trigger;
	dsp::SchmittTrigger hardsync3Trigger;
	bool hardsync2 = false;
	bool hardsync3 = false;
  float prevPitch = 900000.f; //Crude fix for making sure that oscillators oscillate upon module init
  float prevTheta = 900000.f; //Crude fix for making sure that oscillators oscillate upon module init
	float prevDetune = 900000.f; //Crude fix for making sure that oscillators oscillate upon module init


  void Initialize(){
		oscillators = new TinyOscillator[OSC_COUNT];
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(TTOBasePlus::FREQ_PARAM, -3.0f, 3.0f, 0.0f, "Tuning");
    configParam(TTOBasePlus::FREQ_FINE_PARAM, -0.5f, 0.5f, 0.0f, "Fine tuning");
    configParam(TTOBasePlus::THETA_PARAM, 0.0001f, 0.1f, 0.01f, "Theta (smoothness)");
		configParam(TTOBasePlus::DETUNE_PARAM, 0.f, 1.f, 0.f, "Detuning");
		configParam(TTOBasePlus::HARDSYNC2_PARAM, 0.f, 1.f, 0.f, "Sync oscillator 2 to oscillator 1");
		configParam(TTOBasePlus::HARDSYNC3_PARAM, 0.f, 1.f, 0.f, "Sync oscillator 3 to oscillator 2");
  }

	TTOBasePlus() {
		Initialize();
	}

  TTOBasePlus(TinyOscillator::OscillatorType t){
    oscType = t;
    Initialize();
  }

	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		// Hardsync 2+3
		json_object_set_new(rootJ, "hardsync2", json_boolean(hardsync2));
		json_object_set_new(rootJ, "hardsync3", json_boolean(hardsync3));

		AppendBaseJson(rootJ);
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		TinyTricksModule::dataFromJson(rootJ);

		// hardsync2
		json_t *hardsync2J = json_object_get(rootJ, "hardsync2");
		if (hardsync2J)
			hardsync2 = json_is_true(hardsync2J);
		// hardsync2
		json_t *hardsync3J = json_object_get(rootJ, "hardsync3");
		if (hardsync3J)
			hardsync3 = json_is_true(hardsync3J);
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

		float detune = params[DETUNE_PARAM].getValue();
		if(inputs[DETUNE_CV_INPUT].isConnected())
			detune += (inputs[DETUNE_CV_INPUT].getVoltage()+5.f)/10.f;
		bool pitchChanged = (pitch != prevPitch || detune != prevDetune);
		if(pitchChanged){
	 	 prevPitch = pitch;
		 prevDetune = detune;
	 }


		//Setting Theta
		float theta = params[THETA_PARAM].getValue();
		if(inputs[THETA_CV_INPUT].isConnected())
		 theta += inputs[THETA_CV_INPUT].getVoltage()/100.f;
		theta = clamp(theta,0.0001f,0.1f);
		bool thetaChanged = (theta != prevTheta);
		if(thetaChanged)
		 prevTheta = theta;

		if (hardsync2Trigger.process(params[HARDSYNC2_PARAM].value)) {
 			hardsync2 = !hardsync2;
 		}
		if (hardsync3Trigger.process(params[HARDSYNC3_PARAM].value)) {
		 hardsync3 = !hardsync3;
	 	}

		lights[HARDSYNC2_LIGHT].value = (hardsync2);
		lights[HARDSYNC3_LIGHT].value = (hardsync3);


		//Looping oscillators
		float value = 0.f;
		for (int i = 0; i < OSC_COUNT; i++) {
			TinyOscillator *oscillator = &oscillators[i];

	    if(pitchChanged)
	      oscillator->setPitch(pitch+(detune*i));

			if(thetaChanged)
 			 oscillator->setTheta(theta);

	    //Stepping
	    oscillator->step(args.sampleRate);

			if(i>0){
				TinyOscillator *prevOscillator = &oscillators[i-1];
				if(
					(i==1 && hardsync2 && prevOscillator->isEOC())||
					(i==2 && hardsync3 && prevOscillator->isEOC())
				)
					oscillator->reset();
			}


	    //Getting the value
	    switch (oscType) {
	      case TinyOscillator::OscillatorType::SIN:
	        value += oscillator->getSin()/OSC_COUNT;
	        break;

	      case TinyOscillator::OscillatorType::SAW:
	        value += oscillator->getSaw()/OSC_COUNT;
	        break;

	      case TinyOscillator::OscillatorType::SQR:
	        value += oscillator->getSqr()/OSC_COUNT;
	        break;

	      case TinyOscillator::OscillatorType::TRI:
	        value += oscillator->getTri()/OSC_COUNT;
	        break;
	    }
		}
    //Setting output
  	outputs[OSC_OUTPUT].setVoltage(value);

  }
};




struct TTOBasePlusWidget : TinyTricksModuleWidget {

	TTOBasePlusWidget(TTOBasePlus *module) {
		setModule(module);

    addInput(createInput<PJ301MPort>(mm2px(Vec(8.711f,12.003f)), module, TTOBasePlus::FREQ_CV_INPUT));

		//Fine tuning
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(7.7f,27.542f)), module, TTOBasePlus::FREQ_PARAM));
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.f,46.765f)), module, TTOBasePlus::FREQ_FINE_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(14.126f,46.409f)), module, TTOBasePlus::FREQ_FINE_CV_INPUT));

		//Detune
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.f,61.284f)), module, TTOBasePlus::DETUNE_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(14.126f,60.928f)), module, TTOBasePlus::DETUNE_CV_INPUT));


		//Hard sync buttons
		addParam(createParam<LEDButton>(mm2px(Vec(3.825f,98.305f)), module, TTOBasePlus::HARDSYNC2_PARAM));
		addChild(createLight<LargeLight<GreenLight>>(mm2px(Vec(4.275f,98.755f)), module, TTOBasePlus::HARDSYNC2_LIGHT));

		addParam(createParam<LEDButton>(mm2px(Vec(15.004f,98.305f)), module, TTOBasePlus::HARDSYNC3_PARAM));
		addChild(createLight<LargeLight<GreenLight>>(mm2px(Vec(15.454f,98.755f)), module, TTOBasePlus::HARDSYNC3_LIGHT));

		//Output
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(8.848f,113.06f)), module, TTOBasePlus::OSC_OUTPUT));


	}
};


// Sine --------------------------------------------------------------------------------------------------------------
struct TTOSinPlus : TTOBasePlus{
  TTOSinPlus():TTOBasePlus(TinyOscillator::OscillatorType::SIN){
  }
};

struct TTOSinPlusWidget : TTOBasePlusWidget {
	TTOSinPlusWidget(TTOBasePlus *module) : TTOBasePlusWidget(module) {
		InitializeSkin("TTSINPLUS.svg");
	}
};
Model *modelTTSINPLUS = createModel<TTOSinPlus, TTOSinPlusWidget>("TTSINPLUS");


// Saw --------------------------------------------------------------------------------------------------------------
struct TTOSawPlus : TTOBasePlus{
  TTOSawPlus():TTOBasePlus(TinyOscillator::OscillatorType::SAW){
  }
};

struct TTOSawPlusWidget : TTOBasePlusWidget {
	TTOSawPlusWidget(TTOBasePlus *module) : TTOBasePlusWidget(module) {
		InitializeSkin("TTSAWPLUS.svg");
		//Theta
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.f,75.802f)), module, TTOBasePlus::THETA_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(14.126f,75.447f)), module, TTOBasePlus::THETA_CV_INPUT));
	}
};
Model *modelTTSAWPLUS = createModel<TTOSawPlus, TTOSawPlusWidget>("TTSAWPLUS");


// Square --------------------------------------------------------------------------------------------------------------
struct TTOSqrPlus : TTOBasePlus{
  TTOSqrPlus():TTOBasePlus(TinyOscillator::OscillatorType::SQR){
  }
};

struct TTOSqrPlusWidget : TTOBasePlusWidget {
	TTOSqrPlusWidget(TTOBasePlus *module) : TTOBasePlusWidget(module) {
		InitializeSkin("TTSQRPLUS.svg");
		//Theta
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.f,75.802f)), module, TTOBasePlus::THETA_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(14.126f,75.447f)), module, TTOBasePlus::THETA_CV_INPUT));
	}
};
Model *modelTTSQRPLUS = createModel<TTOSqrPlus, TTOSqrPlusWidget>("TTSQRPLUS");


// Square --------------------------------------------------------------------------------------------------------------
struct TTOTriPlus : TTOBasePlus{
  TTOTriPlus():TTOBasePlus(TinyOscillator::OscillatorType::TRI){
  }
};

struct TTOTriPlusWidget : TTOBasePlusWidget {
	TTOTriPlusWidget(TTOBasePlus *module) : TTOBasePlusWidget(module) {
		InitializeSkin("TTTRIPLUS.svg");
		//Theta
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.f,75.802f)), module, TTOBasePlus::THETA_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(14.126f,75.447f)), module, TTOBasePlus::THETA_CV_INPUT));
	}
};
Model *modelTTTRIPLUS = createModel<TTOTriPlus, TTOTriPlusWidget>("TTTRIPLUS");
