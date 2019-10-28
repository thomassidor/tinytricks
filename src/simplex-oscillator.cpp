#include "plugin.hpp"
#include "oscillators/simplex.cpp"
#include "widgets/mini-scope.cpp"


const float SCALE_MAX = 5.5f;
const float SCALE_MIN = 0.5f;
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
		MIRROR_PARAM,
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
		MIRROR_LIGHT,
		NUM_LIGHTS
	};


	SimplexOscillator oscillator;
	float prevPitch = 900000.f; //Crude fix for making sure that oscillators oscillate upon module init
	dsp::SchmittTrigger syncTrigger;
	dsp::SchmittTrigger mirrorTrigger;
	bool mirror = false;

  void Initialize(){
		oscillator.setMirror(mirror);
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(SNOSC::SCALE_PARAM, SCALE_MIN, SCALE_MAX, 2.5f, "Scale");
    configParam(SNOSC::DETAIL_PARAM, DETAIL_MIN, DETAIL_MAX, DETAIL_MIN, "Level of detail");
    configParam(SNOSC::X_PARAM, 0.f, 5.f, 2.5f, "X modulation");
    configParam(SNOSC::Y_PARAM, 0.f, 5.f, 2.5f, "Y modulation");
		configParam(SNOSC::FREQ_PARAM, -3.0f, 3.0f, 0.0f, "Tuning");
    configParam(SNOSC::FREQ_FINE_PARAM, -0.5f, 0.5f, 0.0f, "Fine tuning");
		configParam(SNOSC::MIRROR_PARAM, 0.f, 1.f, 0.f, "Mirror waveform");
  }

	SNOSC() {
		Initialize();
	}

	//Got this approach from https://github.com/Miserlou/RJModules/blob/master/src/ChordSeq.cpp
	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		// Mirror
		json_object_set_new(rootJ, "mirror", json_boolean(mirror));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		// Mirror
		json_t *mirrorJ = json_object_get(rootJ, "mirror");
		if (mirrorJ)
			mirror = json_is_true(mirrorJ);

		oscillator.setMirror(mirror);
	}


	int ticksSinceScopeReset = 0;
  void process(const ProcessArgs &args) override {

		//Setting mirror
		if (mirrorTrigger.process(params[MIRROR_PARAM].value)) {
			mirror = !mirror;
			oscillator.setMirror(mirror);
		}
		lights[MIRROR_LIGHT].value = (mirror);

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

		//Getting scale
		float scale = params[SCALE_PARAM].getValue();
		if(inputs[SCALE_CV_INPUT].isConnected()){
			scale += inputs[SCALE_CV_INPUT].getVoltage()/4.f;
			scale = clamp(scale, SCALE_MIN, SCALE_MAX);
		}

		//Getting detail
		float detail = params[DETAIL_PARAM].getValue();
		if(inputs[DETAIL_CV_INPUT].isConnected()){
			detail += inputs[DETAIL_CV_INPUT].getVoltage()*0.8f;
			detail = clamp(detail, DETAIL_MIN, DETAIL_MAX);
		}

		//Getting x
		float x = params[X_PARAM].getValue();
		if(inputs[X_CV_INPUT].isConnected()){
			x += inputs[X_CV_INPUT].getVoltage()/4.f;
			x = clamp(x, 0.f, 5.f);
		}

		//Getting y
		float y = params[Y_PARAM].getValue();
		if(inputs[Y_CV_INPUT].isConnected()){
			y += inputs[Y_CV_INPUT].getVoltage()/4.f;
			y = clamp(y, 0.f, 5.f);
		}


		//Resetting if synced
		bool forwardSyncReset = false;
		if(inputs[SYNC_INPUT].isConnected()){
			ticksSinceScopeReset++;
			float voltage = inputs[SYNC_INPUT].getVoltage();
			if(syncTrigger.process(voltage)){
				oscillator.reset();
				forwardSyncReset = true;
				if(voltage >= 11.f){
					resetScope();
					ticksSinceScopeReset = 0;
				}
			}
		}
		else{
			ticksSinceScopeReset = 0;
		}

		//Stepping
		oscillator.step(args.sampleRate);
		//Getting result
    float value = oscillator.getNormalizedOsc(detail, x, y, 0.5f, scale);
    //Setting output
  	outputs[OSC_OUTPUT].setVoltage(value);
		//Updating scope
		addFrameToScope(args.sampleRate, value);


		//TODO: Clean up this logic. It's not pretty.
		if(forwardSyncReset){
			outputs[SYNC_OUTPUT].setVoltage(11.f);
		}
		else if(oscillator.isEOC()){
			if(!inputs[SYNC_INPUT].isConnected())
    		outputs[SYNC_OUTPUT].setVoltage(11.f);
			else
				outputs[SYNC_OUTPUT].setVoltage(10.f);
			// Normally we'll reset the scope on EOC,
			// but not if sync is connected - unless it's been too long since last sync
			if(!inputs[SYNC_INPUT].isConnected() || ticksSinceScopeReset > SimplexOscillator::BUFFER_LENGTH)
				resetScope();
		}
		else{
			outputs[SYNC_OUTPUT].setVoltage(0.f);
		}
  }
};


