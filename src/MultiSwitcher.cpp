#define TRIG_MODE 1
#define CV_MODE 0
#define DOWN 0
#define RANDOM 1
#define UP 2

#include "plugin.hpp"

//using namespace std;

struct MultiSwitcher : Module {
	
	int mode = TRIG_MODE;
	float trigValue = 0.f;
	float prevTrigValue = 0.f;

	int currAddr = 0;
	int prevAddr = 0;

	int direction = DOWN;

	bool fading = false;
	float xFadeKnob = 0.f;
	float xFadeValue = 0.f;
	float xFadeCoeff;

	int currInput = 0;
	int prevInput = 0;

	int rstIn = 1;
	float rstTrig = 0.f;
	float prevRstTrig = 0.f;

	int maxInputs = 7;

	int chanL;
	int chanR;
	int prevChanL;
	int prevChanR;

	float outL[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float outR[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	bool firstRun = true;

	bool initStart = false;
	int lastInputUsed = 0;

	unsigned int sampleRate = APP->engine->getSampleRate();

	enum ParamId {
		MODE_SWITCH,
		INS_PARAM,
		DIR_SWITCH,
		XFD_PARAM,
		RST_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TRG_CV_INPUT,
		RST_INPUT,
		ENUMS(IN_LEFT_INPUT, 8),
		ENUMS(IN_RIGHT_INPUT, 8),
		INPUTS_LEN
	};
	enum OutputId {
		OUT_LEFT_OUTPUT,
		OUT_RIGHT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(IN_LIGHT, 8),
		LIGHTS_LEN
	};

	MultiSwitcher() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configSwitch(MODE_SWITCH, 0.f, 1.f, 1.f, "Inputs", {"Cv", "Trig"});

		configInput(TRG_CV_INPUT, "Trg/Cv");

		configSwitch(DIR_SWITCH, 0.f, 2.f, 0.f, "Inputs", {"Down", "Random", "Up"});

		configParam(XFD_PARAM, 0.f,1.f, 0.f, "Crossfade", "ms", 10000.f, 1.f);

		configParam(RST_PARAM, 1.f,8.f, 1.f, "Reset Input");
		paramQuantities[RST_PARAM]->snapEnabled = true;

		configInput(RST_INPUT, "Reset");

		configParam(INS_PARAM, 1.f,8.f, 8.f, "Inputs selector");
		paramQuantities[INS_PARAM]->snapEnabled = true;		

		configInput(IN_LEFT_INPUT, "In1 L");
		configInput(IN_RIGHT_INPUT, "In1 R");
		configInput(IN_LEFT_INPUT+1, "In2 L");
		configInput(IN_RIGHT_INPUT+1, "In2 R");
		configInput(IN_LEFT_INPUT+2, "In3 L");
		configInput(IN_RIGHT_INPUT+2, "In3 R");
		configInput(IN_LEFT_INPUT+3, "In4 L");
		configInput(IN_RIGHT_INPUT+3, "In4 R");
		configInput(IN_LEFT_INPUT+4, "In5 L");
		configInput(IN_RIGHT_INPUT+4, "In5 R");
		configInput(IN_LEFT_INPUT+5, "In6 L");
		configInput(IN_RIGHT_INPUT+5, "In6 R");
		configInput(IN_LEFT_INPUT+6, "In7 L");
		configInput(IN_RIGHT_INPUT+6, "In7 R");
		configInput(IN_LEFT_INPUT+7, "In8 L");
		configInput(IN_RIGHT_INPUT+7, "In8 R");

		configOutput(OUT_LEFT_OUTPUT, "Left");
		configOutput(OUT_RIGHT_OUTPUT, "Right");

	}

	void onReset(const ResetEvent &e) override {
		//initStart = false;
		
		for (int i = 0; i < 8; i++) {
			lights[IN_LIGHT+i].setBrightness(0.f);
		}

		Module::onReset(e);
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "initStart", json_boolean(initStart));
		json_object_set_new(rootJ, "lastInputUsed", json_integer(currInput));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* initStartJ = json_object_get(rootJ, "initStart");
		if (initStartJ)
			initStart = json_boolean_value(initStartJ);

		if (!initStart) {
			json_t* lastInputUsedJ = json_object_get(rootJ, "lastInputUsed");
			if (lastInputUsedJ) {
				lastInputUsed = json_integer_value(lastInputUsedJ);
				if (lastInputUsed >= 0 && lastInputUsed < 8) {
					lights[IN_LIGHT+lastInputUsed].setBrightness(1.f);
					currInput = lastInputUsed;
				} else {
					lights[IN_LIGHT].setBrightness(1.f);
					currInput = 0;
				}
			}
		} else {
			lights[IN_LIGHT].setBrightness(1.f);
			currInput = 0;
		}

		/*
		fading = true;
		xFadeValue = 0;
		xFadeCoeff = 1 / (APP->engine->getSampleRate() * (std::pow(10000.f, params[XFD_PARAM].getValue()) / 1000));
		*/

	}

