#include "plugin.hpp"
#include "shared/shared.cpp"
#include "oscillators/oscillator.cpp"


#define POLY_SIZE 16

const int OSC_COUNT = 3;
struct TTOBasePlus : TinyTricksModule {
  enum ParamIds {
    FREQ_PARAM,
    FREQ_FINE_PARAM,
    THETA_PARAM,
    DETUNE_PARAM,
    HARDSYNC2_PARAM,
    HARDSYNC3_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    FREQ_CV_INPUT,
    FREQ_FINE_CV_INPUT,
    THETA_CV_INPUT,
    DETUNE_CV_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    OSC_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    HARDSYNC2_LIGHT,
    HARDSYNC3_LIGHT,
    NUM_LIGHTS
  };

  TinyOscillator* oscillators[POLY_SIZE];
  TinyOscillator::OscillatorType oscType;
  dsp::SchmittTrigger hardsync2Trigger;
  dsp::SchmittTrigger hardsync3Trigger;
  bool hardsync2 = false;
  bool hardsync3 = false;
  float prevPitch[POLY_SIZE];
  float prevTheta[POLY_SIZE];
  float prevDetune[POLY_SIZE];


  void Initialize() {
    for (auto i = 0; i < POLY_SIZE; ++i) {
      oscillators[i] = new TinyOscillator[OSC_COUNT];
      prevPitch[i] = 90000.f;
      prevTheta[i] = 90000.f;
      prevDetune[i] = 90000.f;
    }

    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(TTOBasePlus::FREQ_PARAM, -3.0f, 3.0f, 0.0f, "Tuning");
    configParam(TTOBasePlus::FREQ_FINE_PARAM, -0.5f, 0.5f, 0.0f, "Fine tuning");
    configParam(TTOBasePlus::THETA_PARAM, 0.0001f, 0.1f, 0.01f, "Theta (smoothness)");
    configParam(TTOBasePlus::DETUNE_PARAM, 0.f, 1.f, 0.f, "Detuning");
    configButton(TTOBasePlus::HARDSYNC2_PARAM, "Sync oscillator 2 to oscillator 1");
    configButton(TTOBasePlus::HARDSYNC3_PARAM, "Sync oscillator 3 to oscillator 2");
    configInput(FREQ_CV_INPUT, "Tuning CV");
    configInput(FREQ_FINE_CV_INPUT, "Fine tuning CV");
    configInput(THETA_CV_INPUT, "Theta (smoothness) CV");
    configInput(DETUNE_CV_INPUT, "Detuning CV");
  }

  TTOBasePlus() {
    oscType = TinyOscillator::OscillatorType::SIN;
    Initialize();
  }

  TTOBasePlus(TinyOscillator::OscillatorType t) {
    oscType = t;
    Initialize();
  }

  ~TTOBasePlus() {
    for (auto i = 0; i < POLY_SIZE; ++i)
      delete[] oscillators[i];
  }

  json_t* dataToJson() override {
    json_t* rootJ = json_object();

    // Hardsync 2+3
    json_object_set_new(rootJ, "hardsync2", json_boolean(hardsync2));
    json_object_set_new(rootJ, "hardsync3", json_boolean(hardsync3));

    AppendBaseJson(rootJ);
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    TinyTricksModule::dataFromJson(rootJ);

    // hardsync2
    json_t* hardsync2J = json_object_get(rootJ, "hardsync2");
    if (hardsync2J)
      hardsync2 = json_is_true(hardsync2J);
    // hardsync2
    json_t* hardsync3J = json_object_get(rootJ, "hardsync3");
    if (hardsync3J)
      hardsync3 = json_is_true(hardsync3J);
  }


