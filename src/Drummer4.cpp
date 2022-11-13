#include "plugin.hpp"

struct Drummer4 : Module {
	enum ParamId {
		ENUMS(LIMIT_SWITCH,4),
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
		LIGHTS_LEN
	};

	float trigValue[4] = {0.f, 0.f, 0.f, 0.f};
	float prevTrigValue[4] = {0.f, 0.f, 0.f, 0.f};
	bool trigState[4] = {false, false, false, false};

	float sustain[4] = {1.f, 1.f, 1.f, 1.f};

	float out[4] = {0.f, 0.f, 0.f, 0.f};

	Drummer4() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(LIMIT_SWITCH, 0.f, 1.f, 0.f, "Limit #1", {"Off", "5v"});
		configSwitch(LIMIT_SWITCH+1, 0.f, 1.f, 0.f, "Limit #2", {"Off", "5v"});
		configSwitch(LIMIT_SWITCH+2, 0.f, 1.f, 0.f, "Limit #3", {"Off", "5v"});
		configSwitch(LIMIT_SWITCH+3, 0.f, 1.f, 0.f, "Limit #4", {"Off", "5v"});
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
	
	void process(const ProcessArgs& args) override {

		trigValue[0] = inputs[TRIG_INPUT].getVoltage();
		if (trigValue[0] >= 1 && prevTrigValue[0] < 1){
			if (inputs[ACCENT_INPUT].getVoltage() >= 1)
				sustain[0] = params[ACCENT_PARAMS].getValue();
			else
				sustain[0] = params[NOACCENT_PARAMS].getValue();
		}
		prevTrigValue[0] = trigValue[0];
		out[0] = inputs[IN_INPUT].getVoltage() * sustain[0];

		trigValue[1] = inputs[TRIG_INPUT+1].getVoltage();
		if (trigValue[1] >= 1 && prevTrigValue[1] < 1){
			if (inputs[ACCENT_INPUT+1].getVoltage() >= 1)
				sustain[1] = params[ACCENT_PARAMS+1].getValue();
			else
				sustain[1] = params[NOACCENT_PARAMS+1].getValue();
		}
		prevTrigValue[1] = trigValue[1];
		out[1] = inputs[IN_INPUT+1].getVoltage() * sustain[1];

		trigValue[2] = inputs[TRIG_INPUT+2].getVoltage();
		if (trigValue[2] >= 1 && prevTrigValue[2] < 1){
			if (inputs[ACCENT_INPUT+2].getVoltage() >= 1)
				sustain[2] = params[ACCENT_PARAMS+2].getValue();
			else
				sustain[2] = params[NOACCENT_PARAMS+2].getValue();
		}
		prevTrigValue[2] = trigValue[2];
		out[2] = inputs[IN_INPUT+2].getVoltage() * sustain[2];

		trigValue[3] = inputs[TRIG_INPUT+3].getVoltage();
		if (trigValue[3] >= 1 && prevTrigValue[3] < 1){
			if (inputs[ACCENT_INPUT+3].getVoltage() >= 1)
				sustain[3] = params[ACCENT_PARAMS+3].getValue();
			else
				sustain[3] = params[NOACCENT_PARAMS+3].getValue();
		}
		prevTrigValue[3] = trigValue[3];
		out[3] = inputs[IN_INPUT+3].getVoltage() * sustain[3];

		// --------------------------------------------------------

		if (outputs[OUT_OUTPUT].isConnected()) { 
			if (params[LIMIT_SWITCH].getValue()) {
				if (out[0] > 5)
					out[0] = 5;
				else if (out[0] < -5)
					out[0] = -5;
			}
			outputs[OUT_OUTPUT].setVoltage(out[0]);
		} else {
			out[1] += out[0];
		}

		if (outputs[OUT_OUTPUT+1].isConnected()) { 
			if (params[LIMIT_SWITCH+1].getValue()) {
				if (out[1] > 5)
					out[1] = 5;
				else if (out[1] < -5)
					out[1] = -5;
			}
			outputs[OUT_OUTPUT+1].setVoltage(out[1]);
		} else {
			out[2] += out[1];
		}

		if (outputs[OUT_OUTPUT+2].isConnected()) { 
			if (params[LIMIT_SWITCH+2].getValue()) {
				if (out[2] > 5)
					out[2] = 5;
				else if (out[2] < -5)
					out[2] = -5;
			}
			outputs[OUT_OUTPUT+2].setVoltage(out[2]);
		} else {
			out[3] += out[2];
		}

		if (outputs[OUT_OUTPUT+3].isConnected()) { 
			if (params[LIMIT_SWITCH+3].getValue()) {
				if (out[3] > 5)
					out[3] = 5;
				else if (out[3] < -5)
					out[3] = -5;
			}
			outputs[OUT_OUTPUT+3].setVoltage(out[3]);
		} else {
			outputs[OUT_OUTPUT+3].setVoltage(0);
		}
	}
};

struct Drummer4Widget : ModuleWidget {
	Drummer4Widget(Drummer4* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Drummer4.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));		

		float x = 13.46;
		float xs = 7.75;

		for (int i=0;i<4;i++) {
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xs+(x*i), 20)), module, Drummer4::TRIG_INPUT+i));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(xs+(x*i), 32)), module, Drummer4::NOACCENT_PARAMS+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xs+(x*i), 49)), module, Drummer4::ACCENT_INPUT+i));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(xs+(x*i), 61)), module, Drummer4::ACCENT_PARAMS+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xs+(x*i), 77)), module, Drummer4::IN_INPUT+i));
			addParam(createParamCentered<CKSS>(mm2px(Vec(xs+(x*i), 95)), module, Drummer4::LIMIT_SWITCH+i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xs+(x*i), 115.5)), module, Drummer4::OUT_OUTPUT+i));
		}
	}

};

Model* modelDrummer4 = createModel<Drummer4, Drummer4Widget>("Drummer4");