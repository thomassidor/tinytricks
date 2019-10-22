#include "plugin.hpp"
#include "utility/SimplexNoise.hpp"

const int NUM_CHANNELS = 8;
const float SPEED_MAX = 1.f;
const float SPEED_MIN = 0.005f;
const float STABILITY_MAX = 1.f;
const float STABILITY_MIN = 8.f;

struct RX8Base : Module {

  enum ParamIds {
    SPEED_PARAM,
    STABILITY_PARAM,
    TRIGONLY_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    ENUMS(AUDIO_L_INPUT, NUM_CHANNELS),
    ENUMS(AUDIO_R_INPUT, NUM_CHANNELS),
    TRIG_INPUT,
    SPEED_CV_INPUT,
    STABILITY_CV_INPUT,
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

  void initialize(){
      simp.init();
      config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
      configParam(SPEED_PARAM, SPEED_MIN, SPEED_MAX, 0.05f, "Speed of change");
      configParam(STABILITY_PARAM, STABILITY_MIN, STABILITY_MAX, 1.f, "Stability of change");
      configParam(TRIGONLY_PARAM, 0.f, 1.f, 1.f, "Flow free or only change on trigger");
  }


  RX8Base() {
    initialize();
  }

  RX8Base(bool isStereo) {
    stereo = isStereo;
    initialize();
  }

  float sumOcatave(int num_iterations, float x, float y, float persistence, float scale){
    float maxAmp = 0.f;
    float amp = 1.f;
    float freq = scale;
    float noise = 0.f;


    for(int i = 0; i < num_iterations; ++i){
        noise += simp.noise(x * freq, y * freq) * amp;
        maxAmp += amp;
        amp *= persistence;
        freq *= 2.f;
      }

    //take the average value of the iterations
    noise /= maxAmp;

    return noise;
  }

  float t = 0.f;
  void process(const ProcessArgs &args) override {

    bool freeflow = (params[TRIGONLY_PARAM].getValue() == 0.f);
    int connected = 0;
    float summedLevels = 0.f;
    t += 1.0f / args.sampleRate;

    if(freeflow || (inputs[TRIG_INPUT].isConnected() && trigger.process(inputs[TRIG_INPUT].getVoltage()))){

      float speed = params[SPEED_PARAM].getValue();
      if(inputs[SPEED_CV_INPUT].isConnected())
        speed += inputs[SPEED_CV_INPUT].getVoltage();
      speed = clamp(speed,)

      float stability = params[STABILITY_PARAM].getValue();

      float x = t;
      for (int i = 0; i < NUM_CHANNELS; i++) {
        if(inputs[AUDIO_L_INPUT + i].isConnected()){
          connected++;
          float y = (2.f*i);
          float noiseVal = sumOcatave(stability,x,y,0.7f,speed);
          float level = clamp(noiseVal*2.5f,-1.5f,1.5f)/1.5f;
          level *= level;
          summedLevels += level;
          levels[i] = level;
          lights[LEVEL_LIGHT + i].value = level;
        }
      }
    }


    float mix = 0.f;
    if(outputs[MIX_L_OUTPUT].isConnected()){
      for (int i = 0; i < NUM_CHANNELS; i++) {
        if(inputs[AUDIO_L_INPUT + i].isConnected())
          mix += inputs[AUDIO_L_INPUT + i].getVoltage()*levels[i];
      }
      if(connected==1)
          outputs[MIX_L_OUTPUT].setVoltage(mix);
      else if(summedLevels>0.f)
        outputs[MIX_L_OUTPUT].setVoltage(mix/summedLevels);
      else
        outputs[MIX_L_OUTPUT].setVoltage(0.f);

    }
  }
};


struct RX8BaseWidget : ModuleWidget {
  RX8BaseWidget(RX8Base *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/RX8.svg")));

    addInput(createInput<PJ301MPort>(mm2px(Vec(3.977f, 12.003f)), module, RX8Base::TRIG_INPUT));

    for (int i = 0; i < NUM_CHANNELS; i++){
      addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(9.685f, 32.511f + 11.f * i)), module, RX8Base::LEVEL_LIGHT + i));
      addInput(createInput<PJ301MPort>(mm2px(Vec(3.977f, 26.016f + 11.f * i)), module, RX8Base::AUDIO_L_INPUT + i));
    }

    addParam(createParam<CKSS>(mm2px(Vec(19.981f,10.992f)), module, RX8Base::TRIGONLY_PARAM));

    //Internal selection controls
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(17.45f,30.677f)), module, RX8Base::SPEED_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(18.389f, 41.992f)), module, RX8Base::SPEED_CV_INPUT));

    addParam(createParam<RoundBlackKnob>(mm2px(Vec(17.45f,58.239f)), module, RX8Base::STABILITY_PARAM));
    addInput(createInput<PJ301MPort>(mm2px(Vec(18.398f, 69.585f)), module, RX8Base::STABILITY_CV_INPUT));
    /*
    {
        auto w = createParam<RoundBlackKnob>(mm2px(Vec(17.45f,49.768f)), module, RX8Base::STABILITY_PARAM);
        dynamic_cast<Knob*>(w)->snap = true;
        addParam(w);
    }
    */



    //Mix output
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(18.398f, 113.403f)), module, RX8Base::MIX_L_OUTPUT));

    //Screws
    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));
  }
};


// Mono --------------------------------------------------------------------------------------------------------------
struct RX8Mono : RX8Base{
  RX8Mono():RX8Base(false){
  }
};

struct RX8MonoWidget : RX8BaseWidget {
	RX8MonoWidget(RX8Base *module) : RX8BaseWidget(module) {
  }
};
Model *modelRX8 = createModel<RX8Mono, RX8MonoWidget>("RX8");

// Stereo --------------------------------------------------------------------------------------------------------------
/*struct RM8Stereo : RX8Base{
  RM8Stereo():RX8Base(true){
  }
};

struct RM8StereoWidget : RX8BaseWidget {
	RM8StereoWidget(RX8Base *module) : RX8BaseWidget(module) {
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/RM8S.svg")));
    for (int i = 0; i < NUM_CHANNELS; i++){
      addInput(createInput<PJ301MPort>(mm2px(Vec(17.788f, 12.003f + 14.f * i)), module, RX8Base::MUTE_L_INPUT + i));
      addInput(createInput<PJ301MPort>(mm2px(Vec(26.994f, 12.003f + 14.f * i)), module, RX8Base::MUTE_R_INPUT + i));
      addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(36.199, 14.992 + 14.f * i)), module, RX8Base::MUTE_LIGHT + i));
      addOutput(createOutput<PJ301MPort>(mm2px(Vec(39.567f, 12.003f + 14.f * i)), module, RX8Base::MUTE_L_OUTPUT  + i));
      addOutput(createOutput<PJ301MPort>(mm2px(Vec(48.773f, 12.003f + 14.f * i)), module, RX8Base::MUTE_R_OUTPUT  + i));
    }
    //Screws
    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));
	}
};
Model *modelRM8S = createModel<RM8Stereo, RM8StereoWidget>("RM8S");
*/