  void process(const ProcessArgs& args) override {
    // We want to use the FREQ_CV_INPUT to drive polyphony
    int nChan = std::max(1, inputs[FREQ_CV_INPUT].getChannels());
    outputs[OSC_OUTPUT].setChannels(nChan);

    // Hardsync is not polyphonic
    if (hardsync2Trigger.process(params[HARDSYNC2_PARAM].value)) {
      hardsync2 = !hardsync2;
    }
    if (hardsync3Trigger.process(params[HARDSYNC3_PARAM].value)) {
      hardsync3 = !hardsync3;
    }

    lights[HARDSYNC2_LIGHT].value = (hardsync2);
    lights[HARDSYNC3_LIGHT].value = (hardsync3);

    for (auto c = 0; c < nChan; ++c) {
      //Setting the pitch
      float pitch = params[FREQ_PARAM].getValue();
      if (inputs[FREQ_CV_INPUT].isConnected())
        pitch += inputs[FREQ_CV_INPUT].getVoltage(c);
      pitch += params[FREQ_FINE_PARAM].getValue();
      if (inputs[FREQ_FINE_CV_INPUT].isConnected())
        pitch += inputs[FREQ_FINE_CV_INPUT].getPolyVoltage(c) / 5.f;
      pitch = clamp(pitch, -3.5f, 3.5f);

      float detune = params[DETUNE_PARAM].getValue();
      if (inputs[DETUNE_CV_INPUT].isConnected())
        detune += (inputs[DETUNE_CV_INPUT].getPolyVoltage(c) + 5.f) / 10.f;

      bool pitchChanged = (pitch != prevPitch[c] || detune != prevDetune[c]);
      if (pitchChanged) {
        prevPitch[c] = pitch;
        prevDetune[c] = detune;
      }


      //Setting Theta
      float theta = params[THETA_PARAM].getValue();
      if (inputs[THETA_CV_INPUT].isConnected())
        theta += inputs[THETA_CV_INPUT].getPolyVoltage(c) / 100.f;
      theta = clamp(theta, 0.0001f, 0.1f);
      bool thetaChanged = (theta != prevTheta[c]);
      if (thetaChanged)
        prevTheta[c] = theta;


      //Looping oscillators
      float value = 0.f;
      for (int i = 0; i < OSC_COUNT; i++) {
        TinyOscillator* oscillator = &oscillators[c][i];

        if (pitchChanged)
          oscillator->setPitch(pitch + (detune * i));

        if (thetaChanged)
          oscillator->setTheta(theta);

        //Stepping
        oscillator->step(args.sampleRate);

        if (i > 0) {
          TinyOscillator* prevOscillator = &oscillators[c][i - 1];
          if (
            (i == 1 && hardsync2 && prevOscillator->isEOC()) ||
            (i == 2 && hardsync3 && prevOscillator->isEOC())
          )
            oscillator->reset();
        }


        //Getting the value
        switch (oscType) {
          case TinyOscillator::OscillatorType::SIN:
            value += oscillator->getSin() / OSC_COUNT;
            break;

          case TinyOscillator::OscillatorType::SAW:
            value += oscillator->getSaw() / OSC_COUNT;
            break;

          case TinyOscillator::OscillatorType::SQR:
            value += oscillator->getSqr() / OSC_COUNT;
            break;

          case TinyOscillator::OscillatorType::TRI:
            value += oscillator->getTri() / OSC_COUNT;
            break;
        }
      }
      //Setting output
      outputs[OSC_OUTPUT].setVoltage(value, c);
    }
  }
};




struct TTOBasePlusWidget : TinyTricksModuleWidget {

