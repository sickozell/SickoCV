#include "plugin.hpp"

struct Blender : Module {
	float mixedOut[2] = {0,0};
	float mix = 0;
	float modAtten = 0;
	float input1[2] = {0,0};
	float input2[2] = {0,0};

	enum ParamId {
		ENUMS(PHASE_SWITCH,2),
		MIX_PARAMS,
		MODMIX_PARAMS,
		MODATTEN_PARAMS,
		RANGEMODMIX_SWITCH,
		RANGEMODATTEN_SWITCH,
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(IN1_INPUT,2),
		ENUMS(IN2_INPUT,2),
		MODMIXCV_INPUT,
		MODATTENCV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT,2),
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Blender() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(IN1_INPUT, "In1 L");
		configInput(IN1_INPUT+1, "In1 R");
		configInput(IN2_INPUT, "In2 L");
		configInput(IN2_INPUT+1, "In2 R");

		configSwitch(PHASE_SWITCH, 0.f, 1.f, 0.f, "In1 Phase", {"Normal", "Inverted"});
		configSwitch(PHASE_SWITCH+1, 0.f, 1.f, 0.f, "In2 Phase", {"Normal", "Inverted"});

		configParam(MIX_PARAMS, 0.f,1.f, 0.5f, "Mix", "%", 0, 100);

		configParam(MODMIX_PARAMS, -1.f,1.f, 0.f, "Mix Mod", "%", 0, 100);
		configInput(MODMIXCV_INPUT, "Mix Mod Cv");
		configSwitch(RANGEMODMIX_SWITCH, 0.f, 1.f, 1.f, "Mix Mod Cv Input Range", {"Bipolar", "Unipolar"});

		configParam(MODATTEN_PARAMS, -1.f,1.f, 0.f, "MixMod Attenuv. Mod", "%", 0, 100);
		configInput(MODATTENCV_INPUT, "MixMod Attenuv. Mod Cv");
		configSwitch(RANGEMODATTEN_SWITCH, 0.f, 1.f, 1.f, "MixMod Attenuv. Mod Cv Input Range", {"Bipolar", "Unipolar"});

		configOutput(OUT_OUTPUT, "L");
		configOutput(OUT_OUTPUT+1, "R");
	}

	void process(const ProcessArgs& args) override {
		if (outputs[OUT_OUTPUT].isConnected() || outputs[OUT_OUTPUT+1].isConnected()){
			if (inputs[MODMIXCV_INPUT].isConnected()){				
				if (inputs[MODATTENCV_INPUT].isConnected()){ 
					if (params[RANGEMODATTEN_SWITCH].getValue() == 1) {
						modAtten = params[MODATTEN_PARAMS].getValue() * inputs[MODATTENCV_INPUT].getVoltage() / 10;
					} else {
						modAtten = params[MODATTEN_PARAMS].getValue() * (inputs[MODATTENCV_INPUT].getVoltage() + 5) / 10;
					}
				} else {
					modAtten = 0;
				}
				if (modAtten > 1) {
					modAtten = 1;
				} else if (modAtten < -1) {
					modAtten = -1;
				}
				if (params[RANGEMODMIX_SWITCH].getValue() == 1) {
					mix = params[MIX_PARAMS].getValue() + ( (params[MODMIX_PARAMS].getValue() * inputs[MODMIXCV_INPUT].getVoltage() / 10) + modAtten);
				} else {
					mix = params[MIX_PARAMS].getValue() + ( (params[MODMIX_PARAMS].getValue() * (inputs[MODMIXCV_INPUT].getVoltage() + 5) / 10) + modAtten);
				}
				if (mix > 1) {
					mix = 1;
				} else if (mix < 0) {
					mix = -mix;
					if (mix < -1) {
						mix = -1;
					}
				}
			} else {
				mix = params[MIX_PARAMS].getValue();
			}
			
			for (int i=0; i<2; i++){
				if (outputs[OUT_OUTPUT+i].isConnected()){
					input1[i] = inputs[IN1_INPUT+i].getVoltage();
					if (params[PHASE_SWITCH].getValue() == 1){
						input1[i] = -input1[i];
					}
					input2[i] = inputs[IN2_INPUT+i].getVoltage();
					if (params[PHASE_SWITCH+1].getValue() == 1){
						input2[i] = -input2[i];
					}
					mixedOut[i] = (input1[i] * (1 - mix)) + (input2[i] * mix);
				} else {
					mixedOut[i] = 0;
				}
			}			
		} else {
			mixedOut[0] = 0;
			mixedOut[1] = 0;
		}
		outputs[OUT_OUTPUT].setVoltage(mixedOut[0]);
		outputs[OUT_OUTPUT+1].setVoltage(mixedOut[1]);
	}		
};

struct BlenderWidget : ModuleWidget {
	BlenderWidget(Blender* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Blender.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7, 25)), module, Blender::IN1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.5, 25)), module, Blender::IN1_INPUT+1));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(29.55, 25)), module, Blender::IN2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.95, 25)), module, Blender::IN2_INPUT+1));
		addParam(createParamCentered<CKSS>(mm2px(Vec(11.75, 34.6)), module, Blender::PHASE_SWITCH));
		addParam(createParamCentered<CKSS>(mm2px(Vec(34.1, 34.6)), module, Blender::PHASE_SWITCH+1));

		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(22.86, 57)), module, Blender::MIX_PARAMS));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(11.9, 83.7)), module, Blender::MODMIX_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(26, 83.7)), module, Blender::MODMIXCV_INPUT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(35, 83.7)), module, Blender::RANGEMODMIX_SWITCH));
		
		addParam(createParamCentered<Trimpot>(mm2px(Vec(11.75, 108.5)), module, Blender::MODATTEN_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7, 116.2)), module, Blender::MODATTENCV_INPUT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(15., 116.2)), module, Blender::RANGEMODATTEN_SWITCH));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(29, 115.2)), module, Blender::OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.2, 115.2)), module, Blender::OUT_OUTPUT+1));
	}
};

Model* modelBlender = createModel<Blender, BlenderWidget>("Blender");