#include "plugin.hpp"
#include "shared/shared.cpp"
#include "utility/SimplexNoise.hpp"

const float SPEED_MAX = 5.f;
const float SPEED_MIN = 0.005f;
const float JITTER_MIN = 1.f;
const float JITTER_MAX = 8.f;
const int MAX_CHANNELS = 8;

struct SNBase : TinyTricksModule {

  enum ParamIds {
    SPEED_PARAM,
    JITTER_PARAM,
    TRIGONLY_PARAM,
    PINNING_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    SPEED_CV_INPUT,
    JITTER_CV_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(SIMPLEX_OUTPUT, MAX_CHANNELS),
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  int numChannels = 1;
  SimplexNoise simp;

  void initialize() {
    simp.init();

    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(SPEED_PARAM, SPEED_MIN, SPEED_MAX, 0.5f, "Speed of change");
    configParam(JITTER_PARAM, JITTER_MIN, JITTER_MAX, JITTER_MIN, "Jitter of change");
    configParam(PINNING_PARAM, 1.f, 10.f, 1.5f, "Amount to pin at top of bottom of curve");
    configInput(SPEED_CV_INPUT, "Speed CV");
    configInput(JITTER_CV_INPUT, "Jitter CV");
    for (int i = 0; i < numChannels; i++) {
      configOutput(SIMPLEX_OUTPUT + i, rack::string::f("Noise %d", i + 1));
    }
  }


  SNBase() {
    initialize();
  }

  SNBase(int _numChannels) {
    numChannels = _numChannels;
    initialize();
  }



  float t = 0.f;
  bool reverse = false;
  void process(const ProcessArgs &args) override {
    float delta = 1.0f / args.sampleRate;

    if (!reverse) {
      t += delta;
      if (t >= 128.f)
        reverse = true;
    }
    else {
      t -= delta;
      if (t < 0) {
        reverse = false;
        t = -t;
      }
    }


    //Getting pinning
    float pinning = params[PINNING_PARAM].getValue();

    //Getting the speed
    float speed = params[SPEED_PARAM].getValue();
    if (inputs[SPEED_CV_INPUT].isConnected()) {
      float speedCV = inputs[SPEED_CV_INPUT].getVoltage();
      speedCV /= 10.f;
      speed = clamp(speed + speedCV, SPEED_MIN, SPEED_MAX);
    }

    //Getting jitter
    float jitter = params[JITTER_PARAM].getValue();
    if (inputs[JITTER_CV_INPUT].isConnected()) {
      float jitterCV = inputs[JITTER_CV_INPUT].getVoltage();
      jitterCV = rescale(jitterCV, -5.f, 5.f, 0.f, 10.f);
      jitterCV /= 2.f;
      jitter = clamp(jitter + jitterCV, JITTER_MIN, JITTER_MAX);
    }

    //Getting new levels
    float x = t;
    for (int i = 0; i < numChannels; i++) {
      if (outputs[SIMPLEX_OUTPUT + i].isConnected()) {
        float y = (2.f * i);
        float noiseVal = simp.SumOctaveSmooth(jitter, x, y, 0.7f, speed);
        float level = clamp(noiseVal * (pinning), -1.f, 1.f);
        outputs[SIMPLEX_OUTPUT + i].setVoltage(level * 5.f);
      }
    }
  }
};


struct SNBaseWidget : TinyTricksModuleWidget {
  SNBaseWidget(SNBase *module) {
    setModule(module);

    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f, 11.051f)), module, SNBase::SPEED_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(3.567f, 22.366f)), module, SNBase::SPEED_CV_INPUT));

    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f, 38.613f)), module, SNBase::JITTER_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(3.567f, 49.96f)), module, SNBase::JITTER_CV_INPUT));

    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f, 67.478f)), module, SNBase::PINNING_PARAM));
  }
};


// 1x --------------------------------------------------------------------------------------------------------------
struct SN1 : SNBase {
  SN1(): SNBase(1) {
  }
};

struct SN1Widget : SNBaseWidget {
  SN1Widget(SNBase *module) : SNBaseWidget(module) {
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(3.523f, 113.403f)), module, SNBase::SIMPLEX_OUTPUT + 0));
    InitializeSkin("SN1.svg");
  }
};
Model *modelSN1 = createModel<SN1, SN1Widget>("SN1");


// 8x --------------------------------------------------------------------------------------------------------------
const int X8_CHANNELS = 8;
struct SN8 : SNBase {
  SN8(): SNBase(X8_CHANNELS) {
  }
};

struct SN8Widget : SNBaseWidget {
  SN8Widget(SNBase *module) : SNBaseWidget(module) {
    for (int i = 0; i < X8_CHANNELS; i++) {
      addOutput(createOutput<TinyTricksPort>(mm2px(Vec(18.501f, 12.003f + (i * 14.f))), module, SNBase::SIMPLEX_OUTPUT + i));
    }
    InitializeSkin("SN8.svg");
  }
};
Model *modelSN8 = createModel<SN8, SN8Widget>("SN8");
