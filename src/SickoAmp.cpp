#define LEFT 0
#define RIGHT 1
#define MONOPHONIC 0
#define POLYPHONIC 1

#include "plugin.hpp"

struct SickoAmp : Module {
	enum ParamId {
		VCA_PARAM,
		CV_ATNV_PARAM,
		LIMIT_SWITCH,
		LIMIT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(IN_INPUT,2),
		CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT,2),
		OUTPUTS_LEN
	};
	enum LightId {
		LIMIT_LIGHT,
		LIGHTS_LEN
	};

	int chan;
	float vcaVal;
	int polyOuts = POLYPHONIC;
	float out[2];
	float sumOut[2];


	SickoAmp() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		
		configInput(IN_INPUT+LEFT, "Left");
		configInput(IN_INPUT+RIGHT, "Right");

		configParam(VCA_PARAM, 0.f, 2.0f, 1.0f, "Amplify", "%", 0, 100);
		configParam(CV_ATNV_PARAM, -1.0f, 1.0f, 0.5f, "CV Attenuv.");
		configInput(CV_INPUT, "CV");

		configSwitch(LIMIT_SWITCH, 0.f, 1.f, 0.f, "Limit", {"Off", "On"});
		configParam(LIMIT_PARAM, 0.f, 10.0f, 10.f, "Limit Voltage", "±v");		

		configOutput(OUT_OUTPUT+LEFT, "Left");
		configOutput(OUT_OUTPUT+RIGHT, "Right");
	}

	void onReset(const ResetEvent &e) override {
		polyOuts = POLYPHONIC;
	    Module::onReset(e);
	}

	void processBypass(const ProcessArgs &args) override {
		outputs[OUT_OUTPUT+LEFT].setVoltage(inputs[IN_INPUT+LEFT].getVoltage());
		outputs[OUT_OUTPUT+RIGHT].setVoltage(inputs[IN_INPUT+RIGHT].getVoltage());
		Module::processBypass(args);
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "PolyOuts", json_integer(polyOuts));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t* polyOutsJ = json_object_get(rootJ, "PolyOuts");
		if (polyOutsJ)
			polyOuts = json_integer_value(polyOutsJ);
	}

	bool isPolyOuts() {
		return polyOuts;
	}

	void setPolyOuts(bool poly) {
		if (poly) 
			polyOuts = POLYPHONIC;
		else {
			polyOuts = MONOPHONIC;
			outputs[OUT_OUTPUT+LEFT].setChannels(1);
			outputs[OUT_OUTPUT+RIGHT].setChannels(1);
		}
	}

	void process(const ProcessArgs& args) override {
		lights[LIMIT_LIGHT].setBrightness(params[LIMIT_SWITCH].getValue());

		chan = std::max(1, inputs[IN_INPUT+LEFT].getChannels());

		vcaVal = params[VCA_PARAM].getValue() + (inputs[CV_INPUT].getVoltage() * params[CV_ATNV_PARAM].getValue() * 0.2);
		if (vcaVal < 0)
			vcaVal = 0;
		else if (vcaVal > 2)
			vcaVal = 2;

		sumOut[0] = 0.f;
		sumOut[1] = 0.f;

		for (int c = 0; c < chan; c++) {

			if (polyOuts) {

				out[LEFT] = inputs[IN_INPUT+LEFT].getVoltage(c) * vcaVal;
				out[RIGHT] = inputs[IN_INPUT+RIGHT].getVoltage(c) * vcaVal;

				if (params[LIMIT_SWITCH].getValue()) {
					if (out[LEFT] < -params[LIMIT_PARAM].getValue())
						out[LEFT] = -params[LIMIT_PARAM].getValue();
					else if (out[LEFT] > params[LIMIT_PARAM].getValue())
						out[LEFT] = params[LIMIT_PARAM].getValue();

					if (out[RIGHT] < -params[LIMIT_PARAM].getValue())
						out[RIGHT] = -params[LIMIT_PARAM].getValue();
					else if (out[RIGHT] > params[LIMIT_PARAM].getValue())
						out[RIGHT] = params[LIMIT_PARAM].getValue();
				} else {
					if (out[LEFT] < -10.f)
						out[LEFT] = -10.f;
					else if (out[LEFT] > 10.f)
						out[LEFT] = 10.f;

					if (out[RIGHT] < -10.f)
						out[RIGHT] = -10.f;
					else if (out[RIGHT] > 10.f)
						out[RIGHT] = 10.f;
				}
				outputs[OUT_OUTPUT+LEFT].setVoltage(out[LEFT], c);
				outputs[OUT_OUTPUT+RIGHT].setVoltage(out[RIGHT], c);
			} else {

				sumOut[LEFT] += inputs[IN_INPUT+LEFT].getVoltage(c);
				sumOut[RIGHT] += inputs[IN_INPUT+RIGHT].getVoltage(c);

			}
		}

		if (polyOuts) {

			outputs[OUT_OUTPUT+LEFT].setChannels(chan);
			outputs[OUT_OUTPUT+RIGHT].setChannels(chan);
		
		} else {

			sumOut[LEFT] *= vcaVal;
			sumOut[RIGHT] *= vcaVal;

			if (params[LIMIT_SWITCH].getValue()) {
				if (sumOut[LEFT] < -params[LIMIT_PARAM].getValue())
					sumOut[LEFT] = -params[LIMIT_PARAM].getValue();
				else if (sumOut[LEFT] > params[LIMIT_PARAM].getValue())
					sumOut[LEFT] = params[LIMIT_PARAM].getValue();

				if (sumOut[RIGHT] < -params[LIMIT_PARAM].getValue())
					sumOut[RIGHT] = -params[LIMIT_PARAM].getValue();
				else if (sumOut[RIGHT] > params[LIMIT_PARAM].getValue())
					sumOut[RIGHT] = params[LIMIT_PARAM].getValue();
			} else {
				if (sumOut[LEFT] < -10.f)
					sumOut[LEFT] = -10.f;
				else if (sumOut[LEFT] > 10.f)
					sumOut[LEFT] = 10.f;

				if (sumOut[RIGHT] < -10.f)
					sumOut[RIGHT] = -10.f;
				else if (sumOut[RIGHT] > 10.f)
					sumOut[RIGHT] = 10.f;
			}

			outputs[OUT_OUTPUT+LEFT].setVoltage(sumOut[LEFT]);
			outputs[OUT_OUTPUT+RIGHT].setVoltage(sumOut[RIGHT]);

		}

		// end process
	}
};

