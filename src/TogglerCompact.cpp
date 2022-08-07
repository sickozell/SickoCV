#include "plugin.hpp"


struct TogglerCompact : Module {
	int mode = 1;
	int gateState = 0;
	int trigState = 0;
	float rst = 0;
	float prevRst = 0;

	bool currentTrigState = false;
	float prevTrigState = 0;

	float arSum = 0;
	float maxFadeSample = 0;
	float currentFadeSample = 0;
	bool fading = false;
	float sustain = 1;
	float startFade = 0;
	float lastFade = 0;
	
	enum ParamId {
		MODE_SWITCH,
		ATTACK_PARAMS,
		SUSTAIN_PARAMS,
		RELEASE_PARAMS,
		PARAMS_LEN
	};
	enum InputId {
		TRIG_INPUT,
		RST_INPUT,
		IN_INPUT,
		RIN_INPUT,
		ATTACK_INPUT,
		SUSTAIN_INPUT,
		RELEASE_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		ROUT_OUTPUT,
		GATE_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	TogglerCompact() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(MODE_SWITCH, 0.f, 1.f, 1.f, "MODE: FlipFlop/Gate");
		configParam(ATTACK_PARAMS, 0.f, 10.f, 0.f, "Attack (s)");
		configParam(SUSTAIN_PARAMS, 0.f, 1.f, 1.f, "Level", "%", 0, 100);
		configParam(RELEASE_PARAMS, 0.f, 10.f, 0.f, "Release (s)");
		configInput(TRIG_INPUT, "Trigger");
		configInput(RST_INPUT, "Reset");
		configInput(IN_INPUT, "L In signal");
		configInput(RIN_INPUT, "R In signal");
		configInput(ATTACK_INPUT, "Attack CV");
		configInput(SUSTAIN_INPUT, "Sustain CV");
		configInput(RELEASE_INPUT, "Release CV");
		configOutput(OUT_OUTPUT, "L Out Signal");
		configOutput(ROUT_OUTPUT, "R Out Signal");
		configOutput(GATE_OUTPUT, "Gate");
	}

