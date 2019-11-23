#include "plugin.hpp"
#include "shared/shared.cpp"
#include "oscillators/simplex.cpp"
#include "widgets/mini-scope.cpp"


const float SCALE_MAX = 5.5f;
const float SCALE_MIN = 0.5f;
const float DETAIL_MIN = 1.f;
const float DETAIL_MAX = 8.f;

struct SNOSC : TinyTricksModule {
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
		MIRROR_TRIGGER_INPUT,
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


	MiniScope* scope;

	SimplexOscillator oscillator;
	float prevPitch = 900000.f; //Crude fix for making sure that oscillators oscillate upon module init
	dsp::SchmittTrigger syncTrigger;
	dsp::SchmittTrigger mirrorButtonTrigger;
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

		AppendBaseJson(rootJ);
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		TinyTricksModule::dataFromJson(rootJ);

		// Mirror
		json_t *mirrorJ = json_object_get(rootJ, "mirror");
		if (mirrorJ)
			mirror = json_is_true(mirrorJ);

		oscillator.setMirror(mirror);
	}


	int ticksSinceScopeReset = 0;
  void process(const ProcessArgs &args) override {

		//Setting mirror
		if (mirrorButtonTrigger.process(params[MIRROR_PARAM].value) || (inputs[MIRROR_TRIGGER_INPUT].isConnected() && mirrorButtonTrigger.process(inputs[MIRROR_TRIGGER_INPUT].value))){
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
					scope->reset();
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
		scope->addFrame(value);


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
				scope->reset();
		}
		else{
			outputs[SYNC_OUTPUT].setVoltage(0.f);
		}
  }
};


struct SNOSCWidget : TinyTricksModuleWidget {
	//void appendContextMenu(Menu *menu) override;

	SNOSCWidget(SNOSC *module) {
		setModule(module);

		if(module){
			MiniScope *scope = new MiniScope();
			scope->box.pos = mm2px(Vec(3.571f, 9.0f));
			scope->box.size = mm2px(Vec(23.337f, 10.366f));
			scope->setGain(1.0f);
			addChild(scope);
			module->scope = scope;
		}
		else{
			SvgWidget* placeholder = createWidget<SvgWidget>(mm2px(Vec(3.571f, 11.0f)));
			placeholder->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/components/Wave.svg")));
			addChild(placeholder);
		}

		//Mirror buttons
		addParam(createParam<LEDButton>(mm2px(Vec(12.065f,25.062f)), module, SNOSC::MIRROR_PARAM));
		addChild(createLight<LargeLight<GreenLight>>(mm2px(Vec(12.065f+0.45f,25.062f+0.45f)), module, SNOSC::MIRROR_LIGHT));
		addInput(createInput<TinyTricksPort>(mm2px(Vec(20.759f,24.184f)), module, SNOSC::MIRROR_TRIGGER_INPUT));

		//Freq
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(11.24f,34.816f)), module, SNOSC::FREQ_PARAM));
		addInput(createInput<TinyTricksPort>(mm2px(Vec(20.759f,34.763f)), module, SNOSC::FREQ_CV_INPUT));

		//Fine
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(11.24f,45.395f)), module, SNOSC::FREQ_FINE_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(20.759f,45.342f)), module, SNOSC::FREQ_FINE_CV_INPUT));

		//X
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(11.24f,55.975f)), module, SNOSC::X_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(20.759f,55.922f)), module, SNOSC::X_CV_INPUT));

		//Y
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(11.24f,66.554f)), module, SNOSC::Y_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(20.759f,66.501f)), module, SNOSC::Y_CV_INPUT));

		//Scale
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(11.24f,77.133f)), module, SNOSC::SCALE_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(20.759f,77.08f)), module, SNOSC::SCALE_CV_INPUT));

		//Detail
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(11.24f,87.712f)), module, SNOSC::DETAIL_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(20.759f,87.659f)), module, SNOSC::DETAIL_CV_INPUT));

		//Sync
    addInput(createInput<TinyTricksPort>(mm2px(Vec(11.143f,98.238f)), module, SNOSC::SYNC_INPUT));

    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(4.617f,113.358f)), module, SNOSC::SYNC_OUTPUT));

		addOutput(createOutput<TinyTricksPort>(mm2px(Vec(17.669f,113.358f)), module, SNOSC::OSC_OUTPUT));

		InitializeSkin("SNOSC.svg");
	}
};


Model *modelSNOSC = createModel<SNOSC, SNOSCWidget>("SNOSC");
