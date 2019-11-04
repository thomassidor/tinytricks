//#include <memory>

struct MiniScope : TransparentWidget {
	static const int SCOPE_BUFFER_SIZE = 2048;
	float buffer[SCOPE_BUFFER_SIZE] = {};
	int bufferIndex = 0;
	int waveEnd = 0;
	bool stopped = false;
	float lineWeight = 1.5f;
	int id = 0;
	float gainCalculated = 0;
	float alpha = 1.0f;
	bool mirror = false;


	MiniScope(){
	}

	MiniScope(int _id){
		id = _id;
	}

	void setGain(float gain){
		gainCalculated = std::pow(2.f, std::round(gain)) / 10.f;
	}

	void setMirror(bool _mirror){
		mirror = _mirror;
	}

	void reset(){
		waveEnd = bufferIndex-1;
		bufferIndex = 0;
		stopped = false;

		//std::cout << "reset miniscope: " << id <<std::endl;
		//std::cout << "waveEnd: " << waveEnd <<std::endl;
	}

	void addFrame(float value){
		if(bufferIndex >= SCOPE_BUFFER_SIZE)
			bufferIndex = 0;

		buffer[bufferIndex] = value;
		bufferIndex++;

		//std::cout << "added frame miniscope: " << id <<std::endl;
	}

	void stop(){
		stopped = true;
	}

  void draw(const DrawArgs &args) override {
		//std::cout << "drawing: " << id <<std::endl;
		if(!stopped){
			Rect b = Rect(Vec(0, 0), box.size);
			drawWave(args, b, gainCalculated, buffer, waveEnd, lineWeight*(alpha), (int)ceil(255.f*alpha));
		}
  }

	void drawWave(const DrawArgs &args, Rect b, float gain, float* buffer, int waveEnd, float lineWeight, int alpha){

		nvgSave(args.vg);

		//Draw scope
		nvgBeginPath(args.vg);
		nvgStrokeColor(args.vg, nvgRGBA(255,255,255,alpha));
		nvgStrokeWidth(args.vg, lineWeight);

		float halfway = waveEnd/2.f;

		//Adding the points
		for (int i = 0; i < waveEnd; i++) {
			int adjustedIndex = i;
			if(mirror && i > halfway){
				adjustedIndex = halfway - (i-halfway);
			}
			Vec v;
			v.x = (float) i / (waveEnd - 1);
			v.y = buffer[adjustedIndex] * gain / 2.f + 0.5f;
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
