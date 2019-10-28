#include "plugin.hpp"
#include <iostream>
#include <cmath>
#include <utility>
#include <vector>
#include <stdexcept>
#include "utility/SimplexNoise.hpp"

using namespace std;

//Source: https://rosettacode.org/wiki/Ramer-Douglas-Peucker_line_simplification#C.2B.2B
typedef std::pair<double, double> Point;

double PerpendicularDistance(const Point &pt, const Point &lineStart, const Point &lineEnd){
	double dx = lineEnd.first - lineStart.first;
	double dy = lineEnd.second - lineStart.second;

	//Normalise
	double mag = pow(pow(dx,2.0)+pow(dy,2.0),0.5);
	if(mag > 0.0){
		dx /= mag; dy /= mag;
	}

	double pvx = pt.first - lineStart.first;
	double pvy = pt.second - lineStart.second;

	//Get dot product (project pv onto normalized direction)
	double pvdot = dx * pvx + dy * pvy;

	//Scale line direction vector
	double dsx = pvdot * dx;
	double dsy = pvdot * dy;

	//Subtract this from pv
	double ax = pvx - dsx;
	double ay = pvy - dsy;

	return pow(pow(ax,2.0)+pow(ay,2.0),0.5);
}

void RamerDouglasPeucker(const vector<Point> &pointList, double epsilon, vector<Point> &out){
	if(pointList.size()<2)
		return; //invalid list size

	// Find the point with the maximum distance from line between start and end
	double dmax = 0.0;
	size_t index = 0;
	size_t end = pointList.size()-1;
	for(size_t i = 1; i < end; i++){
		double d = PerpendicularDistance(pointList[i], pointList[0], pointList[end]);
		if (d > dmax){
			index = i;
			dmax = d;
		}
	}

	// If max distance is greater than epsilon, recursively simplify
	if(dmax > epsilon){
		// Recursive call
		vector<Point> recResults1;
		vector<Point> recResults2;
		vector<Point> firstLine(pointList.begin(), pointList.begin()+index+1);
		vector<Point> lastLine(pointList.begin()+index, pointList.end());
		RamerDouglasPeucker(firstLine, epsilon, recResults1);
		RamerDouglasPeucker(lastLine, epsilon, recResults2);

		// Build the result list
		out.assign(recResults1.begin(), recResults1.end()-1);
		out.insert(out.end(), recResults2.begin(), recResults2.end());
		if(out.size()<2)
			return;//throw runtime_error("Problem assembling output");
	}
	else {
		//Just return start and end points
		out.clear();
		out.push_back(pointList[0]);
		out.push_back(pointList[end]);
	}
}


const float SCALE_MAX = 5.f;
const float SCALE_MIN = 0.005f;
const float DETAIL_MIN = 1.f;
const float DETAIL_MAX = 8.f;
struct SNWAVE : Module {
	enum ParamIds {
    SCALE_PARAM,
    DETAIL_PARAM,
		PITCH_PARAM,
    STARTX_PARAM,
    Y_PARAM,
    Z_PARAM,
    DEGREDATION_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};


  SimplexNoise simp;

  static const unsigned int TABLE_SIZE = 1 << 7;
  unsigned int TABLE_END = TABLE_SIZE;
  float lookuptable[TABLE_SIZE];

  float currentIndex = 0.f;
  float tableDelta = 0.f;

  float prevScale = 0.f;
  unsigned int prevDetail = 0.f;
  float prevXstart = 0.f;
  float prevPitch = 0.f;
  float prevDegredation = 0.f;
  float prevY = 0.f;
  float prevZ = 0.f;


  void Initialize(){
    simp.init();

    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(SCALE_PARAM, SCALE_MIN, SCALE_MAX, 0.5f, "Scale");
    configParam(DETAIL_PARAM, DETAIL_MIN, DETAIL_MAX, DETAIL_MIN, "Level of detail");
    configParam(STARTX_PARAM, 0.f, 5.f, 0.f, "X modulation");
    configParam(Y_PARAM, 0.f, 1.f, 0.5f, "Y modulation");
    configParam(Z_PARAM, 0.f, 1.f, 0.5f, "Z modulation");
		configParam(PITCH_PARAM, -3.0f, 3.0f, 0.0f, "Tuning");
    configParam(DEGREDATION_PARAM, 0.000001f, 0.3f, 0.000001f, "Degradation amount");
  }

