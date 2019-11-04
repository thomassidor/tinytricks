#include "plugin.hpp"
#include <vector>
#include "oscillators/wavetable.cpp"
#include "widgets/wavetable-scope.cpp"


struct WAVE : Module {
	enum ParamIds {
  	FREQ_PARAM,
		FREQ_FINE_PARAM,
		Y_MAIN_PARAM,
		CAPTURE_PARAM,
		MIRROR_PARAM,
		OSC2_ENABLE_PARAM,
		OSC2_SYNC_PARAM,
		OSC2_DETUNE_PARAM,
		OSC2_Y_PARAM,
		OSC3_ENABLE_PARAM,
		OSC3_SYNC_PARAM,
		OSC3_DETUNE_PARAM,
		OSC3_Y_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FREQ_CV_INPUT,
		FREQ_FINE_CV_INPUT,
		Y_MAIN_CV_INPUT,
		TOP_INPUT,
		MIDDLE_INPUT,
		BOTTOM_INPUT,
		SYNC_INPUT,
		OSC2_DETUNE_CV_INPUT,
		OSC2_Y_CV_INPUT,
		OSC3_DETUNE_CV_INPUT,
		OSC3_Y_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		CAPTURE_LIGHT,
		MIRROR_LIGHT,
		OSC2_ENABLE_LIGHT,
		OSC3_ENABLE_LIGHT,
		NUM_LIGHTS
	};

	WaveTableOscillator oscillatorMain;
	WaveTableOscillator oscillator2;
	WaveTableOscillator oscillator3;
	WaveTableScope* scope;



	dsp::SchmittTrigger syncTrigger;

	dsp::SchmittTrigger mirrorTrigger;
	bool mirror = false;

	dsp::SchmittTrigger inCaptureModeTrigger;
	bool inCaptureMode = false;

	dsp::SchmittTrigger osc2EnableTrigger;
	bool osc2Enabled = false;

	dsp::SchmittTrigger osc3EnableTrigger;
	bool osc3Enabled = false;


  void Initialize(){
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		//Recording
		configParam(WAVE::CAPTURE_PARAM, 0.f, 1.f, 0.f, "Record new waveforms");
		configParam(WAVE::MIRROR_PARAM, 0.f, 1.f, 0.f, "Mirror waveform");

		//Main oscillator
  	configParam(WAVE::FREQ_PARAM, -3.0f, 3.0f, 0.0f, "Tuning");
		configParam(WAVE::FREQ_FINE_PARAM, -0.5f, 0.5f, 0.0f, "Fine tuning");
		configParam(WAVE::Y_MAIN_PARAM, 0.0f, 1.0f, 0.5f, "Pos");

		//Oscillator 2
		configParam(WAVE::OSC2_DETUNE_PARAM, 0.0f, 1.0f, 0.0f, "Detune amount");
		configParam(WAVE::OSC2_Y_PARAM, -1.0f, 1.0f, 0.0f, "Pos");
		configParam(WAVE::OSC2_SYNC_PARAM, 0.0f, 1.0f, 0.0f, "Sync mode");

		//Oscillator 3
		configParam(WAVE::OSC3_DETUNE_PARAM, 0.0f, 1.0f, 0.0f, "Detune amount");
		configParam(WAVE::OSC3_Y_PARAM, -1.0f, 1.0f, 0.0f, "Pos");
		configParam(WAVE::OSC3_SYNC_PARAM, 0.0f, 2.0f, 0.0f, "Sync mode");

  }

