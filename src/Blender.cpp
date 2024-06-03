#define MONOPHONIC 0
#define POLYPHONIC 1
#include "plugin.hpp"

struct Blender : Module {
	float mix = 0;
	float mod2 = 0;
	float input1[2][16];
	float input2[2][16];
	float mixedOut[2][16];
	float summedOut[2];
	int limit;
	float vol;
	float vol1;
	float vol2;
	int phase1;
	int phase2;

	int chanL;
	int chanR;
	int polyOuts = POLYPHONIC;

	enum ParamId {
		ENUMS(PHASE_SWITCH,2),
		MIX_PARAMS,
		VOL_PARAMS,
		LIMIT_PARAMS,
		VOL1_PARAMS,
		VOL2_PARAMS,
		MOD1_ATNV_PARAMS,
		MOD2_ATNV_PARAMS,
		MOD1_RANGE_SWITCH,
		MOD2_RANGE_SWITCH,
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(IN1_INPUT,2),
		ENUMS(IN2_INPUT,2),
		MOD1_INPUT,
		MOD2_INPUT,
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

	Blender() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(IN1_INPUT, "In1 L");
		configInput(IN1_INPUT+1, "In1 R");
		configInput(IN2_INPUT, "In2 L");
		configInput(IN2_INPUT+1, "In2 R");

		configSwitch(PHASE_SWITCH, 0.f, 1.f, 1.f, "In1 Phase", {"Inverted", "Normal"});
		configSwitch(PHASE_SWITCH+1, 0.f, 1.f, 1.f, "In2 Phase", {"Inverted", "Normal"});

		configParam(MIX_PARAMS, 0.f,1.f, 0.5f, "Mix", "%", 0, 100);
		configParam(VOL1_PARAMS, 0.f,1.f, 1.f, "Volume 1", "%", 0, 100);
		configParam(VOL2_PARAMS, 0.f,1.f, 1.f, "Volume 2", "%", 0, 100);

		configParam(MOD1_ATNV_PARAMS, -1.f,1.f, 0.f, "Mod Attenuv.", "%", 0, 100);
		configInput(MOD1_INPUT, "Mod");
		configSwitch(MOD1_RANGE_SWITCH, 0.f, 1.f, 1.f, "Mod Input Range", {"Bipolar", "Unipolar"});

		configParam(MOD2_ATNV_PARAMS, -1.f,1.f, 0.f, "Mod2 Attenuv.", "%", 0, 100);
		configInput(MOD2_INPUT, "Mod2");
		configSwitch(MOD2_RANGE_SWITCH, 0.f, 1.f, 1.f, "Mod2 Input Range", {"Bipolar", "Unipolar"});

		configParam(VOL_PARAMS, 0.f,2.f, 1.f, "Master Volume", "%", 0, 100);
		configSwitch(LIMIT_PARAMS, 0.f, 1.f, 0.f, "Limit", {"Off", "±5v"});

		configOutput(OUT_OUTPUT, "L");
		configOutput(OUT_OUTPUT+1, "R");
	}

	void onReset(const ResetEvent &e) override {
		polyOuts = POLYPHONIC;
		Module::onReset(e);
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
			polyOuts = 1;
		else {
			polyOuts = 0;
			outputs[OUT_OUTPUT].setChannels(1);
			outputs[OUT_OUTPUT+1].setChannels(1);
		}
	}

