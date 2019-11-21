#include "plugin.hpp"
#include "shared/shared.cpp"

const int NUM_CHANNELS = 8;

struct A8 : TinyTricksModule {

  enum ParamIds {
    LEVEL_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    ENUMS(ATT_INPUT, NUM_CHANNELS),
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(ATT_OUTPUT, NUM_CHANNELS),
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };



  A8() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(LEVEL_PARAM, 0.f, 1.f, 1.f, "Attenuation level");
  }


  void process(const ProcessArgs &args) override {
    float level = params[LEVEL_PARAM].getValue();
    for (int i = 0; i < NUM_CHANNELS; i++) {
      if (inputs[ATT_INPUT + i].isConnected() && outputs[ATT_OUTPUT + i].isConnected())
        outputs[ATT_OUTPUT + i].setVoltage(inputs[ATT_INPUT + i].getVoltage() * level);
    }
	}
};





struct A8Widget : TinyTricksModuleWidget {
  A8Widget(A8 *module) {
    setModule(module);
    InitializeSkin("A8.svg");

    addParam(createParam<RoundBlackKnob>(mm2px(Vec(7.7f,11.055f)), module, A8::LEVEL_PARAM));

    for (int i = 0; i < NUM_CHANNELS; i++)
      addInput(createInput<PJ301MPort>(mm2px(Vec(3.131f, 29.859f + 11.5f * i)), module, A8::ATT_INPUT + i));

    for (int i = 0; i < NUM_CHANNELS; i++)
      addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.164f, 29.859f + 11.5f * i)), module, A8::ATT_OUTPUT  + i));
  }
};


Model *modelA8 = createModel<A8, A8Widget>("A8");
