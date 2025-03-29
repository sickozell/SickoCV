#define FORWARD 0
#define REVERSE 1
#define CLOCK_MODE 1
#define CV_MODE 0
#define POSITIVE_V 0
#define NEGATIVE_V 1
#define RUN_GATE 0
#define RUN_TRIG 1
#define CV_TYPE 0

#define COLOR_LCD_RED 0xdd, 0x33, 0x33, 0xff
#define COLOR_LCD_GREEN 0x33, 0xdd, 0x33, 0xff

#include "plugin.hpp"
#include "osdialog.h"
#if defined(METAMODULE)
#include "async_filebrowser.hh"
#endif
#include <dirent.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

struct StepSeqPlus : Module {
	enum ParamId {
		ENUMS(STEP_PARAM, 16),
		LENGTH_PARAM,
		MODE_SWITCH,
		RST_PARAM,
		RUNBUT_PARAM,
		PROG_PARAM,
		RECALL_PARAM,
		STORE_PARAM,
		SET_PARAM,
		AUTO_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CLK_INPUT,
		REV_INPUT,
		RUN_INPUT,
		RST_INPUT,
		LENGTH_INPUT,
		PROG_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(STEP_LIGHT, 16),
		RUNBUT_LIGHT,
		RECALL_LIGHT,
		STORE_LIGHT,
		SET_LIGHT,
		AUTO_LIGHT,
		LIGHTS_LEN
	};

	float clkValue = 0;
	float prevClkValue = 0;

	float rstValue = 0;
	float prevRstValue = 0;

	bool direction = FORWARD;

	float out = 0;

	int step = 0;

	bool runSetting = true;
	bool prevRunSetting = true;

	float runButton = 0;
	float runTrig = 0.f;
	float prevRunTrig = 0.f;

	int range = 9;

	bool initStart = false;
	int recStep = 0;

	int revType = POSITIVE_V;
	int runType = RUN_GATE;

	int maxSteps = 16;
	int mode = 0;
	int prevMode = 1;

	int currAddr = 0;
	int prevAddr = 0;

	bool rstOnRun = true;
	bool dontAdvance = false;
	bool dontAdvanceSetting = true;