	SNWAVE() {
		Initialize();
	}

  void createLookuptable(float detailLevel, float scale, float xstart, float y, float z, float degradation){
    vector<Point> pointList;
    vector<Point> pointListOut;
    float x = xstart;
    for (unsigned int i = 0; i < TABLE_SIZE; i++) {
      x += 0.01f;
      float value = simp.SumOctave(detailLevel,x,y,z,scale);
      //baseLookuptable[i] = value;
      pointList.push_back(Point(i,value));
    }

    RamerDouglasPeucker(pointList, degradation, pointListOut);

    int size = pointListOut.size();
    for(int i = 0; i < size; i++){
      lookuptable[i] = pointListOut[i].second;
  		//cout << pointListOut[i].first << "," << pointListOut[i].second << endl;
  	}

    cout << "Size: " << size << endl;

    TABLE_END = size;
    currentIndex = 0.f;
  }

  void setFrequency(float pitch, float sampleRate){
    float frequency = 440.f * powf(2.0f, pitch);
    //auto tableSizeOverSampleRate = TABLE_SIZE / sampleRate;
    auto tableSizeOverSampleRate = TABLE_END / sampleRate;
    tableDelta = frequency * tableSizeOverSampleRate;
  }


  void process(const ProcessArgs &args) override{

    float scale = params[SCALE_PARAM].getValue();
    unsigned int detail = params[DETAIL_PARAM].getValue();
    float xstart = params[STARTX_PARAM].getValue();
    float pitch = params[PITCH_PARAM].getValue();
    float degradation = params[DEGREDATION_PARAM].getValue();
    float y = params[Y_PARAM].getValue();
    float z = params[Z_PARAM].getValue();

    if(scale != prevScale || detail != prevDetail || xstart != prevXstart || degradation != prevDegredation || y != prevY || z != prevZ){
      createLookuptable(detail, scale, xstart, y, z, degradation);
      prevScale = scale;
      prevDetail = detail;
      prevXstart = xstart;
      prevDegredation = degradation;
      prevY = y;
      prevZ = z;
      setFrequency(pitch,args.sampleRate);
    }

    if(pitch != prevPitch){
      setFrequency(pitch,args.sampleRate);
      prevPitch = pitch;
    }

    //Getting indexes for current place in table
    unsigned int index0 = (unsigned int) currentIndex;
    unsigned int index1 = index0 == (TABLE_END - 1) ? (unsigned int) 0 : index0 + 1;
    //unsigned int index1 = index0 == (TABLE_SIZE - 1) ? (unsigned int) 0 : index0 + 1;

    // How far are we from the index
    auto frac = currentIndex - (float) index0;

    //Getting the two samples in the table
    float value0 = lookuptable[index0];
    float value1 = lookuptable[index1];

    //Interpolating between the two
    float currentSample = value0 + frac * (value1 - value0);

    //Adjusting currentIndex
    if ((currentIndex += tableDelta) > TABLE_END)
      currentIndex -= TABLE_END;
    //if ((currentIndex += tableDelta) > TABLE_SIZE)
    //  currentIndex -= TABLE_SIZE;
    outputs[AUDIO_OUTPUT].setVoltage(currentSample*5.f);

  }
};




struct SNWAVEWidget : ModuleWidget {
	SNWAVEWidget(SNWAVE *module) {
		setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Blank.svg")));
		//Screws
		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,11.051f)), module, SNWAVE::SCALE_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,23.613f)), module, SNWAVE::DETAIL_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,37.478f)), module, SNWAVE::STARTX_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,53.478f)), module, SNWAVE::PITCH_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,67.478f)), module, SNWAVE::DEGREDATION_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,79.478f)), module, SNWAVE::Y_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(2.62f,89.478f)), module, SNWAVE::Z_PARAM));

		//Output
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.558f, 113.403f)), module, SNWAVE::AUDIO_OUTPUT));
	}
};

Model *modelSNWAVE = createModel<SNWAVE, SNWAVEWidget>("SNWAVE");
