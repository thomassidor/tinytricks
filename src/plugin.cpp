#include "plugin.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
  pluginInstance = p;

  // Add all Models defined throughout the pluginInstance
  p->addModel(modelSH16);
  p->addModel(modelModulationGeneratorX1);
  p->addModel(modelModulationGeneratorX8);
  p->addModel(modelModulationGeneratorX16);
  p->addModel(modelA8);
  p->addModel(modelRM8);
  p->addModel(modelRM8S);
  p->addModel(modelTTA);
  p->addModel(modelTTL);
  p->addModel(modelTTSIN);
  p->addModel(modelTTSAW);
  p->addModel(modelTTSQR);
  p->addModel(modelTTTRI);
  p->addModel(modelTTSINPLUS);
  p->addModel(modelTTSAWPLUS);
  p->addModel(modelTTSQRPLUS);
  p->addModel(modelTTTRIPLUS);
  p->addModel(modelRX8);
  p->addModel(modelSN1);
  p->addModel(modelSN8);
  p->addModel(modelSNOSC);
  p->addModel(modelWAVE);
  p->addModel(modelRANDOMWRANGLER);


  // Any other pluginInstance initialization may go here.
  // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
