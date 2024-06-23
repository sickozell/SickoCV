#include "plugin.hpp"

struct Drummer : Module {
	enum ParamId {
		CHOKE_SWITCH,
		LIMIT_SWITCH,
		ENUMS(NOACCENT_PARAMS,2),
		ENUMS(ACCENT_PARAMS,2),
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(TRIG_INPUT,2),
		ENUMS(ACCENT_INPUT,2),
		ENUMS(IN_INPUT,2),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT,2),
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	bool chokeMode = false;
	bool limitMode = false;

	float trigValue[2] = {0.f,0.f};
	float prevTrigValue[2] = {0.f,0.f};
	bool trigState[2] = {false,false};
	bool choking[2] = {false, false};
	float sustain[2] = {1.f,1.f};
	
	float out[2] = {0.f,0.f};
	float outFinal = 0.f;
	
	float fadeCoeff = 1000 / APP->engine->getSampleRate(); // 1ms choke
	float currentFade[2] = {0.f,0.f};

	Drummer() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(CHOKE_SWITCH, 0.f, 1.f, 0.f, "Mode", {"Off", "On"});
		configSwitch(LIMIT_SWITCH, 0.f, 1.f, 0.f, "Limit", {"Off", "±5v"});
		configParam(ACCENT_PARAMS, 0.f, 2.f, 1.f, "Accent Level #1", "%", 0, 100);
		configParam(ACCENT_PARAMS+1, 0.f, 2.f, 1.f, "Accent Level #2", "%", 0, 100);
		configParam(NOACCENT_PARAMS, 0.f, 2.f, 1.f, "Standard Level #1", "%", 0, 100);
		configParam(NOACCENT_PARAMS+1, 0.f, 2.f, 1.f, "Standard Level #2", "%", 0, 100);
		configInput(TRIG_INPUT, "Trigger #1");
		configInput(TRIG_INPUT+1, "Trigger #2");
		configInput(ACCENT_INPUT, "Accent #1");
		configInput(ACCENT_INPUT+1, "Accent #2");
		configInput(IN_INPUT, "AUDIO #1");
		configInput(IN_INPUT+1, "AUDIO #2");
		configOutput(OUT_OUTPUT, "AUDIO #1");
		configOutput(OUT_OUTPUT+1, "AUDIO #2");
	}

	void processBypass(const ProcessArgs& args) override {
		outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage());
		outputs[OUT_OUTPUT+1].setVoltage(inputs[IN_INPUT+1].getVoltage());
	}
	
	void onSampleRateChange() override {
		fadeCoeff = 1000 / APP->engine->getSampleRate();	// 1ms choke
	}

	void process(const ProcessArgs& args) override {
		chokeMode = params[CHOKE_SWITCH].getValue();
		limitMode = params[LIMIT_SWITCH].getValue();

		switch (chokeMode) {
			case 0:
				trigValue[0] = inputs[TRIG_INPUT].getVoltage();
				if (trigValue[0] >= 1 && prevTrigValue[0] < 1) {
					if (inputs[ACCENT_INPUT].getVoltage() >= 1)
						sustain[0] = params[ACCENT_PARAMS].getValue();
					else
						sustain[0] = params[NOACCENT_PARAMS].getValue();
				}
				prevTrigValue[0] = trigValue[0];
				out[0] = inputs[IN_INPUT].getVoltage() * sustain[0];

				trigValue[1] = inputs[TRIG_INPUT+1].getVoltage();
				if (trigValue[1] >= 1 && prevTrigValue[1] < 1) {
					if (inputs[ACCENT_INPUT+1].getVoltage() >= 1)
						sustain[1] = params[ACCENT_PARAMS+1].getValue();
					else
						sustain[1] = params[NOACCENT_PARAMS+1].getValue();
				}
				prevTrigValue[1] = trigValue[1];
				out[1] = inputs[IN_INPUT+1].getVoltage() * sustain[1];
			break;

			case 1:
				trigValue[0] = inputs[TRIG_INPUT].getVoltage();
				if (trigValue[0] >= 1 && prevTrigValue[0] < 1) {
					choking[0] = true;
					trigState[0] = true;
					currentFade[0] = 1;

					if (inputs[ACCENT_INPUT].getVoltage() >= 1)
						sustain[0] = params[ACCENT_PARAMS].getValue();
					else
						sustain[0] = params[NOACCENT_PARAMS].getValue();
				}
				prevTrigValue[0] = trigValue[0];

				if (choking[1]) {
					currentFade[0] -= fadeCoeff;
					if (currentFade[0] < 0) {
						choking[1] = false;
						trigState[0] = false;
						currentFade[0] = 0;
					} else {
						if (trigState[0])
							out[0] = inputs[IN_INPUT].getVoltage() * currentFade[0] * sustain[0];							
						else
							out[0] = 0;
					}
				} else {
					if (trigState[0])
						out[0] = inputs[IN_INPUT].getVoltage() * sustain[0];
					else
						out[0] = 0;
				}

				trigValue[1] = inputs[TRIG_INPUT+1].getVoltage();
				if (trigValue[1] >= 1 && prevTrigValue[1] < 1 && !choking[0]) {
					choking[1] = true;
					trigState[1] = true;
					currentFade[1] = 1;

					if (inputs[ACCENT_INPUT+1].getVoltage() >= 1)
						sustain[1] = params[ACCENT_PARAMS+1].getValue();
					else
						sustain[1] = params[NOACCENT_PARAMS+1].getValue();
				}
				prevTrigValue[1] = trigValue[1];

				if (choking[0]) {
					currentFade[1] -= fadeCoeff;
					if (currentFade[1] < 0) {
						choking[0] = false;
						trigState[1] = false;
						currentFade[1] = 0;
					} else {
						if (trigState[1])
							out[1] = inputs[IN_INPUT+1].getVoltage() * currentFade[1] * sustain[1];							
						else
							out[1] = 0;
					}
				} else {
					if (trigState[1])
						out[1] = inputs[IN_INPUT+1].getVoltage() * sustain[1];
					else
						out[1] = 0;
				}
			break;
		}
		if (outputs[OUT_OUTPUT].isConnected() && outputs[OUT_OUTPUT+1].isConnected()) {
			if (limitMode) {
				if (out[0] > 5)
					out[0] = 5;
				else if (out[0] < -5)
					out[0] = -5;
				if (out[1] > 5)
					out[1] = 5;
				else if (out[1] < -5)
					out[1] = -5;
			}
			outputs[OUT_OUTPUT].setVoltage(out[0]);
			outputs[OUT_OUTPUT+1].setVoltage(out[1]);
		} else {
			outFinal = out[0] + out[1];
			if (limitMode) {
				if (outFinal > 5)
					outFinal = 5;
				else if (outFinal < -5)
					outFinal = -5;
			}
			if (outputs[OUT_OUTPUT].isConnected())
				outputs[OUT_OUTPUT].setVoltage(outFinal);
			else
				outputs[OUT_OUTPUT+1].setVoltage(outFinal);
		}
	}
};

