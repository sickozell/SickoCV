#include "plugin.hpp"

#define MAXTRACKS 7

#define LOWER 0.f
#define MIDDLE 0.5f
#define UPPER 1.f

#define WEAK 0.3f
#define AVERAGE 0.5f
#define STRONG 0.7f

//using namespace std;	// this is for debug

struct RandMod7Compact : Module {

	#include "randModTable.hpp"

	enum ParamId {
		RATE_PARAM,
		RATE_ATTENUV_PARAM,
		ENUMS(XRATE_PARAM, 7),
		BIPOLAR_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RATE_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT, 7),
		OUTPUTS_LEN
	};
	enum LightId {
		BIPOLAR_LIGHT,
		LIGHTS_LEN
	};

	int polyChans = 1;

	int bipolar = 0;

	//float rateKnob = 0.f;

	float xRateKnob[7] = {1, 1, 1, 1, 1, 1, 1};
	float prevXrateKnob[7] = {0, 0, 0, 0, 0, 0, 0};

	float rate[MAXTRACKS] = {};
	float masterRate = 0.f;

	float sampleRate = APP->engine->getSampleRate();
	float sampleRateMs = sampleRate / 1000;
	float maxSamples[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0};
	float currSample[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0};

	int magnetPoint = 0;
	float magnetValue = 0.f;

	int strength = 0;
	float strengthValue = WEAK;

	//float rateAtten = 0.f;

	float y[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0};
	float out[MAXTRACKS];

	//float minOut = 10.f;
	//float maxOut = 0.f;

	double startY[MAXTRACKS] = {};
	double targetY[MAXTRACKS] = {};

	float masterRateCv = 0.f;

	float targetRateCv = 0;
	float prevTargetRateCv = -11;

	float factor[MAXTRACKS] = {};

	bool alert = false;

	static constexpr float minRate = 0.1f;  // in milliseconds
	static constexpr float maxRate = 500.f;  // in milliseconds

	static constexpr float minXrate = 0.047619f;  // in milliseconds
	static constexpr float maxXrate = 21.f;  // in milliseconds

	static constexpr float minFreq = 2.f;       // 500 ms
	static constexpr float maxFreq = 10000.f;   // 0.1 ms

	RandMod7Compact() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RATE_PARAM, 0.f, 1.f, 0.f, "Rate", "hz", maxFreq / minFreq, minFreq);
		configInput(RATE_INPUT, "Rate");
		configParam(RATE_ATTENUV_PARAM, -1.f, 1.f, 0.f, "Rate CV", "%", 0, 100);
		for (int i = 0; i < 7; i++) {
			configParam(XRATE_PARAM+i, 0.f, 1.f, 0.5f, "xRate", "x", maxXrate / minXrate, minXrate);
			configOutput(OUT_OUTPUT+i, "");
		}
		configSwitch(BIPOLAR_PARAM, 0.f, 1.f, 0.f, "Bipolar", {"Off", "On"});

	}

	void onReset(const ResetEvent &e) override {
		//rateKnob = 0.0;

		for (int i = 0; i < 7; i++) {

			xRateKnob[i] = 0.5;
			prevXrateKnob[i] = -11.f;
		}

	    Module::onReset(e);
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
		sampleRateMs = sampleRate / 1000;
	}

	
	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "polyChans", json_integer(polyChans));
		json_object_set_new(rootJ, "magnetPoint", json_integer(magnetPoint));
		json_object_set_new(rootJ, "strength", json_integer(strength));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* polyChansJ = json_object_get(rootJ, "polyChans");
		if (polyChansJ)
			polyChans = json_integer_value(polyChansJ);

		json_t* magnetPointJ = json_object_get(rootJ, "magnetPoint");
		if (magnetPointJ)
			magnetPoint = json_integer_value(magnetPointJ);

		switch (magnetPoint) {
			case 0: magnetValue = LOWER;   break;
			case 1: magnetValue = MIDDLE; break;
			case 2: magnetValue = UPPER;   break;
		}

		json_t* strengthJ = json_object_get(rootJ, "strength");
		if (strengthJ)
			strength = json_integer_value(strengthJ);

		switch (strength) {
			case 0: strengthValue = WEAK;   break;
			case 1: strengthValue = AVERAGE; break;
			case 2: strengthValue = STRONG;   break;
		}
	}


	void setPreset(int preset) {
		switch (preset) {

			case 0: // integer *
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.613835);	// 2x
				params[XRATE_PARAM+2].setValue(0.680424);	// 3x
				params[XRATE_PARAM+3].setValue(0.727670);	// 4x
				params[XRATE_PARAM+4].setValue(0.764317);	// 5x
				params[XRATE_PARAM+5].setValue(0.794259);	// 6x
				params[XRATE_PARAM+6].setValue(0.819576);	// 7x
			break;

			case 1: // integer /
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.386165);	// 2x
				params[XRATE_PARAM+2].setValue(0.319576);	// 3x
				params[XRATE_PARAM+3].setValue(0.272330);	// 4x
				params[XRATE_PARAM+4].setValue(0.235683);	// 5x
				params[XRATE_PARAM+5].setValue(0.205741);	// 6x
				params[XRATE_PARAM+6].setValue(0.180424);	// 7x
			break;

			case 2: // even *
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.613835);	// 2x
				params[XRATE_PARAM+2].setValue(0.727670);	// 4x
				params[XRATE_PARAM+3].setValue(0.794259);	// 6x
				params[XRATE_PARAM+4].setValue(0.841505);	// 8x
				params[XRATE_PARAM+5].setValue(0.878152);	// 10x
				params[XRATE_PARAM+6].setValue(0.908095);	// 12x
			break;

			case 3: // even /
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.386165);	// 2x
				params[XRATE_PARAM+2].setValue(0.272330);	// 4x
				params[XRATE_PARAM+3].setValue(0.205741);	// 6x
				params[XRATE_PARAM+4].setValue(0.158495);	// 8x
				params[XRATE_PARAM+5].setValue(0.121848);	// 10x
				params[XRATE_PARAM+6].setValue(0.091905);	// 12x
			break;

			case 4:	// odd +
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.680424);	// 3x
				params[XRATE_PARAM+2].setValue(0.764317);	// 5x
				params[XRATE_PARAM+3].setValue(0.819576);	// 7x
				params[XRATE_PARAM+4].setValue(0.860849);	// 9x
				params[XRATE_PARAM+5].setValue(0.893805);	// 11x
				params[XRATE_PARAM+6].setValue(0.921240);	// 13x
			break;

			case 5:	// odd -
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.319576);	// 3x
				params[XRATE_PARAM+2].setValue(0.235683);	// 5x
				params[XRATE_PARAM+3].setValue(0.180424);	// 7x
				params[XRATE_PARAM+4].setValue(0.139151);	// 9x
				params[XRATE_PARAM+5].setValue(0.106195);	// 11x
				params[XRATE_PARAM+6].setValue(0.078760);	// 13x
			break;

			case 6: // primes *
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.613835);	// 2x
				params[XRATE_PARAM+2].setValue(0.680424);	// 3x
				params[XRATE_PARAM+3].setValue(0.764317);	// 5x
				params[XRATE_PARAM+4].setValue(0.819576);	// 7x
				params[XRATE_PARAM+5].setValue(0.893805);	// 11x
				params[XRATE_PARAM+6].setValue(0.921240);	// 13x
			break;

			case 7:	// prime /
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.386165);	// 2x
				params[XRATE_PARAM+2].setValue(0.319576);	// 3x
				params[XRATE_PARAM+3].setValue(0.235683);	// 5x
				params[XRATE_PARAM+4].setValue(0.180424);	// 7x
				params[XRATE_PARAM+5].setValue(0.106195);	// 11x
				params[XRATE_PARAM+6].setValue(0.078760);	// 13x
			break;

			case 8: // fibonacci *
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.613835);	// 2x
				params[XRATE_PARAM+2].setValue(0.680424);	// 3x
				params[XRATE_PARAM+3].setValue(0.764317);	// 5x
				params[XRATE_PARAM+4].setValue(0.841505);	// 8x
				params[XRATE_PARAM+5].setValue(0.921240);	// 13x
				params[XRATE_PARAM+6].setValue(1);			// 21x
			break;

			case 9:	// fibonacci /
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.386165);	// 2x
				params[XRATE_PARAM+2].setValue(0.319576);	// 3x
				params[XRATE_PARAM+3].setValue(0.235683);	// 5x
				params[XRATE_PARAM+4].setValue(0.158495);	// 8x
				params[XRATE_PARAM+5].setValue(0.078760);	// 13x
				params[XRATE_PARAM+6].setValue(0);			// 21x
			break;
		}

	}

	static float convertRateParamToMs(float param) {
		float freq = minFreq * std::pow(maxFreq / minFreq, param);
		return 1000.f / freq;
	}

	static float getXrateFactor(float cv) {
		return minXrate * std::pow(maxXrate / minXrate, cv);
	}


	float lookupRateParamToMs(float param) const {

	    float p = param * (LUT_SIZE -1);
	    int i = static_cast<int>(p);
	    float frac = p - i;
	    float a = randModTable[i];
	    float b = randModTable[std::min(i + 1, LUT_SIZE - 1)];
	    return a + frac * (b - a);

	}

	inline float mix(float a, float b, float f) {
		return a + f * (b - a);
	}

	void process(const ProcessArgs& args) override {

		bipolar = params[BIPOLAR_PARAM].getValue();
		lights[BIPOLAR_LIGHT].setBrightness(bipolar);

//		rateKnob = params[RATE_PARAM].getValue();

//		rateAtten =  params[RATE_ATTENUV_PARAM].getValue();

//		targetRateCv = clamp(params[RATE_PARAM].getValue() + (inputs[RATE_INPUT].getVoltage() * rateAtten / 10.f), 0.f, 1.f);

		targetRateCv = params[RATE_PARAM].getValue() + inputs[RATE_INPUT].getVoltage() * params[RATE_ATTENUV_PARAM].getValue() / 10.f;

		if (targetRateCv > 1.f)
			targetRateCv = 1.f;
		else if (targetRateCv < 0.f)
			targetRateCv = 0.f;

		if (targetRateCv != prevTargetRateCv) {

			alert = true;
			masterRate = lookupRateParamToMs(targetRateCv);

			prevTargetRateCv = targetRateCv;
			
		}

		for (int t = 0; t < MAXTRACKS; t++) {
			
			xRateKnob[t] = params[XRATE_PARAM+t].getValue();

			if (xRateKnob[t] != prevXrateKnob[t] || alert) {
				factor[t] = getXrateFactor(xRateKnob[t]);
				prevXrateKnob[t] = xRateKnob[t];
			}

			rate[t] = masterRate / factor[t];
			
			if (rate[t] < minRate)
				rate[t] = minRate;

			maxSamples[t] = sampleRateMs * rate[t];

			if (currSample[t] > maxSamples[t]) {

				//currSample[t] = maxSamples[t] - currSample[t];
				currSample[t] = 0;

				startY[t] = targetY[t];

				targetY[t] = mix(random::uniform(), magnetValue, strengthValue);
			}

			float progress = currSample[t] / maxSamples[t];
			progress = progress * progress * (3.f - 2.f * progress);
			y[t] = startY[t] + progress * (targetY[t] - startY[t]);
			
			if (y[t] > 1.f)
				y[t] = 1.f;
			else if (y[t] < 0.f)
				y[t] = 0.f;

			if (!bipolar)
				out[t] = y[t] * 10;
			else
				out[t] = (y[t] - 0.5f) * 10;

			if (out[t] > 10.f)
				out[t] = 10.f;
			else if (out[t] < -10.f)
				out[t] = -10.f;

			if (t != 6)
				outputs[OUT_OUTPUT + t].setVoltage(out[t]);
			else if (polyChans == 1)
				outputs[OUT_OUTPUT + t].setVoltage(out[t]);

			if (polyChans > 1) {
				outputs[OUT_OUTPUT + 6].setVoltage(out[t], t);
			}

			currSample[t]++;

		}
		outputs[OUT_OUTPUT + 6].setChannels(polyChans);

		alert = false;
	}
};

