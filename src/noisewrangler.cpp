#include "plugin.hpp"
#include "shared/shared.cpp"


struct NOISEWRANGLER : TinyTricksModule {
  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    NUM_INPUTS
  };
  enum OutputIds {
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };


  NOISEWRANGLER() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  }

  void process(const ProcessArgs &args) override {

  }
};



struct NOISEWRANGLERWidget : TinyTricksModuleWidget {
  TTAWidget(NOISEWRANGLER *module) {

    setModule(module);



    InitializeSkin("TTA.svg");
  }
};


Model *modelNOISEWRANGLER = createModel<NOISEWRANGLER, NOISEWRANGLERWidget>("NOISEWRANGLER");
