#define GATE_MODE 0
#define TOGGLE_MODE 1
#define IDLE 0
#define WAITING_CLOCK_TO_GATE 1
#define GATING 2
#define WAITING_CLOCK_TO_IDLE 3
#define STOP_STAGE 0
#define ATTACK_STAGE 1
#define DECAY_STAGE 2
#define SUSTAIN_STAGE 3
#define RELEASE_STAGE 4
#define MAXADSRTIME 10.f

#include "plugin.hpp"

struct BtogglerSt : Module {
	enum ParamId {
		ATTACK_PARAM,
		ATTACKATNV_PARAM,
		SUSTAIN_PARAM,
		SUSTAINATNV_PARAM,
		RELEASE_PARAM,
		RELEASEATNV_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CLOCK_INPUT,
		ARM_INPUT,
		RST_INPUT,
		ENUMS(IN_INPUT,2),
		ATTACK_INPUT,
		SUSTAIN_INPUT,
		RELEASE_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT,2),
		GATE_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		WRN_LIGHT,
		OUT_LIGHT,
		LIGHTS_LEN
	};

	bool initStart = false;
	bool disableUnarm = false;
	bool trigOnGateOut = false;
	bool clockState = false;
	float clock = 0;
	float prevClock = 0;

	bool prevGating = false;

	int internalState = IDLE;
	bool trigState = false;
	float trigValue = 0;
	float prevTrigValue = 0;

	float rst = 0;
	float prevRst = 0;

	float attack;
	float sustain;
	float release;

	int stage = STOP_STAGE;
	float stageLevel = 0;
	float stageCoeff;

	float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	bool trigOut = false;
	float trigOutTime = 0.f;

	int chan;

	/*static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	const float maxAdsrTime = 10.f;*/
	const float noEnvTime = 0.00101;

	BtogglerSt() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		//configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Attack", "ms", maxStageTime / minStageTime, minStageTime);
		configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Attack", "ms", 10000.f, 1.f);
		configParam(ATTACKATNV_PARAM, -1.f, 1.f, 0.f, "Attack CV", "%", 0, 100);
		configParam(SUSTAIN_PARAM, 0.f, 1.f, 1.f, "Sustain Level", "%", 0, 100);
		configParam(SUSTAINATNV_PARAM, -1.f, 1.f, 0.f, "Sustain CV", "%", 0, 100);
		//configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "Release", " ms", maxStageTime / minStageTime, minStageTime);
		configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "Release", "ms", 10000.f, 1.f);
		configParam(RELEASEATNV_PARAM, -1.f, 1.f, 0.f, "Release CV", "%", 0, 100);
		configInput(CLOCK_INPUT, "Clock");
		configInput(ARM_INPUT, "Arm");
		configInput(RST_INPUT, "Reset");
		configInput(IN_INPUT, "L");
		configInput(IN_INPUT+1, "R");
		configInput(ATTACK_INPUT, "Attack");
		configInput(SUSTAIN_INPUT, "Sustain");
		configInput(RELEASE_INPUT, "Release");
		configOutput(OUT_OUTPUT, "L");
		configOutput(OUT_OUTPUT+1, "R");
		configOutput(GATE_OUTPUT, "Gate");
	}
	
	void onReset(const ResetEvent &e) override {
		initStart = false;
		disableUnarm = false;
		trigOnGateOut = false;
		internalState = IDLE;
		prevGating = false;
		clockState = false;
		clock = 0;
		prevClock = 0;
		trigState = false;
		trigValue = 0;
		prevTrigValue = 0;
		rst = 0;
		prevRst = 0;
		outputs[GATE_OUTPUT].setVoltage(0);
		outputs[OUT_OUTPUT].setVoltage(0);
		outputs[OUT_OUTPUT+1].setVoltage(0);
		lights[WRN_LIGHT].setBrightness(0.f);
		lights[OUT_LIGHT].setBrightness(0.f);
		Module::onReset(e);
	}

	void onSampleRateChange() override {
		oneMsTime = (APP->engine->getSampleRate()) / 1000;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "InitStart", json_boolean(initStart));
		json_object_set_new(rootJ, "DisableUnarm", json_boolean(disableUnarm));
		json_object_set_new(rootJ, "TrigOnGateOut", json_boolean(trigOnGateOut));
		json_object_set_new(rootJ, "State", json_integer(internalState));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* initStartJ = json_object_get(rootJ, "InitStart");
		if (initStartJ)
			initStart = json_boolean_value(initStartJ);

		json_t* disableUnarmJ = json_object_get(rootJ, "DisableUnarm");
		if (disableUnarmJ)
			disableUnarm = json_boolean_value(disableUnarmJ);

		json_t* trigOnGateOutJ = json_object_get(rootJ, "TrigOnGateOut");
		if (trigOnGateOutJ)
			trigOnGateOut = json_boolean_value(trigOnGateOutJ);

		if (!initStart) {
			json_t* jsonState = json_object_get(rootJ, "State");
			if (jsonState){
				internalState = json_integer_value(jsonState);
				prevGating = true;
			}
		}
	}

	/*static float convertCVToMs(float cv) {		
		//return minStageTime * std::pow(maxStageTime / minStageTime, cv) / 1000;
		return std::pow(10000.f, cv) / 1000;
	}*/

	void process(const ProcessArgs& args) override {

		sustain = params[SUSTAIN_PARAM].getValue() + (inputs[SUSTAIN_INPUT].getVoltage() * params[SUSTAINATNV_PARAM].getValue() * 0.1);
		if (sustain > 1)
			sustain = 1;
		else if (sustain < 0)
			sustain = 0;

		clock = inputs[CLOCK_INPUT].getVoltage();
		
		if (clock >= 1 && prevClock < 1)
			clockState = true;
		else
			clockState = false;

		prevClock = clock;

		if (inputs[RST_INPUT].isConnected()) {
			rst = inputs[RST_INPUT].getVoltage();
			if (rst >= 1 && prevRst < 1) {
				outputs[GATE_OUTPUT].setVoltage(0.f);
				if (trigOnGateOut && internalState == GATING) {
					outputs[GATE_OUTPUT].setVoltage(10.f);
					trigOut = true;
					trigOutTime = oneMsTime;
				}
				lights[OUT_LIGHT].setBrightness(0.f);
				lights[WRN_LIGHT].setBrightness(0.f);
				stage = STOP_STAGE;
				stageLevel = 0;
				internalState = IDLE;
			}
			prevRst = rst; 
		}

		if (prevGating && internalState == GATING) {	// this control starts attack if vcv was closed while gating
			prevGating = false;
			lights[OUT_LIGHT].setBrightness(1.f);
			lights[WRN_LIGHT].setBrightness(0.f);

			/*attack = convertCVToMs(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
			if (attack < noEnvTime) {
				stage = SUSTAIN_STAGE;
			} else {
				stage = ATTACK_STAGE;
				if (attack > MAXADSRTIME)
					attack = MAXADSRTIME;
				stageCoeff = (1-stageLevel) / (args.sampleRate * attack);
			}*/

			attack = (std::pow(10000.f, params[ATTACK_PARAM].getValue()) / 1000) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
			if (attack <= noEnvTime) {
				attack = 0;
				stage = SUSTAIN_STAGE;
			} else {
				stage = ATTACK_STAGE;
				if (attack > MAXADSRTIME)
					attack = MAXADSRTIME;
				stageCoeff = (1-stageLevel) / (args.sampleRate * attack);
			}
		}

		trigValue = inputs[ARM_INPUT].getVoltage();
		if (trigValue >= 1 && prevTrigValue < 1){
			trigState = true;
		} else {
			trigState = false;
		}
		prevTrigValue = trigValue;

		switch (internalState) {

			case IDLE:
				if (trigState) {
					internalState = WAITING_CLOCK_TO_GATE;
					lights[WRN_LIGHT].setBrightness(1.f);
				}
			break;

			case WAITING_CLOCK_TO_GATE:
				if (trigState && !disableUnarm) {
					outputs[GATE_OUTPUT].setVoltage(0);
					lights[WRN_LIGHT].setBrightness(0.f);
					internalState = IDLE;
				} else if (clockState) {
					//outputs[GATE_OUTPUT].setVoltage(10);
					if (!trigOnGateOut) {
						outputs[GATE_OUTPUT].setVoltage(10);
					} else {
						outputs[GATE_OUTPUT].setVoltage(10);
						trigOut = true;
						trigOutTime = oneMsTime;
					}
					lights[OUT_LIGHT].setBrightness(1.f);
					lights[WRN_LIGHT].setBrightness(0.f);
					internalState = GATING;

					/*attack = convertCVToMs(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
					if (attack < noEnvTime) {
						stage = SUSTAIN_STAGE;
					} else {
						stage = ATTACK_STAGE;
						if (attack > MAXADSRTIME)
							attack = MAXADSRTIME;
						stageCoeff = (1-stageLevel) / (args.sampleRate * attack);
					}*/

					attack = (std::pow(10000.f, params[ATTACK_PARAM].getValue()) / 1000) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
					if (attack <= noEnvTime) {
						attack = 0;
						stage = SUSTAIN_STAGE;
					} else {
						stage = ATTACK_STAGE;
						if (attack > MAXADSRTIME)
							attack = MAXADSRTIME;
						stageCoeff = (1-stageLevel) / (args.sampleRate * attack);
					}
				}
			break;

			case GATING:
				//outputs[GATE_OUTPUT].setVoltage(10);
				if (!trigOnGateOut)
					outputs[GATE_OUTPUT].setVoltage(10);
				lights[OUT_LIGHT].setBrightness(1.f);
				if (trigState) {
					lights[WRN_LIGHT].setBrightness(1.f);
					internalState = WAITING_CLOCK_TO_IDLE;
				}
			break;

			case WAITING_CLOCK_TO_IDLE:
				if (trigState && !disableUnarm) {
					internalState = GATING;
					lights[WRN_LIGHT].setBrightness(0.f);
				} else if (clockState) {
					//outputs[GATE_OUTPUT].setVoltage(0);
					if (!trigOnGateOut)
						outputs[GATE_OUTPUT].setVoltage(0);
					else {
						outputs[GATE_OUTPUT].setVoltage(10);
						trigOut = true;
						trigOutTime = oneMsTime;
					}
					lights[WRN_LIGHT].setBrightness(0.f);
					lights[OUT_LIGHT].setBrightness(0.f);
					internalState = IDLE;

					/*release = convertCVToMs(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
					if (release < noEnvTime) {
						stage = STOP_STAGE;
					} else {
						stage = RELEASE_STAGE;
						if (release > MAXADSRTIME)
							release = MAXADSRTIME;
						stageCoeff = stageLevel / (args.sampleRate * release);
					}*/

					release = (std::pow(10000.f, params[RELEASE_PARAM].getValue()) / 1000) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
					if (release < noEnvTime) {
						release = 0;
						stage = STOP_STAGE;
					} else {
						stage = RELEASE_STAGE;
						if (release > MAXADSRTIME)
							release = MAXADSRTIME;
						stageCoeff = stageLevel / (args.sampleRate * release);
					}
				}
			break;
		}

		switch (stage) {
			case STOP_STAGE:
				stageLevel = 0;
			break;

			case ATTACK_STAGE:
				stageLevel += stageCoeff;
				if (stageLevel >= sustain) {
					stageLevel = sustain;
					stage = SUSTAIN_STAGE;
				}
			break;

			case SUSTAIN_STAGE:
				stageLevel = sustain;
			break;

			case RELEASE_STAGE:
				stageLevel -= stageCoeff;
				if (stageLevel < 0) {
					stageLevel = 0;
					stage = STOP_STAGE;
				}
			break;
		}

		chan = std::max(1, inputs[IN_INPUT].getChannels());

		if (inputs[IN_INPUT].isConnected()) {
			//outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage() * stageLevel);
			for (int c = 0; c < chan; c++)
				outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage(c) * stageLevel, c);
			outputs[OUT_OUTPUT].setChannels(chan);
		} else {
			//outputs[OUT_OUTPUT].setVoltage(10 * stageLevel);
			outputs[OUT_OUTPUT].setVoltage(10.f * stageLevel , 0);
			outputs[OUT_OUTPUT].setChannels(1);
		}

		chan = std::max(1, inputs[IN_INPUT+1].getChannels());

		if (inputs[IN_INPUT+1].isConnected()) {
			//outputs[OUT_OUTPUT+1].setVoltage(inputs[IN_INPUT+1].getVoltage() * stageLevel);
			for (int c = 0; c < chan; c++)
				outputs[OUT_OUTPUT+1].setVoltage(inputs[IN_INPUT+1].getVoltage(c) * stageLevel, c);
			outputs[OUT_OUTPUT+1].setChannels(chan);
		} else {
			//outputs[OUT_OUTPUT+1].setVoltage(10 * stageLevel);
			outputs[OUT_OUTPUT+1].setVoltage(10.f * stageLevel , 0);
			outputs[OUT_OUTPUT+1].setChannels(1);
		}

		if (trigOnGateOut) {
			if (trigOut) {
				trigOutTime--;
				if (trigOutTime < 0) {
					trigOut = false;
					outputs[GATE_OUTPUT].setVoltage(0);
				}
			}
		}
	}
};

