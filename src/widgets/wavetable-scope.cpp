#include "mini-scope.cpp"

struct WaveTableScope : FramebufferWidget {
	float** buffer;
	int bufferIndex = 0;
	int waveEnd = 0;

	int waves;
	int subDivisions;
	int totalScopes;

	MiniScope **scopes;

	bool dirty = false;
	bool mirror = false;

	WaveTableScope() {
	}

	float spacing = 5.f;
	void initialize(int _waves, int _subDivisions){
		//std::cout << "scope init" <<std::endl;

		waves = _waves;
		subDivisions = _subDivisions;
		totalScopes = (waves-1)*(subDivisions+1)+1;

		//Initializing mem for wavetable
		buffer = new float*[totalScopes];
		for(int i = 0; i < totalScopes; i++){
	  	buffer[i] = new float[MiniScope::SCOPE_BUFFER_SIZE];
		}

		//Creating the scopes
		scopes = new MiniScope*[totalScopes];
		float scopeHeight = (box.size.y/(float)totalScopes)-spacing;
		//std::cout << "y: " << box.size.y <<std::endl;

		for (int i = 0; i < totalScopes; i++) {
			MiniScope *scope = new MiniScope(i);
			scopes[(totalScopes-1)-i] = scope;
			scope->box.pos = Vec(0,scopeHeight*i+(spacing*i));
			scope->box.size = Vec(box.size.x, scopeHeight);
			scope->lineWeight = 2.5f;
			scope->setGain(1.0f);
			//std::cout << "craeted scope: " << scopeHeight*i <<std::endl;
			addChild(scope);
		}
	}

	void addFrame(float value, float y){
		if(bufferIndex >= MiniScope::SCOPE_BUFFER_SIZE)
			bufferIndex = 0;

		int index = getScopeIndex(y);
		buffer[index][bufferIndex] = value;
		scopes[index]->addFrame(value);
	}


	void setAlpha(float alpha, int index){
		//If it not already highlighted we need a redraw
		if(scopes[index]->alpha != alpha)
			dirty = true;

		scopes[index]->alpha = alpha;
	}

	/*void setY(float y){
		//std::cout << "setY: " << y <<std::endl;
		//float spread = totalScopes/5.f;
		float index = (float)getScopeIndex(y);

		for (int i = 0; i < totalScopes; i++) {
		 //float dist = abs(i-index);
		 //float limited = std::min(dist,spread);
		 //float a = (spread-limited)/spread;
		 //float alpha = std::max(a,0.5f);
		 //float alpha = i == index ? 1.f : 0.5f;
		 //std::cout << "alpha: " << alpha <<std::endl;
		 float alpha = i == index ? 1.f : 0.5f;
		 setAlpha(alpha,i);
		}
	}*/

	void setY1(float y1){
		int index = getScopeIndex(y1);

		for (int i = 0; i < totalScopes; i++) {
		 float alpha = (i == index) ? 1.f : 0.5f;
		 setAlpha(alpha,i);
		}
	}

	void setY2(float y1, float y2){
		int index = getScopeIndex(y1);
		int index2 = getScopeIndex(y2);

		for (int i = 0; i < totalScopes; i++) {
		 float alpha = (i == index || i == index2) ? 1.f : 0.5f;
		 setAlpha(alpha,i);
		}
	}

	void setY3(float y1, float y2, float y3){
		int index = getScopeIndex(y1);
		int index2 = getScopeIndex(y2);
		int index3 = getScopeIndex(y3);

		for (int i = 0; i < totalScopes; i++) {
		 float alpha = (i == index || i == index2 || i == index3) ? 1.f : 0.5f;
		 setAlpha(alpha,i);
		}
	}


	void setMirror(bool _mirror){
		if(_mirror != mirror){
			dirty = true;
			mirror = _mirror;
			for (int i = 0; i < totalScopes; i++) {
				scopes[i]->setMirror(mirror);
			}
		}
	}

	void endFrame(){
		bufferIndex++;
	}

	void startCapture(){
		//std::cout << "starting" << std::endl;
		bufferIndex = 0;
		waveEnd = MiniScope::SCOPE_BUFFER_SIZE-1;
	}

	void endCapture(){
		waveEnd = bufferIndex-1;
		bufferIndex = 0;
		//std::cout << "ending on: " << waveEnd << std::endl;

		if(subDivisions > 0){
			for(int w = 0; w < waves-1; w++){
				//std::cout << "-----------------------------------------" <<std::endl;
				//std::cout << "w: " << w <<std::endl;

				int mainLevel0 = w*(subDivisions+1);
				int mainLevel1 = mainLevel0+(subDivisions+1);

				//std::cout << "mainLevel0: " << mainLevel0 <<std::endl;
				//std::cout << "mainLevel1: " << mainLevel1 <<std::endl;

				for (int s = 1; s <= subDivisions; s++){
					int subDivisionLevel = mainLevel0+s;
					float levelFrac = (float)s/(float)(subDivisions+1);

					//std::cout << "--------" <<std::endl;
					//std::cout << "s: " << s <<std::endl;
					//std::cout << "subDivisionLevel: " << subDivisionLevel <<std::endl;
					//std::cout << "levelFrac: " << levelFrac <<std::endl;
					//std::cout << "scope id: " << scopes[subDivisionLevel]->id <<std::endl;


					for (int i = 0; i <= waveEnd; i++) {
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
			//std::cout << "waveform dirty" << std::endl;
			FramebufferWidget::dirty = true;
			dirty = false;
		}
		FramebufferWidget::step();
	}
};