struct RandMod7CompactWidget : ModuleWidget {
	RandMod7CompactWidget(RandMod7Compact* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RandMod7Compact.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	 

		const float xRtKnob = 10.4;
		const float yRtKnob = 22;

		const float xRateAtnv = 18.2;
		const float yRateAtnv = 33;

		const float xRateIn = 8.7;
		const float yRateIn = 36;

		const float yStart = 57;
		constexpr float yStartShift = 10;

		const float xRt = 6.7;
		const float xOut = 18.7;

		const float xBi = 19;
		const float yBi = 41;

		addParam(createParamCentered<SickoLargeKnob>(mm2px(Vec(xRtKnob, yRtKnob)), module, RandMod7Compact::RATE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xRateAtnv, yRateAtnv)), module, RandMod7Compact::RATE_ATTENUV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRateIn, yRateIn)), module, RandMod7Compact::RATE_INPUT));

		for (int i = 0; i < 7; i++) {
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xRt, yStart + (yStartShift * i))), module, RandMod7Compact::XRATE_PARAM+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yStart + (yStartShift * i))), module, RandMod7Compact::OUT_OUTPUT+i));
		}

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(xBi, yBi)), module, RandMod7Compact::BIPOLAR_PARAM, RandMod7Compact::BIPOLAR_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		RandMod7Compact* module = dynamic_cast<RandMod7Compact*>(this->module);

		const std::string magnetNames[3] = {"Lower", "Middle", "Upper"};

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Magnet Point (Uni/Bi):", magnetNames[module->magnetPoint], [=](Menu * menu) {
			menu->addChild(createMenuItem("Lower (0v/-5v)", CHECKMARK(module->magnetPoint == 0), [=]() {module->magnetPoint = 0; module->magnetValue = LOWER;}));
			menu->addChild(createMenuItem("Middle (5v / 0v)", CHECKMARK(module->magnetPoint == 1), [=]() {module->magnetPoint = 1; module->magnetValue = MIDDLE;}));
			menu->addChild(createMenuItem("Upper (10v/5v)", CHECKMARK(module->magnetPoint == 2), [=]() {module->magnetPoint = 2; module->magnetValue = UPPER;}));
		}));

		const std::string strengthNames[3] = {"weak", "Average", "STRONG"};

		menu->addChild(createSubmenuItem("Magnet Strength:", strengthNames[module->strength], [=](Menu * menu) {
			menu->addChild(createMenuItem("weak", CHECKMARK(module->strength == 0), [=]() {module->strength = 0; module->strengthValue = WEAK;}));
			menu->addChild(createMenuItem("Average", CHECKMARK(module->strength == 1), [=]() {module->strength = 1; module->strengthValue = AVERAGE;}));
			menu->addChild(createMenuItem("STRONG", CHECKMARK(module->strength == 2), [=]() {module->strength = 2; module->strengthValue = STRONG;}));
		}));

		//menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Polyphony on 7th out", std::to_string(module->polyChans), [=](Menu * menu) {
			menu->addChild(createMenuItem("Monophonic", CHECKMARK(module->polyChans == 1), [=]() {module->polyChans = 1;}));
			menu->addChild(createMenuItem("2", CHECKMARK(module->polyChans == 2), [=]() {module->polyChans = 2;}));
			menu->addChild(createMenuItem("3", CHECKMARK(module->polyChans == 3), [=]() {module->polyChans = 3;}));
			menu->addChild(createMenuItem("4", CHECKMARK(module->polyChans == 4), [=]() {module->polyChans = 4;}));
			menu->addChild(createMenuItem("5", CHECKMARK(module->polyChans == 5), [=]() {module->polyChans = 5;}));
			menu->addChild(createMenuItem("6", CHECKMARK(module->polyChans == 6), [=]() {module->polyChans = 6;}));
			menu->addChild(createMenuItem("7", CHECKMARK(module->polyChans == 7), [=]() {module->polyChans = 7;}));
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("xRate Presets"));

		menu->addChild(createMenuItem("Integer *", "", [=]() {module->setPreset(0);}));
		menu->addChild(createMenuItem("Integer /", "", [=]() {module->setPreset(1);}));
		menu->addChild(createMenuItem("Even *", "", [=]() {module->setPreset(2);}));
		menu->addChild(createMenuItem("Even /", "", [=]() {module->setPreset(3);}));
		menu->addChild(createMenuItem("Odd *", "", [=]() {module->setPreset(4);}));
		menu->addChild(createMenuItem("Odd /", "", [=]() {module->setPreset(5);}));		
		menu->addChild(createMenuItem("Prime *", "", [=]() {module->setPreset(6);}));
		menu->addChild(createMenuItem("Prime /", "", [=]() {module->setPreset(7);}));
		menu->addChild(createMenuItem("Fibonacci *", "", [=]() {module->setPreset(8);}));
		menu->addChild(createMenuItem("Fibonacci /", "", [=]() {module->setPreset(9);}));

	}
};

Model* modelRandMod7Compact = createModel<RandMod7Compact, RandMod7CompactWidget>("RandMod7Compact");