	WAVE() {
		Initialize();
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		JSON_REAL_PRECISION(31);
		// Mirror
		json_object_set_new(rootJ, "mirror", json_boolean(mirror));
		json_object_set_new(rootJ, "osc2Enabled", json_boolean(osc2Enabled));
		json_object_set_new(rootJ, "osc3Enabled", json_boolean(osc3Enabled));

		//std::cout << "writing wavetable to json" <<std::endl;

		json_object_set_new(rootJ, "waveEnd", json_integer(oscillatorMain.TABLE_END));
		json_t *wavetableJ = json_array();
		for (int v = 0; v < WaveTableOscillator::WAVEFORM_COUNT; v++) {
			json_t *waveformJ = json_array();
			for (int s = 0; s < oscillatorMain.TABLE_END; s++) {
				json_t *sampleJ = json_real(oscillatorMain.lookuptables[v][s]);
				json_array_append_new(waveformJ, sampleJ);
			}
			json_array_append_new(wavetableJ, waveformJ);

		}
		json_object_set_new(rootJ, "wavetable", wavetableJ);
		//std::cout << "done writing" <<std::endl;
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		JSON_REAL_PRECISION(31);

		//Reading wavetable
		int waveEnd = 0;
		json_t *waveEndJ = json_object_get(rootJ, "waveEnd");
		if (waveEndJ)	waveEnd = (int)json_integer_value(waveEndJ);

		json_t *wavetableJ = json_object_get(rootJ, "wavetable");
		//std::cout << "reading json" <<std::endl;
		if(wavetableJ){
			oscillatorMain.startCapture();
			oscillator2.startCapture();
			oscillator3.startCapture();
			scope->startCapture();
			//std::cout << "wavetable is in json" <<std::endl;
			for (int s = 0; s < waveEnd; s++) {
				for (int v = 0; v < WaveTableOscillator::WAVEFORM_COUNT; v++) {
					json_t *waveJ = json_array_get(wavetableJ, v);
					if(waveJ){
						float y = v/(float)(WaveTableOscillator::WAVEFORM_COUNT-1);
						float value = json_number_value(json_array_get(waveJ, s));
						oscillatorMain.addSampleToFrame(value,y);
						oscillator2.addSampleToFrame(value,y);
						oscillator3.addSampleToFrame(value,y);
						scope->addFrame(value, y);
					}
				}
				oscillatorMain.endFrame();
				oscillator2.endFrame();
				oscillator3.endFrame();
				scope->endFrame();
			}
			oscillatorMain.endCapture();
			oscillator2.endCapture();
			oscillator3.endCapture();
			scope->endCapture();
		}

		// Mirror
		json_t *mirrorJ = json_object_get(rootJ, "mirror");
		if (mirrorJ) mirror = json_is_true(mirrorJ);
		lights[MIRROR_LIGHT].value = mirror;
		oscillatorMain.setMirror(mirror);
		oscillator2.setMirror(mirror);
		oscillator3.setMirror(mirror);
		scope->setMirror(mirror);

		//Osc2
		json_t *osc2EnabledJ = json_object_get(rootJ, "osc2Enabled");
		if (osc2EnabledJ) osc2Enabled = json_is_true(osc2EnabledJ);
		lights[OSC2_ENABLE_LIGHT].value = osc2Enabled;

		//Osc3
		json_t *osc3EnabledJ = json_object_get(rootJ, "osc3Enabled");
		if (osc3EnabledJ) osc3Enabled = json_is_true(osc3EnabledJ);
		lights[OSC3_ENABLE_LIGHT].value = osc3Enabled;
	}


