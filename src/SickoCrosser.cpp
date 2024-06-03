#define TWO_SOURCES 0
#define THREE_SOURCES 1
#define FOUR_SOURCES 2

#include "plugin.hpp"

//using namespace std;

struct SickoCrosser : Module {
	int inSources = 0.f;
	int chan = 1;

	int seekInputNr;
	float xFadeGap;

	float xFadeCoeff;

	float xFade = 0.f;
	float outL = 0.f;
	float outR = 0.f;

	float aL = 0.f;
	float bL = 0.f;
	float aR = 0.f;
	float bR = 0.f;

	enum ParamId {
		IN_SW_SWITCH,
		POLY_PARAM,
		XFD_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		IN_A_L_INPUT,
		IN_B_L_INPUT,
		IN_C_L_INPUT,
		IN_D_L_INPUT,
		IN_A_R_INPUT,
		IN_B_R_INPUT,
		IN_C_R_INPUT,
		IN_D_R_INPUT,
		MOD_XFD_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_L_OUTPUT,
		OUT_R_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	SickoCrosser() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configInput(IN_A_L_INPUT, "In1 L");
		configInput(IN_A_R_INPUT, "In1 R");

		configInput(IN_B_L_INPUT, "In2 L");
		configInput(IN_B_R_INPUT, "In2 R");

		configInput(IN_C_L_INPUT, "In3 L");
		configInput(IN_C_R_INPUT, "In3 R");

		configInput(IN_D_L_INPUT, "In4 L");
		configInput(IN_D_R_INPUT, "In4 R");

		configSwitch(IN_SW_SWITCH, 0.f, 2.f, 0.f, "Inputs", {"2", "3", "4"});

		configParam(POLY_PARAM, 1.f,16.f, 1.f, "Poly Selector", "ch");
		paramQuantities[POLY_PARAM]->snapEnabled = true;

		configParam(XFD_PARAM, 0.f,1.f, 0.f, "xFade", "%", 0, 100);
	
		configInput(MOD_XFD_INPUT, "xFade Mod");

		configOutput(OUT_L_OUTPUT, "Left");
		configOutput(OUT_R_OUTPUT, "Right");

	}

	struct SickoCrosserSwitch : SickoSwitch_CKSS_Three_Vert
    {
        SickoCrosser* crosser;
    };

