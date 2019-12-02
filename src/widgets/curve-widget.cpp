struct CurveWidgetInternal : OpaqueWidget{
	std::vector<float> ys;
	float lineWeight = 1.5f;
	bool isLiniearMode = true;

	CurveWidgetInternal(){
	}

  void draw(const DrawArgs &args) override {
		Rect b = Rect(Vec(0, 0), box.size);
		nvgSave(args.vg);

		//Draw scope
		nvgBeginPath(args.vg);
		nvgStrokeColor(args.vg, nvgRGBA(255,255,255,255));
		nvgStrokeWidth(args.vg, lineWeight);

		size_t points = ys.size();

		//Adding the points
		for (size_t i = 0; i < points; i++) {
			if(!isLiniearMode && i == points-1)
				break;

			Vec v;
			v.x = i/(float)(points-1);
			v.y = ys[i];
			Vec p;
			p.x = rescale(v.x, 0.f, 1.f, b.pos.x, b.pos.x + b.size.x);
			p.y = rescale(v.y, 0.f, 10.f, b.pos.y + b.size.y, b.pos.y);


			if (i == 0)
				nvgMoveTo(args.vg, p.x, p.y);
			else
				nvgLineTo(args.vg, p.x, p.y);

			if(!isLiniearMode && i < points-1){
				Vec v1;
				v1.x = (i+1)/(float)(points-1);
				v1.y = ys[i];
				Vec p1;
				p1.x = rescale(v1.x, 0.f, 1.f, b.pos.x, b.pos.x + b.size.x);
				p1.y = rescale(v1.y, 0.f, 10.f, b.pos.y + b.size.y, b.pos.y);
				nvgLineTo(args.vg, p1.x, p1.y);
			}
		}

		nvgLineCap(args.vg, NVG_ROUND);
		nvgLineJoin(args.vg, NVG_ROUND);
		nvgStroke(args.vg);
		nvgClosePath(args.vg);
		//Done with scope

		nvgRestore(args.vg);
  }
};

struct CurveWidget : FramebufferWidget {
	CurveWidgetInternal* internal;

	CurveWidget(){

	}

	void setMode(bool isLinear){
		internal->isLiniearMode = isLinear;
		dirty = true;
	}

	void setPoints(std::vector<float> _ys){
		internal->ys = _ys;
		dirty = true;
	}

	void rePaint(){
		dirty = true;
	}

	void setup(){
		internal = new CurveWidgetInternal();
		internal->box.pos = Vec(0,0);
		internal->box.size = box.size;
		addChild(internal);
	}

};
