#include "plugin.hpp"

struct SwitcherSt : Module {
	enum ParamId {
		MODE_SWITCH,
		FADE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TRIG_INPUT,
		RST_INPUT,
		ENUMS(IN1_INPUT,2),
		ENUMS(IN2_INPUT,2),
		FADECV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT1_OUTPUT,2),
		ENUMS(OUT2_OUTPUT,2),
		OUTPUTS_LEN
	};
	enum LightId {
		IN1_LIGHT,
		IN2_LIGHT,
		OUT1_LIGHT,
		OUT2_LIGHT,
		LIGHTS_LEN
	};

	bool initStart = false;
	int mode = 1;
	int prevMode = 0;
	bool trigConnection = false;
	bool prevTrigConnection = false;
	int connection = 0;
	int prevConnection = -1;
	bool connectionChange = true;

	float rst = 0;
	float prevRst = 0;

	bool currentSwitch = false;
	bool trigState = false;
	float trigValue = 0;
	float prevTrigValue = 0;

	float fadeValue = 0;
	float cvFadeValue = 0;

	float maxFadeSample = 0;
	float currentFadeSample = 0;
	bool fading = false;
	float startFade = 0;
	float lastFade = 0;

	int chan;
	
	/*static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	const float maxAdsrTime = 10.f;*/
	const float noEnvTime = 0.00101;

	SwitcherSt() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(MODE_SWITCH, 0.f, 1.f, 1.f, "Mode", {"Gate", "Toggle"});
		//configParam(FADE_PARAM, 0.f, 1.f, 0.f, "Fade Time", "ms", maxStageTime / minStageTime, minStageTime);
		configParam(FADE_PARAM, 0.f, 1.f, 0.f, "Fade Time", "ms", 10000.f, 1.f);
		configInput(TRIG_INPUT, "Trig/Gate");
		configInput(RST_INPUT, "Reset");
		configInput(IN1_INPUT, "IN 1 Left");
		configInput(IN1_INPUT+1, "IN 1 Right");
		configInput(IN2_INPUT, "IN 2 Left");
		configInput(IN2_INPUT+1, "IN 2 Right");
		configInput(FADECV_INPUT, "Fade Time CV");
		configOutput(OUT1_OUTPUT, "OUT 1 Left");
		configOutput(OUT1_OUTPUT+1, "OUT 1 Right");
		configOutput(OUT2_OUTPUT, "OUT 2 Left");
		configOutput(OUT2_OUTPUT+1, "OUT 2 Right");
	}

	void onReset(const ResetEvent &e) override {
		initStart = false;
		mode = 1;
		prevMode = 0;
		trigConnection = false;
		prevTrigConnection = false;
		connection = 0;
		prevConnection = -1;
		connectionChange = true;
		currentSwitch = false;
		fading = false;
		outputs[OUT1_OUTPUT].setVoltage(0.f, 0);
		outputs[OUT1_OUTPUT].setChannels(1);
		outputs[OUT1_OUTPUT+1].setVoltage(0.f, 0);
		outputs[OUT1_OUTPUT+1].setChannels(1);
		outputs[OUT2_OUTPUT].setVoltage(0.f, 0);
		outputs[OUT2_OUTPUT].setChannels(1);
		outputs[OUT2_OUTPUT+1].setVoltage(0.f, 0);
		outputs[OUT2_OUTPUT+1].setChannels(1);
		lights[IN1_LIGHT].setBrightness(0.f);
		lights[IN2_LIGHT].setBrightness(0.f);
		lights[OUT1_LIGHT].setBrightness(0.f);
		lights[OUT2_LIGHT].setBrightness(0.f);
		Module::onReset(e);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "InitStart", json_boolean(initStart));
		json_object_set_new(rootJ, "State", json_boolean(currentSwitch));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* initStartJ = json_object_get(rootJ, "InitStart");
		if (initStartJ)
			initStart = json_boolean_value(initStartJ);

		if (!initStart) {
			json_t* jsonState = json_object_get(rootJ, "State");
			if (jsonState)
				currentSwitch = json_boolean_value(jsonState);
		}
	}

	/*static float convertCVToSeconds(float cv) {		
		return minStageTime * std::pow(maxStageTime / minStageTime, cv) / 1000;
	}*/

	void process(const ProcessArgs& args) override {
		trigConnection = inputs[TRIG_INPUT].isConnected();
		if (!trigConnection) {
			if (prevTrigConnection) {
				outputs[OUT1_OUTPUT].setVoltage(0.f, 0);
				outputs[OUT1_OUTPUT].setChannels(1);
				outputs[OUT1_OUTPUT+1].setVoltage(0.f, 0);
				outputs[OUT1_OUTPUT+1].setChannels(1);
				outputs[OUT2_OUTPUT].setVoltage(0.f, 0);
				outputs[OUT2_OUTPUT].setChannels(1);
				outputs[OUT2_OUTPUT+1].setVoltage(0.f, 0);
				outputs[OUT2_OUTPUT+1].setChannels(1);
				lights[IN1_LIGHT].setBrightness(0.f);
				lights[IN2_LIGHT].setBrightness(0.f);
				lights[OUT1_LIGHT].setBrightness(0.f);
				lights[OUT2_LIGHT].setBrightness(0.f);
				connectionChange = false;
				currentSwitch = false;
				prevConnection = -1;
			}
		} else {

			connection = 0;
			if (inputs[IN1_INPUT].isConnected())
				connection = 1;
			if (inputs[IN2_INPUT].isConnected())
				connection += 2;
			if (outputs[OUT1_OUTPUT].isConnected())
				connection += 4;
			if (outputs[OUT2_OUTPUT].isConnected())
				connection += 8;
			if (connection != prevConnection) {
				connectionChange = true;
				prevConnection = connection;
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
			
			mode = params[MODE_SWITCH].getValue();
			if (mode != prevMode) {
				connectionChange = true;
				if (mode == 0)
					currentSwitch = false;
				prevMode = mode;
			}

			trigValue = inputs[TRIG_INPUT].getVoltage();
			
			switch (mode) {
				// ************************************** GATE MODE **********
				case 0:
					if (trigValue >= 1 && prevTrigValue < 1) {
						currentSwitch = true;
						connectionChange = true;
						trigState = true;
					} else if (trigValue < 1 && prevTrigValue >= 1) {
						currentSwitch = false;
						connectionChange = true;
						trigState = true;
					} 
					prevTrigValue = trigValue;

					if (trigState) {
						//fadeValue = convertCVToSeconds(params[FADE_PARAM].getValue()) + inputs[FADECV_INPUT].getVoltage();
						fadeValue = (std::pow(10000.f, params[FADE_PARAM].getValue()) / 1000) + inputs[FADECV_INPUT].getVoltage();
						if (fadeValue > noEnvTime) {
							if (fading) {
								startFade = 1-lastFade;
							} else {
								fading = true;
								startFade = 0;
							}
							currentFadeSample = 0;
						}
						trigState = false;
					}
				break;
				// ************************************** TRIG MODE **********
				case 1:
					if (trigValue >= 1 && prevTrigValue < 1)
						trigState = true;
					else
						trigState = false;

					prevTrigValue = trigValue;

					if (trigState) {
						currentSwitch = !currentSwitch;
						connectionChange = true;
						//fadeValue = convertCVToSeconds(params[FADE_PARAM].getValue()) + inputs[FADECV_INPUT].getVoltage();
						fadeValue = (std::pow(10000.f, params[FADE_PARAM].getValue()) / 1000) + inputs[FADECV_INPUT].getVoltage();
						if (fadeValue > noEnvTime){
							if (fading) {
								startFade = 1-lastFade;
							} else {
								fading = true;
								startFade = 0;
							}
							currentFadeSample = 0;
						}
					}
				break;
			}

			switch (connection) {
				case 4: 										// OUT1 = 4
					if (inputs[RST_INPUT].isConnected()) {
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue > noEnvTime) {
									if (fading) {
										startFade = 1-lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							}
						}
						prevRst = rst; 
					}

					if (fading) {
						if (fadeValue > 10)
							fadeValue = 10;
						else if (fadeValue < 0)
							fadeValue = 0;

						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT1_OUTPUT].setVoltage(10.f * lastFade, 0);
								outputs[OUT1_OUTPUT].setChannels(1);
								outputs[OUT1_OUTPUT+1].setVoltage(10.f * lastFade, 0);
								outputs[OUT1_OUTPUT+1].setChannels(1);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);
							} else {
								outputs[OUT1_OUTPUT].setVoltage(10.f * (1-lastFade), 0);
								outputs[OUT1_OUTPUT].setChannels(1);
								outputs[OUT1_OUTPUT+1].setVoltage(10.f * (1-lastFade), 0);
								outputs[OUT1_OUTPUT+1].setChannels(1);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT1_OUTPUT].setVoltage(10.f, 0);
							outputs[OUT1_OUTPUT].setChannels(1);
							outputs[OUT1_OUTPUT+1].setVoltage(10.f, 0);
							outputs[OUT1_OUTPUT+1].setChannels(1);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							outputs[OUT1_OUTPUT].setVoltage(0.f, 0);
							outputs[OUT1_OUTPUT].setChannels(1);
							outputs[OUT1_OUTPUT+1].setVoltage(0.f, 0);
							outputs[OUT1_OUTPUT+1].setChannels(1);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(0.f);								
							}
						}
					}
				break;
				
				case 8:											// OUT2 = 8
					if (inputs[RST_INPUT].isConnected()) {
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue > noEnvTime) {
									if (fading) {
										startFade = 1-lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							}
						}
						prevRst = rst; 
					}

					if (fading) {
						if (fadeValue > 10)
							fadeValue = 10;
						else if (fadeValue < 0)
							fadeValue = 0;

						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT2_OUTPUT].setVoltage(10.f * (1-lastFade), 0);
								outputs[OUT2_OUTPUT].setChannels(1);
								outputs[OUT2_OUTPUT+1].setVoltage(10.f * (1-lastFade), 0);
								outputs[OUT2_OUTPUT+1].setChannels(1);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							} else {
								outputs[OUT2_OUTPUT].setVoltage(10.f * lastFade, 0);
								outputs[OUT2_OUTPUT].setChannels(1);
								outputs[OUT2_OUTPUT+1].setVoltage(10.f * lastFade, 0);
								outputs[OUT2_OUTPUT+1].setChannels(1);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(lastFade);								
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT2_OUTPUT].setVoltage(0.f, 0);
							outputs[OUT2_OUTPUT].setChannels(1);
							outputs[OUT2_OUTPUT+1].setVoltage(0.f, 0);
							outputs[OUT2_OUTPUT+1].setChannels(1);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							outputs[OUT2_OUTPUT].setVoltage(10.f, 0);
							outputs[OUT2_OUTPUT].setChannels(1);
							outputs[OUT2_OUTPUT+1].setVoltage(10.f, 0);
							outputs[OUT2_OUTPUT+1].setChannels(1);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}							
						}
					}
				break;

				case 12: 										// OUT1 + OUT2 = 12
					if (inputs[RST_INPUT].isConnected()) {
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue > noEnvTime) {
									if (fading) {
										startFade = 1-lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							}
						}
						prevRst = rst; 
					}

					if (fading) {
						if (fadeValue > 10)
							fadeValue = 10;
						else if (fadeValue < 0)
							fadeValue = 0;

						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT1_OUTPUT].setVoltage(10.f * (1-lastFade), 0);
								outputs[OUT1_OUTPUT].setChannels(1);
								outputs[OUT1_OUTPUT+1].setVoltage(10.f * (1-lastFade), 0);
								outputs[OUT1_OUTPUT+1].setChannels(1);

								outputs[OUT2_OUTPUT].setVoltage(10.f * lastFade, 0);
								outputs[OUT2_OUTPUT].setChannels(1);
								outputs[OUT2_OUTPUT+1].setVoltage(10.f * lastFade, 0);
								outputs[OUT2_OUTPUT+1].setChannels(1);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(lastFade);
							} else {
								outputs[OUT1_OUTPUT].setVoltage(10.f * lastFade, 0);
								outputs[OUT1_OUTPUT].setChannels(1);
								outputs[OUT1_OUTPUT+1].setVoltage(10.f * lastFade, 0);
								outputs[OUT1_OUTPUT+1].setChannels(1);

								outputs[OUT2_OUTPUT].setVoltage(10.f * (1-lastFade));
								outputs[OUT2_OUTPUT].setChannels(1);
								outputs[OUT2_OUTPUT+1].setVoltage(10.f * (1-lastFade));
								outputs[OUT2_OUTPUT+1].setChannels(1);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT1_OUTPUT].setVoltage(0.f, 0);
							outputs[OUT1_OUTPUT].setChannels(1);
							outputs[OUT1_OUTPUT+1].setVoltage(0.f, 0);
							outputs[OUT1_OUTPUT+1].setChannels(1);
							
							outputs[OUT2_OUTPUT].setVoltage(10.f, 0);
							outputs[OUT2_OUTPUT].setChannels(1);
							outputs[OUT2_OUTPUT+1].setVoltage(10.f, 0);
							outputs[OUT2_OUTPUT+1].setChannels(1);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						} else {
							outputs[OUT1_OUTPUT].setVoltage(10.f, 0);
							outputs[OUT1_OUTPUT].setChannels(1);
							outputs[OUT1_OUTPUT+1].setVoltage(10.f, 0);
							outputs[OUT1_OUTPUT+1].setChannels(1);

							outputs[OUT2_OUTPUT].setVoltage(0.f, 0);
							outputs[OUT2_OUTPUT].setChannels(1);
							outputs[OUT2_OUTPUT+1].setVoltage(0.f, 0);
							outputs[OUT2_OUTPUT+1].setChannels(1);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						}
					}
				break;

				case 5:											// IN1 + OUT1 = 5
					if (inputs[RST_INPUT].isConnected()) {
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue > noEnvTime) {
									if (fading) {
										startFade = 1-lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							}
						}
						prevRst = rst; 
					}

					if (fading) {
						if (fadeValue > 10)
							fadeValue = 10;
						else if (fadeValue < 0)
							fadeValue = 0;

						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							chan = std::max(1, inputs[IN1_INPUT].getChannels());
							if (currentSwitch) {
								for (int c = 0; c < chan; c++) {
									outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * lastFade, c);
									outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c) * lastFade, c);
								}
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);
							} else {
								for (int c = 0; c < chan; c++) {
									outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * (1-lastFade), c);
									outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c) * (1-lastFade), c);
								}
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);								
							}
						}
						currentFadeSample++;
					} else {
						chan = std::max(1, inputs[IN1_INPUT].getChannels());
						if (currentSwitch) {
							for (int c = 0; c < chan; c++) {
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c), c);
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT1_OUTPUT].setChannels(chan);
							outputs[OUT1_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							for (int c = 0; c < chan; c++) {
								outputs[OUT1_OUTPUT].setVoltage(0.f, c);
								outputs[OUT1_OUTPUT+1].setVoltage(0.f, c);
							}
							outputs[OUT1_OUTPUT].setChannels(chan);
							outputs[OUT1_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}							
						}
					}
				break;
				
				case 10:											// IN2 + OUT2 = 10
					if (inputs[RST_INPUT].isConnected()) {
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue > noEnvTime) {
									if (fading) {
										startFade = 1-lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							}
						}
						prevRst = rst; 
					}

					if (fading) {
						if (fadeValue > 10)
							fadeValue = 10;
						else if (fadeValue < 0)
							fadeValue = 0;

						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							chan = std::max(1, inputs[IN2_INPUT].getChannels());
							if (currentSwitch) {
								for (int c = 0; c < chan; c++) {
									outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c) * lastFade, c);
									outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c) * lastFade, c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(lastFade);
							} else {
								for (int c = 0; c < chan; c++) {
									outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c) * (1-lastFade), c);
									outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c) * (1-lastFade), c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);								
							}
						}
						currentFadeSample++;
					} else {
						chan = std::max(1, inputs[IN2_INPUT].getChannels());
						if (currentSwitch) {
							for (int c = 0; c < chan; c++) {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c), c);
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT2_OUTPUT].setChannels(chan);
							outputs[OUT2_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						} else {
							for (int c = 0; c < chan; c++) {
								outputs[OUT2_OUTPUT].setVoltage(0.f, c);
								outputs[OUT2_OUTPUT+1].setVoltage(0.f, c);
							}
							outputs[OUT2_OUTPUT].setChannels(chan);
							outputs[OUT2_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}							
						}
					}
				break;

				case 9:											// IN1 + OUT2 = 9
					if (inputs[RST_INPUT].isConnected()) {
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue > noEnvTime) {
									if (fading) {
										startFade = 1-lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							}
						}
						prevRst = rst; 
					}

					if (fading) {
						if (fadeValue > 10)
							fadeValue = 10;
						else if (fadeValue < 0)
							fadeValue = 0;

						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							chan = std::max(1, inputs[IN1_INPUT].getChannels());
							if (currentSwitch) {
								for (int c = 0; c < chan; c++) {
									outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * (1-lastFade), c);
									outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c) * (1-lastFade), c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							} else {
								for (int c = 0; c < chan; c++) {
									outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * lastFade, c);
									outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c) * lastFade, c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(lastFade);								
							}
						}
						currentFadeSample++;
					} else {
						chan = std::max(1, inputs[IN1_INPUT].getChannels());
						if (currentSwitch) {
							for (int c = 0; c < chan; c++) {
								outputs[OUT2_OUTPUT].setVoltage(0.f, c);
								outputs[OUT2_OUTPUT+1].setVoltage(0.f, c);
							}
							outputs[OUT2_OUTPUT].setChannels(chan);
							outputs[OUT2_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							for (int c = 0; c < chan; c++) {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c), c);
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT2_OUTPUT].setChannels(chan);
							outputs[OUT2_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}							
						}
					}
				break;

				case 6:											// IN2 + OUT1 = 6
					if (inputs[RST_INPUT].isConnected()) {
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue > noEnvTime) {
									if (fading) {
										startFade = 1-lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							}
						}
						prevRst = rst; 
					}

					if (fading) {
						if (fadeValue > 10)
							fadeValue = 10;
						else 	if (fadeValue < 0)
							fadeValue = 0;

						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							chan = std::max(1, inputs[IN2_INPUT].getChannels());
							if (currentSwitch) {
								for (int c = 0; c < chan; c++) {
									outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c) * (1-lastFade), c);
									outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c) * (1-lastFade), c);
								}
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);
							} else {
								for (int c = 0; c < chan; c++) {
									outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c) * lastFade, c);
									outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c) * lastFade, c);
								}
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);								
							}
						}
						currentFadeSample++;
					} else {
						chan = std::max(1, inputs[IN2_INPUT].getChannels());
						if (currentSwitch) {
							for (int c = 0; c < chan; c++) {
								outputs[OUT1_OUTPUT].setVoltage(0.f, c);
								outputs[OUT1_OUTPUT+1].setVoltage(0.f, c);
							}
							outputs[OUT1_OUTPUT].setChannels(chan);
							outputs[OUT1_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							for (int c = 0; c < chan; c++) {
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c), c);
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT1_OUTPUT].setChannels(chan);
							outputs[OUT1_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}							
						}
					}
				break;

				case 7:													// IN1 + IN2 + OUT1 = 7
					if (inputs[RST_INPUT].isConnected()) {
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue > noEnvTime) {
									if (fading) {
										startFade = 1-lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							}
						}
						prevRst = rst; 
					}

					if (fading) {
						if (fadeValue > 10)
							fadeValue = 10;
						else if (fadeValue < 0)
							fadeValue = 0;

						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							chan = std::max(inputs[IN1_INPUT].getChannels(), inputs[IN2_INPUT].getChannels());
							if (currentSwitch) {
								for (int c = 0; c < chan; c++) {
									outputs[OUT1_OUTPUT].setVoltage((inputs[IN2_INPUT].getVoltage(c) * lastFade) + (inputs[IN1_INPUT].getVoltage(c) * (1-lastFade)), c);
									outputs[OUT1_OUTPUT+1].setVoltage((inputs[IN2_INPUT+1].getVoltage(c) * lastFade) + (inputs[IN1_INPUT+1].getVoltage(c) * (1-lastFade)), c);
								}
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(1-lastFade);
								lights[IN2_LIGHT].setBrightness(lastFade);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							} else {
								for (int c = 0; c < chan; c++) {
									outputs[OUT1_OUTPUT].setVoltage((inputs[IN1_INPUT].getVoltage(c) * lastFade) + (inputs[IN2_INPUT].getVoltage(c) * (1-lastFade)), c);
									outputs[OUT1_OUTPUT+1].setVoltage((inputs[IN1_INPUT+1].getVoltage(c) * lastFade) + (inputs[IN2_INPUT+1].getVoltage(c) * (1-lastFade)), c);
								}
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(lastFade);
								lights[IN2_LIGHT].setBrightness(1-lastFade);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						}
						currentFadeSample++;
					} else {
						chan = std::max(inputs[IN1_INPUT].getChannels(), inputs[IN2_INPUT].getChannels());
						if (currentSwitch) {
							for (int c = 0; c < chan; c++) {
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c), c);
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT1_OUTPUT].setChannels(chan);
							outputs[OUT1_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							for (int c = 0; c < chan; c++) {
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c), c);
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT1_OUTPUT].setChannels(chan);
							outputs[OUT1_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						}
					}
				break;

				case 11:												// IN1 + IN2 + OUT2 = 11
					if (inputs[RST_INPUT].isConnected()) {
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue > noEnvTime) {
									if (fading) {
										startFade = 1-lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							}
						}
						prevRst = rst; 
					}

					if (fading) {
						if (fadeValue > 10)
							fadeValue = 10;
						else if (fadeValue < 0)
							fadeValue = 0;

						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							chan = std::max(inputs[IN1_INPUT].getChannels(), inputs[IN2_INPUT].getChannels());
							if (currentSwitch) {
								for (int c = 0; c < chan; c++) {
									outputs[OUT2_OUTPUT].setVoltage((inputs[IN1_INPUT].getVoltage(c) * lastFade) + (inputs[IN2_INPUT].getVoltage(c) * (1-lastFade)), c);
									outputs[OUT2_OUTPUT+1].setVoltage((inputs[IN1_INPUT+1].getVoltage(c) * lastFade) + (inputs[IN2_INPUT+1].getVoltage(c) * (1-lastFade)), c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(lastFade);
								lights[IN2_LIGHT].setBrightness(1-lastFade);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							} else {
								for (int c = 0; c < chan; c++) {
									outputs[OUT2_OUTPUT].setVoltage((inputs[IN2_INPUT].getVoltage(c) * lastFade) + (inputs[IN1_INPUT].getVoltage(c) * (1-lastFade)), c);
									outputs[OUT2_OUTPUT+1].setVoltage((inputs[IN2_INPUT+1].getVoltage(c) * lastFade) + (inputs[IN1_INPUT+1].getVoltage(c) * (1-lastFade)), c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(1-lastFade);
								lights[IN2_LIGHT].setBrightness(lastFade);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						}
						currentFadeSample++;
					} else {
						chan = std::max(inputs[IN1_INPUT].getChannels(), inputs[IN2_INPUT].getChannels());
						if (currentSwitch) {
							for (int c = 0; c < chan; c++) {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c), c);
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT2_OUTPUT].setChannels(chan);
							outputs[OUT2_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						} else {
							for (int c = 0; c < chan; c++) {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c), c);
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT2_OUTPUT].setChannels(chan);
							outputs[OUT2_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						}
					}
				break;

				case 13:											// IN1 + OUT1 + OUT2 = 13
					if (inputs[RST_INPUT].isConnected()) {
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue > noEnvTime) {
									if (fading) {
										startFade = 1-lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							}
						}
						prevRst = rst; 
					}

					if (fading) {
						if (fadeValue > 10)
							fadeValue = 10;
						else if (fadeValue < 0)
							fadeValue = 0;

						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							chan = std::max(1, inputs[IN1_INPUT].getChannels());
							if (currentSwitch) {
								for (int c = 0; c < chan; c++) {
									outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * lastFade, c);
									outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c) * lastFade, c);
									outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * (1-lastFade), c);
									outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c) * (1-lastFade), c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(lastFade);
							} else {
								for (int c = 0; c < chan; c++) {
									outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * (1-lastFade), c);
									outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c) * (1-lastFade), c);
									outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * lastFade, c);
									outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c) * lastFade, c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							}
						}
						currentFadeSample++;
					} else {
						chan = std::max(1, inputs[IN1_INPUT].getChannels());
						if (currentSwitch) {
							for (int c = 0; c < chan; c++) {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c), c);
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT2_OUTPUT].setChannels(chan);
							outputs[OUT2_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								for (int c = 0; c < chan; c++) {
									outputs[OUT1_OUTPUT].setVoltage(0.f, c);
									outputs[OUT1_OUTPUT+1].setVoltage(0.f, c);
								}
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						} else {
							for (int c = 0; c < chan; c++) {
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c), c);
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT1_OUTPUT].setChannels(chan);
							outputs[OUT1_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								for (int c = 0; c < chan; c++) {
									outputs[OUT2_OUTPUT].setVoltage(0.f, c);
									outputs[OUT2_OUTPUT+1].setVoltage(0.f, c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						}
					}
				break;

				case 14:										// IN2 + OUT1 + OUT2 = 14
					if (inputs[RST_INPUT].isConnected()) {
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue > noEnvTime) {
									if (fading) {
										startFade = 1-lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							}
						}
						prevRst = rst; 
					}

					if (fading) {
						if (fadeValue > 10)
							fadeValue = 10;
						else if (fadeValue < 0)
							fadeValue = 0;

						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							chan = std::max(1, inputs[IN2_INPUT].getChannels());
							if (currentSwitch) {
								for (int c = 0; c < chan; c++) {
									outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c) * (1-lastFade), c);
									outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c) * (1-lastFade), c);
									outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c) * lastFade, c);
									outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c) * lastFade, c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							} else {
								for (int c = 0; c < chan; c++) {
									outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c) * lastFade, c);
									outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c) * lastFade, c);
									outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c) * (1-lastFade), c);
									outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c) * (1-lastFade), c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(lastFade);
							}
						}
						currentFadeSample++;
					} else {
						chan = std::max(1, inputs[IN2_INPUT].getChannels());
						if (currentSwitch) {
							for (int c = 0; c < chan; c++) {
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c), c);
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT1_OUTPUT].setChannels(chan);
							outputs[OUT1_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								for (int c = 0; c < chan; c++) {
									outputs[OUT2_OUTPUT].setVoltage(0.f, c);
									outputs[OUT2_OUTPUT+1].setVoltage(0.f, c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);									
							}
						} else {
							for (int c = 0; c < chan; c++) {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c), c);
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT2_OUTPUT].setChannels(chan);
							outputs[OUT2_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								for (int c = 0; c < chan; c++) {
									outputs[OUT1_OUTPUT].setVoltage(0.f, c);
									outputs[OUT1_OUTPUT+1].setVoltage(0.f, c);
								}
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						}
					}
				break;

				case 15:						// IN1 + IN2 + OUT1 + OUT2 = 15
					if (inputs[RST_INPUT].isConnected()) {
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue > noEnvTime) {
									if (fading) {
										startFade = 1-lastFade;
									} else {
										fading = true;
										startFade = 0;
									}
									currentFadeSample = 0;
								}
							}
						}
						prevRst = rst; 
					}

					if (fading) {
						if (fadeValue > 10)
							fadeValue = 10;
						else if (fadeValue < 0)
							fadeValue = 0;

						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							chan = std::max(inputs[IN1_INPUT].getChannels(), inputs[IN2_INPUT].getChannels());
							if (currentSwitch) {
								for (int c = 0; c < chan; c++) {
									outputs[OUT1_OUTPUT].setVoltage((inputs[IN2_INPUT].getVoltage(c) * lastFade) + (inputs[IN1_INPUT].getVoltage(c) * (1-lastFade)), c);
									outputs[OUT1_OUTPUT+1].setVoltage((inputs[IN2_INPUT+1].getVoltage(c) * lastFade) + (inputs[IN1_INPUT+1].getVoltage(c) * (1-lastFade)), c);
									outputs[OUT2_OUTPUT].setVoltage((inputs[IN1_INPUT].getVoltage(c) * lastFade) + (inputs[IN2_INPUT].getVoltage(c) * (1-lastFade)), c);
									outputs[OUT2_OUTPUT+1].setVoltage((inputs[IN1_INPUT+1].getVoltage(c) * lastFade) + (inputs[IN2_INPUT+1].getVoltage(c) * (1-lastFade)), c);
								}
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(lastFade);
							} else {
								for (int c = 0; c < chan; c++) {
									outputs[OUT1_OUTPUT].setVoltage((inputs[IN1_INPUT].getVoltage(c) * lastFade) + (inputs[IN2_INPUT].getVoltage(c) * (1-lastFade)), c);
									outputs[OUT1_OUTPUT+1].setVoltage((inputs[IN1_INPUT+1].getVoltage(c) * lastFade) + (inputs[IN2_INPUT+1].getVoltage(c) * (1-lastFade)), c);
									outputs[OUT2_OUTPUT].setVoltage((inputs[IN2_INPUT].getVoltage(c) * lastFade) + (inputs[IN1_INPUT].getVoltage(c) * (1-lastFade)), c);
									outputs[OUT2_OUTPUT+1].setVoltage((inputs[IN2_INPUT+1].getVoltage(c) * lastFade) + (inputs[IN1_INPUT+1].getVoltage(c) * (1-lastFade)), c);
								}
								outputs[OUT1_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT+1].setChannels(chan);
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT2_OUTPUT+1].setChannels(chan);

								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							}
						}
						currentFadeSample++;
					} else {
						chan = std::max(inputs[IN1_INPUT].getChannels(), inputs[IN2_INPUT].getChannels());
						if (currentSwitch) {
							for (int c = 0; c < chan; c++) {
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c), c);
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c), c);
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c), c);
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT1_OUTPUT].setChannels(chan);
							outputs[OUT1_OUTPUT+1].setChannels(chan);
							outputs[OUT2_OUTPUT].setChannels(chan);
							outputs[OUT2_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						} else {
							for (int c = 0; c < chan; c++) {
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c), c);
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage(c), c);
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage(c), c);
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage(c), c);
							}
							outputs[OUT1_OUTPUT].setChannels(chan);
							outputs[OUT1_OUTPUT+1].setChannels(chan);
							outputs[OUT2_OUTPUT].setChannels(chan);
							outputs[OUT2_OUTPUT+1].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						}
					}
				break;

				default:
					if (connectionChange) {
						outputs[OUT1_OUTPUT].setVoltage(0.f, 0);
						outputs[OUT1_OUTPUT].setChannels(1);
						outputs[OUT1_OUTPUT+1].setVoltage(0.f, 0);
						outputs[OUT1_OUTPUT+1].setChannels(1);
						outputs[OUT2_OUTPUT].setVoltage(0.f, 0);
						outputs[OUT2_OUTPUT].setChannels(1);
						outputs[OUT2_OUTPUT+1].setVoltage(0.f, 0);
						outputs[OUT2_OUTPUT+1].setChannels(1);
						lights[IN1_LIGHT].setBrightness(0.f);
						lights[IN2_LIGHT].setBrightness(0.f);
						lights[OUT1_LIGHT].setBrightness(0.f);
						lights[OUT2_LIGHT].setBrightness(0.f);
						connectionChange = false;
					}
			}
		}
		prevTrigConnection = trigConnection;
	}
};

