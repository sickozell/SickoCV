#define UNLIMITED 0
#define LIM_0_10 1
#define LIM_10_10 2
#define LIM_5_5 3

#include "plugin.hpp"

struct Calcs : Module {
	enum ParamId {
		RANGE_SWITCH,
		PARAMS_LEN
	};
	enum InputId {
		A_INPUT,
		B_INPUT,
		C_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		BPLUSA_OUTPUT,
		CPLUSA_OUTPUT,
		CPLUSB_OUTPUT,
		ATIMESB_OUTPUT,
		ATIMESC_OUTPUT,
		BTIMESC_OUTPUT,
		
		BMINUSA_OUTPUT,
		CMINUSA_OUTPUT,
		AMINUSB_OUTPUT,
		CMINUSB_OUTPUT,
		AMINUSC_OUTPUT,
		BMINUSC_OUTPUT,
		BDIVA_OUTPUT,
		CDIVA_OUTPUT,
		ADIVB_OUTPUT,
		CDIVB_OUTPUT,
		ADIVC_OUTPUT,
		BDIVC_OUTPUT,
		
		AAVGB_OUTPUT,
		BAVGC_OUTPUT,
		AAVGC_OUTPUT,		
		AVGABC_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	float aValue = 0;
	float bValue = 0;
	float cValue = 0;

	int outRange = 1;

	Calcs() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(A_INPUT, "a");
		configInput(B_INPUT, "b");
		configInput(C_INPUT, "c");

		// this is maintained to not brak patched for previous version
		configSwitch(RANGE_SWITCH, 0.f, 1.f, 1.f, "Input range", {"Bipolar", "Unipolar"});

		configOutput(BPLUSA_OUTPUT, "b+a");
		configOutput(CPLUSA_OUTPUT, "c+a");
		configOutput(CPLUSB_OUTPUT, "c+b");
		configOutput(ATIMESB_OUTPUT, "a*b");
		configOutput(ATIMESC_OUTPUT, "a*c");
		configOutput(BTIMESC_OUTPUT, "b*c");

		configOutput(BMINUSA_OUTPUT, "b-a");
		configOutput(CMINUSA_OUTPUT, "c-a");
		configOutput(AMINUSB_OUTPUT, "a-b");
		configOutput(CMINUSB_OUTPUT, "c-b");
		configOutput(AMINUSC_OUTPUT, "a-c");
		configOutput(BMINUSC_OUTPUT, "b-c");

		configOutput(BDIVA_OUTPUT, "b/a");
		configOutput(CDIVA_OUTPUT, "c/a");
		configOutput(ADIVB_OUTPUT, "a/b");
		configOutput(CDIVB_OUTPUT, "c/b");
		configOutput(ADIVC_OUTPUT, "a/c");
		configOutput(BDIVC_OUTPUT, "b/c");

		configOutput(AAVGB_OUTPUT, "ab average");
		configOutput(BAVGC_OUTPUT, "bc average");
		configOutput(AAVGC_OUTPUT, "ac average");		
		
		configOutput(AVGABC_OUTPUT, "abc average");
	}



	float checkRange(float checkedValue) {

		switch (outRange) {
			case UNLIMITED:
			break;

			case LIM_0_10:
				if (checkedValue > 10.f)
					checkedValue = 10.f;
				else if (checkedValue < 0.f)
					checkedValue = 0.f;
			break;

			case LIM_10_10:
				if (checkedValue > 10.f)
					checkedValue = 10.f;
				else if (checkedValue < -10.f)
					checkedValue = -10.f;
			break;

			case LIM_5_5:
				if (checkedValue > 5.f)
					checkedValue = 5.f;
				else if (checkedValue < -5.f)
					checkedValue = -5.f;
			break;
		}
		return checkedValue;
	}

