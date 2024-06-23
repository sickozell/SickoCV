#include "plugin.hpp"

struct Shifter : Module {
	enum ParamId {
		DELAY_PARAMS,
		STAGE_PARAMS,
		ATNV_PARAMS,
		PARAMS_LEN
	};
	enum InputId {
		TRIG_INPUT,
		IN_INPUT,
		STAGE_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		STAGE_LIGHT,
		LIGHTS_LEN
	};

	bool initStart = false;
	bool outConnection = false;
	bool prevOutConnection = true;

	int regLength = 64;
	float registerValue[65] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
								0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
								0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
								0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
								0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
								0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
								0.f, 0.f, 0.f};
	int regStage = 1;
	int prevRegStage = 1;

	int currentStep = 1;

	float trigValue = 0.f;
	float prevTrigValue = 0.f;

	int preDelay = 0;
	int delayCounter = 0;

	bool trigState = false;

	Shifter() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(DELAY_PARAMS, 0.f, 5.f, 1.f, "Trigger Delay (samples)");
		paramQuantities[DELAY_PARAMS]->snapEnabled = true;
		configParam(STAGE_PARAMS, 1.f, (float)(regLength), 0.f, "Register Stage");
		paramQuantities[STAGE_PARAMS]->snapEnabled = true;
		configParam(ATNV_PARAMS, -1.f, 1.f, 0.f, "Stage CV", "%", 0, 100);
		configInput(TRIG_INPUT, "Trig");
		configInput(IN_INPUT, "IN");
		configInput(STAGE_INPUT, "Stage");
		configOutput(OUT_OUTPUT, "OUT");
	}

	void onReset(const ResetEvent &e) override {
		initStart = false;
		currentStep = 1;
		for (int i=0; i<65; i++)
			registerValue[i] = 0.f;
	    Module::onReset(e);
	}

	void processBypass(const ProcessArgs &args) override {
		outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage());
		Module::processBypass(args);
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "InitStart", json_boolean(initStart));
		json_object_set_new(rootJ, "CurrentStep", json_integer(currentStep));
		
		json_t *registerJ = json_array();
		for (int i=0; i<65; i++)
			json_array_insert_new(registerJ, i, json_real(registerValue[i]));
		json_object_set_new(rootJ, "Register", registerJ);
		return rootJ;
	}
	
	void dataFromJson(json_t *rootJ) override {
		json_t *initStartJ = json_object_get(rootJ, "InitStart");
		if (initStartJ)
			initStart = json_boolean_value(initStartJ);

		if (!initStart) {
			json_t *currentStepJ = json_object_get(rootJ, "CurrentStep");
			if (currentStepJ)
				currentStep = json_integer_value(currentStepJ);

			json_t *registerJ = json_object_get(rootJ, "Register");
			if (registerJ)
				for (int i=0; i<65; i++) {
					json_t *registerArrayJ = json_array_get(registerJ, i);
					if (registerArrayJ)
						registerValue[i] = json_number_value(registerArrayJ);
				}
		}
	}

	void process(const ProcessArgs& args) override {
		preDelay = params[DELAY_PARAMS].getValue();
		
		if (inputs[OUT_OUTPUT].isConnected())
			outConnection = true;
		else
			outConnection = false;

		if (inputs[STAGE_INPUT].isConnected())
			regStage = params[STAGE_PARAMS].getValue() + int((inputs[STAGE_INPUT].getVoltage() / 0.15625) * params[ATNV_PARAMS].getValue());
		else
			regStage = int (params[STAGE_PARAMS].getValue() * (1-params[ATNV_PARAMS].getValue()));

		if (regStage > 64)
			regStage = 64;
		else
			if (regStage < 1)
				regStage = 1;
		
		if (regStage != prevRegStage || outConnection != prevOutConnection) {
			if (currentStep - regStage >= 0)
				outputs[OUT_OUTPUT].setVoltage(registerValue[currentStep - regStage]);
			else
				outputs[OUT_OUTPUT].setVoltage(registerValue[regLength - regStage + currentStep]);

			prevRegStage = regStage;
			prevOutConnection = outConnection;
		}

		if (inputs[TRIG_INPUT].isConnected()) {
			trigValue = inputs[TRIG_INPUT].getVoltage();
			if (trigValue >= 1 && prevTrigValue < 1)
				trigState = true;

			prevTrigValue = trigValue;
			if (trigState) {
				if (delayCounter >= preDelay) {
					registerValue[currentStep] = inputs[IN_INPUT].getVoltage();
					delayCounter = 0;
					trigState = false;

					if (currentStep - regStage >= 0)
						outputs[OUT_OUTPUT].setVoltage(registerValue[currentStep - regStage + 1]);
					else
						outputs[OUT_OUTPUT].setVoltage(registerValue[regLength - regStage + currentStep + 1]);
					
					currentStep++;
					
					if (currentStep > regLength)
						currentStep = 1;

				} else {
					delayCounter++;
				}
			}
			prevTrigValue = trigValue;
		}
		
	}
};

struct TextDisplayWidget : TransparentWidget {
	Shifter *module;
	
	TextDisplayWidget() {
	};

	void drawLayer(const DrawArgs &args, int layer) override {
		if (layer ==1) {
			std::shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf"));
			int val = module ? module->regStage : 64;

		    nvgFontSize(args.vg, 12);
		    nvgFontFaceId(args.vg, font->handle);
		    nvgTextLetterSpacing(args.vg, 1.5);

		    std::string to_display = std::to_string(val);

		    while(to_display.length()<2) to_display = '0' + to_display;

		    Vec textPos = Vec(0.f, 0.f);
		    NVGcolor textColor = nvgRGB(0xdd, 0x33, 0x33);

		    nvgFillColor(args.vg, textColor);
		    nvgText(args.vg, textPos.x, textPos.y, to_display.c_str(), NULL);
		}
		Widget::drawLayer(args, layer);
	}
};

struct ShifterWidget : ModuleWidget {
	ShifterWidget(Shifter* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Shifter.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(7.62, 18)), module, Shifter::TRIG_INPUT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(7.62, 36.1)), module, Shifter::DELAY_PARAMS));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(7.62, 63.3)), module, Shifter::STAGE_PARAMS));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(7.62, 76)), module, Shifter::ATNV_PARAMS));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(7.62, 84.5)), module, Shifter::STAGE_INPUT));
		

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(7.62, 100.8)), module, Shifter::IN_INPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(7.62, 117.4)), module, Shifter::OUT_OUTPUT));

		TextDisplayWidget *display = new TextDisplayWidget();
		display->box.pos = Vec(12,164);
		display->box.size = Vec(2, 2);
		display->module = module;
		addChild(display);
	}

	void appendContextMenu(Menu* menu) override {
		Shifter* module = dynamic_cast<Shifter*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}

};

Model* modelShifter = createModel<Shifter, ShifterWidget>("Shifter");