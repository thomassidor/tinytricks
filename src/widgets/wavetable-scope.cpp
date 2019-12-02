
struct WaveTableScopeInternals : OpaqueWidget {
  FramebufferWidget *parentAsFb = nullptr;

	float SAMPLE_COUNT = 0;

	float** buffer;

  Rect* rects;

	bool stopped = false;

	int waves;
	int subDivisions;
	int totalScopes;

	bool mirror = false;

	float lineWeight = 2.5f;
	float spacing = 5.f;

	std::vector<int> highlights;

	WaveTableScopeInternals() {

	}

	int getScopeIndex(float y){
			return (int)floor(y*(totalScopes-1));
		}

	void setYs(std::vector<float> ys){
		std::vector<int> tmp;
		tmp.reserve(3);
		for(std::vector<float>::iterator it = ys.begin(); it != ys.end(); it++)
			tmp.push_back(getScopeIndex((*it)));

		std::sort(tmp.begin(), tmp.end());

    if(tmp != highlights)
      dirtyParent();

		highlights = tmp;
	}

	void generate(WaveTable* waveTable, int _subDivisions){
		//Setting up size variables
		SAMPLE_COUNT = waveTable->WAVETABLE_SIZE;
		waves = waveTable->WAVEFORM_COUNT;
		subDivisions = _subDivisions;
		totalScopes = (waves-1)*(subDivisions+1)+1;

		//Initializing mem for wavetable
		buffer = new float*[totalScopes];

    // And mem for boxes
    rects = new Rect[totalScopes];
    float scopeHeight = ( (box.size.y - ((totalScopes-1) * spacing) )/ (float) totalScopes);

    //Copying wavetable and creating boxes for drawing
		for(int i = 0; i < totalScopes; i++){
	  	buffer[i] = new float[waveTable->WAVETABLE_SIZE];

      Vec pos = Vec(0,(scopeHeight + spacing)*i);
      Vec size = Vec(box.size.x, scopeHeight);
      Rect b = Rect(pos, size);
      rects[i] = b;
		}

    //Generating all intermediate waveforms
		for (int y = 0; y < totalScopes; y++) {
			float level = (float)y/((float)totalScopes-1);
			for (int x = 0; x < SAMPLE_COUNT; x++) {
				float value = waveTable->getSample(level,(float)x);
				buffer[y][x] = value;
			}
		}
    dirtyParent();
	}

	void setMirror(bool _mirror){
		if(_mirror != mirror){
			mirror = _mirror;
      dirtyParent();
		}
	}

	void stop(){
		stopped = true;
    dirtyParent();
	}

	void start(){
		stopped = false;
    dirtyParent();
	}

  void dirtyParent() {
    if( parentAsFb )
      parentAsFb->dirty = true;
  }

	void draw(const DrawArgs &args) override {
    //INFO( "DRAW WAV IMPL" ); // uncomment this to make sure double buffering still works (it does).
    //std::cout << "Drawing" << std::endl;
		if(!stopped){
			for(int i = 0; i < totalScopes; i++){
				float alpha = 0.5f;
				if(std::find(highlights.begin(), highlights.end(), i) != highlights.end())
					alpha = alpha + 0.5f;
				drawWave(args, rects[i], i, alpha);
			}
		}
	}

	void drawWave(const DrawArgs &args, Rect b, int level, float alpha){

		nvgSave(args.vg);

		//Draw scope
		nvgBeginPath(args.vg);
		nvgStrokeColor(args.vg, nvgRGBA(255,255,255,(int)ceil(255.f*alpha)));
		nvgStrokeWidth(args.vg, lineWeight*(alpha));

		float halfway = SAMPLE_COUNT/2.f;

		//Adding the points
		for (int i = 0; i < SAMPLE_COUNT; i++) {
			int adjustedIndex = i;
				if(mirror && i > halfway){
				adjustedIndex = halfway - (i-halfway);
			}
			Vec v;
			v.x = (float) i / (SAMPLE_COUNT - 1);
			v.y = buffer[level][adjustedIndex] * 0.2f / 2.f + 0.5f;
			Vec p;
			p.x = rescale(v.x, 0.f, 1.f, b.pos.x, b.pos.x + b.size.x);
			p.y = rescale(v.y, 0.f, 1.f, b.pos.y + b.size.y, b.pos.y);
			if (i == 0)
				nvgMoveTo(args.vg, p.x, p.y);
			else
				nvgLineTo(args.vg, p.x, p.y);
		}
		nvgLineCap(args.vg, NVG_ROUND);
		nvgLineJoin(args.vg, NVG_ROUND);
		nvgStroke(args.vg);
		nvgClosePath(args.vg);
		//Done with scope

		nvgRestore(args.vg);
	}
};


struct WaveTableScope : FramebufferWidget {

  WaveTableScopeInternals *impl = nullptr;
  SvgWidget* helpText;

  WaveTableScope(){
  }

  void generate(WaveTable* waveTable, int subDivisions){
    //Marking dirty and removing help text
    helpText->visible = false;
    impl->visible = true;
    impl->generate(waveTable, subDivisions);
  }

  void setup() {
    helpText = createWidget<SvgWidget>(Vec(0,0));
    helpText->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/components/Wavetable-help.svg")));
    addChild(helpText);
    helpText->box.pos = Vec(9,3);

    impl = new WaveTableScopeInternals();
    impl->box.pos = Vec(0,0);
    impl->box.size = box.size;
    impl->parentAsFb = this;
    impl->visible = false;
    addChild(impl);
  }
};
