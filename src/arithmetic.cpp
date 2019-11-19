#include "plugin.hpp"
#include "shared/shared.cpp"


struct TTA : TinyTricksModule {
  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    A_INPUT,
    B_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    APLUSB_OUTPUT,
    AVGAB_OUTPUT,
    AMINUSB_OUTPUT,
    BMINUSA_OUTPUT,
    ADIVB_OUTPUT,
    BDIVA_OUTPUT,
    AMULB_OUTPUT,
    AEXPB_OUTPUT,
    ONEOVERA_OUTPUT,
    ONEOVERB_OUTPUT,
    MINUSA_OUTPUT,
    MINUSB_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };


  TTA() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  }

  void process(const ProcessArgs &args) override {
  	if (inputs[A_INPUT].isConnected() && inputs[B_INPUT].isConnected()) {
      float a = inputs[A_INPUT].getVoltage();
      float b = inputs[B_INPUT].getVoltage();

      outputs[APLUSB_OUTPUT].setVoltage(a+b);
      outputs[AVGAB_OUTPUT].setVoltage((a+b)/2);
      outputs[AMINUSB_OUTPUT].setVoltage(a-b);
      outputs[BMINUSA_OUTPUT].setVoltage(b-a);
      outputs[ADIVB_OUTPUT].setVoltage((b==0.f?0.f:a/b));
      outputs[BDIVA_OUTPUT].setVoltage((a==0.f?0.f:b/a));
      outputs[AMULB_OUTPUT].setVoltage(a*b);
      outputs[AEXPB_OUTPUT].setVoltage(pow(a,b));
      outputs[ONEOVERA_OUTPUT].setVoltage((a==0.f?0.f:1/a));
      outputs[ONEOVERB_OUTPUT].setVoltage((b==0.f?0.f:1/b));
      outputs[MINUSA_OUTPUT].setVoltage(-a);
      outputs[MINUSB_OUTPUT].setVoltage(-b);


  	}
  }
};



struct TTAWidget : TinyTricksModuleWidget {
  TTAWidget(TTA *module) {

    setModule(module);
    InitializeSkin("TTA.svg");

    addInput(createInput<PJ301MPort>(mm2px(Vec(3.2f, 12.003f)), module, TTA::A_INPUT));
    addInput(createInput<PJ301MPort>(mm2px(Vec(14.088f, 12.003f)), module, TTA::B_INPUT));

    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.2f, 29.859f)), module, TTA::APLUSB_OUTPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.088f, 29.859f)), module, TTA::AVGAB_OUTPUT));

    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.2f, 46.21f)), module, TTA::AMINUSB_OUTPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.088f, 46.21f)), module, TTA::BMINUSA_OUTPUT));

    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.2f, 62.561f)), module, TTA::ADIVB_OUTPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.088f, 62.561f)), module, TTA::BDIVA_OUTPUT));

    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.2f, 78.911f)), module, TTA::AMULB_OUTPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.088f, 78.911f)), module, TTA::AEXPB_OUTPUT));

    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.2f, 95.262f)), module, TTA::ONEOVERA_OUTPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.088f, 95.262f)), module, TTA::ONEOVERB_OUTPUT));

    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.2f, 111.613f)), module, TTA::MINUSA_OUTPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.088f, 111.613f)), module, TTA::MINUSB_OUTPUT));
  }
};


Model *modelTTA = createModel<TTA, TTAWidget>("TT-A");
