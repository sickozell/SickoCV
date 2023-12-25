#define MAX_TRACKS 8
#define SAMPLE_HOLD 0
#define TRACK_HOLD 1
#define FULL_NOISE 0

#include "plugin.hpp"

using namespace std;

struct Holder8 : Module {
	enum ParamId {
		ENUMS(MODE_SWITCH, MAX_TRACKS),
		ENUMS(PROB_PARAM, MAX_TRACKS),
		ENUMS(SCALE_PARAM, MAX_TRACKS),
		ENUMS(OFFSET_PARAM, MAX_TRACKS),
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(TRIG_INPUT, MAX_TRACKS),
		ENUMS(IN_INPUT, MAX_TRACKS),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT, MAX_TRACKS),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(MODE_LIGHT, MAX_TRACKS),
		LIGHTS_LEN
	};

	int mode[MAX_TRACKS] = {SAMPLE_HOLD, SAMPLE_HOLD, SAMPLE_HOLD, SAMPLE_HOLD, SAMPLE_HOLD, SAMPLE_HOLD, SAMPLE_HOLD, SAMPLE_HOLD};
	int noiseType = FULL_NOISE;
	
	float trigValue[MAX_TRACKS] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float prevTrigValue[MAX_TRACKS] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	
	int sampleOnGate = 0;

	float out = 0.f;
	int chan = 1;

	float probSetup[MAX_TRACKS] = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};
	float probValue = 0.f;

	bool holding[MAX_TRACKS] = {false, false, false, false, false, false, false, false};

	float oneMsSamples = (APP->engine->getSampleRate()) / 1000;			// samples in 1ms


	//**************************************************************
	//  DEBUG 

	/*
	std::string debugDisplay = "X";
	std::string debugDisplay2 = "X";
	std::string debugDisplay3 = "X";
	std::string debugDisplay4 = "X";
	int debugInt = 0;
	bool debugBool = false;
	*/

	Holder8() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		for (int track = 0; track < MAX_TRACKS; track++) {
			configSwitch(MODE_SWITCH+track, 0.f, 1.f, 0.f, "Mode", {"Sample & Hold", "Track & Hold"});
			configInput(TRIG_INPUT+track, "Trig/Gate");
			configInput(IN_INPUT+track, "Signal");

			configParam(PROB_PARAM+track, 0, 1.f, 1.f, "Probability", "%", 0, 100);
			configParam(SCALE_PARAM+track, -1.f, 1.f, 1.f, "Scale", "%", 0, 100);
			configParam(OFFSET_PARAM+track, -10.f, 10.f, 0.f, "Offset", "v");

			configOutput(OUT_OUTPUT+track, "Signal");
		}
	}

	void onReset(const ResetEvent &e) override {

		for (int track = 0; track < MAX_TRACKS; track++) {
			mode[track] = SAMPLE_HOLD;
			trigValue[track] = 0;
			prevTrigValue[track] = 0;
			outputs[OUT_OUTPUT+track].setVoltage(0);
		}

		noiseType = FULL_NOISE;

		sampleOnGate = 0;

		Module::onReset(e);
	}

	void onSampleRateChange() override {

		oneMsSamples = APP->engine->getSampleRate() / 1000;

	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "noiseType", json_boolean(noiseType));
		json_object_set_new(rootJ, "sampleOnGate", json_integer(sampleOnGate));
		/*
		json_object_set_new(rootJ, "gateOnTH", json_boolean(gateOnTH));
		json_object_set_new(rootJ, "gateInv", json_boolean(gateInv));
		json_object_set_new(rootJ, "trigOnStart", json_boolean(trigOnStart));
		json_object_set_new(rootJ, "trigOnEnd", json_boolean(trigOnEnd));
		*/
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* noiseTypeJ = json_object_get(rootJ, "noiseType");
		if (noiseTypeJ)
			noiseType = json_boolean_value(noiseTypeJ);

		json_t* sampleOnGateJ = json_object_get(rootJ, "sampleOnGate");
		if (sampleOnGateJ)
			sampleOnGate = json_integer_value(sampleOnGateJ);

	}

	bool isSampleOnHighGate() {
		return sampleOnGate;
	}

	void setSampleOnGate(bool sampleOnHighGate) {
		if (sampleOnHighGate) {
			sampleOnGate = 1;
			for (int track = 0; track < MAX_TRACKS; track++)
				holding[track] = true;
		} else {
			sampleOnGate = 0;
			for (int track = 0; track < MAX_TRACKS; track++)
				holding[track] = false;
		}
		
	}

	void process(const ProcessArgs& args) override {

		for (int track = 0; track < MAX_TRACKS; track++) {

			mode[track] = params[MODE_SWITCH+track].getValue();

			lights[MODE_LIGHT+track].setBrightness(mode[track]);

			if (outputs[OUT_OUTPUT+track].isConnected()) {
				
				probSetup[track] = params[PROB_PARAM+track].getValue();

				switch (mode[track]) {

					case SAMPLE_HOLD:
						if (inputs[IN_INPUT+track].isConnected()) {
							trigValue[track] = inputs[TRIG_INPUT+track].getVoltage();
							chan = std::max(1, inputs[IN_INPUT+track].getChannels());
							if (trigValue[track] >= 1.f && prevTrigValue[track] < 1.f) {
								probValue = random::uniform();
								if (probSetup[track] >= probValue) {
									for (int c = 0; c < chan; c++) {

										out = ( inputs[IN_INPUT+track].getVoltage(c) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

										if (out > 10.f)
											out = 10.f;
										else if (out < -10.f)
											out = -10.f;

										outputs[OUT_OUTPUT+track].setVoltage(out, c);
									}

								}					
								
							}
							prevTrigValue[track] = trigValue[track];
							outputs[OUT_OUTPUT+track].setChannels(chan);
							
						} else {
							
							trigValue[track] = inputs[TRIG_INPUT+track].getVoltage();
							if (trigValue[track] >= 1.f && prevTrigValue[track] < 1.f) {
								probValue = random::uniform();
								if (probSetup[track] >= probValue) { 

									if (noiseType == FULL_NOISE) 
										out = ( (random::uniform() * 10 - 5.f) * (params[SCALE_PARAM+track].getValue()) ) +	params[OFFSET_PARAM+track].getValue();
									else
										out = ( random::normal() * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

									if (out > 10.f)
										out = 10.f;
									else if (out < -10.f)
										out = -10.f;

									outputs[OUT_OUTPUT+track].setVoltage(out, 0);

								}
								outputs[OUT_OUTPUT+track].setChannels(1);
							}
							prevTrigValue[track] = trigValue[track];
						}
					
					break;

					case TRACK_HOLD:
						switch (sampleOnGate) {
							case 0:	// sample on LOW GATE
								if (inputs[IN_INPUT+track].isConnected()) {
									trigValue[track] = inputs[TRIG_INPUT+track].getVoltage();
									chan = std::max(1, inputs[IN_INPUT+track].getChannels());

									if (trigValue[track] >= 1.f && prevTrigValue[track] < 1.f) {
										probValue = random::uniform();
										if (probSetup[track] >= probValue) {
											for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT+track].getVoltage(c) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT+track].setVoltage(out, c);

											}

											holding[track] = false;
										}
									} else if (trigValue[track] >= 1.f && !holding[track]) {
										for (int c = 0; c < chan; c++) {

											out = ( inputs[IN_INPUT+track].getVoltage(c) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT+track].setVoltage(out, c);

										}

									} else if (trigValue[track] < 1) {
										if (!holding[track]) {
											holding[track] = true;
											for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT+track].getVoltage(c) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT+track].setVoltage(out, c);

											}
										}
									}

									prevTrigValue[track] = trigValue[track];
									outputs[OUT_OUTPUT+track].setChannels(chan);

								} else {

									trigValue[track] = inputs[TRIG_INPUT+track].getVoltage();

									if (trigValue[track] >= 1.f && prevTrigValue[track] < 1.f) {
										probValue = random::uniform();
										if (probSetup[track] >= probValue) {
											if (noiseType == FULL_NOISE) 
												out = ( (random::uniform() * 10 - 5.f) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();
											else
												out = ( random::normal() * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT+track].setVoltage(out, 0);

											holding[track] = false;
										}
									} else if (trigValue[track] >= 1.f && !holding[track]) {

										if (noiseType == FULL_NOISE) 
											out = ( (random::uniform() * 10 - 5.f) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();
										else
											out = ( random::normal() * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

										if (out > 10.f)
											out = 10.f;
										else if (out < -10.f)
											out = -10.f;

										outputs[OUT_OUTPUT+track].setVoltage(out, 0);

									} else if (trigValue[track] < 1) {
										if (!holding[track]) {
											holding[track] = true;

											if (noiseType == FULL_NOISE) 
												out = ( (random::uniform() * 10 - 5.f) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();
											else
												out = ( random::normal() * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT+track].setVoltage(out, 0);

										}
									}

									prevTrigValue[track] = trigValue[track];
									outputs[OUT_OUTPUT+track].setChannels(1);

								}

							break;

							case 1:	// sample on HIGH GATE
								if (inputs[IN_INPUT+track].isConnected()) {
									trigValue[track] = inputs[TRIG_INPUT+track].getVoltage();
									chan = std::max(1, inputs[IN_INPUT+track].getChannels());
									if (trigValue[track] >= 1.f && prevTrigValue[track] < 1.f) {
										probValue = random::uniform();
										if (probSetup[track] >= probValue) {
											holding[track] = true;
											for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT+track].getVoltage(c) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT+track].setVoltage(out, c);
											}

										} else {

											holding[track] = false;

										}

									} else if (trigValue[track] < 1.f) {

										holding[track] = false;

									}
									prevTrigValue[track] = trigValue[track];

									if (!holding[track]) {
										for (int c = 0; c < chan; c++) {

											out = ( inputs[IN_INPUT+track].getVoltage(c) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT+track].setVoltage(out, c);
										}							
									}

									outputs[OUT_OUTPUT+track].setChannels(chan);
									
								} else {

									if (noiseType == FULL_NOISE) 
										out = ( (random::uniform() * 10 - 5.f) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();
									else
										out = ( random::normal() * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

									trigValue[track] = inputs[TRIG_INPUT+track].getVoltage();
									if (trigValue[track] >= 1.f && prevTrigValue[track] < 1.f) {
										probValue = random::uniform();
										if (probSetup[track] >= probValue) {
											holding[track] = true;
											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT+track].setVoltage(out, 0);
											
										} else {
											holding[track] = false;
										}
									} else if (trigValue[track] < 1.f) {

										holding[track] = false;
									}
									prevTrigValue[track] = trigValue[track];

									if (!holding[track]) {
										if (out > 10.f)
											out = 10.f;
										else if (out < -10.f)
											out = -10.f;

										outputs[OUT_OUTPUT+track].setVoltage(out, 0);
									}
									outputs[OUT_OUTPUT+track].setChannels(1);
								}
							break;
						}
					break;
				}
			} else {
				outputs[OUT_OUTPUT+track].setVoltage(0.f, 0);
				outputs[OUT_OUTPUT+track].setChannels(1);
			}
		}
	}
};

