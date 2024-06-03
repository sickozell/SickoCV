#define ONE_SOURCE 0
#define TWO_SOURCES 1
#define THREE_SOURCES 2
#define FOUR_SOURCES 3

#include "plugin.hpp"

struct SickoCrosser4 : Module {

	int inSources = 0;

	int chan;
	int seekInputNr;
	float xFadeGap;
	float xFadeCoeff;

	int link[4] = {0, 0, 0, 0};
	float xFade = 0.f;
	float out = 0.f;

	float a = 0.f;
	float b = 0.f;
	float c = 0.f;
	float d = 0.f;

	int linkMode = 0;
	int nextChan = 1;

	enum ParamId {
		ENUMS(IN_SW_SWITCH, 4),
		ENUMS(XFD_PARAM, 4),
		ENUMS(LINK_PARAM, 3),
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(IN_INPUT, 16),
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

		configInput(IN_INPUT, "In A #1");
		configInput(IN_INPUT+1, "In A #2");
		configInput(IN_INPUT+2, "In A #3");
		configInput(IN_INPUT+3, "In A #4");

		configInput(IN_INPUT+4, "In B #1");
		configInput(IN_INPUT+5, "In B #2");
		configInput(IN_INPUT+6, "In B #3");
		configInput(IN_INPUT+7, "In B #4");

		configInput(IN_INPUT+8, "In C #1");
		configInput(IN_INPUT+9, "In C #2");
		configInput(IN_INPUT+10, "In C #3");
		configInput(IN_INPUT+11, "In C #4");

		configInput(IN_INPUT+12, "In D #1");
		configInput(IN_INPUT+13, "In D #2");
		configInput(IN_INPUT+14, "In D #3");
		configInput(IN_INPUT+15, "In D #4");		

		configSwitch(IN_SW_SWITCH, 0.f, 3.f, 0.f, "Inputs", {"1", "2", "3", "4"});
		configSwitch(IN_SW_SWITCH+1, 0.f, 3.f, 0.f, "Inputs", {"1", "2", "3", "4"});
		configSwitch(IN_SW_SWITCH+2, 0.f, 3.f, 0.f, "Inputs", {"1", "2", "3", "4"});
		configSwitch(IN_SW_SWITCH+3, 0.f, 3.f, 0.f, "Inputs", {"1", "2", "3", "4"});

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

