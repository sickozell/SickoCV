#define ENV_MODE 0
#define FUNC_MODE 1
#define LOOP_MODE 2

#define STOP_STAGE 0
#define ATTACK_STAGE 1
#define DECAY_STAGE 2
#define SUSTAIN_STAGE 3
#define RELEASE_STAGE 4
#define MONOPHONIC 0
#define POLYPHONIC 1

#include "plugin.hpp"

using namespace std;

struct Enver : Module {

	#include "shapes.hpp"

	enum ParamIds {
		TRIGBUT_PARAM,
		MODE_SWITCH,
		SHAPE_PARAM,
		SHAPEATNV_PARAM,
		ATTACK_PARAM,
		ATTACKATNV_PARAM,
		DECAY_PARAM,
		DECAYATNV_PARAM,
		SUSTAIN_PARAM,
		SUSTAINATNV_PARAM,
		RELEASE_PARAM,
		RELEASEATNV_PARAM,
		VOL_PARAM,
		NUM_PARAMS 
	};
	enum InputIds {
		TRIG_INPUT,
		RETRIG_INPUT,
		SHAPE_INPUT,
		ATTACK_INPUT,
		DECAY_INPUT,
		SUSTAIN_INPUT,
		RELEASE_INPUT,
		LEFT_INPUT,
		RIGHT_INPUT,
		VOL_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ATTACK_OUTPUT,
		DECAY_OUTPUT,
		SUSTAIN_OUTPUT,
		RELEASE_OUTPUT,
		ENV_OUTPUT,
		INV_OUTPUT,
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		TRIGBUT_LIGHT,
		NUM_LIGHTS
	};
  
	unsigned int sampleRate = APP->engine->getSampleRate();
	float oneMsSamples = sampleRate / 1000;
	float tenSmS = sampleRate * 10;

	int mode = 0;

	float shape;

	float trigButton = 0;

	float trigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevTrigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	float retrigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevRetrigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	int polyOuts = POLYPHONIC;
	int polyMaster = POLYPHONIC;
	
