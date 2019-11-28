struct CurveWidget : FramebufferWidget {
	std::vector<float> xs;
	std::vector<float> ys;

	float lineWeight = 1.5f;



	CurveWidget(){
	}

	void setPoints(std::vector<float> _ys){
		ys = _ys;
		dirty = true;
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
		}

		nvgLineCap(args.vg, NVG_ROUND);
		nvgLineJoin(args.vg, NVG_ROUND);
		nvgStroke(args.vg);
		nvgClosePath(args.vg);
		//Done with scope

		nvgRestore(args.vg);
  }
};
