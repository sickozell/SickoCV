#define IDLE 0
#define WAITING_CLOCK_TO_GATE 1
#define GATING 2
#define WAITING_CLOCK_TO_IDLE 3
#define STOP_STAGE 0
#define ATTACK_STAGE 1
#define DECAY_STAGE 2
#define SUSTAIN_STAGE 3
#define RELEASE_STAGE 4
#define SUSTAIN 1.f

#include "plugin.hpp"

struct Btoggler : Module {
	enum ParamId {
		FADE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CLOCK_INPUT,
		RSTALL_INPUT,
		ENUMS(ARM_INPUT,8),
		ENUMS(IN_INPUT,8),
		ENUMS(RST_INPUT,8),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT,8),
		ENUMS(GATE_OUTPUT,8),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(WRN_LIGHT,8),
		ENUMS(OUT_LIGHT,8),
		LIGHTS_LEN
	};

	bool initStart = false;
	bool disableUnarm = false;
	bool clockConnection = false;
	bool prevClockConnection = false;
	bool clockState = false;
	float clock = 0;
	float prevClock = 0;

	bool prevGating[8] = {false,false,false,false,false,false,false,false};

	int internalState[8] = {0,0,0,0,0,0,0,0};
	bool trigState[8] = {false,false,false,false,false,false,false,false};
	float trigValue[8] = {0,0,0,0,0,0,0,0};
	float prevTrigValue[8] = {0,0,0,0,0,0,0,0};
	
	float rst[8] = {0,0,0,0,0,0,0,0};
	float prevRst[8] = {0,0,0,0,0,0,0,0};
	float rstAll = 0;
	float prevRstAll = 0;

	float attack[8];
	//const float sustain = 1.f;
	float release[8];
	
	int stage[8] = {0,0,0,0,0,0,0,0};
	float stageLevel[8] = {0,0,0,0,0,0,0,0};
	float stageCoeff[8];

	/*static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	const float maxAdsrTime = 10.f;
	const float noEnvTime = 0.00101;
	const float noEnvTime = 0.f;*/

	Btoggler() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(CLOCK_INPUT, "Clock");
		//configParam(FADE_PARAM, 0.f, 1.f, 0.f, "Fade", "ms", maxStageTime / minStageTime, minStageTime);
		configParam(FADE_PARAM, 0.f, 1.f, 0.f, "Fade", "ms", 10000.f, 1.f);
		configInput(RSTALL_INPUT, "Reset all toggles");
		configInput(ARM_INPUT, "Arm #1");
		configInput(ARM_INPUT+1, "Arm #2");
		configInput(ARM_INPUT+2, "Arm #3");
		configInput(ARM_INPUT+3, "Arm #4");
		configInput(ARM_INPUT+4, "Arm #5");
		configInput(ARM_INPUT+5, "Arm #6");
		configInput(ARM_INPUT+6, "Arm #7");
		configInput(ARM_INPUT+7, "Arm #8");
		configInput(IN_INPUT, "Input #1");
		configInput(IN_INPUT+1, "Input #2");
		configInput(IN_INPUT+2, "Input #3");
		configInput(IN_INPUT+3, "Input #4");
		configInput(IN_INPUT+4, "Input #5");
		configInput(IN_INPUT+5, "Input #6");
		configInput(IN_INPUT+6, "Input #7");
		configInput(IN_INPUT+7, "Input #8");
		configOutput(OUT_OUTPUT, "Output #1");
		configOutput(OUT_OUTPUT+1, "Output #2");
		configOutput(OUT_OUTPUT+2, "Output #3");
		configOutput(OUT_OUTPUT+3, "Output #4");
		configOutput(OUT_OUTPUT+4, "Output #5");
		configOutput(OUT_OUTPUT+5, "Output #6");
		configOutput(OUT_OUTPUT+6, "Output #7");
		configOutput(OUT_OUTPUT+7, "Output #8");
		configOutput(GATE_OUTPUT, "Gate #1");
		configOutput(GATE_OUTPUT+1, "Gate #2");
		configOutput(GATE_OUTPUT+2, "Gate #3");
		configOutput(GATE_OUTPUT+3, "Gate #4");
		configOutput(GATE_OUTPUT+4, "Gate #5");
		configOutput(GATE_OUTPUT+5, "Gate #6");
		configOutput(GATE_OUTPUT+6, "Gate #7");
		configOutput(GATE_OUTPUT+7, "Gate #8");
		configInput(RST_INPUT, "Reset #1");
		configInput(RST_INPUT+1, "Reset #2");
		configInput(RST_INPUT+2, "Reset #3");
		configInput(RST_INPUT+3, "Reset #4");
		configInput(RST_INPUT+4, "Reset #5");
		configInput(RST_INPUT+5, "Reset #6");
		configInput(RST_INPUT+6, "Reset #7");
		configInput(RST_INPUT+7, "Reset #8");
	}
	
	void onReset(const ResetEvent &e) override {
		initStart = false;
		disableUnarm = false;
		clockConnection = false;
		prevClockConnection = false;
		clockState = false;
		clock = 0;
		prevClock = 0;
		rstAll = 0;
		prevRstAll = 0;

		for (int i=0; i<8; i++) {
			internalState[i] = 0;
			trigState[i] = false;
			trigValue[i] = 0;
			prevTrigValue[i] = 0;
			rst[i] = 0;
			prevRst[i] = 0;
			outputs[GATE_OUTPUT+i].setVoltage(0);
			outputs[OUT_OUTPUT+i].setVoltage(0);
			lights[WRN_LIGHT+i].setBrightness(0.f);
			lights[OUT_LIGHT+i].setBrightness(0.f);
		}
		Module::onReset(e);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "InitStart", json_boolean(initStart));
		json_object_set_new(rootJ, "DisableUnarm", json_boolean(disableUnarm));
		json_object_set_new(rootJ, "State1", json_integer(internalState[0]));
		json_object_set_new(rootJ, "State2", json_integer(internalState[1]));
		json_object_set_new(rootJ, "State3", json_integer(internalState[2]));
		json_object_set_new(rootJ, "State4", json_integer(internalState[3]));
		json_object_set_new(rootJ, "State5", json_integer(internalState[4]));
		json_object_set_new(rootJ, "State6", json_integer(internalState[5]));
		json_object_set_new(rootJ, "State7", json_integer(internalState[6]));
		json_object_set_new(rootJ, "State8", json_integer(internalState[7]));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* initStartJ = json_object_get(rootJ, "InitStart");
		if (initStartJ)
			initStart = json_boolean_value(initStartJ);

		json_t* disableUnarmJ = json_object_get(rootJ, "DisableUnarm");
		if (disableUnarmJ)
			disableUnarm = json_boolean_value(disableUnarmJ);

		if (!initStart) {
			json_t* jsonState1 = json_object_get(rootJ, "State1");
			if (jsonState1) {
				internalState[0] = json_integer_value(jsonState1);
				if (internalState[0]) {
					lights[OUT_LIGHT].setBrightness(1.f);
					prevGating[0] = true;
				}
			}

			json_t* jsonState2 = json_object_get(rootJ, "State2");
			if (jsonState2) {
				internalState[1] = json_integer_value(jsonState2);
				if (internalState[1]) {
					lights[OUT_LIGHT+1].setBrightness(1.f);
					prevGating[1] = true;
				}
			}

			json_t* jsonState3 = json_object_get(rootJ, "State3");
			if (jsonState3) {
				internalState[2] = json_integer_value(jsonState3);
				if (internalState[2]) {
					lights[OUT_LIGHT+2].setBrightness(1.f);
					prevGating[2] = true;
				}
			}

			json_t* jsonState4 = json_object_get(rootJ, "State4");
			if (jsonState4) {
				internalState[3] = json_integer_value(jsonState4);
				if (internalState[3]) {
					lights[OUT_LIGHT+3].setBrightness(1.f);
					prevGating[3] = true;
				}
			}

			json_t* jsonState5 = json_object_get(rootJ, "State5");
			if (jsonState5) {
				internalState[4] = json_integer_value(jsonState5);
				if (internalState[4]) {
					lights[OUT_LIGHT+4].setBrightness(1.f);
					prevGating[4] = true;
				}
			}

			json_t* jsonState6 = json_object_get(rootJ, "State6");
			if (jsonState6) {
				internalState[5] = json_integer_value(jsonState6);
				if (internalState[5]) {
					lights[OUT_LIGHT+5].setBrightness(1.f);
					prevGating[5] = true;
				}
			}

			json_t* jsonState7 = json_object_get(rootJ, "State7");
			if (jsonState7) {
				internalState[6] = json_integer_value(jsonState7);
				if (internalState[6]) {
					lights[OUT_LIGHT+6].setBrightness(1.f);
					prevGating[6] = true;
				}
			}

			json_t* jsonState8 = json_object_get(rootJ, "State8");
			if (jsonState8) {
				internalState[7] = json_integer_value(jsonState8);
				if (internalState[7]) {
					lights[OUT_LIGHT+7].setBrightness(1.f);
					prevGating[7] = true;
				}
			}
		}
	}

	/*static float convertCVToMs(float cv) {		
		//return minStageTime * std::pow(maxStageTime / minStageTime, cv) / 1000;
		return std::pow(10000.f, cv) / 1000;
	}*/

	void process(const ProcessArgs& args) override {

		clock = inputs[CLOCK_INPUT].getVoltage();
		if (clock >= 1 && prevClock < 1)
			clockState = true;
		else
			clockState = false;

		prevClock = clock;

		if (inputs[RSTALL_INPUT].isConnected()) {
			rstAll = inputs[RSTALL_INPUT].getVoltage();
			if (rstAll >= 1 && prevRstAll < 1) {
				for (int i = 0; i < 8; i++) {
					outputs[GATE_OUTPUT+i].setVoltage(0.f);
					lights[OUT_LIGHT+i].setBrightness(0.f);
					lights[WRN_LIGHT+i].setBrightness(0.f);
					stage[i] = STOP_STAGE;
					stageLevel[i] = 0;
					internalState[i] = IDLE;
				}
			}
			prevRstAll = rstAll; 
		}

		for (int i = 0; i < 8; i++) {

			if (inputs[RST_INPUT+i].isConnected()) {
				rst[i] = inputs[RST_INPUT+i].getVoltage();
				if (rst[i] >= 1 && prevRst[i] < 1) {
					outputs[GATE_OUTPUT+i].setVoltage(0.f);
					lights[OUT_LIGHT+i].setBrightness(0.f);
					lights[WRN_LIGHT+i].setBrightness(0.f);
					stage[i] = STOP_STAGE;
					stageLevel[i] = 0;
					internalState[i] = IDLE;
				}
				prevRst[i] = rst[i]; 
			}

			if (prevGating[i] && internalState[i] == GATING) {	// this control starts attack if vcv was closed while gating
				prevGating[i] = false;
				lights[OUT_LIGHT+i].setBrightness(1.f);
				lights[WRN_LIGHT+i].setBrightness(0.f);

				/*attack[i] = convertCVToSeconds(params[FADE_PARAM].getValue());
				if (attack[i] < noEnvTime) {
					stage[i] = SUSTAIN_STAGE;
					stageLevel[i] = sustain;
				} else {
					stage[i] = ATTACK_STAGE;
					if (attack[i] > MAXADSRTIME)
						attack[i] = MAXADSRTIME;
					stageCoeff[i] = (1-stageLevel[i]) / (args.sampleRate * attack[i]);
				}*/
				
				attack[i] = params[FADE_PARAM].getValue();
				if (attack[i] == 0) {
					stage[i] = SUSTAIN_STAGE;
					stageLevel[i] = SUSTAIN;
				} else {
					//attack[i] = convertCVToMs(attack[i]);
					attack[i] = std::pow(10000.f, attack[i]) / 1000;
					stage[i] = ATTACK_STAGE;
					stageCoeff[i] = (1-stageLevel[i]) / (args.sampleRate * attack[i]);
				}

			}

			trigValue[i] = inputs[ARM_INPUT+i].getVoltage();
			if (trigValue[i] >= 1 && prevTrigValue[i] < 1)
				trigState[i] = true;
			else
				trigState[i] = false;

			prevTrigValue[i] = trigValue[i];

			switch (internalState[i]) {
				case IDLE:
					if (trigState[i]){
						internalState[i] = WAITING_CLOCK_TO_GATE;
						lights[WRN_LIGHT+i].setBrightness(1.f);
					}
				break;

				case WAITING_CLOCK_TO_GATE:
					if (trigState[i] && !disableUnarm) {
						outputs[GATE_OUTPUT+i].setVoltage(0);
						lights[WRN_LIGHT+i].setBrightness(0.f);
						internalState[i] = IDLE;
					} else if (clockState) {
						outputs[GATE_OUTPUT+i].setVoltage(10);
						lights[OUT_LIGHT+i].setBrightness(1.f);
						lights[WRN_LIGHT+i].setBrightness(0.f);
						internalState[i] = GATING;

						/*attack[i] = convertCVToSeconds(params[FADE_PARAM].getValue());
						if (attack[i] < noEnvTime) {
							stage[i] = SUSTAIN_STAGE;
							stageLevel[i] = sustain;
						} else {
							stage[i] = ATTACK_STAGE;
							if (attack[i] > MAXADSRTIME)
								attack[i] = MAXADSRTIME;
							stageCoeff[i] = (1-stageLevel[i]) / (args.sampleRate * attack[i]);
						}*/

						attack[i] = params[FADE_PARAM].getValue();
						if (attack[i] == 0) {
							stage[i] = SUSTAIN_STAGE;
							stageLevel[i] = SUSTAIN;
						} else {
							//attack[i] = convertCVToMs(attack[i]);
							attack[i] = std::pow(10000.f, attack[i]) / 1000;
							stage[i] = ATTACK_STAGE;
							stageCoeff[i] = (1-stageLevel[i]) / (args.sampleRate * attack[i]);
						}
					}
				break;

				case GATING:
					outputs[GATE_OUTPUT+i].setVoltage(10);
					lights[OUT_LIGHT+i].setBrightness(1.f);
					if (trigState[i]) {
						lights[WRN_LIGHT+i].setBrightness(1.f);
						internalState[i] = WAITING_CLOCK_TO_IDLE;
					}
				break;

				case WAITING_CLOCK_TO_IDLE:
					if (trigState[i] && !disableUnarm) {
						internalState[i] = GATING;
						lights[WRN_LIGHT+i].setBrightness(0.f);
					} else if (clockState) {
						outputs[GATE_OUTPUT+i].setVoltage(0);
						lights[WRN_LIGHT+i].setBrightness(0.f);
						lights[OUT_LIGHT+i].setBrightness(0.f);
						internalState[i] = IDLE;

						/*release[i] = convertCVToSeconds(params[FADE_PARAM].getValue());
						if (release[i] < noEnvTime) {
							stage[i] = STOP_STAGE;
							stageLevel[i] = 0;
						} else {
							stage[i] = RELEASE_STAGE;
							if (release[i] > MAXADSRTIME)
								release[i] = MAXADSRTIME;
							stageCoeff[i] = stageLevel[i] / (args.sampleRate * release[i]);
						}*/
						release[i] = params[FADE_PARAM].getValue();
						if (release[i] == 0) {
							stage[i] = STOP_STAGE;
							stageLevel[i] = 0;
						} else {
							//release[i] = convertCVToMs(release[i]);
							release[i] = std::pow(10000.f, release[i]) / 1000;
							stage[i] = RELEASE_STAGE;
							stageCoeff[i] = stageLevel[i] / (args.sampleRate * release[i]);
						}
					}
				break;
			}

			switch (stage[i]) {
				/*
				case STOP_STAGE:
					stageLevel[i] = 0;
				break;
				*/

				case ATTACK_STAGE:
					stageLevel[i] += stageCoeff[i];
					if (stageLevel[i] >= SUSTAIN) {
						stage[i] = SUSTAIN_STAGE;
						stageLevel[i] = SUSTAIN;
					}
				break;

				/*
				case SUSTAIN_STAGE:
					stageLevel[i] = 1;
				break;
				*/

				case RELEASE_STAGE:
					stageLevel[i] -= stageCoeff[i];
					if (stageLevel[i] < 0) {
						stage[i] = STOP_STAGE;
						stageLevel[i] = 0;
					}
				break;
			}

			if (inputs[IN_INPUT+i].isConnected())
				outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage() * stageLevel[i]);
			else
				outputs[OUT_OUTPUT+i].setVoltage(10 * stageLevel[i]);
		}
		clockState = false;
	}
};