struct SwitcherStWidget : ModuleWidget {
	SwitcherStWidget(SwitcherSt* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SwitcherSt.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<CKSS>(mm2px(Vec(4, 11.35)), module, SwitcherSt::MODE_SWITCH));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(10.16, 26)), module, SwitcherSt::TRIG_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(10.16, 39)), module, SwitcherSt::RST_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(6, 55.5)), module, SwitcherSt::IN1_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(14.3, 55.5)), module, SwitcherSt::IN1_INPUT+1));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(6, 64.5)), module, SwitcherSt::IN2_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(14.3, 64.5)), module, SwitcherSt::IN2_INPUT+1));

		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(10.16, 52)), module, SwitcherSt::IN1_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(10.16, 61)), module, SwitcherSt::IN2_LIGHT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(10.16, 82.9)), module, SwitcherSt::FADE_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(10.16, 92.5)), module, SwitcherSt::FADECV_INPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(6, 109)), module, SwitcherSt::OUT1_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(14.3, 109)), module, SwitcherSt::OUT1_OUTPUT+1));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(6, 118)), module, SwitcherSt::OUT2_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(14.3, 118)), module, SwitcherSt::OUT2_OUTPUT+1));

		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(10.16, 105.5)), module, SwitcherSt::OUT1_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(10.16, 114.5)), module, SwitcherSt::OUT2_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		SwitcherSt* module = dynamic_cast<SwitcherSt*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}
};

Model* modelSwitcherSt = createModel<SwitcherSt, SwitcherStWidget>("SwitcherSt");