	void process(const ProcessArgs& args) override {

		xFade = params[XFD_PARAM].getValue() + (inputs[MOD_XFD_INPUT].getVoltage() * .1);

		if (xFade < 0.f)
			xFade = 0.f;
		else if (xFade > 1.f)
			xFade = 1.f;

		chan = int(params[POLY_PARAM].getValue());

		if (chan > 1 && inputs[IN_A_L_INPUT].isConnected()) {

			switch (chan) {
				case 2:

					xFadeCoeff = xFade;

					aL = inputs[IN_A_L_INPUT].getVoltage(0);
					bL = inputs[IN_A_L_INPUT].getVoltage(1);

					if (inputs[IN_A_R_INPUT].isConnected()) {
						aR = inputs[IN_A_R_INPUT].getVoltage(0);
						bR = inputs[IN_A_R_INPUT].getVoltage(1);
					} else {
						aR = aL;
						bR = bL;
					}

				break;

				default:
					
					if (xFade == 1) {

						xFadeCoeff = 1;

						aL = 0;
						aR = 0;

						bL = inputs[IN_A_L_INPUT].getVoltage(chan-1);
						
						if (inputs[IN_A_R_INPUT].isConnected())
							bR = inputs[IN_A_R_INPUT].getVoltage(chan-1);
						else
							bR = bL;

					} else {

						seekInputNr = (chan-1) * xFade;
						xFadeGap = 1 / (float(chan) - 1);

						xFadeCoeff = (xFade - (xFadeGap * seekInputNr)) * (chan-1);

						aL = inputs[IN_A_L_INPUT].getVoltage(int(seekInputNr));
						bL = inputs[IN_A_L_INPUT].getVoltage(int(seekInputNr+1));

						if (inputs[IN_A_R_INPUT].isConnected()) {

							aR = inputs[IN_A_R_INPUT].getVoltage(int(seekInputNr));
							bR = inputs[IN_A_R_INPUT].getVoltage(int(seekInputNr+1));
							
						} else {

							aR = aL;
							bR = bL;
							
						}
						
					}

				break;

			}

		} else {

			inSources = int(params[IN_SW_SWITCH].getValue());

			switch (inSources) {
				case TWO_SOURCES:	// 2 sources

					xFadeCoeff = xFade;

					aL = inputs[IN_A_L_INPUT].getVoltage();
					if (inputs[IN_A_R_INPUT].isConnected())
						aR = inputs[IN_A_R_INPUT].getVoltage();
					else
						aR = aL;

					bL = inputs[IN_B_L_INPUT].getVoltage();
					if (inputs[IN_B_R_INPUT].isConnected())
						bR = inputs[IN_B_R_INPUT].getVoltage();
					else
						bR = bL;

				break;

				case THREE_SOURCES:	// 3 sources

					if (xFade < 0.5) {

						xFadeCoeff = xFade * 2.f;

						aL = inputs[IN_A_L_INPUT].getVoltage();
						if (inputs[IN_A_R_INPUT].isConnected())
							aR = inputs[IN_A_R_INPUT].getVoltage();
						else
							aR = aL;

						bL = inputs[IN_B_L_INPUT].getVoltage();
						if (inputs[IN_B_R_INPUT].isConnected())
							bR = inputs[IN_B_R_INPUT].getVoltage();
						else
							bR = bL;

					} else {

						xFadeCoeff = (xFade - 0.5f) * 2.f; 

						aL = inputs[IN_B_L_INPUT].getVoltage();
						if (inputs[IN_B_R_INPUT].isConnected())
							aR = inputs[IN_B_R_INPUT].getVoltage();
						else
							aR = aL;

						bL = inputs[IN_C_L_INPUT].getVoltage();
						if (inputs[IN_C_R_INPUT].isConnected())
							bR = inputs[IN_C_R_INPUT].getVoltage();
						else
							bR = bL;

					}

				break;

				case FOUR_SOURCES:	// 4 sources

					if (xFade < 0.3333333f) {

						xFadeCoeff = xFade * 3.f;

						aL = inputs[IN_A_L_INPUT].getVoltage();
						if (inputs[IN_A_R_INPUT].isConnected())
							aR = inputs[IN_A_R_INPUT].getVoltage();
						else
							aR = aL;

						bL = inputs[IN_B_L_INPUT].getVoltage();
						if (inputs[IN_B_R_INPUT].isConnected())
							bR = inputs[IN_B_R_INPUT].getVoltage();
						else
							bR = bL;

					} else if (xFade > 0.6666667f) {

						xFadeCoeff = (xFade - 0.6666667f) * 3.f;

						aL = inputs[IN_C_L_INPUT].getVoltage();
						if (inputs[IN_C_R_INPUT].isConnected())
							aR = inputs[IN_C_R_INPUT].getVoltage();
						else
							aR = aL;

						bL = inputs[IN_D_L_INPUT].getVoltage();
						if (inputs[IN_D_R_INPUT].isConnected())
							bR = inputs[IN_D_R_INPUT].getVoltage();
						else
							bR = bL;

					} else {

						xFadeCoeff = (xFade - 0.3333333f) * 3.f;

						aL = inputs[IN_B_L_INPUT].getVoltage();
						if (inputs[IN_B_R_INPUT].isConnected())
							aR = inputs[IN_B_R_INPUT].getVoltage();
						else
							aR = aL;

						bL = inputs[IN_C_L_INPUT].getVoltage();
						if (inputs[IN_C_R_INPUT].isConnected())
							bR = inputs[IN_C_R_INPUT].getVoltage();
						else
							bR = bL;

					}

				break;
			}

		}

		outL = (aL * (1.f - xFadeCoeff)) + (bL * xFadeCoeff);
		outR = (aR * (1.f - xFadeCoeff)) + (bR * xFadeCoeff);

		if (outL > 10.f)
			outL = 10.f;
		else if (outL < -10.f)
			outL = -10.f;

		if (outR > 10.f)
			outR = 10.f;
		else if (outR < -10.f)
			outR = -10.f;

		outputs[OUT_L_OUTPUT].setVoltage(outL);
		outputs[OUT_R_OUTPUT].setVoltage(outR);

	}		
};

struct SickoCrosserWidget : ModuleWidget {
	SickoCrosserWidget(SickoCrosser* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SickoCrosser.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float xs = 10.25 + 2.4;
		const float xSw = 10.25 - 2.5 + 1;
		const float xPoly = 17.8;
		const float yPoly = 21.1;
		const float xL = 10.25+2.4-4.9;
		const float xR = 10.25+2.4+4.9;

		const float ySw = 19.3+1.5;
		const float yIn1 = 34;
		const float yIn2 = 45;
		const float yIn3 = 56;
		const float yIn4 = 67;
		const float yKn = 86.2;
		const float yMod = 99;
		const float yOut = 116;

		addParam(createParamCentered<SickoCrosser::SickoCrosserSwitch>(mm2px(Vec(xSw, ySw)), module, SickoCrosser::IN_SW_SWITCH));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xPoly, yPoly)), module, SickoCrosser::POLY_PARAM));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xL, yIn1)), module, SickoCrosser::IN_A_L_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xR, yIn1)), module, SickoCrosser::IN_A_R_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xL, yIn2)), module, SickoCrosser::IN_B_L_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xR, yIn2)), module, SickoCrosser::IN_B_R_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xL, yIn3)), module, SickoCrosser::IN_C_L_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xR, yIn3)), module, SickoCrosser::IN_C_R_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xL, yIn4)), module, SickoCrosser::IN_D_L_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xR, yIn4)), module, SickoCrosser::IN_D_R_INPUT));
		
		addParam(createParamCentered<SickoLargeKnob>(mm2px(Vec(xs, yKn)), module, SickoCrosser::XFD_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs, yMod)), module, SickoCrosser::MOD_XFD_INPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xL, yOut)), module, SickoCrosser::OUT_L_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xR, yOut)), module, SickoCrosser::OUT_R_OUTPUT));
	
	}
};

Model* modelSickoCrosser = createModel<SickoCrosser, SickoCrosserWidget>("SickoCrosser");