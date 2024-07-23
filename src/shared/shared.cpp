#include <atomic>

struct TinyTricksPortLight : app::SvgPort {
  TinyTricksPortLight() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/PortLight.svg")));
  }
};

struct TinyTricksPort : app::SvgPort {

  TinyTricksPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/PortLight.svg")));
  }

  void SetDark(bool value) {
    if (value) {
      setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/PortDark.svg")));
    }
    else {
      setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/PortLight.svg")));
    }

  }
};

struct TinyTricksModule : Module {

  int APPLIED_SKIN = 0;
  std::atomic<bool> APPLIED_SKIN_WINS{false};
  bool FORCED_BRIGHT = false;
  std::atomic<bool> FOLLOW_RACK_SKIN{true};

  TinyTricksModule() {
  }

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "skin", json_integer(APPLIED_SKIN));
    json_object_set_new(rootJ, "forcebright", json_boolean(FORCED_BRIGHT));
    json_object_set_new(rootJ, "followrackskin", json_boolean(FOLLOW_RACK_SKIN));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *skinJ = json_object_get(rootJ, "skin");
    if (skinJ)
    {
      APPLIED_SKIN = (int) json_integer_value(skinJ);
      APPLIED_SKIN_WINS = true;
    }

    json_t *forcebrightJ = json_object_get(rootJ, "forcebright");
    if (forcebrightJ)
      FORCED_BRIGHT = json_is_true(forcebrightJ);

    json_t *followr = json_object_get(rootJ, "followrackskin");
    if (followr)
      FOLLOW_RACK_SKIN = json_is_true(followr);
  }


  void AppendBaseJson(json_t *rootJ) {
    json_t *base = TinyTricksModule::dataToJson();

    /* obj is a JSON object */
    const char *key;
    json_t *value;

    json_object_foreach(base, key, value) {
      json_object_set_new(rootJ, key, value);
    }
  }
};

struct TinyTricksModuleWidget : ModuleWidget {

  static const int SKIN_COUNT = 6;
  std::string SKIN_NAMES[SKIN_COUNT] = {"Light (Default)", "River bed", "Shark", "Oxford Blue", "Cod gray", "Firefly"};
  std::string SKIN_FOLDERS[SKIN_COUNT] = {"panels", "panels-river-bed", "panels-shark", "panels-oxford-blue", "panels-cod-gray", "panels-firefly"};

  std::string SKIN_SVG = "";

  Widget *topSilver{nullptr};
  Widget *bottomSilver{nullptr};

  Widget *topBlack{nullptr};
  Widget *bottomBlack{nullptr};

  bool useRackDarkMode{true};
  int currentSkin{0};

  bool forceUseLightPorts = false;

  TinyTricksModuleWidget() {

  }

  void InitializeSkin(std::string svgName) {
    SKIN_SVG = svgName;
    //std::cout << "svg: " << SKIN_SVG << std::endl;

    setSkin(0, true);

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

    updateScrewsAndPorts();
  }

  void setSkin(int skinId, bool appliedWins = false) {
    if (appliedWins && module)
    {
      skinId = dynamic_cast<TinyTricksModule *>(module)->APPLIED_SKIN;
    }
    currentSkin = skinId;

    if (module)
      dynamic_cast<TinyTricksModule *>(module)->APPLIED_SKIN = skinId;

    //std::cout << "path: " << "res/" + SKIN_FOLDERS[skinId] + "/"+SKIN_SVG << std::endl;

    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/" + SKIN_FOLDERS[skinId] + "/" + SKIN_SVG)));
  }

  void updateScrewsAndPorts() {
    topSilver->visible = (currentSkin == 0);
    bottomSilver->visible = (currentSkin == 0);
    topBlack->visible = (currentSkin != 0);
    bottomBlack->visible = (currentSkin != 0);

    if (forceUseLightPorts)
      updatePorts(false);
    else
      updatePorts(currentSkin != 0);
  }

  void updatePorts(bool useDark) {

    if (module)
      dynamic_cast<TinyTricksModule *>(module)->FORCED_BRIGHT = forceUseLightPorts;

    std::list<Widget *>::iterator it;
    for (it = children.begin(); it != children.end(); ++it) {
      TinyTricksPort *port = dynamic_cast<TinyTricksPort *>((*it));
      if (port  != nullptr)
        port->SetDark(useDark);
    }
  }

  bool updatedFromModule = false;
  void step() override {
    if (!updatedFromModule && module) {
      TinyTricksModule *castModule = dynamic_cast<TinyTricksModule *>(module);
      if (castModule != nullptr) {
        updatedFromModule = true;
        forceUseLightPorts = castModule->FORCED_BRIGHT;
        setSkin(castModule->APPLIED_SKIN, castModule->APPLIED_SKIN_WINS);
        castModule->APPLIED_SKIN_WINS = false;
        updateScrewsAndPorts();
      }
    }

    if (module)
    {
      TinyTricksModule *castModule = dynamic_cast<TinyTricksModule *>(module);
      if (castModule->FOLLOW_RACK_SKIN)
      {
        useRackDarkMode = true;
        auto beDark = rack::settings::preferDarkPanels;
        if (beDark && currentSkin != 4)
        {
          setSkin(4);
        }
        else if (!beDark && currentSkin != 0)
        {
          setSkin(0);
        }
      }
      else
      {
        useRackDarkMode = false;
      }
    }

    ModuleWidget::step();
  }

  void appendContextMenu(Menu *menu) override {
    menu->addChild(new MenuEntry);
    menu->addChild(createMenuLabel("Theme"));

    struct ModeItem : MenuItem {
      TinyTricksModuleWidget *widget;
      int skin;
      void onAction(const event::Action &e) override {
        widget->setSkin(skin);
        widget->updateScrewsAndPorts();
      }
    };

    menu->addChild(rack::createMenuItem("Follow Rack Dark Panels",
                   CHECKMARK(useRackDarkMode),
                   [this](){
                       useRackDarkMode = !useRackDarkMode;
                       TinyTricksModule *castModule = dynamic_cast<TinyTricksModule *>(module);
                       castModule->FOLLOW_RACK_SKIN = useRackDarkMode;
                   }));
    menu->addChild(new rack::MenuSeparator());

    for (int i = 0; i < SKIN_COUNT; i++) {
      ModeItem *modeItem = createMenuItem<ModeItem>(SKIN_NAMES[i]);
      modeItem->rightText = CHECKMARK(currentSkin == i);
      modeItem->widget = this;
      modeItem->skin = i;
      modeItem->disabled = useRackDarkMode;
      menu->addChild(modeItem);
    }

    struct PortItem : MenuItem {
      TinyTricksModuleWidget *widget;
      int skin;
      void onAction(const event::Action &e) override {
        widget->forceUseLightPorts = !widget->forceUseLightPorts;
        widget->updateScrewsAndPorts();
      }
    };

    PortItem *portItem = createMenuItem<PortItem>("- Use light theme for input ports");
    portItem->rightText = CHECKMARK(forceUseLightPorts == true);
    portItem->widget = this;
    menu->addChild(portItem);

  }

};
