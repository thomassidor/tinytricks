#include "plugin.hpp"

struct OscillatorT1{
  private:
    float phase = 0.0f;
    float freq = 0.0f;
    float detuneAmount = 0.0f;

  public:
    void step(float dt){
      phase+= (freq*(1.f-detuneAmount)) / dt;
      if (phase >= 1.0f)
        phase -= 1.0f;
    }

    void detune(float amount){
        detuneAmount = amount;
    }

    float getValue(){
      return sinf(2.0f * M_PI * (phase+1 * 0.125f)) * 5.0f;
    }

    void setPitch(float f){
      freq = 440.0f * powf(2.0f, f);
    }


};


const int NUM_OSCILLATORS = 3;
struct TTO1 : Module {
	enum ParamIds {
		FREQ_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FREQ_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OSC_OUTPUT,
		TRI_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
    FREQ_LIGHT,
		NUM_LIGHTS
	};

  OscillatorT1 oscillators[NUM_OSCILLATORS];

	TTO1() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(TTO1::FREQ_PARAM, -3.0f, 3.0f, 0.0f, "Value", " V");
	}

  void process(const ProcessArgs &args) override{

    //Getting the pitch
  	float pitch = params[FREQ_PARAM].getValue();
  	pitch += inputs[FREQ_CV_INPUT].getVoltage();
  	pitch = clamp(pitch, -3.0f, 6.0f);

    float outValue = 0.f;
    for (int i = 0; i < NUM_OSCILLATORS; i++) {
      //Updating osc
      oscillators[i].setPitch(pitch);
      oscillators[i].detune(i*00001.f);
      oscillators[i].step(args.sampleRate);
      outValue += oscillators[i].getValue()*0.3;
    }

    //Setting out and light
  	outputs[OSC_OUTPUT].setVoltage(outValue);
    lights[FREQ_LIGHT].value = (outputs[OSC_OUTPUT].value > 0.0f) ? 1.0f : 0.0f;

  }
};




struct TTO1Widget : ModuleWidget {

	TTO1Widget(TTO1 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/TTO1.svg")));

    addInput(createInput<PJ301MPort>(mm2px(Vec(3.567f,12.003f)), module, TTO1::FREQ_CV_INPUT));

    addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(6.62f,40.749f)), module, TTO1::FREQ_LIGHT));

    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,29.749f)), module, TTO1::FREQ_PARAM));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.567f,111.934f)), module, TTO1::OSC_OUTPUT));

    //Screws
    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));
	}
};


Model *modelTTO1 = createModel<TTO1, TTO1Widget>("TT-O1");
