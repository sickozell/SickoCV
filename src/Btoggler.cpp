#include "plugin.hpp"


struct Btoggler : Module {
	float clock = 0;
	float prevClock = 0;
	bool clockState = false;
	int gateState[8] = {0,0,0,0,0,0,0,0};
	int trigState[8] = {0,0,0,0,0,0,0,0};
	bool currentTrigState[8] = {false,false,false,false,false,false,false,false};
	float prevTrigState[8] = {0,0,0,0,0,0,0,0};
	
	float rst[8] = {0,0,0,0,0,0,0,0};
	float prevRst[8] = {0,0,0,0,0,0,0,0};
	float rstAll = 0;
	float prevRstAll = 0;
	
	float maxFadeSample = 0;
	float currentFadeSample[8] = {0,0,0,0,0,0,0,0};
	bool fading[8] = {false,false,false,false,false,false,false,false};

	enum ParamId {
		FADE_PARAMS,
		PARAMS_LEN
	};
	enum InputId {
		CLOCK_INPUT,
		RSTALL_INPUT,
		ENUMS(ARM_INPUT,8),
		ENUMS(IN_INPUT,8),
		ENUMS(RST_INPUT,8),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT,8),
		ENUMS(GATE_OUTPUT,8),
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Btoggler() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(CLOCK_INPUT, "Clock");
		configParam(FADE_PARAMS, 0.f, 50.f, 0.f, "Fade (ms)");
		configInput(RSTALL_INPUT, "Reset all toggles");
		configInput(ARM_INPUT, "Arm #1");
		configInput(ARM_INPUT+1, "Arm #2");
		configInput(ARM_INPUT+2, "Arm #3");
		configInput(ARM_INPUT+3, "Arm #4");
		configInput(ARM_INPUT+4, "Arm #5");
		configInput(ARM_INPUT+5, "Arm #6");
		configInput(ARM_INPUT+6, "Arm #7");
		configInput(ARM_INPUT+7, "Arm #8");
		configInput(IN_INPUT, "Input #1");
		configInput(IN_INPUT+1, "Input #2");
		configInput(IN_INPUT+2, "Input #3");
		configInput(IN_INPUT+3, "Input #4");
		configInput(IN_INPUT+4, "Input #5");
		configInput(IN_INPUT+5, "Input #6");
		configInput(IN_INPUT+6, "Input #7");
		configInput(IN_INPUT+7, "Input #8");
		configOutput(OUT_OUTPUT, "Output #1");
		configOutput(OUT_OUTPUT+1, "Output #2");
		configOutput(OUT_OUTPUT+2, "Output #3");
		configOutput(OUT_OUTPUT+3, "Output #4");
		configOutput(OUT_OUTPUT+4, "Output #5");
		configOutput(OUT_OUTPUT+5, "Output #6");
		configOutput(OUT_OUTPUT+6, "Output #7");
		configOutput(OUT_OUTPUT+7, "Output #8");
		configOutput(GATE_OUTPUT, "Gate #1");
		configOutput(GATE_OUTPUT+1, "Gate #2");
		configOutput(GATE_OUTPUT+2, "Gate #3");
		configOutput(GATE_OUTPUT+3, "Gate #4");
		configOutput(GATE_OUTPUT+4, "Gate #5");
		configOutput(GATE_OUTPUT+5, "Gate #6");
		configOutput(GATE_OUTPUT+6, "Gate #7");
		configOutput(GATE_OUTPUT+7, "Gate #8");
		configInput(RST_INPUT, "Reset #1");
		configInput(RST_INPUT+1, "Reset #2");
		configInput(RST_INPUT+2, "Reset #3");
		configInput(RST_INPUT+3, "Reset #4");
		configInput(RST_INPUT+4, "Reset #5");
		configInput(RST_INPUT+5, "Reset #6");
		configInput(RST_INPUT+6, "Reset #7");
		configInput(RST_INPUT+7, "Reset #8");
	}

	void process(const ProcessArgs& args) override {
		if (inputs[CLOCK_INPUT].isConnected()){
			clock = inputs[CLOCK_INPUT].getVoltage();
			if (clock > 0 && prevClock <= 0) {
				clockState = true;
			} else {
				clockState = false;
			}
			prevClock = clock;
		
			if (inputs[RSTALL_INPUT].isConnected()){
				rstAll = inputs[RSTALL_INPUT].getVoltage();
				if (rstAll > 0 && prevRstAll <= 0) {
					for (int i=0; i<8;i++){
						// next lines are duplicated from case 3
						outputs[GATE_OUTPUT+i].setVoltage(0);
						// below is different from original: if gateState is 0 or 1
						// it will not do the fade 
						if (params[FADE_PARAMS].getValue() != 0 && gateState[i] > 1){
							fading[i] = true;
							currentFadeSample[i] = 0;
							maxFadeSample = args.sampleRate / 1000 * params[FADE_PARAMS].getValue();
						}
						gateState[i] = 0;
						// end of duplicated lines
					}
				}
				prevRstAll = rstAll; 
			}

			for (int i=0; i<8;i++){
				if (outputs[OUT_OUTPUT+i].isConnected() || outputs[GATE_OUTPUT+i+i].isConnected()){
					if (inputs[RST_INPUT+i].isConnected()){
						rst[i] = inputs[RST_INPUT+i].getVoltage();
						if (rst[i] > 0 && prevRst[i] <= 0) {
							// next lines are duplicated from case 3
							outputs[GATE_OUTPUT+i].setVoltage(0);
							// below is different from original: if gateState is 0 or 1
							// it will not do the fade 
							if (params[FADE_PARAMS].getValue() != 0 && gateState[i] > 1){
								fading[i] = true;
								currentFadeSample[i] = 0;
								maxFadeSample = args.sampleRate / 1000 * params[FADE_PARAMS].getValue();
							}
							gateState[i] = 0;
							// end of duplicated lines
						}
						prevRst[i] = rst[i]; 
					}
					if (inputs[ARM_INPUT+i].isConnected()){
						trigState[i] = inputs[ARM_INPUT+i].getVoltage();
						if (trigState[i] > 0 && prevTrigState[i] <= 0){
							currentTrigState[i] = true;
						} else {
							currentTrigState[i] = false;
						}
						prevTrigState[i] = trigState[i];

						switch (gateState[i]) {
							case 0: 									// waiting for ARM
								if (currentTrigState[i]){					// if occurs
									gateState[i] = 1;
								} else if (params[FADE_PARAMS].getValue() != 0){
									if (fading[i] == true) {
										if (currentFadeSample[i] > maxFadeSample) {
											fading[i] = false;
											currentFadeSample[i] = 0;
											outputs[OUT_OUTPUT+i].setVoltage(0);
									
										} else {
											outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage() * (1-(currentFadeSample[i] / maxFadeSample)));
										}
										currentFadeSample[i]++;
									} else {									// if FADING has ended
										outputs[OUT_OUTPUT+i].setVoltage(0);
									}
								} else {  // if FADE has not set
									outputs[OUT_OUTPUT+i].setVoltage(0);
								}
								break;
							case 1: 									// triggered waiting for next clock
								if (currentTrigState[i]){						// if another trigger occurs, then abort
									gateState[i] = 0;
								} else if (clockState) { 							// if clock occurs
									outputs[GATE_OUTPUT+i].setVoltage(10);
									gateState[i] = 2;
									if (params[FADE_PARAMS].getValue() != 0){
										fading[i] = true;
										currentFadeSample[i] = 0;
										maxFadeSample = args.sampleRate / 1000 * params[FADE_PARAMS].getValue();
									}
								}
								break;
							case 2: 									// gating
								if (currentTrigState[i]) { 					// if ARM occurs
									gateState[i] = 3;
								} else if (params[FADE_PARAMS].getValue() != 0){
									if (fading[i] == true) {
										outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage() * currentFadeSample[i] / maxFadeSample);
										currentFadeSample[i]++;
										if (currentFadeSample[i] > maxFadeSample) {
											fading[i] = false;
											currentFadeSample[i] = 0;
										}
									} else {
										outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage());
										outputs[GATE_OUTPUT+i].setVoltage(10);
									}
								} else {
									outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage());
									outputs[GATE_OUTPUT+i].setVoltage(10);
								}
								break;
							case 3: 									// gating and triggered, waiting for next clock
								if (currentTrigState[i]){					// if another trigger occurs, then abort
									gateState[i] = 2;
								} else if (clockState) {
									outputs[GATE_OUTPUT+i].setVoltage(0);
									gateState[i] = 0;
									if (params[FADE_PARAMS].getValue() != 0){
										fading[i] = true;
										currentFadeSample[i] = 0;
										maxFadeSample = args.sampleRate / 1000 * params[FADE_PARAMS].getValue();
									}
								} else {
									outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage());
									outputs[GATE_OUTPUT+i].setVoltage(10);
								}
								break;
						}
					}
				}
			}
			clockState = false;
		}
	}
};


struct BtogglerWidget : ModuleWidget {
	BtogglerWidget(Btoggler* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Btoggler.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7, 21.6)), module, Btoggler::CLOCK_INPUT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(23, 21.6)), module, Btoggler::FADE_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.7, 21.6)), module, Btoggler::RSTALL_INPUT));
		
		float x = 8.9;
		float y = 10.8;
		for (int i=0;i<8;i++) {
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.1, 41+(i*y))), module, Btoggler::ARM_INPUT+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.1+x, 41+(i*y))), module, Btoggler::IN_INPUT+i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.1+(2*x), 41+(i*y))), module, Btoggler::OUT_OUTPUT+i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.1+(3*x), 41+(i*y))), module, Btoggler::GATE_OUTPUT+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.1+(4*x), 41+(i*y))), module, Btoggler::RST_INPUT+i));
		}
	}
};


Model* modelBtoggler = createModel<Btoggler, BtogglerWidget>("Btoggler");