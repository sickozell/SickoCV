#define IDLE 0
#define WAITING_CLOCK_TO_GATE 1
#define GATING 2
#define WAITING_CLOCK_TO_IDLE 3

#include "plugin.hpp"

struct Bgates : Module {
	enum ParamId {
		RSTBUT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RSTALL_INPUT,
		ENUMS(CLOCK_INPUT,8),
		ENUMS(ARM_INPUT,8),
		ENUMS(RST_INPUT,8),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(TRIG_OUTPUT,8),
		ENUMS(GATE_OUTPUT,8),
		OUTPUTS_LEN
	};
	enum LightId {
		RSTBUT_LIGHT,
		ENUMS(WRN_LIGHT,8),
		ENUMS(OUT_LIGHT,8),
		LIGHTS_LEN
	};

	bool initStart = false;
	bool disableUnarm = false;
	bool clockConnection[8] = {false,false,false,false,false,false,false,false};
	bool prevClockConnection[8] = {false,false,false,false,false,false,false,false};
	bool clockState[8] = {false,false,false,false,false,false,false,false};
	float clock[8] = {0,0,0,0,0,0,0,0};
	float prevClock[8] = {0,0,0,0,0,0,0,0};

	bool prevGating[8] = {false,false,false,false,false,false,false,false};

	int internalState[8] = {0,0,0,0,0,0,0,0};
	bool trigState[8] = {false,false,false,false,false,false,false,false};
	float trigValue[8] = {0,0,0,0,0,0,0,0};
	float prevTrigValue[8] = {0,0,0,0,0,0,0,0};
	
	float rst[8] = {0,0,0,0,0,0,0,0};
	float prevRst[8] = {0,0,0,0,0,0,0,0};
	float rstAll = 0;
	float prevRstAll = 0;

	float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	bool trigOut[8] = {false,false,false,false,false,false,false,false};
	float trigOutTime[8] = {0,0,0,0,0,0,0,0};

	Bgates() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(RSTBUT_PARAM, 0.f, 1.f, 0.f, "Reset All", {"OFF", "ON"});
		configInput(RSTALL_INPUT, "Reset All");

		configInput(CLOCK_INPUT, "Clock #1");
		configInput(CLOCK_INPUT+1, "Clock #2");
		configInput(CLOCK_INPUT+2, "Clock #3");
		configInput(CLOCK_INPUT+3, "Clock #4");
		configInput(CLOCK_INPUT+4, "Clock #5");
		configInput(CLOCK_INPUT+5, "Clock #6");
		configInput(CLOCK_INPUT+6, "Clock #7");
		configInput(CLOCK_INPUT+7, "Clock #8");
		configInput(ARM_INPUT, "Arm #1");
		configInput(ARM_INPUT+1, "Arm #2");
		configInput(ARM_INPUT+2, "Arm #3");
		configInput(ARM_INPUT+3, "Arm #4");
		configInput(ARM_INPUT+4, "Arm #5");
		configInput(ARM_INPUT+5, "Arm #6");
		configInput(ARM_INPUT+6, "Arm #7");
		configInput(ARM_INPUT+7, "Arm #8");
		configOutput(TRIG_OUTPUT, "Trig #1");
		configOutput(TRIG_OUTPUT+1, "Trig #2");
		configOutput(TRIG_OUTPUT+2, "Trig #3");
		configOutput(TRIG_OUTPUT+3, "Trig #4");
		configOutput(TRIG_OUTPUT+4, "Trig #5");
		configOutput(TRIG_OUTPUT+5, "Trig #6");
		configOutput(TRIG_OUTPUT+6, "Trig #7");
		configOutput(TRIG_OUTPUT+7, "Trig #8");
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
		rstAll = 0;
		prevRstAll = 0;

