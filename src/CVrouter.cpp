#include "plugin.hpp"

struct CVrouter : Module {
	enum ParamId {
		THRESHOLD_PARAM,
		THRESHOLD_ATTENUV_PARAM,
		FADE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CV_INPUT,
		THRESHOLD_CV_INPUT,
		IN1_INPUT,
		FADECV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		OUT1_LIGHT,
		OUT2_LIGHT,
		LIGHTS_LEN
	};

	bool trigConnection = false;
	bool prevTrigConnection = false;
	int connection = 0;
	int prevConnection = -1;
	bool connectionChange = true;

	bool currentSwitch = false;
	bool trigState = false;
	float trigValue = 0;
	float prevTrigValue = 0;

	float fadeValue = 0;

	float maxFadeSample = 0;
	float currentFadeSample = 0;
	bool fading = false;
	float startFade = 0;
	float lastFade = 0;

	int chan;

	float threshold = 0.f;

	/*static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	const float maxAdsrTime = 10.f;*/
	const float noEnvTime = 0.00101;

	CVrouter() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		
		configInput(CV_INPUT, "CV");
		configParam(THRESHOLD_PARAM, -10.f, 10.f, 1.f, "Threshold", "v");
		configInput(THRESHOLD_CV_INPUT, "Threshold");
		configParam(THRESHOLD_ATTENUV_PARAM, -1.f, 1.f, 0.f, "Threshold CV", "%", 0, 100);
		configInput(IN1_INPUT, "IN 1");
		//configParam(FADE_PARAM, 0.f, 1.f, 0.f, "Fade Time", "ms", maxStageTime / minStageTime, minStageTime);
		configParam(FADE_PARAM, 0.f, 1.f, 0.f, "Fade Time", "ms", 10000.f, 1.f);
		configInput(FADECV_INPUT, "Fade Time CV");
		configOutput(OUT1_OUTPUT, "OUT 1");
		configOutput(OUT2_OUTPUT, "OUT 2");
	}

	void onReset(const ResetEvent &e) override {
		trigConnection = false;
		prevTrigConnection = false;
		connection = 0;
		prevConnection = -1;
		connectionChange = true;
		currentSwitch = false;
		fading = false;
		outputs[OUT1_OUTPUT].setVoltage(0.f);
		outputs[OUT1_OUTPUT].setChannels(1);
		outputs[OUT2_OUTPUT].setVoltage(0.f);
		outputs[OUT2_OUTPUT].setChannels(1);
		lights[OUT1_LIGHT].setBrightness(0.f);
		lights[OUT2_LIGHT].setBrightness(0.f);
	    Module::onReset(e);
	}

	/*static float convertCVToSeconds(float cv) {		
		return minStageTime * std::pow(maxStageTime / minStageTime, cv) / 1000;
	}*/

	void process(const ProcessArgs& args) override {
		trigConnection = inputs[CV_INPUT].isConnected();
		if (!trigConnection) {
			if (prevTrigConnection) {
				outputs[OUT1_OUTPUT].setVoltage(0.f, 0);	// ******** TO CHECK IF POLYPHONIC
				outputs[OUT1_OUTPUT].setChannels(1);
				outputs[OUT2_OUTPUT].setVoltage(0.f, 0);
				outputs[OUT2_OUTPUT].setChannels(1);

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

			// da togliere 6 7 10 11 14 15

			if (!inputs[THRESHOLD_CV_INPUT].isConnected()) {
				threshold = params[THRESHOLD_PARAM].getValue();
			} else {
				threshold = params[THRESHOLD_PARAM].getValue() + inputs[THRESHOLD_CV_INPUT].getVoltage() * params[THRESHOLD_ATTENUV_PARAM].getValue();
				if (threshold > 10.f) {
					threshold = 10.f;
				} else if (threshold < -10.f) {
					threshold = -10.f;
				}
			} 

			trigValue = inputs[CV_INPUT].getVoltage();

			if (trigValue >= threshold && !currentSwitch) {
				currentSwitch = true;
				connectionChange = true;
				trigState = true;
			} else if (trigValue < threshold && currentSwitch) {
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

			switch (connection) {
				case 4: 										// OUT1 = 4
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
							if (!currentSwitch) {
								outputs[OUT1_OUTPUT].setVoltage(10.f * lastFade, 0);
								outputs[OUT1_OUTPUT].setChannels(1);

								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);
							} else {
								outputs[OUT1_OUTPUT].setVoltage(10.f * (1-lastFade), 0);
								outputs[OUT1_OUTPUT].setChannels(1);

								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						}
						currentFadeSample++;
					} else {
						if (!currentSwitch) {
							outputs[OUT1_OUTPUT].setVoltage(10.f, 0);
							outputs[OUT1_OUTPUT].setChannels(1);

							if (connectionChange) {
								connectionChange = false;
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							outputs[OUT1_OUTPUT].setVoltage(0.f, 0);
							outputs[OUT1_OUTPUT].setChannels(1);

							if (connectionChange) {
								connectionChange = false;
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						}
					}
				break;
				
				case 8:											// OUT2 = 8
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
							if (!currentSwitch) {
								outputs[OUT2_OUTPUT].setVoltage(10.f * (1-lastFade), 0);
								outputs[OUT2_OUTPUT].setChannels(1);

								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							} else {
								outputs[OUT2_OUTPUT].setVoltage(10.f * lastFade, 0);
								outputs[OUT2_OUTPUT].setChannels(1);

								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(lastFade);								
							}
						}
						currentFadeSample++;
					} else {
						if (!currentSwitch) {
							outputs[OUT2_OUTPUT].setVoltage(0.f, 0);
							outputs[OUT2_OUTPUT].setChannels(1);

							if (connectionChange) {
								connectionChange = false;
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							outputs[OUT2_OUTPUT].setVoltage(10.f, 0);
							outputs[OUT2_OUTPUT].setChannels(1);

							if (connectionChange) {
								connectionChange = false;
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}							
						}
					}
				break;

				case 12: 										// OUT1 + OUT2 = 12
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

								outputs[OUT2_OUTPUT].setVoltage(10.f * lastFade, 0);
								outputs[OUT2_OUTPUT].setChannels(1);

								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(lastFade);
							} else {
								outputs[OUT1_OUTPUT].setVoltage(10.f * lastFade, 0);
								outputs[OUT1_OUTPUT].setChannels(1);

								outputs[OUT2_OUTPUT].setVoltage(10.f * (1-lastFade));
								outputs[OUT2_OUTPUT].setChannels(1);

								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							}
						}
						currentFadeSample++;
					} else {
						if (currentSwitch) {
							outputs[OUT1_OUTPUT].setVoltage(0.f, 0);
							outputs[OUT1_OUTPUT].setChannels(1);

							outputs[OUT2_OUTPUT].setVoltage(10.f, 0);
							outputs[OUT2_OUTPUT].setChannels(1);

							if (connectionChange) {
								connectionChange = false;
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						} else {
							outputs[OUT1_OUTPUT].setVoltage(10.f, 0);
							outputs[OUT1_OUTPUT].setChannels(1);

							outputs[OUT2_OUTPUT].setVoltage(0.f, 0);
							outputs[OUT2_OUTPUT].setChannels(1);

							if (connectionChange) {
								connectionChange = false;
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						}
					}
				break;

				case 5:											// IN1 + OUT1 = 5
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
							if (!currentSwitch) {
								for (int c = 0; c < chan; c++)
									outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * lastFade, c);
								outputs[OUT1_OUTPUT].setChannels(chan);

								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							} else {
								for (int c = 0; c < chan; c++)
									outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * (1-lastFade), c);
								outputs[OUT1_OUTPUT].setChannels(chan);

								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(lastFade);								
							}
						}
						currentFadeSample++;
					} else {
						chan = std::max(1, inputs[IN1_INPUT].getChannels());
						if (!currentSwitch) {
							for (int c = 0; c < chan; c++)
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c), c);
							outputs[OUT1_OUTPUT].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							for (int c = 0; c < chan; c++)
								outputs[OUT1_OUTPUT].setVoltage(0.f , c);
							outputs[OUT1_OUTPUT].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}							
						}
					}
				break;
				
				case 9:											// IN1 + OUT2 = 9
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
							if (!currentSwitch) {
								for (int c = 0; c < chan; c++)
									outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * (1-lastFade), c);
								outputs[OUT2_OUTPUT].setChannels(chan);

								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							} else {
								for (int c = 0; c < chan; c++)
									outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * lastFade, c);
								outputs[OUT2_OUTPUT].setChannels(chan);

								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(lastFade);								
							}
						}
						currentFadeSample++;
					} else {
						chan = std::max(1, inputs[IN1_INPUT].getChannels());
						if (!currentSwitch) {
							for (int c = 0; c < chan; c++)
								outputs[OUT2_OUTPUT].setVoltage(0.f, c);
							outputs[OUT2_OUTPUT].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[OUT1_LIGHT].setBrightness(1.f);
								lights[OUT2_LIGHT].setBrightness(0.f);
							}
						} else {
							for (int c = 0; c < chan; c++)
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c), c);
							outputs[OUT2_OUTPUT].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}							
						}
					}
				break;

				case 13:											// IN1 + OUT1 + OUT2 = 13
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
									outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * (1-lastFade), c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT].setChannels(chan);

								lights[OUT1_LIGHT].setBrightness(1-lastFade);
								lights[OUT2_LIGHT].setBrightness(lastFade);
							} else {
								for (int c = 0; c < chan; c++) {
									outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * (1-lastFade), c);
									outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c) * lastFade, c);
								}
								outputs[OUT2_OUTPUT].setChannels(chan);
								outputs[OUT1_OUTPUT].setChannels(chan);

								lights[OUT1_LIGHT].setBrightness(lastFade);
								lights[OUT2_LIGHT].setBrightness(1-lastFade);
							}
						}
						currentFadeSample++;
					} else {
						chan = std::max(1, inputs[IN1_INPUT].getChannels());
						if (currentSwitch) {
							for (int c = 0; c < chan; c++)
								outputs[OUT2_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c), c);
							outputs[OUT2_OUTPUT].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								for (int c = 0; c < chan; c++)
									outputs[OUT1_OUTPUT].setVoltage(0.f, c);
								outputs[OUT1_OUTPUT].setChannels(chan);

								lights[OUT1_LIGHT].setBrightness(0.f);
								lights[OUT2_LIGHT].setBrightness(1.f);
							}
						} else {
							for (int c = 0; c < chan; c++)
								outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage(c), c);
							outputs[OUT1_OUTPUT].setChannels(chan);

							if (connectionChange) {
								connectionChange = false;
								for (int c = 0; c < chan; c++)
									outputs[OUT2_OUTPUT].setVoltage(0.f, c);
								outputs[OUT2_OUTPUT].setChannels(chan);

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
						outputs[OUT2_OUTPUT].setVoltage(0.f, 0);
						outputs[OUT2_OUTPUT].setChannels(1);
						lights[OUT1_LIGHT].setBrightness(0.f);
						lights[OUT2_LIGHT].setBrightness(0.f);
						connectionChange = false;
					}
			}
		}
		prevTrigConnection = trigConnection;
	}
};

struct CVrouterWidget : ModuleWidget {
	CVrouterWidget(CVrouter* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/CVrouter.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(7.62, 17)), module, CVrouter::CV_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(7.62, 32)), module, CVrouter::THRESHOLD_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(7.62, 41.8)), module, CVrouter::THRESHOLD_ATTENUV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(7.62, 49.5)), module, CVrouter::THRESHOLD_CV_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(7.62, 65)), module, CVrouter::IN1_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(7.62, 82.9)), module, CVrouter::FADE_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(7.62, 92.5)), module, CVrouter::FADECV_INPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(7.62, 109)), module, CVrouter::OUT1_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(7.62, 117.5)), module, CVrouter::OUT2_OUTPUT));

		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(12, 105.5)), module, CVrouter::OUT1_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(12, 114)), module, CVrouter::OUT2_LIGHT));

	}
};

Model* modelCVrouter = createModel<CVrouter, CVrouterWidget>("CVrouter");