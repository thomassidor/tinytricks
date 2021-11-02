#include "plugin.hpp"
#include "shared/shared.cpp"

const int NUM_CHANNELS = 16;
const int NUM_CHANNELS_PER_COLUMN = 8;

struct SH16 : TinyTricksModule {
  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    TRIG_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(HOLD_OUTPUT, NUM_CHANNELS),
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  dsp::SchmittTrigger trigger;

  SH16() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configInput(TRIG_INPUT, "Trigger");
    for (int i = 0; i < NUM_CHANNELS; i++) {
      configOutput(HOLD_OUTPUT + i, string::f("Hold %d", i + 1));
    }
  }
  void process(const ProcessArgs &args) override;
};


void SH16::process(const ProcessArgs &args) {
  if (inputs[TRIG_INPUT].isConnected() && trigger.process(inputs[TRIG_INPUT].getVoltage())) {
    for (int i = 0; i < NUM_CHANNELS; i++) {
      float v = (random::uniform() * 10.0) - 5.0f;
      //  std::cout << "v: " << v << std::endl;
      outputs[HOLD_OUTPUT + i].setVoltage(v);
    }
  }
}



struct SH16Widget : TinyTricksModuleWidget {
  SH16Widget(SH16 *module) {
    setModule(module);
    addInput(createInput<TinyTricksPortLight>(mm2px(Vec(8.647f, 12.003f)), module, SH16::TRIG_INPUT));

    for (int i = 0; i < NUM_CHANNELS_PER_COLUMN; i++)
      addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.321f, 29.859f + 11.5f * i)), module, SH16::HOLD_OUTPUT + i));

    for (int i = 0; i < NUM_CHANNELS_PER_COLUMN; i++)
      addOutput(createOutput<TinyTricksPort>(mm2px(Vec(14.063f, 29.859f + 11.5f * i)), module, SH16::HOLD_OUTPUT + NUM_CHANNELS_PER_COLUMN + i));

    InitializeSkin("SH16.svg");
  }
};


Model *modelSH16 = createModel<SH16, SH16Widget>("SH16");