	//Capture management
	unsigned int ticksSinceRecordingStarted = 0;
	bool useSync = false;
	bool recording = false;
	bool finalizeRecording = false;
	void manageinCaptureMode(){
		//Setting inCaptureMode
		if (!inCaptureMode && inCaptureModeTrigger.process(params[CAPTURE_PARAM].value)) {
			inCaptureMode = true;
			finalizeRecording = false;
			useSync = inputs[SYNC_INPUT].isConnected();

			oscillatorMain.startCapture();
			oscillator2.startCapture();
			oscillator3.startCapture();
			scope->startCapture();

			if(useSync)
				recording = false;
			else
				recording = true;

			ticksSinceRecordingStarted = 0;
		}
		lights[CAPTURE_LIGHT].value = inCaptureMode;

		if(inCaptureMode){
			//Starting recording on sync - if we use sync
			if(useSync && !recording && syncTrigger.process(inputs[SYNC_INPUT].getVoltage()))
				recording = true;
			//Stopping recording on second sync or if no more room for recording
			else if(
					(ticksSinceRecordingStarted >= WaveTableOscillator::MAX_SAMPLE_COUNT)
				||
					(useSync && recording && syncTrigger.process(inputs[SYNC_INPUT].getVoltage()))
				){
				recording = false;
				finalizeRecording = true;
			}
			if(recording){
				float topV = inputs[TOP_INPUT].getNormalVoltage(0.0f);
				oscillatorMain.addSampleToFrame(topV,1.0f);
				oscillator2.addSampleToFrame(topV,1.0f);
				oscillator3.addSampleToFrame(topV,1.0f);
				scope->addFrame(topV, 1.0f);

				float middleV = inputs[MIDDLE_INPUT].getNormalVoltage(0.0f);
				oscillatorMain.addSampleToFrame(middleV,0.5f);
				oscillator2.addSampleToFrame(middleV,0.5f);
				oscillator3.addSampleToFrame(middleV,0.5f);
				scope->addFrame(middleV, 0.5f);


				float bottomV = inputs[BOTTOM_INPUT].getNormalVoltage(0.0f);
				oscillatorMain.addSampleToFrame(bottomV,0.0f);
				oscillator2.addSampleToFrame(bottomV,0.0f);
				oscillator3.addSampleToFrame(bottomV,0.0f);
				scope->addFrame(bottomV, 0.0f);

				scope->endFrame();
				oscillatorMain.endFrame();
				oscillator2.endFrame();
				oscillator3.endFrame();

				ticksSinceRecordingStarted++;
			}

			if(finalizeRecording){
				oscillatorMain.endCapture();
				oscillator2.endCapture();
				oscillator3.endCapture();
				scope->endCapture();
				inCaptureMode = false;
				recording = false;
				finalizeRecording = false;
				oscillatorMain.prevPitch = 0.f;
				oscillator2.prevPitch = 0.f;
				oscillator3.prevPitch = 0.f;
			}
		}
	}


