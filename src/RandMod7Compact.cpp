#include "plugin.hpp"

#define RAMPUP 0
#define TRIANGLE 1
#define RAMPDOWN 2

#define MAXTRACKS 7
#define ALLTRACKS 8
#define MR 7

#define DIR_UP 0
#define DIR_DOWN 1

#define BASE 0
#define MIDDLE 1
#define TOP 2

//using namespace std;	// this is for debug

struct RandMod7compact : Module {
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

	double rateKnob = 0.5f;

	double xRateKnob[7] = {1, 1, 1, 1, 1, 1, 1};
	double prevXrateKnob[7] = {0, 0, 0, 0, 0, 0, 0};

	float rate[MAXTRACKS] = {}; // 
	float masterRate = 0.f;

	double sampleRate = APP->engine->getSampleRate();
	double sampleRateMs = sampleRate / 1000;
	double maxSamples[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0};
	double currSample[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0};

	int magnet = 1;

	int strength = 1;

	float rateAtten = 0.f;

	float y[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0};
	float out[MAXTRACKS];

	int direction[MAXTRACKS] = {BASE, BASE, BASE, BASE, BASE, BASE, BASE};

	//float minOut = 10.f;
	//float maxOut = 0.f;

	double startY[MAXTRACKS] = {};
	double targetY[MAXTRACKS] = {};

	float masterRateCv = 0.f;

	float factor[MAXTRACKS] = {};

	bool alert = false;;

	const float magnetTarget[3] = {0.0f, 0.5f, 1.0f};

	float polarityBlend = 0.f;
	bool prevBipolar = false;

	float smoothedMasterRateCv = 0.f;
	const float rateSmoothing = 0.01f; // regola la velocità di smoothing

	static constexpr float minRate = 0.1f;  // in milliseconds
	static constexpr float maxRate = 500.f;  // in milliseconds

	static constexpr float minXrate = 0.047619f;  // in milliseconds
	static constexpr float maxXrate = 21.f;  // in milliseconds

