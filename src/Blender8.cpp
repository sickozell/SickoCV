#include "plugin.hpp"

struct Blender8 : Module {
	float mixedOut = 0;
	float mix = 0;
	float input2 = 0;

	enum ParamId {
		ENUMS(PHASE_SWITCH,8),
		ENUMS(MIX_PARAMS,8),
		ENUMS(RANGEMODMIX_SWITCH,8),
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(IN1_INPUT,8),
		ENUMS(IN2_INPUT,8),
		ENUMS(MODMIXCV_INPUT,8),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT,8),
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Blender8() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(IN1_INPUT, "In1 #1");
		configInput(IN1_INPUT+1, "In1 #2");
		configInput(IN1_INPUT+2, "In1 #3");
		configInput(IN1_INPUT+3, "In1 #4");
		configInput(IN1_INPUT+4, "In1 #5");
		configInput(IN1_INPUT+5, "In1 #6");
		configInput(IN1_INPUT+6, "In1 #7");
		configInput(IN1_INPUT+7, "In1 #8");

		configInput(IN2_INPUT, "In2 #1");
		configInput(IN2_INPUT+1, "In2 #2");
		configInput(IN2_INPUT+2, "In2 #3");
		configInput(IN2_INPUT+3, "In2 #4");
		configInput(IN2_INPUT+4, "In2 #5");
		configInput(IN2_INPUT+5, "In2 #6");
		configInput(IN2_INPUT+6, "In2 #7");
		configInput(IN2_INPUT+7, "In2 #8");
		configSwitch(PHASE_SWITCH, 0.f, 1.f, 0.f, "In2 #1 Phase", {"Normal", "Inverted"});
		configSwitch(PHASE_SWITCH+1, 0.f, 1.f, 0.f, "In2 #2 Phase", {"Normal", "Inverted"});
		configSwitch(PHASE_SWITCH+2, 0.f, 1.f, 0.f, "In2 #3 Phase", {"Normal", "Inverted"});
		configSwitch(PHASE_SWITCH+3, 0.f, 1.f, 0.f, "In2 #4 Phase", {"Normal", "Inverted"});
		configSwitch(PHASE_SWITCH+4, 0.f, 1.f, 0.f, "In2 #5 Phase", {"Normal", "Inverted"});
		configSwitch(PHASE_SWITCH+5, 0.f, 1.f, 0.f, "In2 #6 Phase", {"Normal", "Inverted"});
		configSwitch(PHASE_SWITCH+6, 0.f, 1.f, 0.f, "In2 #7 Phase", {"Normal", "Inverted"});
		configSwitch(PHASE_SWITCH+7, 0.f, 1.f, 0.f, "In2 #8 Phase", {"Normal", "Inverted"});

		configParam(MIX_PARAMS, -1.f,1.f, 0.f, "Mix #1", "%", 0, 100);
		configParam(MIX_PARAMS+1, -1.f,1.f, 0.f, "Mix #2", "%", 0, 100);
		configParam(MIX_PARAMS+2, -1.f,1.f, 0.f, "Mix #3", "%", 0, 100);
		configParam(MIX_PARAMS+3, -1.f,1.f, 0.f, "Mix #4", "%", 0, 100);
		configParam(MIX_PARAMS+4, -1.f,1.f, 0.f, "Mix #5", "%", 0, 100);
		configParam(MIX_PARAMS+5, -1.f,1.f, 0.f, "Mix #6", "%", 0, 100);
		configParam(MIX_PARAMS+6, -1.f,1.f, 0.f, "Mix #7", "%", 0, 100);
		configParam(MIX_PARAMS+7, -1.f,1.f, 0.f, "Mix #8", "%", 0, 100);
		configInput(MODMIXCV_INPUT, "Mix Mod #1 Cv");
		configInput(MODMIXCV_INPUT+1, "Mix Mod #2 Cv");
		configInput(MODMIXCV_INPUT+2, "Mix Mod #3 Cv");
		configInput(MODMIXCV_INPUT+3, "Mix Mod #4 Cv");
		configInput(MODMIXCV_INPUT+4, "Mix Mod #5 Cv");
		configInput(MODMIXCV_INPUT+5, "Mix Mod #6 Cv");
		configInput(MODMIXCV_INPUT+6, "Mix Mod #7 Cv");
		configInput(MODMIXCV_INPUT+7, "Mix Mod #8 Cv");
		configSwitch(RANGEMODMIX_SWITCH, 0.f, 1.f, 1.f, "Mix Mod Cv Input #1 Range", {"Bipolar", "Unipolar"});
		configSwitch(RANGEMODMIX_SWITCH+1, 0.f, 1.f, 1.f, "Mix Mod Cv Input #2 Range", {"Bipolar", "Unipolar"});
		configSwitch(RANGEMODMIX_SWITCH+2, 0.f, 1.f, 1.f, "Mix Mod Cv Input #3 Range", {"Bipolar", "Unipolar"});
		configSwitch(RANGEMODMIX_SWITCH+3, 0.f, 1.f, 1.f, "Mix Mod Cv Input #4 Range", {"Bipolar", "Unipolar"});
		configSwitch(RANGEMODMIX_SWITCH+4, 0.f, 1.f, 1.f, "Mix Mod Cv Input #5 Range", {"Bipolar", "Unipolar"});
		configSwitch(RANGEMODMIX_SWITCH+5, 0.f, 1.f, 1.f, "Mix Mod Cv Input #6 Range", {"Bipolar", "Unipolar"});
		configSwitch(RANGEMODMIX_SWITCH+6, 0.f, 1.f, 1.f, "Mix Mod Cv Input #7 Range", {"Bipolar", "Unipolar"});
		configSwitch(RANGEMODMIX_SWITCH+7, 0.f, 1.f, 1.f, "Mix Mod Cv Input #8 Range", {"Bipolar", "Unipolar"});

