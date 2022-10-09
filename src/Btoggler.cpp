#include "plugin.hpp"

struct Btoggler : Module {
	bool stateRestore = true;
	int clockConection = 0;
	bool clockState = false;
	float clock = 0;
	float prevClock = 0;

	int internalState[8] = {0,0,0,0,0,0,0,0};
	bool trigState[8] = {false,false,false,false,false,false,false,false};
	float trigValue[8] = {0,0,0,0,0,0,0,0};
	float prevTrigValue[8] = {0,0,0,0,0,0,0,0};
	
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
		ENUMS(WRN_LIGHT,8),
		ENUMS(OUT_LIGHT,8),
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
	
	void onReset() override {
		stateRestore = true;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "StateRestore", json_boolean(stateRestore));
		json_object_set_new(rootJ, "State1", json_integer(internalState[0]));
		json_object_set_new(rootJ, "State2", json_integer(internalState[1]));
		json_object_set_new(rootJ, "State3", json_integer(internalState[2]));
		json_object_set_new(rootJ, "State4", json_integer(internalState[3]));
		json_object_set_new(rootJ, "State5", json_integer(internalState[4]));
		json_object_set_new(rootJ, "State6", json_integer(internalState[5]));
		json_object_set_new(rootJ, "State7", json_integer(internalState[6]));
		json_object_set_new(rootJ, "State8", json_integer(internalState[7]));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* jsonStateRestore = json_object_get(rootJ, "StateRestore");
		if (jsonStateRestore)
			stateRestore = json_boolean_value(jsonStateRestore);

