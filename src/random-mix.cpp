#include "plugin.hpp"
#include "shared/shared.cpp"
#include "utility/SimplexNoise.hpp"

const int NUM_CHANNELS = 8;
const float SPEED_MAX = 1.f;
const float SPEED_MIN = 0.005f;
const float JITTER_MIN = 1.f;
const float JITTER_MAX = 8.f;

struct RX8Base : TinyTricksModule {

  enum ParamIds {
    SPEED_PARAM,
    JITTER_PARAM,
    TRIGONLY_PARAM,
    PINNING_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    ENUMS(AUDIO_L_INPUT, NUM_CHANNELS),
    ENUMS(AUDIO_R_INPUT, NUM_CHANNELS),
    TRIG_INPUT,
    SPEED_CV_INPUT,
    JITTER_CV_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    MIX_L_OUTPUT,
    MIX_R_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    ENUMS(LEVEL_LIGHT, NUM_CHANNELS),
    NUM_LIGHTS
  };

  dsp::SchmittTrigger trigger;
  float levels[NUM_CHANNELS] = {0};
  bool stereo = false;
  SimplexNoise simp;

  void initialize() {
    simp.init();
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(SPEED_PARAM, SPEED_MIN, SPEED_MAX, 0.5f, "Speed of change");
    configParam(JITTER_PARAM, JITTER_MIN, JITTER_MAX, JITTER_MIN, "Jitter of change");
    configSwitch(TRIGONLY_PARAM, 0.f, 1.f, 1.f, "Mode", {"Flow free", "Only change on trigger"});
    configParam(PINNING_PARAM, 1.f, 10.f, 1.5f, "Amount to pin at top of bottom of curve");
    configInput(TRIG_INPUT, "Trigger");
    configInput(SPEED_CV_INPUT, "Speed of change CV");
    configInput(JITTER_CV_INPUT, "Jitter of change CV");
  }


  RX8Base() {
    initialize();
  }

  RX8Base(bool isStereo) {
    stereo = isStereo;
    initialize();
  }



  float t = 0.f;
  bool reverse = false;
  float summedLevels = 0.f;
  void process(const ProcessArgs& args) override {
    bool freeflow = (params[TRIGONLY_PARAM].getValue() == 0.f);


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

    if (freeflow || (inputs[TRIG_INPUT].isConnected() && trigger.process(inputs[TRIG_INPUT].getVoltage()))) {
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
      summedLevels = 0.f;
      for (int i = 0; i < NUM_CHANNELS; i++) {
        if (inputs[AUDIO_L_INPUT + i].isConnected()) {
          float y = (2.f * i);
          float noiseVal = simp.SumOctave(jitter, t, y, 0.7f, speed);
          float level = clamp(noiseVal * (pinning), -1.f, 1.f);
          level *= level;
          summedLevels += level;
          levels[i] = level;
          lights[LEVEL_LIGHT + i].value = level;
        }
      }
    }


    //Mixing signal for output
    float mix = 0.f;
    int connected = 0;
    if (outputs[MIX_L_OUTPUT].isConnected()) {
      for (int i = 0; i < NUM_CHANNELS; i++) {
        if (inputs[AUDIO_L_INPUT + i].isConnected()) {
          connected++;
          mix += inputs[AUDIO_L_INPUT + i].getVoltage() * levels[i];
        }
      }
      if (connected == 1)
        outputs[MIX_L_OUTPUT].setVoltage(mix);
      else if (summedLevels > 0.f)
        outputs[MIX_L_OUTPUT].setVoltage(mix / summedLevels);
      else
        outputs[MIX_L_OUTPUT].setVoltage(0.f);

    }
  }
};


struct RX8BaseWidget : TinyTricksModuleWidget {
  RX8BaseWidget(RX8Base* module) {
    setModule(module);

    addInput(createInput<TinyTricksPortLight>(mm2px(Vec(3.977f, 12.003f)), module, RX8Base::TRIG_INPUT));

    for (int i = 0; i < NUM_CHANNELS; i++) {
      addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(9.641f, 35.995f + 11.6f * i)), module, RX8Base::LEVEL_LIGHT + i));
      addInput(createInput<TinyTricksPort>(mm2px(Vec(3.933f, 29.5f + 11.6f * i)), module, RX8Base::AUDIO_L_INPUT + i));
    }

    addParam(createParam<CKSS>(mm2px(Vec(19.981f, 10.992f)), module, RX8Base::TRIGONLY_PARAM));

    //Internal selection controls
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(17.45f, 30.677f)), module, RX8Base::SPEED_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(18.389f, 41.992f)), module, RX8Base::SPEED_CV_INPUT));

    addParam(createParam<RoundBlackKnob>(mm2px(Vec(17.45f, 58.239f)), module, RX8Base::JITTER_PARAM));
    addInput(createInput<TinyTricksPort>(mm2px(Vec(18.398f, 69.585f)), module, RX8Base::JITTER_CV_INPUT));

    /*{
        auto w = createParam<RoundBlackKnob>(mm2px(Vec(17.45f,58.239f)), module, RX8Base::JITTER_PARAM);
        dynamic_cast<Knob*>(w)->snap = true;
        addParam(w);
    }*/

    addParam(createParam<RoundBlackKnob>(mm2px(Vec(17.45f, 87.104f)), module, RX8Base::PINNING_PARAM));


    //Mix output
    addOutput(createOutput<TinyTricksPort>(mm2px(Vec(18.354f, 113.358f)), module, RX8Base::MIX_L_OUTPUT));

  }
};


// Mono --------------------------------------------------------------------------------------------------------------
struct RX8Mono : RX8Base {
  RX8Mono(): RX8Base(false) {
  }
};

struct RX8MonoWidget : RX8BaseWidget {
  RX8MonoWidget(RX8Base* module) : RX8BaseWidget(module) {
    InitializeSkin("RX8.svg");
  }
};
Model* modelRX8 = createModel<RX8Mono, RX8MonoWidget>("RX8");

// Stereo --------------------------------------------------------------------------------------------------------------
/*struct RM8Stereo : RX8Base{
  RM8Stereo():RX8Base(true){
  }
};

struct RM8StereoWidget : RX8BaseWidget {
	RM8StereoWidget(RX8Base *module) : RX8BaseWidget(module) {
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/RM8S.svg")));
    for (int i = 0; i < NUM_CHANNELS; i++){
      addInput(createInput<TinyTricksPort>(mm2px(Vec(17.788f, 12.003f + 14.f * i)), module, RX8Base::MUTE_L_INPUT + i));
      addInput(createInput<TinyTricksPort>(mm2px(Vec(26.994f, 12.003f + 14.f * i)), module, RX8Base::MUTE_R_INPUT + i));
      addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(36.199, 14.992 + 14.f * i)), module, RX8Base::MUTE_LIGHT + i));
      addOutput(createOutput<TinyTricksPort>(mm2px(Vec(39.567f, 12.003f + 14.f * i)), module, RX8Base::MUTE_L_OUTPUT  + i));
      addOutput(createOutput<TinyTricksPort>(mm2px(Vec(48.773f, 12.003f + 14.f * i)), module, RX8Base::MUTE_R_OUTPUT  + i));
    }
    //Screws
    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));
	}
};
Model *modelRM8S = createModel<RM8Stereo, RM8StereoWidget>("RM8S");
*/