		json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "linkMode", json_integer(linkMode));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* linkModeJ = json_object_get(rootJ, "linkMode");
		if (linkModeJ)
			linkMode = json_integer_value(linkModeJ);

	}

	void process(const ProcessArgs& args) override {

		if (linkMode) {

			// ******************************************************************* LINKED XFD (for stereo)

			for (int i = 0; i < 4; i++) {

				if (i != 3) {
					link[i] = int(params[LINK_PARAM+i].getValue());
					lights[LINK_LIGHT+i].setBrightness(link[i]);
				}

				if (i == 0) {

					xFade = params[XFD_PARAM+i].getValue() + (inputs[MOD_XFD_INPUT+i].getVoltage() * .1);

					if (xFade < 0.f)
						xFade = 0.f;
					else if (xFade > 1.f)
						xFade = 1.f;

				} else if (link[i-1]) {

					// do nothing

				} else {

					xFade = params[XFD_PARAM+i].getValue() + (inputs[MOD_XFD_INPUT+i].getVoltage() * .1);

					if (xFade < 0.f)
						xFade = 0.f;
					else if (xFade > 1.f)
						xFade = 1.f;
				}
				
				inSources = int(params[IN_SW_SWITCH+i].getValue());

				switch (inSources) {

					case ONE_SOURCE:	// 1 source
						a = 0;
						b = (inputs[IN_INPUT+(i*4)].getVoltage() * xFade);
					break;

					case TWO_SOURCES:	// 2 sources
						a = (inputs[IN_INPUT+(i*4)].getVoltage() * (1 - xFade));
						b = (inputs[IN_INPUT+(i*4+1)].getVoltage() * xFade);
					break;

					case THREE_SOURCES:	// 3 sources

						if (xFade < 0.5) {
							a = inputs[IN_INPUT+(i*4)].getVoltage() * (1 - xFade * 2.f);
							b = inputs[IN_INPUT+(i*4+1)].getVoltage() * (xFade * 2.f);
						} else {
							a = inputs[IN_INPUT+(i*4+1)].getVoltage() * (1 - ((xFade - 0.5) * 2.f));
							b = inputs[IN_INPUT+(i*4+2)].getVoltage() * ((xFade - 0.5f) * 2.f);
						}

					break;

					case FOUR_SOURCES:	// 4 sources

						if (xFade < 0.3333333f) {
							a = inputs[IN_INPUT+(i*4)].getVoltage() * (1 - xFade * 3.f) ;
							b = inputs[IN_INPUT+(i*4+1)].getVoltage() * (xFade * 3.f) ;
						} else if (xFade > 0.6666667f) {
							a = inputs[IN_INPUT+(i*4+2)].getVoltage() * (1 - ((xFade - 0.6666667f) * 3.f));
							b = inputs[IN_INPUT+(i*4+3)].getVoltage() * ((xFade - 0.6666667f) * 3.f);
						} else {
							a = inputs[IN_INPUT+(i*4+1)].getVoltage() * (1 - ((xFade - 0.3333333f) * 3.f));
							b = inputs[IN_INPUT+(i*4+2)].getVoltage() * ((xFade - 0.3333333f) * 3.f);
						}

					break;
				}

				out = a + b;

				if (out > 10.f)
					out = 10.f;
				else if (out < -10.f)
					out = -10.f;

				outputs[OUT_OUTPUT+i].setVoltage(out);

			}
		} else {

			// ******************************************************************* LINKED INPUTS

			
			link[0] = int(params[LINK_PARAM].getValue());
			lights[LINK_LIGHT].setBrightness(link[0]);
			link[1] = int(params[LINK_PARAM+1].getValue());
			lights[LINK_LIGHT+1].setBrightness(link[1]);
			link[2] = int(params[LINK_PARAM+2].getValue());
			lights[LINK_LIGHT+2].setBrightness(link[2]);

			if (link[0]) {
				if (link[1]) {
					if (link[2]) {
						chan = 11 + int(params[IN_SW_SWITCH+3].getValue());
						nextChan = 4;
						outputs[OUT_OUTPUT+1].setVoltage(0);
						outputs[OUT_OUTPUT+2].setVoltage(0);
						outputs[OUT_OUTPUT+3].setVoltage(0);
					} else {
						chan = 7 + int(params[IN_SW_SWITCH+2].getValue());
						nextChan = 3;
						outputs[OUT_OUTPUT+1].setVoltage(0);
						outputs[OUT_OUTPUT+2].setVoltage(0);
					}
				} else {
					chan = 4 + int(params[IN_SW_SWITCH+1].getValue());
					nextChan = 2;
					outputs[OUT_OUTPUT+1].setVoltage(0);
				}
			} else {
				chan = int(params[IN_SW_SWITCH+0].getValue());
				nextChan = 1;
			}

			xFade = params[XFD_PARAM+0].getValue() + (inputs[MOD_XFD_INPUT+0].getVoltage() * .1);

			if (xFade < 0.f)
				xFade = 0.f;
			else if (xFade > 1.f)
				xFade = 1.f;
			
			switch (chan) {
				case ONE_SOURCE:	// 1 source
					xFadeCoeff = xFade;
					a = 0;
					b = inputs[IN_INPUT+0].getVoltage();
				break;
				case TWO_SOURCES:	// 2 sources
					xFadeCoeff = xFade;
					a = inputs[IN_INPUT+0].getVoltage();
					b = inputs[IN_INPUT+1].getVoltage();
				break;
				default:
					if (xFade == 1) {
						xFadeCoeff = 1;
						a = 0;
						b = inputs[IN_INPUT+chan].getVoltage();						
					} else {
						seekInputNr = int(chan * xFade);
						xFadeGap = 1 / float(chan);
						xFadeCoeff = (xFade - (xFadeGap * seekInputNr)) * chan;
						a = inputs[IN_INPUT+seekInputNr].getVoltage();
						b = inputs[IN_INPUT+(seekInputNr+1)].getVoltage();
					}
				break;
			}
			out = (a * (1.f - xFadeCoeff)) + (b * xFadeCoeff);
			if (out > 10.f)
				out = 10.f;
			else if (out < -10.f)
				out = -10.f;
			outputs[OUT_OUTPUT+0].setVoltage(out);

			// -----------------------------------------------------------------------

			if (nextChan == 1) {
				if (link[1]) {
					if (link[2]) {
						chan = 7 + int(params[IN_SW_SWITCH+3].getValue());
						nextChan = 4;
						outputs[OUT_OUTPUT+2].setVoltage(0);
						outputs[OUT_OUTPUT+3].setVoltage(0);
					} else {
						chan = 4 + int(params[IN_SW_SWITCH+2].getValue());
						nextChan = 3;
						outputs[OUT_OUTPUT+2].setVoltage(0);
					}
				} else {
					chan = int(params[IN_SW_SWITCH+1].getValue());
					nextChan = 2;
				}

				xFade = params[XFD_PARAM+1].getValue() + (inputs[MOD_XFD_INPUT+1].getVoltage() * .1);

				if (xFade < 0.f)
					xFade = 0.f;
				else if (xFade > 1.f)
					xFade = 1.f;

				switch (chan) {
					case ONE_SOURCE:	// 1 source
						xFadeCoeff = xFade;
						a = 0;
						b = inputs[IN_INPUT+4].getVoltage();
					break;
					case TWO_SOURCES:	// 2 sources
						xFadeCoeff = xFade;
						a = inputs[IN_INPUT+4].getVoltage();
						b = inputs[IN_INPUT+5].getVoltage();
					break;
					default:
						if (xFade == 1) {
							xFadeCoeff = 1;
							a = 0;
							b = inputs[IN_INPUT+(4+chan)].getVoltage();						
						} else {
							seekInputNr = int(chan * xFade);
							xFadeGap = 1 / float(chan);
							xFadeCoeff = (xFade - (xFadeGap * seekInputNr)) * chan;
							a = inputs[IN_INPUT+(4+seekInputNr)].getVoltage();
							b = inputs[IN_INPUT+(5+seekInputNr)].getVoltage();
						}
					break;
				}
				out = (a * (1.f - xFadeCoeff)) + (b * xFadeCoeff);
				if (out > 10.f)
					out = 10.f;
				else if (out < -10.f)
					out = -10.f;
				outputs[OUT_OUTPUT+1].setVoltage(out);
			}

			// -----------------------------------------------------------------------

			if (nextChan == 2) {

				if (link[2]) {
					chan = 4 + int(params[IN_SW_SWITCH+3].getValue());
					nextChan = 4;
					outputs[OUT_OUTPUT+3].setVoltage(0);
				} else {
					chan = int(params[IN_SW_SWITCH+2].getValue());
					nextChan = 3;
				}

				xFade = params[XFD_PARAM+2].getValue() + (inputs[MOD_XFD_INPUT+2].getVoltage() * .1);

				if (xFade < 0.f)
					xFade = 0.f;
				else if (xFade > 1.f)
					xFade = 1.f;

				switch (chan) {
					case ONE_SOURCE:	// 1 source
						xFadeCoeff = xFade;
						a = 0;
						b = inputs[IN_INPUT+8].getVoltage();
					break;
					case TWO_SOURCES:	// 2 sources
						xFadeCoeff = xFade;
						a = inputs[IN_INPUT+8].getVoltage();
						b = inputs[IN_INPUT+9].getVoltage();
					break;
					default:
						if (xFade == 1) {
							xFadeCoeff = 1;
							a = 0;
							b = inputs[IN_INPUT+(8+chan)].getVoltage();						
						} else {
							seekInputNr = int(chan * xFade);
							xFadeGap = 1 / float(chan);
							xFadeCoeff = (xFade - (xFadeGap * seekInputNr)) * chan;
							a = inputs[IN_INPUT+(8+seekInputNr)].getVoltage();
							b = inputs[IN_INPUT+(9+seekInputNr)].getVoltage();
						}
					break;
				}
				out = (a * (1.f - xFadeCoeff)) + (b * xFadeCoeff);
				if (out > 10.f)
					out = 10.f;
				else if (out < -10.f)
					out = -10.f;
				outputs[OUT_OUTPUT+2].setVoltage(out);
			}

			// -----------------------------------------------------------------------

			if (nextChan == 3) {

				chan = int(params[IN_SW_SWITCH+3].getValue());
				//nextChan = 4;

				xFade = params[XFD_PARAM+3].getValue() + (inputs[MOD_XFD_INPUT+3].getVoltage() * .1);

				if (xFade < 0.f)
					xFade = 0.f;
				else if (xFade > 1.f)
					xFade = 1.f;

				switch (chan) {
					case ONE_SOURCE:	// 1 source
						xFadeCoeff = xFade;
						a = 0;
						b = inputs[IN_INPUT+12].getVoltage();
					break;
					case TWO_SOURCES:	// 2 sources
						xFadeCoeff = xFade;
						a = inputs[IN_INPUT+12].getVoltage();
						b = inputs[IN_INPUT+13].getVoltage();
					break;
					default:
						if (xFade == 1) {
							xFadeCoeff = 1;
							a = 0;
							b = inputs[IN_INPUT+(12+chan)].getVoltage();						
						} else {
							seekInputNr = int(chan * xFade);
							xFadeGap = 1 / float(chan);
							xFadeCoeff = (xFade - (xFadeGap * seekInputNr)) * chan;
							a = inputs[IN_INPUT+(12+seekInputNr)].getVoltage();
							b = inputs[IN_INPUT+(13+seekInputNr)].getVoltage();
						}
					break;
				}
				out = (a * (1.f - xFadeCoeff)) + (b * xFadeCoeff);
				if (out > 10.f)
					out = 10.f;
				else if (out < -10.f)
					out = -10.f;
				outputs[OUT_OUTPUT+3].setVoltage(out);
			}

		}

	}		
};

