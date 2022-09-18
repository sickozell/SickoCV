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

	Calcs() {

		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(A_INPUT, "a");
		configInput(B_INPUT, "b");
		configInput(C_INPUT, "c");

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
		if (params[RANGE_SWITCH].getValue() == 0){
			if (checkedValue > 5) {
				checkedValue = 5;
			} else {
				if (checkedValue < -5) {
					checkedValue = -5;
				}
			}
		} else {
			if (checkedValue > 10) {
				checkedValue = 10;
			} else {
				if (checkedValue < 0) {
					checkedValue = 0;
				}
			}
		}
		return checkedValue;
	}

	void process(const ProcessArgs& args) override {
		if (inputs[A_INPUT].isConnected() && inputs[B_INPUT].isConnected()){
			outputs[BPLUSA_OUTPUT].setVoltage(checkRange(inputs[A_INPUT].getVoltage() + inputs[B_INPUT].getVoltage()));
			outputs[ATIMESB_OUTPUT].setVoltage(checkRange(inputs[A_INPUT].getVoltage() * inputs[B_INPUT].getVoltage()));
			outputs[BMINUSA_OUTPUT].setVoltage(checkRange(inputs[B_INPUT].getVoltage() - inputs[A_INPUT].getVoltage()));
			outputs[AMINUSB_OUTPUT].setVoltage(checkRange(inputs[A_INPUT].getVoltage() - inputs[B_INPUT].getVoltage()));
			if (inputs[B_INPUT].getVoltage() != 0) {
				outputs[ADIVB_OUTPUT].setVoltage(checkRange(inputs[A_INPUT].getVoltage() / inputs[B_INPUT].getVoltage()));
			}
			if (inputs[C_INPUT].getVoltage() != 0) {
				outputs[BDIVA_OUTPUT].setVoltage(checkRange(inputs[B_INPUT].getVoltage() / inputs[A_INPUT].getVoltage()));
			}
			outputs[AAVGB_OUTPUT].setVoltage(checkRange((inputs[A_INPUT].getVoltage() + inputs[B_INPUT].getVoltage())/2));
		} else {
			outputs[BPLUSA_OUTPUT].setVoltage(0);
			outputs[ATIMESB_OUTPUT].setVoltage(0);
			outputs[BMINUSA_OUTPUT].setVoltage(0);
			outputs[AMINUSB_OUTPUT].setVoltage(0);
			outputs[ADIVB_OUTPUT].setVoltage(0);
			outputs[BDIVA_OUTPUT].setVoltage(0);
			outputs[AAVGB_OUTPUT].setVoltage(0);
		}
		if (inputs[B_INPUT].isConnected() && inputs[C_INPUT].isConnected()){
			outputs[CPLUSB_OUTPUT].setVoltage(checkRange(inputs[B_INPUT].getVoltage() + inputs[C_INPUT].getVoltage()));
			outputs[BTIMESC_OUTPUT].setVoltage(checkRange(inputs[B_INPUT].getVoltage() * inputs[C_INPUT].getVoltage()));
			outputs[BMINUSC_OUTPUT].setVoltage(checkRange(inputs[B_INPUT].getVoltage() - inputs[C_INPUT].getVoltage()));
			outputs[CMINUSB_OUTPUT].setVoltage(checkRange(inputs[C_INPUT].getVoltage() - inputs[B_INPUT].getVoltage()));
			if (inputs[C_INPUT].getVoltage() != 0) {
				outputs[BDIVC_OUTPUT].setVoltage(checkRange(inputs[B_INPUT].getVoltage() / inputs[C_INPUT].getVoltage()));
			}
			if (inputs[B_INPUT].getVoltage() != 0) {
				outputs[CDIVB_OUTPUT].setVoltage(checkRange(inputs[C_INPUT].getVoltage() / inputs[B_INPUT].getVoltage()));
			}
			outputs[BAVGC_OUTPUT].setVoltage(checkRange((inputs[B_INPUT].getVoltage() + inputs[C_INPUT].getVoltage())/2));	
		} else {
			outputs[CPLUSB_OUTPUT].setVoltage(0);
			outputs[BTIMESC_OUTPUT].setVoltage(0);
			outputs[BMINUSC_OUTPUT].setVoltage(0);
			outputs[CMINUSB_OUTPUT].setVoltage(0);
			outputs[BDIVC_OUTPUT].setVoltage(0);
			outputs[CDIVB_OUTPUT].setVoltage(0);
			outputs[BAVGC_OUTPUT].setVoltage(0);	
		}

		if (inputs[A_INPUT].isConnected() && inputs[C_INPUT].isConnected()){
			outputs[CPLUSA_OUTPUT].setVoltage(checkRange(inputs[A_INPUT].getVoltage() + inputs[C_INPUT].getVoltage()));
			outputs[ATIMESC_OUTPUT].setVoltage(checkRange(inputs[A_INPUT].getVoltage() * inputs[C_INPUT].getVoltage()));
			outputs[AMINUSC_OUTPUT].setVoltage(checkRange(inputs[A_INPUT].getVoltage() - inputs[C_INPUT].getVoltage()));
			outputs[CMINUSA_OUTPUT].setVoltage(checkRange(inputs[C_INPUT].getVoltage() - inputs[A_INPUT].getVoltage()));
			if (inputs[A_INPUT].getVoltage() != 0) {
				outputs[CDIVA_OUTPUT].setVoltage(checkRange(inputs[C_INPUT].getVoltage() / inputs[A_INPUT].getVoltage()));
			}
			if (inputs[C_INPUT].getVoltage() != 0) {
				outputs[ADIVC_OUTPUT].setVoltage(checkRange(inputs[A_INPUT].getVoltage() / inputs[C_INPUT].getVoltage()));
			}
			outputs[AAVGC_OUTPUT].setVoltage(checkRange((inputs[A_INPUT].getVoltage() + inputs[C_INPUT].getVoltage())/2));
		} else {
			outputs[CPLUSA_OUTPUT].setVoltage(0);
			outputs[ATIMESC_OUTPUT].setVoltage(0);
			outputs[AMINUSC_OUTPUT].setVoltage(0);
			outputs[CMINUSA_OUTPUT].setVoltage(0);
			outputs[CDIVA_OUTPUT].setVoltage(0);
			outputs[ADIVC_OUTPUT].setVoltage(0);
			outputs[AAVGC_OUTPUT].setVoltage(0);
		}

		if (inputs[A_INPUT].isConnected() && inputs[B_INPUT].isConnected() && inputs[C_INPUT].isConnected()){
			outputs[AVGABC_OUTPUT].setVoltage(checkRange((inputs[A_INPUT].getVoltage() + inputs[B_INPUT].getVoltage() + inputs[C_INPUT].getVoltage())/3));
		} else {
			outputs[AVGABC_OUTPUT].setVoltage(0);
		}
	}
};


