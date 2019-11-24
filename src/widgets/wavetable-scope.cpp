struct WaveTableScope : FramebufferWidget {
	float SAMPLE_COUNT = 0;

	float** buffer;

	int waves;
	int subDivisions;
	int totalScopes;

	bool dirty = false;
	bool mirror = false;

	float lineWeight = 2.5f;
	float spacing = 5.f;

	float gainCalculated = 0;

	SvgWidget* helpText;

	WaveTableScope() {
		helpText = createWidget<SvgWidget>(Vec(0,0));
		helpText->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/components/Wavetable-help.svg")));
		addChild(helpText);
		helpText->box.pos = Vec(9,3);
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

		//Coping from wavetable
		for (int y = 0; y < waves; y++) {
			for (int x = 0; x <= SAMPLE_COUNT; x++) {
				buffer[y][x] = waveTable->lookuptables[x][y];
			}
		}

		//Generating sub divisions
		if(subDivisions > 0){
			for(int w = 0; w < waves-1; w++){

				int mainLevel0 = w*(subDivisions+1);
				int mainLevel1 = mainLevel0+(subDivisions+1);

				for (int s = 1; s <= subDivisions; s++){
					int subDivisionLevel = mainLevel0+s;
					float levelFrac = (float)s/(float)(subDivisions+1);

					for (int i = 0; i <= SAMPLE_COUNT; i++) {
						float interpolatedValue = buffer[mainLevel0][i] + levelFrac * (buffer[mainLevel1][i] - buffer[mainLevel0][i]);
						buffer[subDivisionLevel][i] = interpolatedValue;
					}
				}
			}
		}
	}

	void setMirror(bool _mirror){
		if(_mirror != mirror){
			FramebufferWidget::dirty = true;
			mirror = _mirror;
		}
	}



	void draw(const DrawArgs &args) override {
		float scopeHeight = (box.size.y/(float)totalScopes)-spacing;
		for(int i = 0; i < totalScopes; i++){
			Vec pos = Vec(0,scopeHeight*i+(spacing*i));
			Vec size = Vec(box.size.x, scopeHeight);
			Rect b = Rect(pos, size);
			float alpha = 1.f;
			drawWave(args, b, buffer[i], alpha);
		}
	}

	void drawWave(const DrawArgs &args, Rect b, float* data, float alpha){

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
			v.y = data[adjustedIndex] /** gainCalculated*/ / 2.f + 0.5f;
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