struct BtogglerWidget : ModuleWidget {
	BtogglerWidget(Btoggler* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Btoggler.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(8, 21.6)), module, Btoggler::CLOCK_INPUT));
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(23, 21.6)), module, Btoggler::FADE_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(37.7, 21.6)), module, Btoggler::RSTALL_INPUT));
		
		const float x = 8.9;
		const float y = 10.8;
		for (int i=0; i<8; i++) {
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(5.1, 41+(i*y))), module, Btoggler::ARM_INPUT+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(5.1+x, 41+(i*y))), module, Btoggler::IN_INPUT+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(5.1+(2*x), 41+(i*y))), module, Btoggler::OUT_OUTPUT+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(5.1+(3*x), 41+(i*y))), module, Btoggler::GATE_OUTPUT+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(5.1+(4*x), 41+(i*y))), module, Btoggler::RST_INPUT+i));
			addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(18.45, 37.7+(i*y))), module, Btoggler::WRN_LIGHT+i));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(27.3, 37.7+(i*y))), module, Btoggler::OUT_LIGHT+i));
		}
	}

	void appendContextMenu(Menu* menu) override {
		Btoggler* module = dynamic_cast<Btoggler*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
		menu->addChild(createBoolPtrMenuItem("Disable Unarm", "", &module->disableUnarm));
	}
};

Model* modelBtoggler = createModel<Btoggler, BtogglerWidget>("Btoggler");