	void process(const ProcessArgs& args) override {
		mode = params[MODE_SWITCH].getValue();
		switch (mode) {
			// ************************************** GATE MODE **********
			case 0:
				if (inputs[TRIG_INPUT].isConnected()){
					trigState = inputs[TRIG_INPUT].getVoltage();
					if (trigState > 0 && prevTrigState <= 0){
						currentTrigState = true;
					} else if (trigState <= 0 && prevTrigState > 0){
						currentTrigState = false;
					}
					prevTrigState = trigState;

					switch (gateState) {
						case 0: 									// waiting for TRIG
							if (currentTrigState){					// if GATE goes HIGH
								outputs[GATE_OUTPUT].setVoltage(10);
								gateState = 1;
								if (params[ATTACK_PARAMS].getValue() != 0 || inputs[ATTACK_INPUT].getVoltage() != 0) {
									if (fading) {
										startFade = lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							} else if (params[RELEASE_PARAMS].getValue() != 0 || inputs[RELEASE_INPUT].getVoltage() != 0){
								if (fading == true) {		// if it's fading
									if (inputs[IN_INPUT].isConnected()) { // if fading and input connected
										// update release value
										arSum = params[RELEASE_PARAMS].getValue() + inputs[RELEASE_INPUT].getVoltage();
										if (arSum > 10) {
											arSum = 10;
										} else if (arSum < 0) {
											arSum = 0;
										}
										maxFadeSample = args.sampleRate * arSum;
										// get sustain value
										sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
										if (sustain > 1) {
											sustain = 1;
										} else if (sustain < 0) {
											sustain = 0;
										}

										lastFade = -(currentFadeSample / maxFadeSample) + startFade;
										
										if (lastFade < 0) {
											fading = false;
											currentFadeSample = 0;
											outputs[OUT_OUTPUT].setVoltage(0);
											outputs[ROUT_OUTPUT].setVoltage(0);
											startFade = 0;
											lastFade = 0;
										} else {
											outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage() * lastFade * sustain);
											if (inputs[RIN_INPUT].isConnected()){
												outputs[ROUT_OUTPUT].setVoltage(inputs[RIN_INPUT].getVoltage() * lastFade * sustain);
											} else {
												outputs[ROUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope
											}
										}
									} else {	// if fading and input is not connected
										// update release value
										arSum = params[RELEASE_PARAMS].getValue() + inputs[RELEASE_INPUT].getVoltage();
										if (arSum > 10) {
											arSum = 10;
										} else if (arSum < 0) {
											arSum = 0;
										}
										// get sustain value
										sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
										if (sustain > 1) {
											sustain = 1;
										} else if (sustain < 0) {
											sustain = 0;
										}
										maxFadeSample = args.sampleRate * arSum;

										lastFade = -(currentFadeSample / maxFadeSample) + startFade;

										if (lastFade < 0) {
											fading = false;
											currentFadeSample = 0;
											outputs[OUT_OUTPUT].setVoltage(0);
											outputs[ROUT_OUTPUT].setVoltage(0);
											startFade = 0;
											lastFade = 0;
										} else {
											outputs[OUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope
											outputs[ROUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope
										}
									}
									currentFadeSample++;
								} else {									// if fading RELEASE has ended (fade = false)
									startFade = 0;
									outputs[OUT_OUTPUT].setVoltage(0);
									if (inputs[RIN_INPUT].isConnected()){
										outputs[ROUT_OUTPUT].setVoltage(0);
									} else {
										outputs[ROUT_OUTPUT].setVoltage(0);
									}
								}
							} else {  // if RELEASE has not set
								outputs[OUT_OUTPUT].setVoltage(0);
								if (inputs[RIN_INPUT].isConnected()){
									outputs[ROUT_OUTPUT].setVoltage(0);
								} else {
										outputs[ROUT_OUTPUT].setVoltage(0);
									}
							}
							break;
						case 1: 									// gating
							if (!currentTrigState) { 					// if GATE goes LOW
								outputs[GATE_OUTPUT].setVoltage(0);
								gateState = 0;
								if (params[RELEASE_PARAMS].getValue() != 0 || inputs[RELEASE_INPUT].getVoltage() != 0){
									if (fading) {
										startFade = lastFade;
									} else {
										fading = true;
										startFade = 1;
									}
									currentFadeSample = 0;
								}
							} else 	if (params[ATTACK_PARAMS].getValue() != 0 || inputs[ATTACK_INPUT].getVoltage() != 0){
								if (fading == true) {
									if (inputs[IN_INPUT].isConnected()) {			// if fading attack and input is connected
										// update attack value
										arSum = params[ATTACK_PARAMS].getValue() + inputs[ATTACK_INPUT].getVoltage();
										if (arSum > 10) {
											arSum = 10;
										} else 	if (arSum < 0) {
											arSum = 0;
										}
										maxFadeSample = args.sampleRate * arSum;
										// get sustain value
										sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
										if (sustain > 1) {
											sustain = 1;
										} else if (sustain < 0) {
											sustain = 0;
										}

										lastFade = (currentFadeSample / maxFadeSample) + startFade;

										if (lastFade > 1) {
											fading = false;
											currentFadeSample = 0;
											startFade = 0;
											lastFade = 0;
										} else {

											outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage() * lastFade * sustain);
											if (inputs[RIN_INPUT].isConnected()) {
												outputs[ROUT_OUTPUT].setVoltage(inputs[RIN_INPUT].getVoltage() * lastFade * sustain);
											} else {
												outputs[ROUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope
											}
										}
									} else {									// if fading attack and input is not connected
										// update attack value
										arSum = params[ATTACK_PARAMS].getValue() + inputs[ATTACK_INPUT].getVoltage();
										if (arSum > 10) {
											arSum = 10;
										} else 	if (arSum < 0) {
											arSum = 0;
										}
										// get sustain value
										sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
										if (sustain > 1) {
											sustain = 1;
										} else if (sustain < 0) {
											sustain = 0;
										}
										maxFadeSample = args.sampleRate * arSum;

										lastFade = (currentFadeSample / maxFadeSample) + startFade;

										if (lastFade > 1) {
											fading = false;
											currentFadeSample = 0;
											startFade = 0;
											lastFade = 0;
										} else {
											outputs[OUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope on left
											outputs[ROUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope on right
										}
									}
									currentFadeSample++;
								} else { // if fading ATTACK  has ended (fade=false)
									if (inputs[IN_INPUT].isConnected()) {		// if GATING and input is connected
										sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
										if (sustain > 1) {
											sustain = 1;
										} else if (sustain < 0) {
											sustain = 0;
										}
										outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage() * sustain);
										if (inputs[RIN_INPUT].isConnected()) {
											outputs[ROUT_OUTPUT].setVoltage(inputs[RIN_INPUT].getVoltage() * sustain);
										} else {
											outputs[ROUT_OUTPUT].setVoltage(10 * sustain);	// send envelope
										}
									} else {											// if GATING and input is not connected
										sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
										if (sustain > 1) {
											sustain = 1;
										} else if (sustain < 0) {
											sustain = 0;
										}
										outputs[OUT_OUTPUT].setVoltage(10 * sustain); // send envelope on left and right channel
										outputs[ROUT_OUTPUT].setVoltage(10 * sustain); // send envelope on right channel
									}
								}
							} else if (inputs[IN_INPUT].isConnected()) {  // if fade Attack parameters are not set and input is connected
								sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
								if (sustain > 1) {
									sustain = 1;
								} else if (sustain < 0) {
									sustain = 0;
								}
								outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage() * sustain);
								if (inputs[RIN_INPUT].isConnected()) {
									outputs[ROUT_OUTPUT].setVoltage(inputs[RIN_INPUT].getVoltage() * sustain);
								} else {
									outputs[ROUT_OUTPUT].setVoltage(10 * sustain);
								}
							} else {	// if fade Attack parameters are not set and input is not connected
								sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
								if (sustain > 1) {
									sustain = 1;
								} else if (sustain < 0) {
									sustain = 0;
								}
								outputs[OUT_OUTPUT].setVoltage(10 * sustain); // send envelope on left and right channel
								outputs[ROUT_OUTPUT].setVoltage(10 * sustain);
							}
							break;
					}
				}
			break;
			// ********************************** TOGGLER MODE *******
			case 1:		
				if (inputs[RST_INPUT].isConnected()){
					rst = inputs[RST_INPUT].getVoltage();
					if (rst > 0 && prevRst <= 0) {
						// next lines are duplicated from case 1
						outputs[GATE_OUTPUT].setVoltage(0.f);
						// below is different from original: if gateState is 0 or 1
						// it will not do the fade 
						if ((params[RELEASE_PARAMS].getValue() != 0 || inputs[RELEASE_INPUT].getVoltage() != 0) && gateState == 1){
							if (fading) {
								startFade = lastFade;
							} else {
								fading = true;
								startFade = 1;
							}
							currentFadeSample = 0;
						}
						gateState = 0;
						// end of duplicated lines
					}
					prevRst = rst; 
				}
				
				if (inputs[TRIG_INPUT].isConnected()){
					trigState = inputs[TRIG_INPUT].getVoltage();
					if (trigState > 0 && prevTrigState <= 0){
						currentTrigState = true;
					} else {
						currentTrigState = false;
					}
					prevTrigState = trigState;

					switch (gateState) {
						case 0: 									// waiting for TRIG
							if (currentTrigState){					// if TRIG occurs
								outputs[GATE_OUTPUT].setVoltage(10);
								gateState = 1;
								if (params[ATTACK_PARAMS].getValue() != 0 || inputs[ATTACK_INPUT].getVoltage() != 0) {
									if (fading) {
										startFade = lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							} else if (params[RELEASE_PARAMS].getValue() != 0 || inputs[RELEASE_INPUT].getVoltage() != 0){
								if (fading == true) {
									if (inputs[IN_INPUT].isConnected()) {
										// update release value
										arSum = params[RELEASE_PARAMS].getValue() + inputs[RELEASE_INPUT].getVoltage();
										if (arSum > 10) {
											arSum = 10;
										} else if (arSum < 0) {
											arSum = 0;
										}
										maxFadeSample = args.sampleRate * arSum;
										// get sustain value
										sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
										if (sustain > 1) {
											sustain = 1;
										} else if (sustain < 0) {
											sustain = 0;
										}

										lastFade = -(currentFadeSample / maxFadeSample) + startFade;

										//if (currentFadeSample > maxFadeSample) {
										if (lastFade < 0) {
											fading = false;
											currentFadeSample = 0;
											outputs[OUT_OUTPUT].setVoltage(0);
											outputs[ROUT_OUTPUT].setVoltage(0);
											startFade = 0;
											lastFade = 0;
										} else {

											outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage() * lastFade * sustain);
											if (inputs[RIN_INPUT].isConnected()) {
												outputs[ROUT_OUTPUT].setVoltage(inputs[RIN_INPUT].getVoltage() * lastFade * sustain);
											} else {
												outputs[ROUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope
											}
										}
									} else { // if fading and input is not connected
										// update attack value
										arSum = params[ATTACK_PARAMS].getValue() + inputs[ATTACK_INPUT].getVoltage();
										if (arSum > 10) {
											arSum = 10;
										} else 	if (arSum < 0) {
											arSum = 0;
										}
										// get sustain value
										sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
										if (sustain > 1) {
											sustain = 1;
										} else if (sustain < 0) {
											sustain = 0;
										}
										maxFadeSample = args.sampleRate * arSum;
										
										lastFade = -(currentFadeSample / maxFadeSample) + startFade;

										if (lastFade < 0) {
											fading = false;
											currentFadeSample = 0;
											outputs[OUT_OUTPUT].setVoltage(0);
											outputs[ROUT_OUTPUT].setVoltage(0);
											startFade = 0;
											lastFade = 0;
										} else {
											outputs[OUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope
											outputs[ROUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope
										}
									}
									currentFadeSample++;
								} else {									// if RELEASING has ended
									outputs[OUT_OUTPUT].setVoltage(0);
									if (inputs[RIN_INPUT].isConnected()) {
										outputs[ROUT_OUTPUT].setVoltage(0);
									} else {
										outputs[ROUT_OUTPUT].setVoltage(0);
									}
								}
							} else {  // if RELEASE has not set
								outputs[OUT_OUTPUT].setVoltage(0);
								if (inputs[RIN_INPUT].isConnected()) {
									outputs[ROUT_OUTPUT].setVoltage(0);
								} else {
									outputs[ROUT_OUTPUT].setVoltage(0);
								}
							}
							break;
						case 1: 									// gating
							if (currentTrigState) { 					// if TRIG occurs
								outputs[GATE_OUTPUT].setVoltage(0);
								gateState = 0;
								if (params[RELEASE_PARAMS].getValue() != 0 || inputs[RELEASE_INPUT].getVoltage() != 0){
									if (fading) {
										startFade = lastFade;
									} else {
										fading = true;
										startFade = 1;
									}
									currentFadeSample = 0;
								}
							} else 	if (params[ATTACK_PARAMS].getValue() != 0 || inputs[ATTACK_INPUT].getVoltage() != 0){
								if (fading == true) {
									if (inputs[IN_INPUT].isConnected()) {	// if fading attack and input connected
										// update attack value
										arSum = params[ATTACK_PARAMS].getValue() + inputs[ATTACK_INPUT].getVoltage();
										if (arSum > 10) {
											arSum = 10;
										} else 	if (arSum < 0) {
											arSum = 0;
										}
										maxFadeSample = args.sampleRate * arSum;
										// get sustain value
										sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
										if (sustain > 1) {
											sustain = 1;
										} else if (sustain < 0) {
											sustain = 0;
										}

										lastFade = (currentFadeSample / maxFadeSample) + startFade;

										if (lastFade > 1) {
											fading = false;
											currentFadeSample = 0;
											startFade = 0;
											lastFade = 0;
										} else {
											outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage() * lastFade * sustain);
											if (inputs[RIN_INPUT].isConnected()) {
												outputs[ROUT_OUTPUT].setVoltage(inputs[RIN_INPUT].getVoltage() * lastFade * sustain);
											} else {
												outputs[ROUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope
											}
										}
									} else {					// if fading attack and input not connected
										// update attack value
										arSum = params[ATTACK_PARAMS].getValue() + inputs[ATTACK_INPUT].getVoltage();
										if (arSum > 10) {
											arSum = 10;
										} else 	if (arSum < 0) {
											arSum = 0;
										}
										sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
										// get sustain value
										if (sustain > 1) {
											sustain = 1;
										} else if (sustain < 0) {
											sustain = 0;
										}
										maxFadeSample = args.sampleRate * arSum;
																			
										lastFade = (currentFadeSample / maxFadeSample) + startFade;

										if (lastFade > 1) {
											fading = false;
											currentFadeSample = 0;
											startFade = 0;
											lastFade = 0;
										} else {
											outputs[OUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope on left
											outputs[ROUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope on right
										}
									}
									currentFadeSample++;
								} else if (inputs[IN_INPUT].isConnected()) { 	// if not fading attack and input connected
									sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
									if (sustain > 1) {
										sustain = 1;
									} else if (sustain < 0) {
										sustain = 0;
									}
									outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage() * sustain);
									if (inputs[RIN_INPUT].isConnected()) {
										outputs[ROUT_OUTPUT].setVoltage(inputs[RIN_INPUT].getVoltage() * sustain);
									} else {
										outputs[ROUT_OUTPUT].setVoltage(10.f * sustain);	// send envelope
									}
								} else {		// if not fading attack and input not connected
									sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
									if (sustain > 1) {
										sustain = 1;
									} else if (sustain < 0) {
										sustain = 0;
									}
									outputs[OUT_OUTPUT].setVoltage(10 * sustain); // send envelope on left and right channel
									outputs[ROUT_OUTPUT].setVoltage(10 * sustain); // send envelope on right channel
								}
							} else if (inputs[IN_INPUT].isConnected()) { // if attack parameters are not set and input is connected
								sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
								if (sustain > 1) {
									sustain = 1;
								} else if (sustain < 0) {
									sustain = 0;
								}
								outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage() * sustain);
								if (inputs[RIN_INPUT].isConnected()) {
									outputs[ROUT_OUTPUT].setVoltage(inputs[RIN_INPUT].getVoltage() * sustain);
								} else {
									outputs[ROUT_OUTPUT].setVoltage(10 * sustain);
								}
							} else {									// if attack parameters are not set and input is not connected
								sustain = params[SUSTAIN_PARAMS].getValue() + inputs[SUSTAIN_INPUT].getVoltage() / 10;
								if (sustain > 1) {
									sustain = 1;
								} else if (sustain < 0) {
									sustain = 0;
								}
								outputs[OUT_OUTPUT].setVoltage(10 * sustain); // send envelope on left and right channel
								outputs[ROUT_OUTPUT].setVoltage(10 * sustain);
							}
							break;
					}
				}
			break;
		}
	}
};


struct TogglerCompactWidget : ModuleWidget {
	TogglerCompactWidget(TogglerCompact* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/TogglerCompact.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<CKSS>(mm2px(Vec(4, 11.2)), module, TogglerCompact::MODE_SWITCH));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 24.5)), module, TogglerCompact::TRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 36.2)), module, TogglerCompact::RST_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.2, 52.8)), module, TogglerCompact::IN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.5, 52.8)), module, TogglerCompact::RIN_INPUT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(6.32, 62)), module, TogglerCompact::ATTACK_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.32, 69)), module, TogglerCompact::ATTACK_INPUT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.32, 73)), module, TogglerCompact::SUSTAIN_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.32, 80)), module, TogglerCompact::SUSTAIN_INPUT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(6.32, 84)), module, TogglerCompact::RELEASE_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.32, 91)), module, TogglerCompact::RELEASE_INPUT));


		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.2, 107.5)), module, TogglerCompact::OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.5, 107.5)), module, TogglerCompact::ROUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 119)), module, TogglerCompact::GATE_OUTPUT));
	}
};


Model* modelTogglerCompact = createModel<TogglerCompact, TogglerCompactWidget>("TogglerCompact");