struct DrummerWidget : ModuleWidget {
	DrummerWidget(Drummer* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Drummer.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));		

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(11.5, 17.8)), module, Drummer::TRIG_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(11.5, 31.3)), module, Drummer::ACCENT_INPUT));
		
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(25, 17.8)), module, Drummer::NOACCENT_PARAMS));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(25, 31.3)), module, Drummer::ACCENT_PARAMS));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(9.5, 50)), module, Drummer::IN_INPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(26.5, 50)), module, Drummer::OUT_OUTPUT));
		
		addParam(createParamCentered<CKSS>(mm2px(Vec(5.35, 67.45)), module, Drummer::CHOKE_SWITCH));
		addParam(createParamCentered<CKSS>(mm2px(Vec(22.25, 67.45)), module, Drummer::LIMIT_SWITCH));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(11.5, 84.4)), module, Drummer::TRIG_INPUT+1));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(11.5, 97.8)), module, Drummer::ACCENT_INPUT+1));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(25, 84.4)), module, Drummer::NOACCENT_PARAMS+1));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(25, 97.8)), module, Drummer::ACCENT_PARAMS+1));
		
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(9.5, 117)), module, Drummer::IN_INPUT+1));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(26.5, 117)), module, Drummer::OUT_OUTPUT+1));
	}
};

Model* modelDrummer = createModel<Drummer, DrummerWidget>("Drummer");