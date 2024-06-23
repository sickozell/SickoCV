#define SAMPLE_HOLD 0
#define TRACK_HOLD 1
#define FULL_NOISE 0

#include "plugin.hpp"

using namespace std;

struct HolderCompact : Module {
	enum ParamId {
		MODE_SWITCH,
		PROB_PARAM,
		SCALE_PARAM,
		OFFSET_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TRIG_INPUT,
		IN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		TRIG_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		MODE_LIGHT,
		LIGHTS_LEN
	};

	int mode = SAMPLE_HOLD;
	int noiseType = FULL_NOISE;
	
	float trigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevTrigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	
	bool trigOnStart = true;
	bool trigOnEnd = true;
	int sampleOnGate = 0;
	bool gateOnTH = false;

	float out = 0.f;
	int chan = 1;
	int chanTrig = 1;

	float probSetup = 1.f;
	float probValue = 0.f;

	bool holding[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	float oneMsSamples = (APP->engine->getSampleRate()) / 1000;			// samples in 1ms
	float outTrigSample[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	bool outTrig[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

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

	HolderCompact() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(MODE_SWITCH, 0.f, 1.f, 0.f, "Mode", {"Sample & Hold", "Track & Hold"});
		configInput(TRIG_INPUT, "Trig/Gate");
		configInput(IN_INPUT, "Signal");

		configParam(PROB_PARAM, 0, 1.f, 1.f, "Probability", "%", 0, 100);

		configParam(SCALE_PARAM, -1.f, 1.f, 1.f, "Scale", "%", 0, 100);

		configParam(OFFSET_PARAM, -10.f, 10.f, 0.f, "Offset", "v");

		configOutput(OUT_OUTPUT, "Signal");
		configOutput(TRIG_OUTPUT, "Gate");
	}

	void onReset(const ResetEvent &e) override {

		mode = SAMPLE_HOLD;

		for (int i = 0; i < 16; i++) {
			trigValue[i] = 0;
			prevTrigValue[i] = 0;
			outputs[OUT_OUTPUT].setVoltage(0, i);
		}

		trigOnStart = true;
		trigOnEnd = true;
		sampleOnGate = 0;
		gateOnTH = false;

		noiseType = FULL_NOISE;

		//outputs[OUT_OUTPUT].setChannels(1);

		Module::onReset(e);
	}

	void onSampleRateChange() override {

		oneMsSamples = APP->engine->getSampleRate() / 1000;

	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "noiseType", json_boolean(noiseType));
		json_object_set_new(rootJ, "sampleOnGate", json_integer(sampleOnGate));
		json_object_set_new(rootJ, "gateOnTH", json_boolean(gateOnTH));
		json_object_set_new(rootJ, "trigOnStart", json_boolean(trigOnStart));
		json_object_set_new(rootJ, "trigOnEnd", json_boolean(trigOnEnd));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* noiseTypeJ = json_object_get(rootJ, "noiseType");
		if (noiseTypeJ)
			noiseType = json_boolean_value(noiseTypeJ);

		json_t* sampleOnGateJ = json_object_get(rootJ, "sampleOnGate");
		if (sampleOnGateJ)
			sampleOnGate = json_integer_value(sampleOnGateJ);

		json_t* gateOnTHJ = json_object_get(rootJ, "gateOnTH");
		if (gateOnTHJ)
			gateOnTH = json_boolean_value(gateOnTHJ);

		json_t* trigOnStartJ = json_object_get(rootJ, "trigOnStart");
		if (trigOnStartJ)
			trigOnStart = json_boolean_value(trigOnStartJ);

		json_t* trigOnEndJ = json_object_get(rootJ, "trigOnEnd");
		if (trigOnEndJ)
			trigOnEnd = json_boolean_value(trigOnEndJ);

	}

	bool isSampleOnHighGate() {
		return sampleOnGate;
	}

	void setSampleOnGate(bool sampleOnHighGate) {
		if (sampleOnHighGate) {
			sampleOnGate = 1;
			for (int i = 0; i < 16; i++)
				holding[i] = true;
		} else {
			sampleOnGate = 0;
			for (int i = 0; i < 16; i++)
				holding[i] = false;
		}
	}

	bool isGateOut() {
		return gateOnTH;
	}

	void setGateOut(bool gateOut) {
		if (gateOut)
			gateOnTH = true;
		else
			gateOnTH = false;

		for (int i = 0; i < 16; i++)
			outputs[TRIG_OUTPUT].setVoltage(0.f, i);
		
		outputs[TRIG_OUTPUT].setChannels(1);
	}

	void setNoisePreset() {
		params[MODE_SWITCH].setValue(TRACK_HOLD);
		params[SCALE_PARAM].setValue(1);
		params[OFFSET_PARAM].setValue(0);
		sampleOnGate = 1;
	}

	void process(const ProcessArgs& args) override {

		mode = params[MODE_SWITCH].getValue();

		lights[MODE_LIGHT].setBrightness(mode);

		if (outputs[OUT_OUTPUT].isConnected()) {
			
			probSetup = params[PROB_PARAM].getValue();

			switch (mode) {
				case SAMPLE_HOLD:
					if (inputs[IN_INPUT].isConnected()) {
						
						chan = inputs[IN_INPUT].getChannels();

						chanTrig = inputs[TRIG_INPUT].getChannels();

						if (chanTrig == 1) {	// SAMPLE & HOLD - mono Trigger

							trigValue[0] = inputs[TRIG_INPUT].getVoltage();

							if (trigValue[0] >= 1.f && prevTrigValue[0] < 1.f) {
								probValue = random::uniform();
								if (probSetup >= probValue) {
									for (int c = 0; c < chan; c++) {

										out = ( inputs[IN_INPUT].getVoltage(c) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

										if (out > 10.f)
											out = 10.f;
										else if (out < -10.f)
											out = -10.f;

										outputs[OUT_OUTPUT].setVoltage(out, c);
									}
									outTrig[0] = true;
									outTrigSample[0] = oneMsSamples;
								}					
								
							}
							prevTrigValue[0] = trigValue[0];
							outputs[OUT_OUTPUT].setChannels(chan);

						} else { // SAMPLE & HOLD - poly Triggers

							for (int cT = 0; cT < chanTrig; cT++) {
								trigValue[cT] = inputs[TRIG_INPUT].getVoltage(cT);
								if (trigValue[cT] >= 1.f && prevTrigValue[cT] < 1.f) {
									probValue = random::uniform();
									if (probSetup >= probValue) {
										out = ( inputs[IN_INPUT].getVoltage(cT) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

										if (out > 10.f)
											out = 10.f;
										else if (out < -10.f)
											out = -10.f;

										outputs[OUT_OUTPUT].setVoltage(out, cT);
									}
									outTrig[cT] = true;
									outTrigSample[cT] = oneMsSamples;
								}
								prevTrigValue[cT] = trigValue[cT];
							}
							outputs[OUT_OUTPUT].setChannels(chanTrig);
						}
						
					} else {	// Sample & Hold with noise generator
						
						chanTrig = inputs[TRIG_INPUT].getChannels();

						for (int cT = 0; cT < chanTrig; cT++) {
							trigValue[cT] = inputs[TRIG_INPUT].getVoltage(cT);
							if (trigValue[cT] >= 1.f && prevTrigValue[cT] < 1.f) {
								probValue = random::uniform();
								if (probSetup >= probValue) { 

									if (noiseType == FULL_NOISE) 
										out = ( (random::uniform() * 10 - 5.f) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();
									else
										out = ( random::normal() * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

									if (out > 10.f)
										out = 10.f;
									else if (out < -10.f)
										out = -10.f;

									outputs[OUT_OUTPUT].setVoltage(out, cT);
									outTrig[cT] = true;
									outTrigSample[cT] = oneMsSamples;
								}
							}
							prevTrigValue[cT] = trigValue[cT];
						}
						outputs[OUT_OUTPUT].setChannels(chanTrig);

					}
				
				break;

				case TRACK_HOLD:
					switch (sampleOnGate) {
						case 0:	// sample on LOW GATE
							if (inputs[IN_INPUT].isConnected()) {

								chan = inputs[IN_INPUT].getChannels();
								chanTrig = inputs[TRIG_INPUT].getChannels();

								if (chanTrig == 1) {	// monophonic trigger

									trigValue[0] = inputs[TRIG_INPUT].getVoltage();

									if (trigValue[0] >= 1.f && prevTrigValue[0] < 1.f) {
										probValue = random::uniform();
										if (probSetup >= probValue) {
											for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT].getVoltage(c) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();
												
												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT].setVoltage(out, c);

											}

											if (!gateOnTH && trigOnStart) {
												outTrig[0] = true;
												outTrigSample[0] = oneMsSamples;
											}

											holding[0] = false;
										}
									} else if (trigValue[0] >= 1.f && !holding[0]) {
										for (int c = 0; c < chan; c++) {

											out = ( inputs[IN_INPUT].getVoltage(c) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT].setVoltage(out, c);

										}

										if (!gateOnTH && trigOnStart && prevTrigValue[0] < 1.f) {
											outTrig[0] = true;
											outTrigSample[0] = oneMsSamples;
										}

									} else if (trigValue[0] < 1) {
										if (!holding[0]) {
											holding[0] = true;
											for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT].getVoltage(c) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT].setVoltage(out, c);

											}

											if (!gateOnTH && trigOnEnd) {
												outTrig[0] = true;
												outTrigSample[0] = oneMsSamples;
											}
										}
									}

									prevTrigValue[0] = trigValue[0];
									outputs[OUT_OUTPUT].setChannels(chan);

									if (gateOnTH) {
										if (holding[0]) {
											outputs[TRIG_OUTPUT].setVoltage(0.f);
										} else {
											outputs[TRIG_OUTPUT].setVoltage(10.f);
										}
									}
									outputs[TRIG_OUTPUT].setChannels(1);

								} else {	// TRACK & HOLD  SAMPLE ON LOW GATE: polyphonic Triggers

									for (int cT = 0; cT < chanTrig; cT++) {
										trigValue[cT] = inputs[TRIG_INPUT].getVoltage(cT);

										if (trigValue[cT] >= 1.f && prevTrigValue[cT] < 1.f) {
											probValue = random::uniform();
											if (probSetup >= probValue) {
												//for (int c = 0; c < chan; c++) {

													out = ( inputs[IN_INPUT].getVoltage(cT) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

													if (out > 10.f)
														out = 10.f;
													else if (out < -10.f)
														out = -10.f;

													outputs[OUT_OUTPUT].setVoltage(out, cT);

												//}

												if (!gateOnTH && trigOnStart) {
													outTrig[cT] = true;
													outTrigSample[cT] = oneMsSamples;
												}

												holding[cT] = false;
											}
										} else if (trigValue[cT] >= 1.f && !holding[cT]) {
											//for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT].getVoltage(cT) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT].setVoltage(out, cT);

											//}

											if (!gateOnTH && trigOnStart && prevTrigValue[cT] < 1.f) {
												outTrig[cT] = true;
												outTrigSample[cT] = oneMsSamples;
											}

										} else if (trigValue[cT] < 1) {
											if (!holding[cT]) {
												holding[cT] = true;
												//for (int c = 0; c < chan; c++) {

													out = ( inputs[IN_INPUT].getVoltage(cT) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

													if (out > 10.f)
														out = 10.f;
													else if (out < -10.f)
														out = -10.f;

													outputs[OUT_OUTPUT].setVoltage(out, cT);

												//}

												if (!gateOnTH && trigOnEnd) {
													outTrig[cT] = true;
													outTrigSample[cT] = oneMsSamples;
												}
											}
										}

										prevTrigValue[cT] = trigValue[cT];
										

										if (gateOnTH) {
											if (holding[cT]) {
												outputs[TRIG_OUTPUT].setVoltage(0.f, cT);
											} else {
												outputs[TRIG_OUTPUT].setVoltage(10.f, cT);
											}
										}
									}
									outputs[TRIG_OUTPUT].setChannels(chanTrig);
									outputs[OUT_OUTPUT].setChannels(chanTrig);

								}

							} else {	// TRACK & HOLD - SAMPLE ON LOW GATE - with Noise Generator

								chanTrig = inputs[TRIG_INPUT].getChannels();

								for (int cT = 0; cT < chanTrig; cT++) {

									trigValue[cT] = inputs[TRIG_INPUT].getVoltage(cT);

									if (trigValue[cT] >= 1.f && prevTrigValue[cT] < 1.f) {
										probValue = random::uniform();
										if (probSetup >= probValue) {
											if (noiseType == FULL_NOISE) 
												out = ( (random::uniform() * 10 - 5.f) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();
											else
												out = ( random::normal() * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT].setVoltage(out, cT);

											if (!gateOnTH && trigOnStart) {
												outTrig[cT] = true;
												outTrigSample[cT] = oneMsSamples;
											}

											holding[cT] = false;
										}
									} else if (trigValue[cT] >= 1.f && !holding[cT]) {

										if (noiseType == FULL_NOISE) 
											out = ( (random::uniform() * 10 - 5.f) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();
										else
											out = ( random::normal() * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

										if (out > 10.f)
											out = 10.f;
										else if (out < -10.f)
											out = -10.f;

										outputs[OUT_OUTPUT].setVoltage(out, cT);

										if (!gateOnTH && trigOnStart && prevTrigValue[cT] < 1.f) {
											outTrig[cT] = true;
											outTrigSample[cT] = oneMsSamples;
										}

									} else if (trigValue[cT] < 1) {
										if (!holding[cT]) {
											holding[cT] = true;

											if (noiseType == FULL_NOISE) 
												out = ( (random::uniform() * 10 - 5.f) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();
											else
												out = ( random::normal() * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT].setVoltage(out, cT);

											if (!gateOnTH && trigOnEnd) {
												outTrig[cT] = true;
												outTrigSample[cT] = oneMsSamples;
											}
										}
									}

									prevTrigValue[cT] = trigValue[cT];
									
									if (gateOnTH) {
										if (holding[cT]) {
											outputs[TRIG_OUTPUT].setVoltage(0.f, cT);
										} else {
											outputs[TRIG_OUTPUT].setVoltage(10.f, cT);
										}
									}
									
								}
								outputs[OUT_OUTPUT].setChannels(chanTrig);
								outputs[TRIG_OUTPUT].setChannels(chanTrig);
							}

						break;

						case 1:		// sample on HIGH GATE	
							if (inputs[IN_INPUT].isConnected()) {

								chan = inputs[IN_INPUT].getChannels();
								chanTrig = inputs[TRIG_INPUT].getChannels();

								if (chanTrig == 1) {	// TRACK & HOLD - SAMPLE ON HIGH GATE - monophonic trigger

									trigValue[0] = inputs[TRIG_INPUT].getVoltage();

									if (trigValue[0] >= 1.f && prevTrigValue[0] < 1.f) {
										probValue = random::uniform();
										if (probSetup >= probValue) {
											holding[0] = true;
											for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT].getVoltage(c) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;
												outputs[OUT_OUTPUT].setVoltage(out, c);
											}

											if (gateOnTH) {
												outputs[TRIG_OUTPUT].setVoltage(10.f);
											} else {
												if (trigOnStart) {
													outTrig[0] = true;
													outTrigSample[0] = oneMsSamples;
												}
											}
										} else {
											holding[0] = false;
										}
									} else if (trigValue[0] < 1.f) {

										if (gateOnTH) {
											outputs[TRIG_OUTPUT].setVoltage(0.f);
										} else {
											if (holding[0] && trigOnEnd) {
												outTrig[0] = true;
												outTrigSample[0] = oneMsSamples;
											}
										}
										outputs[TRIG_OUTPUT].setChannels(1);

										holding[0] = false;
									}
									prevTrigValue[0] = trigValue[0];

									if (!holding[0]) {
										for (int c = 0; c < chan; c++) {

											out = ( inputs[IN_INPUT].getVoltage(c) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT].setVoltage(out, c);
										}							
									}

									outputs[OUT_OUTPUT].setChannels(chan);
								
								} else {	// TRACK & HOLD - SAMPLE ON HIGH GATE - polyphonic triggers

									for (int cT = 0; cT < chanTrig; cT++) {

										trigValue[cT] = inputs[TRIG_INPUT].getVoltage(cT);

										if (trigValue[cT] >= 1.f && prevTrigValue[cT] < 1.f) {
											probValue = random::uniform();
											if (probSetup >= probValue) {
												holding[cT] = true;
												//for (int c = 0; c < chan; c++) {

													out = ( inputs[IN_INPUT].getVoltage(cT) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

													if (out > 10.f)
														out = 10.f;
													else if (out < -10.f)
														out = -10.f;
													outputs[OUT_OUTPUT].setVoltage(out, cT);
												//}

												if (gateOnTH) {
													outputs[TRIG_OUTPUT].setVoltage(10.f, cT);
												} else {
													if (trigOnStart) {
														outTrig[cT] = true;
														outTrigSample[cT] = oneMsSamples;
													}
												}
											} else {
												holding[cT] = false;
											}
										} else if (trigValue[cT] < 1.f) {

											if (gateOnTH) {
												outputs[TRIG_OUTPUT].setVoltage(0.f, cT);
											} else {
												if (holding[cT] && trigOnEnd) {
													outTrig[cT] = true;
													outTrigSample[cT] = oneMsSamples;
												}
											}

											holding[cT] = false;
										}
										prevTrigValue[cT] = trigValue[cT];

										if (!holding[cT]) {
											//for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT].getVoltage(cT) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT].setVoltage(out, cT);
											//}							
										}
									}

									outputs[OUT_OUTPUT].setChannels(chanTrig);
									outputs[TRIG_OUTPUT].setChannels(chanTrig);

								}
							} else {	// TRACK & HOLD - Sample on HIGH gate - noise generator

								chanTrig = inputs[TRIG_INPUT].getChannels();

								if (chanTrig == 0)
									chanTrig = 1;

								for (int cT = 0; cT < chanTrig; cT++) {

									if (noiseType == FULL_NOISE) 
										out = ( (random::uniform() * 10 - 5.f) * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();
									else
										out = ( random::normal() * params[SCALE_PARAM].getValue() ) + params[OFFSET_PARAM].getValue();

									trigValue[cT] = inputs[TRIG_INPUT].getVoltage(cT);
									if (trigValue[cT] >= 1.f && prevTrigValue[cT] < 1.f) {
										probValue = random::uniform();
										if (probSetup >= probValue) {
											holding[cT] = true;
											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT].setVoltage(out, cT);
											if (gateOnTH) {
												outputs[TRIG_OUTPUT].setVoltage(10.f, cT);
											} else {
												if (trigOnStart) {
													outTrig[cT] = true;
													outTrigSample[cT] = oneMsSamples;
												}
											}
										} else {
											holding[cT] = false;
										}
									} else if (trigValue[cT] < 1.f) {

										if (gateOnTH) {
											outputs[TRIG_OUTPUT].setVoltage(0.f, cT);
										} else {
											if (holding[cT] && trigOnEnd) {
												outTrig[cT] = true;
												outTrigSample[cT] = oneMsSamples;
											}
										}
										holding[cT] = false;
									}
									prevTrigValue[cT] = trigValue[cT];

									if (!holding[cT]) {
										if (out > 10.f)
											out = 10.f;
										else if (out < -10.f)
											out = -10.f;

										outputs[OUT_OUTPUT].setVoltage(out, cT);
									}
								}
								outputs[OUT_OUTPUT].setChannels(chanTrig);
								outputs[TRIG_OUTPUT].setChannels(chanTrig);
							}
						break;
					}	
				break;
			}
		} else {
			outputs[OUT_OUTPUT].setVoltage(0.f, 0);
			outputs[OUT_OUTPUT].setChannels(1);
			outputs[TRIG_OUTPUT].setVoltage(0.f);
			outputs[TRIG_OUTPUT].setChannels(1);
		}

		for (int c = 0; c < 16; c++) {
			if (outTrig[c]) {
				outTrigSample[c]--;
				if (outTrigSample[c] > 0) {
					outputs[TRIG_OUTPUT].setVoltage(10.f, c);
				} else {
					outTrig[c] = false;
					outputs[TRIG_OUTPUT].setVoltage(0.f, c);
				}
			}
		}
	}
};

struct HolderCompactDebugDisplay : TransparentWidget {
	HolderCompact *module;
	int frame = 0;
	HolderCompactDebugDisplay() {
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

struct HolderCompactWidget : ModuleWidget {
	HolderCompactWidget(HolderCompact* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/HolderCompact.svg")));

		/*
		{
			HolderCompactDebugDisplay *display = new HolderCompactDebugDisplay();
			display->box.pos = Vec(23, 3);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float xCenter = 7.62f;

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xCenter, 16.4)), module, HolderCompact::MODE_SWITCH, HolderCompact::MODE_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, 28.8)), module, HolderCompact::TRIG_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, 45.5)), module, HolderCompact::IN_INPUT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, 61)), module, HolderCompact::PROB_PARAM));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xCenter, 74.5)), module, HolderCompact::SCALE_PARAM));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xCenter, 88.7)), module, HolderCompact::OFFSET_PARAM));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xCenter, 104.1)), module, HolderCompact::OUT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xCenter, 117.5)), module, HolderCompact::TRIG_OUTPUT));

	}

	void appendContextMenu(Menu* menu) override {
		HolderCompact* module = dynamic_cast<HolderCompact*>(this->module);

		struct ModeItem : MenuItem {
			HolderCompact* module;
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
		menu->addChild(createMenuLabel("Track & Hold options:"));

		menu->addChild(createBoolMenuItem("Sample on HIGH Gate", "", [=]() {
					return module->isSampleOnHighGate();
				}, [=](bool sampleOnHighGate) {
					module->setSampleOnGate(sampleOnHighGate);
			}));

		if (module->gateOnTH == true) {
			menu->addChild(createMenuLabel("Trig on start"));
			menu->addChild(createMenuLabel("Trig on end"));
		} else {
			menu->addChild(createBoolPtrMenuItem("Trig on start", "", &module->trigOnStart));
			menu->addChild(createBoolPtrMenuItem("Trig on end", "", &module->trigOnEnd));
		}

		menu->addChild(createBoolMenuItem("Gate Out instead Trig", "", [=]() {
					return module->isGateOut();
				}, [=](bool gateOut) {
					module->setGateOut(gateOut);
			}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Noise Generator preset", "", [=]() {module->setNoisePreset();}));
	}
};

Model* modelHolderCompact = createModel<HolderCompact, HolderCompactWidget>("HolderCompact");