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
    configInput(A_INPUT, "A");
    configInput(B_INPUT, "B");
    configOutput(AND_OUTPUT, "AND");
    configOutput(OR_OUTPUT, "OR");
    configOutput(XOR_OUTPUT, "XOR");
    configOutput(NOR_OUTPUT, "NOR");
    configOutput(ALTB_OUTPUT, "A<B");
    configOutput(AGTB_OUTPUT, "A>B");
    configOutput(ALTEB_OUTPUT, "A<=B");
    configOutput(AGTEB_OUTPUT, "A>=B");
    configOutput(AISB_OUTPUT, "A=B");
    configOutput(AISNOTB_OUTPUT, "A!=B");
    configOutput(NOTA_OUTPUT, "¬A");
    configOutput(NOTB_OUTPUT, "¬B");
  }

  void process(const ProcessArgs& args) override {
    int nChan = std::max(1, inputs[A_INPUT].getChannels());
    for (int op = AND_OUTPUT; op < NUM_OUTPUTS; op++)
      outputs[op].setChannels(nChan);

    for (auto c = 0; c < nChan; ++c) {
      if (inputs[A_INPUT].isConnected() && inputs[B_INPUT].isConnected()) {
        float a = inputs[A_INPUT].getVoltage(c);
        float b = inputs[B_INPUT].getPolyVoltage(c);

        bool isA = (a != 0.f);
        bool isB = (b != 0.f);

        outputs[AND_OUTPUT].setVoltage((isA && isB ? 10.f : 0.f), c);
        outputs[OR_OUTPUT].setVoltage((isA || isB ? 10.f : 0.f), c);
        outputs[XOR_OUTPUT].setVoltage((isA != isB ? 10.f : 0.f), c);
        outputs[NOR_OUTPUT].setVoltage((!(a || b) ? 10.f : 0.f), c);
        outputs[ALTB_OUTPUT].setVoltage((a < b ? 10.f : 0.f), c);
        outputs[AGTB_OUTPUT].setVoltage((a > b ? 10.f : 0.f), c);
        outputs[ALTEB_OUTPUT].setVoltage((a <= b ? 10.f : 0.f), c);
        outputs[AGTEB_OUTPUT].setVoltage((a >= b ? 10.f : 0.f), c);
        outputs[AISB_OUTPUT].setVoltage((a == b ? 10.f : 0.f), c);
        outputs[AISNOTB_OUTPUT].setVoltage((a != b ? 10.f : 0.f), c);
        outputs[NOTA_OUTPUT].setVoltage(!a, c);
        outputs[NOTB_OUTPUT].setVoltage(!b, c);
      }
    }
  }
};



struct TTLWidget : TinyTricksModuleWidget {
  TTLWidget(TTL* module) {
    setModule(module);


    addInput(createInput<TinyTricksPortLight>(mm2px(Vec(3.2f, 12.003f)), module, TTL::A_INPUT));
    addInput(createInput<TinyTricksPortLight>(mm2px(Vec(14.088f, 12.003f)), module, TTL::B_INPUT));

    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.2f, 29.859f)), module, TTL::AND_OUTPUT));
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(14.088f, 29.859f)), module, TTL::OR_OUTPUT));

    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.2f, 46.21f)), module, TTL::XOR_OUTPUT));
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(14.088f, 46.21f)), module, TTL::NOR_OUTPUT));

    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.2f, 62.561f)), module, TTL::ALTB_OUTPUT));
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(14.088f, 62.561f)), module, TTL::AGTB_OUTPUT));

    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.2f, 78.911f)), module, TTL::ALTEB_OUTPUT));
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(14.088f, 78.911f)), module, TTL::AGTEB_OUTPUT));

    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.2f, 95.262f)), module, TTL::AISB_OUTPUT));
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(14.088f, 95.262f)), module, TTL::AISNOTB_OUTPUT));

    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.2f, 111.613f)), module, TTL::NOTA_OUTPUT));
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(14.088f, 111.613f)), module, TTL::NOTB_OUTPUT));

    InitializeSkin("TTL.svg");
  }
};


Model* modelTTL = createModel<TTL, TTLWidget>("TT-L");
