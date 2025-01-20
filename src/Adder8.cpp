#include "plugin.hpp"
 
struct Adder8 : Module {
	
	bool outMode = true;	//  true: Stop On Cable ---- false: sum all outputs
	int voltDefaultOption = 0;
	bool attenuator = false;
	float voltDefaults[3] = {0.f, 1.f, 10.f};
	float cv[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

	enum ParamId {
		ENUMS(ADDSUB_SWITCH,8),
		ENUMS(VOLT_PARAMS,8),
		ENUMS(MODE_SWITCH,8),
		OUTMODE_SWITCH,
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(CV_INPUT,8),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT,8),
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Adder8() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configInput(CV_INPUT+0, "CV #1");
		configInput(CV_INPUT+1, "CV #2");
		configInput(CV_INPUT+2, "CV #3");
		configInput(CV_INPUT+3, "CV #4");
		configInput(CV_INPUT+4, "CV #5");
		configInput(CV_INPUT+5, "CV #6");
		configInput(CV_INPUT+6, "CV #7");
		configInput(CV_INPUT+7, "CV #8");

		configSwitch(ADDSUB_SWITCH+0, -1.f, 1.f, 0.f, "Operation", {"Subtract", "Off", "Add"});
		configSwitch(ADDSUB_SWITCH+1, -1.f, 1.f, 0.f, "Operation", {"Subtract", "Off", "Add"});
		configSwitch(ADDSUB_SWITCH+2, -1.f, 1.f, 0.f, "Operation", {"Subtract", "Off", "Add"});
		configSwitch(ADDSUB_SWITCH+3, -1.f, 1.f, 0.f, "Operation", {"Subtract", "Off", "Add"});
		configSwitch(ADDSUB_SWITCH+4, -1.f, 1.f, 0.f, "Operation", {"Subtract", "Off", "Add"});
		configSwitch(ADDSUB_SWITCH+5, -1.f, 1.f, 0.f, "Operation", {"Subtract", "Off", "Add"});
		configSwitch(ADDSUB_SWITCH+6, -1.f, 1.f, 0.f, "Operation", {"Subtract", "Off", "Add"});
		configSwitch(ADDSUB_SWITCH+7, -1.f, 1.f, 0.f, "Operation", {"Subtract", "Off", "Add"});

		configParam(VOLT_PARAMS+0, -10.f,10.f, 0.f, "Volt", "v");
		configParam(VOLT_PARAMS+1, -10.f,10.f, 0.f, "Volt", "v");
		configParam(VOLT_PARAMS+2, -10.f,10.f, 0.f, "Volt", "v");
		configParam(VOLT_PARAMS+3, -10.f,10.f, 0.f, "Volt", "v");
		configParam(VOLT_PARAMS+4, -10.f,10.f, 0.f, "Volt", "v");
		configParam(VOLT_PARAMS+5, -10.f,10.f, 0.f, "Volt", "v");
		configParam(VOLT_PARAMS+6, -10.f,10.f, 0.f, "Volt", "v");
		configParam(VOLT_PARAMS+7, -10.f,10.f, 0.f, "Volt", "v");

		configSwitch(MODE_SWITCH+0, -1.f, 1.f, 0.f, "Mode", {"Subtract/Off", "Add/Off/Subtract", "Off/Add"});
		configSwitch(MODE_SWITCH+1, -1.f, 1.f, 0.f, "Mode", {"Subtract/Off", "Add/Off/Subtract", "Off/Add"});
		configSwitch(MODE_SWITCH+2, -1.f, 1.f, 0.f, "Mode", {"Subtract/Off", "Add/Off/Subtract", "Off/Add"});
		configSwitch(MODE_SWITCH+3, -1.f, 1.f, 0.f, "Mode", {"Subtract/Off", "Add/Off/Subtract", "Off/Add"});
		configSwitch(MODE_SWITCH+4, -1.f, 1.f, 0.f, "Mode", {"Subtract/Off", "Add/Off/Subtract", "Off/Add"});
		configSwitch(MODE_SWITCH+5, -1.f, 1.f, 0.f, "Mode", {"Subtract/Off", "Add/Off/Subtract", "Off/Add"});
		configSwitch(MODE_SWITCH+6, -1.f, 1.f, 0.f, "Mode", {"Subtract/Off", "Add/Off/Subtract", "Off/Add"});
		configSwitch(MODE_SWITCH+7, -1.f, 1.f, 0.f, "Mode", {"Subtract/Off", "Add/Off/Subtract", "Off/Add"});

		configOutput(OUT_OUTPUT+0, "Out #1");
		configOutput(OUT_OUTPUT+1, "Out #2");
		configOutput(OUT_OUTPUT+2, "Out #3");
		configOutput(OUT_OUTPUT+3, "Out #4");
		configOutput(OUT_OUTPUT+4, "Out #5");
		configOutput(OUT_OUTPUT+5, "Out #6");
		configOutput(OUT_OUTPUT+6, "Out #7");
		configOutput(OUT_OUTPUT+7, "Out #8");

	}

