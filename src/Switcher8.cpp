#include "plugin.hpp"

struct Switcher8 : Module {
	enum ParamId {
		ENUMS(MODE_SWITCH, 8),
		ENUMS(FADE_PARAM, 8),
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(TRIG_INPUT, 8),
		ENUMS(RST_INPUT, 8),
		ENUMS(IN1_INPUT, 8),
		ENUMS(IN2_INPUT, 8),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT1_OUTPUT, 8),
		ENUMS(OUT2_OUTPUT, 8),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(MODE_LIGHT, 8),
		ENUMS(IN1_LIGHT, 8),
		ENUMS(IN2_LIGHT, 8),
		ENUMS(OUT1_LIGHT, 8),
		ENUMS(OUT2_LIGHT, 8),
		LIGHTS_LEN
	};

	bool initStart = false;
	int mode[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int prevMode[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	bool trigConnection[8] = {false, false, false, false, false, false, false, false};
	bool prevTrigConnection[8] = {false, false, false, false, false, false, false, false};
	int connection[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int prevConnection[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
	bool connectionChange[8] = {true, true, true, true, true, true, true, true};

	float rst[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	float prevRst[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	bool currentSwitch[8] = {false, false, false, false, false, false, false, false};
	bool trigState[8] = {false, false, false, false, false, false, false, false};
	float trigValue[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	float prevTrigValue[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	float fadeParam[8] =  {0, 0, 0, 0, 0, 0, 0, 0};
	float prevFadeParam[8] = {-1, -1, -1, -1, -1, -1, -1, -1};

	float fadeValue[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	float maxFadeSample[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	float currentFadeSample[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	bool fading[8] = {false, false, false, false, false, false, false, false};
	float startFade[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	float lastFade[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	int chan;

	/*static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	const float maxAdsrTime = 10.f;*/
	const float noEnvTime = 0.00101;

	Switcher8() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		for (int i = 0; i < 8; i++) {
			configSwitch(MODE_SWITCH+i, 0.f, 1.f, 0.f, "Mode", {"Toggle", "Gate"});
			configParam(FADE_PARAM+i, 0.f, 1.f, 0.f, "Fade Time", "ms", 10000.f, 1.f);
			configInput(TRIG_INPUT+i, "Trig/Gate");
			configInput(RST_INPUT+i, "Reset");
			configInput(IN1_INPUT+i, "IN 1");
			configInput(IN2_INPUT+i, "IN 2");
			configOutput(OUT1_OUTPUT+i, "OUT 1");
			configOutput(OUT2_OUTPUT+i, "OUT 2");
		}
	}

	void onReset(const ResetEvent &e) override {
		initStart = false;
		for (int i = 0; i < 8; i++) {
			mode[i] = 0;
			prevMode[i] = 0;
			trigConnection[i] = false;
			prevTrigConnection[i] = false;
			connection[i] = 0;
			prevConnection[i] = -1;
			connectionChange[i] = true;
			currentSwitch[i] = false;
			fading[i] = false;
			outputs[OUT1_OUTPUT+i].setVoltage(0.f);
			outputs[OUT1_OUTPUT+i].setChannels(1);
			outputs[OUT2_OUTPUT+i].setVoltage(0.f);
			outputs[OUT2_OUTPUT+i].setChannels(1);
			lights[IN1_LIGHT+i].setBrightness(0.f);
			lights[IN2_LIGHT+i].setBrightness(0.f);
			lights[OUT1_LIGHT+i].setBrightness(0.f);
			lights[OUT2_LIGHT+i].setBrightness(0.f);
		}
	    Module::onReset(e);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "InitStart", json_boolean(initStart));
		json_object_set_new(rootJ, "State1", json_boolean(currentSwitch[0]));
		json_object_set_new(rootJ, "State2", json_boolean(currentSwitch[1]));
		json_object_set_new(rootJ, "State3", json_boolean(currentSwitch[2]));
		json_object_set_new(rootJ, "State4", json_boolean(currentSwitch[3]));
		json_object_set_new(rootJ, "State5", json_boolean(currentSwitch[4]));
		json_object_set_new(rootJ, "State6", json_boolean(currentSwitch[5]));
		json_object_set_new(rootJ, "State7", json_boolean(currentSwitch[6]));
		json_object_set_new(rootJ, "State8", json_boolean(currentSwitch[7]));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* initStartJ = json_object_get(rootJ, "InitStart");
		if (initStartJ)
			initStart = json_boolean_value(initStartJ);

		if (!initStart) {
			json_t* jsonState1 = json_object_get(rootJ, "State1");
			if (jsonState1)
				currentSwitch[0] = json_boolean_value(jsonState1);

			json_t* jsonState2 = json_object_get(rootJ, "State2");
			if (jsonState2)
				currentSwitch[1] = json_boolean_value(jsonState2);

			json_t* jsonState3 = json_object_get(rootJ, "State3");
			if (jsonState3)
				currentSwitch[2] = json_boolean_value(jsonState3);

			json_t* jsonState4 = json_object_get(rootJ, "State4");
			if (jsonState4)
				currentSwitch[3] = json_boolean_value(jsonState4);

			json_t* jsonState5 = json_object_get(rootJ, "State5");
			if (jsonState5)
				currentSwitch[4] = json_boolean_value(jsonState5);

			json_t* jsonState6 = json_object_get(rootJ, "State6");
			if (jsonState6)
				currentSwitch[5] = json_boolean_value(jsonState6);

			json_t* jsonState7 = json_object_get(rootJ, "State7");
			if (jsonState7)
				currentSwitch[6] = json_boolean_value(jsonState7);

			json_t* jsonState8 = json_object_get(rootJ, "State8");
			if (jsonState8)
				currentSwitch[7] = json_boolean_value(jsonState8);
		}
	}

	/*static float convertCVToSeconds(float cv) {		
		return minStageTime * std::pow(maxStageTime / minStageTime, cv) / 1000;
	}*/

	void inline checkFadeParam(int track) {
		fadeParam[track] = params[FADE_PARAM+track].getValue();
		if (fadeParam[track] != prevFadeParam[track]) {
			fadeValue[track] = (std::pow(10000.f, fadeParam[track]) / 1000);
			prevFadeParam[track] = fadeParam[track];
		}
	}

	/*
	void inline checkFadeValue(int track) {
		if (fadeValue[track] > 10)
			fadeValue[track] = 10;
		else if (fadeValue[track] < 0)
			fadeValue[track] = 0;
	}

	void inline initFadeValue(int track) {
		maxFadeSample[track] = args.sampleRate * fadeValue[track];
		lastFade[track] = (currentFadeSample[track] / maxFadeSample[track]) + startFade[track];
	}
	*/

	void process(const ProcessArgs& args) override {

		for (int track = 0; track < 8; track++) {

			mode[track] = params[MODE_SWITCH+track].getValue();
			lights[MODE_LIGHT+track].setBrightness(mode[track]);

			trigConnection[track] = inputs[TRIG_INPUT+track].isConnected();
			if (!trigConnection[track]) {
				if (prevTrigConnection[track]) {
					outputs[OUT1_OUTPUT+track].setVoltage(0.f, 0);	// ******** TO CHECK IF POLYPHONIC
					outputs[OUT1_OUTPUT+track].setChannels(1);
					outputs[OUT2_OUTPUT+track].setVoltage(0.f, 0);
					outputs[OUT2_OUTPUT+track].setChannels(1);

					lights[IN1_LIGHT+track].setBrightness(0.f);
					lights[IN2_LIGHT+track].setBrightness(0.f);
					lights[OUT1_LIGHT+track].setBrightness(0.f);
					lights[OUT2_LIGHT+track].setBrightness(0.f);
					connectionChange[track] = false;
					currentSwitch[track] = false;
					prevConnection[track] = -1;
				}
			} else {

				connection[track] = 0;
				if (inputs[IN1_INPUT+track].isConnected())
					connection[track] = 1;
				if (inputs[IN2_INPUT+track].isConnected())
					connection[track] += 2;
				if (outputs[OUT1_OUTPUT+track].isConnected())
					connection[track] += 4;
				if (outputs[OUT2_OUTPUT+track].isConnected())
					connection[track] += 8;
				if (connection[track] != prevConnection[track]) {
					connectionChange[track] = true;
					prevConnection[track] = connection[track];
				}
				
				// legenda:
				// OUT1 = 4
				// OUT2 = 8
				// OUT1 + OUT2 = 12
				// IN1 + OUT1 = 5
				// IN1 + OUT2 = 9
				// IN2 + OUT1 = 6
				// IN2 + OUT2 = 10
				// IN1 + IN2 + OUT1 = 7
				// IN1 + IN2 + OUT2 = 11
				// IN1 + OUT1 + OUT2 = 13
				// IN2 + OUT1 + OUT2 = 14
				// IN1 + IN2 + OUT1 + OUT2 = 15
				//

				//mode[track] = params[MODE_SWITCH+track].getValue();
				//lights[MODE_LIGHT+track].setBrightness(mode[track]);

				if (mode[track] != prevMode[track]) {
					connectionChange[track] = true;
					//if (mode[track] == 0)
					if (mode[track] == 1)
						currentSwitch[track] = false;
					prevMode[track] = mode[track];
				}

				trigValue[track] = inputs[TRIG_INPUT+track].getVoltage();
				
				switch (mode[track]) {
					// ************************************** GATE MODE **********
					case 1:
						if (trigValue[track] >= 1 && prevTrigValue[track] < 1) {
							currentSwitch[track] = true;
							connectionChange[track] = true;
							trigState[track] = true;
						} else if (trigValue[track] < 1 && prevTrigValue[track] >= 1) {
							currentSwitch[track] = false;
							connectionChange[track] = true;
							trigState[track] = true;
						} 
						prevTrigValue[track] = trigValue[track];

						if (trigState[track]) {
							//fadeValue = convertCVToSeconds(params[FADE_PARAM].getValue()) + inputs[FADECV_INPUT].getVoltage();
							//fadeValue = (std::pow(10000.f, params[FADE_PARAM].getValue()) / 1000) + inputs[FADECV_INPUT].getVoltage();
							//fadeValue[track] = (std::pow(10000.f, params[FADE_PARAM+track].getValue()) / 1000);
							checkFadeParam(track);

							if (fadeValue[track] > noEnvTime) {
								if (fading[track]) {
									startFade[track] = 1-lastFade[track];
								} else {
									fading[track] = true;
									startFade[track] = 0;
								}
								currentFadeSample[track] = 0;
							}
							trigState[track] = false;
						}
					break;
					// ************************************** TRIG MODE **********
					case 0:
						if (trigValue[track] >= 1 && prevTrigValue[track] < 1)
							trigState[track] = true;
						else
							trigState[track] = false;

						prevTrigValue[track] = trigValue[track];

						if (trigState[track]) {
							currentSwitch[track] = !currentSwitch[track];
							connectionChange[track] = true;
							//fadeValue = convertCVToSeconds(params[FADE_PARAM].getValue()) + inputs[FADECV_INPUT].getVoltage();
							//fadeValue = (std::pow(10000.f, params[FADE_PARAM].getValue()) / 1000) + inputs[FADECV_INPUT].getVoltage();
							//fadeValue[track] = (std::pow(10000.f, params[FADE_PARAM+track].getValue()) / 1000);
							checkFadeParam(track);

							if (fadeValue[track] > noEnvTime) {
								if (fading[track]) {
									startFade[track] = 1-lastFade[track];
								} else {
									fading[track] = true;
									startFade[track] = 0;
								}
								currentFadeSample[track] = 0;
							}
						}
					break;
				}

				switch (connection[track]) {
					case 4: 										// OUT1 = 4
						if (inputs[RST_INPUT+track].isConnected()) {
							rst[track] = inputs[RST_INPUT+track].getVoltage();
							if (rst[track] >= 1 && prevRst[track] < 1) {
								if (currentSwitch[track]) {
									currentSwitch[track] = 0;
									connectionChange[track] = true;
									if (fadeValue[track] > noEnvTime) {
										if (fading[track]) {
											startFade[track] = 1-lastFade[track];
										} else {
											fading[track] = true;
											startFade[track] = 0;
										}
										currentFadeSample[track] = 0;
									}
								}
							}
							prevRst[track] = rst[track]; 
						}

						if (fading[track]) {

							maxFadeSample[track] = args.sampleRate * fadeValue[track];
							lastFade[track] = (currentFadeSample[track] / maxFadeSample[track]) + startFade[track];

							if (lastFade[track] > 1) {
								fading[track] = false;
								currentFadeSample[track] = 0;
								startFade[track] = 0;
								lastFade[track] = 0;
							} else {
								if (currentSwitch[track]) {
									outputs[OUT1_OUTPUT+track].setVoltage(10.f * lastFade[track], 0);
									outputs[OUT1_OUTPUT+track].setChannels(1);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								} else {
									outputs[OUT1_OUTPUT+track].setVoltage(10.f * (1-lastFade[track]), 0);
									outputs[OUT1_OUTPUT+track].setChannels(1);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(1-lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							}
							currentFadeSample[track]++;
						} else {
							if (currentSwitch[track]) {
								outputs[OUT1_OUTPUT+track].setVoltage(10.f, 0);
								outputs[OUT1_OUTPUT+track].setChannels(1);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(1.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							} else {
								outputs[OUT1_OUTPUT+track].setVoltage(0.f, 0);
								outputs[OUT1_OUTPUT+track].setChannels(1);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							}
						}
					break;
					
					case 8:											// OUT2 = 8
						if (inputs[RST_INPUT+track].isConnected()) {
							rst[track] = inputs[RST_INPUT+track].getVoltage();
							if (rst[track] >= 1 && prevRst[track] < 1) {
								if (currentSwitch[track]) {
									currentSwitch[track] = 0;
									connectionChange[track] = true;
									if (fadeValue[track] > noEnvTime) {
										if (fading[track]) {
											startFade[track] = 1-lastFade[track];
										} else {
											fading[track] = true;
											startFade[track] = 0;
										}
										currentFadeSample[track] = 0;
									}
								}
							}
							prevRst[track] = rst[track]; 
						}

						if (fading[track]) {

							maxFadeSample[track] = args.sampleRate * fadeValue[track];
							lastFade[track] = (currentFadeSample[track] / maxFadeSample[track]) + startFade[track];

							if (lastFade[track] > 1) {
								fading[track] = false;
								currentFadeSample[track] = 0;
								startFade[track] = 0;
								lastFade[track] = 0;
							} else {
								if (currentSwitch[track]) {
									outputs[OUT2_OUTPUT+track].setVoltage(10.f * (1-lastFade[track]), 0);
									outputs[OUT2_OUTPUT+track].setChannels(1);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1-lastFade[track]);
								} else {
									outputs[OUT2_OUTPUT+track].setVoltage(10.f * lastFade[track], 0);
									outputs[OUT2_OUTPUT+track].setChannels(1);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(lastFade[track]);								
								}
							}
							currentFadeSample[track]++;
						} else {
							if (currentSwitch[track]) {
								outputs[OUT2_OUTPUT+track].setVoltage(0.f, 0);
								outputs[OUT2_OUTPUT+track].setChannels(1);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							} else {
								outputs[OUT2_OUTPUT+track].setVoltage(10.f, 0);
								outputs[OUT2_OUTPUT+track].setChannels(1);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1.f);
								}							
							}
						}
					break;

					case 12: 										// OUT1 + OUT2 = 12
						if (inputs[RST_INPUT+track].isConnected()) {
							rst[track] = inputs[RST_INPUT+track].getVoltage();
							if (rst[track] >= 1 && prevRst[track] < 1) {
								if (currentSwitch[track]) {
									currentSwitch[track] = 0;
									connectionChange[track] = true;
									if (fadeValue[track] > noEnvTime) {
										if (fading[track]) {
											startFade[track] = 1-lastFade[track];
										} else {
											fading[track] = true;
											startFade[track] = 0;
										}
										currentFadeSample[track] = 0;
									}
								}
							}
							prevRst[track] = rst[track]; 
						}

						if (fading[track]) {

							maxFadeSample[track] = args.sampleRate * fadeValue[track];
							lastFade[track] = (currentFadeSample[track] / maxFadeSample[track]) + startFade[track];

							if (lastFade[track] > 1) {
								fading[track] = false;
								currentFadeSample[track] = 0;
								startFade[track] = 0;
								lastFade[track] = 0;
							} else {
								if (currentSwitch[track]) {
									outputs[OUT1_OUTPUT+track].setVoltage(10.f * (1-lastFade[track]), 0);
									outputs[OUT1_OUTPUT+track].setChannels(1);

									outputs[OUT2_OUTPUT+track].setVoltage(10.f * lastFade[track], 0);
									outputs[OUT2_OUTPUT+track].setChannels(1);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(1-lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(lastFade[track]);
								} else {
									outputs[OUT1_OUTPUT+track].setVoltage(10.f * lastFade[track], 0);
									outputs[OUT1_OUTPUT+track].setChannels(1);

									outputs[OUT2_OUTPUT+track].setVoltage(10.f * (1-lastFade[track]));
									outputs[OUT2_OUTPUT+track].setChannels(1);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(1-lastFade[track]);
								}
							}
							currentFadeSample[track]++;
						} else {
							if (currentSwitch[track]) {
								outputs[OUT1_OUTPUT+track].setVoltage(0.f, 0);
								outputs[OUT1_OUTPUT+track].setChannels(1);

								outputs[OUT2_OUTPUT+track].setVoltage(10.f, 0);
								outputs[OUT2_OUTPUT+track].setChannels(1);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1.f);
								}
							} else {
								outputs[OUT1_OUTPUT+track].setVoltage(10.f, 0);
								outputs[OUT1_OUTPUT+track].setChannels(1);

								outputs[OUT2_OUTPUT+track].setVoltage(0.f, 0);
								outputs[OUT2_OUTPUT+track].setChannels(1);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(1.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							}
						}
					break;

					case 5:											// IN1 + OUT1 = 5
						if (inputs[RST_INPUT+track].isConnected()) {
							rst[track] = inputs[RST_INPUT+track].getVoltage();
							if (rst[track] >= 1 && prevRst[track] < 1) {
								if (currentSwitch[track]) {
									currentSwitch[track] = 0;
									connectionChange[track] = true;
									if (fadeValue[track] > noEnvTime) {
										if (fading[track]) {
											startFade[track] = 1-lastFade[track];
										} else {
											fading[track] = true;
											startFade[track] = 0;
										}
										currentFadeSample[track] = 0;
									}
								}
							}
							prevRst[track] = rst[track]; 
						}

						if (fading[track]) {

							maxFadeSample[track] = args.sampleRate * fadeValue[track];
							lastFade[track] = (currentFadeSample[track] / maxFadeSample[track]) + startFade[track];

							if (lastFade[track] > 1) {
								fading[track] = false;
								currentFadeSample[track] = 0;
								startFade[track] = 0;
								lastFade[track] = 0;
							} else {
								chan = std::max(1, inputs[IN1_INPUT+track].getChannels());
								if (currentSwitch[track]) {
									for (int c = 0; c < chan; c++)
										outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c) * lastFade[track], c);
									outputs[OUT1_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								} else {
									for (int c = 0; c < chan; c++)
										outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c) * (1-lastFade[track]), c);
									outputs[OUT1_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(1-lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(0.f);								
								}
							}
							currentFadeSample[track]++;
						} else {
							chan = std::max(1, inputs[IN1_INPUT+track].getChannels());
							if (currentSwitch[track]) {
								for (int c = 0; c < chan; c++)
									outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c), c);
								outputs[OUT1_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(1.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							} else {
								for (int c = 0; c < chan; c++)
									outputs[OUT1_OUTPUT+track].setVoltage(0.f , c);
								outputs[OUT1_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}							
							}
						}
					break;
					
					case 10:											// IN2 + OUT2 = 10
						if (inputs[RST_INPUT+track].isConnected()) {
							rst[track] = inputs[RST_INPUT+track].getVoltage();
							if (rst[track] >= 1 && prevRst[track] < 1) {
								if (currentSwitch[track]) {
									currentSwitch[track] = 0;
									connectionChange[track] = true;
									if (fadeValue[track] > noEnvTime) {
										if (fading[track]) {
											startFade[track] = 1-lastFade[track];
										} else {
											fading[track] = true;
											startFade[track] = 0;
										}
										currentFadeSample[track] = 0;
									}
								}
							}
							prevRst[track] = rst[track]; 
						}

						if (fading[track]) {

							maxFadeSample[track] = args.sampleRate * fadeValue[track];
							lastFade[track] = (currentFadeSample[track] / maxFadeSample[track]) + startFade[track];

							if (lastFade[track] > 1) {
								fading[track] = false;
								currentFadeSample[track] = 0;
								startFade[track] = 0;
								lastFade[track] = 0;
							} else {
								chan = std::max(1, inputs[IN2_INPUT+track].getChannels());
								if (currentSwitch[track]) {
									for (int c = 0; c < chan; c++)
										outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c) * lastFade[track], c);
									outputs[OUT2_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(lastFade[track]);
								} else {
									for (int c = 0; c < chan; c++)
										outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c) * (1-lastFade[track]), c);
									outputs[OUT2_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1-lastFade[track]);								
								}
							}
							currentFadeSample[track]++;
						} else {
							chan = std::max(1, inputs[IN2_INPUT+track].getChannels());
							if (currentSwitch[track]) {
								for (int c = 0; c < chan; c++)
									outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c), c);
								outputs[OUT2_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1.f);
								}
							} else {
								for (int c = 0; c < chan; c++)
									outputs[OUT2_OUTPUT+track].setVoltage(0.f, c);
								outputs[OUT2_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}							
							}
						}
					break;

					case 9:											// IN1 + OUT2 = 9
						if (inputs[RST_INPUT+track].isConnected()) {
							rst[track] = inputs[RST_INPUT+track].getVoltage();
							if (rst[track] >= 1 && prevRst[track] < 1) {
								if (currentSwitch[track]) {
									currentSwitch[track] = 0;
									connectionChange[track] = true;
									if (fadeValue[track] > noEnvTime) {
										if (fading[track]) {
											startFade[track] = 1-lastFade[track];
										} else {
											fading[track] = true;
											startFade[track] = 0;
										}
										currentFadeSample[track] = 0;
									}
								}
							}
							prevRst[track] = rst[track]; 
						}

						if (fading[track]) {

							maxFadeSample[track] = args.sampleRate * fadeValue[track];
							lastFade[track] = (currentFadeSample[track] / maxFadeSample[track]) + startFade[track];

							if (lastFade[track] > 1) {
								fading[track] = false;
								currentFadeSample[track] = 0;
								startFade[track] = 0;
								lastFade[track] = 0;
							} else {
								chan = std::max(1, inputs[IN1_INPUT+track].getChannels());
								if (currentSwitch[track]) {
									for (int c = 0; c < chan; c++)
										outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c) * (1-lastFade[track]), c);
									outputs[OUT2_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1-lastFade[track]);
								} else {
									for (int c = 0; c < chan; c++)
										outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c) * lastFade[track], c);
									outputs[OUT2_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(lastFade[track]);								
								}
							}
							currentFadeSample[track]++;
						} else {
							chan = std::max(1, inputs[IN1_INPUT+track].getChannels());
							if (currentSwitch[track]) {
								for (int c = 0; c < chan; c++)
									outputs[OUT2_OUTPUT+track].setVoltage(0.f, c);
								outputs[OUT2_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							} else {
								for (int c = 0; c < chan; c++)
									outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c), c);
								outputs[OUT2_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1.f);
								}							
							}
						}
					break;

					case 6:											// IN2 + OUT1 = 6
						if (inputs[RST_INPUT+track].isConnected()) {
							rst[track] = inputs[RST_INPUT+track].getVoltage();
							if (rst[track] >= 1 && prevRst[track] < 1) {
								if (currentSwitch[track]) {
									currentSwitch[track] = 0;
									connectionChange[track] = true;
									if (fadeValue[track] > noEnvTime) {
										if (fading[track]) {
											startFade[track] = 1-lastFade[track];
										} else {
											fading[track] = true;
											startFade[track] = 0;
										}
										currentFadeSample[track] = 0;
									}
								}
							}
							prevRst[track] = rst[track]; 
						}

						if (fading[track]) {

							maxFadeSample[track] = args.sampleRate * fadeValue[track];
							lastFade[track] = (currentFadeSample[track] / maxFadeSample[track]) + startFade[track];

							if (lastFade[track] > 1) {
								fading[track] = false;
								currentFadeSample[track] = 0;
								startFade[track] = 0;
								lastFade[track] = 0;
							} else {
								chan = std::max(1, inputs[IN2_INPUT+track].getChannels());
								if (currentSwitch[track]) {
									for (int c = 0; c < chan; c++)
										outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c) * (1-lastFade[track]), c);
									outputs[OUT1_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(1-lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								} else {
									for (int c = 0; c < chan; c++)
										outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c) * lastFade[track], c);
									outputs[OUT1_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							}
							currentFadeSample[track]++;
						} else {
							chan = std::max(1, inputs[IN2_INPUT+track].getChannels());
							if (currentSwitch[track]) {
								for (int c = 0; c < chan; c++)
									outputs[OUT1_OUTPUT+track].setVoltage(0.f, c);
								outputs[OUT1_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							} else {
								for (int c = 0; c < chan; c++)
									outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c), c);
								outputs[OUT1_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(1.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}							
							}
						}
					break;

					case 7:													// IN1 + IN2 + OUT1 = 7
						if (inputs[RST_INPUT+track].isConnected()) {
							rst[track] = inputs[RST_INPUT+track].getVoltage();
							if (rst[track] >= 1 && prevRst[track] < 1) {
								if (currentSwitch[track]) {
									currentSwitch[track] = 0;
									connectionChange[track] = true;
									if (fadeValue[track] > noEnvTime) {
										if (fading[track]) {
											startFade[track] = 1-lastFade[track];
										} else {
											fading[track] = true;
											startFade[track] = 0;
										}
										currentFadeSample[track] = 0;
									}
								}
							}
							prevRst[track] = rst[track]; 
						}

						if (fading[track]) {

							maxFadeSample[track] = args.sampleRate * fadeValue[track];
							lastFade[track] = (currentFadeSample[track] / maxFadeSample[track]) + startFade[track];

							if (lastFade[track] > 1) {
								fading[track] = false;
								currentFadeSample[track] = 0;
								startFade[track] = 0;
								lastFade[track] = 0;
							} else {
								chan = std::max(inputs[IN2_INPUT+track].getChannels(), inputs[IN1_INPUT+track].getChannels());
								if (currentSwitch[track]) {
									for (int c = 0; c < chan; c++)
										outputs[OUT1_OUTPUT+track].setVoltage((inputs[IN2_INPUT+track].getVoltage(c) * lastFade[track]) + (inputs[IN1_INPUT+track].getVoltage(c) * (1-lastFade[track])), c);
									outputs[OUT1_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(1-lastFade[track]);
									lights[IN2_LIGHT+track].setBrightness(lastFade[track]);
									lights[OUT1_LIGHT+track].setBrightness(1.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								} else {
									for (int c = 0; c < chan; c++)
										outputs[OUT1_OUTPUT+track].setVoltage((inputs[IN1_INPUT+track].getVoltage(c) * lastFade[track]) + (inputs[IN2_INPUT+track].getVoltage(c) * (1-lastFade[track])), c);
									outputs[OUT1_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(lastFade[track]);
									lights[IN2_LIGHT+track].setBrightness(1-lastFade[track]);
									lights[OUT1_LIGHT+track].setBrightness(1.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							}
							currentFadeSample[track]++;
						} else {
							chan = std::max(inputs[IN2_INPUT+track].getChannels(), inputs[IN1_INPUT+track].getChannels());
							if (currentSwitch[track]) {
								for (int c = 0; c < chan; c++)
									outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c), c);
								outputs[OUT1_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(1.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							} else {
								for (int c = 0; c < chan; c++)
									outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c), c);
								outputs[OUT1_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(1.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							}
						}
					break;

					case 11:												// IN1 + IN2 + OUT2 = 11
						if (inputs[RST_INPUT+track].isConnected()) {
							rst[track] = inputs[RST_INPUT+track].getVoltage();
							if (rst[track] >= 1 && prevRst[track] < 1) {
								if (currentSwitch[track]) {
									currentSwitch[track] = 0;
									connectionChange[track] = true;
									if (fadeValue[track] > noEnvTime) {
										if (fading[track]) {
											startFade[track] = 1-lastFade[track];
										} else {
											fading[track] = true;
											startFade[track] = 0;
										}
										currentFadeSample[track] = 0;
									}
								}
							}
							prevRst[track] = rst[track]; 
						}

						if (fading[track]) {

							maxFadeSample[track] = args.sampleRate * fadeValue[track];
							lastFade[track] = (currentFadeSample[track] / maxFadeSample[track]) + startFade[track];

							if (lastFade[track] > 1) {
								fading[track] = false;
								currentFadeSample[track] = 0;
								startFade[track] = 0;
								lastFade[track] = 0;
							} else {
								chan = std::max(inputs[IN2_INPUT+track].getChannels(), inputs[IN1_INPUT+track].getChannels());
								if (currentSwitch[track]) {
									for (int c = 0; c < chan; c++)
										outputs[OUT2_OUTPUT+track].setVoltage((inputs[IN1_INPUT+track].getVoltage(c) * lastFade[track]) + (inputs[IN2_INPUT+track].getVoltage(c) * (1-lastFade[track])), c);
									outputs[OUT2_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(lastFade[track]);
									lights[IN2_LIGHT+track].setBrightness(1-lastFade[track]);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1.f);
								} else {
									for (int c = 0; c < chan; c++)
										outputs[OUT2_OUTPUT+track].setVoltage((inputs[IN2_INPUT+track].getVoltage(c) * lastFade[track]) + (inputs[IN1_INPUT+track].getVoltage(c) * (1-lastFade[track])), c);
									outputs[OUT2_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(1-lastFade[track]);
									lights[IN2_LIGHT+track].setBrightness(lastFade[track]);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1.f);
								}
							}
							currentFadeSample[track]++;
						} else {
							chan = std::max(inputs[IN2_INPUT+track].getChannels(), inputs[IN1_INPUT+track].getChannels());
							if (currentSwitch[track]) {
								for (int c = 0; c < chan; c++)
									outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c), c);
								outputs[OUT2_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1.f);
								}
							} else {
								for (int c = 0; c < chan; c++)
									outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c), c);
								outputs[OUT2_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1.f);
								}
							}
						}
					break;

					case 13:											// IN1 + OUT1 + OUT2 = 13
						if (inputs[RST_INPUT+track].isConnected()) {
							rst[track] = inputs[RST_INPUT+track].getVoltage();
							if (rst[track] >= 1 && prevRst[track] < 1) {
								if (currentSwitch[track]) {
									currentSwitch[track] = 0;
									connectionChange[track] = true;
									if (fadeValue[track] > noEnvTime) {
										if (fading[track]) {
											startFade[track] = 1-lastFade[track];
										} else {
											fading[track] = true;
											startFade[track] = 0;
										}
										currentFadeSample[track] = 0;
									}
								}
							}
							prevRst[track] = rst[track]; 
						}

						if (fading[track]) {

							maxFadeSample[track] = args.sampleRate * fadeValue[track];
							lastFade[track] = (currentFadeSample[track] / maxFadeSample[track]) + startFade[track];

							if (lastFade[track] > 1) {
								fading[track] = false;
								currentFadeSample[track] = 0;
								startFade[track] = 0;
								lastFade[track] = 0;
							} else {
								chan = std::max(1, inputs[IN1_INPUT+track].getChannels());
								if (currentSwitch[track]) {
									for (int c = 0; c < chan; c++) {
										outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c) * lastFade[track], c);
										outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c) * (1-lastFade[track]), c);
									}
									outputs[OUT2_OUTPUT+track].setChannels(chan);
									outputs[OUT1_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(1-lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(lastFade[track]);
								} else {
									for (int c = 0; c < chan; c++) {
										outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c) * (1-lastFade[track]), c);
										outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c) * lastFade[track], c);
									}
									outputs[OUT2_OUTPUT+track].setChannels(chan);
									outputs[OUT1_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(1-lastFade[track]);
								}
							}
							currentFadeSample[track]++;
						} else {
							chan = std::max(1, inputs[IN1_INPUT+track].getChannels());
							if (currentSwitch[track]) {
								for (int c = 0; c < chan; c++)
									outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c), c);
								outputs[OUT2_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									for (int c = 0; c < chan; c++)
										outputs[OUT1_OUTPUT+track].setVoltage(0.f, c);
									outputs[OUT1_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1.f);
								}
							} else {
								for (int c = 0; c < chan; c++)
									outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c), c);
								outputs[OUT1_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									for (int c = 0; c < chan; c++)
										outputs[OUT2_OUTPUT+track].setVoltage(0.f, c);
									outputs[OUT2_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(1.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							}
						}
					break;

					case 14:										// IN2 + OUT1 + OUT2 = 14
						if (inputs[RST_INPUT+track].isConnected()) {
							rst[track] = inputs[RST_INPUT+track].getVoltage();
							if (rst[track] >= 1 && prevRst[track] < 1) {
								if (currentSwitch[track]) {
									currentSwitch[track] = 0;
									connectionChange[track] = true;
									if (fadeValue[track] > noEnvTime) {
										if (fading[track]) {
											startFade[track] = 1-lastFade[track];
										} else {
											fading[track] = true;
											startFade[track] = 0;
										}
										currentFadeSample[track] = 0;
									}
								}
							}
							prevRst[track] = rst[track]; 
						}

						if (fading[track]) {

							maxFadeSample[track] = args.sampleRate * fadeValue[track];
							lastFade[track] = (currentFadeSample[track] / maxFadeSample[track]) + startFade[track];

							if (lastFade[track] > 1) {
								fading[track] = false;
								currentFadeSample[track] = 0;
								startFade[track] = 0;
								lastFade[track] = 0;
							} else {
								chan = std::max(inputs[IN1_INPUT+track].getChannels(), inputs[IN2_INPUT+track].getChannels());
								if (currentSwitch[track]) {
									for (int c = 0; c < chan; c++) {
										outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c) * (1-lastFade[track]), c);
										outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c) * lastFade[track], c);
									}
									outputs[OUT2_OUTPUT+track].setChannels(chan);
									outputs[OUT1_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(1-lastFade[track]);
								} else {
									for (int c = 0; c < chan; c++) {
										outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c) * lastFade[track], c);
										outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c) * (1-lastFade[track]), c);
									}
									outputs[OUT2_OUTPUT+track].setChannels(chan);
									outputs[OUT1_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(1-lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(lastFade[track]);
								}
							}
							currentFadeSample[track]++;
						} else {
							chan = std::max(inputs[IN1_INPUT+track].getChannels(), inputs[IN2_INPUT+track].getChannels());
							if (currentSwitch[track]) {
								for (int c = 0; c < chan; c++)
									outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c), c);
								outputs[OUT1_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									for (int c = 0; c < chan; c++)
										outputs[OUT2_OUTPUT+track].setVoltage(0.f, c);
									outputs[OUT2_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(1.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);									
								}
							} else {
								for (int c = 0; c < chan; c++)
									outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c), c);
								outputs[OUT2_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									for (int c = 0; c < chan; c++)
										outputs[OUT1_OUTPUT+track].setVoltage(0.f, c);
									outputs[OUT1_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(0.f);
									lights[IN2_LIGHT+track].setBrightness(1.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1.f);
								}
							}
						}
					break;

					case 15:						// IN1 + IN2 + OUT1 + OUT2 = 15
						if (inputs[RST_INPUT+track].isConnected()) {
							rst[track] = inputs[RST_INPUT+track].getVoltage();
							if (rst[track] >= 1 && prevRst[track] < 1) {
								if (currentSwitch[track]) {
									currentSwitch[track] = 0;
									connectionChange[track] = true;
									if (fadeValue[track] > noEnvTime) {
										if (fading[track]) {
											startFade[track] = 1-lastFade[track];
										} else {
											fading[track] = true;
											startFade[track] = 0;
										}
										currentFadeSample[track] = 0;
									}
								}
							}
							prevRst[track] = rst[track]; 
						}

						if (fading[track]) {

							maxFadeSample[track] = args.sampleRate * fadeValue[track];
							lastFade[track] = (currentFadeSample[track] / maxFadeSample[track]) + startFade[track];

							if (lastFade[track] > 1) {
								fading[track] = false;
								currentFadeSample[track] = 0;
								startFade[track] = 0;
								lastFade[track] = 0;
							} else {
								chan = std::max(inputs[IN1_INPUT+track].getChannels(), inputs[IN2_INPUT+track].getChannels());
								if (currentSwitch[track]) {
									for (int c = 0; c < chan; c++) {
										outputs[OUT1_OUTPUT+track].setVoltage((inputs[IN2_INPUT+track].getVoltage(c) * lastFade[track]) + (inputs[IN1_INPUT+track].getVoltage(c) * (1-lastFade[track])), c);
										outputs[OUT2_OUTPUT+track].setVoltage((inputs[IN1_INPUT+track].getVoltage(c) * lastFade[track]) + (inputs[IN2_INPUT+track].getVoltage(c) * (1-lastFade[track])), c);
									}
									outputs[OUT1_OUTPUT+track].setChannels(chan);
									outputs[OUT2_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(1-lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(lastFade[track]);
								} else {
									for (int c = 0; c < chan; c++) {
										outputs[OUT1_OUTPUT+track].setVoltage((inputs[IN1_INPUT+track].getVoltage(c) * lastFade[track]) + (inputs[IN2_INPUT+track].getVoltage(c) * (1-lastFade[track])), c);
										outputs[OUT2_OUTPUT+track].setVoltage((inputs[IN2_INPUT+track].getVoltage(c) * lastFade[track]) + (inputs[IN1_INPUT+track].getVoltage(c) * (1-lastFade[track])), c);
									}
									outputs[OUT1_OUTPUT+track].setChannels(chan);
									outputs[OUT2_OUTPUT+track].setChannels(chan);

									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(lastFade[track]);
									lights[OUT2_LIGHT+track].setBrightness(1-lastFade[track]);
								}
							}
							currentFadeSample[track]++;
						} else {
							chan = std::max(inputs[IN1_INPUT+track].getChannels(), inputs[IN2_INPUT+track].getChannels());
							if (currentSwitch[track]) {
								for (int c = 0; c < chan; c++) {
									outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c), c);
									outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c), c);
								}
								outputs[OUT1_OUTPUT+track].setChannels(chan);
								outputs[OUT2_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(0.f);
									lights[OUT2_LIGHT+track].setBrightness(1.f);
								}
							} else {
								for (int c = 0; c < chan; c++) {
									outputs[OUT1_OUTPUT+track].setVoltage(inputs[IN1_INPUT+track].getVoltage(c), c);
									outputs[OUT2_OUTPUT+track].setVoltage(inputs[IN2_INPUT+track].getVoltage(c), c);
								}
								outputs[OUT1_OUTPUT+track].setChannels(chan);
								outputs[OUT2_OUTPUT+track].setChannels(chan);

								if (connectionChange[track]) {
									connectionChange[track] = false;
									lights[IN1_LIGHT+track].setBrightness(1.f);
									lights[IN2_LIGHT+track].setBrightness(0.f);
									lights[OUT1_LIGHT+track].setBrightness(1.f);
									lights[OUT2_LIGHT+track].setBrightness(0.f);
								}
							}
						}
					break;

					default:
						if (connectionChange[track]) {
							outputs[OUT1_OUTPUT+track].setVoltage(0.f, 0);
							outputs[OUT1_OUTPUT+track].setChannels(1);
							outputs[OUT2_OUTPUT+track].setVoltage(0.f, 0);
							outputs[OUT2_OUTPUT+track].setChannels(1);
							lights[IN1_LIGHT+track].setBrightness(0.f);
							lights[IN2_LIGHT+track].setBrightness(0.f);
							lights[OUT1_LIGHT+track].setBrightness(0.f);
							lights[OUT2_LIGHT+track].setBrightness(0.f);
							connectionChange[track] = false;
						}
				}	
			}
			prevTrigConnection[track] = trigConnection[track];
		}
	}
};

