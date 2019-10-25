#include "plugin.hpp"
#include "oscillators/simplex.cpp"
#include "widgets/mini-scope.cpp"


const float SCALE_MAX = 5.f;
const float SCALE_MIN = 0.005f;
const float DETAIL_MIN = 1.f;
const float DETAIL_MAX = 8.f;

struct SNOSC : Module, ScopedModule {
	enum ParamIds {
    SCALE_PARAM,
    DETAIL_PARAM,
		FREQ_PARAM,
		FREQ_FINE_PARAM,
    X_PARAM,
    Y_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FREQ_CV_INPUT,
    FREQ_FINE_CV_INPUT,
    SYNC_INPUT,
		X_CV_INPUT,
    Y_CV_INPUT,
		SCALE_CV_INPUT,
    DETAIL_CV_INPUT,
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


	SimplexOscillator oscillator;
	float prevPitch = 900000.f; //Crude fix for making sure that oscillators oscillate upon module init
	dsp::SchmittTrigger syncTrigger;

  void Initialize(){


    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(SCALE_PARAM, SCALE_MIN, SCALE_MAX, 0.5f, "Scale");
    configParam(DETAIL_PARAM, DETAIL_MIN, DETAIL_MAX, DETAIL_MIN, "Level of detail");
    configParam(X_PARAM, 0.f, 5.f, 0.f, "X modulation");
    configParam(Y_PARAM, 0.f, 1.f, 0.5f, "Y modulation");
		configParam(FREQ_PARAM, -3.0f, 3.0f, 0.0f, "Tuning");
    configParam(FREQ_FINE_PARAM, -0.5f, 0.5f, 0.0f, "Fine tuning");
  }

	SNOSC() {
		Initialize();
	}


  void process(const ProcessArgs &args) override {

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

		//Stepping
    oscillator.step(args.sampleRate);

    //Resetting if synced
    if(inputs[SYNC_INPUT].isConnected() && syncTrigger.process(inputs[SYNC_INPUT].getVoltage()))
      oscillator.reset();

		//Getting the value
		float scale = params[SCALE_PARAM].getValue();
		unsigned int detail = params[DETAIL_PARAM].getValue();
		float x = params[X_PARAM].getValue();
		float y = params[Y_PARAM].getValue();
    float value = oscillator.getOsc(detail, x, y, 0.5f, scale);

    //Setting output
  	outputs[OSC_OUTPUT].setVoltage(value);

		//Updating scope
		addFrameToScope(args.sampleRate, value);


		if(oscillator.isEOC()){
    	outputs[SYNC_OUTPUT].setVoltage(10.f);
			resetScope();
		}
		else{
			outputs[SYNC_OUTPUT].setVoltage(0.f);
		}
  }
};


struct SNOSCWidget : ModuleWidget {
	SNOSCWidget(SNOSC *module) {
		setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/SNOSC.svg")));

		MiniScope *scope = new MiniScope();
		scope->module = module;
		scope->box.pos = mm2px(Vec(1.571f, 6.0f));
		scope->box.size = mm2px(Vec(27.337f, 16.366f));
		addChild(scope);

		//Screws
		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		//Freq
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(10.673f,26.365f)), module, SNOSC::FREQ_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(20.312f,26.365f)), module, SNOSC::FREQ_CV_INPUT));

		//Fine
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(10.673f,38.182f)), module, SNOSC::FREQ_FINE_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(20.312f,38.182f)), module, SNOSC::FREQ_FINE_CV_INPUT));

		//X
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(10.673f,49.998f)), module, SNOSC::X_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(20.312f,49.998f)), module, SNOSC::X_CV_INPUT));

		//Y
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(10.673f,61.815f)), module, SNOSC::Y_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(20.312f,61.815f)), module, SNOSC::Y_CV_INPUT));

		//Scale
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(10.673f,73.632f)), module, SNOSC::SCALE_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(20.312f,73.632f)), module, SNOSC::SCALE_CV_INPUT));

		//Detail
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(10.673f,85.449f)), module, SNOSC::DETAIL_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(20.312f,85.449f)), module, SNOSC::DETAIL_CV_INPUT));

		//Sync
    addInput(createInput<PJ301MPort>(mm2px(Vec(10.673f,97.215f)), module, SNOSC::SYNC_INPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.661f,113.402f)), module, SNOSC::SYNC_OUTPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(17.713f,113.402f)), module, SNOSC::OSC_OUTPUT));
	}
};

Model *modelSNOSC = createModel<SNOSC, SNOSCWidget>("SNOSC");
