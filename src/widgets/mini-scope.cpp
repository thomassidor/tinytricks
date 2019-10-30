#include <memory>

struct MiniScope : TransparentWidget {
	static const int SCOPE_BUFFER_SIZE = 2048;
	float buffer[SCOPE_BUFFER_SIZE] = {};
	int bufferIndex = 0;
	int waveEnd = 0;
	bool stopped = false;
	float lineWeight = 1.5f;
	int id = 0;
	float gainCalculated = 0;


	MiniScope(){
	}

	MiniScope(int _id){
		id = _id;
	}

	void setGain(float gain){
		gainCalculated = std::pow(2.f, std::round(gain)) / 10.f;
	}

	void reset(){
		waveEnd = bufferIndex-1;
		bufferIndex = 0;
		stopped = false;

		//std::cout << "reset miniscope: " << id <<std::endl;
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
			//Rect b = Rect(Vec(5, 15), box.size.minus(Vec(5*2, 15 * 2)));
			drawWave(args, box, gainCalculated, buffer, waveEnd, lineWeight, 0.7f);
		}
  }

	void drawWave(const DrawArgs &args, Rect b, float gain, float* buffer, int waveEnd, float lineWeight, float alpha){

		nvgSave(args.vg);

		//Draw scope
		nvgBeginPath(args.vg);
		nvgStrokeColor(args.vg, nvgRGBA(255,255,255,255*alpha));
		nvgStrokeWidth(args.vg, lineWeight);

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
};

struct WaveTableScope : FramebufferWidget {
	float** buffer;
	int bufferIndex = 0;
	int waveEnd = 0;

	int waves;
	int subDivisions;
	int totalScopes;

	MiniScope **scopes;

	bool dirty = false;

	WaveTableScope() {
	}

	void initialize(int _waves, int _subDivisions){
		waves = _waves;
		subDivisions = _subDivisions;
		totalScopes = waves+(subDivisions*(waves-1));


		//Initializing mem for wavetable
		buffer = new float*[totalScopes];
		for(int i = 0; i < totalScopes; i++){
	  	buffer[i] = new float[MiniScope::SCOPE_BUFFER_SIZE];
		}


		//Creating the scopes
		scopes = new MiniScope*[totalScopes];
		float scopeHeight = box.size.y/(float)totalScopes;
		//std::cout << "y: " << box.size.y <<std::endl;

		for (int i = 0; i < totalScopes; i++) {
			MiniScope *scope = new MiniScope(i);
			scopes[i] = scope;
			scope->box.pos = Vec(0,scopeHeight*i);
			scope->box.size = Vec(box.size.x, scopeHeight);
			scope->setGain(1.0f);
			//std::cout << "pos: " << scopeHeight*i <<std::endl;
			addChild(scope);
		}
	}

	void addFrame(float value, float y){
		int index = getScopeIndex(y);
		//std::cout << "y: " << y << std::endl;
		//std::cout << "adding frame to level: " << index << std::endl;
		//std::cout << "setting value in buffer: " << bufferIndex << std::endl;

		buffer[index][bufferIndex] = value;

		//std::cout << "adding frame: " << index << std::endl;
		scopes[index]->addFrame(value);
	}

	void endFrame(){
		if(bufferIndex >= MiniScope::SCOPE_BUFFER_SIZE)
			bufferIndex = 0;
		bufferIndex++;
	}

	void startCapture(){
		std::cout << "starting" << std::endl;
		bufferIndex = 0;
		waveEnd = MiniScope::SCOPE_BUFFER_SIZE-1;
	}

	void endCapture(){
		waveEnd = bufferIndex-1;
		bufferIndex = 0;
		std::cout << "ending on: " << waveEnd << std::endl;

		if(subDivisions > 0){
			for(int w = 0; w < waves-1; w++){
				std::cout << "-----------------------------------------" <<std::endl;
				std::cout << "w: " << w <<std::endl;
				for (int s = 1; s <= subDivisions; s++){

					int subDivisionLevel = (w*waves)+s;

					int mainLevel0 = w*((subDivisions-1)*(waves-1));
					int mainLevel1 = (w+1)*((subDivisions-1)*(waves-1));

					float levelFrac = (float)s/(float)(subDivisions);

					std::cout << "--------" <<std::endl;
					std::cout << "s: " << s <<std::endl;
					std::cout << "subDivisionLevel: " << subDivisionLevel <<std::endl;
					std::cout << "mainLevel0: " << mainLevel0 <<std::endl;
					std::cout << "mainLevel1: " << mainLevel1 <<std::endl;
					std::cout << "levelFrac: " << levelFrac <<std::endl;

					for (int i = 0; i < waveEnd; i++) {
						float interpolatedValue = buffer[mainLevel0][i] + levelFrac * (buffer[mainLevel1][i] - buffer[mainLevel0][i]);
						scopes[subDivisionLevel]->addFrame(interpolatedValue);
					}
				}
			}
		}

		for (int i = 0; i < totalScopes; i++)
			scopes[i]->reset();

		dirty = true;
	}

	void stop(){
		for (int i = 0; i < totalScopes; i++)
			scopes[i]->stop();

		dirty = false;
	}

	int getScopeIndex(float y){
		return (int)floor(y*(totalScopes-1));
	}

	void step() override{
		if(dirty){
			std::cout << "dirty" << std::endl;
			FramebufferWidget::dirty = true;
			dirty = false;
		}
		FramebufferWidget::step();
	}
};
