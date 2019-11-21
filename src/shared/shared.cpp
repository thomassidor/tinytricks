//std::cout << "muteCount: " << muteCount << std::endl;
//std::cout << "Voltage: " << inputs[MUTE_COUNT_CV_INPUT].getVoltage() << std::endl;
struct TinyTricksModule : Module {

  int APPLIED_SKIN = 0;

  TinyTricksModule(){
  }

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "skin", json_integer(APPLIED_SKIN));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
		json_t *skinJ = json_object_get(rootJ, "skin");
		if (skinJ) APPLIED_SKIN = (int)json_integer_value(skinJ);
    //std::cout << "skin from json: " << APPLIED_SKIN << std::endl;
  }


  void AppendBaseJson(json_t *rootJ){
    json_t *base = TinyTricksModule::dataToJson();

    /* obj is a JSON object */
    const char *key;
    json_t *value;

    json_object_foreach(base, key, value) {
        json_object_set_new(rootJ,key,value);
    }
  }
};

struct TinyTricksModuleWidget : ModuleWidget{

  static const int SKIN_COUNT = 6;
  std::string SKIN_NAMES[SKIN_COUNT] = {"Light (Default)", "River bed", "Shark", "Oxford Blue", "Cod gray", "Firefly"};
  std::string SKIN_FOLDERS[SKIN_COUNT] = {"panels", "panels-river-bed", "panels-shark", "panels-oxford-blue", "panels-cod-gray", "panels-firefly"};

  std::string SKIN_SVG = "";

  Widget* topSilver;
  Widget* bottomSilver;

  Widget* topBlack;
  Widget* bottomBlack;

  int currentSkin = 0;

  TinyTricksModuleWidget(){

  }


  void InitializeSkin(std::string svgName){
    SKIN_SVG = svgName;
    //std::cout << "svg: " << SKIN_SVG << std::endl;

    setSkin(0);

    //std::cout << "module: " << module << std::endl;
    //std::cout << "skin: " << currentSkin << std::endl;

    //Screws
    topSilver = createWidget<ScrewSilver>(Vec(0, 0));
    bottomSilver = createWidget<ScrewSilver>(Vec(box.size.x - 15, 365));
    topBlack = createWidget<ScrewBlack>(Vec(0, 0));
    bottomBlack = createWidget<ScrewBlack>(Vec(box.size.x - 15, 365));

    addChild(topSilver);
    addChild(bottomSilver);
    addChild(topBlack);
    addChild(bottomBlack);

    updateScrews();
  }

  void setSkin(int skinId){
    currentSkin = skinId;

    if(module)
      dynamic_cast<TinyTricksModule*>(module)->APPLIED_SKIN = skinId;

    //std::cout << "path: " << "res/" + SKIN_FOLDERS[skinId] + "/"+SKIN_SVG << std::endl;

    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/" + SKIN_FOLDERS[skinId] + "/"+SKIN_SVG)));
  }

  void updateScrews(){
    topSilver->visible = (currentSkin == 0);
    bottomSilver->visible = (currentSkin == 0);
    topBlack->visible = (currentSkin != 0);
    bottomBlack->visible = (currentSkin != 0);
  }

  void step() override{
    //std::cout << "currentSkin: " << currentSkin << std::endl;
    if(module){
      int moduleSkin = dynamic_cast<TinyTricksModule*>(module)->APPLIED_SKIN;
      if(moduleSkin != currentSkin){
        setSkin(moduleSkin);
        updateScrews();
      }
    }

    ModuleWidget::step();
  }

  void appendContextMenu(Menu* menu) override {
    menu->addChild(new MenuEntry);
    menu->addChild(createMenuLabel("Theme"));

    struct ModeItem : MenuItem {
      TinyTricksModuleWidget* widget;
      int skin;
      void onAction(const event::Action& e) override {
        widget->setSkin(skin);
        widget->updateScrews();
      }
    };


    for (int i = 0; i < SKIN_COUNT; i++) {
      ModeItem* modeItem = createMenuItem<ModeItem>(SKIN_NAMES[i]);
      modeItem->rightText = CHECKMARK(currentSkin == i);
      modeItem->widget = this;
      modeItem->skin = i;
      menu->addChild(modeItem);
    }
  }
};