struct BtogglerStWidget : ModuleWidget {
	BtogglerStWidget(BtogglerSt* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/BtogglerSt.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(30, 15.68)), module, BtogglerSt::CLOCK_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(12.5, 37.5)), module, BtogglerSt::ARM_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(33, 37.5)), module, BtogglerSt::RST_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(8.48, 60)), module, BtogglerSt::ATTACK_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(8.48, 71.5)), module, BtogglerSt::ATTACKATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(8.48, 80.5)), module, BtogglerSt::ATTACK_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(22.8, 60)), module, BtogglerSt::SUSTAIN_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(22.8, 71.5)), module, BtogglerSt::SUSTAINATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(22.8, 80.5)), module, BtogglerSt::SUSTAIN_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(37.32, 60)), module, BtogglerSt::RELEASE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(37.32, 71.5)), module, BtogglerSt::RELEASEATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(37.32, 80.5)), module, BtogglerSt::RELEASE_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(7, 108.8)), module, BtogglerSt::IN_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(16.5, 108.8)), module, BtogglerSt::IN_INPUT+1));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(29, 103.2)), module, BtogglerSt::OUT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(39.2, 103.2)), module, BtogglerSt::OUT_OUTPUT+1));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(34, 116.5)), module, BtogglerSt::GATE_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(41.2, 114.5)), module, BtogglerSt::WRN_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(41.2, 118.7)), module, BtogglerSt::OUT_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		BtogglerSt* module = dynamic_cast<BtogglerSt*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
		menu->addChild(createBoolPtrMenuItem("Disable Unarm", "", &module->disableUnarm));
		menu->addChild(createBoolPtrMenuItem("Trigger on Gate Out", "", &module->trigOnGateOut));
	}
};

Model* modelBtogglerSt = createModel<BtogglerSt, BtogglerStWidget>("BtogglerSt");