#include "plugin.hpp"
#include "shared/shared.cpp"


struct TTL : TinyTricksModule {
  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    A_INPUT,
    B_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    AND_OUTPUT,
    OR_OUTPUT,
    XOR_OUTPUT,
    NOR_OUTPUT,
    ALTB_OUTPUT,
    AGTB_OUTPUT,
    ALTEB_OUTPUT,
    AGTEB_OUTPUT,
    AISB_OUTPUT,
    AISNOTB_OUTPUT,
    NOTA_OUTPUT,
    NOTB_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };


  TTL() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  }

  void process(const ProcessArgs &args) override {
  	if (inputs[A_INPUT].isConnected() && inputs[B_INPUT].isConnected()) {
      float a = inputs[A_INPUT].getVoltage();
      float b = inputs[B_INPUT].getVoltage();

      bool isA = (a!=0.f);
      bool isB = (b!=0.f);

      outputs[AND_OUTPUT].setVoltage((isA && isB?10.f:0.f));
      outputs[OR_OUTPUT].setVoltage((isA || isB?10.f:0.f));
      outputs[XOR_OUTPUT].setVoltage((isA != isB?10.f:0.f));
      outputs[NOR_OUTPUT].setVoltage((!(a || b)?10.f:0.f));
      outputs[ALTB_OUTPUT].setVoltage((a<b?10.f:0.f));
      outputs[AGTB_OUTPUT].setVoltage((a>b?10.f:0.f));
      outputs[ALTEB_OUTPUT].setVoltage((a<=b?10.f:0.f));
      outputs[AGTEB_OUTPUT].setVoltage((a>=b?10.f:0.f));
      outputs[AISB_OUTPUT].setVoltage((a==b?10.f:0.f));
      outputs[AISNOTB_OUTPUT].setVoltage((a!=b?10.f:0.f));
      outputs[NOTA_OUTPUT].setVoltage(!a);
      outputs[NOTB_OUTPUT].setVoltage(!b);



  	}
  }
};



struct TTLWidget : TinyTricksModuleWidget {
  TTLWidget(TTL *module) {
    setModule(module);
    InitializeSkin("TTL.svg");

    addInput(createInput<PJ301MPort>(mm2px(Vec(3.2f, 12.003f)), module, TTL::A_INPUT));
    addInput(createInput<PJ301MPort>(mm2px(Vec(14.088f, 12.003f)), module, TTL::B_INPUT));

    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.2f, 29.859f)), module, TTL::AND_OUTPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.088f, 29.859f)), module, TTL::OR_OUTPUT));

    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.2f, 46.21f)), module, TTL::XOR_OUTPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.088f, 46.21f)), module, TTL::NOR_OUTPUT));

    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.2f, 62.561f)), module, TTL::ALTB_OUTPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.088f, 62.561f)), module, TTL::AGTB_OUTPUT));

    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.2f, 78.911f)), module, TTL::ALTEB_OUTPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.088f, 78.911f)), module, TTL::AGTEB_OUTPUT));

    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.2f, 95.262f)), module, TTL::AISB_OUTPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.088f, 95.262f)), module, TTL::AISNOTB_OUTPUT));

    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.2f, 111.613f)), module, TTL::NOTA_OUTPUT));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(14.088f, 111.613f)), module, TTL::NOTB_OUTPUT));
  }
};


Model *modelTTL = createModel<TTL, TTLWidget>("TT-L");