	float progSeq[32][16] = {
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},

								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},

								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f},
								{0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f}
							};
	int progSteps[32] = {16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16};
	int progRst[32] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

	// --------------workingSeq

	float wSeq[16] = {0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f};
	int wSteps = 16;
	int wRst = 1;

	float nextSeq[16] = {0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f};
	int nextSteps = 16;
	int nextRst = 1;

	// --------------prog
	int progKnob = 0;
	int prevProgKnob = 0;
	int savedProgKnob = 0;

	float progTrig = 0;
	float prevProgTrig = 0;

	int selectedProg = 0;
	bool progChanged = false;

	float recallBut = 0;
	float prevRecallBut = 0;

	// --------------store
	float storeBut = 0;
	float prevStoreBut = 0;

	bool storeWait = false;
	float storeTime = 0;
	float storeSamples = APP->engine->getSampleRate() / 1.5f;

	bool storedProgram = false;
	int storedProgramTime = 0;
	float maxStoredProgramTime = APP->engine->getSampleRate() * 1.5;

	// -------------- working

	int workingProg = 0;

	bool instantProgChange = false;

	bool progToSet = false;
	float progSetBut = 0;
	float prevProgSetBut = 0;

	// ------- set button light

	bool setButLight = false;
	float setButLightDelta = 2 / APP->engine->getSampleRate();
	float setButLightValue = 0.f;

	int progInType = CV_TYPE;
	int lastProg = 0;

	StepSeqPlus() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(MODE_SWITCH, 0.f, 1.f, 1.f, "Mode", {"Cv", "Clock"});
		configInput(CLK_INPUT, "Clock");
		configInput(REV_INPUT, "Reverse");

		configSwitch(RUNBUT_PARAM, 0.f, 1.f, 1.f, "Run", {"OFF", "ON"});
		configInput(RUN_INPUT, "Run");
		
		configParam(RST_PARAM, 1.f,16.f, 1.f, "Reset Input");
		paramQuantities[RST_PARAM]->snapEnabled = true;
		configInput(RST_INPUT, "Reset");

		configOutput(OUT_OUTPUT, "Out");

		configParam(LENGTH_PARAM, 1.f,16.f, 16.f, "Length");
		paramQuantities[LENGTH_PARAM]->snapEnabled = true;

		configInput(LENGTH_INPUT, "Length");

		struct RangeQuantity : ParamQuantity {
			float getDisplayValue() override {
				StepSeqPlus* module = reinterpret_cast<StepSeqPlus*>(this->module);

				switch (module->range) {
					case 0:	// 0/1v
						displayMultiplier = 1.f;
						displayOffset = 0.f;
					break;
					case 1:	// 0/2v
						displayMultiplier = 2.f;
						displayOffset = 0.f;
					break;
					case 2:	// 0/3v
						displayMultiplier = 3.f;
						displayOffset = 0.f;
					break;
					case 3:	// 0/5v
						displayMultiplier = 5.f;
						displayOffset = 0.f;
					break;
					case 4:	// 0/10v
						displayMultiplier = 10.f;
						displayOffset = 0.f;
					break;
					case 5:	// -1/+1v
						displayMultiplier = 2.f;
						displayOffset = -1.f;
					break;
					case 6:	// -2/+2v
						displayMultiplier = 4.f;
						displayOffset = -2.f;
					break;
					case 7:	// -3/+3v
						displayMultiplier = 6.f;
						displayOffset = -3.f;
					break;
					case 8:	// -5/+5v
						displayMultiplier = 10.f;
						displayOffset = -5.f;
					break;
					case 9:	// -10/+10v
						displayMultiplier = 20.f;
						displayOffset = -10.f;
					break;
				}
				return ParamQuantity::getDisplayValue();
			}
		};
		configParam<RangeQuantity>(STEP_PARAM+0, 0, 1.f, 0.5f, "Step 1");
		configParam<RangeQuantity>(STEP_PARAM+1, 0, 1.f, 0.5f, "Step 2");
		configParam<RangeQuantity>(STEP_PARAM+2, 0, 1.f, 0.5f, "Step 3");
		configParam<RangeQuantity>(STEP_PARAM+3, 0, 1.f, 0.5f, "Step 4");
		configParam<RangeQuantity>(STEP_PARAM+4, 0, 1.f, 0.5f, "Step 5");
		configParam<RangeQuantity>(STEP_PARAM+5, 0, 1.f, 0.5f, "Step 6");
		configParam<RangeQuantity>(STEP_PARAM+6, 0, 1.f, 0.5f, "Step 7");
		configParam<RangeQuantity>(STEP_PARAM+7, 0, 1.f, 0.5f, "Step 8");
		configParam<RangeQuantity>(STEP_PARAM+8, 0, 1.f, 0.5f, "Step 9");
		configParam<RangeQuantity>(STEP_PARAM+9, 0, 1.f, 0.5f, "Step 10");
		configParam<RangeQuantity>(STEP_PARAM+10, 0, 1.f, 0.5f, "Step 11");
		configParam<RangeQuantity>(STEP_PARAM+11, 0, 1.f, 0.5f, "Step 12");
		configParam<RangeQuantity>(STEP_PARAM+12, 0, 1.f, 0.5f, "Step 13");
		configParam<RangeQuantity>(STEP_PARAM+13, 0, 1.f, 0.5f, "Step 14");
		configParam<RangeQuantity>(STEP_PARAM+14, 0, 1.f, 0.5f, "Step 15");
		configParam<RangeQuantity>(STEP_PARAM+15, 0, 1.f, 0.5f, "Step 16");

		configParam(PROG_PARAM, 0.f, 31.f, 0.f, "Prog");
		configInput(PROG_INPUT, "Prog");
		paramQuantities[PROG_PARAM]->snapEnabled = true;
		configSwitch(SET_PARAM, 0, 1.f, 0.f, "Set", {"OFF", "ON"});
		configSwitch(AUTO_PARAM, 0, 1.f, 0.f, "Auto", {"OFF", "ON"});
		configSwitch(RECALL_PARAM, 0, 1.f, 0.f, "Recall", {"OFF", "ON"});
		configSwitch(STORE_PARAM, 0, 1.f, 0.f, "Store", {"OFF", "ON"});
		
	}

	void onReset(const ResetEvent &e) override {

		initStart = false;
		
		step = 0;

		lights[STEP_LIGHT].setBrightness(1);
		lights[STEP_LIGHT+1].setBrightness(0);
		lights[STEP_LIGHT+2].setBrightness(0);
		lights[STEP_LIGHT+3].setBrightness(0);
		lights[STEP_LIGHT+4].setBrightness(0);
		lights[STEP_LIGHT+5].setBrightness(0);
		lights[STEP_LIGHT+6].setBrightness(0);
		lights[STEP_LIGHT+7].setBrightness(0);
		lights[STEP_LIGHT+8].setBrightness(0);
		lights[STEP_LIGHT+9].setBrightness(0);
		lights[STEP_LIGHT+10].setBrightness(0);
		lights[STEP_LIGHT+11].setBrightness(0);
		lights[STEP_LIGHT+12].setBrightness(0);
		lights[STEP_LIGHT+13].setBrightness(0);
		lights[STEP_LIGHT+14].setBrightness(0);
		lights[STEP_LIGHT+15].setBrightness(0);

		setButLight = false;
		setButLightValue = 0.f;

		for (int i = 0; i < 16; i++) {
			wSeq[i] = 0.5;
			params[STEP_PARAM+i].setValue(wSeq[i]);
		}
		wSteps = 16;
		params[LENGTH_PARAM].setValue(wSteps);
		wRst = 1;
		params[RST_PARAM].setValue(wRst);

		Module::onReset(e);
	}

	void onSampleRateChange() override {

		storeSamples = APP->engine->getSampleRate() / 1.5f;
		maxStoredProgramTime = APP->engine->getSampleRate() * 1.5;
		setButLightDelta = 2 / APP->engine->getSampleRate();

	}

	json_t* dataToJson() override {
		if (initStart)
			recStep = 0;
		else
			recStep = step;

		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "range", json_integer(range));
		json_object_set_new(rootJ, "runType", json_integer(runType));
		json_object_set_new(rootJ, "revType", json_integer(revType));
		json_object_set_new(rootJ, "rstOnRun", json_boolean(rstOnRun));
		json_object_set_new(rootJ, "dontAdvanceSetting", json_boolean(dontAdvanceSetting));
		json_object_set_new(rootJ, "step", json_integer(recStep));
		json_object_set_new(rootJ, "initStart", json_boolean(initStart));

		json_object_set_new(rootJ, "savedProgKnob", json_integer(savedProgKnob));

		json_object_set_new(rootJ, "progInType", json_boolean(progInType));
		json_object_set_new(rootJ, "lastProg", json_integer(lastProg));

		json_t *wSeq_json_array = json_array();
		for (int st = 0; st < 16; st++) {
			json_array_append_new(wSeq_json_array, json_real(wSeq[st]));
		}
		json_object_set_new(rootJ, "wSeq", wSeq_json_array);

		for (int prog = 0; prog < 32; prog++) {
			json_t *prog_json_array = json_array();
			for (int tempStep = 0; tempStep < 16; tempStep++) {
				json_array_append_new(prog_json_array, json_real(progSeq[prog][tempStep]));
			}
			json_object_set_new(rootJ, ("prog"+to_string(prog)).c_str(), prog_json_array);	
		}

		for (int prog = 0; prog < 32; prog++) {
			json_t *progSteps_json_array = json_array();
			json_array_append_new(progSteps_json_array, json_integer(progSteps[prog]));
			json_object_set_new(rootJ, ("progSteps"+to_string(prog)).c_str(), progSteps_json_array);
		}

		for (int prog = 0; prog < 32; prog++) {
			json_t *progRst_json_array = json_array();
			json_array_append_new(progRst_json_array, json_integer(progRst[prog]));
			json_object_set_new(rootJ, ("progRst"+to_string(prog)).c_str(), progRst_json_array);
		}

		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {

		json_t* rangeJ = json_object_get(rootJ, "range");
		if (rangeJ) {
			range = json_integer_value(rangeJ);
			if (range < 0 || range > 9)
				range = 9;
		}
		
		json_t* runTypeJ = json_object_get(rootJ, "runType");
		if (runTypeJ) {
			runType = json_integer_value(runTypeJ);
			if (runType < 0 || runType > 1)
				runType = 0;
		}

		json_t* revTypeJ = json_object_get(rootJ, "revType");
		if (revTypeJ) {
			revType = json_integer_value(revTypeJ);
			if (revType < 0 || revType > 1)
				revType = 0;
		}

		json_t* rstOnRunJ = json_object_get(rootJ, "rstOnRun");
		if (rstOnRunJ) {
			rstOnRun = json_boolean_value(rstOnRunJ);
		}

		json_t* dontAdvanceSettingJ = json_object_get(rootJ, "dontAdvanceSetting");
		if (dontAdvanceSettingJ) {
			dontAdvanceSetting = json_boolean_value(dontAdvanceSettingJ);
		}

		json_t* stepJ = json_object_get(rootJ, "step");
		if (stepJ) {
			step = json_integer_value(stepJ);
			if (step < 0 || step > 15)
				step = 0;
			lights[STEP_LIGHT+step].setBrightness(1);

		} 
		
		json_t* initStartJ = json_object_get(rootJ, "initStart");
		if (initStartJ) {
			initStart = json_boolean_value(initStartJ);
			if (initStart)
				step = 0;
		}

		// ----------------

		json_t *wSeq_json_array = json_object_get(rootJ, "wSeq");
		size_t wSeq_st;
		json_t *wSeq_json_value;
		if (wSeq_json_array) {
			json_array_foreach(wSeq_json_array, wSeq_st, wSeq_json_value) {
				params[STEP_PARAM+wSeq_st].setValue(json_real_value(wSeq_json_value));
			}
		}

		json_t* savedProgKnobJ = json_object_get(rootJ, "savedProgKnob");
		if (savedProgKnobJ) {
			savedProgKnob = json_integer_value(savedProgKnobJ);
			if (savedProgKnob < 0 || savedProgKnob > 31)
				savedProgKnob = 0;
			
		}

		json_t* progInTypeJ = json_object_get(rootJ, "progInType");
		if (progInTypeJ) {
			progInType = json_boolean_value(progInTypeJ);
		}

		json_t* lastProgJ = json_object_get(rootJ, "lastProg");
		if (lastProgJ) {
			lastProg = json_integer_value(lastProgJ);
			if (lastProg < 0 || lastProg > 31)
				lastProg = 0;
		}

		selectedProg = savedProgKnob;
		workingProg = selectedProg;
		prevProgKnob = selectedProg;
		params[PROG_PARAM].setValue(selectedProg);

		for (int prog = 0; prog < 32; prog++) {
			json_t *prog_json_array = json_object_get(rootJ, ("prog"+to_string(prog)).c_str());
			size_t tempSeq;
			json_t *json_value;
			if (prog_json_array) {
				json_array_foreach(prog_json_array, tempSeq, json_value) {
					progSeq[prog][tempSeq] = json_real_value(json_value);
				}
			}
		}

		for (int prog = 0; prog < 32; prog++) {
			json_t *progSteps_json_array = json_object_get(rootJ, ("progSteps"+to_string(prog)).c_str());
			size_t tempSeq;
			json_t *json_value;
			if (progSteps_json_array) {
				json_array_foreach(progSteps_json_array, tempSeq, json_value) {
					progSteps[prog] = json_integer_value(json_value);
				}
			}
		}

		for (int prog = 0; prog < 32; prog++) {
			json_t *progRst_json_array = json_object_get(rootJ, ("progRst"+to_string(prog)).c_str());
			size_t tempSeq;
			json_t *json_value;
			if (progRst_json_array) {
				json_array_foreach(progRst_json_array, tempSeq, json_value) {
					progRst[prog] = json_integer_value(json_value);
				}
			}
		}

	}

	json_t *presetToJson() {

		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "range", json_integer(range));
		json_object_set_new(rootJ, "runType", json_integer(runType));
		json_object_set_new(rootJ, "revType", json_integer(revType));
		json_object_set_new(rootJ, "rstOnRun", json_boolean(rstOnRun));
		json_object_set_new(rootJ, "dontAdvanceSetting", json_boolean(dontAdvanceSetting));

		json_object_set_new(rootJ, "progInType", json_boolean(progInType));
		json_object_set_new(rootJ, "lastProg", json_integer(lastProg));

		for (int prog = 0; prog < 32; prog++) {
			json_t *prog_json_array = json_array();
			for (int tempSeq = 0; tempSeq < 16; tempSeq++) {
				json_array_append_new(prog_json_array, json_real(progSeq[prog][tempSeq]));
			}
			json_object_set_new(rootJ, ("prog"+to_string(prog)).c_str(), prog_json_array);	
		}

		for (int prog = 0; prog < 32; prog++) {
			json_t *progSteps_json_array = json_array();
			json_array_append_new(progSteps_json_array, json_integer(progSteps[prog]));
			json_object_set_new(rootJ, ("progSteps"+to_string(prog)).c_str(), progSteps_json_array);
		}

		for (int prog = 0; prog < 32; prog++) {
			json_t *progRst_json_array = json_array();
			json_array_append_new(progRst_json_array, json_integer(progRst[prog]));
			json_object_set_new(rootJ, ("progRst"+to_string(prog)).c_str(), progRst_json_array);
		}

		return rootJ;
	}

	void presetFromJson(json_t *rootJ) {

		json_t* rangeJ = json_object_get(rootJ, "range");
		if (rangeJ) {
			range = json_integer_value(rangeJ);
			if (range < 0 || range > 9)
				range = 9;
		}
		
		json_t* runTypeJ = json_object_get(rootJ, "runType");
		if (runTypeJ) {
			runType = json_integer_value(runTypeJ);
			if (runType < 0 || runType > 1)
				runType = 0;
		}

		json_t* revTypeJ = json_object_get(rootJ, "revType");
		if (revTypeJ) {
			revType = json_integer_value(revTypeJ);
			if (revType < 0 || revType > 1)
				revType = 0;
		}

		json_t* rstOnRunJ = json_object_get(rootJ, "rstOnRun");
		if (rstOnRunJ) {
			rstOnRun = json_boolean_value(rstOnRunJ);
		}

		json_t* dontAdvanceSettingJ = json_object_get(rootJ, "dontAdvanceSetting");
		if (dontAdvanceSettingJ) {
			dontAdvanceSetting = json_boolean_value(dontAdvanceSettingJ);
		}

		json_t* progInTypeJ = json_object_get(rootJ, "progInType");
		if (progInTypeJ) {
			progInType = json_boolean_value(progInTypeJ);
		}

		json_t* lastProgJ = json_object_get(rootJ, "lastProg");
		if (lastProgJ) {
			lastProg = json_integer_value(lastProgJ);
			if (lastProg < 0 || lastProg > 31)
				lastProg = 0;
		}

		for (int prog = 0; prog < 32; prog++) {
			json_t *prog_json_array = json_object_get(rootJ, ("prog"+to_string(prog)).c_str());
			size_t tempSeq;
			json_t *json_value;
			if (prog_json_array) {
				json_array_foreach(prog_json_array, tempSeq, json_value) {
					progSeq[prog][tempSeq] = json_real_value(json_value);
				}
			}
		}

		for (int prog = 0; prog < 32; prog++) {
			json_t *progSteps_json_array = json_object_get(rootJ, ("progSteps"+to_string(prog)).c_str());
			size_t tempSeq;
			json_t *json_value;
			if (progSteps_json_array) {
				json_array_foreach(progSteps_json_array, tempSeq, json_value) {
					progSteps[prog] = json_integer_value(json_value);
				}
			}
		}

		for (int prog = 0; prog < 32; prog++) {
			json_t *progRst_json_array = json_object_get(rootJ, ("progRst"+to_string(prog)).c_str());
			size_t tempSeq;
			json_t *json_value;
			if (progRst_json_array) {
				json_array_foreach(progRst_json_array, tempSeq, json_value) {
					progRst[prog] = json_integer_value(json_value);
				}
			}
		}
	}

	void menuLoadPreset() {
		static const char FILE_FILTERS[] = "stepSeq preset (.ssp):ssp,SSP";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		if (path)
			loadPreset(path);

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadPreset(std::string path) {

		FILE *file = fopen(path.c_str(), "r");
		json_error_t error;
		json_t *rootJ = json_loadf(file, 0, &error);
		if (rootJ == NULL) {
			WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		}

		fclose(file);

		if (rootJ) {

			presetFromJson(rootJ);

		} else {
			WARN("problem loading preset json file");
			//return;
		}
		
	}

	void menuSavePreset() {

		static const char FILE_FILTERS[] = "stepSeq preset (.ssp):ssp,SSP";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".ssp" and strPath.substr(strPath.size() - 4) != ".SSP")
				strPath += ".ssp";
			path = strcpy(new char[strPath.length() + 1], strPath.c_str());
			savePreset(path, presetToJson());
		}

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void savePreset(std::string path, json_t *rootJ) {

		if (rootJ) {
			FILE *file = fopen(path.c_str(), "w");
			if (!file) {
				WARN("[ SickoCV ] cannot open '%s' to write\n", path.c_str());
				//return;
			} else {
				json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
				json_decref(rootJ);
				fclose(file);
			}
		}
	}

	void copyClipboard() {
		for (int i = 0; i < 16; i++)
			stepSeq_cbSeq[i] = wSeq[i];
		
		stepSeq_cbSteps = wSteps;
		stepSeq_cbRst = wRst;
		stepSeq_clipboard = true;
	}

	void pasteClipboard() {
		for (int i = 0; i < 16; i++) {
			wSeq[i] = stepSeq_cbSeq[i];
			params[STEP_PARAM+i].setValue(wSeq[i]);
		}
		
		wSteps = stepSeq_cbSteps;
		params[LENGTH_PARAM].setValue(wSteps);
		wRst = stepSeq_cbRst;
		params[RST_PARAM].setValue(wRst);
	}

	void eraseProgs() {
		for (int p = 0; p < 32; p++) {
			progSteps[p] = 16;
			progRst[p] = 1;

			for (int s = 0; s < 16; s++)
				progSeq[p][s] = 0.5f;
		}
		lastProg = 0;
	}

	void inline resetStep() {
		lights[STEP_LIGHT+step].setBrightness(0);
		step = wRst - 1;

		if (mode == CLOCK_MODE && dontAdvanceSetting)
			dontAdvance = true;

		if (progInType != CV_TYPE)
				progKnob = 0;
	}

	void scanLastProg() {
		lastProg = 31;
		bool exitFunc = false;

		for (int p = 31; p >= 0; p--) {
			for (int st = 0; st < 16; st++) {
				if (progSeq[p][st] != 0.5f) {
					st = 16;
					exitFunc = true;
				}
			}
			if (progSteps[p] != 16 || progRst[p] != 1)
				exitFunc = true;
				
			lastProg = p;
			
			if (exitFunc)
				p = 0;

		}

	}
	
	void randomizeTrack() {
		for (int st = 0; st < 16; st++) {
			wSeq[st] = random::uniform();
			params[STEP_PARAM+st].setValue(wSeq[st]);
		}
	}

	void process(const ProcessArgs& args) override {

		// ----------- AUTO SWITCH

		instantProgChange = int(params[AUTO_PARAM].getValue());
		lights[AUTO_LIGHT].setBrightness(instantProgChange);

		// ----------- PROGRAM MANAGEMENT

		if (progInType == CV_TYPE) {

			progKnob = int(params[PROG_PARAM].getValue() + (inputs[PROG_INPUT].getVoltage() * 3.2));
			if (progKnob < 0)
				progKnob = 0;
			else if (progKnob > 31)
				progKnob = 31;

		} else {

			progKnob = params[PROG_PARAM].getValue();
			if (progKnob < 0)
				progKnob = 0;
			else if (progKnob > 31)
				progKnob = 31;

			progTrig = inputs[PROG_INPUT].getVoltage();
			if (progTrig >= 1.f && prevProgTrig < 1.f) {
				progKnob++;
				if (progKnob > lastProg)
					progKnob = 0;

				params[PROG_PARAM].setValue(progKnob);
			}
			prevProgTrig = progTrig;

		}

		if (progKnob != prevProgKnob) {

			progChanged = true;
			selectedProg = progKnob;
			prevProgKnob = progKnob;

			for (int i = 0; i < 16; i++) {
				nextSeq[i] = progSeq[selectedProg][i];
				params[STEP_PARAM+i].setValue(nextSeq[i]);
			}
			nextSteps = progSteps[selectedProg];
			nextRst = progRst[selectedProg];

			params[LENGTH_PARAM].setValue(nextSteps);
			params[RST_PARAM].setValue(nextRst);

			setButLight = true;
			setButLightValue = 0.f;
		}

		// -------- CURRENT SEQ UPDATE

		progToSet = false;

		progSetBut = params[SET_PARAM].getValue();
		if (progSetBut >= 1.f && prevProgSetBut < 1.f)
			progToSet = true;

		prevProgSetBut = progSetBut;

		if (!progChanged) {

			for (int i = 0; i < 16; i++)
				wSeq[i] = params[STEP_PARAM+i].getValue();

			wSteps = params[LENGTH_PARAM].getValue();
			wRst = params[RST_PARAM].getValue();

		} else {
			if (instantProgChange || progToSet) {

				for (int i = 0; i < 16; i++)
					wSeq[i] = nextSeq[i];

				wSteps = nextSteps;
				wRst = nextRst;

				params[LENGTH_PARAM].setValue(wSteps);
				params[RST_PARAM].setValue(wRst);

				workingProg = selectedProg;
				if (progInType == CV_TYPE)
					savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
				else
					savedProgKnob = progKnob;

				progChanged = false;
				progToSet = false;
				setButLight = false;
				setButLightValue = 0.f;

			} else {	// IF SET IS PENDING -> GET NEW SETTINGS
					
				for (int i = 0; i < 16; i++)
					nextSeq[i] = params[STEP_PARAM+i].getValue();

				nextSteps = params[LENGTH_PARAM].getValue();
				nextRst = params[RST_PARAM].getValue();

			}
		
		}

		// -------------------------- RECALL PROG

		recallBut = params[RECALL_PARAM].getValue();
		lights[RECALL_LIGHT].setBrightness(recallBut);

		if (recallBut >= 1.f && prevRecallBut < 1.f) {

			if (!progChanged) {

				for (int i = 0; i < 16; i++) {
					wSeq[i] = progSeq[selectedProg][i];
					params[STEP_PARAM+i].setValue(wSeq[i]);
				}
				wSteps = progSteps[selectedProg];
				wRst = progRst[selectedProg];

				params[LENGTH_PARAM].setValue(wSteps);
				params[RST_PARAM].setValue(wRst);

				workingProg = selectedProg;
				if (progInType == CV_TYPE)
					savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
				else
					savedProgKnob = progKnob;

			} else {

				for (int i = 0; i < 16; i++)
					params[STEP_PARAM+i].setValue(wSeq[i]);

				params[LENGTH_PARAM].setValue(wSteps);
				params[RST_PARAM].setValue(wRst);

				params[PROG_PARAM].setValue(workingProg);
				
				//savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
				savedProgKnob = workingProg;
				selectedProg = workingProg;
				prevProgKnob = workingProg;
			}

			progChanged = false;
			setButLight = false;
			setButLightValue = 0.f;

		}
		prevRecallBut = recallBut;

		// -----------------------------------
		// ------------ STORE MANAGEMENT
		// -----------------------------------

		storeBut = params[STORE_PARAM].getValue();
		lights[STORE_LIGHT].setBrightness(storeBut);

		if (storeBut >= 1 && prevStoreBut < 1) {
			if (!storeWait) {
				storeWait = true;
				storeTime = storeSamples;
			} else {
				storeWait = false;
				for (int i = 0; i < 16; i++)
					progSeq[progKnob][i] = wSeq[i];

				progSteps[progKnob] = wSteps;
				progRst[progKnob] = wRst;

				if (progKnob > lastProg)
					lastProg = progKnob;

				storedProgram = true;
				storedProgramTime = maxStoredProgramTime;
			}
		}
		
		if (storeWait) {
			storeTime--;
			if (storeTime < 0)
				storeWait = false;
		}
		prevStoreBut = storeBut;

		if (storedProgram) {
			storedProgramTime--;
			if (storedProgramTime < 0) {
				lights[STORE_LIGHT].setBrightness(0);
				storedProgram = false;
			} else {
				lights[STORE_LIGHT].setBrightness(1);
			}

		}

		if (setButLight) {
			if (setButLightValue > 1 || setButLightValue < 0) {
				setButLightDelta *= -1;
			}
			setButLightValue += setButLightDelta;
		}

		lights[SET_LIGHT].setBrightness(setButLightValue);

		// -------------------------------------------------
		// -------------------------------------------------
		// -------------------------------------------------
		// -------------------------------------------------
		// -------------------------------------------------

		out = 0.f;

		mode = params[MODE_SWITCH].getValue();

		rstValue = inputs[RST_INPUT].getVoltage();
		if (mode == CLOCK_MODE && rstValue >= 1.f && prevRstValue < 1.f)
			resetStep();

		prevRstValue = rstValue;

		if (inputs[RUN_INPUT].isConnected()) {

			runTrig = inputs[RUN_INPUT].getVoltage();

			if (runType == RUN_GATE) {
				
				if (runTrig > 1) {
					runSetting = 1;
					if (!prevRunSetting && mode == CLOCK_MODE && rstOnRun)
						resetStep();
				} else {
					runSetting = 0;
				}

			} else {	// runType == RUN_TRIG

				if (runTrig > 1 && prevRunTrig <=1) {
					if (runSetting) {
						runSetting = 0;
						params[RUNBUT_PARAM].setValue(0);
					} else {
						runSetting = 1;
						params[RUNBUT_PARAM].setValue(1);
						if (!prevRunSetting && mode == CLOCK_MODE && rstOnRun)
							resetStep();
					}
				}				
			}
			prevRunSetting = runSetting;
			prevRunTrig = runTrig;
		
		} else {
			
			runSetting = params[RUNBUT_PARAM].getValue();
			if (mode == CLOCK_MODE && rstOnRun && runSetting && !prevRunSetting)
				resetStep();
		}

		prevRunSetting = runSetting;

		lights[RUNBUT_LIGHT].setBrightness(runSetting);


		if (runSetting) {

			maxSteps = wSteps;

			if (inputs[LENGTH_INPUT].isConnected()) {
				float stepsIn = inputs[LENGTH_INPUT].getVoltage();
				if (stepsIn < 0.f)
					stepsIn = 0.f;
				else if (stepsIn > 10.f)
					stepsIn = 10.f;

				int addSteps = int(stepsIn / 10 * (16 - maxSteps));

				maxSteps += addSteps;
				if (maxSteps > 16)
					maxSteps = 16;
			}
			
			if (mode == CV_MODE && prevMode == CLOCK_MODE) {
				prevClkValue = 11.f;
				prevAddr = 11.f;
			}
			prevMode = mode;

			clkValue = inputs[CLK_INPUT].getVoltage();

			switch (mode) {
				case CLOCK_MODE:

					if (clkValue >= 1.f && prevClkValue < 1.f) {

						if (revType == POSITIVE_V) {
							if (inputs[REV_INPUT].getVoltage() < 1)
								direction = FORWARD;
							else
								direction = REVERSE;
						} else {
							if (inputs[REV_INPUT].getVoltage() < -1)
								direction = REVERSE;
							else
								direction = FORWARD;
						}

						lights[STEP_LIGHT + step].setBrightness(0);

						if (direction == FORWARD) {

							if (!dontAdvance)
								step++;
							else
								dontAdvance = false;

							if (step >= maxSteps)
								step = 0;
						} else {

							if (!dontAdvance)
								step--;
							else
								dontAdvance = false;

							if (step < 0)
								step = maxSteps - 1;
						}

					}
					prevClkValue = clkValue;

				break;

				case CV_MODE:
					if (clkValue > 10.f)
						clkValue = 10.f;
					else if (clkValue < 0.f)
						clkValue = 0.f;
	
					if (clkValue != prevClkValue) {
						
						currAddr = 1+int(clkValue / 10 * (maxSteps));
						if (currAddr >= maxSteps)
							currAddr = maxSteps;
						if (currAddr != prevAddr) {
							lights[STEP_LIGHT+step].setBrightness(0);
							step = currAddr-1;
							prevAddr = currAddr;

						}
					}
					prevClkValue = clkValue;
					
				break;
			}
		}
			
		//out = params[STEP_PARAM+step].getValue();
		out = wSeq[step];

		switch (range) {
			case 0:
			break;

			case 1:
				out *= 2;
			break;

			case 2:
				out *= 3;
			break;

			case 3:
				out *= 5;
			break;

			case 4:
				out *= 10;
			break;

			case 5:
				out = (out * 2) - 1;
			break;

			case 6:
				out = (out * 4) - 2;
			break;

			case 7:
				out = (out * 6) - 3;
			break;

			case 8:
				out = (out * 10) - 5;
			break;

			case 9:
				out = (out * 20) - 10;
			break;
		}

		outputs[OUT_OUTPUT].setVoltage(out);

		lights[STEP_LIGHT+step].setBrightness(1);

	}
};

