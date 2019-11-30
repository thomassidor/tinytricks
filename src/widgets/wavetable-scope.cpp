
struct WaveTableScope : FramebufferWidget {
	float SAMPLE_COUNT = 0;

	float** buffer;

	bool stopped = false;

	int waves;
	int subDivisions;
	int totalScopes;

	bool dirty = false;
	bool mirror = false;

	float lineWeight = 2.5f;
	float spacing = 5.f;

	SvgWidget* helpText;

	std::vector<int> highlights;

	WaveTableScope() {
		helpText = createWidget<SvgWidget>(Vec(0,0));
		helpText->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/components/Wavetable-help.svg")));
		addChild(helpText);
		helpText->box.pos = Vec(9,3);
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
			dirty = true;

		highlights = tmp;
	}

	void generate(WaveTable* waveTable, int _subDivisions){

		//Marking dirty and removing help text
		FramebufferWidget::dirty = true;
		helpText->visible = false;

		//Setting up size variables
		SAMPLE_COUNT = waveTable->WAVETABLE_SIZE;
		waves = waveTable->WAVEFORM_COUNT;
		subDivisions = _subDivisions;
		totalScopes = (waves-1)*(subDivisions+1)+1;

		//Initializing mem for wavetable
		buffer = new float*[totalScopes];
		for(int i = 0; i < totalScopes; i++){
	  	buffer[i] = new float[waveTable->WAVETABLE_SIZE];
		}

		for (int y = 0; y < totalScopes; y++) {
			float level = (float)y/((float)totalScopes-1);
			for (int x = 0; x < SAMPLE_COUNT; x++) {
				float value = waveTable->getSample(level,(float)x);
				buffer[y][x] = value;
			}
		}
	}

	void setMirror(bool _mirror){
		if(_mirror != mirror){
			dirty = true;
			mirror = _mirror;
		}
	}

	void stop(){
		stopped = true;
	}

	void start(){
		stopped = false;
	}

	void draw(const DrawArgs &args) override {
		if(!stopped && dirty){
			float scopeHeight = (box.size.y/(float)totalScopes)-spacing;
			for(int i = 0; i < totalScopes; i++){
				Vec pos = Vec(0,scopeHeight*i+(spacing*i));
				Vec size = Vec(box.size.x, scopeHeight);
				Rect b = Rect(pos, size);
				float alpha = 0.5f;
				if(std::find(highlights.begin(), highlights.end(), i) != highlights.end())
					alpha = alpha + 0.5f;
				drawWave(args, b, i, alpha);
			}
		}
		FramebufferWidget::draw(args);
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
