#include "plugin.hpp"


struct BtogglerPlus : Module {
	float clock = 0;
	float prevClock = 0;
	bool clockState = false;
	int gateState[8] = {0,0,0,0,0,0,0,0};
	int trigState[8] = {0,0,0,0,0,0,0,0};
	float rst[8] = {0,0,0,0,0,0,0,0};
	float prevRst[8] = {0,0,0,0,0,0,0,0};
	float rstAll = 0;
	float prevRstAll = 0;

	bool currentTrigState[8] = {false,false,false,false,false,false,false,false};
	float prevTrigState[8] = {0,0,0,0,0,0,0,0};
	int warnCounter[8] = {0,0,0,0,0,0,0,0};
	int warnInOn = 0;
	int warnInOff = 0;
	int warnOutOn = 0;
	int warnOutOff = 0;

	float maxFadeSample = 0;
	float currentFadeSample[8] = {0,0,0,0,0,0,0,0};
	bool fading[8] = {false,false,false,false,false,false,false,false};

	enum ParamId {
		FADE_PARAMS,
		WARNIN_PARAMS,
		WARNOUT_PARAMS,
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
		ENUMS(WARN_OUTPUT,8),
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	BtogglerPlus() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(CLOCK_INPUT, "Clock");
		configParam(FADE_PARAMS, 0.f, 50.f, 0.f, "Fade (ms)");
		configParam(WARNIN_PARAMS, 0.f, 200.f, 25.f, "Warn Attack pulse rate (ms)");
		configParam(WARNOUT_PARAMS, 0.f, 200.f, 25.f, "Warn Release pulse rate (ms)");
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
		configOutput(WARN_OUTPUT, "Warn #1");
		configOutput(WARN_OUTPUT+1, "Warn #2");
		configOutput(WARN_OUTPUT+2, "Warn #3");
		configOutput(WARN_OUTPUT+3, "Warn #4");
		configOutput(WARN_OUTPUT+4, "Warn #5");
		configOutput(WARN_OUTPUT+5, "Warn #6");
		configOutput(WARN_OUTPUT+6, "Warn #7");
		configOutput(WARN_OUTPUT+7, "Warn #8");
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
						warnCounter[i] = 0;
						outputs[WARN_OUTPUT+i].setVoltage(0);
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
				if (inputs[RST_INPUT+i].isConnected()){
					rst[i] = inputs[RST_INPUT+i].getVoltage();
					if (rst[i] > 0 && prevRst[i] <= 0) {
						// next lines are duplicated from case 3
						warnCounter[i] = 0;
						outputs[WARN_OUTPUT+i].setVoltage(0);
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
							if (currentTrigState[i]){					// if ARM occurs
								gateState[i] = 1;
								warnInOn = args.sampleRate / 2000 * params[WARNIN_PARAMS].getValue();
								warnInOff = warnInOn + warnInOn;
							} else if (params[FADE_PARAMS].getValue() != 0){ // if a FADE value is set
								if (fading[i] == true) {					// if it's currently fading
									if (currentFadeSample[i] > maxFadeSample) { // if FADING has reached end
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
						case 1: 									// ARMed ON, waiting for next clock
							if (currentTrigState[i]){						// if another ARM occurs, then abort
								outputs[WARN_OUTPUT+i].setVoltage(0);
								gateState[i] = 0;
							} else if (clockState) { 							// if clock occurs
								outputs[GATE_OUTPUT+i].setVoltage(10);
								outputs[WARN_OUTPUT+i].setVoltage(0);
								warnCounter[i] = 0;
								gateState[i] = 2;
								if (params[FADE_PARAMS].getValue() != 0){
									fading[i] = true;
									currentFadeSample[i] = 0;
									maxFadeSample = args.sampleRate / 1000 * params[FADE_PARAMS].getValue();
								}
							} else if (params[WARNIN_PARAMS].getValue() == 0) {// if clock has not reached and it's still warning
								outputs[WARN_OUTPUT+i].setVoltage(10);
							} else if (params[WARNIN_PARAMS].getValue() == 200){
								outputs[WARN_OUTPUT+i].setVoltage(0);
							} else {
								warnCounter[i]++;
								if (warnCounter[i] > warnInOff) {
									warnCounter[i] = 0;
								} else if (warnCounter[i] > warnInOn) {
									outputs[WARN_OUTPUT+i].setVoltage(0);
								} else {
									outputs[WARN_OUTPUT+i].setVoltage(10);
								}
							}
							break;
						case 2: 									// gating
							if (currentTrigState[i]) { 					// if ARMing occurs
								gateState[i] = 3;
								warnOutOn = args.sampleRate / 2000 * params[WARNOUT_PARAMS].getValue();
								warnOutOff = warnOutOn + warnOutOn;
							} else if (params[FADE_PARAMS].getValue() != 0){ // if it's currently GATING, if FADE is set
								if (fading[i] == true) {					// if is currently FADING
									outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage() * currentFadeSample[i] / maxFadeSample);
									outputs[WARN_OUTPUT+i].setVoltage(10);
									currentFadeSample[i]++;
									if (currentFadeSample[i] > maxFadeSample) { // if FADE has reached end
										fading[i] = false;
										currentFadeSample[i] = 0;
									}
								} else {									// if FADE is set and FADE ended
									outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage());
									outputs[WARN_OUTPUT+i].setVoltage(10); 
								}
							} else {								// if it's currently GATING and FADE IS NOT SET
								outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage());
								outputs[WARN_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage());
							}
							break;
						case 3: 									// gating and ARMed off, waiting for next clock
							if (currentTrigState[i]){					// if another ARM occurs, then abort
								gateState[i] = 2;
							} else if (clockState) {							// if clock occurs
								warnCounter[i] = 0;
								outputs[WARN_OUTPUT+i].setVoltage(0);
								outputs[GATE_OUTPUT+i].setVoltage(0);
								gateState[i] = 0;
								if (params[FADE_PARAMS].getValue() != 0){
									fading[i] = true;
									currentFadeSample[i] = 0;
									maxFadeSample = args.sampleRate / 1000 * params[FADE_PARAMS].getValue();
								}
							} else if (params[WARNOUT_PARAMS].getValue() == 0) { // if clock is not reached
								outputs[WARN_OUTPUT+i].setVoltage(10);
							} else if (params[WARNOUT_PARAMS].getValue() == 200){
								outputs[WARN_OUTPUT+i].setVoltage(0);
							} else {
								warnCounter[i]++;
								if (warnCounter[i] > warnOutOff) {
									warnCounter[i] = 0;
								} else if (warnCounter[i] > warnOutOn) {
									outputs[WARN_OUTPUT+i].setVoltage(0);
								} else {
									outputs[WARN_OUTPUT+i].setVoltage(10);
								}
							}
							outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage());
							break;
					}
				} 
			}
			clockState = false;
		}
	}
};