struct StepSeqPlusDisplay : TransparentWidget {
	StepSeqPlus *module;
	int frame = 0;
	StepSeqPlusDisplay() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/vgatrue.ttf"));
				
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				std::string currentDisplay;

				currentDisplay = to_string(module->workingProg);

				if (!module->progChanged) {
					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_GREEN));
					nvgFontSize(args.vg, 32);
					if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 8, 30, 80, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 16, 30, 80, currentDisplay.c_str(), NULL);

				} else {
					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_GREEN));
					nvgFontSize(args.vg, 26);
					if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 6, 21, 80, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 12, 21, 80, currentDisplay.c_str(), NULL);
					
					currentDisplay = to_string(module->selectedProg);

					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED));
					nvgFontSize(args.vg, 20);
					if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 20, 36, 60, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 25, 36, 60, currentDisplay.c_str(), NULL);
				}
				
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct StepSeqPlusWidget : ModuleWidget {
	StepSeqPlusWidget(StepSeqPlus* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/StepSeqPlus.svg")));

		{
			StepSeqPlusDisplay *display = new StepSeqPlusDisplay();
			display->box.pos = mm2px(Vec(35.6, 7.8));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float xLeft = 7;

		const float yCvSw = 19;
		const float yTrig = 33;

		const float yRunBut = 48.5;
		const float yRunIn = 57;

		const float yRev = 74;

		const float yRstKn = 90.9;
		const float yRst = 100;

		const float yOut = 117.5;

		const float xLength = 20.5;
		const float yLength = 19.6;

		const float xStepsIn = 29.5;
		const float yStepsIn = 24;

		const float xInL = 19.3;
		const float xInR = xInL+9;
		const float xLightL = xInL+3;
		const float xLightR = xInR+3;

		const float ys = 34;
		const float yShift = 11;

		const float yShiftBlock = 3;
		const float yShiftBlock2 = 3.5;

		const float yLightShift = 4.7;

		const float xProg = 42.7;
		const float yProgKnob = 36.8;
		const float yProgIn = 51;

		const float ySet = 70;
		const float yAuto = 82.5;

		const float yRecall = 100;
		const float yStore = 115.6;
		

		addParam(createParamCentered<CKSS>(mm2px(Vec(xLeft, yCvSw)), module, StepSeqPlus::MODE_SWITCH));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yTrig)), module, StepSeqPlus::CLK_INPUT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xLeft, yRstKn)), module, StepSeqPlus::RST_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yRst)), module, StepSeqPlus::RST_INPUT));

		addParam(createLightParamCentered<VCVLightBezelLatch<BlueLight>>(mm2px(Vec(xLeft, yRunBut)), module, StepSeqPlus::RUNBUT_PARAM, StepSeqPlus::RUNBUT_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yRunIn)), module, StepSeqPlus::RUN_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yRev)), module, StepSeqPlus::REV_INPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xLeft, yOut)), module, StepSeqPlus::OUT_OUTPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xStepsIn, yStepsIn)), module, StepSeqPlus::LENGTH_INPUT));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xLength, yLength)), module, StepSeqPlus::LENGTH_PARAM));

		for (int i = 0; i < 4; i++) {

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xInL, ys+(i*yShift))), module, StepSeqPlus::STEP_PARAM+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xInR, ys+(i*yShift)+yShiftBlock2)), module, StepSeqPlus::STEP_PARAM+i+8));

			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xLightL, ys+(i*yShift)-yLightShift)), module, StepSeqPlus::STEP_LIGHT+i));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xLightR, ys+(i*yShift)+yShiftBlock2-yLightShift)), module, StepSeqPlus::STEP_LIGHT+i+8));
		
		}

		for (int i = 4; i < 8; i++) {

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xInL, ys+(i*yShift)+yShiftBlock)), module, StepSeqPlus::STEP_PARAM+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xInR, ys+(i*yShift)+yShiftBlock+yShiftBlock2)), module, StepSeqPlus::STEP_PARAM+i+8));

			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xLightL, ys+(i*yShift)+yShiftBlock-yLightShift)), module, StepSeqPlus::STEP_LIGHT+i));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xLightR, ys+(i*yShift)+yShiftBlock+yShiftBlock2-yLightShift)), module, StepSeqPlus::STEP_LIGHT+i+8));
		}
		
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xProg, yProgKnob)), module, StepSeqPlus::PROG_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xProg, yProgIn)), module, StepSeqPlus::PROG_INPUT));

		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xProg, ySet)), module, StepSeqPlus::SET_PARAM, StepSeqPlus::SET_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xProg, yAuto)), module, StepSeqPlus::AUTO_PARAM, StepSeqPlus::AUTO_LIGHT));

		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xProg, yRecall)), module, StepSeqPlus::RECALL_PARAM, StepSeqPlus::RECALL_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xProg, yStore)), module, StepSeqPlus::STORE_PARAM, StepSeqPlus::STORE_LIGHT));
		

	}

	void appendContextMenu(Menu* menu) override {
		StepSeqPlus* module = dynamic_cast<StepSeqPlus*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Randomize Steps", "", [=]() {module->randomizeTrack();}));

		menu->addChild(new MenuSeparator());

		struct RangeItem : MenuItem {
			StepSeqPlus* module;
			int range;
			void onAction(const event::Action& e) override {
				module->range = range;
			}
		};

		std::string rangeNames[10] = {"0/1v", "0/2v", "0/3v", "0/5v", "0/10v", "-1/+1v", "-2/+2v", "-3/+3v", "-5/+5v", "-10/+10v"};
		menu->addChild(createSubmenuItem("Knobs Range", rangeNames[module->range], [=](Menu * menu) {
			for (int i = 0; i < 10; i++) {
				RangeItem* rangeItem = createMenuItem<RangeItem>(rangeNames[i]);
				rangeItem->rightText = CHECKMARK(module->range == i);
				rangeItem->module = module;
				rangeItem->range = i;
				menu->addChild(rangeItem);
			}
		}));
		
		menu->addChild(new MenuSeparator());
		
		struct RunTypeItem : MenuItem {
			StepSeqPlus* module;
			int runType;
			void onAction(const event::Action& e) override {
				module->runType = runType;
			}
		};

		std::string RunTypeNames[2] = {"Gate", "Trig"};
		menu->addChild(createSubmenuItem("Run Input", (RunTypeNames[module->runType]), [=](Menu * menu) {
			for (int i = 0; i < 2; i++) {
				RunTypeItem* runTypeItem = createMenuItem<RunTypeItem>(RunTypeNames[i]);
				runTypeItem->rightText = CHECKMARK(module->runType == i);
				runTypeItem->module = module;
				runTypeItem->runType = i;
				menu->addChild(runTypeItem);
			}
		}));

		struct RevTypeItem : MenuItem {
			StepSeqPlus* module;
			int revType;
			void onAction(const event::Action& e) override {
				module->revType = revType;
			}
		};
		std::string RevTypeNames[2] = {"Positive", "Negative"};
		menu->addChild(createSubmenuItem("Reverse Input Voltage", (RevTypeNames[module->revType]), [=](Menu * menu) {
			for (int i = 0; i < 2; i++) {
				RevTypeItem* revTypeItem = createMenuItem<RevTypeItem>(RevTypeNames[i]);
				revTypeItem->rightText = CHECKMARK(module->revType == i);
				revTypeItem->module = module;
				revTypeItem->revType = i;
				menu->addChild(revTypeItem);
			}
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Reset on Run", "", &module->rstOnRun));
		
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("1st clock after reset:"));
		menu->addChild(createBoolPtrMenuItem("Don't advance", "", &module->dontAdvanceSetting));

		menu->addChild(new MenuSeparator());
		struct ProgInTypeItem : MenuItem {
			StepSeqPlus* module;
			int progInType;
			void onAction(const event::Action& e) override {
				module->progInType = progInType;
			}
		};

		std::string ProgInTypeNames[2] = {"CV", "Trig"};
		menu->addChild(createSubmenuItem("Prog Input type", (ProgInTypeNames[module->progInType]), [=](Menu * menu) {
			for (int i = 0; i < 2; i++) {
				ProgInTypeItem* progInTypeItem = createMenuItem<ProgInTypeItem>(ProgInTypeNames[i]);
				progInTypeItem->rightText = CHECKMARK(module->progInType == i);
				progInTypeItem->module = module;
				progInTypeItem->progInType = i;
				menu->addChild(progInTypeItem);
			}
		}));
		menu->addChild(createMenuItem("Scan Last Prog", "current: " + to_string(module->lastProg), [=]() {module->scanLastProg();}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Copy Sequence", "", [=]() {module->copyClipboard();}));
		if (stepSeq_clipboard)
			menu->addChild(createMenuItem("Paste Sequence", "", [=]() {module->pasteClipboard();}));
		else
			menu->addChild(createMenuLabel("Paste Sequence"));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("DISK operations", "", [=](Menu * menu) {
			menu->addChild(createMenuItem("Load preset", "", [=]() {module->menuLoadPreset();}));
			menu->addChild(createMenuItem("Save preset", "", [=]() {module->menuSavePreset();}));
		}));

		menu->addChild(createSubmenuItem("Erase ALL progs", "", [=](Menu * menu) {
			menu->addChild(createSubmenuItem("Are you Sure?", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("ERASE!", "", [=]() {module->eraseProgs();}));
			}));
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Tips", "", [=](Menu * menu) {
			menu->addChild(createMenuLabel("Store Programs with double-click"));
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("Remember to store programs"));
			menu->addChild(createMenuLabel("when importing or pasting"));
		}));
	}

};

Model* modelStepSeqPlus = createModel<StepSeqPlus, StepSeqPlusWidget>("StepSeqPlus");