struct SickoCrosser4Widget : ModuleWidget {
	SickoCrosser4Widget(SickoCrosser4* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SickoCrosser4.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	

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
			addParam(createParamCentered<SickoSwitch_CKSS_Four_Horiz>(mm2px(Vec(xs+(x*i), ySw)), module, SickoCrosser4::IN_SW_SWITCH+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs+(x*i), yIn1)), module, SickoCrosser4::IN_INPUT+(i*4)));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs+(x*i), yIn2)), module, SickoCrosser4::IN_INPUT+(i*4+1)));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs+(x*i), yIn3)), module, SickoCrosser4::IN_INPUT+(i*4+2)));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs+(x*i), yIn4)), module, SickoCrosser4::IN_INPUT+(i*4+3)));
			
			addParam(createParamCentered<SickoKnob>(mm2px(Vec(xs+(x*i), yKn)), module, SickoCrosser4::XFD_PARAM+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs+(x*i), yMod)), module, SickoCrosser4::MOD_XFD_INPUT+i));

			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xs+(x*i), yOut)), module, SickoCrosser4::OUT_OUTPUT+i));
		
			if (i != 3)
				addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xLink+(x*i), yLink)), module, SickoCrosser4::LINK_PARAM+i, SickoCrosser4::LINK_LIGHT+i));

		}
	}

	void appendContextMenu(Menu* menu) override {
		SickoCrosser4* module = dynamic_cast<SickoCrosser4*>(this->module);

		struct ModeItem : MenuItem {
			SickoCrosser4* module;
			int linkMode;
			void onAction(const event::Action& e) override {
				module->linkMode = linkMode;
			}
		};

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Link Mode"));
		std::string modeNames[2] = {"xFD + Inputs", "xFD only"};
		for (int i = 0; i < 2; i++) {
			ModeItem* modeItem = createMenuItem<ModeItem>(modeNames[i]);
			modeItem->rightText = CHECKMARK(module->linkMode == i);
			modeItem->module = module;
			modeItem->linkMode = i;
			menu->addChild(modeItem);
		}

	}
};

Model* modelSickoCrosser4 = createModel<SickoCrosser4, SickoCrosser4Widget>("SickoCrosser4");