struct Holder8DebugDisplay : TransparentWidget {
	Holder8 *module;
	int frame = 0;
	Holder8DebugDisplay() {
	}

	/*
	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-bold.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff)); 
				
				nvgTextBox(args.vg, 9, 6,120, module->debugDisplay.c_str(), NULL);
				//nvgTextBox(args.vg, 9, 16,120, module->debugDisplay2.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 6,120, module->debugDisplay3.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 16,120, module->debugDisplay4.c_str(), NULL);

			}
		}
		Widget::drawLayer(args, layer);
	}
	*/
};

struct Holder8Widget : ModuleWidget {
	Holder8Widget(Holder8* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Holder8.svg")));

		/*
		{
			Holder8DebugDisplay *display = new Holder8DebugDisplay();
			display->box.pos = Vec(23, 3);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// buttons --- 4.1
		// trimpot --- x  3.7 --- y 4.3
		// trimpot senza stanghetta --- y 3.7
		// smallRoundKnob --- x 4.6 --- y 5.1
		// roundBlackKnob --- x 5.7 --- y 6.4
		// input/output --- 4.5

		constexpr float xStart = 6.6f;
		constexpr float xDelta = 9.7f;
		
		constexpr float yStart = 19.f;
		constexpr float yDelta = 14.f;

		for (int track = 0; track < MAX_TRACKS; track++) {

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xStart, yStart+(track*yDelta))), module, Holder8::TRIG_INPUT+track));

			addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xStart+xDelta, yStart+(track*yDelta))), module, Holder8::MODE_SWITCH+track, Holder8::MODE_LIGHT+track));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xStart+(2*xDelta), yStart+(track*yDelta))), module, Holder8::IN_INPUT+track));

			addParam(createParamCentered<Trimpot>(mm2px(Vec(xStart+(3*xDelta), yStart+(track*yDelta))), module, Holder8::PROB_PARAM+track));

			addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xStart+(4*xDelta), yStart+(track*yDelta))), module, Holder8::SCALE_PARAM+track));

			addParam(createParamCentered<Trimpot>(mm2px(Vec(xStart+(5*xDelta), yStart+(track*yDelta))), module, Holder8::OFFSET_PARAM+track));

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xStart+(6*xDelta), yStart+(track*yDelta))), module, Holder8::OUT_OUTPUT+track));
		}
	}

	void appendContextMenu(Menu* menu) override {
		Holder8* module = dynamic_cast<Holder8*>(this->module);

		struct ModeItem : MenuItem {
			Holder8* module;
			int noiseType;
			void onAction(const event::Action& e) override {
				module->noiseType = noiseType;
			}
		};

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("White Noise Type"));
		std::string modeNames[2] = {"Full", "Centered"};
		for (int i = 0; i < 2; i++) {
			ModeItem* modeItem = createMenuItem<ModeItem>(modeNames[i]);
			modeItem->rightText = CHECKMARK(module->noiseType == i);
			modeItem->module = module;
			modeItem->noiseType = i;
			menu->addChild(modeItem);
		}


		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Track & Hold:"));

		menu->addChild(createBoolMenuItem("Sample on HIGH Gate", "", [=]() {
					return module->isSampleOnHighGate();
				}, [=](bool sampleOnHighGate) {
					module->setSampleOnGate(sampleOnHighGate);
			}));
		/*
		menu->addChild(createBoolMenuItem("Gate Out instead Trig", "", [=]() {
					return module->isGateOut();
				}, [=](bool gateOut) {
					module->setGateOut(gateOut);
			}));

		if (module->gateOnTH == true) {
			menu->addChild(createBoolPtrMenuItem("Gate Inversion", "", &module->gateInv));
			menu->addChild(createMenuLabel("Trig on start"));
			menu->addChild(createMenuLabel("Trig on end"));
		} else {
			menu->addChild(createMenuLabel("Gate Inversion"));
			menu->addChild(createBoolPtrMenuItem("Trig on start", "", &module->trigOnStart));
			menu->addChild(createBoolPtrMenuItem("Trig on end", "", &module->trigOnEnd));
		}
		*/
	}
};

Model* modelHolder8 = createModel<Holder8, Holder8Widget>("Holder8");