	RandMod7compact() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RATE_PARAM, 0.f, 1.f, 1.f, "Rate", "ms", maxRate / minRate, minRate);
		configInput(RATE_INPUT, "Rate");
		configParam(RATE_ATTENUV_PARAM, -1.f, 1.f, 0.f, "Rate CV", "%", 0, 100);
		for (int i = 0; i < 7; i++) {
			configParam(XRATE_PARAM+i, 0.f, 1.f, 0.5f, "xRate", "x", maxXrate / minXrate, minXrate);
			configOutput(OUT_OUTPUT+i, "");
		}
		configSwitch(BIPOLAR_PARAM, 0.f, 1.f, 0.f, "Bipolar", {"Off", "On"});
	}

	void onReset(const ResetEvent &e) override {
		rateKnob = 0.5;

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
		json_object_set_new(rootJ, "magnet", json_integer(magnet));
		json_object_set_new(rootJ, "strength", json_integer(strength));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* polyChansJ = json_object_get(rootJ, "polyChans");
		if (polyChansJ)
			polyChans = json_integer_value(polyChansJ);

		json_t* magnetJ = json_object_get(rootJ, "magnet");
		if (magnetJ)
			magnet = json_integer_value(magnetJ);

		json_t* strengthJ = json_object_get(rootJ, "strength");
		if (strengthJ)
			strength = json_integer_value(strengthJ);
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

	static float getXrateFactor(float cv) {
		return minXrate * std::pow(maxXrate / minXrate, cv);
	}

	static float convertRateToMilliseconds(float cv) {
		return minRate * std::pow(maxRate / minRate, cv);
	}

	void process(const ProcessArgs& args) override {

		bipolar = params[BIPOLAR_PARAM].getValue() > 0.5f;
		lights[BIPOLAR_LIGHT].setBrightness(bipolar ? 1.f : 0.f);

		float targetBlend = bipolar ? 1.f : 0.f;
		
		float blendSpeed = 0.00005f;  // oppure 0.01f per più veloce

		polarityBlend += (targetBlend - polarityBlend) * blendSpeed;

		polarityBlend += (targetBlend - polarityBlend) * blendSpeed;
		
		rateKnob = params[RATE_PARAM].getValue();

		rateAtten =  params[RATE_ATTENUV_PARAM].getValue();

		float targetRateCv = clamp(params[RATE_PARAM].getValue() + (inputs[RATE_INPUT].getVoltage() * rateAtten / 10.f), 0.f, 1.f);

		if (abs(targetRateCv - smoothedMasterRateCv) > 1e-6) {
			smoothedMasterRateCv += (targetRateCv - smoothedMasterRateCv) * rateSmoothing;
			alert = true;
			masterRate = convertRateToMilliseconds(smoothedMasterRateCv);
		}

		for (int t = 0; t < MAXTRACKS; t++) {
			
			xRateKnob[t] = params[XRATE_PARAM+t].getValue();

			if (xRateKnob[t] != prevXrateKnob[t] || alert) {
				factor[t] = getXrateFactor(xRateKnob[t]);
				prevXrateKnob[t] = xRateKnob[t];
			}

			rate[t] = masterRate * factor[t];
			
			if (rate[t] < minRate)
				rate[t] = minRate;

			maxSamples[t] = sampleRateMs * rate[t];

			if (currSample[t] > maxSamples[t]) {

				currSample[t] = maxSamples[t] - currSample[t];
				//currSample[t] = 0;
				startY[t] = y[t];

				float center = magnetTarget[magnet];
				float stdDev = 0.4f / (strength + 1);  // strength: 0 -> più largo
				float next = center + random::normal() * stdDev;
				targetY[t] = clamp(next, 0.f, 1.f);
			}

			float phase = currSample[t] / maxSamples[t];
			float eased = phase * phase * (3.f - 2.f * phase);  // Smoothstep
			y[t] = startY[t] + (targetY[t] - startY[t]) * eased;
			
			if (y[t] > 1.f)
				y[t] = 1.f;
			else if (y[t] < 0.f)
				y[t] = 0.f;

			float unipolarOut = y[t] * 10.f;
			float bipolarOut = (y[t] - 0.5f) * 10.f;
			out[t] = (1.f - polarityBlend) * unipolarOut + polarityBlend * bipolarOut;

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

struct RandMod7compactWidget : ModuleWidget {
	RandMod7compactWidget(RandMod7compact* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RandMod7compact.svg")));

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

		addParam(createParamCentered<SickoLargeKnob>(mm2px(Vec(xRtKnob, yRtKnob)), module, RandMod7compact::RATE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xRateAtnv, yRateAtnv)), module, RandMod7compact::RATE_ATTENUV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRateIn, yRateIn)), module, RandMod7compact::RATE_INPUT));

		for (int i = 0; i < 7; i++) {
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xRt, yStart + (yStartShift * i))), module, RandMod7compact::XRATE_PARAM+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yStart + (yStartShift * i))), module, RandMod7compact::OUT_OUTPUT+i));
		}

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(xBi, yBi)), module, RandMod7compact::BIPOLAR_PARAM, RandMod7compact::BIPOLAR_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		RandMod7compact* module = dynamic_cast<RandMod7compact*>(this->module);

		const std::string magnetNames[3] = {"Base", "Middle", "Top"};

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Magnet to (Uni/Bi):", magnetNames[module->magnet], [=](Menu * menu) {
			menu->addChild(createMenuItem("Base (0v/-5v)", CHECKMARK(module->magnet == 0), [=]() {module->magnet = 0;}));
			menu->addChild(createMenuItem("Middle (5v / 0v)", CHECKMARK(module->magnet == 1), [=]() {module->magnet = 1;}));
			menu->addChild(createMenuItem("Top (10v/5v)", CHECKMARK(module->magnet == 2), [=]() {module->magnet = 2;}));
		}));

		const std::string strengthNames[3] = {"Weak", "Average", "Strong"};

		menu->addChild(createSubmenuItem("Magnet Strength:", strengthNames[module->strength], [=](Menu * menu) {
			menu->addChild(createMenuItem("LOW", CHECKMARK(module->strength == 0), [=]() {module->strength = 0;}));
			menu->addChild(createMenuItem("MEDIUM", CHECKMARK(module->strength == 1), [=]() {module->strength = 1;}));
			menu->addChild(createMenuItem("HIGH", CHECKMARK(module->strength == 2), [=]() {module->strength = 2;}));
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

Model* modelRandMod7compact = createModel<RandMod7compact, RandMod7compactWidget>("RandMod7compact");