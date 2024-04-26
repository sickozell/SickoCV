#include "plugin.hpp"

struct SickoCrosser : Module {
	int inSources = 0.f;
	float xFade = 0.f;
	float out = 0.f;

	float a = 0.f;
	float b = 0.f;
	float c = 0.f;
	float d = 0.f;

	enum ParamId {
		IN_SW_SWITCH,
		XFD_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		IN_A_INPUT,
		IN_B_INPUT,
		IN_C_INPUT,
		IN_D_INPUT,
		MOD_XFD_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	SickoCrosser() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configInput(IN_A_INPUT, "In 1");

		configInput(IN_B_INPUT, "In 2");

		configInput(IN_C_INPUT, "In 3");

		configInput(IN_D_INPUT, "In 4");

		configSwitch(IN_SW_SWITCH, 0.f, 2.f, 0.f, "Inputs", {"2", "3", "4"});

		configParam(XFD_PARAM, 0.f,1.f, 0.f, "xFade", "%", 0, 100);
	
		configInput(MOD_XFD_INPUT, "xFade Mod");

		configOutput(OUT_OUTPUT, "Out");

	}

	void process(const ProcessArgs& args) override {


		xFade = params[XFD_PARAM].getValue() + (inputs[MOD_XFD_INPUT].getVoltage() * .1);

		if (xFade < 0.f)
			xFade = 0.f;
		else if (xFade > 1.f)
			xFade = 1.f;

		inSources = int(params[IN_SW_SWITCH].getValue());

		switch (inSources) {
			case 0:	// 2 sources

				out = (inputs[IN_A_INPUT].getVoltage() * (1 - xFade)) +	(inputs[IN_B_INPUT].getVoltage() * xFade);

			break;

			case 1:	// 3 sources

				if (xFade < 0.5) {
					a = inputs[IN_A_INPUT].getVoltage() * (1 - xFade * 2.f);
					b = inputs[IN_B_INPUT].getVoltage() * (xFade * 2.f);
					c = 0.f;
				} else {
					a = 0.f;
					b = inputs[IN_B_INPUT].getVoltage() * (1 - ((xFade - 0.5) * 2.f));
					c = inputs[IN_C_INPUT].getVoltage() * ((xFade - 0.5f) * 2.f);
				}

				out = a + b + c;

			break;

			case 2:	// 4 sources

				if (xFade < 0.3333333f) {
					a = inputs[IN_A_INPUT].getVoltage() * (1 - xFade * 3.f) ;
					b = inputs[IN_B_INPUT].getVoltage() * (xFade * 3.f) ;
					c = 0.f;
					d = 0.f;
				} else if (xFade > 0.6666667f) {
					a = 0.f;
					b = 0.f;
					c = inputs[IN_C_INPUT].getVoltage() * (1 - ((xFade - 0.6666667f) * 3.f));
					d = inputs[IN_D_INPUT].getVoltage() * ((xFade - 0.6666667f) * 3.f);
				} else {
					a = 0.f;
					b = inputs[IN_B_INPUT].getVoltage() * (1 - ((xFade - 0.3333333f) * 3.f));
					c = inputs[IN_C_INPUT].getVoltage() * ((xFade - 0.3333333f) * 3.f);
					d = 0.f;
				}

				out = a + b + c + d;

			break;
		}

		if (out > 10.f)
			out = 10.f;
		else if (out < -10.f)
			out = -10.f;

		outputs[OUT_OUTPUT].setVoltage(out);

	}		
};

struct SickoCrosserWidget : ModuleWidget {
	SickoCrosserWidget(SickoCrosser* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SickoCrosser.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		//addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		//addChild(createWidget<ScrewBlack>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float xs = 10.25;

		const float ySw = 19.3;
		const float yIn1 = 33;
		const float yIn2 = 44.5;
		const float yIn3 = 56;
		const float yIn4 = 67.5;
		const float yKn = 86.2;
		const float yMod = 99;
		const float yOut = 116;

		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(xs, ySw)), module, SickoCrosser::IN_SW_SWITCH));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs, yIn1)), module, SickoCrosser::IN_A_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs, yIn2)), module, SickoCrosser::IN_B_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs, yIn3)), module, SickoCrosser::IN_C_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs, yIn4)), module, SickoCrosser::IN_D_INPUT));
		
		//addParam(createParamCentered<SickoKnob>(mm2px(Vec(xs, yKn)), module, SickoCrosser::XFD_PARAM));
		addParam(createParamCentered<SickoLargeKnob>(mm2px(Vec(xs, yKn)), module, SickoCrosser::XFD_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs, yMod)), module, SickoCrosser::MOD_XFD_INPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xs, yOut)), module, SickoCrosser::OUT_OUTPUT));
	
	}
};

Model* modelSickoCrosser = createModel<SickoCrosser, SickoCrosserWidget>("SickoCrosser");