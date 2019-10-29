#include "plugin.hpp"
#include <iostream>
#include <cmath>
#include <utility>
#include <vector>
#include <stdexcept>
#include "utility/Ramer-Douglas-Peucker.cpp"
#include "oscillators/wavetable.cpp"


struct WAVE : Module {
	enum ParamIds {
  	PITCH_PARAM,
      DEGREDATION_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	WaveTableOscillatorWithRDP wTable;

  float prevPitch = 0.f;
  float prevDegredation = 0.f;


  void Initialize(){
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  	configParam(PITCH_PARAM, -3.0f, 3.0f, 0.0f, "Tuning");
    configParam(DEGREDATION_PARAM, 0.000001f, 0.3f, 0.000001f, "Degradation amount");
  }

	WAVE() {
		Initialize();
	}



  void process(const ProcessArgs &args) override{

    float pitch = params[PITCH_PARAM].getValue();
    float degradation = params[DEGREDATION_PARAM].getValue();

    if(degradation != prevDegredation){
      wTable.simplify(degradation);
      prevDegredation = degradation;
    }

    if(pitch != prevPitch){
      setFrequency(pitch,args.sampleRate);
      prevPitch = pitch;
    }

		wTable.step();

		float currentSample = wTable.getSample();

    outputs[AUDIO_OUTPUT].setVoltage(currentSample*5.f);

  }
};




struct WAVEWidget : ModuleWidget {
	WAVEWidget(WAVE *module) {
		setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Blank.svg")));
		//Screws
		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,11.051f)), module, WAVE::SCALE_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,23.613f)), module, WAVE::DETAIL_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,37.478f)), module, WAVE::STARTX_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,53.478f)), module, WAVE::PITCH_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,67.478f)), module, WAVE::DEGREDATION_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,79.478f)), module, WAVE::Y_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,89.478f)), module, WAVE::Z_PARAM));

		//Output
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.558f, 113.403f)), module, WAVE::AUDIO_OUTPUT));
	}
};

Model *modelWAVE = createModel<WAVE, WAVEWidget>("WAVE");
