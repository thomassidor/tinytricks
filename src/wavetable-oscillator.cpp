#include "plugin.hpp"
#include "oscillators/wavetable.cpp"
#include "widgets/mini-scope.cpp"


struct WAVE : Module {
	enum ParamIds {
  	FREQ_PARAM,
		FREQ_FINE_PARAM,
		Y_PARAM,
		CAPTURE_PARAM,
		MIRROR_PARAM,
    DETAIL_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FREQ_CV_INPUT,
		FREQ_FINE_CV_INPUT,
		Y_CV_INPUT,
		TOP_INPUT,
		MIDDLE_INPUT,
		BOTTOM_INPUT,
		SYNC_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		CAPTURE_LIGHT,
		MIRROR_LIGHT,
		NUM_LIGHTS
	};

	WaveTableOscillatorWithRDP oscillator;

  float prevPitch = 90000.f;
  float prevDetailLevel = 90000.f;

	dsp::SchmittTrigger syncTrigger;
	dsp::SchmittTrigger mirrorTrigger;
	dsp::SchmittTrigger captureTrigger;

	WaveTableScope* scope;

	bool mirror = false;
	bool capture = false;


  void Initialize(){
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  	configParam(WAVE::FREQ_PARAM, -3.0f, 3.0f, 0.0f, "Tuning");
		configParam(WAVE::FREQ_FINE_PARAM, -0.5f, 0.5f, 0.0f, "Fine tuning");
		configParam(WAVE::Y_PARAM, 0.0f, 1.0f, 0.5f, "Degradation amount");

		configParam(WAVE::CAPTURE_PARAM, 0.f, 1.f, 0.f, "Record new waveforms");
		configParam(WAVE::MIRROR_PARAM, 0.f, 1.f, 0.f, "Mirror waveform");
    configParam(WAVE::DETAIL_PARAM, 0.000001f, 0.3f, 0.000001f, "Degradation amount");
  }

	WAVE() {
		Initialize();
	}

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


	unsigned int ticksSinceRecordingStarted = 0;
  void process(const ProcessArgs &args) override{

		//Setting mirror
		if (mirrorTrigger.process(params[MIRROR_PARAM].value)) {
			mirror = !mirror;
			oscillator.setMirror(mirror);
			scope->stop();
		}
		lights[MIRROR_LIGHT].value = (mirror);


		//Setting capture
		if (captureTrigger.process(params[CAPTURE_PARAM].value)) {
			capture = true;
			oscillator.reset();
			ticksSinceRecordingStarted = 0;
		}
		lights[CAPTURE_LIGHT].value = (capture);

		if(capture){
			if(ticksSinceRecordingStarted == 0){
				oscillator.startCapture();
				scope->startCapture();
			}
			if(ticksSinceRecordingStarted < oscillator.TABLE_SIZE){

				float topV = inputs[TOP_INPUT].getNormalVoltage(0.0f);
				oscillator.addSampleToFrame(topV,1.0f);
				scope->addFrame(topV, 1.0f);

				float middleV = inputs[MIDDLE_INPUT].getNormalVoltage(0.0f);
				oscillator.addSampleToFrame(middleV,0.5f);
				scope->addFrame(middleV, 0.5f);


				float bottomV = inputs[BOTTOM_INPUT].getNormalVoltage(0.0f);
				oscillator.addSampleToFrame(bottomV,0.0f);
				scope->addFrame(bottomV, 0.0f);

				scope->endFrame();
				oscillator.endFrame();
				ticksSinceRecordingStarted++;
			}
			else{
				oscillator.endCapture();
				scope->endCapture();
				capture = false;
			}
		}

		/*
		//Setting detaillevel
    float detailLevel = params[DETAIL_PARAM].getValue();
    if(detailLevel != prevDetailLevel){
      wTable.simplify(detailLevel);
      prevDetailLevel = detailLevel;
    }
		*/

		//Setting the pitch
  	float pitch = params[FREQ_PARAM].getValue();
    if(inputs[FREQ_CV_INPUT].isConnected())
      pitch += inputs[FREQ_CV_INPUT].getVoltage();
    pitch += params[FREQ_FINE_PARAM].getValue();
    if(inputs[FREQ_FINE_CV_INPUT].isConnected())
      pitch += inputs[FREQ_FINE_CV_INPUT].getVoltage()/5.f;
  	pitch = clamp(pitch, -3.5f, 3.5f);
    if(pitch != prevPitch){
      oscillator.setPitch(pitch, args.sampleRate);
      prevPitch = pitch;
    }

		//Getting y
		float y = params[Y_PARAM].getValue();
		if(inputs[Y_CV_INPUT].isConnected()){
			y += inputs[Y_CV_INPUT].getVoltage()/5.f;
			y = clamp(y, 0.f, 1.f);
		}



		if(!capture){
			oscillator.step();
			float currentSample = oscillator.getSample(y);
    	outputs[AUDIO_OUTPUT].setVoltage(currentSample);
		}
		else
			outputs[AUDIO_OUTPUT].setVoltage(0.f);
  }
};




struct WAVEWidget : ModuleWidget {
	WAVEWidget(WAVE *module) {
		setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/WAVE.svg")));
		//Screws
		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		//Capture button
		addParam(createParam<LEDButton>(mm2px(Vec(7.869f,11.125f)), module, WAVE::CAPTURE_PARAM));
		addChild(createLight<LargeLight<GreenLight>>(mm2px(Vec(7.914f,11.170f)), module, WAVE::CAPTURE_LIGHT));

		//Inputs
		addInput(createInput<PJ301MPort>(mm2px(Vec(6.992f,28.474f)), module, WAVE::TOP_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(6.992f,43.887f)), module, WAVE::MIDDLE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(6.992f,59.301f)), module, WAVE::BOTTOM_INPUT));

		//Sync
		addInput(createInput<PJ301MPort>(mm2px(Vec(6.992f,73.141f)), module, WAVE::SYNC_INPUT));

		//Mirror button
		addParam(createParam<LEDButton>(mm2px(Vec(7.869f,90.138f)), module, WAVE::MIRROR_PARAM));
		addChild(createLight<LargeLight<GreenLight>>(mm2px(Vec(7.914f,90.183f)), module, WAVE::MIRROR_LIGHT));

		//Detail
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(7.045f,105.244f)), module, WAVE::DETAIL_PARAM));

		if(module){
			//Top scope
			WaveTableScope *scope = new WaveTableScope();
			scope->box.pos = mm2px(Vec(22.737f, 9.1f));
			scope->box.size = mm2px(Vec(35.807f, 110.354f));
			scope->initialize(3,3);
			addChild(scope);
			module->scope = scope;

		}


		//Freq
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(66.379f,11.125f)), module, WAVE::FREQ_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(67.327f,22.44f)), module, WAVE::FREQ_CV_INPUT));

		//Fine
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(66.379f,37.858f)), module, WAVE::FREQ_FINE_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(67.327f,49.205f)), module, WAVE::FREQ_FINE_CV_INPUT));

		//Y
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(66.379f,64.623f)), module, WAVE::Y_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(67.327f,75.97f)), module, WAVE::Y_CV_INPUT));

		//Output
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(67.327f, 113.402f)), module, WAVE::AUDIO_OUTPUT));
	}
};

Model *modelWAVE = createModel<WAVE, WAVEWidget>("WAVE");
