#define STD2x_PROGRESSION 0
#define P_1_3_PROGRESSION 2 // 1.3x
#define FIBONACCI_PROGRESSION 3

#define BIT_8 0
#define BIT_16 1

#define OUT_TRIG 0
#define OUT_GATE 1
#define OUT_CLOCK 2

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

struct RandLoops : Module {
	enum ParamId {
		CTRL_PARAM,
		LENGTH_PARAM,
		SCALE_PARAM,
		DEL_BUTTON,
		ADD_BUTTON,
		RND_BUTTON,
		//ENUMS(REGISTER_PARAM, 16),
		PROG_PARAM,
		RECALL_PARAM,
		STORE_PARAM,
		SET_PARAM,
		AUTO_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CTRL_INPUT,
		DEL_INPUT,
		ADD_INPUT,
		CLK_INPUT,
		RST_INPUT,
		CLEAR_INPUT,
		RND_INPUT,
		PROG_INPUT,
		RECALL_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		TRIG_OUTPUT,
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		DEL_LIGHT,
		ADD_LIGHT,
		RND_LIGHT,
		ENUMS(REGISTER_LIGHT, 16),
		RECALL_LIGHT,
		STORE_LIGHT,
		SET_LIGHT,
		AUTO_LIGHT,
		LIGHTS_LEN
	};

	bool initStart = false;

	float clock = 0;
	float prevClock = 0;

	float rstValue = 0;
	float prevRstValue = 0;

	float volt = 0;
	float cvOut = 0;
	float trigOut = 0;

	float controlValue = 0;

	bool dontAdvance = false;
	bool dontAdvanceSetting = true;

	int progression = STD2x_PROGRESSION;

	const float tableVolts[3][2][16] = {
		{
			{0.03922, 0.07844, 0.15688, 0.31376, 0.62752, 1.25504, 2.51008, 5.02016, 0, 0, 0, 0, 0, 0, 0, 0},
			{0.00015259, 0.000305181, 0.000610361, 0.001220722, 0.002441445, 0.00488289, 0.009765779, 0.019531558, 0.039063117, 0.078126234, 0.156252467, 0.312504934, 0.625009869, 1.250019738, 2.500039475, 5.00007895}
		},
		{
			{0.4191521, 0.54489773, 0.708367049, 0.920877164, 1.197140313, 1.556282407, 2.023167129, 2.630117267, 0, 0, 0, 0, 0, 0, 0, 0},
			{0.04577242, 0.059504146, 0.07735539, 0.100562007, 0.130730609, 0.169949791, 0.220934729, 0.287215147, 0.373379692, 0.485393599, 0.631011679, 0.820315183, 1.066409737, 1.386332659, 1.802232456, 2.342902193}
		},
		{
			{0.114942529, 0.229885058, 0.344827586, 0.574712644, 0.91954023, 1.494252874, 2.413793105, 3.908045979, 0, 0, 0, 0, 0, 0, 0, 0},
			{0.002392917, 0.004785834, 0.007178751, 0.011964585, 0.019143336, 0.031107921, 0.050251256, 0.081359177, 0.131610433, 0.21296961, 0.344580044, 0.557549654, 0.902129698, 1.459679352, 2.361809049, 3.821488401}
		}
	};

	int progSeq[32][16] = {
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
							};

	int progSteps[32] = {16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16};

	// --------------workingSeq

