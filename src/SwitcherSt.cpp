#include "plugin.hpp"

struct SwitcherSt : Module {
	bool stateRestore = true;
	int mode = 1;
	int prevMode = 0;
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
	
	enum ParamId {
		MODE_SWITCH,
		FADE_PARAMS,
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

	SwitcherSt() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(MODE_SWITCH, 0.f, 1.f, 1.f, "Mode", {"Gate", "Toggle"});
		configParam(FADE_PARAMS, 0.f, 10.f, 0.f, "Fade Time (s)");
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

	void onReset() override {
		stateRestore = true;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "StateRestore", json_boolean(stateRestore));
		json_object_set_new(rootJ, "State", json_boolean(currentSwitch));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* jsonStateRestore = json_object_get(rootJ, "StateRestore");
		if (jsonStateRestore)
			stateRestore = json_boolean_value(jsonStateRestore);

		if (stateRestore) {
			json_t* jsonState = json_object_get(rootJ, "State");
			if (jsonState)
				currentSwitch = json_boolean_value(jsonState);
		}
	}

	void process(const ProcessArgs& args) override {
		if (!inputs[TRIG_INPUT].isConnected()){
			outputs[OUT1_OUTPUT].setVoltage(0);
			outputs[OUT2_OUTPUT].setVoltage(0);
			lights[IN1_LIGHT].setBrightness(0.f);
			lights[IN2_LIGHT].setBrightness(0.f);
			lights[OUT1_LIGHT].setBrightness(0.f);
			lights[OUT2_LIGHT].setBrightness(0.f);
			connectionChange = false;
			currentSwitch = false;
			prevConnection = -1;
		} else {
			fadeValue = params[FADE_PARAMS].getValue() + inputs[FADECV_INPUT].getVoltage();

			connection = 0;
			if (inputs[IN1_INPUT].isConnected()){
				connection = 1;
			}
			if (inputs[IN2_INPUT].isConnected()){
				connection += 2;
			}
			if (outputs[OUT1_OUTPUT].isConnected()){
				connection += 4;
			}
			if (outputs[OUT2_OUTPUT].isConnected()){
				connection += 8;
			}
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
			// IN1 + IN2 + OUT1 = 7  *OK
			// IN1 + IN2 + OUT2 = 11  *OK
			// IN1 + OUT1 + OUT2 = 13 *OK
			// IN2 + OUT1 + OUT2 = 14 *OK
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
					if (trigValue >= 1 && prevTrigValue < 1){
						currentSwitch = true;
						connectionChange = true;
						trigState = true;
					} else if (trigValue < 1 && prevTrigValue >= 1){
						currentSwitch = false;
						connectionChange = true;
						trigState = true;
					} 
					prevTrigValue = trigValue;

					if (trigState) {
						if (fadeValue != 0){
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
					if (trigValue >= 1 && prevTrigValue < 1){
						trigState = true;
					} else {
						trigState = false;
					}
					prevTrigValue = trigValue;

					if (trigState) {
						currentSwitch = !currentSwitch;
						connectionChange = true;
						if (fadeValue != 0){
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
					if (inputs[RST_INPUT].isConnected()){
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue != 0) {
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
						if (fadeValue > 10) {
							fadeValue = 10;
						} else 	if (fadeValue < 0) {
							fadeValue = 0;
						}
						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT1_OUTPUT].setVoltage(10 * lastFade);
								outputs[OUT1_OUTPUT+1].setVoltage(10 * lastFade);
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);
							} else {
								outputs[OUT1_OUTPUT].setVoltage(10 * (1-lastFade));
								outputs[OUT1_OUTPUT+1].setVoltage(10 * (1-lastFade));
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT1_OUTPUT].setVoltage(10);
							outputs[OUT1_OUTPUT+1].setVoltage(10);
							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							outputs[OUT1_OUTPUT].setVoltage(0);
							outputs[OUT1_OUTPUT+1].setVoltage(0);
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
					if (inputs[RST_INPUT].isConnected()){
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue != 0) {
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
						if (fadeValue > 10) {
							fadeValue = 10;
						} else 	if (fadeValue < 0) {
							fadeValue = 0;
						}
						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT2_OUTPUT].setVoltage(10 * (1-lastFade));
								outputs[OUT2_OUTPUT+1].setVoltage(10 * (1-lastFade));
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							} else {
								outputs[OUT2_OUTPUT].setVoltage(10 * lastFade);
								outputs[OUT2_OUTPUT+1].setVoltage(10 * lastFade);
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(lastFade);								
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT2_OUTPUT].setVoltage(0);
							outputs[OUT2_OUTPUT+1].setVoltage(0);
							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							outputs[OUT2_OUTPUT].setVoltage(10);
							outputs[OUT2_OUTPUT+1].setVoltage(10);
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
					if (inputs[RST_INPUT].isConnected()){
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue != 0) {
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
						if (fadeValue > 10) {
							fadeValue = 10;
						} else 	if (fadeValue < 0) {
							fadeValue = 0;
						}
						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT1_OUTPUT].setVoltage(10 * (1-lastFade));
								outputs[OUT1_OUTPUT+1].setVoltage(10 * (1-lastFade));
								outputs[OUT2_OUTPUT].setVoltage(10 * lastFade);
								outputs[OUT2_OUTPUT+1].setVoltage(10 * lastFade);
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(lastFade);
							} else {
								outputs[OUT1_OUTPUT].setVoltage(10 * lastFade);
								outputs[OUT1_OUTPUT+1].setVoltage(10 * lastFade);
								outputs[OUT2_OUTPUT].setVoltage(10 * (1-lastFade));
								outputs[OUT2_OUTPUT+1].setVoltage(10 * (1-lastFade));
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT1_OUTPUT].setVoltage(0);
							outputs[OUT1_OUTPUT+1].setVoltage(0);
							outputs[OUT2_OUTPUT].setVoltage(10);
							outputs[OUT2_OUTPUT+1].setVoltage(10);
							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						} else {
							outputs[OUT1_OUTPUT].setVoltage(10);
							outputs[OUT1_OUTPUT+1].setVoltage(10);
							outputs[OUT2_OUTPUT].setVoltage(0);
							outputs[OUT2_OUTPUT+1].setVoltage(0);
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
					if (inputs[RST_INPUT].isConnected()){
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue != 0) {
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
						if (fadeValue > 10) {
							fadeValue = 10;
						} else 	if (fadeValue < 0) {
							fadeValue = 0;
						}
						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage() * lastFade);
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage() * lastFade);
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);
							} else {
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage() * (1-lastFade));
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage() * (1-lastFade));
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);								
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage());
							outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage());
							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							outputs[OUT1_OUTPUT].setVoltage(0);
							outputs[OUT1_OUTPUT+1].setVoltage(0);
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
					if (inputs[RST_INPUT].isConnected()){
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue != 0) {
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
						if (fadeValue > 10) {
							fadeValue = 10;
						} else 	if (fadeValue < 0) {
							fadeValue = 0;
						}
						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage() * lastFade);
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage() * lastFade);
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(lastFade);
							} else {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage() * (1-lastFade));
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage() * (1-lastFade));
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);								
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage());
							outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage());
							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						} else {
							outputs[OUT2_OUTPUT].setVoltage(0);
							outputs[OUT2_OUTPUT+1].setVoltage(0);
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
					if (inputs[RST_INPUT].isConnected()){
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue != 0) {
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
						if (fadeValue > 10) {
							fadeValue = 10;
						} else 	if (fadeValue < 0) {
							fadeValue = 0;
						}
						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage() * (1-lastFade));
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage() * (1-lastFade));
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							} else {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage() * lastFade);
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage() * lastFade);
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(lastFade);								
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT2_OUTPUT].setVoltage(0);
							outputs[OUT2_OUTPUT+1].setVoltage(0);
							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage());
							outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage());
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
					if (inputs[RST_INPUT].isConnected()){
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue != 0) {
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
						if (fadeValue > 10) {
							fadeValue = 10;
						} else 	if (fadeValue < 0) {
							fadeValue = 0;
						}
						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage() * (1-lastFade));
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage() * (1-lastFade));
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);
							} else {
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage() * lastFade);
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage() * lastFade);
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);								
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT1_OUTPUT].setVoltage(0);
							outputs[OUT1_OUTPUT+1].setVoltage(0);
							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage());
							outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage());
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
					if (inputs[RST_INPUT].isConnected()){
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue != 0) {
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
						if (fadeValue > 10) {
							fadeValue = 10;
						} else 	if (fadeValue < 0) {
							fadeValue = 0;
						}
						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT1_OUTPUT].setVoltage( (inputs[IN2_INPUT].getVoltage() * lastFade) + (inputs[IN1_INPUT].getVoltage() * (1-lastFade)) );
								outputs[OUT1_OUTPUT+1].setVoltage( (inputs[IN2_INPUT+1].getVoltage() * lastFade) + (inputs[IN1_INPUT+1].getVoltage() * (1-lastFade)) );
								lights[IN1_LIGHT].setBrightness(1-lastFade);
								lights[IN2_LIGHT].setBrightness(lastFade);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							} else {
								outputs[OUT1_OUTPUT].setVoltage( (inputs[IN1_INPUT].getVoltage() * lastFade) + (inputs[IN2_INPUT].getVoltage() * (1-lastFade)) );
								outputs[OUT1_OUTPUT+1].setVoltage( (inputs[IN1_INPUT+1].getVoltage() * lastFade) + (inputs[IN2_INPUT+1].getVoltage() * (1-lastFade)) );
								lights[IN1_LIGHT].setBrightness(lastFade);
								lights[IN2_LIGHT].setBrightness(1-lastFade);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage());
							outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage());
							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage());
							outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage());
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
					if (inputs[RST_INPUT].isConnected()){
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue != 0) {
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
						if (fadeValue > 10) {
							fadeValue = 10;
						} else 	if (fadeValue < 0) {
							fadeValue = 0;
						}
						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT2_OUTPUT].setVoltage( (inputs[IN1_INPUT].getVoltage() * lastFade) + (inputs[IN2_INPUT].getVoltage() * (1-lastFade)) );
								outputs[OUT2_OUTPUT+1].setVoltage( (inputs[IN1_INPUT+1].getVoltage() * lastFade) + (inputs[IN2_INPUT+1].getVoltage() * (1-lastFade)) );
								lights[IN1_LIGHT].setBrightness(lastFade);
								lights[IN2_LIGHT].setBrightness(1-lastFade);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							} else {
								outputs[OUT2_OUTPUT].setVoltage( (inputs[IN2_INPUT].getVoltage() * lastFade) + (inputs[IN1_INPUT].getVoltage() * (1-lastFade)) );
								outputs[OUT2_OUTPUT+1].setVoltage( (inputs[IN2_INPUT+1].getVoltage() * lastFade) + (inputs[IN1_INPUT+1].getVoltage() * (1-lastFade)) );
								lights[IN1_LIGHT].setBrightness(1-lastFade);
								lights[IN2_LIGHT].setBrightness(lastFade);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage());
							outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage());
							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						} else {
							outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage());
							outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage());
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
					if (inputs[RST_INPUT].isConnected()){
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue != 0) {
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
						if (fadeValue > 10) {
							fadeValue = 10;
						} else 	if (fadeValue < 0) {
							fadeValue = 0;
						}
						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage() * lastFade);
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage() * lastFade);
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage() * (1-lastFade));
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage() * (1-lastFade));
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(lastFade);
							} else {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage() * (1-lastFade));
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage() * (1-lastFade));
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage() * lastFade);
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage() * lastFade);
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage());
							outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage());
							if (connectionChange) {
								connectionChange = false;
								outputs[OUT1_OUTPUT].setVoltage(0);
								outputs[OUT1_OUTPUT+1].setVoltage(0);
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						} else {
							outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage());
							outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage());
							if (connectionChange) {
								connectionChange = false;
								outputs[OUT2_OUTPUT].setVoltage(0);
								outputs[OUT2_OUTPUT+1].setVoltage(0);
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						}
					}
				break;

				case 14:										// IN2 + OUT1 + OUT2 = 14
					if (inputs[RST_INPUT].isConnected()){
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue != 0) {
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
						if (fadeValue > 10) {
							fadeValue = 10;
						} else 	if (fadeValue < 0) {
							fadeValue = 0;
						}
						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage() * (1-lastFade));
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage() * (1-lastFade));
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage() * lastFade);
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage() * lastFade);
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							} else {
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage() * lastFade);
								outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage() * lastFade);
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage() * (1-lastFade));
								outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage() * (1-lastFade));
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(lastFade);
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage());
							outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage());
							if (connectionChange) {
								connectionChange = false;
								outputs[OUT2_OUTPUT].setVoltage(0);
								outputs[OUT2_OUTPUT+1].setVoltage(0);
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);									
							}
						} else {
							outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage());
							outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage());
							if (connectionChange) {
								connectionChange = false;
								outputs[OUT1_OUTPUT].setVoltage(0);
								outputs[OUT1_OUTPUT+1].setVoltage(0);
								lights[IN1_LIGHT].setBrightness(0.f);
								lights[IN2_LIGHT].setBrightness(1.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						}
					}
				break;

				case 15:						// IN1 + IN2 + OUT1 + OUT2 = 15
					if (inputs[RST_INPUT].isConnected()){
						rst = inputs[RST_INPUT].getVoltage();
						if (rst >= 1 && prevRst < 1) {
							if (currentSwitch) {
								currentSwitch = 0;
								connectionChange = true;
								if (fadeValue != 0) {
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
						if (fadeValue > 10) {
							fadeValue = 10;
						} else 	if (fadeValue < 0) {
							fadeValue = 0;
						}
						maxFadeSample = args.sampleRate * fadeValue;
						lastFade = (currentFadeSample / maxFadeSample) + startFade;

						if (lastFade > 1) {
							fading = false;
							currentFadeSample = 0;
							startFade = 0;
							lastFade = 0;
						} else {
							if (currentSwitch) {
								outputs[OUT1_OUTPUT].setVoltage( (inputs[IN2_INPUT].getVoltage() * lastFade) + (inputs[IN1_INPUT].getVoltage() * (1-lastFade)) );
								outputs[OUT1_OUTPUT+1].setVoltage( (inputs[IN2_INPUT+1].getVoltage() * lastFade) + (inputs[IN1_INPUT+1].getVoltage() * (1-lastFade)) );
								outputs[OUT2_OUTPUT].setVoltage( (inputs[IN1_INPUT].getVoltage() * lastFade) + (inputs[IN2_INPUT].getVoltage() * (1-lastFade)) );
								outputs[OUT2_OUTPUT+1].setVoltage( (inputs[IN1_INPUT+1].getVoltage() * lastFade) + (inputs[IN2_INPUT+1].getVoltage() * (1-lastFade)) );
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(lastFade);
							} else {
								outputs[OUT1_OUTPUT].setVoltage( (inputs[IN1_INPUT].getVoltage() * lastFade) + (inputs[IN2_INPUT].getVoltage() * (1-lastFade)) );
								outputs[OUT1_OUTPUT+1].setVoltage( (inputs[IN1_INPUT+1].getVoltage() * lastFade) + (inputs[IN2_INPUT+1].getVoltage() * (1-lastFade)) );
								outputs[OUT2_OUTPUT].setVoltage( (inputs[IN2_INPUT].getVoltage() * lastFade) + (inputs[IN1_INPUT].getVoltage() * (1-lastFade)) );
								outputs[OUT2_OUTPUT+1].setVoltage( (inputs[IN2_INPUT+1].getVoltage() * lastFade) + (inputs[IN1_INPUT+1].getVoltage() * (1-lastFade)) );
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT1_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage());
							outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage());
							outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage());
							outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage());
							if (connectionChange) {
								connectionChange = false;
								lights[IN1_LIGHT].setBrightness(1.f);
								lights[IN2_LIGHT].setBrightness(0.f);
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						} else {
							outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage());
							outputs[OUT1_OUTPUT+1].setVoltage(inputs[IN1_INPUT+1].getVoltage());
							outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage());
							outputs[OUT2_OUTPUT+1].setVoltage(inputs[IN2_INPUT+1].getVoltage());
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
						outputs[OUT1_OUTPUT].setVoltage(0);
						outputs[OUT1_OUTPUT+1].setVoltage(0);
						outputs[OUT2_OUTPUT].setVoltage(0);
						outputs[OUT2_OUTPUT+1].setVoltage(0);
						lights[IN1_LIGHT].setBrightness(0.f);
						lights[IN2_LIGHT].setBrightness(0.f);
						lights[OUT1_LIGHT].setBrightness(0.f);
						lights[OUT2_LIGHT].setBrightness(0.f);
						connectionChange = false;
					}
			}
		}
	}
};

