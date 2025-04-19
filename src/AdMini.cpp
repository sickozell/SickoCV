//#define ENV_MODE 0
#define FUNC_MODE 0
#define LOOP_MODE 1

#define STOP_STAGE 0
#define ATTACK_STAGE 1
#define DECAY_STAGE 2
//#define SUSTAIN_STAGE 3
#define RELEASE_STAGE 4	// used only when triggered while looping

#include "plugin.hpp"
#include "EnverMiniExp.hpp"

//using namespace std;

struct AdMini : Module {

	#include "shapes.hpp"

	enum ParamIds {
		SHAPE_PARAM,
		ATTACK_PARAM,
		DECAY_PARAM,
		LVL_PARAM,
		NUM_PARAMS 
	};
	enum InputIds {
		TRIG_INPUT,
		ATTACK_INPUT,
		DECAY_INPUT,
		SIGNAL_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENV_OUTPUT,
		SIGNAL_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(MODE_LIGHT, 3),
		LVL2ENV_LIGHT,
		NUM_LIGHTS
	};
  
	unsigned int sampleRate = APP->engine->getSampleRate();
	float sr = float(sampleRate);
	float srCoeff = 1 / sr;

	float stageCoeff[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	float slopeCorr[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

	int mode = FUNC_MODE;

	float shape;

	float trigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevTrigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	int stage[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float stageLevel[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	int chanVca = 1;
	int chanTrig = 1;

	float env[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float outSignal = 0.f;

	float attackValue;
	float decayValue;

	float attackInValue;
	float decayInValue;

	float refValue[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float deltaValue[16] = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};

	float attackKnob = 0.f;
	float prevAttackKnob = -1.f;
	float decayKnob = 0.f;
	float prevDecayKnob = -1.f;

	float volLevel;

	float lightRValue = 0.f;
	float lightGValue = 0.f;
	//float lightBValue = 0.f;

	bool lvlToEnv = false;

	static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	
	static constexpr float minStageTimeSec = 0.001f;  // in seconds
	static constexpr float maxStageTimeSec = 10.f;  // in seconds
	
	const float maxAdsrTime = 10.f;
	const float minAdsrTime = 0.001f;

	//**************************************************************
	// EXPANDER variables

	const bool connectedToMaster = true;

	EnvMiniExpMsg expInputMessage[2][1];
	EnvMiniExpMsg expOutputMessage[2][1];

	AdMini() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configInput(TRIG_INPUT,"Gate/Trig");
		configParam(SHAPE_PARAM, 0.f, 1.f, 0.5f, "Shape","", 0, 100);
		configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Attack", " ms", maxStageTime / minStageTime, minStageTime);
		configInput(ATTACK_INPUT,"Attack CV");
		configParam(DECAY_PARAM, 0.f, 1.f, 0.f, "Decay", " ms", maxStageTime / minStageTime, minStageTime);
		configInput(DECAY_INPUT,"Decay CV");
		configOutput(ENV_OUTPUT,"Envelope");
		configInput(SIGNAL_INPUT,"Signal");
		configParam(LVL_PARAM, 0.f, 1.f, 1.f, "Level","%", 0, 100);
		configOutput(SIGNAL_OUTPUT,"Signal");

		lights[MODE_LIGHT+2].setBrightness(0.f);
	}

	/*
	void onReset(const ResetEvent &e) override {

		Module::onReset(e);
	}
	*/

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "mode", json_integer(mode));
		json_object_set_new(rootJ, "lvlToEnv", json_boolean(lvlToEnv));

		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* modeJ = json_object_get(rootJ, "mode");
		if (modeJ)
			mode = json_integer_value(modeJ);
		json_t* lvlToEnvJ = json_object_get(rootJ, "lvlToEnv");
		if (lvlToEnvJ)
			lvlToEnv = json_boolean_value(lvlToEnvJ);
	}


	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
		sr = float(sampleRate);
		srCoeff = 1 / sr;
	}

	void addExpander( Model* model, ModuleWidget* parentModWidget, bool left = false ) {
		Module* module = model->createModule();
		APP->engine->addModule(module);
		ModuleWidget* modWidget = model->createModuleWidget(module);
		APP->scene->rack->setModulePosForce( modWidget, Vec( parentModWidget->box.pos.x + (left ? -modWidget->box.size.x : parentModWidget->box.size.x), parentModWidget->box.pos.y));
		APP->scene->rack->addModule(modWidget);
		history::ModuleAdd* h = new history::ModuleAdd;
		h->name = "create "+model->name;
		h->setModule(modWidget);
		APP->history->push(h);
	}

	float shapeResponse(float value) {
		if (shape < 0.5f)
			return (expTable[0][int(expTableCoeff * value)] * (1 - (shape * 2))) + (value * shape * 2.f);
		else
			return (value * (1 - ((shape - 0.5f) * 2.f))) + (expTable[1][int(expTableCoeff * value)] * ((shape - 0.5f) * 2.f));
	}

	float shapeResponse2(float value) {
		if (shape < 0.5f)
			return (expTable[1][int(expTableCoeff * value)] * (1 - (shape * 2))) + (value * shape * 2.f);
		else
			return (value * (1 - ((shape - 0.5f) * 2.f))) + (expTable[0][int(expTableCoeff * value)] * ((shape - 0.5f) * 2.f));
	}

	static float convertCVToSec(float cv) {		
		return minStageTimeSec * std::pow(maxStageTimeSec / minStageTimeSec, cv);
	}

	void process(const ProcessArgs &args) override {

		lights[LVL2ENV_LIGHT].setBrightness(lvlToEnv);

		switch (mode) {

			case FUNC_MODE:
				lightRValue = 0.5f;
				lightGValue = 0.f;
			break;

			case LOOP_MODE:
				lightRValue = 0.5f;
				lightGValue = 0.5f;
			break;
		}

		lights[MODE_LIGHT+0].setBrightness(lightRValue);
		lights[MODE_LIGHT+1].setBrightness(lightGValue);

		shape = params[SHAPE_PARAM].getValue();

		volLevel = params[LVL_PARAM].getValue();

		if (inputs[TRIG_INPUT].isConnected())
			chanTrig = inputs[TRIG_INPUT].getChannels();
		else
			chanTrig = 1;

		for (int c = 0; c < chanTrig; c++) {

			trigValue[c] = inputs[TRIG_INPUT].getVoltage(c);

			switch (mode) {

				case FUNC_MODE:
					if (trigValue[c] >= 1.f && prevTrigValue[c] < 1.f) {
						if (stage[c] == STOP_STAGE) {
							
							stage[c] = ATTACK_STAGE;
							stageLevel[c] = 0;
							refValue[c] = 0;
							deltaValue[c] = 1.f;
							slopeCorr[c] = 1.f;

						} else {
							if (stage[c] == ATTACK_STAGE) {

							} else if (stage[c] == DECAY_STAGE) {

								stage[c] = ATTACK_STAGE;
								stageLevel[c] = 0;
								refValue[c] = env[c];
								deltaValue[c] = 1-env[c];
								slopeCorr[c] = 1-env[c];

							} else if (stage[c] == RELEASE_STAGE) {

								stage[c] = ATTACK_STAGE;
								stageLevel[c] = 0;
								refValue[c] = env[c];
								deltaValue[c] = 1-env[c];
								slopeCorr[c] = 1-env[c];

							}
						}
					
					}
					prevTrigValue[c] = trigValue[c];
				break;

				case LOOP_MODE:
					if (trigValue[c] >= 1.f && prevTrigValue[c] < 1.f) {

						if (stage[c] == STOP_STAGE) {

							stage[c] = ATTACK_STAGE;
							stageLevel[c] = 0;
							refValue[c] = 0;
							deltaValue[c] = 1.f;
							slopeCorr[c] = 1.f;

						} else {
							if (stage[c] == RELEASE_STAGE) {
								stage[c] = ATTACK_STAGE;
								stageLevel[c] = 0;
								refValue[c] = env[c];
								deltaValue[c] = 1-env[c];
								slopeCorr[c] = 1-env[c];
							} else {

								stage[c] = RELEASE_STAGE;
								stageLevel[c] = 1;
								refValue[c] = 0;
								deltaValue[c] = env[c];

							}
						}
					}
					prevTrigValue[c] = trigValue[c];
				break;

			}

			switch (stage[c]) {
				case ATTACK_STAGE:

					attackKnob = params[ATTACK_PARAM].getValue();
					if (attackKnob != prevAttackKnob)
						attackValue = convertCVToSec(attackKnob);
					prevAttackKnob = attackKnob;

					if (!inputs[ATTACK_INPUT].isConnected()) {
						stageCoeff[c] = srCoeff / attackValue;
					} else {
						attackInValue = attackValue + inputs[ATTACK_INPUT].getVoltage();

						if (attackInValue < minAdsrTime)
							attackInValue = minAdsrTime;

						stageCoeff[c] = srCoeff / attackInValue;
					}

					stageLevel[c] += stageCoeff[c] / slopeCorr[c];

					if (stageLevel[c] >= 1.f) {
						
						stage[c] = DECAY_STAGE;
						stageLevel[c] = 1.f;

						refValue[c] = 0.f;
						deltaValue[c] = 1.f;

					}

					env[c] = refValue[c] + (shapeResponse(stageLevel[c]) * deltaValue[c]);

				break;

				case DECAY_STAGE:

					decayKnob = params[DECAY_PARAM].getValue();
					if (decayKnob != prevDecayKnob)
						decayValue = convertCVToSec(decayKnob);
					prevDecayKnob = decayKnob;
	
					if (!inputs[DECAY_INPUT].isConnected()) {
						stageCoeff[c] = srCoeff / decayValue;
					} else {
						decayInValue = decayValue + inputs[DECAY_INPUT].getVoltage();
						if (decayInValue < minAdsrTime)
							decayInValue = minAdsrTime;
						stageCoeff[c] = srCoeff / decayInValue;
					}

					if (mode == FUNC_MODE) {

						stageLevel[c] -= stageCoeff[c];
						if (stageLevel[c] <= 0.f) {
							stage[c] = STOP_STAGE;
							stageLevel[c] = 0;
							refValue[c] = 0;
							deltaValue[c] = 0;
						}

					} else {	// loop mode

						stageLevel[c] -= stageCoeff[c];
						if (stageLevel[c] <= 0.f) {
							stage[c] = ATTACK_STAGE;
							stageLevel[c] = 0;
							refValue[c] = 0;
							deltaValue[c] = 1;
							slopeCorr[c] = 1;
						}
					}

					env[c] = refValue[c] + (shapeResponse2(stageLevel[c]) * deltaValue[c]);

				break;

				case RELEASE_STAGE:

					decayKnob = params[DECAY_PARAM].getValue();
					if (decayKnob != prevDecayKnob)
						decayValue = convertCVToSec(decayKnob);
					prevDecayKnob = decayKnob;
	
					if (!inputs[DECAY_INPUT].isConnected()) {
						stageCoeff[c] = srCoeff / decayValue;
					} else {
						decayInValue = decayValue + inputs[DECAY_INPUT].getVoltage();
						if (decayInValue < minAdsrTime)
							decayInValue = minAdsrTime;
						stageCoeff[c] = srCoeff / decayInValue;
					} 

					stageLevel[c] -= stageCoeff[c];

					if (stageLevel[c] <= 0) {
						
						stage[c] = STOP_STAGE;
						stageLevel[c] = 0;
						refValue[c] = 0;
						deltaValue[c] = 1;

					}

					env[c] = refValue[c] + (shapeResponse2(stageLevel[c]) * deltaValue[c]);

				break;

			}

			if (!lvlToEnv)
				outputs[ENV_OUTPUT].setVoltage(env[c] * 10, c);
			else
				outputs[ENV_OUTPUT].setVoltage(env[c] * 10 * volLevel, c);

		}

		outputs[ENV_OUTPUT].setChannels(chanTrig);

		

		// --------- VCA

		if (outputs[SIGNAL_OUTPUT].isConnected()) {

			if (chanTrig == 1) {

				if (outputs[SIGNAL_OUTPUT].isConnected()) {
					chanVca = inputs[SIGNAL_INPUT].getChannels();

					for (int c = 0; c < chanVca; c++) {
						outSignal = inputs[SIGNAL_INPUT].getVoltage(c) * env[0] * volLevel;
						if (outSignal > 10.f)
							outSignal = 10;
						else if (outSignal < -10.f)
							outSignal = -10.f;
						outputs[SIGNAL_OUTPUT].setVoltage(outSignal, c);
					}
					outputs[SIGNAL_OUTPUT].setChannels(chanVca);
				}

			} else {	// trigger polyPhony

				for (int c = 0; c < chanTrig; c++) {

					outSignal = inputs[SIGNAL_INPUT].getVoltage(c) * env[c] * volLevel;
					if (outSignal > 10.f)
						outSignal = 10;
					else if (outSignal < -10.f)
						outSignal = -10.f;
					outputs[SIGNAL_OUTPUT].setVoltage(outSignal, c);

				}
				outputs[SIGNAL_OUTPUT].setChannels(chanTrig);

			}

		}

		// --------------------------------------------------------------------------------------------------------  E X P A N D E R

		if (rightExpander.module && rightExpander.module->model == modelEnverMiniX) {
			EnvMiniExpMsg *msgToModule = (EnvMiniExpMsg *)(rightExpander.module->leftExpander.producerMessage);

			msgToModule->connectedToMaster = true;

			msgToModule->chanTrig = chanTrig;
			msgToModule->volLevel = volLevel;
			for (int c = 0; c < chanTrig; c++)
				msgToModule->env[c] = env[c];

			rightExpander.module->leftExpander.messageFlipRequested = true;
		}

	}
	
};


struct AdMiniWidget : ModuleWidget {
	AdMiniWidget(AdMini *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/AdMini.svg")));

		const float xCenter = 5.08f;
		const float xLight = xCenter + 3.5f;
		const float yLightShift = 4.f;

		const float yGate = 15.5;
		const float yAtt = 27.2f;		
		const float yAttIn = yAtt + 8.f;
		const float yDec = yAttIn + 8.f;
		const float yDecIn = yDec + 8.f;
		const float yShape = 64.2;
		const float yVol = 78.2;
		const float yIn1 = 90.5;
		const float yOut1 = 104.5;
		const float yEnv = 117.5;

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yGate)), module, AdMini::TRIG_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, yShape)), module, AdMini::SHAPE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, yAtt)), module, AdMini::ATTACK_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yAttIn)), module, AdMini::ATTACK_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, yDec)), module, AdMini::DECAY_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yDecIn)), module, AdMini::DECAY_INPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xCenter, yEnv)), module, AdMini::ENV_OUTPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yIn1)), module, AdMini::SIGNAL_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, yVol)), module, AdMini::LVL_PARAM));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xCenter, yOut1)), module, AdMini::SIGNAL_OUTPUT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(xLight, yGate - yLightShift)), module, AdMini::MODE_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(xLight, yEnv - yLightShift)), module, AdMini::LVL2ENV_LIGHT));
	}

		void appendContextMenu(Menu* menu) override {
		AdMini* module = dynamic_cast<AdMini*>(this->module);

		menu->addChild(new MenuSeparator());

		struct ModeItem : MenuItem {
			AdMini* module;
			int mode;
			void onAction(const event::Action& e) override {
				module->mode = mode;
			}
		};

		menu->addChild(createMenuLabel("Module Mode (led)"));
		std::string modeNames[2] = {"Function (red)", "Loop (yellow)"};
		for (int i = 0; i < 2; i++) {
			ModeItem* modeItem = createMenuItem<ModeItem>(modeNames[i]);
			modeItem->rightText = CHECKMARK(module->mode == i);
			modeItem->module = module;
			modeItem->mode = i;
			menu->addChild(modeItem);
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("LVL knob -> ENV out", "", &module->lvlToEnv));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Add Expander", "", [=]() {module->addExpander(modelEnverMiniX, this);}));

	}

};

Model *modelAdMini = createModel<AdMini, AdMiniWidget>("AdMini");
