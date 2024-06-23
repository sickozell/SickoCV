#define ENV_MODE 0
#define FUNC_MODE 1
#define LOOP_MODE 2

#define STOP_STAGE 0
#define ATTACK_STAGE 1
#define DECAY_STAGE 2
#define SUSTAIN_STAGE 3
#define RELEASE_STAGE 4

#define SLEW_START 2

#include "plugin.hpp"

//using namespace std;

struct Slewer : Module {

	enum ParamIds {
		ATTACK_PARAM,
		ATTACK_ATNV_PARAM,
		DECAY_PARAM,
		DECAY_ATNV_PARAM,
		SHAPE_PARAM,
		SHAPE_ATNV_PARAM,
		SYMM_PARAM,
		NUM_PARAMS 
	};
	enum InputIds {
		ATTACK_INPUT,
		DECAY_INPUT,
		SHAPE_INPUT,
		SIGNAL_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIGNAL_OUTPUT,
		EOA_OUTPUT,
		EOD_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		SYMM_LIGHT,
		EOA_LIGHT,
		EOD_LIGHT,
		NUM_LIGHTS
	};
  
	unsigned int sampleRate = APP->engine->getSampleRate();
	float sr = float(sampleRate);
	float srCoeff = 1 / sr;

	float shape;

	float normalShape;
	float normalStage;
	float normalCoeff;
	float normalCoeff2;
	float normalRange;