	void onReset(const ResetEvent &e) override {
		outMode = true;
		voltDefaultOption = 0;
		for (int i = 0; i < 8; i++) {
			engine::ParamQuantity* pq = getParamQuantity(VOLT_PARAMS+i);
			pq->defaultValue = voltDefaults[voltDefaultOption];
		}
		Module::onReset(e);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "OutMode", json_boolean(outMode));
		json_object_set_new(rootJ, "VoltDefaultOption", json_integer(voltDefaultOption));
		json_object_set_new(rootJ, "Attenuator", json_boolean(attenuator));

		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* outModeJ = json_object_get(rootJ, "OutMode");
		if (outModeJ)
			outMode = json_boolean_value(outModeJ);
		json_t* voltDefaultOptionJ = json_object_get(rootJ, "VoltDefaultOption");
		if (voltDefaultOptionJ)
			voltDefaultOption = json_integer_value(voltDefaultOptionJ);
		json_t* attenuatorJ = json_object_get(rootJ, "Attenuator");
		if (attenuatorJ)
			attenuator = json_boolean_value(attenuatorJ);
	}

    struct Add8modeSwitch : SickoSwitch_Three_Horiz
    {
        Adder8* adder;
    };

    struct Add8AddSubSwitch : SickoSwitch_CKSS_Horiz
    {
        Adder8* adder;
    };

	void process(const ProcessArgs& args) override {

		if (params[MODE_SWITCH].getValue() < 0.f) {	// Subtract/Off
			if (params[ADDSUB_SWITCH].getValue() > 0.f)
				params[ADDSUB_SWITCH].setValue(-1.f);
		} else if (params[MODE_SWITCH].getValue() > 0.f) {	// Off/Add
			if (params[ADDSUB_SWITCH].getValue() < 0.f)
				params[ADDSUB_SWITCH].setValue(0.f);
		}

		engine::ParamQuantity* pq = getParamQuantity(VOLT_PARAMS);
		pq->defaultValue = voltDefaults[voltDefaultOption];

		if (inputs[CV_INPUT].isConnected())
			if (!attenuator)
				cv[0] = inputs[CV_INPUT].getVoltage() * params[VOLT_PARAMS].getValue() * params[ADDSUB_SWITCH].getValue() / 10.f;
			else
				cv[0] = inputs[CV_INPUT].getVoltage() * (params[VOLT_PARAMS].getValue() + 10.f) * params[ADDSUB_SWITCH].getValue() / 20.f;
		else
			cv[0] = params[VOLT_PARAMS].getValue() * params[ADDSUB_SWITCH].getValue();

		if (cv[0] > 10.f)
			cv[0] = 10.f;
		else if (cv[0] < -10.f)
			cv[0] = -10.f;

		if (outputs[OUT_OUTPUT].isConnected()) {
			outputs[OUT_OUTPUT].setVoltage(cv[0]);
			if (outMode)
				cv[0] = 0.f;
		}

		for (int i = 1; i < 8; i++) {

			engine::ParamQuantity* pq = getParamQuantity(VOLT_PARAMS+i);
			pq->defaultValue = voltDefaults[voltDefaultOption];;

			if (params[MODE_SWITCH+i].getValue() < 0.f) {	// Subtract/Off
				if (params[ADDSUB_SWITCH+i].getValue() > 0.f)
					params[ADDSUB_SWITCH+i].setValue(-1.f);
			} else if (params[MODE_SWITCH+i].getValue() > 0.f) {	// Off/Add
				if (params[ADDSUB_SWITCH+i].getValue() < 0.f)
					params[ADDSUB_SWITCH+i].setValue(0.f);
			}

			if (inputs[CV_INPUT+i].isConnected())
				if (!attenuator)
					cv[i] = cv[i-1] + (inputs[CV_INPUT+i].getVoltage() * params[VOLT_PARAMS+i].getValue() * params[ADDSUB_SWITCH+i].getValue() / 10.f);
				else
					cv[i] = cv[i-1] + (inputs[CV_INPUT+i].getVoltage() * (params[VOLT_PARAMS+i].getValue() + 10.f) * params[ADDSUB_SWITCH+i].getValue() / 20.f);
			else
				cv[i] = cv[i-1] + (params[VOLT_PARAMS+i].getValue() * params[ADDSUB_SWITCH+i].getValue());

			if (cv[i] > 10.f)
				cv[i] = 10.f;
			else if (cv[i] < -10.f)
				cv[i] = -10.f;
			
			if (outputs[OUT_OUTPUT+i].isConnected()) {
				outputs[OUT_OUTPUT+i].setVoltage(cv[i]);
				if (outMode)
					cv[i] = 0.f;
			}			
		}	
	}		
};

