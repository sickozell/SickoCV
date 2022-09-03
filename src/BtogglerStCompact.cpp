#include "plugin.hpp"


struct BtogglerStCompact : Module {
	float clock = 0;
	float prevClock = 0;
	bool clockState = false;

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
		ATTACK_PARAMS,
		SUSTAIN_PARAMS,
		RELEASE_PARAMS,
		PARAMS_LEN
	};
	enum InputId {
		CLOCK_INPUT,
		ARM_INPUT,
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

	BtogglerStCompact() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(ATTACK_PARAMS, 0.f, 10.f, 0.f, "Attack (s)");
		configParam(SUSTAIN_PARAMS, 0.f, 1.f, 1.f, "Level", "%", 0, 100);
		configParam(RELEASE_PARAMS, 0.f, 10.f, 0.f, "Release (s)");
		configInput(CLOCK_INPUT, "Clock");
		configInput(ARM_INPUT, "Arm");
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
		if (inputs[CLOCK_INPUT].isConnected()){
			clock = inputs[CLOCK_INPUT].getVoltage();
			if (clock > 0 && prevClock <= 0) {
				clockState = true;
			} else {
				clockState = false;
			}
			prevClock = clock;


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
			
			if (inputs[ARM_INPUT].isConnected()){
				trigState = inputs[ARM_INPUT].getVoltage();
				if (trigState > 0 && prevTrigState <= 0){
					currentTrigState = true;
				} else {
					currentTrigState = false;
				}
				prevTrigState = trigState;

				switch (gateState) {

					case 0: 									// waiting for ARM
						if (currentTrigState){					// if occurs go to state waiting for next clock
							gateState = 1;
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

					case 1: 									// ARMED waiting for next clock
						if (currentTrigState){					// // if another trigger occurs, then abort
							outputs[GATE_OUTPUT].setVoltage(0);
							gateState = 0;
						} else if (clockState){
							outputs[GATE_OUTPUT].setVoltage(10);
							gateState = 2;
													
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

					case 2: 									// gating
						outputs[GATE_OUTPUT].setVoltage(10);
						if (currentTrigState) { 					// if TRIG occurs
							gateState = 3; // go to state waiting for next clock
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

					case 3: 									// gating and ARMED, waiting for next clock
						if (currentTrigState){					// if another trigger occurs, then abort
							gateState = 2;
						} else if (clockState) {
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
			
		}
	}
};


struct BtogglerStCompactWidget : ModuleWidget {
	BtogglerStCompactWidget(BtogglerStCompact* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/BtogglerStCompact.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.5, 11.1)), module, BtogglerStCompact::CLOCK_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 24.5)), module, BtogglerStCompact::ARM_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 36.2)), module, BtogglerStCompact::RST_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.2, 52.8)), module, BtogglerStCompact::IN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.5, 52.8)), module, BtogglerStCompact::RIN_INPUT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(6.32, 62)), module, BtogglerStCompact::ATTACK_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.32, 69)), module, BtogglerStCompact::ATTACK_INPUT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.32, 73)), module, BtogglerStCompact::SUSTAIN_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.32, 80)), module, BtogglerStCompact::SUSTAIN_INPUT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(6.32, 84)), module, BtogglerStCompact::RELEASE_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.32, 91)), module, BtogglerStCompact::RELEASE_INPUT));


		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.2, 107.5)), module, BtogglerStCompact::OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.5, 107.5)), module, BtogglerStCompact::ROUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 119)), module, BtogglerStCompact::GATE_OUTPUT));
	}
};


Model* modelBtogglerStCompact = createModel<BtogglerStCompact, BtogglerStCompactWidget>("BtogglerStCompact");