struct SNOSCWidget : ModuleWidget {
	//void appendContextMenu(Menu *menu) override;

	SNOSCWidget(SNOSC *module) {
		setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/SNOSC.svg")));

		MiniScope *scope = new MiniScope();
		scope->module = module;
		scope->box.pos = mm2px(Vec(1.571f, 6.0f));
		scope->box.size = mm2px(Vec(27.337f, 14.366f));
		addChild(scope);

		//Screws
		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		//Mirror buttons
		addParam(createParam<LEDButton>(mm2px(Vec(12.065f,19.191f)), module, SNOSC::MIRROR_PARAM));
		addChild(createLight<LargeLight<GreenLight>>(mm2px(Vec(12.515f,19.641f)), module, SNOSC::MIRROR_LIGHT));

		//Freq
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(11.24f,31.304f)), module, SNOSC::FREQ_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(20.803f,31.251f)), module, SNOSC::FREQ_CV_INPUT));

		//Fine
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(11.24f,42.415f)), module, SNOSC::FREQ_FINE_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(20.803f,42.362f)), module, SNOSC::FREQ_FINE_CV_INPUT));

		//X
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(11.24f,53.526f)), module, SNOSC::X_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(20.803f,53.473f)), module, SNOSC::X_CV_INPUT));

		//Y
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(11.24f,64.99f)), module, SNOSC::Y_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(20.803f,64.937f)), module, SNOSC::Y_CV_INPUT));

		//Scale
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(11.24f,76.101f)), module, SNOSC::SCALE_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(20.803f,76.048f)), module, SNOSC::SCALE_CV_INPUT));

		//Detail
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(11.24f,87.213f)), module, SNOSC::DETAIL_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(20.803f,87.16f)), module, SNOSC::DETAIL_CV_INPUT));

		//Sync
    addInput(createInput<PJ301MPort>(mm2px(Vec(11.24f,98.273f)), module, SNOSC::SYNC_INPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(4.661f,113.402f)), module, SNOSC::SYNC_OUTPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(17.713f,113.402f)), module, SNOSC::OSC_OUTPUT));
	}
};

/*
struct BaseFreqItem : MenuItem {
	SNOSC *osc;
	void onAction(EventAction &e) override {
			osc->freqFactor = tiare->freqFactor == 1 ? 100 : 1;
		}
	void step() override {
		rightText = tiare->freqFactor == 1 ? "OSC" : "LFO";
		MenuItem::step();
	}
};

void SNOSCWidget::appendContextMenu(Menu *menu) {
	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	SNOSC *osc = dynamic_cast<SNOSC*>(module);
	assert(tiare);

	BaseFreqItem *baseFreqItem = new BaseFreqItem();
	modeItem->text = "Use A4 as base frequency";
	modeItem->osc = osc;
	menu->addChild(modeItem);

	return menu;
}
*/


Model *modelSNOSC = createModel<SNOSC, SNOSCWidget>("SNOSC");