struct CalcsWidget : ModuleWidget {
	CalcsWidget(Calcs* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Calcs.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float xa = 4.4;
		float xb = 12.6;
		float xc = 20.7;

		float yStart = 12;
		float yIncrement = 8;

		addParam(createParamCentered<CKSS>(mm2px(Vec(27.5, 11)), module, Calcs::RANGE_SWITCH));
		
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xa, yStart)), module, Calcs::A_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xb, yStart)), module, Calcs::B_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc, yStart)), module, Calcs::C_INPUT));

		xa = 8.2;
		xb = 16.3;
		xc = 24.4;
		yStart = 23.9;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xb, yStart)), module, Calcs::BPLUSA_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xc, yStart)), module, Calcs::CPLUSA_OUTPUT));
		yStart += yIncrement;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xc, yStart)), module, Calcs::CPLUSB_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xa, yStart)), module, Calcs::ATIMESB_OUTPUT));
		yStart += yIncrement;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xa, yStart)), module, Calcs::ATIMESC_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xb, yStart)), module, Calcs::BTIMESC_OUTPUT));

		yStart = 51.3;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xb, yStart)), module, Calcs::BMINUSA_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xc, yStart)), module, Calcs::CMINUSA_OUTPUT));
		yStart += yIncrement;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xa, yStart)), module, Calcs::AMINUSB_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xc, yStart)), module, Calcs::CMINUSB_OUTPUT));
		yStart += yIncrement;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xa, yStart)), module, Calcs::AMINUSC_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xb, yStart)), module, Calcs::BMINUSC_OUTPUT));

		yStart = 79.1;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xb, yStart)), module, Calcs::BDIVA_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xc, yStart)), module, Calcs::CDIVA_OUTPUT));
		yStart += yIncrement;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xa, yStart)), module, Calcs::ADIVB_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xc, yStart)), module, Calcs::CDIVB_OUTPUT));
		yStart += yIncrement;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xa, yStart)), module, Calcs::ADIVC_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xb, yStart)), module, Calcs::BDIVC_OUTPUT));
		
		xa = 6;
		xb = 15.24;
		xc = 24.5;
		yStart = 109.8;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xa, yStart)), module, Calcs::AAVGB_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xb, yStart)), module, Calcs::BAVGC_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xc, yStart)), module, Calcs::AAVGC_OUTPUT));
		xa = 6;
		yStart = 119;
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xa, yStart)), module, Calcs::AVGABC_OUTPUT));
	}
};


Model* modelCalcs = createModel<Calcs, CalcsWidget>("Calcs");