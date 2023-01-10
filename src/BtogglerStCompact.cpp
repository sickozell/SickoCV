#include "plugin.hpp"

struct BtogglerStCompact : Module {
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
		ENUMS(IN_INPUT,2),
		ATTACK_INPUT,
		SUSTAIN_INPUT,
		RELEASE_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT,2),
		GATE_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		WRN_LIGHT,
		OUT_LIGHT,
		LIGHTS_LEN
	};

	bool initStart = false;
	bool clockConnection = false;
	bool prevClockConnection = false;
	bool clockState = false;
	float clock = 0;
	float prevClock = 0;

	int internalState = 0;
	bool trigState = false;
	float trigValue = 0;
	float prevTrigValue = 0;

	float rst = 0;
	float prevRst = 0;

	float maxFadeSample = 0;
	float currentFadeSample = 0;
	bool fading = false;
	float startFade = 0;
	float lastFade = 0;

	float attack;
	float sustain;
	float release;

	BtogglerStCompact() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(ATTACK_PARAMS, 0.f, 10.f, 0.f, "Attack", "ms", 0, 1000);
		configParam(SUSTAIN_PARAMS, 0.f, 1.f, 1.f, "Sustain Level", "%", 0, 100);
		configParam(RELEASE_PARAMS, 0.f, 10.f, 0.f, "Release", "ms", 0, 1000);
		configInput(CLOCK_INPUT, "Clock");
		configInput(ARM_INPUT, "Arm");
		configInput(RST_INPUT, "Reset");
		configInput(IN_INPUT, "L");
		configInput(IN_INPUT+1, "R");
		configInput(ATTACK_INPUT, "Attack CV");
		configInput(SUSTAIN_INPUT, "Sustain CV");
		configInput(RELEASE_INPUT, "Release CV");
		configOutput(OUT_OUTPUT, "L");
		configOutput(OUT_OUTPUT+1, "R");
		configOutput(GATE_OUTPUT, "Gate");
	}

	void onReset(const ResetEvent &e) override {
		initStart = false;
		internalState = 0;
		clockConnection = false;
		prevClockConnection = false;
		clockState = false;
		clock = 0;
		prevClock = 0;
		trigState = false;
		trigValue = 0;
		prevTrigValue = 0;
		rst = 0;
		prevRst = 0;
		maxFadeSample = 0;
		currentFadeSample = 0;
		fading = false;
		startFade = 0;
		lastFade = 0;
		outputs[GATE_OUTPUT].setVoltage(0);
		outputs[OUT_OUTPUT].setVoltage(0);
		outputs[OUT_OUTPUT+1].setVoltage(0);
		lights[WRN_LIGHT].setBrightness(0.f);
		lights[OUT_LIGHT].setBrightness(0.f);
		Module::onReset(e);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "InitStart", json_boolean(initStart));
		json_object_set_new(rootJ, "State", json_integer(internalState));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* initStartJ = json_object_get(rootJ, "InitStart");
		if (initStartJ)
			initStart = json_boolean_value(initStartJ);

		if (!initStart) {
			json_t* jsonState = json_object_get(rootJ, "State");
			if (jsonState)
				internalState = json_integer_value(jsonState);
		}
	}

	void process(const ProcessArgs& args) override {
		attack = params[ATTACK_PARAMS].getValue() + inputs[ATTACK_INPUT].getVoltage();
		if (attack > 10)
			attack = 10;
		else if (attack < 0)
			attack = 0;

		sustain = params[SUSTAIN_PARAMS].getValue() + (inputs[SUSTAIN_INPUT].getVoltage() * 0.1);
		if (sustain > 1)
			sustain = 1;
		else if (sustain < 0)
			sustain = 0;

		release = params[RELEASE_PARAMS].getValue() + inputs[RELEASE_INPUT].getVoltage();
		if (release > 10)
			release = 10;
		else if (release < 0)
			release = 0;

		clockConnection = inputs[CLOCK_INPUT].isConnected();

		if (clockConnection) {
			clock = inputs[CLOCK_INPUT].getVoltage();
			if (clock >= 1 && prevClock < 1) {
				clockState = true;
			} else {
				clockState = false;
			}
			prevClock = clock;

			if (inputs[RST_INPUT].isConnected()){
				rst = inputs[RST_INPUT].getVoltage();
				if (rst >= 1 && prevRst < 1) {
					// next lines are duplicated from case 1
					outputs[GATE_OUTPUT].setVoltage(0.f);
					lights[OUT_LIGHT].setBrightness(0.f);
					lights[WRN_LIGHT].setBrightness(0.f);
					// below is different from original: if internalState is 0 or 1
					// it will not do the fade 
					if (release != 0 && internalState == 1){
						if (fading) {
							startFade = lastFade;
						} else {
							fading = true;
							startFade = 1;
						}
						currentFadeSample = 0;
					}
					internalState = 0;
					// end of duplicated lines
				}
				prevRst = rst; 
			}
			
			if (inputs[ARM_INPUT].isConnected()){
				trigValue = inputs[ARM_INPUT].getVoltage();
				if (trigValue >= 1 && prevTrigValue < 1){
					trigState = true;
				} else {
					trigState = false;
				}
				prevTrigValue = trigValue;

				switch (internalState) {

					case 0: 									// waiting for ARM
						if (trigState){					// if occurs go to state waiting for next clock
							internalState = 1;
							lights[WRN_LIGHT].setBrightness(1.f);
						} else if (release != 0){
							if (fading == true) {
								if (inputs[IN_INPUT].isConnected() || inputs[IN_INPUT+1].isConnected()) {
									maxFadeSample = args.sampleRate * release;
									lastFade = -(currentFadeSample / maxFadeSample) + startFade;

									if (lastFade < 0) {
										fading = false;
										currentFadeSample = 0;
										outputs[OUT_OUTPUT].setVoltage(0);
										outputs[OUT_OUTPUT+1].setVoltage(0);
										startFade = 0;
										lastFade = 0;
									} else {
										for (int i=0; i<2; i++){
											if (inputs[IN_INPUT+i].isConnected()) {
												outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage() * lastFade * sustain);
											} else {
												outputs[OUT_OUTPUT+i].setVoltage(10 * lastFade * sustain); // send envelope
											}
										}
									}
								} else { // if fading and BOTH inputs are not connected

									maxFadeSample = args.sampleRate * release;
									
									lastFade = -(currentFadeSample / maxFadeSample) + startFade;

									if (lastFade < 0) {
										fading = false;
										currentFadeSample = 0;
										outputs[OUT_OUTPUT].setVoltage(0);
										outputs[OUT_OUTPUT+1].setVoltage(0);
										startFade = 0;
										lastFade = 0;
									} else {
										outputs[OUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope
										outputs[OUT_OUTPUT+1].setVoltage(10 * lastFade * sustain); // send envelope
									}
								}
								currentFadeSample++;
							} else {									// if RELEASING has ended
								outputs[OUT_OUTPUT].setVoltage(0);
								outputs[OUT_OUTPUT+1].setVoltage(0);
							}
						} else {  // if RELEASE has not set
							outputs[OUT_OUTPUT].setVoltage(0);
							outputs[OUT_OUTPUT+1].setVoltage(0);
						}	
					break;

					case 1: 									// ARMED waiting for next clock
						if (trigState){					// // if another trigger occurs, then abort
							outputs[GATE_OUTPUT].setVoltage(0);
							lights[WRN_LIGHT].setBrightness(0.f);
							internalState = 0;
						} else if (clockState){
							outputs[GATE_OUTPUT].setVoltage(10);
							lights[OUT_LIGHT].setBrightness(1.f);
							lights[WRN_LIGHT].setBrightness(0.f);
							internalState = 2;
													
							if (attack != 0) {
								if (fading) {
									startFade = lastFade;
								} else {
									fading = true;
									startFade = 0;
								}
								currentFadeSample = 0;
							}
						} else if (release != 0){
							if (fading == true) {
								if (inputs[IN_INPUT].isConnected() || inputs[IN_INPUT+1].isConnected()) {

									maxFadeSample = args.sampleRate * release;

									lastFade = -(currentFadeSample / maxFadeSample) + startFade;

									if (lastFade < 0) {
										fading = false;
										currentFadeSample = 0;
										outputs[OUT_OUTPUT].setVoltage(0);
										outputs[OUT_OUTPUT+1].setVoltage(0);
										startFade = 0;
										lastFade = 0;
									} else {
										for (int i=0; i<2; i++){
											if (inputs[IN_INPUT+i].isConnected()) {
												outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage() * lastFade * sustain);
											} else {
												outputs[OUT_OUTPUT+i].setVoltage(10 * lastFade * sustain); // send envelope
											}
										}
									}
								} else { // if fading and inputs are BOTH not connected

									maxFadeSample = args.sampleRate * release;
									
									lastFade = -(currentFadeSample / maxFadeSample) + startFade;

									if (lastFade < 0) {
										fading = false;
										currentFadeSample = 0;
										outputs[OUT_OUTPUT].setVoltage(0);
										outputs[OUT_OUTPUT+1].setVoltage(0);
										startFade = 0;
										lastFade = 0;
									} else {
										outputs[OUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope
										outputs[OUT_OUTPUT+1].setVoltage(10 * lastFade * sustain); // send envelope
									}
								}
								currentFadeSample++;
							} else {									// if RELEASING has ended
								outputs[OUT_OUTPUT].setVoltage(0);
								outputs[OUT_OUTPUT+1].setVoltage(0);
							}
						} else {  // if RELEASE has not set
							outputs[OUT_OUTPUT].setVoltage(0);
							outputs[OUT_OUTPUT+1].setVoltage(0);
						}
					break;

					case 2: 									// gating
						outputs[GATE_OUTPUT].setVoltage(10);
						lights[OUT_LIGHT].setBrightness(1.f);
						if (trigState) { 					// if TRIG occurs
							lights[WRN_LIGHT].setBrightness(1.f);
							internalState = 3; // go to state waiting for next clock
						} else 	if (attack != 0){
							if (fading == true) {
								if (inputs[IN_INPUT].isConnected() || inputs[IN_INPUT+1].isConnected()) {	// if fading attack and input connected
									
									maxFadeSample = args.sampleRate * attack;

									lastFade = (currentFadeSample / maxFadeSample) + startFade;

									if (lastFade > 1) {
										fading = false;
										currentFadeSample = 0;
										startFade = 0;
										lastFade = 0;
									} else {
										for (int i=0; i<2; i++) {
											if (inputs[IN_INPUT+i].isConnected()) {
												outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage() * lastFade * sustain);
											} else {
												outputs[OUT_OUTPUT+i].setVoltage(10 * lastFade * sustain); // send envelope
											}
										}
									}
								} else {					// if fading attack and inputs are BOTH not connected

									maxFadeSample = args.sampleRate * attack;
																		
									lastFade = (currentFadeSample / maxFadeSample) + startFade;

									if (lastFade > 1) {
										fading = false;
										currentFadeSample = 0;
										startFade = 0;
										lastFade = 0;
									} else {
										outputs[OUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope on left
										outputs[OUT_OUTPUT+1].setVoltage(10 * lastFade * sustain); // send envelope on right
									}
								}
								currentFadeSample++;
							} else if (inputs[IN_INPUT].isConnected() || inputs[IN_INPUT+1].isConnected()) { 	// if not fading attack and one input connected

								for (int i=0; i<2; i++){
									if (inputs[IN_INPUT+i].isConnected()) {
										outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage() * sustain);
									} else {
										outputs[OUT_OUTPUT+i].setVoltage(10.f * sustain);	// send envelope
									}
								}
							} else {		// if not fading attack and input are BOTH not connected
								outputs[OUT_OUTPUT].setVoltage(10 * sustain); // send envelope on left and right channel
								outputs[OUT_OUTPUT+1].setVoltage(10 * sustain); // send envelope on right channel
							}

						} else if (inputs[IN_INPUT].isConnected() || inputs[IN_INPUT+1].isConnected()) { // if attack parameters are not set and ONE input is connected

							for (int i=0; i<2; i++){
								if (inputs[IN_INPUT+i].isConnected()) {
									outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage() * sustain);
								} else {
									outputs[OUT_OUTPUT+i].setVoltage(10 * sustain);
								}
							}
						} else {									// if attack parameters are not set and BOTH inputs are not connected

							outputs[OUT_OUTPUT].setVoltage(10 * sustain); // send envelope on left and right channel
							outputs[OUT_OUTPUT+1].setVoltage(10 * sustain);

						}
					break;

					case 3: 									// gating and ARMED, waiting for next clock
						if (trigState){					// if another trigger occurs, then abort
							internalState = 2;
							lights[WRN_LIGHT].setBrightness(0.f);
						} else if (clockState) {
							outputs[GATE_OUTPUT].setVoltage(0);
							lights[WRN_LIGHT].setBrightness(0.f);
							lights[OUT_LIGHT].setBrightness(0.f);
							internalState = 0;
							if (release != 0){
								if (fading) {
									startFade = lastFade;
								} else {
									fading = true;
									startFade = 1;
								}
								currentFadeSample = 0;
							}
						} else 	if (attack != 0){
							if (fading == true) {
								if (inputs[IN_INPUT].isConnected() || inputs[IN_INPUT+1].isConnected()) {	// if fading attack and ONE input connected

									maxFadeSample = args.sampleRate * attack;

									lastFade = (currentFadeSample / maxFadeSample) + startFade;

									if (lastFade > 1) {
										fading = false;
										currentFadeSample = 0;
										startFade = 0;
										lastFade = 0;
									} else {
										for (int i=0; i<2; i++){
											if (inputs[IN_INPUT+i].isConnected()) {
												outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage() * lastFade * sustain);
											} else {
												outputs[OUT_OUTPUT+i].setVoltage(10 * lastFade * sustain); // send envelope
											}
										}
									}
								} else {					// if fading attack and BOTH input are not connected

									maxFadeSample = args.sampleRate * attack;
																		
									lastFade = (currentFadeSample / maxFadeSample) + startFade;

									if (lastFade > 1) {
										fading = false;
										currentFadeSample = 0;
										startFade = 0;
										lastFade = 0;
									} else {
										outputs[OUT_OUTPUT].setVoltage(10 * lastFade * sustain); // send envelope on left
										outputs[OUT_OUTPUT+1].setVoltage(10 * lastFade * sustain); // send envelope on right
									}
								}
								currentFadeSample++;
							} else if (inputs[IN_INPUT].isConnected() || inputs[IN_INPUT+1].isConnected()) { 	// if not fading attack and ONE input connected

								for (int i=0; i<2; i++){
									if (inputs[IN_INPUT+i].isConnected()) {
										outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage() * sustain);
									} else {
										outputs[OUT_OUTPUT+i].setVoltage(10.f * sustain);	// send envelope
									}
								}
							} else {		// if not fading attack and BOTH inputs are not connected
	
								outputs[OUT_OUTPUT].setVoltage(10 * sustain); // send envelope on left and right channel
								outputs[OUT_OUTPUT+1].setVoltage(10 * sustain); // send envelope on right channel
							}
						} else if (inputs[IN_INPUT].isConnected() || inputs[IN_INPUT+1].isConnected()) { // if attack parameters are not set and ONE input is connected
							
							for (int i=0; i<2; i++){
								if (inputs[IN_INPUT+i].isConnected()) {
									outputs[OUT_OUTPUT+i].setVoltage(inputs[IN_INPUT+i].getVoltage() * sustain);
								} else {
									outputs[OUT_OUTPUT+i].setVoltage(10 * sustain);
								}
							}
						} else {									// if attack parameters are not set and BOTH inputs are not connected
							
							outputs[OUT_OUTPUT].setVoltage(10 * sustain); // send envelope on left and right channel
							outputs[OUT_OUTPUT+1].setVoltage(10 * sustain);
						}
					break;
				}
			} else { // if arm is not connected
				internalState = 0;
				outputs[OUT_OUTPUT].setVoltage(0);
				outputs[OUT_OUTPUT+1].setVoltage(0);
				outputs[GATE_OUTPUT].setVoltage(0);
				lights[OUT_LIGHT].setBrightness(0.f);
				lights[WRN_LIGHT].setBrightness(0.f);
			}
		} else {
			if (prevClockConnection) {
				internalState = 0;
				outputs[OUT_OUTPUT].setVoltage(0);
				outputs[OUT_OUTPUT+1].setVoltage(0);
				outputs[GATE_OUTPUT].setVoltage(0);
				lights[OUT_LIGHT].setBrightness(0.f);
				lights[WRN_LIGHT].setBrightness(0.f);
			}
		}
		prevClockConnection = clockConnection;
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
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.5, 52.8)), module, BtogglerStCompact::IN_INPUT+1));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(6.32, 62)), module, BtogglerStCompact::ATTACK_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.32, 69)), module, BtogglerStCompact::ATTACK_INPUT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(14.32, 73)), module, BtogglerStCompact::SUSTAIN_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.32, 80)), module, BtogglerStCompact::SUSTAIN_INPUT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(6.32, 84)), module, BtogglerStCompact::RELEASE_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.32, 91)), module, BtogglerStCompact::RELEASE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.2, 107.5)), module, BtogglerStCompact::OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.5, 107.5)), module, BtogglerStCompact::OUT_OUTPUT+1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 119)), module, BtogglerStCompact::GATE_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(16.5, 116.8)), module, BtogglerStCompact::WRN_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(16.5, 121)), module, BtogglerStCompact::OUT_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		BtogglerStCompact* module = dynamic_cast<BtogglerStCompact*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}
};

Model* modelBtogglerStCompact = createModel<BtogglerStCompact, BtogglerStCompactWidget>("BtogglerStCompact");