	int wSeq[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int wSteps = 16;

	int nextSeq[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int nextSteps = 16;

	int pendingSeq[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int pendingSteps = 16;


	// ---------- OLD shiftRegister randLoops

	int shiftRegister[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	int saveRegister[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	int tempRegister[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	
	int tempSaveRegister[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	// ------------------------

	float probCtrl = 0;
	float probCtrlRnd = 0;
	float probRegister = 0;

	bool incomingRegister = false;

	int startingStep = 0;

	const int bitResTable[2] = {8, 16};
	int bitResolution = BIT_8;

	//std::string resolutionName[2] = {"8 bit", "16 bit"};
	//std::string progressionName[3] = {"2x (std)", "1.3x", "Fibonacci"};

	bool bufferedAddDel = true;
	bool bufferedRandom = true;

	float delTrig = 0;
	float prevDelTrig = 0;
	bool delWait = false;

	float addTrig = 0;
	float prevAddTrig = 0;
	bool addWait = false;

	float rndTrig = 0;
	float prevRndTrig = 0;
	bool rndWait = false;

	float clrValue = 0;
	float prevClrValue = 0;
	
	//bool pulse = false;
	//float pulseTime = 0;

	//int tableLength[8] = {1, 2, 3, 4, 5, 7, 11, 15};

	//bool trigMode = false;

	int outType = OUT_TRIG;

	float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	bool stepPulse = false;
	float stepPulseTime = 0;
	bool outGate = false;

		// --------------prog
	int progKnob = 0;
	int prevProgKnob = 0;
	int savedProgKnob = 0;

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

	bool instantScaleChange = false;

	bool butSetScale = false;
	float scaleSetBut = 0;
	float prevScaleSetBut = 0;

	float resetScale = 0;
	float prevResetScale = 0;

	bool pendingUpdate = false;
	bool seqChanged = false;

	// ------- set button light

	bool setButLight = false;
	float setButLightDelta = 2 / APP->engine->getSampleRate();
	float setButLightValue = 0.f;

	// ------- clipboard

	bool clipboard = false;
	int cbSeq[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int cbSteps = 16;


	RandLoops() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(CTRL_PARAM, -1, 1.f, 0.f, "Control");
		configInput(CTRL_INPUT, "Ctrl CV");

		configParam(LENGTH_PARAM, 1.f, 16.f, 16.f, "Length");
		paramQuantities[LENGTH_PARAM]->snapEnabled = true;

		configParam(SCALE_PARAM, 0.f, 1.f, 1.f, "Scale", "%", 0, 100);

		configSwitch(DEL_BUTTON, 0.f, 1.f, 0.f, "Delete", {"OFF", "ON"});
		configSwitch(ADD_BUTTON, 0.f, 1.f, 0.f, "Add", {"OFF", "ON"});
		configSwitch(RND_BUTTON, 0.f, 1.f, 0.f, "Random", {"OFF", "ON"});

		configInput(DEL_INPUT, "Delete");
		configInput(ADD_INPUT, "Add");
		configInput(RND_INPUT, "Random");

		configInput(CLK_INPUT, "Clock");
		configInput(CLEAR_INPUT, "Clear");

		configInput(RST_INPUT, "Reset");
		
		configOutput(TRIG_OUTPUT, "Trig");
		configOutput(OUT_OUTPUT, "Output");

		configParam(PROG_PARAM, 0.f, 31.f, 0.f, "Prog");
		configInput(PROG_INPUT, "Prog");
		paramQuantities[PROG_PARAM]->snapEnabled = true;
		configSwitch(SET_PARAM, 0, 1.f, 0.f, "Set", {"OFF", "ON"});
		configSwitch(RECALL_PARAM, 0, 1.f, 0.f, "Recall", {"OFF", "ON"});
		configInput(RECALL_INPUT, "Recall");
		configSwitch(STORE_PARAM, 0, 1.f, 0.f, "Store", {"OFF", "ON"});
		configSwitch(AUTO_PARAM, 0, 1.f, 0.f, "Auto", {"OFF", "ON"});

		//if (!trigMode)
			calcVoltage();

	}

	void onReset(const ResetEvent &e) override {

		/*
		for (int i=0; i < 16; i++) {
			shiftRegister[i] = false;
			lights[REGISTER_LIGHT+i].setBrightness(0.f);
		}
		*/

		rndWait = false;
		addWait = false;
		delWait = false;

		initStart = false;
		
		clearAll(); // ------------------------------------------------------------<<<<<<<<<<<<<<<<<

		setButLight = false;
		setButLightDelta = 2 / APP->engine->getSampleRate();
		setButLightValue = 0.f;

		for (int i = 0; i < 16; i++) {
			wSeq[i] = 0;
			//params[STEP_PARAM+i].setValue(wSeq[i]);
			//lights[REGISTER_LIGHT+i].setBrightness(wSeq[i]);
		}
		wSteps = 16;
		params[LENGTH_PARAM].setValue(wSteps);
		//wRst = 1;
		//params[RST_PARAM].setValue(wRst);

		Module::onReset(e);
	}

	void onSampleRateChange() override {
		oneMsTime = (APP->engine->getSampleRate()) / 1000;
		storeSamples = APP->engine->getSampleRate() / 1.5f;
		maxStoredProgramTime = APP->engine->getSampleRate() * 1.5;
		setButLightDelta = 2 / APP->engine->getSampleRate();
	}
	
	json_t* dataToJson() override {

		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "initStart", json_boolean(initStart));
		json_object_set_new(rootJ, "dontAdvanceSetting", json_boolean(dontAdvanceSetting));
		json_object_set_new(rootJ, "bitResolution", json_integer(bitResolution));
		json_object_set_new(rootJ, "progression", json_integer(progression));
		json_object_set_new(rootJ, "bufferedAddDel", json_boolean(bufferedAddDel));
		json_object_set_new(rootJ, "bufferedRandom", json_boolean(bufferedRandom));
		json_object_set_new(rootJ, "outType", json_integer(outType));
		
		json_object_set_new(rootJ, "savedProgKnob", json_integer(savedProgKnob));

		storeSequence();

		json_t *seq_json_array = json_array();
		for (int tempStep = 0; tempStep < 16; tempStep++) {
			json_array_append_new(seq_json_array, json_integer(saveRegister[tempStep]));
		}
		json_object_set_new(rootJ, "wSeq", seq_json_array);	

		json_object_set_new(rootJ, "wSteps", json_integer(wSteps));

		for (int prog = 0; prog < 32; prog++) {
			json_t *prog_json_array = json_array();
			for (int tempStep = 0; tempStep < 16; tempStep++) {
				json_array_append_new(prog_json_array, json_integer(progSeq[prog][tempStep]));
			}
			json_object_set_new(rootJ, ("prog"+to_string(prog)).c_str(), prog_json_array);	
		}

		for (int prog = 0; prog < 32; prog++) {
			json_t *progSteps_json_array = json_array();
			json_array_append_new(progSteps_json_array, json_integer(progSteps[prog]));
			json_object_set_new(rootJ, ("progSteps"+to_string(prog)).c_str(), progSteps_json_array);
		}

		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {

		json_t* initStartJ = json_object_get(rootJ, "InitStart");
		if (initStartJ)
			initStart = json_boolean_value(initStartJ);

		json_t* bitResolutionJ = json_object_get(rootJ, "bitResolution");
		if (bitResolutionJ) {
			bitResolution = json_integer_value(bitResolutionJ);
			if (bitResolution < 0 && bitResolution > 1)
				bitResolution = BIT_8;
		}

		json_t* progressionJ = json_object_get(rootJ, "progression");
		if (progressionJ) {
			progression = json_integer_value(progressionJ);
			if (progression < 0 && progression > 2)
				progression = STD2x_PROGRESSION;
		}

		json_t* dontAdvanceSettingJ = json_object_get(rootJ, "dontAdvanceSetting");
		if (dontAdvanceSettingJ) {
			dontAdvanceSetting = json_boolean_value(dontAdvanceSettingJ);
		}
		
		json_t* bufferedAddDelJ = json_object_get(rootJ, "bufferedAddDel");
		if (bufferedAddDelJ)
			bufferedAddDel = json_boolean_value(bufferedAddDelJ);

		json_t* bufferedRandomJ = json_object_get(rootJ, "bufferedRandom");
		if (bufferedRandomJ)
			bufferedRandom = json_boolean_value(bufferedRandomJ);

		json_t* outTypeJ = json_object_get(rootJ, "outType");
		if (outTypeJ) {
			outType = json_integer_value(outTypeJ);
			if (outType < 0 || outType > 2)
				outType = 0;
		}

		if (!initStart) {

			json_t *prog_json_array = json_object_get(rootJ, "wSeq");
			size_t tempSeq;
			json_t *json_value;
			if (prog_json_array) {
				json_array_foreach(prog_json_array, tempSeq, json_value) {
					wSeq[tempSeq] = json_integer_value(json_value);
				}
			}
			startingStep = 0;

			json_t* lengthJ = json_object_get(rootJ, "wSteps");
			if (lengthJ) {
				if (json_integer_value(lengthJ) < 1 || json_integer_value(lengthJ) > 16)
					wSteps = 16;				
				else
					wSteps = json_integer_value(lengthJ);

				params[LENGTH_PARAM].setValue(wSteps);
			}
		}

		json_t* savedProgKnobJ = json_object_get(rootJ, "savedProgKnob");
		if (savedProgKnobJ) {
			savedProgKnob = json_integer_value(savedProgKnobJ);
			if (savedProgKnob < 0 || savedProgKnob > 31)
				savedProgKnob = 0;
			
		} else {
			savedProgKnob = 0;
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
					progSeq[prog][tempSeq] = json_integer_value(json_value);
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

	}

	// ------------------------ LOAD / SAVE   FULL PRESET

	json_t *presetToJson() {

		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "dontAdvanceSetting", json_boolean(dontAdvanceSetting));
		json_object_set_new(rootJ, "bitResolution", json_integer(bitResolution));
		json_object_set_new(rootJ, "progression", json_integer(progression));
		json_object_set_new(rootJ, "bufferedAddDel", json_boolean(bufferedAddDel));
		json_object_set_new(rootJ, "bufferedRandom", json_boolean(bufferedRandom));
		json_object_set_new(rootJ, "outType", json_integer(outType));

		for (int prog = 0; prog < 32; prog++) {
			json_t *prog_json_array = json_array();
			for (int tempStep = 0; tempStep < 16; tempStep++) {
				json_array_append_new(prog_json_array, json_integer(progSeq[prog][tempStep]));
			}
			json_object_set_new(rootJ, ("prog"+to_string(prog)).c_str(), prog_json_array);	
		}

		for (int prog = 0; prog < 32; prog++) {
			json_t *progSteps_json_array = json_array();
			json_array_append_new(progSteps_json_array, json_integer(progSteps[prog]));
			json_object_set_new(rootJ, ("progSteps"+to_string(prog)).c_str(), progSteps_json_array);
		}

		return rootJ;
	}

	void presetFromJson(json_t *rootJ) {

		json_t* bitResolutionJ = json_object_get(rootJ, "bitResolution");
		if (bitResolutionJ) {
			bitResolution = json_integer_value(bitResolutionJ);
			if (bitResolution < 0 && bitResolution > 1)
				bitResolution = BIT_8;
		}

		json_t* progressionJ = json_object_get(rootJ, "progression");
		if (progressionJ) {
			progression = json_integer_value(progressionJ);
			if (progression < 0 && progression > 2)
				progression = STD2x_PROGRESSION;
		}

		json_t* dontAdvanceSettingJ = json_object_get(rootJ, "dontAdvanceSetting");
		if (dontAdvanceSettingJ) {
			dontAdvanceSetting = json_boolean_value(dontAdvanceSettingJ);
		}
		
		json_t* bufferedAddDelJ = json_object_get(rootJ, "bufferedAddDel");
		if (bufferedAddDelJ)
			bufferedAddDel = json_boolean_value(bufferedAddDelJ);

		json_t* bufferedRandomJ = json_object_get(rootJ, "bufferedRandom");
		if (bufferedRandomJ)
			bufferedRandom = json_boolean_value(bufferedRandomJ);

		json_t* outTypeJ = json_object_get(rootJ, "outType");
		if (outTypeJ) {
			outType = json_integer_value(outTypeJ);
			if (outType < 0 || outType > 2)
				outType = 0;
		}

		for (int prog = 0; prog < 32; prog++) {
			json_t *prog_json_array = json_object_get(rootJ, ("prog"+to_string(prog)).c_str());
			size_t tempSeq;
			json_t *json_value;
			if (prog_json_array) {
				json_array_foreach(prog_json_array, tempSeq, json_value) {
					progSeq[prog][tempSeq] = json_integer_value(json_value);
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
	}

	void menuLoadPreset() {
		static const char FILE_FILTERS[] = "randLoops preset (.rlp):rlp,RLP";
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

		static const char FILE_FILTERS[] = "randLoops preset (.rlp):rlp,RLP";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".rlp" and strPath.substr(strPath.size() - 4) != ".RLP")
				strPath += ".rlp";
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

	// ------------------------ LOAD / SAVE   SINGLE SEQUENCE

	json_t *sequenceToJson() {
		storeSequence();

		json_t *rootJ = json_object();
		
		json_t *prog_json_array = json_array();
		for (int tempStep = 0; tempStep < 16; tempStep++) {
			json_array_append_new(prog_json_array, json_integer(saveRegister[tempStep]));
		}
		json_object_set_new(rootJ, "sr", prog_json_array);	
	
		//json_object_set_new(rootJ, "length", json_integer((int)params[LENGTH_PARAM].getValue()));
		json_object_set_new(rootJ, "length", json_integer(wSteps));
		return rootJ;
	}

	void sequenceFromJson(json_t *rootJ) {

		json_t *prog_json_array = json_object_get(rootJ, "sr");
		size_t tempSeq;
		json_t *json_value;
		if (prog_json_array) {
			json_array_foreach(prog_json_array, tempSeq, json_value) {
				wSeq[tempSeq] = json_integer_value(json_value);
			}
		}
		startingStep = 0;

		json_t* lengthJ = json_object_get(rootJ, "length");
		if (lengthJ) {
			if (json_integer_value(lengthJ) < 1 || json_integer_value(lengthJ) > 16)
				wSteps = 16;				
			else
				wSteps = json_integer_value(lengthJ);

			params[LENGTH_PARAM].setValue(wSteps);
		}

	}

	void menuLoadSequence() {
		static const char FILE_FILTERS[] = "trigSeq preset (.tss):tss,TSS";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		if (path)
			loadSequence(path);

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadSequence(std::string path) {

		FILE *file = fopen(path.c_str(), "r");
		json_error_t error;
		json_t *rootJ = json_loadf(file, 0, &error);
		if (rootJ == NULL) {
			WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		}

		fclose(file);

		if (rootJ) {

			sequenceFromJson(rootJ);

		} else {
			WARN("problem loading preset json file");
			//return;
		}
		
	}

	void menuSaveSequence() {

		static const char FILE_FILTERS[] = "trigSeq sequence (.tss):tss,TSS";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".tss" and strPath.substr(strPath.size() - 4) != ".TSS")
				strPath += ".tss";
			path = strcpy(new char[strPath.length() + 1], strPath.c_str());
			saveSequence(path, sequenceToJson());
		}

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void saveSequence(std::string path, json_t *rootJ) {

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


	void inline storeSequence() {
		int cursor = 0;
		for (int i = startingStep; i < wSteps; i++) {
			tempSaveRegister[cursor] = wSeq[i];
			cursor++;
		}

		for (int i = 0; i < startingStep; i++) {
			tempSaveRegister[cursor] = wSeq[i];
			cursor++;
		}

		int fillCursor = 0;
		for (int i = cursor; i < 16; i++) {
			tempSaveRegister[i] = tempSaveRegister[fillCursor];
			fillCursor++;
			if (fillCursor >= wSteps)
				fillCursor = 0;
		}

		for (int i = 0; i < 16; i++)
			saveRegister[i] = tempSaveRegister[i];

	}

	void copyClipboard() {
		storeSequence();
		for (int i = 0; i < 16; i++)
			cbSeq[i] = saveRegister[i];
		
		cbSteps = wSteps;
		clipboard = true;
	}

	void pasteClipboard() {
		for (int i = 0; i < 16; i++)
			wSeq[i] = cbSeq[i];
		
		wSteps = cbSteps;
		params[LENGTH_PARAM].setValue(wSteps);

	}

	void eraseProgs() {
		for (int i = 0; i < 32; i++) {
			progSteps[i] = 16;
			for (int j = 0; j < 16; j++)
				progSeq[i][j] = 0;
		}
	}

	void inline resetCheck() {
		if (rstValue >= 1.f && prevRstValue < 1.f) {
			
			int cursor = 0;
			//for (int i = startingStep; i < int(params[LENGTH_PARAM].getValue()); i++) {
			for (int i = startingStep; i < wSteps; i++) {
				//tempRegister[cursor] = shiftRegister[i];
				tempRegister[cursor] = wSeq[i];
				cursor++;
			}

			for (int i = 0; i < startingStep; i++) {
				//tempRegister[cursor] = shiftRegister[i];
				tempRegister[cursor] = wSeq[i];
				cursor++;
			}

			int fillCursor = 0;
			for (int i = cursor; i < 16; i++) {
				tempRegister[i] = tempRegister[fillCursor];
				fillCursor++;
				//if (fillCursor >= int(params[LENGTH_PARAM].getValue()))
				if (fillCursor >= wSteps)
					fillCursor = 0;
			}

			for (int i = 0; i < 16; i++)
				//shiftRegister[i] = tempRegister[i];
				wSeq[i] = tempRegister[i];

			//debugResettt(t);

			startingStep = 0;

			if (dontAdvanceSetting)
				dontAdvance = true;
		
			calcVoltage();

		}
		prevRstValue = rstValue;
	}

	void inline calcVoltage() {
		volt = 0;
		for (int i=0; i < bitResTable[bitResolution]; i++) {
			//if (shiftRegister[i])
			if (wSeq[i])
				volt += tableVolts[progression][bitResolution][i];
		}
	}

	void clearAll() {
		for (int step = 0; step < 16; step++)
			//shiftRegister[step] = 0;
			wSeq[step] = 0;

		calcVoltage();
	}

	void inline calcRandom() {

		int cursor = 0;
		//for (int i = 0; i < int(params[LENGTH_PARAM].getValue()); i++) {
		for (int i = 0; i < wSteps; i++) {
			probRegister = random::uniform();
			if (probRegister > 0.5)
				//shiftRegister[i] = 1;
				wSeq[i] = 1;
			else
				//shiftRegister[i] = 0;
				wSeq[i] = 0;
			cursor++;
		}


		int fillCursor = 0;
		for (int i = cursor; i < 16; i++) {
			//shiftRegister[i] = shiftRegister[fillCursor];
			wSeq[i] = wSeq[fillCursor];
			fillCursor++;
			//if (fillCursor >= int(params[LENGTH_PARAM].getValue()))
			if (fillCursor >= wSteps)
				fillCursor = 0;
		}

		/*
		for (int i=0; i<16; i++)
			//lights[REGISTER_LIGHT+i].setBrightness(shiftRegister[i]);
			lights[REGISTER_LIGHT+i].setBrightness(wSeq[i]);
		*/
	}


	
	void process(const ProcessArgs& args) override {

		// ----------- AUTO SWITCH

		instantScaleChange = int(params[AUTO_PARAM].getValue());
		lights[AUTO_LIGHT].setBrightness(instantScaleChange);

		// ----------- PROGRAM MANAGEMENT

		progKnob = int(params[PROG_PARAM].getValue() + (inputs[PROG_INPUT].getVoltage() * 3.2));
		if (progKnob < 0)
			progKnob = 0;
		else if (progKnob > 31)
			progKnob = 31;

		if (progKnob != prevProgKnob) {

			if (progKnob != workingProg) {

				//pendingUpdate = true;
				progChanged = true;
				selectedProg = progKnob;
				prevProgKnob = progKnob;

				for (int i = 0; i < 16; i++) {
					nextSeq[i] = progSeq[selectedProg][i];
					//pendingSeq[i] = nextSeq[i];

					//params[STEP_PARAM+i].setValue(nextSeq[i]);
				}
				nextSteps = progSteps[selectedProg];
				//pendingSteps = nextSteps;
				params[LENGTH_PARAM].setValue(nextSteps);

				//nextRst = progRst[selectedProg];
				//pendingRst = nextRst;
				//params[RST_PARAM].setValue(nextRst);

				//seqChanged = true;

				setButLight = true;
				setButLightValue = 0.f;
			} else {
				progChanged = false;
				selectedProg = progKnob;
				prevProgKnob = progKnob;
				params[LENGTH_PARAM].setValue(wSteps);
				setButLight = false;
				setButLightValue = 0.f;
			}

		}

		// -------- populate next seq array and show it
		/*
		if (pendingUpdate) {
			for (int i = 0; i < 16; i++) {
				//nextSeq[i] = params[STEP_PARAM+i].getValue();
				nextSeq[i] = wSeq[i];
				if (nextSeq[i] != pendingSeq[i])
					seqChanged = true;

				//params[STEP_PARAM+i].setValue(nextSeq[i]);
				//lights[STEPBUT_LIGHT+i].setBrightness(nextSeq[i]);
				lights[REGISTER_LIGHT+i].setBrightness(nextSeq[i]);
			}
			if (nextSteps != pendingSteps)
				seqChanged = true;
			params[LENGTH_PARAM].setValue(nextSteps);
			
			//if (nextRst != pendingRst)
			//	seqChanged = true;
			//params[RST_PARAM].setValue(nextRst);
			

		} else {
			for (int i = 0; i < 16; i++) {
				//nextSeq[i] = params[STEP_PARAM+i].getValue();
				nextSeq[i] = wSeq[i];
				if (nextSeq[i] != wSeq[i])
					seqChanged = true;

				//params[STEP_PARAM+i].setValue(nextSeq[i]);
				//lights[STEPBUT_LIGHT+i].setBrightness(nextSeq[i]);
				lights[REGISTER_LIGHT+i].setBrightness(nextSeq[i]);
			}
			nextSteps = params[LENGTH_PARAM].getValue();
			if (nextSteps != wSteps)
				seqChanged = true;
			params[LENGTH_PARAM].setValue(nextSteps);

			//nextRst = params[RST_PARAM].getValue();
			//if (nextRst != wRst)
			//	seqChanged = true;
			//params[RST_PARAM].setValue(nextRst);
			
		}
		*/
		// -------- CURRENT SEQ UPDATE

		butSetScale = false;

		scaleSetBut = params[SET_PARAM].getValue();
		if (scaleSetBut >= 1.f && prevScaleSetBut < 1.f)
			butSetScale = true;

		prevScaleSetBut = scaleSetBut;

		//if (seqChanged) {
			//if (pendingUpdate) {
			if (progChanged) {
				if (instantScaleChange) {

					for (int i = 0; i < 16; i++)
						wSeq[i] = nextSeq[i];

					wSteps = nextSteps;
					//wRst = nextRst;
					startingStep = 0;

					//pendingUpdate = false;
					progChanged = false;

					//if (progChanged) {
						workingProg = selectedProg;
						savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
					//}
					//seqChanged = false;

					setButLight = false;
					setButLightValue = 0.f;

				} else {
					
					if (butSetScale) {
						butSetScale = false;

						for (int i = 0; i < 16; i++)
							wSeq[i] = nextSeq[i];

						wSteps = nextSteps;
						//wRst = nextRst;
						startingStep = 0;

						//pendingUpdate = false;
						progChanged = false;

						//if (progChanged) {
							workingProg = selectedProg;
							savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
						//}
						//seqChanged = false;

						setButLight = false;
						setButLightValue = 0.f;
					}

				}
		
			} /*else {	// if there are NOT pending prog updates (only manual steps are changed)

				for (int i = 0; i < 16; i++)
					wSeq[i] = nextSeq[i];

				wSteps = nextSteps;
				//wRst = nextRst;

				seqChanged = false;
			}*/
		//}

		// -------------------------- RECALL PROG

		recallBut = params[RECALL_PARAM].getValue() + inputs[RECALL_INPUT].getVoltage();
		lights[RECALL_LIGHT].setBrightness(recallBut);

		if (recallBut >= 1.f && prevRecallBut < 1.f) {

			for (int i = 0; i < 16; i++) {
				wSeq[i] = progSeq[selectedProg][i];
				startingStep = 0;
				//nextSeq[i] = wSeq[i];
				//params[STEP_PARAM+i].setValue(wSeq[i]);
				//lights[STEPBUT_LIGHT+i].setBrightness(wSeq[i]);
				//lights[REGISTER_LIGHT+i].setBrightness(wSeq[i]);
			}
			wSteps = progSteps[selectedProg];
			params[LENGTH_PARAM].setValue(wSteps);
			//wRst = progRst[selectedProg];
			//params[RST_PARAM].setValue(wRst);

			workingProg = selectedProg;
			savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
			//seqChanged = false;
			//pendingUpdate = false;
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

				// registra la sequenza nel programma partendo dallo startingStep
				/*
				for (int i = 0; i < 16; i++)
					progSeq[progKnob][i] = nextSeq[i];
				progSteps[progKnob] = nextSteps;
				//progRst[progKnob] = nextRst;
				*/
				int cursor = 0;
				//for (int i = startingStep; i < int(params[LENGTH_PARAM].getValue()); i++) {
				for (int i = startingStep; i < 16; i++) {
					//tempRegister[cursor] = wSeq[i];
					progSeq[progKnob][cursor] = wSeq[i];
					cursor++;
				}

				for (int i = 0; i <= startingStep; i++) {
					//tempRegister[cursor] = shiftRegister[i];
					//tempRegister[cursor] = wSeq[i];
					progSeq[progKnob][cursor] = wSeq[i];
					cursor++;
				}

				progSteps[progKnob] = wSteps;

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

		// ----------- get working steps

		if (!progChanged)
			wSteps = params[LENGTH_PARAM].getValue();

		// -------------------------------- clear sequence

		clrValue = inputs[CLEAR_INPUT].getVoltage();
		if (clrValue >= 1.f && prevClrValue < 1.f) {

			for (int i = 0; i < 16; i++) {
				//shiftRegister[i] = false;
				wSeq[i] = 0;

				//lights[REGISTER_LIGHT+i].setBrightness(0.f);
			}

			rndWait = false;
			addWait = false;
			delWait = false;

			lights[RND_LIGHT].setBrightness(0.f);
			lights[ADD_LIGHT].setBrightness(0.f);
			lights[DEL_LIGHT].setBrightness(0.f);

		}
		prevClrValue = clrValue;

		// -------------------------------- random trigger

		rndTrig = inputs[RND_INPUT].getVoltage() + params[RND_BUTTON].getValue();
		if (rndTrig >= 1.f && prevRndTrig < 1.f) {
			rndTrig = 1;
			if (bufferedRandom) {
				rndWait = true;
				lights[RND_LIGHT].setBrightness(1.f);
			} else {
				calcRandom();
				rndWait = false;
			}
		}
		prevRndTrig = rndTrig;

		if (!bufferedRandom)
			lights[RND_LIGHT].setBrightness(rndTrig);

		// -------------------------------- del trigger

		delTrig = inputs[DEL_INPUT].getVoltage() + params[DEL_BUTTON].getValue();
		
		if (bufferedAddDel) {
			if (delTrig >= 1.f && prevDelTrig < 1.f) {
				delTrig = 1;
				delWait = true;
				lights[DEL_LIGHT].setBrightness(1.f);
			}
		} else {
			if (delTrig >= 1.f) {
				delTrig = 1;
				delWait = true;
			}
			lights[DEL_LIGHT].setBrightness(delTrig);
		}
		
		prevDelTrig = delTrig;
		
		// -------------------------------- add trigger

		addTrig = inputs[ADD_INPUT].getVoltage() + params[ADD_BUTTON].getValue();
		
		if (bufferedAddDel) {
			if (addTrig >= 1.f && prevAddTrig < 1.f) {
				addTrig = 1;
				addWait = true;
				lights[ADD_LIGHT].setBrightness(1.f);
			}
		} else {
			if (addTrig >= 1.f) {
				addTrig = 1;
				addWait = true;
			}
			lights[ADD_LIGHT].setBrightness(addTrig);
		}

		prevAddTrig = addTrig;

		// -------------------------------- reset check

		rstValue = inputs[RST_INPUT].getVoltage();
				
		resetCheck();

		// -------------------------------- clock trigger

		clock = inputs[CLK_INPUT].getVoltage();
		if (clock >= 1.f && prevClock < 1.f) {

			if (!dontAdvance) {

				if (rndWait && bufferedRandom) {
					calcRandom();
					rndWait = false;
					lights[RND_LIGHT].setBrightness(0.f);
				}

				startingStep++;

				//if (startingStep >= int(params[LENGTH_PARAM].getValue()))
				if (startingStep >= wSteps)
					startingStep = 0;

				if (addWait) {

					incomingRegister = 1;
					addWait = false;
					if (bufferedAddDel)
						lights[ADD_LIGHT].setBrightness(0.f);

				} else if (delWait) {

					incomingRegister = 0;
					delWait = false;
					if (bufferedAddDel)
						lights[DEL_LIGHT].setBrightness(0.f);

				} else {

					controlValue = params[CTRL_PARAM].getValue() + (inputs[CTRL_INPUT].getVoltage() / 10.f);
					if (controlValue < -1.f)
						controlValue = -1.f;
					else if (controlValue > 1.f)
						controlValue = 1.f;

					probCtrl = int(abs(controlValue * 10));
					probCtrlRnd = int(random::uniform() * 10);

					if (probCtrlRnd > probCtrl) {
						probRegister = random::uniform();
						if (probRegister > 0.5)
							incomingRegister = 1;
						else
							incomingRegister = 0;

					} else {

						//incomingRegister = shiftRegister[int(params[LENGTH_PARAM].getValue())-1];
						//incomingRegister = wSeq[int(params[LENGTH_PARAM].getValue())-1];
						incomingRegister = wSeq[wSteps-1];
						if (controlValue < 0)
							incomingRegister = !incomingRegister;
					}
				}

				/*
				shiftRegister[15] = shiftRegister[14];
				shiftRegister[14] = shiftRegister[13];
				shiftRegister[13] = shiftRegister[12];
				shiftRegister[12] = shiftRegister[11];
				shiftRegister[11] = shiftRegister[10];
				shiftRegister[10] = shiftRegister[9];
				shiftRegister[9] = shiftRegister[8];
				shiftRegister[8] = shiftRegister[7];
				shiftRegister[7] = shiftRegister[6];
				shiftRegister[6] = shiftRegister[5];
				shiftRegister[5] = shiftRegister[4];
				shiftRegister[4] = shiftRegister[3];
				shiftRegister[3] = shiftRegister[2];
				shiftRegister[2] = shiftRegister[1];
				shiftRegister[1] = shiftRegister[0];
				shiftRegister[0] = incomingRegister;
				*/
				wSeq[15] = wSeq[14];
				wSeq[14] = wSeq[13];
				wSeq[13] = wSeq[12];
				wSeq[12] = wSeq[11];
				wSeq[11] = wSeq[10];
				wSeq[10] = wSeq[9];
				wSeq[9] = wSeq[8];
				wSeq[8] = wSeq[7];
				wSeq[7] = wSeq[6];
				wSeq[6] = wSeq[5];
				wSeq[5] = wSeq[4];
				wSeq[4] = wSeq[3];
				wSeq[3] = wSeq[2];
				wSeq[2] = wSeq[1];
				wSeq[1] = wSeq[0];
				wSeq[0] = incomingRegister;

				calcVoltage();

				if (incomingRegister) {
					stepPulse = true;
					stepPulseTime = oneMsTime;
					if (outType == OUT_GATE)
						outGate = true;
				} else {
					if (outType == OUT_GATE) {
						outGate = false;
						trigOut = 0.f;
					}
				}

			} else {
				dontAdvance = false;
			}

		}
		prevClock = clock;

		if (stepPulse) {

			if (outType == OUT_TRIG) {
				stepPulseTime--;
				if (stepPulseTime < 0) {
					stepPulse = false;
					trigOut = 0.f;
				} else {
					trigOut = 10.f;
				}
			} else if (outType == OUT_CLOCK) {
				trigOut = clock;
				if (trigOut < 1.f) {
					trigOut = 0.f;
					stepPulse = false;
				}
			} else if (outType == OUT_GATE) {
				if (outGate)
					trigOut = 10.f;
				else
					trigOut = 0.f;
			}
		}

		cvOut = volt * params[SCALE_PARAM].getValue();

		if (cvOut > 10.f)
			cvOut = 10.f;
		else if (cvOut < -10.f)
			cvOut = -10.f;

		outputs[OUT_OUTPUT].setVoltage(cvOut);

		outputs[TRIG_OUTPUT].setVoltage(trigOut);

		if (!progChanged) {
			for (int i = 0; i < 16; i++)
				//lights[REGISTER_LIGHT+i].setBrightness(shiftRegister[i]);
				lights[REGISTER_LIGHT+i].setBrightness(wSeq[i]);
		} else {
			for (int i = 0; i < 16; i++)
				//lights[REGISTER_LIGHT+i].setBrightness(shiftRegister[i]);
				lights[REGISTER_LIGHT+i].setBrightness(nextSeq[i]);
		}

		if (!bufferedAddDel) {
			delWait = false;
			addWait = false;
		}

		if (!bufferedRandom) {
			rndWait = false;
		}

	}
};

struct RandLoopsDisplay : TransparentWidget {
	RandLoops *module;
	int frame = 0;
	RandLoopsDisplay() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/vgatrue.ttf"));
				
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				std::string currentDisplay;

				currentDisplay = to_string(module->workingProg);

				//if (!module->pendingUpdate) {
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

struct RandLoopsWidget : ModuleWidget {
	RandLoopsWidget(RandLoops* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RandLoops.svg")));

		{
			RandLoopsDisplay *display = new RandLoopsDisplay();
			//display->box.pos = mm2px(Vec(35.6, 7.8));
			display->box.pos = mm2px(Vec(40.2, 7.8));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float xCtrl = 26;
		const float yCtrl = 20.5;

		const float xCtrlCV = 10;
		const float yCtrlCV = 24;

		const float xLength = 11.7;
		const float yLength = 46;

		const float xScale = 30.9;
		const float yScale = 51.4;

		const float xDelBut = 8.5;
		const float yDelBut = 69.5;

		const float xAddBut = 20.5;
		const float yAddBut = 69.5;

		const float xRndBut = 32.5;
		const float yRndBut = 69.5;

		const float xDelIn = 8.5;
		const float yDelIn = 79.5;

		const float xAddIn = 20.5;
		const float yAddIn = 79.5;

		const float xRndIn = 32.5;
		const float yRndIn = 79.5;

		const float xClk = 8.5;
		const float yClk = 98.5;

		const float xRst = 20.5;
		const float yRst = 98.5;

		const float xClr = 32.5;
		const float yClr = 98.5;

		const float xTrg = 11;
		const float yTrg = 116;

		const float xOut = 30;
		const float yOut = 116;

		const float xLgStart = 24.5;
		const float xLgShift = 4;
		const float yLgStart1 = 31;
		const float yLgStart2 = 34;
		const float yLgStart3 = 37;
		const float yLgStart4 = 40;

		const float xProg = 42.7 + 5;
		const float yProgKnob = 36.8 - 2;
		const float yProgIn = 51 - 4;

		const float ySet = 70 - 6;
		const float yAuto = 82.5 - 6;

		const float yRecall = 92;
		const float yRecallIn = 101.9;
		const float yStore = 116.1;
		
		addParam(createParamCentered<SickoBigKnob>(mm2px(Vec(xCtrl, yCtrl)), module, RandLoops::CTRL_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCtrlCV, yCtrlCV)), module, RandLoops::CTRL_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xLength, yLength)), module, RandLoops::LENGTH_PARAM));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xScale, yScale)), module, RandLoops::SCALE_PARAM));

		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xDelBut, yDelBut)), module, RandLoops::DEL_BUTTON, RandLoops::DEL_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<GreenLight>>(mm2px(Vec(xAddBut, yAddBut)), module, RandLoops::ADD_BUTTON, RandLoops::ADD_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xRndBut, yRndBut)), module, RandLoops::RND_BUTTON, RandLoops::RND_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xDelIn, yDelIn)), module, RandLoops::DEL_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xAddIn, yAddIn)), module, RandLoops::ADD_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRndIn, yRndIn)), module, RandLoops::RND_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xClk, yClk)), module, RandLoops::CLK_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xClr, yClr)), module, RandLoops::CLEAR_INPUT));
		
		
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart, yLgStart1)), module, RandLoops::REGISTER_LIGHT+0));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + xLgShift, yLgStart1)), module, RandLoops::REGISTER_LIGHT+1));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart1)), module, RandLoops::REGISTER_LIGHT+2));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart1)), module, RandLoops::REGISTER_LIGHT+3));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart, yLgStart2)), module, RandLoops::REGISTER_LIGHT+4));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + xLgShift, yLgStart2)), module, RandLoops::REGISTER_LIGHT+5));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart2)), module, RandLoops::REGISTER_LIGHT+6));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart2)), module, RandLoops::REGISTER_LIGHT+7));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart, yLgStart3)), module, RandLoops::REGISTER_LIGHT+8));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + xLgShift, yLgStart3)), module, RandLoops::REGISTER_LIGHT+9));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart3)), module, RandLoops::REGISTER_LIGHT+10));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart3)), module, RandLoops::REGISTER_LIGHT+11));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart, yLgStart4)), module, RandLoops::REGISTER_LIGHT+12));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + xLgShift, yLgStart4)), module, RandLoops::REGISTER_LIGHT+13));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart4)), module, RandLoops::REGISTER_LIGHT+14));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart4)), module, RandLoops::REGISTER_LIGHT+15));
		

		/*
		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart, yLgStart1)), module, RandLoops::REGISTER_PARAM+0, RandLoops::REGISTER_LIGHT+0));
		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart + xLgShift, yLgStart1)), module, RandLoops::REGISTER_PARAM+1, RandLoops::REGISTER_LIGHT+1));
		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart1)), module, RandLoops::REGISTER_PARAM+2, RandLoops::REGISTER_LIGHT+2));
		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart1)), module, RandLoops::REGISTER_PARAM+3, RandLoops::REGISTER_LIGHT+3));

		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart, yLgStart2)), module, RandLoops::REGISTER_PARAM+4, RandLoops::REGISTER_LIGHT+4));
		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart + xLgShift, yLgStart2)), module, RandLoops::REGISTER_PARAM+5, RandLoops::REGISTER_LIGHT+5));
		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart2)), module, RandLoops::REGISTER_PARAM+6, RandLoops::REGISTER_LIGHT+6));
		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart2)), module, RandLoops::REGISTER_PARAM+7, RandLoops::REGISTER_LIGHT+7));

		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart, yLgStart3)), module, RandLoops::REGISTER_PARAM+8, RandLoops::REGISTER_LIGHT+8));
		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart + xLgShift, yLgStart3)), module, RandLoops::REGISTER_PARAM+9, RandLoops::REGISTER_LIGHT+9));
		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart3)), module, RandLoops::REGISTER_PARAM+10, RandLoops::REGISTER_LIGHT+10));
		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart3)), module, RandLoops::REGISTER_PARAM+11, RandLoops::REGISTER_LIGHT+11));

		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart, yLgStart4)), module, RandLoops::REGISTER_PARAM+12, RandLoops::REGISTER_LIGHT+12));
		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart + xLgShift, yLgStart4)), module, RandLoops::REGISTER_PARAM+13, RandLoops::REGISTER_LIGHT+13));
		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart4)), module, RandLoops::REGISTER_PARAM+14, RandLoops::REGISTER_LIGHT+14));
		addChild(createLightParamCentered<VCVLightLatch<TinySimpleLight<RedLight>>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart4)), module, RandLoops::REGISTER_PARAM+15, RandLoops::REGISTER_LIGHT+15));
		*/
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRst, yRst)), module, RandLoops::RST_INPUT));
		
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xTrg, yTrg)), module, RandLoops::TRIG_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yOut)), module, RandLoops::OUT_OUTPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xProg, yProgKnob)), module, RandLoops::PROG_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xProg, yProgIn)), module, RandLoops::PROG_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xProg, ySet)), module, RandLoops::SET_PARAM, RandLoops::SET_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xProg, yAuto)), module, RandLoops::AUTO_PARAM, RandLoops::AUTO_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xProg, yRecall)), module, RandLoops::RECALL_PARAM, RandLoops::RECALL_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xProg, yRecallIn)), module, RandLoops::RECALL_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xProg, yStore)), module, RandLoops::STORE_PARAM, RandLoops::STORE_LIGHT));

	}

	
	void appendContextMenu(Menu* menu) override {
		RandLoops* module = dynamic_cast<RandLoops*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Buffered Del/Add", "", &module->bufferedAddDel));
		menu->addChild(createBoolPtrMenuItem("Buffered Random", "", &module->bufferedRandom));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Math Settings"));

		struct BitResTypeItem : MenuItem {
			RandLoops* module;
			int bitResType;
			void onAction(const event::Action& e) override {
				module->bitResolution = bitResType;
			}
		};
		std::string BitResTypeNames[2] = {"8 bit", "16 bit"};

		menu->addChild(createSubmenuItem("Bit Resolution", (BitResTypeNames[module->bitResolution]), [=](Menu * menu) {
			for (int i = 0; i < 2; i++) {
				BitResTypeItem* bitResTypeItem = createMenuItem<BitResTypeItem>(BitResTypeNames[i]);
				bitResTypeItem->rightText = CHECKMARK(module->bitResolution == i);
				bitResTypeItem->module = module;
				bitResTypeItem->bitResType = i;
				menu->addChild(bitResTypeItem);
			}
		}));

		/*
		menu->addChild(createSubmenuItem("Out Reference", (module->resolutionName[module->bitResolution]), [=](Menu * menu) {
			menu->addChild(createMenuItem("8 bit", "", [=]() {module->bitResolution = BIT_8;}));
			menu->addChild(createMenuItem("16 bit", "", [=]() {module->bitResolution = BIT_16;}));
		}));
		*/

		struct ProgressionTypeItem : MenuItem {
			RandLoops* module;
			int progressionType;
			void onAction(const event::Action& e) override {
				module->progression = progressionType;
			}
		};
		std::string ProgressionTypeNames[3] = {"2x (std.)", "1.3x", "Fibonacci"};

		menu->addChild(createSubmenuItem("Voltage progression", (ProgressionTypeNames[module->progression]), [=](Menu * menu) {
			for (int i = 0; i < 3; i++) {
				ProgressionTypeItem* progressionTypeItem = createMenuItem<ProgressionTypeItem>(ProgressionTypeNames[i]);
				progressionTypeItem->rightText = CHECKMARK(module->progression == i);
				progressionTypeItem->module = module;
				progressionTypeItem->progressionType = i;
				menu->addChild(progressionTypeItem);
			}
		}));

		struct OutTypeItem : MenuItem {
			RandLoops* module;
			int outType;
			void onAction(const event::Action& e) override {
				module->outType = outType;
			}
		};
		std::string OutTypeNames[3] = {"Trig", "Gate", "Clock Width"};

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Trig Output type", (OutTypeNames[module->outType]), [=](Menu * menu) {
			for (int i = 0; i < 3; i++) {
				OutTypeItem* outTypeItem = createMenuItem<OutTypeItem>(OutTypeNames[i]);
				outTypeItem->rightText = CHECKMARK(module->outType == i);
				outTypeItem->module = module;
				outTypeItem->outType = i;
				menu->addChild(outTypeItem);
			}
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("1st clock after reset:"));
		menu->addChild(createBoolPtrMenuItem("Don't advance", "", &module->dontAdvanceSetting));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Load PROG preset", "", [=]() {module->menuLoadPreset();}));
		menu->addChild(createMenuItem("Save PROG preset", "", [=]() {module->menuSavePreset();}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Erase ALL progs", "", [=](Menu * menu) {
			menu->addChild(createSubmenuItem("Are you Sure?", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("ERASE!", "", [=]() {module->eraseProgs();}));
			}));
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Import Single Sequence", "", [=]() {module->menuLoadSequence();}));
		menu->addChild(createMenuItem("Export Single Sequence", "", [=]() {module->menuSaveSequence();}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Hints", "", [=](Menu * menu) {
			menu->addChild(createMenuLabel("Store Programs with double-click"));
		}));
	}
	

};

Model* modelRandLoops = createModel<RandLoops, RandLoopsWidget>("RandLoops");