	void process(const ProcessArgs& args) override {

		aValue = inputs[A_INPUT].getVoltage();
		bValue = inputs[B_INPUT].getVoltage();
		cValue = inputs[C_INPUT].getVoltage();


		outputs[BPLUSA_OUTPUT].setVoltage(checkRange(aValue + bValue));
		outputs[ATIMESB_OUTPUT].setVoltage(checkRange(aValue * bValue));
		outputs[BMINUSA_OUTPUT].setVoltage(checkRange(bValue - aValue));
		outputs[AMINUSB_OUTPUT].setVoltage(checkRange(aValue - bValue));
		
		if (bValue != 0.f)
			outputs[ADIVB_OUTPUT].setVoltage(checkRange(aValue / bValue));
		else
			outputs[ADIVB_OUTPUT].setVoltage(0.f);

		if (cValue != 0.f)
			outputs[BDIVA_OUTPUT].setVoltage(checkRange(bValue / aValue));
		else
			outputs[BDIVA_OUTPUT].setVoltage(0.f);

		outputs[AAVGB_OUTPUT].setVoltage(checkRange((aValue + bValue)/2));		

		// ---------------------------

		outputs[CPLUSB_OUTPUT].setVoltage(checkRange(bValue + cValue));
		outputs[BTIMESC_OUTPUT].setVoltage(checkRange(bValue * cValue));
		outputs[BMINUSC_OUTPUT].setVoltage(checkRange(bValue - cValue));
		outputs[CMINUSB_OUTPUT].setVoltage(checkRange(cValue - bValue));

		if (cValue != 0.f)
			outputs[BDIVC_OUTPUT].setVoltage(checkRange(bValue / cValue));
		else
			outputs[BDIVC_OUTPUT].setVoltage(0.f);

		if (bValue != 0.f)
			outputs[CDIVB_OUTPUT].setVoltage(checkRange(cValue / bValue));

		outputs[BAVGC_OUTPUT].setVoltage(checkRange((bValue + cValue)/2));	

		// --------------------------

		outputs[CPLUSA_OUTPUT].setVoltage(checkRange(aValue + cValue));
		outputs[ATIMESC_OUTPUT].setVoltage(checkRange(aValue * cValue));
		outputs[AMINUSC_OUTPUT].setVoltage(checkRange(aValue - cValue));
		outputs[CMINUSA_OUTPUT].setVoltage(checkRange(cValue - aValue));

		if (aValue != 0.f)
			outputs[CDIVA_OUTPUT].setVoltage(checkRange(cValue / aValue));

		if (cValue != 0.f)
			outputs[ADIVC_OUTPUT].setVoltage(checkRange(aValue / cValue));

		outputs[AAVGC_OUTPUT].setVoltage(checkRange((aValue + cValue)/2));

		// --------------------------

		outputs[AVGABC_OUTPUT].setVoltage(checkRange((aValue + bValue + cValue)/3));
	}
};

struct CalcsWidget : ModuleWidget {
	CalcsWidget(Calcs* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Calcs.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float xa = 5.7f;
		float xb = 15.2f;
		float xc = 24.9f;

		float yStart = 12.3f;
		float yIncrement = 8.f;

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xa, yStart)), module, Calcs::A_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xb, yStart)), module, Calcs::B_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xc, yStart)), module, Calcs::C_INPUT));

		xa = 8.2f;
		xb = 16.3f;
		xc = 24.4f;
		yStart = 23.9f;
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xb, yStart)), module, Calcs::BPLUSA_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xc, yStart)), module, Calcs::CPLUSA_OUTPUT));
		yStart += yIncrement;
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xc, yStart)), module, Calcs::CPLUSB_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xa, yStart)), module, Calcs::ATIMESB_OUTPUT));
		yStart += yIncrement;
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xa, yStart)), module, Calcs::ATIMESC_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xb, yStart)), module, Calcs::BTIMESC_OUTPUT));

		yStart = 51.3f;
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xb, yStart)), module, Calcs::BMINUSA_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xc, yStart)), module, Calcs::CMINUSA_OUTPUT));
		yStart += yIncrement;
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xa, yStart)), module, Calcs::AMINUSB_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xc, yStart)), module, Calcs::CMINUSB_OUTPUT));
		yStart += yIncrement;
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xa, yStart)), module, Calcs::AMINUSC_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xb, yStart)), module, Calcs::BMINUSC_OUTPUT));

		yStart = 79.1f;
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xb, yStart)), module, Calcs::BDIVA_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xc, yStart)), module, Calcs::CDIVA_OUTPUT));
		yStart += yIncrement;
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xa, yStart)), module, Calcs::ADIVB_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xc, yStart)), module, Calcs::CDIVB_OUTPUT));
		yStart += yIncrement;
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xa, yStart)), module, Calcs::ADIVC_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xb, yStart)), module, Calcs::BDIVC_OUTPUT));
		
		xa = 6.f;
		xb = 15.24f;
		xc = 24.5f;
		yStart = 109.8f;
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xa, yStart)), module, Calcs::AAVGB_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xb, yStart)), module, Calcs::BAVGC_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xc, yStart)), module, Calcs::AAVGC_OUTPUT));
		xa = 6.f;
		yStart = 119.f;
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xa, yStart)), module, Calcs::AVGABC_OUTPUT));
	}

	void appendContextMenu(Menu *menu) override {
	   	Calcs *module = dynamic_cast<Calcs*>(this->module);
			assert(module);
		
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Output Range"));
		struct RangeItem : MenuItem {
			Calcs* module;
			int outRange;
			void onAction(const event::Action& e) override {
				module->outRange = outRange;
			}
		};

		std::string outRangeNames[4] = {"Unlimited", "0-10v", "± 10v", "± 5v"};
		for (int i = 0; i < 4; i++) {
			RangeItem* rangeItem = createMenuItem<RangeItem>(outRangeNames[i]);
			rangeItem->rightText = CHECKMARK(module->outRange == i);
			rangeItem->module = module;
			rangeItem->outRange = i;
			menu->addChild(rangeItem);
		}
	}
};

Model* modelCalcs = createModel<Calcs, CalcsWidget>("Calcs");