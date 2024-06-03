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
	int chanVca;
	float vcaValue;
	float limitValue;
	bool limit = false;
	int polyOuts = POLYPHONIC;
	float out[2];
	float sumOut[2];


	SickoAmp() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		
		configInput(IN_INPUT+LEFT, "Left");
		configInput(IN_INPUT+RIGHT, "Right");

		configParam(VCA_PARAM, 0.f, 2.f, 0.f, "Base Level", "%", 0, 100);
		configParam(CV_ATNV_PARAM, -1.f, 1.f, 0.5f, "Level CV", "%", 0, 200);
		configInput(CV_INPUT, "Level");

		configSwitch(LIMIT_SWITCH, 0.f, 1.f, 0.f, "Limit", {"Off", "On"});
		configParam(LIMIT_PARAM, 0.f, 10.f, 5.f, "Limit Voltage", "±v");		

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

		chan = inputs[IN_INPUT+LEFT].getChannels();

		if (inputs[CV_INPUT].isConnected())
			chanVca = inputs[CV_INPUT].getChannels();
		else
			chanVca = 1;

		limit = params[LIMIT_SWITCH].getValue();
		if (!limit)
			limitValue = 10;
		else
			limitValue = params[LIMIT_PARAM].getValue();

		sumOut[LEFT] = 0.f;
		sumOut[RIGHT] = 0.f;

		if (chanVca == 1) {
			
			vcaValue = params[VCA_PARAM].getValue() + (inputs[CV_INPUT].getVoltage() * params[CV_ATNV_PARAM].getValue() * 0.2);
			if (vcaValue < 0)
				vcaValue = 0;
			else if (vcaValue > 2)
				vcaValue = 2;

			for (int c = 0; c < chan; c++) {

				if (polyOuts) {

					out[LEFT] = inputs[IN_INPUT+LEFT].getVoltage(c) * vcaValue;
					out[RIGHT] = inputs[IN_INPUT+RIGHT].getVoltage(c) * vcaValue;

					if (out[LEFT] < -limitValue)
						out[LEFT] = -limitValue;
					else if (out[LEFT] > limitValue)
						out[LEFT] = limitValue;

					if (out[RIGHT] < -limitValue)
						out[RIGHT] = -limitValue;
					else if (out[RIGHT] > limitValue)
						out[RIGHT] = limitValue;

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

				sumOut[LEFT] *= vcaValue;
				sumOut[RIGHT] *= vcaValue;

				if (sumOut[LEFT] < -limitValue)
					sumOut[LEFT] = -limitValue;
				else if (sumOut[LEFT] > limitValue)
					sumOut[LEFT] = limitValue;

				if (sumOut[RIGHT] < -limitValue)
					sumOut[RIGHT] = -limitValue;
				else if (sumOut[RIGHT] > limitValue)
					sumOut[RIGHT] = limitValue;

				outputs[OUT_OUTPUT+LEFT].setVoltage(sumOut[LEFT]);
				outputs[OUT_OUTPUT+RIGHT].setVoltage(sumOut[RIGHT]);

				outputs[OUT_OUTPUT+LEFT].setChannels(1);
				outputs[OUT_OUTPUT+RIGHT].setChannels(1);

			}

		} else {	// polyVCA

			for (int c = 0; c < chanVca; c++) {
				vcaValue = params[VCA_PARAM].getValue() + (inputs[CV_INPUT].getVoltage(c) * params[CV_ATNV_PARAM].getValue() * 0.2);
				if (vcaValue < 0)
					vcaValue = 0;
				else if (vcaValue > 2)
					vcaValue = 2;

				if (polyOuts) {

					out[LEFT] = inputs[IN_INPUT+LEFT].getVoltage(c) * vcaValue;
					out[RIGHT] = inputs[IN_INPUT+RIGHT].getVoltage(c) * vcaValue;

					if (out[LEFT] < -limitValue)
						out[LEFT] = -limitValue;
					else if (out[LEFT] > limitValue)
						out[LEFT] = limitValue;

					if (out[RIGHT] < -limitValue)
						out[RIGHT] = -limitValue;
					else if (out[RIGHT] > limitValue)
						out[RIGHT] = limitValue;

					outputs[OUT_OUTPUT+LEFT].setVoltage(out[LEFT], c);
					outputs[OUT_OUTPUT+RIGHT].setVoltage(out[RIGHT], c);
				} else {
					sumOut[LEFT] += inputs[IN_INPUT+LEFT].getVoltage(c);
					sumOut[RIGHT] += inputs[IN_INPUT+RIGHT].getVoltage(c);
				}
			}


			if (polyOuts) {

				outputs[OUT_OUTPUT+LEFT].setChannels(chanVca);
				outputs[OUT_OUTPUT+RIGHT].setChannels(chanVca);
			
			} else {

				sumOut[LEFT] *= vcaValue;
				sumOut[RIGHT] *= vcaValue;

				if (sumOut[LEFT] < -limitValue)
					sumOut[LEFT] = -limitValue;
				else if (sumOut[LEFT] > limitValue)
					sumOut[LEFT] = limitValue;

				if (sumOut[RIGHT] < -limitValue)
					sumOut[RIGHT] = -limitValue;
				else if (sumOut[RIGHT] > limitValue)
					sumOut[RIGHT] = limitValue;

				outputs[OUT_OUTPUT+LEFT].setVoltage(sumOut[LEFT]);
				outputs[OUT_OUTPUT+RIGHT].setVoltage(sumOut[RIGHT]);

				outputs[OUT_OUTPUT+LEFT].setChannels(1);
				outputs[OUT_OUTPUT+RIGHT].setChannels(1);

			}
		}
		
		// end process
	}
};

struct SickoAmpWidget : ModuleWidget {
	SickoAmpWidget(SickoAmp* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SickoAmp.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float x = 7.62f;
		const float yIn1 = 17;
		const float yIn2 = 26;

		const float yVcaAtnv = 44.9;
		const float yVcaIn = 55.8;
		const float yVca = 66.9;

		const float yLimSw = 81.7;
		const float yLimKnob = 91.7;

		const float yOut1 = 108.5;
		const float yOut2 = 117.5f;

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(x, yIn1)), module, SickoAmp::IN_INPUT+LEFT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(x, yIn2)), module, SickoAmp::IN_INPUT+RIGHT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(x, yVca)), module, SickoAmp::VCA_PARAM));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(x, yVcaAtnv)), module, SickoAmp::CV_ATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(x, yVcaIn)), module, SickoAmp::CV_INPUT));
	
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(x, yLimSw)), module, SickoAmp::LIMIT_SWITCH, SickoAmp::LIMIT_LIGHT));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(x, yLimKnob)), module, SickoAmp::LIMIT_PARAM));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(x, yOut1)), module, SickoAmp::OUT_OUTPUT+LEFT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(x, yOut2)), module, SickoAmp::OUT_OUTPUT+RIGHT));

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