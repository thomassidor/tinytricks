const int SCOPE_BUFFER_SIZE = 2048;

struct ScopedModule{
	float buffer[SCOPE_BUFFER_SIZE] = {};
	int bufferIndex = 0;
	int waveEnd = 0;

	ScopedModule(){
	}

	void resetScope(){
		waveEnd = bufferIndex-1;
		//waveEnd = ceil((waveEnd+bufferIndex-1)/2.f);
		bufferIndex = 0;
	}

	void addFrameToScope(int sampleRate, float value){

		if(bufferIndex >= SCOPE_BUFFER_SIZE)
			bufferIndex = 0;

		buffer[bufferIndex] = value;
		bufferIndex++;
	}
};

struct MiniScope : TransparentWidget {
	ScopedModule* module;

	MiniScope() {
	}

  void draw(const DrawArgs &args) override {
    if(module){
      nvgSave(args.vg);

      //Draw scope
      nvgBeginPath(args.vg);
      nvgStrokeColor(args.vg, nvgRGBA(255,255,255,255));
      nvgStrokeWidth(args.vg, 1.5f);

			float* buffer = module->buffer;
			int waveEnd = module->waveEnd;

			float gain = std::pow(2.f, std::round(2.f)) / 10.f;
			Rect b = Rect(Vec(5, 15), box.size.minus(Vec(5*2, 15 * 2)));

      //Adding the points
			for (int i = 0; i < waveEnd; i++) {
						Vec v;
						v.x = (float) i / (waveEnd - 1);
						v.y = buffer[i] * gain / 2.f + 0.5f;
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
    else
        return;
  }
};