struct BtogglerPlusWidget : ModuleWidget {
	BtogglerPlusWidget(BtogglerPlus* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/BtogglerPlus.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH * 2, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH * 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.7, 21)), module, BtogglerPlus::CLOCK_INPUT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.8, 21)), module, BtogglerPlus::FADE_PARAMS));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(39.2, 17)), module, BtogglerPlus::WARNIN_PARAMS));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(39.2, 25)), module, BtogglerPlus::WARNOUT_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(49.2, 21)), module, BtogglerPlus::RSTALL_INPUT));

		float x = 8.9f;
		float y = 10.5f;
		for (int i=0;i<8;i++) {
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.7, 42+(i*y))), module, BtogglerPlus::ARM_INPUT+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.7+x, 42+(i*y))), module, BtogglerPlus::IN_INPUT+i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.7+(2*x), 42+(i*y))), module, BtogglerPlus::OUT_OUTPUT+i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.7+(3*x), 42+(i*y))), module, BtogglerPlus::GATE_OUTPUT+i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.7+(4*x), 42+(i*y))), module, BtogglerPlus::WARN_OUTPUT+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.7+(5*x), 42+(i*y))), module, BtogglerPlus::RST_INPUT+i));
		}
	}
};


Model* modelBtogglerPlus = createModel<BtogglerPlus, BtogglerPlusWidget>("BtogglerPlus");