		configOutput(OUT_OUTPUT, "Out #1");
		configOutput(OUT_OUTPUT+1, "Out #2");
		configOutput(OUT_OUTPUT+2, "Out #3");
		configOutput(OUT_OUTPUT+3, "Out #4");
		configOutput(OUT_OUTPUT+4, "Out #5");
		configOutput(OUT_OUTPUT+5, "Out #6");
		configOutput(OUT_OUTPUT+6, "Out #7");
		configOutput(OUT_OUTPUT+7, "Out #8");

	}

	void process(const ProcessArgs& args) override {
		for (int i=0; i<8; i++) {
			if (outputs[OUT_OUTPUT+i].isConnected()) {
				if (inputs[MODMIXCV_INPUT+i].isConnected()) {
					if (params[RANGEMODMIX_SWITCH+i].getValue() == 1)
						mix = params[MIX_PARAMS+i].getValue() * inputs[MODMIXCV_INPUT+i].getVoltage() / 10;
					else
						mix = params[MIX_PARAMS+i].getValue() * (inputs[MODMIXCV_INPUT+i].getVoltage() + 5) / 10;
					if (mix > 1)
						mix = 1;
					else if (mix < 0) {
						mix = -mix;
						if (mix < -1)
							mix = -1;
					}
				} else {
					mix = (params[MIX_PARAMS+i].getValue() + 1)/2;
				}
				input2 = inputs[IN2_INPUT+i].getVoltage();
				if (params[PHASE_SWITCH+i].getValue() == 1)
					input2 = -input2;
				mixedOut = (inputs[IN1_INPUT+i].getVoltage() * (1 - mix)) + (input2 * mix);
			} else {
				mixedOut = 0;
			}
			outputs[OUT_OUTPUT+i].setVoltage(mixedOut);
		}
	}		
};

struct Blender8Widget : ModuleWidget {
	Blender8Widget(Blender8* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Blender8.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float y = 13;
		const float ys = 22;
		for (int i=0; i<8; i++) {
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(6.7, ys+(i*y))), module, Blender8::IN1_INPUT+i));
			
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(16.1, ys+(i*y))), module, Blender8::IN2_INPUT+i));
			addParam(createParamCentered<CKSS>(mm2px(Vec(23.4, ys+(i*y))), module, Blender8::PHASE_SWITCH+i));
			
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(33, ys+(i*y))), module, Blender8::MIX_PARAMS+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(42, ys+(i*y))), module, Blender8::MODMIXCV_INPUT+i));
			addParam(createParamCentered<CKSS>(mm2px(Vec(50, ys+(i*y))), module, Blender8::RANGEMODMIX_SWITCH+i));
			
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(59.3, ys+(i*y))), module, Blender8::OUT_OUTPUT+i));
		}
	}
};

Model* modelBlender8 = createModel<Blender8, Blender8Widget>("Blender8");