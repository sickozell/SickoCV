#define MAXTRACKS 7

#define DIR_UP 0
#define DIR_DOWN 1

#define BASE 0
#define MIDDLE 1
#define TOP 2

#include "plugin.hpp"

using namespace std;	// this is for debug

struct RandMod7 : Module {

	#include "randModTable.hpp"

	enum ParamId {
		RATE_PARAM,
		RATE_ATTENUV_PARAM,

		ENUMS(XRATE_PARAM, 7),
		ENUMS(MIN_PARAM, 7),
		ENUMS(MAX_PARAM, 7),
		ENUMS(MAGNET_PARAM, 7),
		ENUMS(STRENGTH_PARAM, 7),
		PARAMS_LEN
	};
	enum InputId {
		RATE_INPUT,
		ENUMS(STRENGTH_INPUT, 7),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT, 7),
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	//**************************************************************
	//  DEBUG 

	/*
	std::string debugDisplay = "X";
	std::string debugDisplay2 = "X";
	std::string debugDisplay3 = "X";
	std::string debugDisplay4 = "X";
	int debugInt = 0;
	bool debugBool = false;
	*/


	// ***************************************************************

	double rateKnob = 1.f;

	double xRateKnob[7] = {0, 0, 0, 0, 0, 0, 0};
	double prevXrateKnob[7] = {-11.f, -11.f, -11.f, -11.f, -11.f, -11.f, -11.f};

	float rate[MAXTRACKS] = {}; // 
	float masterRate = 0.f;

	double sampleRate = APP->engine->getSampleRate();
	double sampleRateMs = sampleRate / 1000;
	double sampleRateCoeff = 1 / sampleRate;
	double sampleRateCoeffMs = 1 / sampleRateMs;
	double maxSamples[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0};

	double currSample[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0};

	//float strength[MAXTRACKS] = {};

	int magnet[MAXTRACKS]= {};

	float rateAtten = 0.f;

	float y[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0};
	float out[MAXTRACKS];

	float minOut[MAXTRACKS] = {};
	float maxOut[MAXTRACKS] = {};
	
	double startY[MAXTRACKS] = {};
	double targetY[MAXTRACKS] = {};

	float factor[MAXTRACKS] = {};

	bool alert = false;;

	float targetRateCv = 0;
	float prevTargetRateCv = -11;

	const float magnetTarget[3] = {0.0f, 0.5f, 1.0f};

	int polyChans = 1;

	float smoothedMasterRateCv = 0.f;
	const float rateSmoothing = 0.01f; // regola la velocità di smoothing

	static constexpr float minRate = 0.1f;  // in milliseconds
	static constexpr float maxRate = 500.f;  // in milliseconds

	static constexpr float minXrate = 0.047619f;  // in milliseconds
	static constexpr float maxXrate = 21.f;  // in milliseconds

	static constexpr float minFreq = 2.f;       // 500 ms
	static constexpr float maxFreq = 10000.f;   // 0.1 ms

	bool firstRun = true;