	float in[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevIn[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	int stage[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	float stageSample[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

	float stageLevel[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float stageCoeff = 1;
	float stageCoeff_D = 1;

	float attackValue;
	float decayValue;
	float attackInValue;
	float decayInValue;
	float attackKnob = 0.f;
	float prevAttackKnob = -1.f;
	float decayKnob = 0.f;
	float prevDecayKnob = -1.f;

	float slewStart[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float slewEnd[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	float slewLogCoeff[16] = {SLEW_START, SLEW_START, SLEW_START, SLEW_START, SLEW_START, SLEW_START, SLEW_START, SLEW_START, SLEW_START, SLEW_START, SLEW_START, SLEW_START, SLEW_START, SLEW_START, SLEW_START, SLEW_START};
	float slewExpCoeff[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	bool symm = false;
	bool prevStop[16] = {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true};

	float out = 0;

	int chan;

	bool slewKnobs = false;

	static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	
	static constexpr float minStageTimeSec = 0.001f;  // in seconds
	static constexpr float maxStageTimeSec = 10.f;  // in seconds
	
	const float maxAdsrTime = 10.f;
	const float minAdsrTime = 0.001f;
	

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

	Slewer() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(ATTACK_PARAM, 0.f, 1.f, 0.5f, "Attack", " ms", maxStageTime / minStageTime, minStageTime);
		configParam(ATTACK_ATNV_PARAM, -1.f, 1.f, 0.f, "Attack CV", "%", 0, 100);
		configInput(ATTACK_INPUT,"Attack");

		configParam(DECAY_PARAM, 0.f, 1.f, 0.5f, "Decay", " ms", maxStageTime / minStageTime, minStageTime);
		configParam(DECAY_ATNV_PARAM, -1.f, 1.f, 0.f, "Decay CV", "%", 0, 100);
		configInput(DECAY_INPUT,"Decay");

		configParam(SHAPE_PARAM, 0.f, 1.f, 0.5f, "Curve","", 0, 100);
		configParam(SHAPE_ATNV_PARAM, -1.f, 1.f, 0.f, "Curve CV", "%", 0, 100);
		configInput(SHAPE_INPUT,"Curve");

		configSwitch(SYMM_PARAM, 0.f, 1.f, 0.f, "Symmetric", {"OFF", "ON"});

		configInput(SIGNAL_INPUT,"Signal");
		configOutput(SIGNAL_OUTPUT,"Signal");

		configOutput(EOA_OUTPUT,"EoA");
		configOutput(EOD_OUTPUT,"EoD");

	}

	/*
	void onReset(const ResetEvent &e) override {

		Module::onReset(e);
	}
	*/

	void processBypass(const ProcessArgs &args) override {
		
		chan = inputs[SIGNAL_INPUT].getChannels();
		for (int c = 0; c < chan; c++)
			outputs[SIGNAL_OUTPUT].setVoltage(inputs[SIGNAL_INPUT].getVoltage(c), c);
		outputs[SIGNAL_OUTPUT].setChannels(chan);

		Module::processBypass(args);
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
		sr = float(sampleRate);
		srCoeff = 1 / sr;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "slewKnobs", json_boolean(slewKnobs));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* slewKnobsJ = json_object_get(rootJ, "slewKnobs");
		if (slewKnobsJ)
			slewKnobs = json_boolean_value(slewKnobsJ);
	}

	static float convertCVToSec(float cv) {	

		return minStageTimeSec * std::pow(maxStageTimeSec / minStageTimeSec, cv);

	}

	void process(const ProcessArgs &args) override {

		shape = params[SHAPE_PARAM].getValue() + (inputs[SHAPE_INPUT].getVoltage() * params[SHAPE_ATNV_PARAM].getValue() * 0.1);

		if (shape < 0)
			shape = 0;
		else if (shape > 1)
			shape = 1;

		symm = params[SYMM_PARAM].getValue();
		lights[SYMM_LIGHT].setBrightness(symm);

		// ----------------- attack / decay

		if (!slewKnobs) {
			attackKnob = params[ATTACK_PARAM].getValue();
			if (attackKnob != prevAttackKnob)
				attackValue = convertCVToSec(attackKnob);
			prevAttackKnob = attackKnob;

			if (!inputs[ATTACK_INPUT].isConnected()) {
				stageCoeff = srCoeff / attackValue;

			} else {

				attackInValue = attackValue + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACK_ATNV_PARAM].getValue() * 0.1);

				if (attackInValue < minAdsrTime)
					attackInValue = minAdsrTime;

				stageCoeff = srCoeff / attackInValue;
				
			}

			decayKnob = params[DECAY_PARAM].getValue();
			if (decayKnob != prevDecayKnob)
				decayValue = convertCVToSec(decayKnob);
			prevDecayKnob = decayKnob;

			if (!inputs[DECAY_INPUT].isConnected()) {
				stageCoeff_D = srCoeff / decayValue;

			} else {

				decayInValue = decayValue + (inputs[DECAY_INPUT].getVoltage() * params[DECAY_ATNV_PARAM].getValue() * 0.1);

				if (decayInValue < minAdsrTime)
					decayInValue = minAdsrTime;

				stageCoeff_D = srCoeff / decayInValue;
				
			}
		
		} else {

			attackKnob = params[ATTACK_PARAM].getValue();
			if (attackKnob != prevAttackKnob)
				attackValue = convertCVToSec(attackKnob);
			prevAttackKnob = attackKnob;

			if (!inputs[ATTACK_INPUT].isConnected()) {
				stageCoeff = srCoeff / attackValue;
				stageCoeff_D = srCoeff / attackValue;

			} else {

				attackInValue = attackValue + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACK_ATNV_PARAM].getValue() * 0.1);

				if (attackInValue < minAdsrTime)
					attackInValue = minAdsrTime;

				stageCoeff = srCoeff / attackInValue;
				stageCoeff_D = srCoeff / attackInValue;
				
			}
			
			decayKnob = params[DECAY_PARAM].getValue() + (inputs[DECAY_INPUT].getVoltage() * params[DECAY_ATNV_PARAM].getValue() * 0.1);
			
			if (decayKnob < 0.001)
				decayKnob = 0.001;
			else if (decayKnob > 0.999)
				decayKnob = 0.999;

			stageCoeff /= (1 - decayKnob);
			stageCoeff_D /= decayKnob;

		}

		// ---------------------------

		if (inputs[SIGNAL_INPUT].isConnected()) {

			chan = inputs[SIGNAL_INPUT].getChannels();

			for (int c = 0; c < chan; c++) {
				in[c] = inputs[SIGNAL_INPUT].getVoltage(c);

				if (in[c] == prevIn[c]) {

					slewStart[c] = in[c];

					stageSample[c] = 1;
					slewLogCoeff[c] = SLEW_START;
					slewExpCoeff[c] = 0;
					stage[c] = STOP_STAGE;

					prevStop[c] = true;

				//} else if (in > prevIn || in > slewEnd) {
				} else if (in[c] > prevIn[c]) {
					if (stage[c] == DECAY_STAGE) {
						stageSample[c] = 1;
						slewLogCoeff[c] = SLEW_START;
						slewExpCoeff[c] = 0;
						slewStart[c] = prevIn[c];
					} else if (stage[c] == ATTACK_STAGE) {
						if (prevStop[c]) {
							prevStop[c] = false;
							stageSample[c] = 1;
							slewLogCoeff[c] = SLEW_START;
							slewExpCoeff[c] = 0;
							slewStart[c] = prevIn[c];
						}
					}
					stage[c] = ATTACK_STAGE;
					
					slewEnd[c] = in[c];

				//} else if (in < prevIn || in < slewEnd) {
				} else if (in[c] < prevIn[c]) {
					if (stage[c] == ATTACK_STAGE) {
						stageSample[c] = 1;
						slewLogCoeff[c] = SLEW_START;
						slewExpCoeff[c] = 0;
						slewStart[c] = prevIn[c];
					} else if (stage[c] == DECAY_STAGE) {
						if (prevStop[c]) {
							prevStop[c] = false;
							stageSample[c] = 1;
							slewLogCoeff[c] = SLEW_START;
							slewExpCoeff[c] = 0;
							slewStart[c] = prevIn[c];
						}
						
					}

					stage[c] = DECAY_STAGE;

					slewEnd[c] = in[c];
				}

				switch (stage[c]) {

					case STOP_STAGE:
						outputs[EOA_OUTPUT].setVoltage(0.f, c);
						lights[EOA_LIGHT].setBrightness(0.f);
						outputs[EOD_OUTPUT].setVoltage(0.f, c);
						lights[EOD_LIGHT].setBrightness(0.f);
					break;

					case ATTACK_STAGE:

						outputs[EOA_OUTPUT].setVoltage(10.f, c);
						lights[EOA_LIGHT].setBrightness(1.f);
						outputs[EOD_OUTPUT].setVoltage(0.f, c);
						lights[EOD_LIGHT].setBrightness(0.f);

						normalCoeff = stageCoeff * 2.06;
						normalRange = slewEnd[c] - slewStart[c];
						normalCoeff2 = stageCoeff * normalRange;
						
						if (shape < 0.5) {
							normalShape = shape * 2;

							stageSample[c]++;
							slewLogCoeff[c] -= normalCoeff;
							
							if (slewLogCoeff[c] < 0.3)
								slewLogCoeff[c] = 0.3;

							stageLevel[c] += (slewLogCoeff[c] * normalCoeff2 * (1 - normalShape)) + (normalCoeff2 * normalShape);
						
						} else {
							normalShape = (shape - 0.5) * 2;

							slewExpCoeff[c] += normalCoeff;
							stageLevel[c] += (normalCoeff2 * (1 - normalShape)) + (slewExpCoeff[c] * normalCoeff2 * normalShape);
							stageSample[c]++;
						}

						if (stageLevel[c] >= slewEnd[c]) {
							stageLevel[c] = slewEnd[c];
							stage[c] = STOP_STAGE;

							prevStop[c] = true;
							stageSample[c] = 1;
							slewLogCoeff[c] = SLEW_START;
							slewExpCoeff[c] = 0;
						}

						
					break;

					case DECAY_STAGE:
						outputs[EOA_OUTPUT].setVoltage(0.f, c);
						lights[EOA_LIGHT].setBrightness(0.f);
						outputs[EOD_OUTPUT].setVoltage(10.f, c);
						lights[EOD_LIGHT].setBrightness(1.f);

						normalCoeff = stageCoeff_D * 2.06;
						normalRange = slewStart[c] - slewEnd[c];
						normalCoeff2 = stageCoeff_D * normalRange;

						if (!symm) {

							if (shape < 0.5) {
								normalShape = shape * 2;

								stageSample[c]++;
								slewLogCoeff[c] -= normalCoeff;
								
								if (slewLogCoeff[c] < 0.3)
									slewLogCoeff[c] = 0.3;

								stageLevel[c] -= (slewLogCoeff[c] * normalCoeff2 * (1 - normalShape)) + (normalCoeff2 * normalShape);
							
							} else {
								normalShape = (shape - 0.5) * 2;

								slewExpCoeff[c] += normalCoeff;
								stageLevel[c] -= (normalCoeff2 * (1 - normalShape)) + (slewExpCoeff[c] * normalCoeff2 * normalShape);
								stageSample[c]++;
							}

						} else {	// symmetric decay

							if (shape < 0.5) {
								normalShape = shape * 2;

								stageSample[c]++;
								slewExpCoeff[c] += normalCoeff;
								
								stageLevel[c] -= (slewExpCoeff[c] * normalCoeff2 * (1 - normalShape)) + (normalCoeff2 * normalShape);
							
							} else {
								normalShape = (shape - 0.5) * 2;

								slewLogCoeff[c] -= normalCoeff;
								if (slewLogCoeff[c] < 0.3)
									slewLogCoeff[c] = 0.3;

								stageLevel[c] -= (normalCoeff2 * (1 - normalShape)) + (slewLogCoeff[c] * normalCoeff2 * normalShape);
								stageSample[c]++;
							}
						}

					
						if (stageLevel[c] <= slewEnd[c]) {
							stageLevel[c] = slewEnd[c];
							stage[c] = STOP_STAGE;
							prevStop[c] = true;
							stageSample[c] = 1;
							slewLogCoeff[c] = SLEW_START;
							slewExpCoeff[c] = 0;
						}

					break;
				}


				prevIn[c] = stageLevel[c];

				// ----------------------------

				out = stageLevel[c];

				if (out > 10)
					out = 10;
				else if (out < -10)
					out = -10;

				outputs[SIGNAL_OUTPUT].setVoltage(out, c);

			}

			outputs[SIGNAL_OUTPUT].setChannels(chan);

		} else {
			
			if (outputs[SIGNAL_OUTPUT].isConnected()) {

				switch (stage[0]) {

					case STOP_STAGE:
						outputs[EOA_OUTPUT].setVoltage(0.f);
						lights[EOA_LIGHT].setBrightness(0.f);
						outputs[EOD_OUTPUT].setVoltage(0.f);
						lights[EOD_LIGHT].setBrightness(0.f);

						stageSample[0] = 1;
						slewLogCoeff[0] = SLEW_START;
						slewExpCoeff[0] = 0;
						slewStart[0] = 0;
						slewEnd[0] = 10;
						
						stage[0] = ATTACK_STAGE;
						
					break;

					case ATTACK_STAGE:

						outputs[EOA_OUTPUT].setVoltage(10.f);
						lights[EOA_LIGHT].setBrightness(1.f);
						outputs[EOD_OUTPUT].setVoltage(0.f);
						lights[EOD_LIGHT].setBrightness(0.f);

						normalCoeff = stageCoeff * 2.06;
						normalRange = slewEnd[0] - slewStart[0];
						normalCoeff2 = stageCoeff * normalRange;
						
						if (shape < 0.5) {
							normalShape = shape * 2;

							stageSample[0]++;
							slewLogCoeff[0] -= normalCoeff;
							
							if (slewLogCoeff[0] < 0.3)
								slewLogCoeff[0] = 0.3;

							stageLevel[0] += (slewLogCoeff[0] * normalCoeff2 * (1 - normalShape)) + (normalCoeff2 * normalShape);
						
						} else {
							normalShape = (shape - 0.5) * 2;

							slewExpCoeff[0] += normalCoeff;
							stageLevel[0] += (normalCoeff2 * (1 - normalShape)) + (slewExpCoeff[0] * normalCoeff2 * normalShape);
							stageSample[0]++;
						}
						
						if (stageLevel[0] >= slewEnd[0]) {

							stageSample[0] = 1;
							slewLogCoeff[0] = SLEW_START;
							slewExpCoeff[0] = 0;
							slewStart[0] = prevIn[0];
							prevStop[0] = false;
							
							stage[0] = DECAY_STAGE;

							//slewEnd = in;
							slewStart[0] = 10;
							slewEnd[0] = 0;
						}

						
					break;

					case DECAY_STAGE:
						outputs[EOA_OUTPUT].setVoltage(0.f);
						lights[EOA_LIGHT].setBrightness(0.f);
						outputs[EOD_OUTPUT].setVoltage(10.f);
						lights[EOD_LIGHT].setBrightness(1.f);

						normalCoeff = stageCoeff_D * 2.06;
						normalRange = slewStart[0] - slewEnd[0];
						normalCoeff2 = stageCoeff_D * normalRange;

						if (!symm) {

							if (shape < 0.5) {
								normalShape = shape * 2;

								stageSample[0]++;
								slewLogCoeff[0] -= normalCoeff;
								
								if (slewLogCoeff[0] < 0.3)
									slewLogCoeff[0] = 0.3;

								stageLevel[0] -= (slewLogCoeff[0] * normalCoeff2 * (1 - normalShape)) + (normalCoeff2 * normalShape);
							
							} else {
								normalShape = (shape - 0.5) * 2;

								slewExpCoeff[0] += normalCoeff;
								stageLevel[0] -= (normalCoeff2 * (1 - normalShape)) + (slewExpCoeff[0] * normalCoeff2 * normalShape);
								stageSample[0]++;
							}

						} else {	// symmetric decay

							if (shape < 0.5) {
								normalShape = shape * 2;

								stageSample[0]++;
								slewExpCoeff[0] += normalCoeff;
								
								stageLevel[0] -= (slewExpCoeff[0] * normalCoeff2 * (1 - normalShape)) + (normalCoeff2 * normalShape);
							
							} else {
								normalShape = (shape - 0.5) * 2;

								slewLogCoeff[0] -= normalCoeff;
								if (slewLogCoeff[0] < 0.3)
									slewLogCoeff[0] = 0.3;

								stageLevel[0] -= (normalCoeff2 * (1 - normalShape)) + (slewLogCoeff[0] * normalCoeff2 * normalShape);
								stageSample[0]++;
							}
						}

						if (stageLevel[0] <= slewEnd[0]) {

							stageSample[0] = 1;
							slewLogCoeff[0] = SLEW_START;
							slewExpCoeff[0] = 0;
							slewStart[0] = 0;
							slewEnd[0] = 10;
							
							stage[0] = ATTACK_STAGE;
						}

					break;
				}

				prevIn[0] = stageLevel[0];

				// ----------------------------

				out = stageLevel[0];

				if (out > 10)
					out = 10;
				else if (out < -10)
					out = -10;

				outputs[SIGNAL_OUTPUT].setVoltage(out);

				outputs[SIGNAL_OUTPUT].setChannels(1);

			} else {
				
				for (int c = 0; c < 16; c++) {
					stage[c] = STOP_STAGE;
					prevStop[c] = true;
					stageSample[c] = 1;
					slewLogCoeff[c] = SLEW_START;
					slewExpCoeff[c] = 0;
					prevIn[c] = 0;
				}
				outputs[SIGNAL_OUTPUT].clearVoltages();
				outputs[EOA_OUTPUT].clearVoltages();
				outputs[EOD_OUTPUT].clearVoltages();

				outputs[SIGNAL_OUTPUT].setChannels(1);
				outputs[EOA_OUTPUT].setChannels(1);
				outputs[EOD_OUTPUT].setChannels(1);

				lights[EOA_LIGHT].setBrightness(0);
				lights[EOD_LIGHT].setBrightness(0);
			}

		}

	}
};

/*
struct SlewerDebugDisplay : TransparentWidget {
	Slewer *module;
	int frame = 0;
	SlewerDebugDisplay() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-bold.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff)); 
				
				nvgTextBox(args.vg, 9, 6,120, module->debugDisplay.c_str(), NULL);
				nvgTextBox(args.vg, 9, 16,120, module->debugDisplay2.c_str(), NULL);
				nvgTextBox(args.vg, 9, 26,120, module->debugDisplay3.c_str(), NULL);
				nvgTextBox(args.vg, 9, 36,120, module->debugDisplay4.c_str(), NULL);

			}
		}
		Widget::drawLayer(args, layer);
	}
};
*/

struct SlewerWidget : ModuleWidget {
	SlewerWidget(Slewer *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Slewer.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		/*
		{
			SlewerDebugDisplay *display = new SlewerDebugDisplay();
			display->box.pos = Vec(70, 20);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/

		const float xAttKn = 12.9;
		const float yAttKn = 20.15;

		const float xAttIn = 7;
		const float yAttIn = 30.5;

		const float xAttAtn = 18;
		const float yAttAtn = 30.652;

		const float xDecIn = 7;
		const float yDecIn = 47.2;

		const float xDecAtn = 18;
		const float yDecAtn = 47.352;

		const float xDecKn = 12.9;
		const float yDecKn = 57.65;

		const float xShpKn = 8.85;
		const float yShpKn = 75.9;

		const float xSym = 19;
		const float ySym = 75.9;

		const float xShpIn = 7;
		const float yShpIn = 88.5;

		const float xShpAtn = 18;
		const float yShpAtn = 88.352;

		const float xEoa = 6.5 - 0.3;
		const float yEoa = 103.5;

		const float xEoaL = 6.5 + 4;
		const float yEoaL = 103.5 - 3;

		const float xEod = 18.8 - 0.3;
		const float yEod = 103.5;

		const float xEodL = 18.8 + 4;
		const float yEodL = 103.5 - 3;

		const float xIn = 6.5;
		const float yIn = 117.5;

		const float xOut = 18.8;
		const float yOut = 117.5;

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xAttKn, yAttKn)), module, Slewer::ATTACK_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xAttIn, yAttIn)), module, Slewer::ATTACK_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xAttAtn, yAttAtn)), module, Slewer::ATTACK_ATNV_PARAM));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xDecIn, yDecIn)), module, Slewer::DECAY_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xDecAtn, yDecAtn)), module, Slewer::DECAY_ATNV_PARAM));
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xDecKn, yDecKn)), module, Slewer::DECAY_PARAM));
		
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xShpKn, yShpKn)), module, Slewer::SHAPE_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xShpIn, yShpIn)), module, Slewer::SHAPE_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xShpAtn, yShpAtn)), module, Slewer::SHAPE_ATNV_PARAM));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xSym, ySym)), module, Slewer::SYMM_PARAM, Slewer::SYMM_LIGHT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xEoa, yEoa)), module, Slewer::EOA_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xEod, yEod)), module, Slewer::EOD_OUTPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xIn, yIn)), module, Slewer::SIGNAL_INPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yOut)), module, Slewer::SIGNAL_OUTPUT));

		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(xEoaL, yEoaL)), module, Slewer::EOA_LIGHT));
		addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(xEodL, yEodL)), module, Slewer::EOD_LIGHT));

	}

	void appendContextMenu(Menu* menu) override {
		Slewer* module = dynamic_cast<Slewer*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Duration/Slew knobs", "", &module->slewKnobs));

	}
};

Model *modelSlewer = createModel<Slewer, SlewerWidget>("Slewer");
