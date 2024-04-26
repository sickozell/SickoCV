#include "plugin.hpp"

struct SickoCrosser4 : Module {
	int inSources[4] = {0, 0, 0, 0};
	int link[4] = {0, 0, 0, 0};
	float xFade[4] = {0.f, 0.f, 0.f, 0.f};
	float out = 0.f;

	float a = 0.f;
	float b = 0.f;
	float c = 0.f;
	float d = 0.f;

	enum ParamId {
		ENUMS(IN_SW_SWITCH, 4),
		ENUMS(XFD_PARAM, 4),
		ENUMS(LINK_PARAM, 3),
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(IN_A_INPUT, 4),
		ENUMS(IN_B_INPUT, 4),
		ENUMS(IN_C_INPUT, 4),
		ENUMS(IN_D_INPUT, 4),
		ENUMS(MOD_XFD_INPUT, 4),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT, 4),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(LINK_LIGHT, 3),
		LIGHTS_LEN
	};

	SickoCrosser4() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configInput(IN_A_INPUT, "In A #1");
		configInput(IN_A_INPUT+1, "In A #2");
		configInput(IN_A_INPUT+2, "In A #3");
		configInput(IN_A_INPUT+3, "In A #4");

		configInput(IN_B_INPUT, "In B #1");
		configInput(IN_B_INPUT+1, "In B #2");
		configInput(IN_B_INPUT+2, "In B #3");
		configInput(IN_B_INPUT+3, "In B #4");

		configInput(IN_C_INPUT, "In C #1");
		configInput(IN_C_INPUT+1, "In C #2");
		configInput(IN_C_INPUT+2, "In C #3");
		configInput(IN_C_INPUT+3, "In C #4");

		configInput(IN_D_INPUT, "In D #1");
		configInput(IN_D_INPUT+1, "In D #2");
		configInput(IN_D_INPUT+2, "In D #3");
		configInput(IN_D_INPUT+3, "In D #4");		

		configSwitch(IN_SW_SWITCH, 0.f, 2.f, 0.f, "Inputs", {"2", "3", "4"});
		configSwitch(IN_SW_SWITCH+1, 0.f, 2.f, 0.f, "Inputs", {"2", "3", "4"});
		configSwitch(IN_SW_SWITCH+2, 0.f, 2.f, 0.f, "Inputs", {"2", "3", "4"});
		configSwitch(IN_SW_SWITCH+3, 0.f, 2.f, 0.f, "Inputs", {"2", "3", "4"});

		configSwitch(LINK_PARAM, 0.f, 1.f, 0.f, "Link", {"Off", "Linked"});
		configSwitch(LINK_PARAM+1, 0.f, 1.f, 0.f, "Link", {"Off", "Linked"});
		configSwitch(LINK_PARAM+2, 0.f, 1.f, 0.f, "Link", {"Off", "Linked"});

		configParam(XFD_PARAM, 0.f,1.f, 0.f, "xFade #1", "%", 0, 100);
		configParam(XFD_PARAM+1, 0.f,1.f, 0.f, "xFade #2", "%", 0, 100);
		configParam(XFD_PARAM+2, 0.f,1.f, 0.f, "xFade #3", "%", 0, 100);
		configParam(XFD_PARAM+3, 0.f,1.f, 0.f, "xFade #4", "%", 0, 100);

		configInput(MOD_XFD_INPUT, "xFade Mod #1");
		configInput(MOD_XFD_INPUT+1, "xFade Mod #2");
		configInput(MOD_XFD_INPUT+2, "xFade Mod #3");
		configInput(MOD_XFD_INPUT+3, "xFade Mod #4");

		configOutput(OUT_OUTPUT, "Out #1");
		configOutput(OUT_OUTPUT+1, "Out #2");
		configOutput(OUT_OUTPUT+2, "Out #3");
		configOutput(OUT_OUTPUT+3, "Out #4");


	}

	void process(const ProcessArgs& args) override {

		for (int i = 0; i < 4; i++) {

			if (i != 3) {
				link[i] = int(params[LINK_PARAM+i].getValue());
				lights[LINK_LIGHT+i].setBrightness(link[i]);
			}

			if (i == 0) {

				xFade[i] = params[XFD_PARAM+i].getValue() + (inputs[MOD_XFD_INPUT+i].getVoltage() * .1);

				if (xFade[i] < 0.f)
					xFade[i] = 0.f;
				else if (xFade[i] > 1.f)
					xFade[i] = 1.f;

			} else if (link[i-1]) {

				xFade[i] = xFade[i-1];

			} else {

				xFade[i] = params[XFD_PARAM+i].getValue() + (inputs[MOD_XFD_INPUT+i].getVoltage() * .1);

				if (xFade[i] < 0.f)
					xFade[i] = 0.f;
				else if (xFade[i] > 1.f)
					xFade[i] = 1.f;
			}
			
			inSources[i] = int(params[IN_SW_SWITCH+i].getValue());

			switch (inSources[i]) {
				case 0:	// 2 sources
					out = (inputs[IN_A_INPUT+i].getVoltage() * (1 - xFade[i])) +
							(inputs[IN_B_INPUT+i].getVoltage() * xFade[i]);

				break;

				case 1:	// 3 sources

					if (xFade[i] < 0.5) {
						a = inputs[IN_A_INPUT+i].getVoltage() * (1 - xFade[i] * 2.f);
						b = inputs[IN_B_INPUT+i].getVoltage() * (xFade[i] * 2.f);
						c = 0.f;
					} else {
						a = 0.f;
						b = inputs[IN_B_INPUT+i].getVoltage() * (1 - ((xFade[i] - 0.5) * 2.f));
						c = inputs[IN_C_INPUT+i].getVoltage() * ((xFade[i] - 0.5f) * 2.f);
					}

					out = a + b + c;

				break;

				case 2:	// 4 sources

					if (xFade[i] < 0.3333333f) {
						a = inputs[IN_A_INPUT+i].getVoltage() * (1 - xFade[i] * 3.f) ;
						b = inputs[IN_B_INPUT+i].getVoltage() * (xFade[i] * 3.f) ;
						c = 0.f;
						d = 0.f;
					} else if (xFade[i] > 0.6666667f) {
						a = 0.f;
						b = 0.f;
						c = inputs[IN_C_INPUT+i].getVoltage() * (1 - ((xFade[i] - 0.6666667f) * 3.f));
						d = inputs[IN_D_INPUT+i].getVoltage() * ((xFade[i] - 0.6666667f) * 3.f);
					} else {
						a = 0.f;
						b = inputs[IN_B_INPUT+i].getVoltage() * (1 - ((xFade[i] - 0.3333333f) * 3.f));
						c = inputs[IN_C_INPUT+i].getVoltage() * ((xFade[i] - 0.3333333f) * 3.f);
						d = 0.f;
					}

					out = a + b + c + d;

				break;
			}

			if (out > 10.f)
				out = 10.f;
			else if (out < -10.f)
				out = -10.f;

			outputs[OUT_OUTPUT+i].setVoltage(out);

		}
	}		
};