	RandMod7() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RATE_PARAM, 0.f, 1.f, 0.f, "Rate", "hz", maxFreq / minFreq, minFreq);
		configInput(RATE_INPUT, "Rate");
		configParam(RATE_ATTENUV_PARAM, -1.f, 1.f, 0.f, "Rate CV", "%", 0, 100);

		for (int i = 0; i < 7; i++) {
			configParam(XRATE_PARAM+i, 0.f, 1.f, 0.5f, "xRate", "x", maxXrate / minXrate, minXrate);
			configParam(MIN_PARAM+i, -10.f, 10.f, 0.f, "Min", "v");
			configParam(MAX_PARAM+i, -10.f, 10.f, 10.f, "Max", "v");
			configSwitch(MAGNET_PARAM+i, 0.f, 2.f, 0.f, "Magnet point", {"Lower", "Middle", "Upper"});
			configParam(STRENGTH_PARAM+i, 0.f, 1.f, 0.5f, "Strength", "%", 0, 100);
			configInput(STRENGTH_INPUT+i, ("Strength #"+to_string(i+1)).c_str());
			configOutput(OUT_OUTPUT+i, ("#"+to_string(i+1)).c_str());
			startY[i] = y[i] = targetY[i] = random::uniform();
			currSample[i] = 0;
			maxSamples[i] = 1.f;
		}
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "polyChans", json_integer(polyChans));
		

		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {

		json_t* polyChansJ = json_object_get(rootJ, "polyChans");
		if (polyChansJ)
			polyChans = json_integer_value(polyChansJ);

	}

	void onReset(const ResetEvent &e) override {
		rateKnob = 1;

		for (int t = 0; t < 7; t++) {
			startY[t] = y[t] = targetY[t] = random::uniform();
			currSample[t] = 0;
			maxSamples[t] = 1.f;
		}

	    Module::onReset(e);
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
		sampleRateMs = sampleRate / 1000;
		sampleRateCoeff = 1 / sampleRate;
		sampleRateCoeffMs = 1 / sampleRateMs;

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

	float lookupRateParamToMs(float param) const {

	    float p = param * (LUT_SIZE -1);
	    int i = static_cast<int>(p);
	    float frac = p - i;
	    float a = randModTable[i];
	    float b = randModTable[std::min(i + 1, LUT_SIZE - 1)];
	    return a + frac * (b - a);

	}

	static float getXrateFactor(float cv) {
		return minXrate * std::pow(maxXrate / minXrate, cv);
	}

	inline float mix(float a, float b, float f) {
		return a + f * (b - a);
	}

	void process(const ProcessArgs& args) override {

		//rateKnob = params[RATE_PARAM].getValue();

		//rateAtten =  params[RATE_ATTENUV_PARAM].getValue();

		//targetRateCv = clamp(params[RATE_PARAM].getValue() + (inputs[RATE_INPUT].getVoltage() * rateAtten / 10.f), 0.f, 1.f);

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
			
			magnet[t] = params[MAGNET_PARAM+t].getValue();
			
			if (rate[t] < minRate)
				rate[t] = minRate;

			maxSamples[t] = sampleRateMs * rate[t];

			if (currSample[t] > maxSamples[t]) {

				//currSample[t] = maxSamples[t] - currSample[t];
				currSample[t] = 0;

				startY[t] = targetY[t];

				float magnetPoint;
				switch ((int)magnet[t]) {
					case 0: magnetPoint = 0.f; break;
					case 1: magnetPoint = 0.5f; break;
					case 2: magnetPoint = 1.f; break;
					default: magnetPoint = 0.f; break;
				}

				float strength = params[STRENGTH_PARAM+t].getValue() + inputs[STRENGTH_INPUT + t].getVoltage() / 10.f;
				if (strength > 1.f)
					strength = 1.f;
				else if (strength < 0.f)
					strength = 0.f;

				targetY[t] = mix(random::uniform(), magnetPoint, strength);
			}

			float progress = currSample[t] / maxSamples[t];
			progress = progress * progress * (3.f - 2.f * progress);
			y[t] = startY[t] + progress * (targetY[t] - startY[t]);

			if (y[t] > 1.f)
				y[t] = 1.f;
			else if (y[t] < 0.f)
				y[t] = 0.f;

			float minVal = params[MIN_PARAM + t].getValue();
			float maxVal = params[MAX_PARAM + t].getValue();

			if (minVal > maxVal) {
				float tempVal = minVal;
				minVal = maxVal;
				maxVal = tempVal;
			}

			out[t] = y[t] * (maxVal - minVal) + minVal;
			
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


struct RandMod7Widget : ModuleWidget {
	RandMod7Widget(RandMod7* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RandMod7.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	 


		const float xRtKnob = 12;
		const float yRtKnob = 22;
		const float xRateAtnv = 20.3;
		const float yRateAtnv = 33;

		const float xRateIn = 11;
		const float yRateIn = 39;

		const float xStart1 = 36;
		const float xStart2 = 52-5-1.5;
		
		const float yRow1 = 23.5;
		const float yRow2 = 15;

		const float yShift = 8;

		const float yStart = 57;
		constexpr float yStartShift = 10;

		const float xRt     = 7.3f;
		const float xType     = 41.0f;
		const float xMin      = 19.4 - 0.6f;
		const float xMax      = 28.8f;
		const float xStr = 52.8f;
		const float xOut   = 63.7f;

		addParam(createParamCentered<SickoLargeKnob>(mm2px(Vec(xRtKnob, yRtKnob)), module, RandMod7::RATE_PARAM));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xRateAtnv, yRateAtnv)), module, RandMod7::RATE_ATTENUV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRateIn, yRateIn)), module, RandMod7::RATE_INPUT));

		for (int i = 0; i < 3; i++)
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xStart1 + (i * 5.5), yRow1 + (yShift * i))), module, RandMod7::STRENGTH_INPUT+i+4));

		for (int i = 0; i < 4; i++)
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xStart2 + (i * 5.5), yRow2 + (yShift * i))), module, RandMod7::STRENGTH_INPUT+i));


		for (int i = 0; i < 7; i++) {
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xRt, yStart + (yStartShift * i))), module, RandMod7::XRATE_PARAM+i));
			addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(xType, yStart + (yStartShift * i))), module, RandMod7::MAGNET_PARAM+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xMin, yStart + (yStartShift * i))), module, RandMod7::MIN_PARAM+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xMax, yStart + (yStartShift * i))), module, RandMod7::MAX_PARAM+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xStr, yStart + (yStartShift * i))), module, RandMod7::STRENGTH_PARAM+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yStart + (yStartShift * i))), module, RandMod7::OUT_OUTPUT+i));
		}
	}

	void appendContextMenu(Menu* menu) override {
		RandMod7* module = dynamic_cast<RandMod7*>(this->module);

		menu->addChild(new MenuSeparator());
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

Model* modelRandMod7 = createModel<RandMod7, RandMod7Widget>("RandMod7");