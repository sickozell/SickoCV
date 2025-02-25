#include "plugin.hpp"

//using namespace std;	// this is for debug

struct Modulator7Compact : Module {
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

	double sampleRateCoeff = APP->engine->getSampleRate() / 2;
	double rateProvv = 1.f;
	double rate = 1.f;
	double rateKnob = 0.5f;
	double prevRateKnob = 1.f;
	double xRate[7] = {1, 1, 1, 1, 1, 1, 1};
	double xRateKnob[7] = {1, 1, 1, 1, 1, 1, 1};
	double prevXrateKnob[7] = {0, 0, 0, 0, 0, 0, 0};
	double waveCoeff = 0;
	double waveValue[7] = {0, 0, 0, 0, 0, 0, 0};

	int waveSlope[7] = {1, 1, 1, 1, 1, 1, 1};

	int polyChans = 1;
	float out = 0;

	int bipolar = 0;

	int biType[2] = {0, -5};

	static constexpr float minRate = 0.01f;  // in milliseconds
	static constexpr float maxRate = 100.f;  // in milliseconds

	static constexpr float minXrate = 0.047619f;  // in milliseconds
	static constexpr float maxXrate = 21.f;  // in milliseconds

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

	Modulator7Compact() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RATE_PARAM, 0.f, 1.f, 0.5f, "Rate", "Hz", maxRate / minRate, minRate);
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
		prevRateKnob = 1;
		//polyChans = 1;
		for (int i = 0; i < 7; i++) {
			waveSlope[i] = 1;
			waveValue[i] = 0;
			xRateKnob[i] = 1;
			prevXrateKnob[i] = 0;
		}

		waveCoeff = 0;

	    Module::onReset(e);
	}

	void onSampleRateChange() override {
		sampleRateCoeff = APP->engine->getSampleRate() / 2;
	}

	
	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "polyChans", json_integer(polyChans));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* polyChansJ = json_object_get(rootJ, "polyChans");
		if (polyChansJ)
			polyChans = json_integer_value(polyChansJ);
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

	static float convertXrateToSeconds(float cv) {		
		return minXrate * std::pow(maxXrate / minXrate, cv);
	}

	static float convertRateToSeconds(float cv) {		
		return minRate * std::pow(maxRate / minRate, cv);
	}

	void process(const ProcessArgs& args) override {

		bipolar = int(params[BIPOLAR_PARAM].getValue());
		lights[BIPOLAR_LIGHT].setBrightness(bipolar);

		rateKnob = params[RATE_PARAM].getValue();
		
		if (rateKnob != prevRateKnob) {
			rateProvv = convertRateToSeconds(rateKnob);
			prevRateKnob = rateKnob;
		}

		rate = rateProvv + (inputs[RATE_INPUT].getVoltage() * params[RATE_ATTENUV_PARAM].getValue() * 10);

		if (rate > 100)
			rate = 100;
		else if (rate < 0.01)
			rate = 0.01;

		waveCoeff = rate / sampleRateCoeff;

		for (int i = 0; i < 6; i++) {

			xRateKnob[i] = params[XRATE_PARAM+i].getValue();

			if (xRateKnob[i] != prevXrateKnob[i]) {
				xRate[i] = convertXrateToSeconds(xRateKnob[i]);
				prevXrateKnob[i] = xRateKnob[i];
			}

			waveValue[i] += xRate[i] * waveCoeff * waveSlope[i];
					
			if (waveValue[i] > 1) {
				waveSlope[i] = -1;
				waveValue[i] = 2 - waveValue[i];
			} else if (waveValue[i] < 0) {
				waveSlope[i] = 1;
				waveValue[i] = -waveValue[i];
			}

			out = waveValue[i] * 10.f;
			if (polyChans >= i){
				outputs[OUT_OUTPUT+6].setVoltage(out+biType[bipolar], i);	
			}
			outputs[OUT_OUTPUT+i].setVoltage(out+biType[bipolar]);

		}

		xRateKnob[6] = params[XRATE_PARAM+6].getValue();

		if (xRateKnob[6] != prevXrateKnob[6]) {
			xRate[6] = convertXrateToSeconds(xRateKnob[6]);
			prevXrateKnob[6] = xRateKnob[6];
		}

		waveValue[6] += xRate[6] * waveCoeff * waveSlope[6];
				
		if (waveValue[6] > 1) {
			waveSlope[6] = -1;
			waveValue[6] = 2 - waveValue[6];
		} else if (waveValue[6] < 0) {
			waveSlope[6] = 1;
			waveValue[6] = -waveValue[6];
		}

		out = waveValue[6] * 10.f;

		if (polyChans == 1) {
			outputs[OUT_OUTPUT+6].setVoltage(out+biType[bipolar]);
			outputs[OUT_OUTPUT+6].setChannels(1);
		} else {
			outputs[OUT_OUTPUT+6].setVoltage(out+biType[bipolar], 6);
			outputs[OUT_OUTPUT+6].setChannels(polyChans);
		}
			
		
		
		
		//debugDisplay = to_string(params[XRATE_PARAM].getValue());
	}
};