struct VoltItem : MenuItem {
	Adder8* module;
	int volt;
	void onAction(const event::Action& e) override {
		module->voltDefaultOption = volt;
	}
};

struct Adder8Widget : ModuleWidget {
	Adder8Widget(Adder8* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Adder8.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float y = 13;
		const float ys = 23;

		for (int i=0; i<8; i++) {

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(6.1, ys+(i*y))), module, Adder8::CV_INPUT+i));
			
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(15.4, ys+(i*y))), module, Adder8::VOLT_PARAMS+i));

			addParam(createParamCentered<Adder8::Add8modeSwitch>(mm2px(Vec(25.9, ys+(i*y) - 0.5f)), module, Adder8::ADDSUB_SWITCH+i));

			addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(37.2, ys+(i*y))), module, Adder8::MODE_SWITCH+i));
			
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(49.7, ys+(i*y))), module, Adder8::OUT_OUTPUT+i));

		}
	}

	void appendContextMenu(Menu* menu) override {
		Adder8* module = dynamic_cast<Adder8*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Stop Adding on Out Cable", "", &module->outMode));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Attenuator knobs", "", &module->attenuator));

		menu->addChild(createSubmenuItem("Volt Knob Default", "", [=](Menu* menu) {
			std::string menuNames[3] = {"0V", "+1V", "+10V"};
			for (int i = 0; i < 3; i++) {
				VoltItem* voltItem = createMenuItem<VoltItem>(menuNames[i]);
				voltItem->rightText = CHECKMARK(module->voltDefaultOption == i);
				voltItem->module = module;
				voltItem->volt = i;
				menu->addChild(voltItem);
			}
		}));
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Reset All Knobs to Default", "", [=]() {
			for (int i = 0; i < 8; i++)
				module->params[module->VOLT_PARAMS+i].setValue(module->voltDefaults[module->voltDefaultOption]);
		}));
	}
};

Model* modelAdder8 = createModel<Adder8, Adder8Widget>("Adder8");