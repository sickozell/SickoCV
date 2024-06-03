#include "plugin.hpp"

struct Drummer4 : Module {
	enum ParamId {
		ENUMS(CHOKE_PARAMS,3),
		ENUMS(LIMIT_PARAMS,4),
		ENUMS(NOACCENT_PARAMS,4),
		ENUMS(ACCENT_PARAMS,4),
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(TRIG_INPUT,4),
		ENUMS(ACCENT_INPUT,4),
		ENUMS(IN_INPUT,4),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT,4),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(CHOKE_LIGHT,3),
		ENUMS(LIMIT_LIGHT,4),
		LIGHTS_LEN
	};

	float trigValue[4] = {0.f, 0.f, 0.f, 0.f};
	float prevTrigValue[4] = {0.f, 0.f, 0.f, 0.f};
	bool trigState[4] = {false, false, false, false};

	bool choke[3] = {false, false, false};
	bool choking[3] = {false, false, false};
	int limit[4];

	float fadeCoeff = 1000 / APP->engine->getSampleRate(); // 1ms choke
	float currentFade[3] = {0.f,0.f,0.f};

	float sustain[4] = {1.f, 1.f, 1.f, 1.f};

	float out[4] = {0.f, 0.f, 0.f, 0.f};
	float outSum;

	Drummer4() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(LIMIT_PARAMS, 0.f, 1.f, 0.f, "Limit #1", {"Off", "±5v"});
		configSwitch(LIMIT_PARAMS+1, 0.f, 1.f, 0.f, "Limit #2", {"Off", "±5v"});
		configSwitch(LIMIT_PARAMS+2, 0.f, 1.f, 0.f, "Limit #3", {"Off", "±5v"});
		configSwitch(LIMIT_PARAMS+3, 0.f, 1.f, 0.f, "Limit #4", {"Off", "±5v"});
		configSwitch(CHOKE_PARAMS, 0.f, 1.f, 0.f, "Choke #1", {"Off", "On"});
		configSwitch(CHOKE_PARAMS+1, 0.f, 1.f, 0.f, "Choke #2", {"Off", "On"});
		configSwitch(CHOKE_PARAMS+2, 0.f, 1.f, 0.f, "Choke #3", {"Off", "On"});
		configParam(ACCENT_PARAMS, 0.f, 2.f, 1.f, "Accent Level #1", "%", 0, 100);
		configParam(ACCENT_PARAMS+1, 0.f, 2.f, 1.f, "Accent Level #2", "%", 0, 100);
		configParam(ACCENT_PARAMS+2, 0.f, 2.f, 1.f, "Accent Level #3", "%", 0, 100);
		configParam(ACCENT_PARAMS+3, 0.f, 2.f, 1.f, "Accent Level #4", "%", 0, 100);
		configParam(NOACCENT_PARAMS, 0.f, 2.f, 1.f, "Standard Level #1", "%", 0, 100);
		configParam(NOACCENT_PARAMS+1, 0.f, 2.f, 1.f, "Standard Level #2", "%", 0, 100);
		configParam(NOACCENT_PARAMS+2, 0.f, 2.f, 1.f, "Standard Level #3", "%", 0, 100);
		configParam(NOACCENT_PARAMS+3, 0.f, 2.f, 1.f, "Standard Level #4", "%", 0, 100);
		configInput(TRIG_INPUT, "Trigger #1");
		configInput(TRIG_INPUT+1, "Trigger #2");
		configInput(TRIG_INPUT+2, "Trigger #3");
		configInput(TRIG_INPUT+3, "Trigger #4");
		configInput(ACCENT_INPUT, "Accent #1");
		configInput(ACCENT_INPUT+1, "Accent #2");
		configInput(ACCENT_INPUT+2, "Accent #3");
		configInput(ACCENT_INPUT+3, "Accent #4");
		configInput(IN_INPUT, "AUDIO #1");
		configInput(IN_INPUT+1, "AUDIO #2");
		configInput(IN_INPUT+2, "AUDIO #3");
		configInput(IN_INPUT+3, "AUDIO #4");
		configOutput(OUT_OUTPUT, "AUDIO #1");
		configOutput(OUT_OUTPUT+1, "AUDIO #2");
		configOutput(OUT_OUTPUT+2, "AUDIO #3");
		configOutput(OUT_OUTPUT+3, "AUDIO #4");
	}

	void processBypass(const ProcessArgs& args) override {
		outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage());
		outputs[OUT_OUTPUT+1].setVoltage(inputs[IN_INPUT+1].getVoltage());
		outputs[OUT_OUTPUT+2].setVoltage(inputs[IN_INPUT+2].getVoltage());
		outputs[OUT_OUTPUT+3].setVoltage(inputs[IN_INPUT+3].getVoltage());
	}
	
	void onSampleRateChange() override {
		fadeCoeff = 1000 / APP->engine->getSampleRate();	// 1ms choke
	}

	void process(const ProcessArgs& args) override {

		choke[0] = params[CHOKE_PARAMS].getValue();
		choke[1] = params[CHOKE_PARAMS+1].getValue();
		choke[2] = params[CHOKE_PARAMS+2].getValue();
		lights[CHOKE_LIGHT].setBrightness(choke[0]);
		lights[CHOKE_LIGHT+1].setBrightness(choke[1]);
		lights[CHOKE_LIGHT+2].setBrightness(choke[2]);
		limit[0] = params[LIMIT_PARAMS].getValue();
		limit[1] = params[LIMIT_PARAMS+1].getValue();
		limit[2] = params[LIMIT_PARAMS+2].getValue();
		limit[3] = params[LIMIT_PARAMS+3].getValue();
		lights[LIMIT_LIGHT].setBrightness(limit[0]);
		lights[LIMIT_LIGHT+1].setBrightness(limit[1]);
		lights[LIMIT_LIGHT+2].setBrightness(limit[2]);
		lights[LIMIT_LIGHT+3].setBrightness(limit[3]);

		// --------  SLOT  1  --------

		if (inputs[TRIG_INPUT].isConnected()) {
			trigValue[0] = inputs[TRIG_INPUT].getVoltage();
			if (trigValue[0] >= 1 && prevTrigValue[0] < 1) {
				trigState[0] = true;
				if (inputs[ACCENT_INPUT].getVoltage() >= 1)
					sustain[0] = params[ACCENT_PARAMS].getValue();
				else
					sustain[0] = params[NOACCENT_PARAMS].getValue();
				
				if (choke[0]) {
					trigState[1] = false;
					choking[0] = true;
					currentFade[0] = 1;
				}
			}
			prevTrigValue[0] = trigValue[0];
			out[0] = inputs[IN_INPUT].getVoltage() * sustain[0];
		} else
			out[0] = 0;

		// --------  SLOT  2  --------

		if (inputs[TRIG_INPUT+1].isConnected()) {
			trigValue[1] = inputs[TRIG_INPUT+1].getVoltage();
			if (trigValue[1] >= 1 && prevTrigValue[1] < 1) {
				trigState[1] = true;
				if (inputs[ACCENT_INPUT+1].getVoltage() >= 1)
					sustain[1] = params[ACCENT_PARAMS+1].getValue();
				else
					sustain[1] = params[NOACCENT_PARAMS+1].getValue();
				
				if (choke[1]) {
					choking[1] = true;
					currentFade[1] = 1;
				}
			}
			prevTrigValue[1] = trigValue[1];
			out[1] = inputs[IN_INPUT+1].getVoltage() * sustain[1];
		} else
			out[1] = 0;

		// --------  SLOT  3  --------

		if (inputs[TRIG_INPUT+2].isConnected()) {
		trigValue[2] = inputs[TRIG_INPUT+2].getVoltage();
			if (trigValue[2] >= 1 && prevTrigValue[2] < 1) {
				trigState[2] = true;
				if (inputs[ACCENT_INPUT+2].getVoltage() >= 1)
					sustain[2] = params[ACCENT_PARAMS+2].getValue();
				else
					sustain[2] = params[NOACCENT_PARAMS+2].getValue();
				
				if (choke[2]) {
					choking[2] = true;
					currentFade[2] = 1;
				}
			}
			prevTrigValue[2] = trigValue[2];
			out[2] = inputs[IN_INPUT+2].getVoltage() * sustain[2];
		} else 
			out[2] = 0;

		// --------  SLOT  4  --------

		if (inputs[TRIG_INPUT+3].isConnected()) {
			trigValue[3] = inputs[TRIG_INPUT+3].getVoltage();
			if (trigValue[3] >= 1 && prevTrigValue[3] < 1) {
				trigState[3] = true;
				if (inputs[ACCENT_INPUT+3].getVoltage() >= 1)
					sustain[3] = params[ACCENT_PARAMS+3].getValue();
				else
					sustain[3] = params[NOACCENT_PARAMS+3].getValue();
			}
			prevTrigValue[3] = trigValue[3];
			out[3] = inputs[IN_INPUT+3].getVoltage() * sustain[3];
		} else
			out[3] = 0;

		// ------------------------------------------------------------

		// --------  OUT  1  --------

		outSum = out[0];

		if (limit[0]) {
			if (outSum > 5)
				outSum = 5;
			else if (outSum < -5)
				outSum = -5;
		}

		if (outputs[OUT_OUTPUT].isConnected()) { 
			outputs[OUT_OUTPUT].setVoltage(outSum);
			outSum = 0;
		} else {
			outSum = out[0];
		}

		// --------  OUT  2  --------
			
		if (choke[0]) {
			if (choking[0]) {
				currentFade[0] -= fadeCoeff;
				if (currentFade[0] < 0) {
					choking[0] = false;
					if (inputs[TRIG_INPUT+1].isConnected())
						trigState[0] = false;
					trigState[1] = false;
					currentFade[0] = 0;
				} else {
					if (trigState[0] && trigState[1])
						out[1] *= currentFade[0];
					else
						out[1] = 0;
				}
			} else {
				if (!trigState[1])
					out[1] = 0;
			}
		}

		outSum += out[1];
		if (limit[1]) {
			if (outSum > 5)
				outSum = 5;
			else if (outSum < -5)
				outSum = -5;
		}

		if (outputs[OUT_OUTPUT+1].isConnected()) { 
			outputs[OUT_OUTPUT+1].setVoltage(outSum);
			outSum = 0;
		}

		// --------  OUT  3  --------
		
		if (choke[1]) {
			if (choking[1]) {
				currentFade[1] -= fadeCoeff;
				if (currentFade[1] < 0) {
					choking[1] = false;
					if (inputs[TRIG_INPUT+2].isConnected())
						trigState[1] = false;
					trigState[2] = false;
					currentFade[1] = 0;
				} else {
					if (trigState[1] && trigState[2])
						out[2] *= currentFade[1];
					else
						out[2] = 0;
				}
			} else {
				if (!trigState[2])
					out[2] = 0;
			}
		}

		outSum += out[2];
		if (limit[2]) {
			if (outSum > 5)
				outSum = 5;
			else if (outSum < -5)
				outSum = -5;
		}

		if (outputs[OUT_OUTPUT+2].isConnected()) { 
			outputs[OUT_OUTPUT+2].setVoltage(outSum);
			outSum = 0;
		}

		// --------  OUT  4  --------

		if (choke[2]) {
			if (choking[2]) {
				currentFade[2] -= fadeCoeff;
				if (currentFade[2] < 0) {
					choking[2] = false;
					if (inputs[TRIG_INPUT+3].isConnected())
						trigState[2] = false;
					trigState[3] = false;
					currentFade[2] = 0;
				} else {
					if (trigState[2] && trigState[3])
						out[3] *= currentFade[2];
					else
						out[3] = 0;
				}
			} else {
				if (!trigState[3])
					out[3] = 0;
			}
		}

		if (outputs[OUT_OUTPUT+3].isConnected()) { 
			outSum += out[3];
			if (limit[3]) {
			if (outSum > 5)
				outSum = 5;
			else if (outSum < -5)
				outSum = -5;
			}
			outputs[OUT_OUTPUT+3].setVoltage(outSum);
		} else {
			outputs[OUT_OUTPUT+3].setVoltage(0);
		}
	}
};

struct Drummer4Widget : ModuleWidget {
	Drummer4Widget(Drummer4* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Drummer4.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));		

		const float x = 13.46;
		const float xs = 7.75;

		for (int i=0; i<4; i++) {
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs+(x*i), 20)), module, Drummer4::TRIG_INPUT+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xs+(x*i), 32)), module, Drummer4::NOACCENT_PARAMS+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs+(x*i), 49)), module, Drummer4::ACCENT_INPUT+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xs+(x*i), 61)), module, Drummer4::ACCENT_PARAMS+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xs+(x*i), 77)), module, Drummer4::IN_INPUT+i));
			if (i < 3)
				addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xs+(x*i), 90.5)), module, Drummer4::CHOKE_PARAMS+i, Drummer4::CHOKE_LIGHT+i));
			addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(xs+(x*i), 101.5)), module, Drummer4::LIMIT_PARAMS+i, Drummer4::LIMIT_LIGHT+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xs+(x*i), 115.5)), module, Drummer4::OUT_OUTPUT+i));
		}
	}
};

Model* modelDrummer4 = createModel<Drummer4, Drummer4Widget>("Drummer4");