struct SwitcherStWidget : ModuleWidget {
	SwitcherStWidget(SwitcherSt* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SwitcherSt.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<CKSS>(mm2px(Vec(4, 11.35)), module, SwitcherSt::MODE_SWITCH));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 26)), module, SwitcherSt::TRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 39)), module, SwitcherSt::RST_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6, 55.5)), module, SwitcherSt::IN1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.3, 55.5)), module, SwitcherSt::IN1_INPUT+1));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6, 64.5)), module, SwitcherSt::IN2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.3, 64.5)), module, SwitcherSt::IN2_INPUT+1));

		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(10.16, 52)), module, SwitcherSt::IN1_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(10.16, 61)), module, SwitcherSt::IN2_LIGHT));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 82.9)), module, SwitcherSt::FADE_PARAMS));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 92.5)), module, SwitcherSt::FADECV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6, 109)), module, SwitcherSt::OUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.3, 109)), module, SwitcherSt::OUT1_OUTPUT+1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6, 118)), module, SwitcherSt::OUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.3, 118)), module, SwitcherSt::OUT2_OUTPUT+1));

		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(10.16, 105.5)), module, SwitcherSt::OUT1_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(10.16, 114.5)), module, SwitcherSt::OUT2_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		SwitcherSt* module = dynamic_cast<SwitcherSt*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Restore State on Load", "", &module->stateRestore));
	}
};

Model* modelSwitcherSt = createModel<SwitcherSt, SwitcherStWidget>("SwitcherSt");