#include "plugin.hpp"
#include "shared/shared.cpp"

const int NUM_CHANNELS = 8;

struct RM8Base : TinyTricksModule {

  enum ParamIds {
    MUTE_COUNT_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    TRIG_INPUT,
    MUTE_COUNT_CV_INPUT,
    ENUMS(MUTE_L_INPUT, NUM_CHANNELS),
    ENUMS(MUTE_R_INPUT, NUM_CHANNELS),
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(MUTE_L_OUTPUT, NUM_CHANNELS),
    ENUMS(MUTE_R_OUTPUT, NUM_CHANNELS),
    NUM_OUTPUTS
  };
  enum LightIds {
    ENUMS(MUTE_LIGHT, NUM_CHANNELS),
    NUM_LIGHTS
  };

  dsp::SchmittTrigger trigger;
  bool muted[NUM_CHANNELS] = {0};
  bool stereo = false;


  void initialize() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(MUTE_COUNT_PARAM, 0.f, NUM_CHANNELS, 0.f, "Number of channels to leave unmuted");
    configInput(TRIG_INPUT, "Trigger");
    configInput(MUTE_COUNT_CV_INPUT, "Mute CV");
    for (int i = 0; i < NUM_CHANNELS; i++) {
      if (stereo) {
        configInput(MUTE_L_INPUT + i, string::f("Channel L%d", i + 1));
        configOutput(MUTE_L_OUTPUT + i, string::f("Channel L%d", i + 1));
        configInput(MUTE_R_INPUT + i, string::f("Channel R%d", i + 1));
        configOutput(MUTE_R_OUTPUT + i, string::f("Channel R%d", i + 1));
      }
      else {
        configInput(MUTE_L_INPUT + i, string::f("Channel %d", i + 1));
        configOutput(MUTE_L_OUTPUT + i, string::f("Channel %d", i + 1));
      }
    }
  }


  RM8Base() {
    initialize();
  }

  RM8Base(bool isStereo) {
    stereo = isStereo;
    initialize();
  }


  void process(const ProcessArgs &args) override {

    if (inputs[TRIG_INPUT].isConnected() && trigger.process(inputs[TRIG_INPUT].getVoltage())) {
      int muteCount = 0;

      muteCount = params[MUTE_COUNT_PARAM].getValue();
      if (inputs[MUTE_COUNT_CV_INPUT].isConnected())
        muteCount += static_cast<int>(round(rescale(inputs[MUTE_COUNT_CV_INPUT].getVoltage(), -5.f, 5.f, 0.f, 8.f)));

      //Getting candidates for mutes = connected channels
      std::vector<int> candidates;
      for (int i = 0; i < NUM_CHANNELS; i++) {
        muted[i] = true;   //reset mutes while we're at it
        bool leftIsConnected = (inputs[MUTE_L_INPUT + i].isConnected() && outputs[MUTE_L_OUTPUT + i].isConnected());
        bool rightIsConnected = (stereo && (inputs[MUTE_R_INPUT + i].isConnected() && outputs[MUTE_R_OUTPUT + i].isConnected()));

        if (leftIsConnected || rightIsConnected)
          candidates.push_back(i);
      }

      int candidateCount = static_cast<int>(candidates.size());
      if (candidateCount > muteCount) {
        std::random_shuffle(candidates.begin(), candidates.end());
        for (int i = 0; i < candidateCount - muteCount; i++)
          muted[candidates[i]] = false;
      }
    }

    for (int i = 0; i < NUM_CHANNELS; i++) {
      bool leftIsConnected = (inputs[MUTE_L_INPUT + i].isConnected() && outputs[MUTE_L_OUTPUT + i].isConnected());
      bool rightIsConnected = (stereo && (inputs[MUTE_R_INPUT + i].isConnected() && outputs[MUTE_R_OUTPUT + i].isConnected()));

      if (!muted[i]) {
        lights[MUTE_LIGHT + i].value = 1.f;
        if (leftIsConnected)
          outputs[MUTE_L_OUTPUT + i].setVoltage(inputs[MUTE_L_INPUT + i].getVoltage());
        if (rightIsConnected)
          outputs[MUTE_R_OUTPUT + i].setVoltage(inputs[MUTE_R_INPUT + i].getVoltage());
      }
      else {
        lights[MUTE_LIGHT + i].value = 0.f;
        if (leftIsConnected)
          outputs[MUTE_L_OUTPUT + i].setVoltage(0.f);
        if (rightIsConnected)
          outputs[MUTE_R_OUTPUT + i].setVoltage(0.f);
      }

    }
  }
};


struct RM8BaseWidget : TinyTricksModuleWidget {
  RM8BaseWidget(RM8Base *module) {
    setModule(module);

    addInput(createInput<TinyTricksPortLight>(mm2px(Vec(3.847f, 12.003f)), module, RM8Base::TRIG_INPUT));

    {
      auto w = createParam<RoundSmallBlackKnob>(mm2px(Vec(3.9f, 31.62f)), module, RM8Base::MUTE_COUNT_PARAM);
      dynamic_cast<Knob *>(w)->snap = true;
      addParam(w);
    }
    addInput(createInput<TinyTricksPort>(mm2px(Vec(3.847, 41.251f)), module, RM8Base::MUTE_COUNT_CV_INPUT));
  }
};


// Mono --------------------------------------------------------------------------------------------------------------
struct RM8Mono : RM8Base {
  RM8Mono(): RM8Base(false) {
  }
};

struct RM8MonoWidget : RM8BaseWidget {
  RM8MonoWidget(RM8Base *module) : RM8BaseWidget(module) {
    for (int i = 0; i < NUM_CHANNELS; i++) {
      addInput(createInput<TinyTricksPort>(mm2px(Vec(17.424f, 11.782f + 14.f * i)), module, RM8Base::MUTE_L_INPUT + i));
      addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(26.209, 14.701 + 14.f * i)), module, RM8Base::MUTE_LIGHT + i));
      addOutput(createOutput<TinyTricksPort>(mm2px(Vec(29.122f, 11.782f + 14.f * i)), module, RM8Base::MUTE_L_OUTPUT  + i));
    }
    InitializeSkin("RM8.svg");
  }
};
Model *modelRM8 = createModel<RM8Mono, RM8MonoWidget>("RM8");

// Stereo --------------------------------------------------------------------------------------------------------------
struct RM8Stereo : RM8Base {
  RM8Stereo(): RM8Base(true) {
  }
};

struct RM8StereoWidget : RM8BaseWidget {
  RM8StereoWidget(RM8Base *module) : RM8BaseWidget(module) {
    for (int i = 0; i < NUM_CHANNELS; i++) {
      addInput(createInput<TinyTricksPort>(mm2px(Vec(17.788f, 12.003f + 14.f * i)), module, RM8Base::MUTE_L_INPUT + i));
      addInput(createInput<TinyTricksPort>(mm2px(Vec(26.994f, 12.003f + 14.f * i)), module, RM8Base::MUTE_R_INPUT + i));
      addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(36.199, 14.992 + 14.f * i)), module, RM8Base::MUTE_LIGHT + i));
      addOutput(createOutput<TinyTricksPort>(mm2px(Vec(39.567f, 12.003f + 14.f * i)), module, RM8Base::MUTE_L_OUTPUT  + i));
      addOutput(createOutput<TinyTricksPort>(mm2px(Vec(48.773f, 12.003f + 14.f * i)), module, RM8Base::MUTE_R_OUTPUT  + i));
    }
    InitializeSkin("RM8S.svg");
  }
};
Model *modelRM8S = createModel<RM8Stereo, RM8StereoWidget>("RM8S");
