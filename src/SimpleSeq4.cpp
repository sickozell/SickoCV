#define FORWARD 0
#define REVERSE 1
#define POSITIVE_V 0
#define NEGATIVE_V 1

#include "plugin.hpp"

//using namespace std;

struct SimpleSeq4 : Module {
	enum ParamId {
		ENUMS(KNOB_PARAM, 4),
		PARAMS_LEN
	};
	enum InputId {
		TRIG_INPUT,
		RST_INPUT,
		REV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(KNOB_LIGHT, 4),
		LIGHTS_LEN
	};

	float trigValue = 0;
	float prevTrigValue = 0;

	float rstValue = 0;
	float prevRstValue = 0;

	bool direction = FORWARD;

	float out = 0;

	int step = 0;

	int range = 9;

	bool initStart = false;
	int recStep = 0;

	int revType = POSITIVE_V;

	SimpleSeq4() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(TRIG_INPUT, "Trig");
		configInput(RST_INPUT, "Reset");

		configParam(KNOB_PARAM+0, 0, 1.f, 0.5f, "Knob 1");
		configParam(KNOB_PARAM+1, 0, 1.f, 0.5f, "Knob 2");
		configParam(KNOB_PARAM+2, 0, 1.f, 0.5f, "Knob 3");
		configParam(KNOB_PARAM+3, 0, 1.f, 0.5f, "Knob 4");

		configInput(REV_INPUT, "Reverse");

		configOutput(OUT_OUTPUT, "Output");

	}

	void onReset(const ResetEvent &e) override {

		step = 0;

		lights[KNOB_LIGHT].setBrightness(1);
		lights[KNOB_LIGHT+1].setBrightness(0);
		lights[KNOB_LIGHT+2].setBrightness(0);
		lights[KNOB_LIGHT+3].setBrightness(0);

		Module::onReset(e);
	}


	json_t* dataToJson() override {
		if (initStart)
			recStep = 0;
		else
			recStep = step;

		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "range", json_integer(range));
		json_object_set_new(rootJ, "revType", json_integer(revType));
		json_object_set_new(rootJ, "step", json_integer(recStep));
		json_object_set_new(rootJ, "initStart", json_boolean(initStart));

		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {

		json_t* rangeJ = json_object_get(rootJ, "range");
		if (rangeJ) {
			range = json_integer_value(rangeJ);
			if (range < 0 || range > 9)
				range = 9;
		}

		json_t* revTypeJ = json_object_get(rootJ, "revType");
		if (revTypeJ) {
			revType = json_integer_value(revTypeJ);
			if (revType < 0 || revType > 1)
				revType = 0;
		}

		json_t* stepJ = json_object_get(rootJ, "step");
		if (stepJ) {
			step = json_integer_value(stepJ);
			if (step < 0 || step > 3)
				range = 0;
			lights[KNOB_LIGHT+step].setBrightness(1);

		} 
		
		json_t* initStartJ = json_object_get(rootJ, "initStart");
		if (initStartJ) {
			initStart = json_boolean_value(initStartJ);
			if (initStart)
				step = 0;
		}

	}
	
	void process(const ProcessArgs& args) override {

		rstValue = inputs[RST_INPUT].getVoltage();
		if (rstValue >= 1.f && prevRstValue < 1.f) {
			lights[KNOB_LIGHT+step].setBrightness(0);
			step = 0;
		}
		prevRstValue = rstValue;

		trigValue = inputs[TRIG_INPUT].getVoltage();
		if (trigValue >= 1.f && prevTrigValue < 1.f) {

			if (revType == POSITIVE_V) {
				if (inputs[REV_INPUT].getVoltage() < 1)
					direction = FORWARD;
				else
					direction = REVERSE;
			} else {
				if (inputs[REV_INPUT].getVoltage() < -1)
					direction = REVERSE;
				else
					direction = FORWARD;
			}

			lights[KNOB_LIGHT + step].setBrightness(0);
			if (direction == FORWARD) {
				step++;
				if (step > 3)
					step = 0;
			} else {
				step--;
				if (step < 0)
					step = 3;
			}
		}
		prevTrigValue = trigValue;

		out = params[KNOB_PARAM+step].getValue();

		switch (range) {
			//case 0:
			//break;

			case 1:
				out *= 2;
			break;

			case 2:
				out *= 3;
			break;

			case 3:
				out *= 5;
			break;

			case 4:
				out *= 10;
			break;

			case 5:
				out = (out * 2) - 1;
			break;

			case 6:
				out = (out * 4) - 2;
			break;

			case 7:
				out = (out * 6) - 3;
			break;

			case 8:
				out = (out * 10) - 5;
			break;

			case 9:
				out = (out * 20) - 10;
			break;
		}

		outputs[OUT_OUTPUT].setVoltage(out);
		
		lights[KNOB_LIGHT+step].setBrightness(1);

	}
};

