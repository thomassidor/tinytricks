#include "plugin.hpp"

const int NUM_CHANNELS = 16;
const int NUM_CHANNELS_PER_COLUMN = 8;

struct SH16 : Module {
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



struct SH16Widget : ModuleWidget {
  SH16Widget(SH16 *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/SH16.svg")));

    addInput(createInput<PJ301MPort>(mm2px(Vec(8.647f, 12.003f)), module, SH16::TRIG_INPUT));

    for (int i = 0; i < NUM_CHANNELS_PER_COLUMN; i++) {
      addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.321f, 29.859f + 11.5f * i)), module, SH16::HOLD_OUTPUT + i));
    }

    for (int i = 0; i < NUM_CHANNELS_PER_COLUMN; i++) {
      addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.063f, 29.859f + 11.5f * i)), module, SH16::HOLD_OUTPUT + NUM_CHANNELS_PER_COLUMN + i));
    }

    //Screws
    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));
  }
};


Model *modelSH16 = createModel<SH16, SH16Widget>("SH16");