	void process(const ProcessArgs& args) override {
		
		//outL = 0.f;
		//outR = 0.f;

		maxInputs = params[INS_PARAM].getValue();
		
		mode = params[MODE_SWITCH].getValue();

		direction = params[DIR_SWITCH].getValue();

		rstTrig = inputs[RST_INPUT].getVoltage();
		if (rstTrig > 1.f && prevRstTrig <= 1.f) {
			prevInput = currInput;
			lights[IN_LIGHT+prevInput].setBrightness(0.f);
			currInput = params[RST_PARAM].getValue() - 1;
			lights[IN_LIGHT+currInput].setBrightness(1.f);

			xFadeKnob = params[XFD_PARAM].getValue();

			if (xFadeKnob != 0) {
				xFadeValue = 0;
				xFadeCoeff = 1 / (sampleRate * (std::pow(10000.f, xFadeKnob) / 1000));
				fading = true;
			}

		}
		prevRstTrig = rstTrig;

		switch (mode) {
			case TRIG_MODE:
				trigValue = inputs[TRG_CV_INPUT].getVoltage();
				if (trigValue > 1.f && prevTrigValue <= 1.f) {
					prevInput = currInput;
					lights[IN_LIGHT+prevInput].setBrightness(0.f);
					switch (direction) {
						case DOWN:
							currInput++;
							if (currInput > maxInputs - 1) 
								currInput = 0;
						break;

						case UP:
							currInput--;
							if (currInput < 0)
								currInput = maxInputs - 1;
						break;

						case RANDOM:
							currInput = random::uniform() * maxInputs;
							if (currInput > maxInputs)
								currInput = maxInputs- 1;
						break;
					}
					lights[IN_LIGHT+currInput].setBrightness(1.f);

					xFadeKnob = params[XFD_PARAM].getValue();

					if (xFadeKnob != 0) {
						xFadeValue = 0;
						xFadeCoeff = 1 / (sampleRate * (std::pow(10000.f, xFadeKnob) / 1000));
						fading = true;
					}
				}
				prevTrigValue = trigValue;

			break;

			case CV_MODE:
				trigValue = inputs[TRG_CV_INPUT].getVoltage();
				if (trigValue > 10.f)
					trigValue = 10.f;
				else if (trigValue < 0.f)
					trigValue = 0.f;

				if (trigValue != prevTrigValue) {
					currAddr = int(trigValue / 10 * maxInputs);
					if (currAddr >= maxInputs)
						currAddr = maxInputs-1;
					if (currAddr != prevAddr) {
						prevInput = currInput;
						lights[IN_LIGHT+prevInput].setBrightness(0.f);
						currInput = currAddr;
						lights[IN_LIGHT+currInput].setBrightness(1.f);
						
						prevAddr = currAddr;

						xFadeKnob = params[XFD_PARAM].getValue();

						if (xFadeKnob != 0) {
							xFadeValue = 0;
							xFadeCoeff = 1 / (sampleRate * (std::pow(10000.f, xFadeKnob) / 1000));
							fading = true;
						}

					}
				}
				prevTrigValue = trigValue;
			break;
		}

		if (!fading) {

			chanL = inputs[IN_LEFT_INPUT+currInput].getChannels();

			for (int c = 0; c < chanL; c++) {
				outL[c] = inputs[IN_LEFT_INPUT+currInput].getVoltage(c);

				if (outL[c] > 10.f)
					outL[c] = 10.f;
				else if (outL[c] < -10.f)
					outL[c] = -10.f;
				outputs[OUT_LEFT_OUTPUT].setVoltage(outL[c], c);
			}
			outputs[OUT_LEFT_OUTPUT].setChannels(chanL);

			if (inputs[IN_RIGHT_INPUT+currInput].isConnected()) {

				chanR = inputs[IN_RIGHT_INPUT+currInput].getChannels();

				for (int c = 0; c < chanR; c++) {
					outR[c] = inputs[IN_RIGHT_INPUT+currInput].getVoltage(c);
					if (outR[c] > 10.f)
						outR[c] = 10.f;
					else if (outR[c] < -10.f)
						outR[c] = -10.f;
					outputs[OUT_RIGHT_OUTPUT].setVoltage(outR[c], c);
				}
				outputs[OUT_RIGHT_OUTPUT].setChannels(chanR);
				
			} else {
				for (int c = 0; c < chanL; c++)
					outputs[OUT_RIGHT_OUTPUT].setVoltage(outL[c], c);
				outputs[OUT_RIGHT_OUTPUT].setChannels(chanL);
			}

		} else {

			xFadeValue += xFadeCoeff;
			if (xFadeValue > 1) {
				xFadeValue = 1;
				fading = false;
			}

			// LEFT CHANNEL

			chanL = inputs[IN_LEFT_INPUT+currInput].getChannels();
			prevChanL = inputs[IN_LEFT_INPUT+prevInput].getChannels();

			for (int c = 0; c < prevChanL; c++)
				outL[c] = inputs[IN_LEFT_INPUT+prevInput].getVoltage(c) * (1-xFadeValue);

			if (chanL > prevChanL) {
				for (int c = prevChanL; c < chanL; c++) {
					outL[c] = inputs[IN_LEFT_INPUT+prevInput].getVoltage(c) * (1-xFadeValue);
					outputs[OUT_LEFT_OUTPUT].setVoltage(outL[c], c);
				}
			}

			for (int c = 0; c < chanL; c++) {
				outL[c] += inputs[IN_LEFT_INPUT+currInput].getVoltage(c) * xFadeValue;

				if (outL[c] > 10.f)
					outL[c] = 10.f;
				else if (outL[c] < -10.f)
					outL[c] = -10.f;
				
				outputs[OUT_LEFT_OUTPUT].setVoltage(outL[c], c);
			}

			if (prevChanL > chanL) {
				for (int c = chanL; c < prevChanL; c++) {
					outL[c] += inputs[IN_LEFT_INPUT+currInput].getVoltage(c) * xFadeValue;

					if (outL[c] > 10.f)
						outL[c] = 10.f;
					else if (outL[c] < -10.f)
						outL[c] = -10.f;
					
					outputs[OUT_LEFT_OUTPUT].setVoltage(outL[c], c);
				}
			}

			// set left channel

			if (chanL > prevChanL)
				outputs[OUT_LEFT_OUTPUT].setChannels(chanL);
			else
				outputs[OUT_LEFT_OUTPUT].setChannels(prevChanL);

			// RIGHT CHANNEL

			if (inputs[IN_RIGHT_INPUT+currInput].isConnected()) 
				chanR = inputs[IN_RIGHT_INPUT+currInput].getChannels();
			else
				chanR = chanL;

			if (inputs[IN_RIGHT_INPUT+prevInput].isConnected()) 
				prevChanR = inputs[IN_RIGHT_INPUT+prevInput].getChannels();
			else
				prevChanR = prevChanR;

			// prev right channel

			if (inputs[IN_RIGHT_INPUT+prevInput].isConnected()) {
				for (int c = 0; c < prevChanR; c++)
					outR[c] = inputs[IN_RIGHT_INPUT+prevInput].getVoltage(c) * (1-xFadeValue);

				if (chanR > prevChanR) {
					for (int c = prevChanR; c < chanR; c++) {
						outR[c] = inputs[IN_RIGHT_INPUT+prevInput].getVoltage(c) * (1-xFadeValue);
						outputs[OUT_RIGHT_OUTPUT].setVoltage(outR[c], c);
					}
				}
			} else {
				prevChanR = prevChanL;
				for (int c = 0; c < prevChanR; c++)
					outR[c] = inputs[IN_LEFT_INPUT+prevInput].getVoltage(c) * (1-xFadeValue);

				if (chanR > prevChanR) {
					for (int c = prevChanR; c < chanR; c++) {
						outR[c] = inputs[IN_LEFT_INPUT+prevInput].getVoltage(c) * (1-xFadeValue);
						outputs[OUT_RIGHT_OUTPUT].setVoltage(outR[c], c);
					}
				}
			}

			// curr right channel

			if (inputs[IN_RIGHT_INPUT+currInput].isConnected()) {

				for (int c = 0; c < chanR; c++) {
					outR[c] += inputs[IN_RIGHT_INPUT+currInput].getVoltage(c) * xFadeValue;

					if (outR[c] > 10.f)
						outR[c] = 10.f;
					else if (outR[c] < -10.f)
						outR[c] = -10.f;

					outputs[OUT_RIGHT_OUTPUT].setVoltage(outR[c], c);
				}

				if (prevChanR > chanR) {
					for (int c = chanR; c < prevChanR; c++) {
						outR[c] += inputs[IN_RIGHT_INPUT+currInput].getVoltage(c) * xFadeValue;

						if (outR[c] > 10.f)
							outR[c] = 10.f;
						else if (outR[c] < -10.f)
							outR[c] = -10.f;
						
						outputs[OUT_RIGHT_OUTPUT].setVoltage(outR[c], c);
					}
				}

			} else {

				chanR = chanL;

				for (int c = 0; c < chanL; c++) {
					outR[c] += inputs[IN_LEFT_INPUT+currInput].getVoltage(c) * xFadeValue;

					if (outR[c] > 10.f)
						outR[c] = 10.f;
					else if (outR[c] < -10.f)
						outR[c] = -10.f;

					outputs[OUT_RIGHT_OUTPUT].setVoltage(outR[c], c);
				}

				if (prevChanR > chanR) {
					for (int c = chanR; c < prevChanR; c++) {
						outR[c] += inputs[IN_LEFT_INPUT+currInput].getVoltage(c) * xFadeValue;

						if (outR[c] > 10.f)
							outR[c] = 10.f;
						else if (outR[c] < -10.f)
							outR[c] = -10.f;
						
						outputs[OUT_RIGHT_OUTPUT].setVoltage(outR[c], c);
					}
				}

			}

			// set right channel

			if (chanR > prevChanR)
				outputs[OUT_RIGHT_OUTPUT].setChannels(chanR);
			else
				outputs[OUT_RIGHT_OUTPUT].setChannels(prevChanR);

		}
		
	}		
};

