#define GATE_MODE 0
#define TOGGLE_MODE 1
#define IDLE 0
#define GATING 1
#define STOP_STAGE 0
#define ATTACK_STAGE 1
#define DECAY_STAGE 2
#define SUSTAIN_STAGE 3
#define RELEASE_STAGE 4

#include "plugin.hpp"

struct TogglerCompact : Module {
	enum ParamId {
		MODE_SWITCH,
		ATTACK_PARAM,
		SUSTAIN_PARAM,
		RELEASE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TRIG_INPUT,
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
		OUT_LIGHT,
		LIGHTS_LEN
	};

	bool initStart = false;
	bool prevGating = false;
	int mode = 1;
	int internalState = 0;
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

	static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	const float maxAdsrTime = 10.f;
	const float noEnvTime = 0.00101;

	TogglerCompact() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(MODE_SWITCH, 0.f, 1.f, 1.f, "Mode", {"Gate", "Toggle"});
		configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Attack", " ms", maxStageTime / minStageTime, minStageTime);
		configParam(SUSTAIN_PARAM, 0.f, 1.f, 1.f, "Level", "%", 0, 100);
		configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "Release", " ms", maxStageTime / minStageTime, minStageTime);
		configInput(TRIG_INPUT, "Trig/Gate");
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
		prevGating = false;
		mode = 1;
		internalState = 0;
		trigState = false;
		trigValue = 0;
		prevTrigValue = 0;
		rst = 0;
		prevRst = 0;

		outputs[GATE_OUTPUT].setVoltage(0);
		outputs[OUT_OUTPUT].setVoltage(0);
		outputs[OUT_OUTPUT+1].setVoltage(0);
		lights[OUT_LIGHT].setBrightness(0.f);
		Module::onReset(e);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "InitStart", json_boolean(initStart));
		json_object_set_new(rootJ, "State", json_integer(internalState));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* initStartJ = json_object_get(rootJ, "InitStart");
		if (initStartJ)
			initStart = json_boolean_value(initStartJ);

		if (!initStart) {
			json_t* jsonState = json_object_get(rootJ, "State");
			if (jsonState){
				internalState = json_integer_value(jsonState);
				prevGating = true;
			}
		}
	}

	static float convertCVToSeconds(float cv) {		
		return minStageTime * std::pow(maxStageTime / minStageTime, cv) / 1000;
	}
	
	void process(const ProcessArgs& args) override {

		sustain = params[SUSTAIN_PARAM].getValue() + inputs[SUSTAIN_INPUT].getVoltage();
		if (sustain > 1)
			sustain = 1;
		else if (sustain < 0)
			sustain = 0;

		mode = params[MODE_SWITCH].getValue();
		trigValue = inputs[TRIG_INPUT].getVoltage();
		switch (mode) {
			case GATE_MODE:
				if (trigValue >= 1 && prevTrigValue < 1)
					trigState = true;
				else if (trigValue < 1 && prevTrigValue >= 1)
					trigState = false;
			break;

			case TOGGLE_MODE:
				if (inputs[RST_INPUT].isConnected()) {
					rst = inputs[RST_INPUT].getVoltage();
					if (rst >= 1 && prevRst < 1) {
						outputs[GATE_OUTPUT].setVoltage(0.f);
						lights[OUT_LIGHT].setBrightness(0.f);
						stage = STOP_STAGE;
						stageLevel = 0;
						internalState = 0;
					}
					prevRst = rst; 
				}

				if (prevGating && internalState == GATING) {	// this control starts attack if vcv was closed while gating
					prevGating = false;
					outputs[GATE_OUTPUT].setVoltage(10);
					lights[OUT_LIGHT].setBrightness(1.f);
					attack = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + inputs[ATTACK_INPUT].getVoltage();
					if (attack < noEnvTime) {
						stage = SUSTAIN_STAGE;
					} else {
						stage = ATTACK_STAGE;
						if (attack > maxAdsrTime)
							attack = maxAdsrTime;
						stageCoeff = (1-stageLevel) / (args.sampleRate * attack);
					}
				}

				if (trigValue >= 1 && prevTrigValue < 1)
					trigState = true;
				else
					trigState = false;
				
			break;			
		}
		prevTrigValue = trigValue;

		switch (internalState) {
			case IDLE:
				if (trigState) {
					outputs[GATE_OUTPUT].setVoltage(10);
					lights[OUT_LIGHT].setBrightness(1.f);
					internalState = 1;

					attack = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + inputs[ATTACK_INPUT].getVoltage();
					if (attack < noEnvTime) {
						stage = SUSTAIN_STAGE;
					} else {
						stage = ATTACK_STAGE;
						if (attack > maxAdsrTime)
							attack = maxAdsrTime;
						stageCoeff = (1-stageLevel) / (args.sampleRate * attack);
					}
				}
			break;

			case GATING:
				outputs[GATE_OUTPUT].setVoltage(10);
				if ((mode == GATE_MODE && !trigState) || (mode == TOGGLE_MODE && trigState)) {
					outputs[GATE_OUTPUT].setVoltage(0);
					lights[OUT_LIGHT].setBrightness(0.f);
					internalState = 0;

					release = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + inputs[RELEASE_INPUT].getVoltage();
					if (release < noEnvTime) {
						stage = STOP_STAGE;
					} else {
						stage = RELEASE_STAGE;
						if (release > maxAdsrTime)
							release = maxAdsrTime;
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

		if (inputs[IN_INPUT].isConnected())
			outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage() * stageLevel);
		else
			outputs[OUT_OUTPUT].setVoltage(10 * stageLevel);

		if (inputs[IN_INPUT+1].isConnected())
			outputs[OUT_OUTPUT+1].setVoltage(inputs[IN_INPUT+1].getVoltage() * stageLevel);
		else
			outputs[OUT_OUTPUT+1].setVoltage(10 * stageLevel);
	}
};

struct TogglerCompactWidget : ModuleWidget {
	TogglerCompactWidget(TogglerCompact* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/TogglerCompact.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<CKSS>(mm2px(Vec(4, 11.35)), module, TogglerCompact::MODE_SWITCH));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 24.5)), module, TogglerCompact::TRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 36.2)), module, TogglerCompact::RST_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.2, 52.8)), module, TogglerCompact::IN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.5, 52.8)), module, TogglerCompact::IN_INPUT+1));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(6.32, 62)), module, TogglerCompact::ATTACK_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.32, 69)), module, TogglerCompact::ATTACK_INPUT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.32, 73)), module, TogglerCompact::SUSTAIN_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.32, 80)), module, TogglerCompact::SUSTAIN_INPUT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(6.32, 84)), module, TogglerCompact::RELEASE_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.32, 91)), module, TogglerCompact::RELEASE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.2, 107.5)), module, TogglerCompact::OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.5, 107.5)), module, TogglerCompact::OUT_OUTPUT+1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 119)), module, TogglerCompact::GATE_OUTPUT));

		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(16.5, 121)), module, TogglerCompact::OUT_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		TogglerCompact* module = dynamic_cast<TogglerCompact*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}
};

Model* modelTogglerCompact = createModel<TogglerCompact, TogglerCompactWidget>("TogglerCompact");