  void process(const ProcessArgs &args) override{
		//Setting mirror
		if (mirrorTrigger.process(params[MIRROR_PARAM].value)) {
			mirror = !mirror;
			oscillatorMain.setMirror(mirror);
			oscillator2.setMirror(mirror);
			oscillator3.setMirror(mirror);
			scope->setMirror(mirror);
		}
		lights[MIRROR_LIGHT].value = mirror;

		//Setting osc2 enable
		if (osc2EnableTrigger.process(params[OSC2_ENABLE_PARAM].value)) {
			osc2Enabled = !osc2Enabled;
			oscillator2.reset();
		}
		lights[OSC2_ENABLE_LIGHT].value = osc2Enabled;

		//Setting osc2 enable
		if (osc3EnableTrigger.process(params[OSC3_ENABLE_PARAM].value)) {
			osc3Enabled = !osc3Enabled;
			oscillator3.reset();
		}
		lights[OSC3_ENABLE_LIGHT].value = osc3Enabled;

		//Manage capture...
		manageinCaptureMode();

		if(!inCaptureMode){
			//Setting the pitch
	  	float pitch = params[FREQ_PARAM].getValue();
	    if(inputs[FREQ_CV_INPUT].isConnected())
	      pitch += inputs[FREQ_CV_INPUT].getVoltage();
	    pitch += params[FREQ_FINE_PARAM].getValue();
	    if(inputs[FREQ_FINE_CV_INPUT].isConnected())
	      pitch += inputs[FREQ_FINE_CV_INPUT].getVoltage()/5.f;
	  	pitch = clamp(pitch, -3.5f, 3.5f);
	    oscillatorMain.setPitch(pitch, args.sampleRate);


			float osc2Detune = pitch + params[OSC2_DETUNE_PARAM].getValue();
			if(inputs[OSC2_DETUNE_CV_INPUT].isConnected())
	      osc2Detune += inputs[OSC2_DETUNE_CV_INPUT].getVoltage()/5.f;
			oscillator2.setPitch(osc2Detune, args.sampleRate);


			float osc3Detune = osc2Detune + params[OSC3_DETUNE_PARAM].getValue();
			if(inputs[OSC3_DETUNE_CV_INPUT].isConnected())
	      osc3Detune += inputs[OSC3_DETUNE_CV_INPUT].getVoltage()/5.f;
			oscillator3.setPitch(osc3Detune, args.sampleRate);


			//Getting y
			float y = params[Y_MAIN_PARAM].getValue();
			if(inputs[Y_MAIN_CV_INPUT].isConnected()){
				y += inputs[Y_MAIN_CV_INPUT].getVoltage()/10.f;
				y = clamp(y, 0.f, 1.f);
			}

			float osc2Y = y + params[OSC2_Y_PARAM].getValue();
			if(inputs[OSC2_Y_CV_INPUT].isConnected())
				osc2Y += inputs[OSC2_Y_CV_INPUT].getVoltage()/10.f;
			osc2Y = clamp(osc2Y, 0.f, 1.f);


			float osc3Y = y + params[OSC3_Y_PARAM].getValue();
			if(inputs[OSC3_Y_CV_INPUT].isConnected())
				osc3Y += inputs[OSC3_Y_CV_INPUT].getVoltage()/10.f;
			osc3Y = clamp(osc3Y, 0.f, 1.f);

			//TODO: update to update if osc2 or osc3 y changes
			if(!osc2Enabled && !osc3Enabled)
				scope->setY1(y);
			else if (osc2Enabled && !osc3Enabled)
				scope->setY2(y,osc2Y);
			else if (!osc2Enabled && osc3Enabled)
				scope->setY2(y,osc3Y);
			else if (osc2Enabled && osc3Enabled)
				scope->setY3(y, osc2Y, osc3Y);

			//Stepping and syncing
			bool syncOsc2ToMain = (params[OSC2_SYNC_PARAM].getValue() == 1.f);
			int osc3SyncMode = params[OSC3_SYNC_PARAM].getValue();
			oscillatorMain.step();

			//Syncing osc2
			if(syncOsc2ToMain && oscillatorMain.isEOC())
				oscillator2.reset();
			oscillator2.step();

			if(osc3SyncMode != 0.f){
				if(osc3SyncMode == 1.f && osc2Enabled && oscillator2.isEOC())
					oscillator3.reset();
				else if(osc3SyncMode == 2.f && oscillatorMain.isEOC())
					oscillator3.reset();
			}
			oscillator3.step();

			//Getting samples
			int divisor = 1.f;
			float currentSample1 = oscillatorMain.getSample(y);
			float currentSample2 = 0.f;
			if(osc2Enabled){
				currentSample2 = oscillator2.getSample(osc2Y);
				divisor += 1.f;
			}
			float currentSample3 = 0.f;
			if(osc3Enabled){
				currentSample3 = oscillator3.getSample(osc3Y);
				divisor += 1.f;
			}
			float mix = (currentSample1+currentSample2+currentSample3)/divisor;
			//Setting the mix out
    	outputs[AUDIO_OUTPUT].setVoltage(mix);
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

		//inCaptureMode button
		addParam(createParam<LEDButton>(mm2px(Vec(7.164f,11.125f)), module, WAVE::CAPTURE_PARAM));
		addChild(createLight<LargeLight<GreenLight>>(mm2px(Vec(7.164f+0.45f,11.125f+0.45f)), module, WAVE::CAPTURE_LIGHT));

		//Inputs
		addInput(createInput<PJ301MPort>(mm2px(Vec(6.286f,28.474f)), module, WAVE::TOP_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(6.286f,43.887f)), module, WAVE::MIDDLE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(6.286f,59.301f)), module, WAVE::BOTTOM_INPUT));