struct MultiSwitcherWidget : ModuleWidget {
	MultiSwitcherWidget(MultiSwitcher* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/MultiSwitcher.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float xLeft = 6.8;
		const float yCvSw = 19.5;
		const float yCvSwIn = 35;

		const float xDir = 5.8;
		const float yDir = 59.8;

		const float yXfd = 85.6;
		const float yRstKn = 105.7;
		const float yRstIn = 116;

		const float xIns = 28.5;
		const float yIns = 15.6;

		const float xInL = 19.25;
		const float xInR = 29.05;
		const float xLight = 24.17;

		constexpr float ys = 26.5;
		constexpr float yShift = 10;

		const float yOut = 116;

		addParam(createParamCentered<CKSS>(mm2px(Vec(xLeft, yCvSw)), module, MultiSwitcher::MODE_SWITCH));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yCvSwIn)), module, MultiSwitcher::TRG_CV_INPUT));
		addParam(createParamCentered<CKSSThree>(mm2px(Vec(xDir, yDir)), module, MultiSwitcher::DIR_SWITCH));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xLeft, yXfd)), module, MultiSwitcher::XFD_PARAM));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xLeft, yRstKn)), module, MultiSwitcher::RST_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yRstIn)), module, MultiSwitcher::RST_INPUT));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xIns, yIns)), module, MultiSwitcher::INS_PARAM));

		for (int i = 0; i < 8; i++) {
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInL, ys+(i*yShift))), module, MultiSwitcher::IN_LEFT_INPUT+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInR, ys+(i*yShift))), module, MultiSwitcher::IN_RIGHT_INPUT+i));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(xLight, ys+(i*yShift)-2.5)), module, MultiSwitcher::IN_LIGHT+i));
		}

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xInL, yOut)), module, MultiSwitcher::OUT_LEFT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xInR, yOut)), module, MultiSwitcher::OUT_RIGHT_OUTPUT));

	}

	void appendContextMenu(Menu* menu) override {
		MultiSwitcher* module = dynamic_cast<MultiSwitcher*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));

	}
};

Model* modelMultiSwitcher = createModel<MultiSwitcher, MultiSwitcherWidget>("MultiSwitcher");