	int stage[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float stageSample[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float maxStageSample[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float stageLevel[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


	int chanVca = 1;
	int chanTrig = 1;

	float env[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float outSignal = 0.f;

	float attackValue;
	float decayValue;
	float sustainValue;
	float releaseValue;

	float refValue[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

	float stageCoeff[16];

	float attackKnob = 0.f;
	float prevAttackKnob = -1.f;
	float decayKnob = 0.f;
	float prevDecayKnob = -1.f;
	float sustainKnob = 0.f;
	float prevSustainKnob = -1.f;
	float releaseKnob = 0.f;
	float prevReleaseKnob = -1.f;

	float masterLevel;
	float volLevel;

	bool eoA[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool eoD[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool eoS[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool eoR[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	float eoAtime[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float eoDtime[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float eoStime[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float eoRtime[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};


	static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	const float maxAdsrTime = 10.f;
	const float minAdsrTime = 0.001f;

	Enver() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configSwitch(MODE_SWITCH, 0.f, 2.f, 0.f, "Mode", {"Envelope", "Function", "Loop"});
		configSwitch(TRIGBUT_PARAM, 0.f, 1.f, 0.f, "Gate/Trig", {"OFF", "ON"});
		configInput(TRIG_INPUT,"Gate/Trig");
		configInput(RETRIG_INPUT,"Retrig");

		configParam(SHAPE_PARAM, 0.f, 1.f, 0.5f, "Shape","", 0, 100);
		configParam(SHAPEATNV_PARAM, -1.f, 1.f, 0.f, "Shape CV","%", 0, 100);
		configInput(SHAPE_INPUT,"Shape");

		configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Attack", " ms", maxStageTime / minStageTime, minStageTime);
		configParam(ATTACKATNV_PARAM, -1.f, 1.f, 0.f, "Attack CV","%", 0, 100);
		configInput(ATTACK_INPUT,"Attack");

		configParam(DECAY_PARAM, 0.f, 1.f, 0.f, "Decay", " ms", maxStageTime / minStageTime, minStageTime);
		configParam(DECAYATNV_PARAM, -1.f, 1.f, 0.f, "Decay CV","%", 0, 100);
		configInput(DECAY_INPUT,"Decay");

		configParam(SUSTAIN_PARAM, 0.f, 1.f, 1.f, "Sustain","%", 0, 100);
		configParam(SUSTAINATNV_PARAM, -1.f, 1.f, 0.f, "Sustain CV","%", 0, 100);
		configInput(SUSTAIN_INPUT,"Sustain");

		configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "Release", " ms", maxStageTime / minStageTime, minStageTime);
		configParam(RELEASEATNV_PARAM, -1.f, 1.f, 0.f, "Release CV","%", 0, 100);
		configInput(RELEASE_INPUT,"Release");
		
		configOutput(ATTACK_OUTPUT,"Attack");
		configOutput(DECAY_OUTPUT,"Decay");
		configOutput(SUSTAIN_OUTPUT,"Sustain");
		configOutput(RELEASE_OUTPUT,"Release");
		configOutput(ENV_OUTPUT,"Envelope");
		configOutput(INV_OUTPUT,"Inv.Envelope");

		configInput(LEFT_INPUT,"Left");
		configInput(RIGHT_INPUT,"Right");

		configParam(VOL_PARAM, 0.f, 1.f, 1.f, "Vol","", 0, 100);
		configInput(VOL_INPUT,"Vol");

		configOutput(LEFT_OUTPUT,"Left");
		configOutput(RIGHT_OUTPUT,"Right");

	}

	/*
	struct EnverSwitch : SickoSwitch_CKSS_Three_Vert
    {
        Enver* enver;
    };
    */

	void onReset(const ResetEvent &e) override {
		/*
		for (int i = 0; i < 16; i++) {
			play[i] = false;
			stage[i] = STOP_STAGE;
			stageLevel[i] = 0;
			voct[i] = 0.f;
			prevVoct[i] = 11.f;
			reversePlaying[i] = FORWARD;
		}
		clearSlot();
		antiAlias = 1;
		polyOuts = POLYPHONIC;
		polyMaster = POLYPHONIC;
		disableNav = false;
		sampleInPatch = true;
		prevTune = -1.f;
		reverseStart = false;
		system::removeRecursively(getPatchStorageDirectory().c_str());
		*/
		Module::onReset(e);
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
		oneMsSamples = sampleRate / 1000;
		tenSmS = sampleRate * 10;
		//sampleCoeff = sampleRate / (APP->engine->getSampleRate());			// the % distance between samples at speed 1x
	}

	/*
	json_t *dataToJson() override {
		
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "AntiAlias", json_integer(antiAlias));
		json_object_set_new(rootJ, "PolyOuts", json_integer(polyOuts));
		json_object_set_new(rootJ, "PolyMaster", json_integer(polyMaster));
		json_object_set_new(rootJ, "DisableNav", json_boolean(disableNav));
		json_object_set_new(rootJ, "sampleInPatch", json_boolean(sampleInPatch));
		json_object_set_new(rootJ, "Slot", json_string(storedPath.c_str()));
		json_object_set_new(rootJ, "UserFolder", json_string(userFolder.c_str()));
		
		return rootJ;

	}


	void dataFromJson(json_t *rootJ) override {
		
		json_t* antiAliasJ = json_object_get(rootJ, "AntiAlias");
		if (antiAliasJ)
			antiAlias = json_integer_value(antiAliasJ);
		json_t* polyOutsJ = json_object_get(rootJ, "PolyOuts");
		if (polyOutsJ)
			polyOuts = json_integer_value(polyOutsJ);
		json_t* polyMasterJ = json_object_get(rootJ, "PolyMaster");
		if (polyMasterJ)
			polyMaster = json_integer_value(polyMasterJ);
		json_t* disableNavJ = json_object_get(rootJ, "DisableNav");
		if (disableNavJ)
			disableNav = json_boolean_value(disableNavJ);
		json_t* sampleInPatchJ = json_object_get(rootJ, "sampleInPatch");
		if (sampleInPatchJ)
			sampleInPatch = json_boolean_value(sampleInPatchJ);
		json_t *slotJ = json_object_get(rootJ, "Slot");
		if (slotJ) {
			storedPath = json_string_value(slotJ);
			if (storedPath != "")
				loadSample(storedPath);
		}
		json_t *userFolderJ = json_object_get(rootJ, "UserFolder");
		if (userFolderJ) {
			userFolder = json_string_value(userFolderJ);
			if (userFolder != "") {
				createFolder(userFolder);
				if (rootFound) {
					folderTreeData.push_back(tempTreeData);
					folderTreeDisplay.push_back(tempTreeDisplay);
				}
			}
		}
		
	}
	*/
	
	/*
	bool isPolyOuts() {
		return polyOuts;
	}

	void setPolyOuts(bool poly) {
		if (poly) 
			polyOuts = 1;
		else {
			polyOuts = 0;
			outputs[OUT_OUTPUT].setChannels(1);
		}
	}
	*/

	float shapeResponse(float value) {
		
		//return ((expTable[0][int(expTableCoeff * value)] * shape) + (expTable[1][int(expTableCoeff * value)] * (1-shape)));

		/*
		float a;
		float b;
		
		if (shape < 0.25f) {
			a = expTable[0][int(expTableCoeff * value)] * (1 - shape * 4.f);
			b = expTable[2][int(expTableCoeff * value)] * (shape * 4.f);
		} else if (shape < 0.5f) {
			a = expTable[2][int(expTableCoeff * value)] * (1 - ((shape - 0.25f) * 4.f));
			b = value * ((shape - 0.25f) * 4.f);
		} else if (shape < 0.75f) {
			a = value * (1 - ((shape - 0.5f) * 4.f));
			b = expTable[2][int(expTableCoeff * value)] * ((shape - 0.5f) * 4.f);
		} else {
			a = expTable[2][int(expTableCoeff * value)] * (1 - ((shape - 0.75f) * 4.f));
			b = expTable[1][int(expTableCoeff * value)] * ((shape - 0.75f) * 4.f);
		}

		return a+b;
		*/

		if (shape < 0.25f) {
			return (expTable[0][int(expTableCoeff * value)] * (1 - shape * 4.f)) + (expTable[2][int(expTableCoeff * value)] * (shape * 4.f));
		} else if (shape < 0.5f) {
			return (expTable[2][int(expTableCoeff * value)] * (1 - ((shape - 0.25f) * 4.f))) + (value * ((shape - 0.25f) * 4.f));
		} else if (shape < 0.75f) {
			return (value * (1 - ((shape - 0.5f) * 4.f))) + (expTable[2][int(expTableCoeff * value)] * ((shape - 0.5f) * 4.f));
		} else {
			return (expTable[2][int(expTableCoeff * value)] * (1 - ((shape - 0.75f) * 4.f))) + (expTable[1][int(expTableCoeff * value)] * ((shape - 0.75f) * 4.f));
		}		
	}

	static float convertCVToSeconds(float cv) {		
		return minStageTime * std::pow(maxStageTime / minStageTime, cv) / 1000;
	}

	static float convertCVToMs(float cv) {		
		return minStageTime * std::pow(maxStageTime / minStageTime, cv);
	}

	void process(const ProcessArgs &args) override {

		mode = params[MODE_SWITCH].getValue();

		shape = params[SHAPE_PARAM].getValue() + (inputs[SHAPE_INPUT].getVoltage() * params[SHAPEATNV_PARAM].getValue() * 0.1);
		if (shape > 1)
			shape = 1;
		else if (shape < 0)
			shape = 0;

		sustainValue = params[SUSTAIN_PARAM].getValue() + (inputs[SUSTAIN_INPUT].getVoltage() * params[SUSTAINATNV_PARAM].getValue() * 0.1);
		if (sustainValue > 1)
			sustainValue = 1;
		else if (sustainValue < 0)
			sustainValue = 0;

		if (inputs[TRIG_INPUT].isConnected())
			chanTrig = inputs[TRIG_INPUT].getChannels();
		else
			chanTrig = 1;

		trigButton = params[TRIGBUT_PARAM].getValue();
		lights[TRIGBUT_LIGHT].setBrightness(trigButton);

		for (int c = 0; c < chanTrig; c++) {

			trigValue[c] = inputs[TRIG_INPUT].getVoltage(c) + trigButton;
			//lights[TRIGBUT_LIGHT].setBrightness(trigValue[c]);

			switch (mode) {
				case ENV_MODE:
					if (trigValue[c] >= 1.f) {
						if (stage[c] == STOP_STAGE) {
							stage[c] = ATTACK_STAGE;
							stageSample[c] = 0;
							refValue[c] = 0;
						} else {
							if (stage[c] == RELEASE_STAGE) {
								stage[c] = ATTACK_STAGE;
								stageSample[c] = 0;
								refValue[c] = stageLevel[c];

								eoR[c] = true;
								eoRtime[c] = oneMsSamples;
								outputs[RELEASE_OUTPUT].setVoltage(10.f, c);
							}
						}
					} else {
						if (stage[c] != STOP_STAGE) {
							if (stage[c] != RELEASE_STAGE) {
								if (stage[c] == SUSTAIN_STAGE) {
									eoS[c] = true;
									eoStime[c] = oneMsSamples;
									outputs[SUSTAIN_OUTPUT].setVoltage(10.f, c);
								} else	if (stage[c] == ATTACK_STAGE) {
									eoA[c] = true;
									eoAtime[c] = oneMsSamples;
									outputs[ATTACK_OUTPUT].setVoltage(10.f, c);
								} else if (stage[c] == DECAY_STAGE) {
									eoD[c] = true;
									eoDtime[c] = oneMsSamples;
									outputs[DECAY_OUTPUT].setVoltage(10.f, c);
								}
								refValue[c] = stageLevel[c];
								stage[c] = RELEASE_STAGE;
								stageSample[c] = 0;
							}
						}
					}
				break;

				case FUNC_MODE:
					if (trigValue[c] >= 1.f && prevTrigValue[c] < 1.f) {
						if (stage[c] == STOP_STAGE) {
							stage[c] = ATTACK_STAGE;
							stageSample[c] = 0;
							refValue[c] = 0;
						} else {
							if (stage[c] == RELEASE_STAGE) {
								stage[c] = ATTACK_STAGE;

								eoR[c] = true;
								eoRtime[c] = oneMsSamples;
								outputs[RELEASE_OUTPUT].setVoltage(10.f, c);
							} else {
								if (stage[c] == SUSTAIN_STAGE) {
									eoS[c] = true;
									eoStime[c] = oneMsSamples;
									outputs[SUSTAIN_OUTPUT].setVoltage(10.f, c);
								} else	if (stage[c] == ATTACK_STAGE) {
									eoA[c] = true;
									eoAtime[c] = oneMsSamples;
									outputs[ATTACK_OUTPUT].setVoltage(10.f, c);
								} else if (stage[c] == DECAY_STAGE) {
									eoD[c] = true;
									eoDtime[c] = oneMsSamples;
									outputs[DECAY_OUTPUT].setVoltage(10.f, c);
								}
								stage[c] = RELEASE_STAGE;
								stageSample[c] = 0;
								refValue[c] = stageLevel[c];
							}
						}
					}
					prevTrigValue[c] = trigValue[c];
				break;

				case LOOP_MODE:
					if (trigValue[c] >= 1.f && prevTrigValue[c] < 1.f) {
						if (stage[c] == STOP_STAGE) {
							stage[c] = ATTACK_STAGE;
							stageSample[c] = 0;
							refValue[c] = 0;
						} else {
							if (stage[c] == RELEASE_STAGE) {
								eoD[c] = true;
								eoDtime[c] = oneMsSamples;
								outputs[DECAY_OUTPUT].setVoltage(10.f, c);
								stage[c] = ATTACK_STAGE;
							} else {
								if (stage[c] == ATTACK_STAGE) {
									eoA[c] = true;
									eoAtime[c] = oneMsSamples;
									outputs[ATTACK_OUTPUT].setVoltage(10.f, c);
								} else {
									eoD[c] = true;
									eoDtime[c] = oneMsSamples;
									outputs[DECAY_OUTPUT].setVoltage(10.f, c);
								}
								stage[c] = RELEASE_STAGE;
							}
							stageSample[c] = 0;
							refValue[c] = stageLevel[c];
						}
					}
					prevTrigValue[c] = trigValue[c];
				break;

			}

			switch (stage[c]) {
				case ATTACK_STAGE:

					stageSample[c]++;
					attackKnob = params[ATTACK_PARAM].getValue();
					if (attackKnob != prevAttackKnob)
						attackValue = convertCVToMs(attackKnob);
					prevAttackKnob = attackKnob;

					if (!inputs[ATTACK_INPUT].isConnected()) {
						maxStageSample[c] = attackValue * oneMsSamples;
					} else {
						maxStageSample[c] = (attackValue + (1000 * inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue())) * oneMsSamples;
						if (maxStageSample[c] > tenSmS)
							maxStageSample[c] = tenSmS;
						else if (maxStageSample[c] < 0)
							maxStageSample[c] = 0;
					}

					stageLevel[c] = (shapeResponse(stageSample[c] / maxStageSample[c]) * (1 - refValue[c])) + refValue[c];

					if (stageSample[c] >= maxStageSample[c]) {
						
						stage[c] = DECAY_STAGE;
						stageSample[c] = 0;
						stageLevel[c] = 1.f;

						eoA[c] = true;
						eoAtime[c] = oneMsSamples;
						outputs[ATTACK_OUTPUT].setVoltage(10.f, c);
					}

				break;

				case DECAY_STAGE:

					retrigValue[c] = inputs[RETRIG_INPUT].getVoltage(c);
					if (retrigValue[c] >= 1.f && prevRetrigValue[c] < 1) {

						refValue[c] = stageLevel[c];

						stage[c] = ATTACK_STAGE;

						stageSample[c] = 0;
						//stageLevel[c] = 0;

						eoD[c] = true;
						eoDtime[c] = oneMsSamples;
						outputs[DECAY_OUTPUT].setVoltage(10.f, c);

					} else {

						stageSample[c]++;
						decayKnob = params[DECAY_PARAM].getValue();
						if (decayKnob != prevDecayKnob)
							decayValue = convertCVToMs(decayKnob);
						prevDecayKnob = decayKnob;

						if (!inputs[DECAY_INPUT].isConnected()) {
							maxStageSample[c] = decayValue * oneMsSamples;
						} else {
							maxStageSample[c] = (decayValue + (1000 * inputs[DECAY_INPUT].getVoltage() * params[DECAYATNV_PARAM].getValue())) * oneMsSamples;
							if (maxStageSample[c] > tenSmS)
								maxStageSample[c] = tenSmS;
							else if (maxStageSample[c] < 0)
								maxStageSample[c] = 0;
						}

						if (mode != LOOP_MODE) {
							stageLevel[c] = (shapeResponse(1 - (stageSample[c] / maxStageSample[c])) * (1 - sustainValue)) + sustainValue;

							if (stageSample[c] >= maxStageSample[c]) {
								stage[c] = SUSTAIN_STAGE;

								stageSample[c] = 0;
								stageLevel[c] = sustainValue;

								eoD[c] = true;
								eoDtime[c] = oneMsSamples;
								outputs[DECAY_OUTPUT].setVoltage(10.f, c);

							}
						} else {	// loop mode

							stageLevel[c] = (shapeResponse(1 - (stageSample[c] / maxStageSample[c])) );

							if (stageSample[c] >= maxStageSample[c]) {
								stage[c] = ATTACK_STAGE;

								stageSample[c] = 0;
								stageLevel[c] = 0;

								eoD[c] = true;
								eoDtime[c] = oneMsSamples;
								outputs[DECAY_OUTPUT].setVoltage(10.f, c);

							}
						}
					}
					retrigValue[c] = retrigValue[c];

				break;

				case SUSTAIN_STAGE:

					retrigValue[c] = inputs[RETRIG_INPUT].getVoltage(c);
					if (retrigValue[c] >= 1.f && prevRetrigValue[c] < 1) {

						refValue[c] = sustainValue;

						stage[c] = ATTACK_STAGE;

						stageSample[c] = 0;
						//stageLevel[c] = 0;

						eoS[c] = true;
						eoStime[c] = oneMsSamples;
						outputs[SUSTAIN_OUTPUT].setVoltage(10.f, c);


					} else {
						stageLevel[c] = sustainValue;
					}
					retrigValue[c] = retrigValue[c];

				break;

				case RELEASE_STAGE:
					stageSample[c]++;
					releaseKnob = params[RELEASE_PARAM].getValue();
					if (releaseKnob != prevReleaseKnob)
						releaseValue = convertCVToMs(releaseKnob);
					prevReleaseKnob = releaseKnob;

					if (!inputs[RELEASE_INPUT].isConnected()) {
						maxStageSample[c] = releaseValue * oneMsSamples;
					} else {
						maxStageSample[c] = (releaseValue + (1000 * inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue())) * oneMsSamples;
						if (maxStageSample[c] > tenSmS)
							maxStageSample[c] = tenSmS;
						else if (maxStageSample[c] < 0)
							maxStageSample[c] = 0;
					}

					stageLevel[c] = shapeResponse(1 - (stageSample[c] / maxStageSample[c])) * refValue[c];

					if (stageSample[c] >= maxStageSample[c]) {
						
						stageSample[c] = 0;
						stageLevel[c] = 0.f;

						stage[c] = STOP_STAGE;
						eoR[c] = true;
						eoRtime[c] = oneMsSamples;
						outputs[RELEASE_OUTPUT].setVoltage(10.f, c);

					}

				break;
			}

			env[c] = stageLevel[c];

			if (mode == LOOP_MODE)
				env[c] *= sustainValue;

			outputs[ENV_OUTPUT].setVoltage(env[c] * 10, c);
			outputs[INV_OUTPUT].setVoltage((1 - env[c]) * 10, c);
					
			
			// --------- eoA eoD eoS eoR

			if (eoA[c]) {
				eoAtime[c]--;
				if (eoAtime[c] < 0) {
					eoA[c] = false;
					outputs[ATTACK_OUTPUT].setVoltage(0.f, c);
				}
			}

			if (eoD[c]) {
				eoDtime[c]--;
				if (eoDtime[c] < 0) {
					eoD[c] = false;
					outputs[DECAY_OUTPUT].setVoltage(0.f, c);
				}
			}

			if (eoS[c]) {
				eoStime[c]--;
				if (eoStime[c] < 0) {
					eoS[c] = false;
					outputs[SUSTAIN_OUTPUT].setVoltage(0.f, c);
				}
			}

			if (eoR[c]) {
				eoRtime[c]--;
				if (eoRtime[c] < 0) {
					eoR[c] = false;
					outputs[RELEASE_OUTPUT].setVoltage(0.f, c);
				}
			}

		}
		
		outputs[ATTACK_OUTPUT].setChannels(chanTrig);
		outputs[DECAY_OUTPUT].setChannels(chanTrig);
		outputs[SUSTAIN_OUTPUT].setChannels(chanTrig);
		outputs[RELEASE_OUTPUT].setChannels(chanTrig);

		outputs[ENV_OUTPUT].setChannels(chanTrig);
		outputs[INV_OUTPUT].setChannels(chanTrig);

		// --------- VCA

		if (outputs[LEFT_OUTPUT].isConnected() || outputs[RIGHT_OUTPUT].isConnected()) {

			masterLevel = params[VOL_PARAM].getValue();

			if (chanTrig == 1) {

				if (!inputs[VOL_INPUT].isConnected())
					volLevel = masterLevel;
				else
					volLevel = masterLevel * (inputs[VOL_INPUT].getVoltage(0) / 10);

				if (volLevel > 1)
					volLevel = 1;
				else if (volLevel < 0)
					volLevel = 0;

				if (outputs[LEFT_OUTPUT].isConnected()) {
					chanVca = inputs[LEFT_INPUT].getChannels();

					for (int c = 0; c < chanVca; c++) {
						outSignal = inputs[LEFT_INPUT].getVoltage(c) * env[0] * volLevel;
						if (outSignal > 10.f)
							outSignal = 10;
						else if (outSignal < -10.f)
							outSignal = -10.f;
						outputs[LEFT_OUTPUT].setVoltage(outSignal, c);
					}
					outputs[LEFT_OUTPUT].setChannels(chanVca);
				}

				if (outputs[RIGHT_OUTPUT].isConnected()) {
					chanVca = inputs[RIGHT_INPUT].getChannels();
					for (int c = 0; c < chanVca; c++) {
						outSignal = inputs[RIGHT_INPUT].getVoltage(c) * env[0] * volLevel;
						if (outSignal > 10.f)
							outSignal = 10;
						else if (outSignal < -10.f)
							outSignal = -10.f;
						outputs[RIGHT_OUTPUT].setVoltage(outSignal, c);
					}
					outputs[RIGHT_OUTPUT].setChannels(chanVca);
				}
			} else {	// trigger polyPhony

				for (int c = 0; c < chanTrig; c++) {

					volLevel = masterLevel + (inputs[VOL_INPUT].getVoltage(c) / 10);

					if (volLevel > 1)
						volLevel = 1;
					else if (volLevel < 0)
						volLevel = 0;

					outSignal = inputs[LEFT_INPUT].getVoltage(c) * env[c] * volLevel;
					if (outSignal > 10.f)
						outSignal = 10;
					else if (outSignal < -10.f)
						outSignal = -10.f;
					outputs[LEFT_OUTPUT].setVoltage(outSignal, c);

					outSignal = inputs[RIGHT_INPUT].getVoltage(c) * env[c] * volLevel;
					if (outSignal > 10.f)
						outSignal = 10;
					else if (outSignal < -10.f)
						outSignal = -10.f;
					outputs[RIGHT_OUTPUT].setVoltage(outSignal, c);
				}
				outputs[LEFT_OUTPUT].setChannels(chanTrig);
				outputs[RIGHT_OUTPUT].setChannels(chanTrig);
			}

		}
		
	}
};


struct EnverWidget : ModuleWidget {
	EnverWidget(Enver *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Enver.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		const float xGate = 7.5;
		const float xRetrig = 16.5;
		const float yGateBut = 18.5;
		const float yGateIn = 29;
		
		const float xMode = 25.351;
		const float yMode = 16.668;

		const float xSh = 41;
		const float ySh = 17.475;

		const float xShIn = 34;
		const float xShAtn = 43.471;
		const float yShIn = 29;

		const float adsrXstart = 8;
		const float adsrXdelta = 11.8;

		const float adsrYknob = 48.035;
		const float adsrYatnv = 59.8;
		const float adsrYin = 69;

		const float adsrOutX1 = 9.5;
		const float adsrOutX2 = 22;
		const float adsrOutY1 = 81.5;
		const float adsrOutY2 = 91;

		const float xInv = 33.5;

		const float xEnv = 43.5;
		const float yEnv = 86;

		const float xIn = 9;
		const float xVol = 25;
		const float yVol = 108;
		const float xOut = 42;
		
		const float yAudio1 = 107.5;
		const float yAudio2 = 117;

		addParam(createLightParamCentered<VCVLightBezel<YellowLight>>(mm2px(Vec(xGate, yGateBut)), module, Enver::TRIGBUT_PARAM, Enver::TRIGBUT_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xGate, yGateIn)), module, Enver::TRIG_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRetrig, yGateIn)), module, Enver::RETRIG_INPUT));

		//addParam(createParamCentered<Enver::EnverSwitch>(mm2px(Vec(xMode, yMode)), module, Enver::MODE_SWITCH));
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(xMode, yMode)), module, Enver::MODE_SWITCH));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xSh, ySh)), module, Enver::SHAPE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xShAtn, yShIn)), module, Enver::SHAPEATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xShIn, yShIn)), module, Enver::SHAPE_INPUT));
		
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(adsrXstart, adsrYknob)), module, Enver::ATTACK_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(adsrXstart, adsrYatnv)), module, Enver::ATTACKATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(adsrXstart, adsrYin)), module, Enver::ATTACK_INPUT));
		
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(adsrXstart+adsrXdelta, adsrYknob)), module, Enver::DECAY_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(adsrXstart+adsrXdelta, adsrYatnv)), module, Enver::DECAYATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(adsrXstart+adsrXdelta, adsrYin)), module, Enver::DECAY_INPUT));
		
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(adsrXstart+(2*adsrXdelta), adsrYknob)), module, Enver::SUSTAIN_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(adsrXstart+(2*adsrXdelta), adsrYatnv)), module, Enver::SUSTAINATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(adsrXstart+(2*adsrXdelta), adsrYin)), module, Enver::SUSTAIN_INPUT));
		
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(adsrXstart+(3*adsrXdelta), adsrYknob)), module, Enver::RELEASE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(adsrXstart+(3*adsrXdelta), adsrYatnv)), module, Enver::RELEASEATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(adsrXstart+(3*adsrXdelta), adsrYin)), module, Enver::RELEASE_INPUT));
		
		
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(adsrOutX1, adsrOutY1)), module, Enver::ATTACK_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(adsrOutX2, adsrOutY1)), module, Enver::DECAY_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(adsrOutX1, adsrOutY2)), module, Enver::SUSTAIN_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(adsrOutX2, adsrOutY2)), module, Enver::RELEASE_OUTPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xInv, adsrOutY2)), module, Enver::INV_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xEnv, yEnv)), module, Enver::ENV_OUTPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xIn, yAudio1)), module, Enver::LEFT_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xIn, yAudio2)), module, Enver::RIGHT_INPUT));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xVol, yVol)), module, Enver::VOL_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xVol, yAudio2)), module, Enver::VOL_INPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yAudio1)), module, Enver::LEFT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yAudio2)), module, Enver::RIGHT_OUTPUT));

	}

	/*
	void appendContextMenu(Menu *menu) override {
	   	Enver *module = dynamic_cast<Enver*>(this->module);
			assert(module);
		
		menu->addChild(new MenuSeparator());

		menu->addChild(createMenuItem("Load Sample", "", [=]() {
			//module->menuLoadSample();
			bool temploadFromPatch = module->loadFromPatch;
			module->loadFromPatch = false;
			module->menuLoadSample();
			if (module->restoreLoadFromPatch)
				module->loadFromPatch = temploadFromPatch;
		}));

		if (module->folderTreeData.size() > 0) {
			menu->addChild(createSubmenuItem("Samples Browser", "", [=](Menu* menu) {
				//module->folderTreeData.resize(1);
				//module->folderTreeDisplay.resize(1);
				module->refreshRootFolder();
				for (unsigned int i = 1; i < module->folderTreeData[0].size(); i++) {
					if (module->folderTreeData[0][i].substr(module->folderTreeData[0][i].length()-1, module->folderTreeData[0][i].length()-1) == "/")  {
						module->tempDir = module->folderTreeData[0][i];
						menu->addChild(createSubmenuItem(module->folderTreeDisplay[0][i], "", [=](Menu* menu) {
							loadSubfolder(menu, module->folderTreeData[0][i]);
						}));
					} else {
						menu->addChild(createMenuItem(module->folderTreeDisplay[0][i], "", [=]() {module->loadFromPatch = false;module->loadSample(module->folderTreeData[0][i]);}));
					}
				}
			}));
		}

		if (module->storedPath != "") {
			menu->addChild(new MenuSeparator());
			if (module->fileLoaded) {
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription));
				menu->addChild(createMenuLabel(" " + module->samplerateDisplay));
			} else {
				menu->addChild(createMenuLabel("Sample ERROR:"));
				menu->addChild(createMenuLabel(module->fileDescription));
			}
			menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot();}));
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Select Samples Root", "", [=]() {module->selectRootFolder();}));

		if (module->userFolder != "") {
			menu->addChild(createMenuLabel(module->userFolder));
			//menu->addChild(createMenuItem("", "Refresh", [=]() {module->refreshRootFolder();}));
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Anti-aliasing filter", "", &module->antiAlias));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolMenuItem("Polyphonic OUTs", "", [=]() {
				return module->isPolyOuts();
			}, [=](bool poly) {
				module->setPolyOuts(poly);
		}));
		menu->addChild(createBoolPtrMenuItem("Polyphonic Master IN", "", &module->polyMaster));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Disable NAV Buttons", "", &module->disableNav));
		menu->addChild(createBoolPtrMenuItem("Store Sample in Patch", "", &module->sampleInPatch));
	}
	*/
};

Model *modelEnver = createModel<Enver, EnverWidget>("Enver");