		if (stateRestore) {
			json_t* jsonState1 = json_object_get(rootJ, "State1");
			if (jsonState1) {
				internalState[0] = json_integer_value(jsonState1);
				if (internalState[0])
					lights[OUT_LIGHT].setBrightness(1.f);
			}

			json_t* jsonState2 = json_object_get(rootJ, "State2");
			if (jsonState2) {
				internalState[1] = json_integer_value(jsonState2);
				if (internalState[1])
					lights[OUT_LIGHT+1].setBrightness(1.f);
			}

			json_t* jsonState3 = json_object_get(rootJ, "State3");
			if (jsonState3) {
				internalState[2] = json_integer_value(jsonState3);
				if (internalState[2])
					lights[OUT_LIGHT+2].setBrightness(1.f);
			}

			json_t* jsonState4 = json_object_get(rootJ, "State4");
			if (jsonState4) {
				internalState[3] = json_integer_value(jsonState4);
				if (internalState[3])
					lights[OUT_LIGHT+3].setBrightness(1.f);
			}

			json_t* jsonState5 = json_object_get(rootJ, "State5");
			if (jsonState5) {
				internalState[4] = json_integer_value(jsonState5);
				if (internalState[4])
					lights[OUT_LIGHT+4].setBrightness(1.f);
			}

			json_t* jsonState6 = json_object_get(rootJ, "State6");
			if (jsonState6) {
				internalState[5] = json_integer_value(jsonState6);
				if (internalState[5])
					lights[OUT_LIGHT+5].setBrightness(1.f);
			}

			json_t* jsonState7 = json_object_get(rootJ, "State7");
			if (jsonState7) {
				internalState[6] = json_integer_value(jsonState7);
				if (internalState[6])
					lights[OUT_LIGHT+6].setBrightness(1.f);
			}

			json_t* jsonState8 = json_object_get(rootJ, "State8");
			if (jsonState8) {
				internalState[7] = json_integer_value(jsonState8);
				if (internalState[7])
					lights[OUT_LIGHT+7].setBrightness(1.f);
			}
		}
	}

	void process(const ProcessArgs& args) override {
		if (inputs[CLOCK_INPUT].isConnected()){
			clockConection = 1;
		} else {
			clockConection = 0;
		}

		switch (clockConection) {
			case 1:
				clock = inputs[CLOCK_INPUT].getVoltage();
				if (clock >= 1 && prevClock < 1) {
					clockState = true;
				} else {
					clockState = false;
				}
				prevClock = clock;
			
				if (inputs[RSTALL_INPUT].isConnected()){
					rstAll = inputs[RSTALL_INPUT].getVoltage();
					if (rstAll >= 1 && prevRstAll < 1) {
						for (int i=0; i<8;i++){
							// next lines are duplicated from case 3
							outputs[GATE_OUTPUT+i].setVoltage(0);
							lights[OUT_LIGHT+i].setBrightness(0.f);
							lights[WRN_LIGHT+i].setBrightness(0.f);
							// below is different from original: if internalState is 0 or 1
							// it will not do the fade 
							if (params[FADE_PARAMS].getValue() != 0 && internalState[i] > 1){
								fading[i] = true;
								currentFadeSample[i] = 0;
								maxFadeSample = args.sampleRate / 1000 * params[FADE_PARAMS].getValue();
							}
							internalState[i] = 0;
							// end of duplicated lines
						}
					}
					prevRstAll = rstAll; 
				}

				for (int i=0; i<8;i++){
					if (inputs[RST_INPUT+i].isConnected()){
						rst[i] = inputs[RST_INPUT+i].getVoltage();
						if (rst[i] >= 1 && prevRst[i] < 1) {
							// next lines are duplicated from case 3
							outputs[GATE_OUTPUT+i].setVoltage(0);
							lights[OUT_LIGHT+i].setBrightness(0.f);
							lights[WRN_LIGHT+i].setBrightness(0.f);
							// below is different from original: if internalState is 0 or 1
							// it will not do the fade 
							if (params[FADE_PARAMS].getValue() != 0 && internalState[i] > 1){
								fading[i] = true;
								currentFadeSample[i] = 0;
								maxFadeSample = args.sampleRate / 1000 * params[FADE_PARAMS].getValue();
							}
							internalState[i] = 0;
							// end of duplicated lines
						}
						prevRst[i] = rst[i]; 
					}

					if (inputs[ARM_INPUT+i].isConnected()){
						trigValue[i] = inputs[ARM_INPUT+i].getVoltage();
						if (trigValue[i] >= 1 && prevTrigValue[i] < 1){
							trigState[i] = true;
						} else {
							trigState[i] = false;
						}
						prevTrigValue[i] = trigValue[i];

						switch (internalState[i]) {
							case 0: 									// waiting for ARM
								if (trigState[i]){					// if occurs
									internalState[i] = 1;
									lights[WRN_LIGHT+i].setBrightness(1.f);
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
								if (trigState[i]){						// if another trigger occurs, then abort
									lights[WRN_LIGHT+i].setBrightness(0.f);
									internalState[i] = 0;
								} else if (clockState) { 							// if clock occurs
									outputs[GATE_OUTPUT+i].setVoltage(10);
									lights[OUT_LIGHT+i].setBrightness(1.f);
									lights[WRN_LIGHT+i].setBrightness(0.f);
									internalState[i] = 2;
									if (params[FADE_PARAMS].getValue() != 0){
										fading[i] = true;
										currentFadeSample[i] = 0;
										maxFadeSample = args.sampleRate / 1000 * params[FADE_PARAMS].getValue();
									}
								}
							break;

							case 2: 									// gating
								if (trigState[i]) { 					// if ARM occurs
									internalState[i] = 3;
									lights[WRN_LIGHT+i].setBrightness(1.f);
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
								if (trigState[i]){					// if another trigger occurs, then abort
									internalState[i] = 2;
									lights[WRN_LIGHT+i].setBrightness(0.f);
								} else if (clockState) {
									outputs[GATE_OUTPUT+i].setVoltage(0);
									internalState[i] = 0;
									lights[OUT_LIGHT+i].setBrightness(0.f);
									lights[WRN_LIGHT+i].setBrightness(0.f);
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
					} else {	// if ARM INPUT is not connected
						internalState[i] = 0;
						outputs[OUT_OUTPUT+i].setVoltage(0);
						outputs[GATE_OUTPUT+i].setVoltage(0);
						lights[OUT_LIGHT+i].setBrightness(0.f);
						lights[WRN_LIGHT+i].setBrightness(0.f);
					}
				}
				clockState = false;
			break;

			default: // ******************* if clock is not connected

				if (inputs[RSTALL_INPUT].isConnected()){
					rstAll = inputs[RSTALL_INPUT].getVoltage();
					if (rstAll >= 1 && prevRstAll < 1) {
						for (int i=0; i<8;i++){
							// next lines are duplicated from case 3
							outputs[GATE_OUTPUT+i].setVoltage(0);
							lights[OUT_LIGHT+i].setBrightness(0.f);
							lights[WRN_LIGHT+i].setBrightness(0.f);
							// below is different from original: if internalState is 0 or 1
							// it will not do the fade 
							if (params[FADE_PARAMS].getValue() != 0 && internalState[i] > 1){
								fading[i] = true;
								currentFadeSample[i] = 0;
								maxFadeSample = args.sampleRate / 1000 * params[FADE_PARAMS].getValue();
							}
							internalState[i] = 0;
							// end of duplicated lines
						}
					}
					prevRstAll = rstAll; 
				}

				for (int i=0; i<8;i++){
					if (inputs[RST_INPUT+i].isConnected()){
						rst[i] = inputs[RST_INPUT+i].getVoltage();
						if (rst[i] >= 1 && prevRst[i] < 1) {
							// next lines are duplicated from case 3
							outputs[GATE_OUTPUT+i].setVoltage(0);
							lights[OUT_LIGHT+i].setBrightness(0.f);
							lights[WRN_LIGHT+i].setBrightness(0.f);
							// below is different from original: if internalState is 0 or 1
							// it will not do the fade 
							if (params[FADE_PARAMS].getValue() != 0 && internalState[i] > 1){
								fading[i] = true;
								currentFadeSample[i] = 0;
								maxFadeSample = args.sampleRate / 1000 * params[FADE_PARAMS].getValue();
							}
							internalState[i] = 0;
							// end of duplicated lines
						}
						prevRst[i] = rst[i]; 
					}

					if (inputs[ARM_INPUT+i].isConnected()){
						trigValue[i] = inputs[ARM_INPUT+i].getVoltage();
						if (trigValue[i] >= 1 && prevTrigValue[i] < 1){
							trigState[i] = true;
						} else {
							trigState[i] = false;
						}
						prevTrigValue[i] = trigValue[i];

						switch (internalState[i]) {
							case 0: 									// waiting for ARM
								if (trigState[i]){					// if occurs
									outputs[GATE_OUTPUT+i].setVoltage(10);
									lights[OUT_LIGHT+i].setBrightness(1.f);
									lights[WRN_LIGHT+i].setBrightness(0.f);
									internalState[i] = 2;
									if (params[FADE_PARAMS].getValue() != 0){
										fading[i] = true;
										currentFadeSample[i] = 0;
										maxFadeSample = args.sampleRate / 1000 * params[FADE_PARAMS].getValue();
									}
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

							case 2: 									// gating
								if (trigState[i]) { 					// if ARM occurs
									internalState[i] = 0;
									outputs[GATE_OUTPUT+i].setVoltage(0);
									lights[OUT_LIGHT+i].setBrightness(0.f);
									lights[WRN_LIGHT+i].setBrightness(0.f);
									if (params[FADE_PARAMS].getValue() != 0){
										fading[i] = true;
										currentFadeSample[i] = 0;
										maxFadeSample = args.sampleRate / 1000 * params[FADE_PARAMS].getValue();
									}
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

							default: 
								internalState[i] = 0;
								lights[OUT_LIGHT+i].setBrightness(0.f);
								lights[WRN_LIGHT+i].setBrightness(0.f);
						}
					} else {	// if ARM INPUT is not connected
						internalState[i] = 0;
						outputs[OUT_OUTPUT+i].setVoltage(0);
						outputs[GATE_OUTPUT+i].setVoltage(0);
						lights[OUT_LIGHT+i].setBrightness(0.f);
						lights[WRN_LIGHT+i].setBrightness(0.f);
					}
				}
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
			addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(18.45, 37.7+(i*y))), module, Btoggler::WRN_LIGHT+i));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(27.3, 37.7+(i*y))), module, Btoggler::OUT_LIGHT+i));
		}
	}

	void appendContextMenu(Menu* menu) override {
		Btoggler* module = dynamic_cast<Btoggler*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Restore State on Load", "", &module->stateRestore));
	}
};

Model* modelBtoggler = createModel<Btoggler, BtogglerWidget>("Btoggler");