		//Sync
		addInput(createInput<PJ301MPort>(mm2px(Vec(6.286f,73.141f)), module, WAVE::SYNC_INPUT));

		//Mirror button
		addParam(createParam<LEDButton>(mm2px(Vec(7.164f,90.138f)), module, WAVE::MIRROR_PARAM));
		addChild(createLight<LargeLight<GreenLight>>(mm2px(Vec(7.164f+0.45f,90.138f+0.45f)), module, WAVE::MIRROR_LIGHT));

		//Output
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(6.286f, 113.402f)), module, WAVE::AUDIO_OUTPUT));

		if(module){
			//Top scope
			WaveTableScope *scope = new WaveTableScope();
			scope->box.pos = mm2px(Vec(22.737f, 9.1f));
			scope->box.size = mm2px(Vec(35.807f, 110.354f));
			scope->initialize(3,10);
			addChild(scope);
			module->scope = scope;

		}


		// Main oscillator ------------------------------------------------
		//Freq
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(65.369f,17.068f)), module, WAVE::FREQ_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(66.317f,28.383f)), module, WAVE::FREQ_CV_INPUT));

		//Fine
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(78.85f,17.068f)), module, WAVE::FREQ_FINE_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(79.797f,28.383f)), module, WAVE::FREQ_FINE_CV_INPUT));

		//Y
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(92.33f,17.068f)), module, WAVE::Y_MAIN_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(93.277f,28.383f)), module, WAVE::Y_MAIN_CV_INPUT));

		// Oscillator 2 ------------------------------------------------
		//Enable
		addParam(createParam<LEDButton>(mm2px(Vec(67.195f,54.602f)), module, WAVE::OSC2_ENABLE_PARAM));
		addChild(createLight<LargeLight<GreenLight>>(mm2px(Vec(67.195f+0.45f,54.602f+0.45f)), module, WAVE::OSC2_ENABLE_LIGHT));

		//Sync
		addParam(createParam<CKSS>(mm2px(Vec(67.9f,68.698f)), module, WAVE::OSC2_SYNC_PARAM));

		//Detune
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(83.814f,53.777f)), module, WAVE::OSC2_DETUNE_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(93.277f,53.724f)), module, WAVE::OSC2_DETUNE_CV_INPUT));

		//Pos
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(83.814f,68.33f)), module, WAVE::OSC2_Y_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(93.277f,68.277f)), module, WAVE::OSC2_Y_CV_INPUT));

		// Oscillator 3 ------------------------------------------------
		//Enable
		addParam(createParam<LEDButton>(mm2px(Vec(67.195f,96.01f)), module, WAVE::OSC3_ENABLE_PARAM));
		addChild(createLight<LargeLight<GreenLight>>(mm2px(Vec(67.195f+0.45f,96.01f+0.45f)), module, WAVE::OSC3_ENABLE_LIGHT));

		//Sync
		addParam(createParam<CKSSThree>(mm2px(Vec(67.9f,109.738f)), module, WAVE::OSC3_SYNC_PARAM));

		//Detune
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(83.814f,95.186f)), module, WAVE::OSC3_DETUNE_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(93.277f,95.133f)), module, WAVE::OSC3_DETUNE_CV_INPUT));

		//Pos
		addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(83.814f,109.738f)), module, WAVE::OSC3_Y_PARAM));
		addInput(createInput<PJ301MPort>(mm2px(Vec(93.277f,109.685f)), module, WAVE::OSC3_Y_CV_INPUT));

	}
};

Model *modelWAVE = createModel<WAVE, WAVEWidget>("WAVE");
