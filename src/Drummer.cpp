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
	
	float maxFadeSample[2] = {0.f,0.f};
	float currentFadeSample[2] = {0.f,0.f};
	float startFade[2] = {0.f,0.f};
	float lastFade[2] = {0.f,0.f};

	Drummer() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(CHOKE_SWITCH, 0.f, 1.f, 0.f, "Mode", {"Off", "On"});
		configSwitch(LIMIT_SWITCH, 0.f, 1.f, 0.f, "Limit", {"Off", "5v"});
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
	
	void process(const ProcessArgs& args) override {
		chokeMode = params[CHOKE_SWITCH].getValue();
		limitMode = params[LIMIT_SWITCH].getValue();

		switch (chokeMode) {
			case 0:
				for (int i=0;i<2;i++) {
					out[i] = 0;
					trigValue[i] = inputs[TRIG_INPUT+i].getVoltage();
					if (trigValue[i] >= 1 && prevTrigValue[i] < 1){
						if (inputs[ACCENT_INPUT+i].getVoltage() >= 1)
							sustain[i] = params[ACCENT_PARAMS+i].getValue();
						else
							sustain[i] = params[NOACCENT_PARAMS+i].getValue();
					}
					prevTrigValue[i] = trigValue[i];
					out[i] = inputs[IN_INPUT+i].getVoltage() * sustain[i];
					
					if (limitMode) {
						if (out[i] > 5)
							out[i] = 5;
						else if (out[i] < -5)
							out[i] = -5;
					}
				}
			break;

			case 1:
				for (int i=0;i<2;i++) {
					out[i] = 0;
					trigValue[i] = inputs[TRIG_INPUT+i].getVoltage();
					if (trigValue[i] >= 1 && prevTrigValue[i] < 1){
						choking[i] = true;
						trigState[i] = true;
						startFade[i] = 1;
						maxFadeSample[i] = args.sampleRate * 0.05f;
						currentFadeSample[i] = 0;

						if (inputs[ACCENT_INPUT+i].getVoltage() >= 1)
							sustain[i] = params[ACCENT_PARAMS+i].getValue();
						else
							sustain[i] = params[NOACCENT_PARAMS+i].getValue();
					}
					prevTrigValue[i] = trigValue[i];

					if (choking[!i]) {
						lastFade[i] = -(currentFadeSample[i] / maxFadeSample[i]) + startFade[i];
						if (lastFade[i] < 0) {
							choking[!i] = false;
							trigState[i] = false;
							currentFadeSample[i] = 0;
							startFade[i] = 0;
							lastFade[i] = 0;
						} else {
							out[i] = inputs[IN_INPUT+i].getVoltage() * lastFade[i] * sustain[i];
							if (limitMode) {
								if (out[i] > 5)
									out[i] = 5;
								else if (out[i] < -5)
									out[i] = -5;
							}
							if (!trigState[i])
								out[i] = 0;
						}
						currentFadeSample[i]++;
					} else {
						if (trigState[i]) {
							out[i] = inputs[IN_INPUT+i].getVoltage() * sustain[i];
							if (limitMode) {
								if (out[i] > 5)
									out[i] = 5;
								else if (out[i] < -5)
									out[i] = -5;
							}
							if (!trigState[i])
								out[i] = 0;
						}
					}
				}
			break;
		}
		if (outputs[OUT_OUTPUT].isConnected() && outputs[OUT_OUTPUT+1].isConnected()) {
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

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));		

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.5, 18.8)), module, Drummer::TRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.5, 31.9)), module, Drummer::ACCENT_INPUT));
		
		addParam(createParamCentered<Trimpot>(mm2px(Vec(25, 17.8)), module, Drummer::NOACCENT_PARAMS));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(25, 31.3)), module, Drummer::ACCENT_PARAMS));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.5, 50)), module, Drummer::IN_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(26.5, 50)), module, Drummer::OUT_OUTPUT));
		
		addParam(createParamCentered<CKSS>(mm2px(Vec(5.35, 67.45)), module, Drummer::CHOKE_SWITCH));
		addParam(createParamCentered<CKSS>(mm2px(Vec(22.25, 67.45)), module, Drummer::LIMIT_SWITCH));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.5, 85.4)), module, Drummer::TRIG_INPUT+1));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.5, 98.5)), module, Drummer::ACCENT_INPUT+1));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(25, 84.4)), module, Drummer::NOACCENT_PARAMS+1));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(25, 97.8)), module, Drummer::ACCENT_PARAMS+1));
		
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.5, 117)), module, Drummer::IN_INPUT+1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(26.5, 117)), module, Drummer::OUT_OUTPUT+1));
	}

};

Model* modelDrummer = createModel<Drummer, DrummerWidget>("Drummer");