/*
struct Modulator7CompactDebugDisplay : TransparentWidget {
	Modulator7Compact *module;
	int frame = 0;
	Modulator7CompactDebugDisplay() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-bold.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff)); 
				
				nvgTextBox(args.vg, 9, 6,120, module->debugDisplay.c_str(), NULL);
				nvgTextBox(args.vg, 9, 16,120, module->debugDisplay2.c_str(), NULL);
				nvgTextBox(args.vg, 129, 6,120, module->debugDisplay3.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 16,120, module->debugDisplay4.c_str(), NULL);

			}
		}
		Widget::drawLayer(args, layer);
	}
};
*/

struct Modulator7CompactWidget : ModuleWidget {
	Modulator7CompactWidget(Modulator7Compact* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Modulator7Compact.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	 

		/*
		{
			Modulator7CompactDebugDisplay *display = new Modulator7CompactDebugDisplay();
			display->box.pos = Vec(23, 3);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/

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

		addParam(createParamCentered<SickoLargeKnob>(mm2px(Vec(xRtKnob, yRtKnob)), module, Modulator7Compact::RATE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xRateAtnv, yRateAtnv)), module, Modulator7Compact::RATE_ATTENUV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRateIn, yRateIn)), module, Modulator7Compact::RATE_INPUT));

		for (int i = 0; i < 7; i++) {
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xRt, yStart + (yStartShift * i))), module, Modulator7Compact::XRATE_PARAM+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yStart + (yStartShift * i))), module, Modulator7Compact::OUT_OUTPUT+i));
		}

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(xBi, yBi)), module, Modulator7Compact::BIPOLAR_PARAM, Modulator7Compact::BIPOLAR_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		Modulator7Compact* module = dynamic_cast<Modulator7Compact*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Polyphony on 7th out", std::to_string(module->polyChans), [=](Menu * menu) {
			menu->addChild(createMenuItem("Monophonic", "", [=]() {module->polyChans = 1;}));
			menu->addChild(createMenuItem("2", "", [=]() {module->polyChans = 2;}));
			menu->addChild(createMenuItem("3", "", [=]() {module->polyChans = 3;}));
			menu->addChild(createMenuItem("4", "", [=]() {module->polyChans = 4;}));
			menu->addChild(createMenuItem("5", "", [=]() {module->polyChans = 5;}));
			menu->addChild(createMenuItem("6", "", [=]() {module->polyChans = 6;}));
			menu->addChild(createMenuItem("7", "", [=]() {module->polyChans = 7;}));
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

Model* modelModulator7Compact = createModel<Modulator7Compact, Modulator7CompactWidget>("Modulator7Compact");