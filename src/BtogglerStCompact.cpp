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

struct BtogglerStCompact : Module {
	enum ParamId {
		ATTACK_PARAM,
		SUSTAIN_PARAM,
		RELEASE_PARAM,
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

	BtogglerStCompact() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		//configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Attack", " ms", maxStageTime / minStageTime, minStageTime);
		configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Attack", "ms", 10000.f, 1.f);
		configParam(SUSTAIN_PARAM, 0.f, 1.f, 1.f, "Sustain Level", "%", 0, 100);
		//configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "Release", " ms", maxStageTime / minStageTime, minStageTime);
		configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "Release", "ms", 10000.f, 1.f);
		configInput(CLOCK_INPUT, "Clock");
		configInput(ARM_INPUT, "Arm");
		configInput(RST_INPUT, "Reset");
		configInput(IN_INPUT, "L");
		configInput(IN_INPUT+1, "R");
		configInput(ATTACK_INPUT, "Attack CV");
		configInput(SUSTAIN_INPUT, "Sustain CV");
		configInput(RELEASE_INPUT, "Release CV");
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

		sustain = params[SUSTAIN_PARAM].getValue() + inputs[SUSTAIN_INPUT].getVoltage();
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

			/*attack = convertCVToMs(params[ATTACK_PARAM].getValue()) + inputs[ATTACK_INPUT].getVoltage();
			if (attack < noEnvTime) {
				stage = SUSTAIN_STAGE;
			} else {
				stage = ATTACK_STAGE;
				if (attack > maxAdsrTime)
					attack = maxAdsrTime;
				stageCoeff = (1-stageLevel) / (args.sampleRate * attack);
			}*/

			attack = (std::pow(10000.f, params[ATTACK_PARAM].getValue()) / 1000) + inputs[ATTACK_INPUT].getVoltage();
			if (attack < noEnvTime) {
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

					/*attack = convertCVToMs(params[ATTACK_PARAM].getValue()) + inputs[ATTACK_INPUT].getVoltage();
					if (attack < noEnvTime) {
						stage = SUSTAIN_STAGE;
					} else {
						stage = ATTACK_STAGE;
						if (attack > maxAdsrTime)
							attack = maxAdsrTime;
						stageCoeff = (1-stageLevel) / (args.sampleRate * attack);
					}*/

					attack = (std::pow(10000.f, params[ATTACK_PARAM].getValue()) / 1000) + inputs[ATTACK_INPUT].getVoltage();
					if (attack < noEnvTime) {
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

					/*release = convertCVToMs(params[RELEASE_PARAM].getValue()) + inputs[RELEASE_INPUT].getVoltage();
					if (release < noEnvTime) {
						stage = STOP_STAGE;
					} else {
						stage = RELEASE_STAGE;
						if (release > maxAdsrTime)
							release = maxAdsrTime;
						stageCoeff = stageLevel / (args.sampleRate * release);
					}*/

					release = (std::pow(10000.f, params[RELEASE_PARAM].getValue()) / 1000) + inputs[RELEASE_INPUT].getVoltage();
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

struct BtogglerStCompactWidget : ModuleWidget {
	BtogglerStCompactWidget(BtogglerStCompact* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/BtogglerStCompact.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(14.5, 11.1)), module, BtogglerStCompact::CLOCK_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(10.16, 24.5)), module, BtogglerStCompact::ARM_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(10.16, 36.2)), module, BtogglerStCompact::RST_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(6.2, 52.8)), module, BtogglerStCompact::IN_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(14.5, 52.8)), module, BtogglerStCompact::IN_INPUT+1));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(6.32, 62)), module, BtogglerStCompact::ATTACK_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(6.32, 69)), module, BtogglerStCompact::ATTACK_INPUT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(14.32, 73)), module, BtogglerStCompact::SUSTAIN_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(14.32, 80)), module, BtogglerStCompact::SUSTAIN_INPUT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(6.32, 84)), module, BtogglerStCompact::RELEASE_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(6.32, 91)), module, BtogglerStCompact::RELEASE_INPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(6.2, 107.5)), module, BtogglerStCompact::OUT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(14.5, 107.5)), module, BtogglerStCompact::OUT_OUTPUT+1));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(10.16, 119)), module, BtogglerStCompact::GATE_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(16.5, 116.8)), module, BtogglerStCompact::WRN_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(16.5, 121)), module, BtogglerStCompact::OUT_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		BtogglerStCompact* module = dynamic_cast<BtogglerStCompact*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
		menu->addChild(createBoolPtrMenuItem("Disable Unarm", "", &module->disableUnarm));
		menu->addChild(createBoolPtrMenuItem("Trigger on Gate Out", "", &module->trigOnGateOut));
	}
};

Model* modelBtogglerStCompact = createModel<BtogglerStCompact, BtogglerStCompactWidget>("BtogglerStCompact");