#include "plugin.hpp"
#include <cmath>
#include <string>
#include <iostream>
#include "lfo.cpp"


struct ModulationSource {
  const float FREQ_LOW_BOUND = -8.f;
	const float FREQ_HIGH_BOUND = 8.f;
	const float FREQ_MAX_SPREAD = 8.f;
	const float FREQ_MIDDLE = 0.f;

  LowFrequencyOscillator oscillator;
  float lfoWave;
  bool holding = false;
  float holdValue = 0.f;
  bool isOffset = false;

  float value = 0.f;

  ModulationSource(){}


  void regenerate(float rangeParam, float biasParam, bool shOn){

    //Randomly select S&H or LFO
    if(shOn){
      holding = (random::uniform() >= 0.5f);
    }
    else{
      holding = false;
    }

    //Generate S&H
    if(holding){
      //std::cout << "Regenerated: Holding" << std::endl;
      float spread = 5.f*rangeParam;
      float rescaledBiasParam = rescale(biasParam,-1.f,1.f,-5.f,5.f);
      float lowerRange = fmax(-5.f,rescaledBiasParam-spread);
      float upperRange = fmin(5.f,rescaledBiasParam+spread);

      holdValue = rescale(random::uniform(),0.f, 1.f,lowerRange,upperRange);

      if(isOffset){
        holdValue += 5.f;
      }

      //std::cout << "Value: " << holdValue << std::endl;

      value = holdValue;
    }
    //Generate LFO
    else{
      //std::cout << "Regenerated: LFO" << std::endl;
      //Choosing wave
  		lfoWave = random::uniform() * 3.0f; //should be between 0.f and 3.f

  		//Setting pitch
  		float spread = FREQ_MAX_SPREAD*rangeParam;
      float rescaledBiasParam = rescale(biasParam,-1.f,1.f,FREQ_LOW_BOUND,FREQ_HIGH_BOUND);
  		float lowerRange = fmax(FREQ_LOW_BOUND,rescaledBiasParam-spread);
  		float upperRange = fmin(FREQ_HIGH_BOUND,rescaledBiasParam+spread);

  		float pitch = rescale(random::uniform(),0.f, 1.f,lowerRange,upperRange);

      //std::cout << "Pitch: " << pitch << std::endl;
      //std::cout << "Waveform: " << lfoWave << std::endl;


  		oscillator.setPitch(pitch);
      oscillator.setPhase(random::normal());

  		//Resetting
  		oscillator.setReset(1.f);
    }
  }


  void setOffset(bool offset){
    if(!holding)
      oscillator.offset = offset;
    else if(offset != isOffset){
      if(offset)
        holdValue += 5.f;
      else
        holdValue -= 5.f;
      }
    //Saving offset state
    isOffset = offset;
  }


  bool tick(float timeDelta){
    //Tick LFO if not S&H
    if(!holding){
      oscillator.step(timeDelta);

      float v = 0.f;
      v += oscillator.sin() * fmax(0.f, 1.f - fabs(lfoWave - 0.f));
      v += oscillator.tri() * fmax(0.f, 1.f - fabs(lfoWave - 1.f));
      v += oscillator.saw() * fmax(0.f, 1.f - fabs(lfoWave - 2.f));
      v += oscillator.sqr() * fmax(0.f, 1.f - fabs(lfoWave - 3.f));
      value = 5.f * v;

      return true;

    }
    //S&H but offset changed
    else if(holdValue != value){
      value = holdValue;
      return true;
    }
    else  //Nothing changed
      return false;
  }


  float getValue(){
    return value;
  }
};



struct ModulationGeneratorBase : Module {

  enum ParamIds {
		OFFSET_PARAM,
		RANGE_PARAM,
		BIAS_PARAM,
    SH_ON_PARAM,
		NUM_PARAMS
	};
  public:
	enum InputIds {
		TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(MOD_OUTPUT,16),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};


  int outChannelsCount = 1;
	dsp::SchmittTrigger trigger;
  ModulationSource* modSources;

  void initializeModule(){
    modSources = new ModulationSource[outChannelsCount];

    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(OFFSET_PARAM, 0.f, 1.f, 0.f, "Offset");
    configParam(SH_ON_PARAM, 0.f, 1.f, 1.f, "Enable random S&H values");
    configParam(RANGE_PARAM, 0.f, 1.f, 1.f, "Frequency variance");
    configParam(BIAS_PARAM, -1.f, 1.f, 0.f, "Bias");
  }



	ModulationGeneratorBase() {
    initializeModule();
	}

  ModulationGeneratorBase(int c){
    outChannelsCount = c;
    initializeModule();
  }


