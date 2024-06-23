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
	
	float trigValue[MAX_TRACKS][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}};

	float prevTrigValue[MAX_TRACKS][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}};
	
	int sampleOnGate = 0;

	float out = 0.f;
	int chan = 1;
	int chanTrig = 1;

	float probSetup[MAX_TRACKS] = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};
	float probValue = 0.f;

	bool holding[MAX_TRACKS][16] = {{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
									{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
									{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
									{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
									{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
									{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
									{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
									{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}};

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
			for (int i = 0; i < 16; i++) {
				trigValue[track][i] = 0;
				prevTrigValue[track][i] = 0;
				holding[track][i] = false;
			}
			mode[track] = SAMPLE_HOLD;
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
				for (int i = 0; i < 16; i++)
					holding[track][i] = true;
		} else {
			sampleOnGate = 0;
			for (int track = 0; track < MAX_TRACKS; track++)
				for (int i = 0; i < 16; i++)
					holding[track][i] = false;
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
							
							chan = inputs[IN_INPUT+track].getChannels();
							chanTrig = inputs[TRIG_INPUT+track].getChannels();

							if (chanTrig == 1) {	// SAMPLE & HOLD - mono Trigger
								trigValue[track][0] = inputs[TRIG_INPUT+track].getVoltage();

								if (trigValue[track][0] >= 1.f && prevTrigValue[track][0] < 1.f) {
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
								prevTrigValue[track][0] = trigValue[track][0];
								outputs[OUT_OUTPUT+track].setChannels(chan);
							} else {  // SAMPLE & HOLD - poly Triggers
								for (int cT = 0; cT < chanTrig; cT++) {
									trigValue[track][cT] = inputs[TRIG_INPUT+track].getVoltage(cT);
									if (trigValue[track][cT] >= 1.f && prevTrigValue[track][cT] < 1.f) {
										probValue = random::uniform();
										if (probSetup[track] >= probValue) {
											out = ( inputs[IN_INPUT+track].getVoltage(cT) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT+track].setVoltage(out, cT);
										}
									}
									prevTrigValue[track][cT] = trigValue[track][cT];
								}
								outputs[OUT_OUTPUT+track].setChannels(chanTrig);
							}
							
						} else { 	// Sample & Hold with noise generator
							
							chanTrig = inputs[TRIG_INPUT].getChannels();

							for (int cT = 0; cT < chanTrig; cT++) {
								trigValue[track][cT] = inputs[TRIG_INPUT+track].getVoltage(cT);
								if (trigValue[track][cT] >= 1.f && prevTrigValue[track][cT] < 1.f) {
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

										outputs[OUT_OUTPUT+track].setVoltage(out, cT);

									}
								}
								prevTrigValue[track][cT] = trigValue[track][cT];
							}
							outputs[OUT_OUTPUT+track].setChannels(chanTrig);

						}
					
					break;

					case TRACK_HOLD:
						switch (sampleOnGate) {
							case 0:	// sample on LOW GATE
								if (inputs[IN_INPUT+track].isConnected()) {

									chan = inputs[IN_INPUT+track].getChannels();
									chanTrig = inputs[TRIG_INPUT+track].getChannels();

									if (chanTrig == 1) {	// monophonic trigger

										trigValue[track][0] = inputs[TRIG_INPUT+track].getVoltage();

										if (trigValue[track][0] >= 1.f && prevTrigValue[track][0] < 1.f) {
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

												holding[track][0] = false;
											}
										} else if (trigValue[track][0] >= 1.f && !holding[track][0]) {
											for (int c = 0; c < chan; c++) {

												out = ( inputs[IN_INPUT+track].getVoltage(c) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT+track].setVoltage(out, c);

											}

										} else if (trigValue[track][0] < 1) {
											if (!holding[track][0]) {
												holding[track][0] = true;
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

										prevTrigValue[track][0] = trigValue[track][0];
										outputs[OUT_OUTPUT+track].setChannels(chan);
									
									} else {	// TRACK & HOLD  SAMPLE ON LOW GATE: polyphonic Triggers

										for (int cT = 0; cT < chanTrig; cT++) {
											trigValue[track][cT] = inputs[TRIG_INPUT+track].getVoltage(cT);

											if (trigValue[track][cT] >= 1.f && prevTrigValue[track][cT] < 1.f) {
												probValue = random::uniform();
												if (probSetup[track] >= probValue) {
													//for (int c = 0; c < chan; c++) {

														out = ( inputs[IN_INPUT+track].getVoltage(cT) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

														if (out > 10.f)
															out = 10.f;
														else if (out < -10.f)
															out = -10.f;

														outputs[OUT_OUTPUT+track].setVoltage(out, cT);

													//}

													holding[track][cT] = false;
												}
											} else if (trigValue[track][cT] >= 1.f && !holding[track][cT]) {
												//for (int c = 0; c < chan; c++) {

													out = ( inputs[IN_INPUT+track].getVoltage(cT) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

													if (out > 10.f)
														out = 10.f;
													else if (out < -10.f)
														out = -10.f;

													outputs[OUT_OUTPUT+track].setVoltage(out, cT);

												//}

											} else if (trigValue[track][cT] < 1) {
												if (!holding[track][cT]) {
													holding[track][cT] = true;
													//for (int c = 0; c < chan; c++) {

														out = ( inputs[IN_INPUT+track].getVoltage(cT) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

														if (out > 10.f)
															out = 10.f;
														else if (out < -10.f)
															out = -10.f;

														outputs[OUT_OUTPUT+track].setVoltage(out, cT);

													//}
												}
											}

											prevTrigValue[track][cT] = trigValue[track][cT];
											
										}
										outputs[OUT_OUTPUT+track].setChannels(chanTrig);
									}

								} else { // TRACK & HOLD - SAMPLE ON LOW GATE - with Noise Generator

									chanTrig = inputs[TRIG_INPUT+track].getChannels();

									for (int cT = 0; cT < chanTrig; cT++) {

										trigValue[track][cT] = inputs[TRIG_INPUT+track].getVoltage(cT);

										if (trigValue[track][cT] >= 1.f && prevTrigValue[track][cT] < 1.f) {
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

												outputs[OUT_OUTPUT+track].setVoltage(out, cT);

												holding[track][cT] = false;
											}
										} else if (trigValue[track][cT] >= 1.f && !holding[track][cT]) {

											if (noiseType == FULL_NOISE) 
												out = ( (random::uniform() * 10 - 5.f) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();
											else
												out = ( random::normal() * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT+track].setVoltage(out, cT);

										} else if (trigValue[track][cT] < 1) {
											if (!holding[track][cT]) {
												holding[track][cT] = true;

												if (noiseType == FULL_NOISE) 
													out = ( (random::uniform() * 10 - 5.f) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();
												else
													out = ( random::normal() * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT+track].setVoltage(out, cT);

											}
										}

										prevTrigValue[track][cT] = trigValue[track][cT];
									}
									outputs[OUT_OUTPUT+track].setChannels(chanTrig);

								}

							break;

							case 1:	// sample on HIGH GATE
								if (inputs[IN_INPUT+track].isConnected()) {
									chan = inputs[IN_INPUT].getChannels();
									chanTrig = inputs[TRIG_INPUT].getChannels();

									if (chanTrig == 1) {	// TRACK & HOLD - SAMPLE ON HIGH GATE - monophonic trigger
										
										trigValue[track][0] = inputs[TRIG_INPUT+track].getVoltage();
										chan = inputs[IN_INPUT+track].getChannels();
										if (trigValue[track][0] >= 1.f && prevTrigValue[track][0] < 1.f) {
											probValue = random::uniform();
											if (probSetup[track] >= probValue) {
												holding[track][0] = true;
												for (int c = 0; c < chan; c++) {

													out = ( inputs[IN_INPUT+track].getVoltage(c) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

													if (out > 10.f)
														out = 10.f;
													else if (out < -10.f)
														out = -10.f;

													outputs[OUT_OUTPUT+track].setVoltage(out, c);
												}

											} else {

												holding[track][0] = false;

											}

										} else if (trigValue[track][0] < 1.f) {

											holding[track][0] = false;

										}
										prevTrigValue[track][0] = trigValue[track][0];

										if (!holding[track][0]) {
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
									
									} else {	// TRACK & HOLD - SAMPLE ON HIGH GATE - polyphonic triggers


										for (int cT = 0; cT < chanTrig; cT++) {
											trigValue[track][cT] = inputs[TRIG_INPUT+track].getVoltage(cT);
											
											if (trigValue[track][cT] >= 1.f && prevTrigValue[track][cT] < 1.f) {
												probValue = random::uniform();
												if (probSetup[track] >= probValue) {
													holding[track][cT] = true;
													//for (int c = 0; c < chan; c++) {

														out = ( inputs[IN_INPUT+track].getVoltage(cT) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

														if (out > 10.f)
															out = 10.f;
														else if (out < -10.f)
															out = -10.f;

														outputs[OUT_OUTPUT+track].setVoltage(out, cT);
													//}

												} else {

													holding[track][cT] = false;

												}

											} else if (trigValue[track][cT] < 1.f) {

												holding[track][cT] = false;

											}
											prevTrigValue[track][cT] = trigValue[track][cT];

											if (!holding[track][cT]) {
												//for (int c = 0; c < chan; c++) {

													out = ( inputs[IN_INPUT+track].getVoltage(cT) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

													if (out > 10.f)
														out = 10.f;
													else if (out < -10.f)
														out = -10.f;

													outputs[OUT_OUTPUT+track].setVoltage(out, cT);
												//}							
											}
										}
										outputs[OUT_OUTPUT+track].setChannels(chanTrig);

									}

								} else {	// TRACK & HOLD - Sample on HIGH gate - noise generator

									chanTrig = inputs[TRIG_INPUT+track].getChannels();

									if (chanTrig == 0)
										chanTrig = 1;

									for (int cT = 0; cT < chanTrig; cT++) {

										if (noiseType == FULL_NOISE) 
											out = ( (random::uniform() * 10 - 5.f) * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();
										else
											out = ( random::normal() * params[SCALE_PARAM+track].getValue() ) + params[OFFSET_PARAM+track].getValue();

										trigValue[track][cT] = inputs[TRIG_INPUT+track].getVoltage(cT);
										if (trigValue[track][cT] >= 1.f && prevTrigValue[track][cT] < 1.f) {
											probValue = random::uniform();
											if (probSetup[track] >= probValue) {
												holding[track][cT] = true;
												if (out > 10.f)
													out = 10.f;
												else if (out < -10.f)
													out = -10.f;

												outputs[OUT_OUTPUT+track].setVoltage(out, cT);
												
											} else {
												holding[track][cT] = false;
											}
										} else if (trigValue[track][cT] < 1.f) {

											holding[track][cT] = false;
										}
										prevTrigValue[track][cT] = trigValue[track][cT];

										if (!holding[track][cT]) {
											if (out > 10.f)
												out = 10.f;
											else if (out < -10.f)
												out = -10.f;

											outputs[OUT_OUTPUT+track].setVoltage(out, cT);
										}
									}
									outputs[OUT_OUTPUT+track].setChannels(chanTrig);
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

		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	 

		constexpr float yStart = 19.f;
		constexpr float yDelta = 14.f;

		const float xTrg = 6.6;
		const float xMode = 15.6;
		const float xIn = 24.6;
		const float xProb = 34.7;
		const float xScale = 45;
		const float xOffset = 55.1;
		const float xOut = 64.8;

		for (int track = 0; track < MAX_TRACKS; track++) {

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xTrg, yStart+(track*yDelta))), module, Holder8::TRIG_INPUT+track));

			addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xMode, yStart+(track*yDelta))), module, Holder8::MODE_SWITCH+track, Holder8::MODE_LIGHT+track));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xIn, yStart+(track*yDelta))), module, Holder8::IN_INPUT+track));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xProb, yStart+(track*yDelta))), module, Holder8::PROB_PARAM+track));

			addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xScale, yStart+(track*yDelta))), module, Holder8::SCALE_PARAM+track));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xOffset, yStart+(track*yDelta))), module, Holder8::OFFSET_PARAM+track));

			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yStart+(track*yDelta))), module, Holder8::OUT_OUTPUT+track));
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