struct SimpleSeq4Widget : ModuleWidget {
	SimpleSeq4Widget(SimpleSeq4* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SimpleSeq4.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float xCenter = 7.62;

		const float yTrig = 17.5;
		const float yRst = 31;
		const float yDir = 101;
		const float yOut = 117.5;

		const float yKnStart = 46;
		const float yKnShift = 13.5;

		const float xLight = 12;
		const float yLgStart = 40;

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yTrig)), module, SimpleSeq4::TRIG_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yRst)), module, SimpleSeq4::RST_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xCenter, yKnStart)), module, SimpleSeq4::KNOB_PARAM+0));
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xCenter, yKnStart + yKnShift)), module, SimpleSeq4::KNOB_PARAM+1));
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xCenter, yKnStart + (yKnShift * 2))), module, SimpleSeq4::KNOB_PARAM+2));
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xCenter, yKnStart + (yKnShift * 3))), module, SimpleSeq4::KNOB_PARAM+3));

		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xLight, yLgStart)), module, SimpleSeq4::KNOB_LIGHT+0));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xLight, yLgStart + yKnShift)), module, SimpleSeq4::KNOB_LIGHT+1));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xLight, yLgStart + (yKnShift * 2))), module, SimpleSeq4::KNOB_LIGHT+2));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xLight, yLgStart + (yKnShift * 3))), module, SimpleSeq4::KNOB_LIGHT+3));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yDir)), module, SimpleSeq4::REV_INPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xCenter, yOut)), module, SimpleSeq4::OUT_OUTPUT));

	}

	void appendContextMenu(Menu* menu) override {
		SimpleSeq4* module = dynamic_cast<SimpleSeq4*>(this->module);

		struct RangeItem : MenuItem {
			SimpleSeq4* module;
			int range;
			void onAction(const event::Action& e) override {
				module->range = range;
			}
		};

		menu->addChild(new MenuSeparator());
		std::string rangeNames[10] = {"0/1v", "0/2v", "0/3v", "0/5v", "0/10v", "-1/+1v", "-2/+2v", "-3/+3v", "-5/+5v", "-10/+10v"};
		menu->addChild(createSubmenuItem("Knobs Range", rangeNames[module->range], [=](Menu * menu) {
			for (int i = 0; i < 10; i++) {
				RangeItem* rangeItem = createMenuItem<RangeItem>(rangeNames[i]);
				rangeItem->rightText = CHECKMARK(module->range == i);
				rangeItem->module = module;
				rangeItem->range = i;
				menu->addChild(rangeItem);
			}
		}));

		struct RevTypeItem : MenuItem {
			SimpleSeq4* module;
			int revType;
			void onAction(const event::Action& e) override {
				module->revType = revType;
			}
		};

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Reverse Input Voltage"));
		std::string RevTypeNames[2] = {"Positive", "Negative"};
		for (int i = 0; i < 2; i++) {
			RevTypeItem* revTypeItem = createMenuItem<RevTypeItem>(RevTypeNames[i]);
			revTypeItem->rightText = CHECKMARK(module->revType == i);
			revTypeItem->module = module;
			revTypeItem->revType = i;
			menu->addChild(revTypeItem);
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}

};

Model* modelSimpleSeq4 = createModel<SimpleSeq4, SimpleSeq4Widget>("SimpleSeq4");