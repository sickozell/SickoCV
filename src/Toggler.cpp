#define GATE_MODE 0
#define TOGGLE_MODE 1
#define IDLE 0
#define GATING 1
#define STOP_STAGE 0
#define ATTACK_STAGE 1
#define DECAY_STAGE 2
#define SUSTAIN_STAGE 3
#define RELEASE_STAGE 4
#define MAXADSRTIME 10.f

#include "plugin.hpp"

#if defined(METAMODULE_BUILTIN)
struct SickoToggler : Module {
#else
struct Toggler : Module {
#endif

	enum ParamId {
		MODE_SWITCH,
		ATTACK_PARAM,
		ATTACKATNV_PARAM,
		SUSTAIN_PARAM,
		SUSTAINATNV_PARAM,
		RELEASE_PARAM,
		RELEASEATNV_PARAM,
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

	int chan;

	/*static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	const float maxAdsrTime = 10.f;*/
	const float noEnvTime = 0.00101;
	
	#if defined(METAMODULE_BUILTIN)
	SickoToggler() {
	#else
	Toggler() {
	#endif
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(MODE_SWITCH, 0.f, 1.f, 1.f, "Mode", {"Gate", "Toggle"});
		//configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Attack", " ms", maxStageTime / minStageTime, minStageTime);
		configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Attack", "ms", 10000.f, 1.f);
		configParam(ATTACKATNV_PARAM, -1.f, 1.f, 0.f, "Attack CV", "%", 0, 100);
		configParam(SUSTAIN_PARAM, 0.f, 1.f, 1.f, "Sustain Level", "%", 0, 100);
		configParam(SUSTAINATNV_PARAM, -1.f, 1.f, 0.f, "Sustain CV", "%", 0, 100);
		//configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "Release", " ms", maxStageTime / minStageTime, minStageTime);
		configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "Release", "ms", 10000.f, 1.f);
		configParam(RELEASEATNV_PARAM, -1.f, 1.f, 0.f, "Release CV", "%", 0, 100);
		configInput(TRIG_INPUT, "Trig/Gate");
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

	/*static float convertCVToSeconds(float cv) {		
		return minStageTime * std::pow(maxStageTime / minStageTime, cv) / 1000;
	}*/
	
	void process(const ProcessArgs& args) override {

		sustain = params[SUSTAIN_PARAM].getValue() + (inputs[SUSTAIN_INPUT].getVoltage() * params[SUSTAINATNV_PARAM].getValue() * 0.1);
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
					//attack = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
					attack = (std::pow(10000.f, params[ATTACK_PARAM].getValue()) / 1000) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
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

					//attack = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
					attack = (std::pow(10000.f, params[ATTACK_PARAM].getValue()) / 1000) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
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
				outputs[GATE_OUTPUT].setVoltage(10);
				if ((mode == GATE_MODE && !trigState) || (mode == TOGGLE_MODE && trigState)) { 		// if GATE goes LOW
					outputs[GATE_OUTPUT].setVoltage(0);
					lights[OUT_LIGHT].setBrightness(0.f);
					internalState = 0;

					//release = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
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
			for (int c = 0; c < chan; c++)
				outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage(c) * stageLevel, c);
			outputs[OUT_OUTPUT].setChannels(chan);
		} else {
			outputs[OUT_OUTPUT].setVoltage(10.f * stageLevel , 0);
			outputs[OUT_OUTPUT].setChannels(1);
		}

		chan = std::max(1, inputs[IN_INPUT+1].getChannels());

		if (inputs[IN_INPUT+1].isConnected()) {
			for (int c = 0; c < chan; c++)
				outputs[OUT_OUTPUT+1].setVoltage(inputs[IN_INPUT+1].getVoltage(c) * stageLevel, c);
			outputs[OUT_OUTPUT+1].setChannels(chan);
		} else {
			outputs[OUT_OUTPUT+1].setVoltage(10.f * stageLevel , 0);
			outputs[OUT_OUTPUT+1].setChannels(1);
		}
	}
};


struct TogglerWidget : ModuleWidget {
#if defined(METAMODULE_BUILTIN)
	TogglerWidget(SickoToggler* module) {

		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Toggler.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<CKSS>(mm2px(Vec(21.458, 15.75)), module, SickoToggler::MODE_SWITCH));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(12.5, 38.5)), module, SickoToggler::TRIG_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(33, 38.5)), module, SickoToggler::RST_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(8.48, 60)), module, SickoToggler::ATTACK_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(8.48, 71.5)), module, SickoToggler::ATTACKATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(8.48, 80.5)), module, SickoToggler::ATTACK_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(22.8, 60)), module, SickoToggler::SUSTAIN_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(22.8, 71.5)), module, SickoToggler::SUSTAINATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(22.8, 80.5)), module, SickoToggler::SUSTAIN_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(37.32, 60)), module, SickoToggler::RELEASE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(37.32, 71.5)), module, SickoToggler::RELEASEATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(37.32, 80.5)), module, SickoToggler::RELEASE_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(7, 108.8)), module, SickoToggler::IN_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(16.5, 108.8)), module, SickoToggler::IN_INPUT+1));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(29, 103.2)), module, SickoToggler::OUT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(39.2, 103.2)), module, SickoToggler::OUT_OUTPUT+1));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(34, 116.5)), module, SickoToggler::GATE_OUTPUT));

		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(41.2, 118.7)), module, SickoToggler::OUT_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		SickoToggler* module = dynamic_cast<SickoToggler*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}
};

Model* modelToggler = createModel<SickoToggler, TogglerWidget>("Toggler");

#else
	TogglerWidget(Toggler* module) {

		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Toggler.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<CKSS>(mm2px(Vec(21.458, 15.75)), module, Toggler::MODE_SWITCH));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(12.5, 38.5)), module, Toggler::TRIG_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(33, 38.5)), module, Toggler::RST_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(8.48, 60)), module, Toggler::ATTACK_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(8.48, 71.5)), module, Toggler::ATTACKATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(8.48, 80.5)), module, Toggler::ATTACK_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(22.8, 60)), module, Toggler::SUSTAIN_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(22.8, 71.5)), module, Toggler::SUSTAINATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(22.8, 80.5)), module, Toggler::SUSTAIN_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(37.32, 60)), module, Toggler::RELEASE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(37.32, 71.5)), module, Toggler::RELEASEATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(37.32, 80.5)), module, Toggler::RELEASE_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(7, 108.8)), module, Toggler::IN_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(16.5, 108.8)), module, Toggler::IN_INPUT+1));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(29, 103.2)), module, Toggler::OUT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(39.2, 103.2)), module, Toggler::OUT_OUTPUT+1));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(34, 116.5)), module, Toggler::GATE_OUTPUT));

		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(41.2, 118.7)), module, Toggler::OUT_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		Toggler* module = dynamic_cast<Toggler*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}
};

Model* modelToggler = createModel<Toggler, TogglerWidget>("Toggler");
#endif