	void process(const ProcessArgs& args) override {
		
		limit = params[LIMIT_PARAMS].getValue();
		lights[LIMIT_LIGHT].setBrightness(limit);

		if (outputs[OUT_OUTPUT].isConnected() || outputs[OUT_OUTPUT+1].isConnected()) {
			vol1 = params[VOL1_PARAMS].getValue();
			vol2 = params[VOL2_PARAMS].getValue();
			phase1 = params[PHASE_SWITCH].getValue();
			phase2 = params[PHASE_SWITCH+1].getValue();
			vol = params[VOL_PARAMS].getValue();

			if (inputs[MOD1_INPUT].isConnected()) {		
				if (inputs[MOD2_INPUT].isConnected()) {
					if (params[MOD2_RANGE_SWITCH].getValue() == 1)
						mod2 = params[MOD2_ATNV_PARAMS].getValue() * inputs[MOD2_INPUT].getVoltage() * 0.1;
					else
						mod2 = params[MOD2_ATNV_PARAMS].getValue() * (inputs[MOD2_INPUT].getVoltage() + 5) * 0.1;
				} else {
					mod2 = 0;
				}

				if (mod2 > 1)
					mod2 = 1;
				else if (mod2 < -1)
					mod2 = -1;

				if (params[MOD1_RANGE_SWITCH].getValue() == 1)
					mix = params[MIX_PARAMS].getValue() + ( (params[MOD1_ATNV_PARAMS].getValue() * inputs[MOD1_INPUT].getVoltage() * 0.1) + mod2);
				else
					mix = params[MIX_PARAMS].getValue() + ( (params[MOD1_ATNV_PARAMS].getValue() * (inputs[MOD1_INPUT].getVoltage() + 5) * 0.1) + mod2);

				if (mix > 1)
					mix = 1;
				else if (mix < 0) {
					mix = -mix;
					if (mix < -1)
						mix = -1;
				}
			} else {
				mix = params[MIX_PARAMS].getValue();
			}
		}

		// LEFT CHANNEL

		if (outputs[OUT_OUTPUT].isConnected()) {
			chanL = std::max(1, inputs[IN1_INPUT].getChannels());
			summedOut[0] = 0;

			for (int c = 0; c < chanL; c++) {

				input1[0][c] = inputs[IN1_INPUT].getVoltage(c) * vol1;
				if (!phase1)	// inverted if because of inverted switch widget
					input1[0][c] = -input1[0][c];

				input2[0][c] = inputs[IN2_INPUT].getVoltage(c) * vol2;
				if (!phase2)	// inverted if because of inverted switch widget
					input2[0][c] = -input2[0][c];
				
				mixedOut[0][c] = (input1[0][c] * (1 - mix)) + (input2[0][c] * mix) * vol;

				switch (polyOuts) {
					case MONOPHONIC:
						summedOut[0] += mixedOut[0][c];
					break;

					case POLYPHONIC:
						if (limit) {
							if (mixedOut[0][c] > 5)
								mixedOut[0][c] = 5;
							else if (mixedOut[0][c] < -5)
								mixedOut[0][c] = -5;
						}
						outputs[OUT_OUTPUT].setVoltage(mixedOut[0][c], c);
					break;
				}

			}

		} else {
			for (int c = 0; c < 16; c++)
				outputs[OUT_OUTPUT].setVoltage(0, c);
		}

		// RIGHT CHANNEL

		if (outputs[OUT_OUTPUT+1].isConnected()) {
			chanR = std::max(1, inputs[IN1_INPUT+1].getChannels());
			summedOut[1] = 0;

			for (int c = 0; c < chanR; c++) {

				input1[1][c] = inputs[IN1_INPUT+1].getVoltage(c) * vol1;
				if (!phase1)	// inverted if because of inverted switch widget
					input1[1][c] = -input1[1][c];

				input2[1][c] = inputs[IN2_INPUT+1].getVoltage(c) * vol2;
				if (!phase2)	// inverted if because of inverted switch widget
					input2[1][c] = -input2[1][c];

				mixedOut[1][c] = (input1[1][c] * (1 - mix)) + (input2[1][c] * mix) * vol;

				switch (polyOuts) {
					case MONOPHONIC:
						summedOut[1] += mixedOut[1][c];
					break;

					case POLYPHONIC:
						if (limit) {
							if (mixedOut[1][c] > 5)
								mixedOut[1][c] = 5;
							else if (mixedOut[1][c] < -5)
								mixedOut[1][c] = -5;
						}
						outputs[OUT_OUTPUT+1].setVoltage(mixedOut[1][c], c);
					break;
				}
					
			}

		} else {
			for (int c = 0; c < 16; c++)
				outputs[OUT_OUTPUT+1].setVoltage(0, c);
		}

		switch (polyOuts) {
			case MONOPHONIC:
				if (limit) {
					if (summedOut[0] > 5)
						summedOut[0] = 5;
					else if (summedOut[0] < -5)
						summedOut[0] = -5;

					if (summedOut[1] > 5)
						summedOut[1] = 5;
					else if (summedOut[1] < -5)
						summedOut[1] = -5;
				}
				outputs[OUT_OUTPUT].setVoltage(summedOut[0]);
				outputs[OUT_OUTPUT+1].setVoltage(summedOut[1]);
			break;

			case POLYPHONIC:
				outputs[OUT_OUTPUT].setChannels(chanL);
				outputs[OUT_OUTPUT+1].setChannels(chanR);
			break;
		}
		
	}
};

struct BlenderWidget : ModuleWidget {
	BlenderWidget(Blender* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Blender.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(7, 25)), module, Blender::IN1_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(16.5, 25)), module, Blender::IN1_INPUT+1));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(29.55, 25)), module, Blender::IN2_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(38.95, 25)), module, Blender::IN2_INPUT+1));
		addParam(createParamCentered<CKSS>(mm2px(Vec(11.75, 34.6)), module, Blender::PHASE_SWITCH));
		addParam(createParamCentered<CKSS>(mm2px(Vec(34.1, 34.6)), module, Blender::PHASE_SWITCH+1));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(11, 47.9)), module, Blender::VOL1_PARAMS));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(34.92, 47.9)), module, Blender::VOL2_PARAMS));

		addParam(createParamCentered<SickoLargeKnob>(mm2px(Vec(22.86, 54)), module, Blender::MIX_PARAMS));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(8, 77.7)), module, Blender::MOD1_INPUT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(18.4, 77.7)), module, Blender::MOD1_RANGE_SWITCH));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(31.4, 77.7)), module, Blender::MOD1_ATNV_PARAMS));
		
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(11.75, 115.7)), module, Blender::MOD2_INPUT));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(11.75, 94)), module, Blender::MOD2_ATNV_PARAMS));
		addParam(createParamCentered<CKSS>(mm2px(Vec(11.75, 105)), module, Blender::MOD2_RANGE_SWITCH));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(31.1, 93.4)), module, Blender::VOL_PARAMS));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(39.75, 93.4)), module, Blender::LIMIT_PARAMS, Blender::LIMIT_LIGHT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(27.8, 115.2)), module, Blender::OUT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(38.4, 115.2)), module, Blender::OUT_OUTPUT+1));
	}

	void appendContextMenu(Menu *menu) override {
	   	Blender *module = dynamic_cast<Blender*>(this->module);
		assert(module);
		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolMenuItem("Polyphonic OUTs", "", [=]() {
				return module->isPolyOuts();
			}, [=](bool poly) {
				module->setPolyOuts(poly);
		}));
	}
};

Model* modelBlender = createModel<Blender, BlenderWidget>("Blender");