struct SickoAmpWidget : ModuleWidget {
	SickoAmpWidget(SickoAmp* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SickoAmp.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// buttons --- 4.1
		// trimpot --- x  3.7 --- y 4.3
		// trimpot senza stanghetta --- y 3.7
		// smallRoundKnob --- x 4.6 --- y 5.1
		// roundBlackKnob --- x 5.7 --- y 6.4
		// input/output --- 4.5

		const float x = 7.62f;
		const float yIn1 = 17;
		const float yIn2 = 26;

		const float yVca = 44.9;
		const float yVcaAtnv = 56.7;
		const float yVcaIn = 65.9;

		const float yLimSw = 82.5;
		const float yLimKnob = 92;

		const float yOut1 = 108.5;
		const float yOut2 = 117.5f;

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x, yIn1)), module, SickoAmp::IN_INPUT+LEFT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x, yIn2)), module, SickoAmp::IN_INPUT+RIGHT));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x, yVca)), module, SickoAmp::VCA_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(x, yVcaAtnv)), module, SickoAmp::CV_ATNV_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x, yVcaIn)), module, SickoAmp::CV_INPUT));
	
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(x, yLimSw)), module, SickoAmp::LIMIT_SWITCH, SickoAmp::LIMIT_LIGHT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(x, yLimKnob)), module, SickoAmp::LIMIT_PARAM));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(x, yOut1)), module, SickoAmp::OUT_OUTPUT+LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(x, yOut2)), module, SickoAmp::OUT_OUTPUT+RIGHT));

	}

	void appendContextMenu(Menu *menu) override {
	   	SickoAmp *module = dynamic_cast<SickoAmp*>(this->module);
			assert(module);
		
		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolMenuItem("Polyphonic OUTs", "", [=]() {
				return module->isPolyOuts();
			}, [=](bool poly) {
				module->setPolyOuts(poly);
		}));
	}
};

Model* modelSickoAmp = createModel<SickoAmp, SickoAmpWidget>("SickoAmp");