		for (int i=0; i<8; i++) {
			clockConnection[i] = false;
			prevClockConnection[i] = false;
			clockState[i] = false;
			clock[i] = 0;
			prevClock[i] = 0;
			internalState[i] = 0;
			trigState[i] = false;
			trigValue[i] = 0;
			prevTrigValue[i] = 0;
			rst[i] = 0;
			prevRst[i] = 0;
			outputs[GATE_OUTPUT+i].setVoltage(0);
			outputs[TRIG_OUTPUT+i].setVoltage(0);
			lights[WRN_LIGHT+i].setBrightness(0.f);
			lights[OUT_LIGHT+i].setBrightness(0.f);
		}
		Module::onReset(e);
	}

	void onSampleRateChange() override {
		oneMsTime = (APP->engine->getSampleRate()) / 1000;
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

	void process(const ProcessArgs& args) override {
		
		rstAll = params[RSTBUT_PARAM].getValue();
		lights[RSTBUT_LIGHT].setBrightness(rstAll);

		rstAll += inputs[RSTALL_INPUT].getVoltage();

		if (rstAll >= 1 && prevRstAll < 1) {
			for (int i = 0; i < 8; i++) {
				if (internalState[i] == GATING) {
					outputs[TRIG_OUTPUT+i].setVoltage(10.f);
					trigOut[i] = true;
					trigOutTime[i] = oneMsTime;
				}
				outputs[GATE_OUTPUT+i].setVoltage(0.f);
				lights[OUT_LIGHT+i].setBrightness(0.f);
				lights[WRN_LIGHT+i].setBrightness(0.f);

				internalState[i] = IDLE;
			}
		}
		prevRstAll = rstAll; 

		for (int i = 0; i < 8; i++) {

			clockState[i] = false;

			if (inputs[CLOCK_INPUT+i].isConnected())
				clock[i] = inputs[CLOCK_INPUT+i].getVoltage();
			else if (i != 0)
				clock[i] = clock[i-1];


			if (clock[i] >= 1 && prevClock[i] < 1)
				clockState[i] = true;
			else
				clockState[i] = false;

			prevClock[i] = clock[i];

			if (inputs[RST_INPUT+i].isConnected()) {
				rst[i] = inputs[RST_INPUT+i].getVoltage();
				if (rst[i] >= 1 && prevRst[i] < 1) {
					if (internalState[i] == GATING) {
						outputs[TRIG_OUTPUT+i].setVoltage(10.f);
						trigOut[i] = true;
						trigOutTime[i] = oneMsTime;
					}
					outputs[GATE_OUTPUT+i].setVoltage(0.f);
					lights[OUT_LIGHT+i].setBrightness(0.f);
					lights[WRN_LIGHT+i].setBrightness(0.f);
					internalState[i] = IDLE;
				}
				prevRst[i] = rst[i]; 
			}

			if (prevGating[i] && internalState[i] == GATING) {	// this control starts attack if vcv was closed while gating
				prevGating[i] = false;
				lights[OUT_LIGHT+i].setBrightness(1.f);
				lights[WRN_LIGHT+i].setBrightness(0.f);
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
					} else if (clockState[i]) {
						
						outputs[TRIG_OUTPUT+i].setVoltage(10.f);
						trigOut[i] = true;
						trigOutTime[i] = oneMsTime;

						outputs[GATE_OUTPUT+i].setVoltage(10);

						lights[OUT_LIGHT+i].setBrightness(1.f);
						lights[WRN_LIGHT+i].setBrightness(0.f);
						internalState[i] = GATING;
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
					} else if (clockState[i]) {
						outputs[TRIG_OUTPUT+i].setVoltage(10);
						trigOut[i] = true;
						trigOutTime[i] = oneMsTime;

						outputs[GATE_OUTPUT+i].setVoltage(0);
						lights[WRN_LIGHT+i].setBrightness(0.f);
						lights[OUT_LIGHT+i].setBrightness(0.f);
						internalState[i] = IDLE;
					}
				break;
			}

			if (trigOut[i]) {
				trigOutTime[i]--;
				if (trigOutTime[i] < 0) {
					trigOut[i] = false;
					outputs[TRIG_OUTPUT+i].setVoltage(0);
				}
			}
		}
	}
};

struct BgatesWidget : ModuleWidget {
	BgatesWidget(Bgates* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Bgates.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createLightParamCentered<VCVLightBezel<YellowLight>>(mm2px(Vec(23, 14.2)), module, Bgates::RSTBUT_PARAM, Bgates::RSTBUT_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(32, 14.2)), module, Bgates::RSTALL_INPUT));
		
		const float x = 8.9;
		const float y = 12;
		const float startY = 31.0;
		const float startYled = 27.7;

		for (int i=0; i<8; i++) {
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(5.1, startY+(i*y))), module, Bgates::CLOCK_INPUT+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(5.1+x, startY+(i*y))), module, Bgates::ARM_INPUT+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(5.1+(2*x), startY+(i*y))), module, Bgates::TRIG_OUTPUT+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(5.1+(3*x), startY+(i*y))), module, Bgates::GATE_OUTPUT+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(5.1+(4*x), startY+(i*y))), module, Bgates::RST_INPUT+i));
			addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(18.45, startYled+(i*y))), module, Bgates::WRN_LIGHT+i));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(27.3, startYled+(i*y))), module, Bgates::OUT_LIGHT+i));
		}
	}

	void appendContextMenu(Menu* menu) override {
		Bgates* module = dynamic_cast<Bgates*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
		menu->addChild(createBoolPtrMenuItem("Disable Unarm", "", &module->disableUnarm));
	}
};

Model* modelBgates = createModel<Bgates, BgatesWidget>("Bgates");