struct SickoCrosser4Widget : ModuleWidget {
	SickoCrosser4Widget(SickoCrosser4* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SickoCrosser4.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float x = 16.2;
		const float xs = 8.75;

		const float xLink = 16.8;

		const float ySw = 19.3;
		const float yIn1 = 33;
		const float yIn2 = 45;
		const float yIn3 = 57;
		const float yIn4 = 69;
		const float yKn = 86;
		const float yMod = 99;
		const float yOut = 116;
		const float yLink = 94.4;

		for (int i = 0; i < 4; i++) {
			addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(xs+(x*i), ySw)), module, SickoCrosser4::IN_SW_SWITCH+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs+(x*i), yIn1)), module, SickoCrosser4::IN_A_INPUT+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs+(x*i), yIn2)), module, SickoCrosser4::IN_B_INPUT+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs+(x*i), yIn3)), module, SickoCrosser4::IN_C_INPUT+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs+(x*i), yIn4)), module, SickoCrosser4::IN_D_INPUT+i));
			
			addParam(createParamCentered<SickoKnob>(mm2px(Vec(xs+(x*i), yKn)), module, SickoCrosser4::XFD_PARAM+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs+(x*i), yMod)), module, SickoCrosser4::MOD_XFD_INPUT+i));

			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xs+(x*i), yOut)), module, SickoCrosser4::OUT_OUTPUT+i));
		
			if (i != 3)
				addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xLink+(x*i), yLink)), module, SickoCrosser4::LINK_PARAM+i, SickoCrosser4::LINK_LIGHT+i));

		}
	}
};

Model* modelSickoCrosser4 = createModel<SickoCrosser4, SickoCrosser4Widget>("SickoCrosser4");