  TTOBasePlusWidget(TTOBasePlus* module) {
    setModule(module);

    addInput(createInput<TinyTricksPort>(mm2px(Vec(8.667f, 22.403f)), module, TTOBasePlus::FREQ_CV_INPUT));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(7.7f, 11.051f)), module, TTOBasePlus::FREQ_PARAM));

    //Fine tuning
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.f, 40.415f)), module, TTOBasePlus::FREQ_FINE_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(14.082f, 40.015f)), module, TTOBasePlus::FREQ_FINE_CV_INPUT));

    //Detune
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.f, 54.934f)), module, TTOBasePlus::DETUNE_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(14.082f, 54.534f)), module, TTOBasePlus::DETUNE_CV_INPUT));


    //Hard sync buttons
    addParam(createParam<LEDButton>(mm2px(Vec(3.825f, 91.955f)), module, TTOBasePlus::HARDSYNC2_PARAM));
    addChild(createLight<LargeLight<GreenLight>>(mm2px(Vec(3.825f + 0.45f, 91.955f + 0.45f)), module, TTOBasePlus::HARDSYNC2_LIGHT));

    addParam(createParam<LEDButton>(mm2px(Vec(15.004f, 91.955f)), module, TTOBasePlus::HARDSYNC3_PARAM));
    addChild(createLight<LargeLight<GreenLight>>(mm2px(Vec(15.004f + 0.45f, 91.955f + 0.45f)), module, TTOBasePlus::HARDSYNC3_LIGHT));

    //Output
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(8.804f, 113.016f)), module, TTOBasePlus::OSC_OUTPUT));


  }
};


// Sine --------------------------------------------------------------------------------------------------------------
struct TTOSinPlus : TTOBasePlus {
  TTOSinPlus(): TTOBasePlus(TinyOscillator::OscillatorType::SIN) {
  }
};

struct TTOSinPlusWidget : TTOBasePlusWidget {
  TTOSinPlusWidget(TTOBasePlus* module) : TTOBasePlusWidget(module) {
    InitializeSkin("TTSINPLUS.svg");
  }
};
Model* modelTTSINPLUS = createModel<TTOSinPlus, TTOSinPlusWidget>("TTSINPLUS");


// Saw --------------------------------------------------------------------------------------------------------------
struct TTOSawPlus : TTOBasePlus {
  TTOSawPlus(): TTOBasePlus(TinyOscillator::OscillatorType::SAW) {
  }
};

struct TTOSawPlusWidget : TTOBasePlusWidget {
  TTOSawPlusWidget(TTOBasePlus* module) : TTOBasePlusWidget(module) {
    //Theta
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.f, 69.452f)), module, TTOBasePlus::THETA_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(14.082f, 69.053f)), module, TTOBasePlus::THETA_CV_INPUT));

    InitializeSkin("TTSAWPLUS.svg");
  }
};
Model* modelTTSAWPLUS = createModel<TTOSawPlus, TTOSawPlusWidget>("TTSAWPLUS");


// Square --------------------------------------------------------------------------------------------------------------
struct TTOSqrPlus : TTOBasePlus {
  TTOSqrPlus(): TTOBasePlus(TinyOscillator::OscillatorType::SQR) {
  }
};

struct TTOSqrPlusWidget : TTOBasePlusWidget {
  TTOSqrPlusWidget(TTOBasePlus* module) : TTOBasePlusWidget(module) {
    //Theta
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.f, 69.452f)), module, TTOBasePlus::THETA_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(14.082f, 69.053f)), module, TTOBasePlus::THETA_CV_INPUT));

    InitializeSkin("TTSQRPLUS.svg");
  }
};
Model* modelTTSQRPLUS = createModel<TTOSqrPlus, TTOSqrPlusWidget>("TTSQRPLUS");


// Square --------------------------------------------------------------------------------------------------------------
struct TTOTriPlus : TTOBasePlus {
  TTOTriPlus(): TTOBasePlus(TinyOscillator::OscillatorType::TRI) {
  }
};

struct TTOTriPlusWidget : TTOBasePlusWidget {
  TTOTriPlusWidget(TTOBasePlus* module) : TTOBasePlusWidget(module) {
    //Theta
    addParam(createParam<RoundSmallBlackKnob>(mm2px(Vec(3.f, 69.452f)), module, TTOBasePlus::THETA_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(14.082f, 69.053f)), module, TTOBasePlus::THETA_CV_INPUT));

    InitializeSkin("TTTRIPLUS.svg");
  }
};
Model* modelTTTRIPLUS = createModel<TTOTriPlus, TTOTriPlusWidget>("TTTRIPLUS");