  void process(const ProcessArgs &args) override {

    bool regenerate = inputs[TRIG_INPUT].isConnected() && trigger.process(inputs[TRIG_INPUT].getVoltage());
    bool offsetOn = (params[OFFSET_PARAM].getValue() == 1.f);

    float rangeParam = 1.f;
    float biasParam = 1.f;
    bool shOn;

    if (regenerate){
      rangeParam = params[RANGE_PARAM].getValue();
      biasParam = params[BIAS_PARAM].getValue();
      shOn = (params[SH_ON_PARAM].getValue() == 1.f);
    }

    for(int i = 0; i < outChannelsCount; i++){
      ModulationSource *modSource = &modSources[i];

      //Sending offset
      modSource->setOffset(offsetOn);

      //Triggered - generate new LFO or S&H
    	if (regenerate) {
        modSource->regenerate(rangeParam,biasParam,shOn);
    	}

      if(outputs[MOD_OUTPUT + i].isConnected())
      {
        if(modSource->tick(1.0f / args.sampleRate)){
          float newValue = modSource->getValue();
          //std::cout << "New Value: " << newValue << std::endl;
          outputs[MOD_OUTPUT + i].setVoltage(newValue);
        }
      }
    }
  }

};


struct ModulationGeneratorBaseWidget : ModuleWidget {
	ModulationGeneratorBaseWidget(ModulationGeneratorBase *module) {
		setModule(module);

		addInput(createInput<PJ301MPort>(mm2px(Vec(3.567f,12.003f)), module, ModulationGeneratorBase::TRIG_INPUT));

		addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,29.749f)), module, ModulationGeneratorBase::RANGE_PARAM));
		addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,49.743f)), module, ModulationGeneratorBase::BIAS_PARAM));

    addParam(createParam<CKSS>(mm2px(Vec(5.151f,70.697f)), module, ModulationGeneratorBase::SH_ON_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(5.151f,88.025f)), module, ModulationGeneratorBase::OFFSET_PARAM));

    //Screws
    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));
	}
};





// X1 --------------------------------------------------------------------------------------------------------------
struct ModulationGeneratorX1 : ModulationGeneratorBase{
  ModulationGeneratorX1():ModulationGeneratorBase(1){
  }
};

struct ModulationGeneratorX1Widget : ModulationGeneratorBaseWidget {
	ModulationGeneratorX1Widget(ModulationGeneratorBase *module) : ModulationGeneratorBaseWidget(module) {
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/LFO1.svg")));
    addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.567f,111.934f)), module, ModulationGeneratorBase::MOD_OUTPUT+0));
	}
};
Model *modelModulationGeneratorX1 = createModel<ModulationGeneratorX1, ModulationGeneratorX1Widget>("MG1");




// X8 --------------------------------------------------------------------------------------------------------------
const int X8_CHANNELS = 8;
struct ModulationGeneratorX8 : ModulationGeneratorBase{
  ModulationGeneratorX8():ModulationGeneratorBase(X8_CHANNELS){
  }
};

struct ModulationGeneratorX8Widget : ModulationGeneratorBaseWidget {
	ModulationGeneratorX8Widget(ModulationGeneratorBase *module) : ModulationGeneratorBaseWidget(module) {
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/LFO8.svg")));
    for(int i = 0; i < X8_CHANNELS; i++){
        addOutput(createOutput<PJ301MPort>(mm2px(Vec(18.501f,12.003f + (i*14.f))), module, ModulationGeneratorBase::MOD_OUTPUT + i));
    }
	}
};
Model *modelModulationGeneratorX8 = createModel<ModulationGeneratorX8, ModulationGeneratorX8Widget>("MG8");


// X16 --------------------------------------------------------------------------------------------------------------
const int X16_CHANNELS = 16;
struct ModulationGeneratorX16 : ModulationGeneratorBase{
  ModulationGeneratorX16():ModulationGeneratorBase(X16_CHANNELS){
  }
};

struct ModulationGeneratorX16Widget : ModulationGeneratorBaseWidget {
	ModulationGeneratorX16Widget(ModulationGeneratorBase *module) : ModulationGeneratorBaseWidget(module) {
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/LFO16.svg")));

    for(int i = 0; i < X16_CHANNELS/2; i++)
        addOutput(createOutput<PJ301MPort>(mm2px(Vec(18.501,12.003f + (i*14.f))), module, ModulationGeneratorBase::MOD_OUTPUT + i));

    for(int i = 0; i < X16_CHANNELS/2; i++)
        addOutput(createOutput<PJ301MPort>(mm2px(Vec(28.818f,12.003f + (i*14.f))), module, ModulationGeneratorBase::MOD_OUTPUT + i + X16_CHANNELS/2));

	}
};
Model *modelModulationGeneratorX16 = createModel<ModulationGeneratorX16, ModulationGeneratorX16Widget>("MG16");