struct Switcher8Widget : ModuleWidget {
	Switcher8Widget(Switcher8* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Switcher8.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		constexpr float yStart = 18.9f;
		constexpr float yDelta = 14.f;

		const float xTrg = 6.6f;
		const float xRst = 16.3f;

		const float xMode = 25.5f;
		
		const float xIn1 = 34.f;
		const float xIn2 = 44.f;
		
		const float xFade = 54.1f;

		const float xOut1 = 63.8f;
		const float xOut2 = 73.8f;

		constexpr float xLight = 4.38f;
		constexpr float yLight = 3.5f;

		for (int track = 0; track < 8; track++) {

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xTrg, yStart+(track*yDelta))), module, Switcher8::TRIG_INPUT+track));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRst, yStart+(track*yDelta))), module, Switcher8::RST_INPUT+track));

			addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xMode, yStart+(track*yDelta))), module, Switcher8::MODE_SWITCH+track, Switcher8::MODE_LIGHT+track));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xIn1, yStart+(track*yDelta))), module, Switcher8::IN1_INPUT+track));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xIn2, yStart+(track*yDelta))), module, Switcher8::IN2_INPUT+track));

			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(xIn1 + xLight, yStart+(track*yDelta)-yLight)), module, Switcher8::IN1_LIGHT+track));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(xIn2 + xLight, yStart+(track*yDelta)-yLight)), module, Switcher8::IN2_LIGHT+track));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xFade, yStart+(track*yDelta))), module, Switcher8::FADE_PARAM+track));

			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut1, yStart+(track*yDelta))), module, Switcher8::OUT1_OUTPUT+track));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut2, yStart+(track*yDelta))), module, Switcher8::OUT2_OUTPUT+track));

			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(xOut1 + xLight, yStart+(track*yDelta)-yLight)), module, Switcher8::OUT1_LIGHT+track));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(xOut2 + xLight, yStart+(track*yDelta)-yLight)), module, Switcher8::OUT2_LIGHT+track));
		}
	}

	void appendContextMenu(Menu* menu) override {
		Switcher8* module = dynamic_cast<Switcher8*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}
};

Model* modelSwitcher8 = createModel<Switcher8, Switcher8Widget>("Switcher8");