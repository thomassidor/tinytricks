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
    int nChan = std::max(1, inputs[A_INPUT].getChannels());
    for( int op=APLUSB_OUTPUT; op < NUM_OUTPUTS; op++ )
        outputs[op].setChannels(nChan);

    for( auto c=0; c<nChan; ++c )
    {
      if (inputs[A_INPUT].isConnected() && inputs[B_INPUT].isConnected()) {
        float a = inputs[A_INPUT].getVoltage(c);
        float b = inputs[B_INPUT].getPolyVoltage(c);

        outputs[APLUSB_OUTPUT].setVoltage(a+b, c);
        outputs[AVGAB_OUTPUT].setVoltage((a+b)/2, c);
        outputs[AMINUSB_OUTPUT].setVoltage(a-b, c);
        outputs[BMINUSA_OUTPUT].setVoltage(b-a, c);
        outputs[ADIVB_OUTPUT].setVoltage((b==0.f?0.f:a/b), c);
        outputs[BDIVA_OUTPUT].setVoltage((a==0.f?0.f:b/a), c);
        outputs[AMULB_OUTPUT].setVoltage(a*b, c);
        outputs[AEXPB_OUTPUT].setVoltage(pow(a,b), c);
        outputs[ONEOVERA_OUTPUT].setVoltage((a==0.f?0.f:1/a), c);
        outputs[ONEOVERB_OUTPUT].setVoltage((b==0.f?0.f:1/b), c);
        outputs[MINUSA_OUTPUT].setVoltage(-a, c);
        outputs[MINUSB_OUTPUT].setVoltage(-b, c);
      }
    }
  }
};



struct TTAWidget : TinyTricksModuleWidget {
  TTAWidget(TTA *module) {

    setModule(module);


    addInput(createInput<TinyTricksPortLight>(mm2px(Vec(3.2f, 12.003f)), module, TTA::A_INPUT));
    addInput(createInput<TinyTricksPortLight>(mm2px(Vec(14.088f, 12.003f)), module, TTA::B_INPUT));

    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.2f, 29.859f)), module, TTA::APLUSB_OUTPUT));
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(14.088f, 29.859f)), module, TTA::AVGAB_OUTPUT));

    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.2f, 46.21f)), module, TTA::AMINUSB_OUTPUT));
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(14.088f, 46.21f)), module, TTA::BMINUSA_OUTPUT));

    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.2f, 62.561f)), module, TTA::ADIVB_OUTPUT));
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(14.088f, 62.561f)), module, TTA::BDIVA_OUTPUT));

    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.2f, 78.911f)), module, TTA::AMULB_OUTPUT));
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(14.088f, 78.911f)), module, TTA::AEXPB_OUTPUT));

    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.2f, 95.262f)), module, TTA::ONEOVERA_OUTPUT));
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(14.088f, 95.262f)), module, TTA::ONEOVERB_OUTPUT));

    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.2f, 111.613f)), module, TTA::MINUSA_OUTPUT));
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(14.088f, 111.613f)), module, TTA::MINUSB_OUTPUT));

    InitializeSkin("TTA.svg");
  }
};


Model *modelTTA = createModel<TTA, TTAWidget>("TT-A");
