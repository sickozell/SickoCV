//#define BEAT 0
//#define BAR 1

#define PPQN1 0
#define PPQN2 1
#define PPQN4 2
#define PPQN8 3
#define PPQN12 4
#define PPQN16 5
#define PPQN24 6
#define REGISTER_SIZE 168

#define NO_SMOOTH 0
#define LOW_SMOOTH 1
#define MEDIUM_SMOOTH 2
#define HIGH_SMOOTH 3

#include "plugin.hpp"
//#include "osdialog.h"
//#define DR_WAV_IMPLEMENTATION
//#include "dr_wav.h"
//#include <vector>
//#include "cmath"
//#include <dirent.h>
//#include <libgen.h>
//#include <sys/types.h>
//#include <sys/stat.h>

using namespace std;

struct tpDivMult : ParamQuantity {
	std::string getDisplayValueString() override {
		const std::string valueDisplay[45] = {"/256", "/128", "/64", "/48", "/32", "/24", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "x1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x24", "x32", "x48", "x64", "x128", "x256"};
		return valueDisplay[int(getValue())];
	}
};

struct Clocker2 : Module {
	enum ParamIds {
		BPM_KNOB_PARAM,
		PW_KNOB_PARAM,
		RUN_BUT_PARAM,
		RESET_BUT_PARAM,
		ENUMS(DIVMULT_KNOB_PARAM, 6),
		ENUMS(DIVPW_KNOB_PARAM, 6),
		NUM_PARAMS 
	};
	enum InputIds {
		EXTCLOCK_INPUT,
		RESET_INPUT,
		RUN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CLOCK_OUTPUT,
		RESET_OUTPUT,
		ENUMS(DIVMULT_OUTPUT, 6),
		NUM_OUTPUTS
	};
	enum LightIds {
		RUN_BUT_LIGHT,
		RESET_BUT_LIGHT,
		ENUMS(DIVSWING_LIGHT, 6),
		NUM_LIGHTS
	};

	//**************************************************************
	//  DEBUG 

	/*	
	std::string debugDisplay = "X";
	std::string debugDisplay2 = "X";
	std::string debugDisplay3 = "X";
	std::string debugDisplay4 = "X";
	std::string debugDisplay5 = "X";
	std::string debugDisplay6 = "X";
	std::string debugDisplay7 = "X";
	int debugInt = 0;
	bool debugBool = false;
	*/

 	//**************************************************************
	//   

	const std::string divMultDisplay[45] = {"/256", "/128", "/64", "/48", "/32", "/24", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "x1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x24", "x32", "x48", "x64", "x128", "x256"};

	const float divMult[45] = {256, 128, 64, 48, 32, 24, 17, 16, 15, 14, 13, 12, 11, 10, 9 , 8, 7, 6, 5, 4, 3, 2, 1,
							2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 24, 32, 48, 64, 128, 256};

	//**************************************************************
	//  	

	//unsigned int sampleRate[2];

	double sampleRateCoeff = (double)APP->engine->getSampleRate() * 60;

	double bpm = 0.1;

	std::string storedPath[2] = {"",""};
	std::string fileDescription[2] = {"--none--","--none--"};

	float extTrigValue = 0.f;
	float prevExtTrigValue = 0.f;

	//int click_setting;
	
	float resetBut = 0;
	float resetValue = 0;
	float prevResetValue = 0;
	bool resetStart = true;

	int runSetting = 1;
	int prevRunSetting = 0;

	float runButton = 0;
	float runTrig = 0.f;
	float prevRunTrig = 0.f;

	double clockSample = 1.0;
	double clockMaxSample = 0.0;

	/*
	double extClockSample = 1.0;
	double prevExtClockSample = 1.0;
	double extClockMaxSample = 0.0;
	double lastGoodAvg = 1.0;
	*/
	

	double maxPulseSample = 0.0;

	int ppqnTable[7] = {1, 2, 4, 8, 12, 16, 24}; // 1 2 4 8 12 16 24
	int ppqn = PPQN1;
	int tempPpqn = ppqn;
	bool ppqnChange = false;

	int ppqnValue = ppqnTable[ppqn];
	//int ppqnValue2 = ppqnValue + 1;
	int ppqnComparison = ppqnValue - 1;

	int pulseNr = 0;
	int pulseSample = 0;

	//double avgPulse = 0.0;
	//double avgPulse = 999999999.0;

	/*
	// **************************** experimental: EXTERNAL CLOCK AUTO STOP
	bool extClockPaused = false;
	int extStop = 0;
	int extStopTable[4] = {0, 7, 5, 3};
	int extStopValue = 0;
	*/

	/* 
	//experimental: EXTERNAL CLOCK SMOOTHING
	int smooth = NO_SMOOTH;
	int tempSmooth = NO_SMOOTH;
	bool smoothChange = false;

	int smoothTable[7][4] = {{1, 2, 4, 6},
							{1, 2, 6, 14},
							{1, 4, 12, 28},
							{1, 8, 24, 56},
							{1, 12, 36, 84},
							{1, 16, 48, 112},
							{1, 24, 72, 168}};
	
	int registerValue[REGISTER_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0};

	int registerSum = 0;

	int registerWidth = smoothTable[ppqn][HIGH_SMOOTH];
	int registerPos = 0;
	bool firstRegSync = true;
	*/

	float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	//float oneMsTime = (APP->engine->getSampleRate()) / 10;	// for testing purposes
	bool resetPulse = false;
	float resetPulseTime = 0.f;

	bool resetOnRun = true;
	bool resetPulseOnRun = false;
	bool resetOnStop = false;
	bool resetPulseOnStop = false;

	bool extSync = false;
	bool extConn = false;
	bool prevExtConn = true;
	bool extBeat = false;
	
	double divClockSample[6] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
	double divMaxSample[6][2] = {{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}};
	int divOddCounter[6] = {0, 0, 0, 0, 0, 0};
	bool divPulse[6] = {false, false, false, false, false, false};
	float divPulseTime[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	int divCount[6] = {1, 1, 1, 1, 1, 1};

	bool divSwing[6] = {false, false, false, false, false, false};

	Clocker2() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		configInput(EXTCLOCK_INPUT,"External Clock");

		configInput(RESET_INPUT,"Reset");
		configSwitch(RESET_BUT_PARAM, 0.f, 1.f, 0.f, "Reset", {"OFF", "ON"});
		configOutput(RESET_OUTPUT,"Reset");

		configSwitch(RUN_BUT_PARAM, 0.f, 1.f, 1.f, "Run", {"OFF", "ON"});
		configInput(RUN_INPUT,"Run");

		configParam(BPM_KNOB_PARAM, 300.f, 3000.f, 1200.f, "Tempo", " bpm", 0, 0.1);
		paramQuantities[BPM_KNOB_PARAM]->snapEnabled = true;

		configParam(PW_KNOB_PARAM, 0.f, 1.0f, 0.5f, "PW Level", "%", 0, 100);

		configParam<tpDivMult>(DIVMULT_KNOB_PARAM+0, 0.f, 44.f, 22.f, "Mult/Div #1");
		paramQuantities[DIVMULT_KNOB_PARAM+0]->snapEnabled = true;
		configParam(DIVPW_KNOB_PARAM+0, 0.f, 1.0f, 0.5f, "PW/Swing Level", "%", 0, 100);

		configParam<tpDivMult>(DIVMULT_KNOB_PARAM+1, 0.f, 44.f, 22.f, "Mult/Div #2");
		paramQuantities[DIVMULT_KNOB_PARAM+1]->snapEnabled = true;
		configParam(DIVPW_KNOB_PARAM+1, 0.f, 1.0f, 0.5f, "PW/Swing Level", "%", 0, 100);

		configParam<tpDivMult>(DIVMULT_KNOB_PARAM+2, 0.f, 44.f, 22.f, "Mult/Div #3");
		paramQuantities[DIVMULT_KNOB_PARAM+2]->snapEnabled = true;
		configParam(DIVPW_KNOB_PARAM+2, 0.f, 1.0f, 0.5f, "PW/Swing Level", "%", 0, 100);

		configParam<tpDivMult>(DIVMULT_KNOB_PARAM+3, 0.f, 44.f, 22.f, "Mult/Div #4");
		paramQuantities[DIVMULT_KNOB_PARAM+3]->snapEnabled = true;
		configParam(DIVPW_KNOB_PARAM+3, 0.f, 1.0f, 0.5f, "PW/Swing Level", "%", 0, 100);

		configParam<tpDivMult>(DIVMULT_KNOB_PARAM+4, 0.f, 44.f, 22.f, "Mult/Div #5");
		paramQuantities[DIVMULT_KNOB_PARAM+4]->snapEnabled = true;
		configParam(DIVPW_KNOB_PARAM+4, 0.f, 1.0f, 0.5f, "PW/Swing Level", "%", 0, 100);

		configParam<tpDivMult>(DIVMULT_KNOB_PARAM+5, 0.f, 44.f, 22.f, "Mult/Div #6");
		paramQuantities[DIVMULT_KNOB_PARAM+5]->snapEnabled = true;
		configParam(DIVPW_KNOB_PARAM+5, 0.f, 1.0f, 0.5f, "PW/Swing Level", "%", 0, 100);

		configOutput(DIVMULT_OUTPUT+0,"Div/Mult #1");
		configOutput(DIVMULT_OUTPUT+1,"Div/Mult #2");
		configOutput(DIVMULT_OUTPUT+2,"Div/Mult #3");
		configOutput(DIVMULT_OUTPUT+3,"Div/Mult #4");
		configOutput(DIVMULT_OUTPUT+4,"Div/Mult #5");
		configOutput(DIVMULT_OUTPUT+5,"Div/Mult #6");

		configOutput(CLOCK_OUTPUT,"Clock");

	}

	void onReset(const ResetEvent &e) override {
		resetStart = true;
		extSync = false;
		extConn = false;
		prevExtConn = true;
		extBeat = false;

		runSetting = 1;
		prevRunSetting = 0;

		runButton = 0;
		runTrig = 0.f;
		prevRunTrig = 0.f;

		clockSample = 1.0;

		clockSample = 1.0;
		resetOnRun = true;
		resetPulseOnRun = false;
		resetOnStop = false;
		resetPulseOnStop = false;

		bpm = 0.1;

		for (int d = 0; d < 6; d++) {
			divClockSample[d] = 1.0;
			divMaxSample[d][0] = 0.0;
			divMaxSample[d][1] = 0.0;
			divPulse[d] = false;
			divPulseTime[d] = false;
			divCount[d] = 1;
			divSwing[d] = false;
		}

		/*
		for (int i = 0; i < REGISTER_SIZE; i++)
			registerValue[i] = 0;
		*/
		
		Module::onReset(e);
	}

	void onSampleRateChange() override {
		sampleRateCoeff = (double)APP->engine->getSampleRate() * 60;
		oneMsTime = (APP->engine->getSampleRate()) / 1000;
		//oneMsTime = (APP->engine->getSampleRate()) / 10; // for testing purposes

	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "ppqn", json_integer(ppqn));
		//json_object_set_new(rootJ, "smooth", json_integer(smooth));		// experimental: EXTERNAL CLOCK SMOOTH
		//json_object_set_new(rootJ, "extStop", json_integer(extStop));		// expertimental: EXTERNAL CLOCK AUTO STOP
		json_object_set_new(rootJ, "ResetOnRun", json_boolean(resetOnRun));
		json_object_set_new(rootJ, "ResetPulseOnRun", json_boolean(resetPulseOnRun));
		json_object_set_new(rootJ, "ResetOnStop", json_boolean(resetOnStop));
		json_object_set_new(rootJ, "ResetPulseOnStop", json_boolean(resetPulseOnStop));
		json_object_set_new(rootJ, "Swing1", json_boolean(divSwing[0]));
		json_object_set_new(rootJ, "Swing2", json_boolean(divSwing[1]));
		json_object_set_new(rootJ, "Swing3", json_boolean(divSwing[2]));
		json_object_set_new(rootJ, "Swing4", json_boolean(divSwing[3]));
		json_object_set_new(rootJ, "Swing5", json_boolean(divSwing[4]));
		json_object_set_new(rootJ, "Swing6", json_boolean(divSwing[5]));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t* ppqnJ = json_object_get(rootJ, "ppqn");
		if (ppqnJ) {
			tempPpqn = json_integer_value(ppqnJ);
			if (tempPpqn < 0 || tempPpqn > 6)
				tempPpqn = 0;
			if (tempPpqn != ppqn)
				changePpqnSetting();
		}

		/*
		// expertimental: EXTERNAL CLOCK SMOOTH

		json_t* smoothJ = json_object_get(rootJ, "smooth");
		if (smoothJ) {
			tempSmooth = json_integer_value(smoothJ);
			if (tempSmooth < 0 || tempSmooth > 3)
				tempSmooth = 0;
			if (tempSmooth != smooth)
				changeSmoothSetting();
		}
		*/

		/*
		// expertimental: EXTERNAL CLOCK AUTO STOP
		json_t* extStopJ = json_object_get(rootJ, "extStop");
		if (extStopJ) {
			extStop = json_integer_value(extStopJ);
			if (extStop < 0 || extStop > 3)
				extStop = 0;
			extStopValue = extStopTable[extStop];
		}
		*/

		json_t* resetOnRunJ = json_object_get(rootJ, "ResetOnRun");
		if (resetOnRunJ)
			resetOnRun = json_boolean_value(resetOnRunJ);
		json_t* resetPulseOnRunJ = json_object_get(rootJ, "ResetPulseOnRun");
		if (resetPulseOnRunJ)
			resetPulseOnRun = json_boolean_value(resetPulseOnRunJ);

		json_t* resetOnStopJ = json_object_get(rootJ, "ResetOnStop");
		if (resetOnStopJ)
			resetOnStop = json_boolean_value(resetOnStopJ);
		json_t* resetPulseOnStopJ = json_object_get(rootJ, "ResetPulseOnStop");
		if (resetPulseOnStopJ)
			resetPulseOnStop = json_boolean_value(resetPulseOnStopJ);

		json_t* swing1J = json_object_get(rootJ, "Swing1");
		if (swing1J)
			divSwing[0] = json_boolean_value(swing1J);
		json_t* swing2J = json_object_get(rootJ, "Swing2");
		if (swing2J)
			divSwing[1] = json_boolean_value(swing2J);
		json_t* swing3J = json_object_get(rootJ, "Swing3");
		if (swing3J)
			divSwing[2] = json_boolean_value(swing3J);
		json_t* swing4J = json_object_get(rootJ, "Swing4");
		if (swing4J)
			divSwing[3] = json_boolean_value(swing4J);
		json_t* swing5J = json_object_get(rootJ, "Swing5");
		if (swing5J)
			divSwing[4] = json_boolean_value(swing5J);
		json_t* swing6J = json_object_get(rootJ, "Swing6");
		if (swing6J)
			divSwing[5] = json_boolean_value(swing6J);

	}

	void changePpqnSetting() {
		ppqnChange = false;
		ppqn = tempPpqn;
		ppqnValue = ppqnTable[ppqn];
		//ppqnValue2 = ppqnValue + 1;
		ppqnComparison = ppqnValue - 1;
		pulseNr = 0;
		extSync = false;

		if (extConn)
			clockSample = 1.0;

		/*
		registerSum = 0;
		registerPos = 0;
		
		extClockSample = 1.0;
		prevExtClockSample = 1.0;
		firstRegSync = true;

		pulseSample = 0;
		registerWidth = smoothTable[ppqn][smooth];
		*/
		
	}

	/*
	void changeSmoothSetting() {
		smoothChange = false;
		smooth = tempSmooth;
		registerSum = 0;
		registerPos = 0;
		pulseNr = 0;
		pulseSample = 0;
		extClockSample = 1.0;
		prevExtClockSample = 1.0;
		firstRegSync = true;
		extSync = false;

		registerWidth = smoothTable[ppqn][smooth];
	}
	*/

	void process(const ProcessArgs &args) override {
	
		lights[DIVSWING_LIGHT+0].setBrightness(divSwing[0]);
		lights[DIVSWING_LIGHT+1].setBrightness(divSwing[1]);
		lights[DIVSWING_LIGHT+2].setBrightness(divSwing[2]);
		lights[DIVSWING_LIGHT+3].setBrightness(divSwing[3]);
		lights[DIVSWING_LIGHT+4].setBrightness(divSwing[4]);
		lights[DIVSWING_LIGHT+5].setBrightness(divSwing[5]);

		// ********* ppqn and smoth changes
		
		if (ppqnChange)
			changePpqnSetting();

		/*
		if (smoothChange)
			changeSmoothSetting();
		*/

		// ********* EXTERNAL CONNECTION

		extConn = inputs[EXTCLOCK_INPUT].isConnected();
		if (extConn && !prevExtConn) {
			extSync = false;
			bpm = 0.0;
			pulseNr = 0;
			/*
			firstRegSync = true;
			registerSum = 0;
			registerPos = 0;
			
			pulseSample = 0;
			extClockSample = 1.0;
			prevExtClockSample = 1.0;
			*/
		}
		prevExtConn = extConn;

		// ********* RUN SETTING

		runTrig = inputs[RUN_INPUT].getVoltage();
		if (runTrig > 1 && prevRunTrig <=1) {
			if (runSetting) {
				runSetting = 0;
				params[RUN_BUT_PARAM].setValue(0);
			} else {
				runSetting = 1;
				params[RUN_BUT_PARAM].setValue(1);
			}
		}
		prevRunTrig = runTrig;

		runSetting = params[RUN_BUT_PARAM].getValue();
		lights[RUN_BUT_LIGHT].setBrightness(runSetting);

		if (!runSetting && prevRunSetting) {
			runSetting = 0;
			if (resetOnStop) {
				resetStart = true;
				if (!extConn)
					clockSample = 1.0;
				outputs[CLOCK_OUTPUT].setVoltage(0.f);
				for (int d = 0; d < 6; d++) {
					divPulse[d] = false;
					divClockSample[d] = 1.0;
					divMaxSample[d][0] = 0.0;
					divMaxSample[d][1] = 0.0;
					outputs[DIVMULT_OUTPUT+d].setVoltage(0.f);
				}
			}
			if (resetPulseOnStop) {
				resetPulse = true;
				resetPulseTime = oneMsTime;
			}
		} else if (runSetting && !prevRunSetting) {
			runSetting = 1;
			if (resetOnRun) {
				resetStart = true;
				if (!extConn)
					clockSample = 1.0;
				else
					extSync = false;
				outputs[CLOCK_OUTPUT].setVoltage(0.f);
				for (int d = 0; d < 6; d++) {
					divPulse[d] = false;
					divClockSample[d] = 1.0;
					divMaxSample[d][0] = 0.0;
					divMaxSample[d][1] = 0.0;
					outputs[DIVMULT_OUTPUT+d].setVoltage(0.f);
				}
			}
			if (resetPulseOnRun) {
				resetPulse = true;
				resetPulseTime = oneMsTime;
			}
		}

		prevRunSetting = runSetting;

		// ---------------------------------------------------------------------------------

		// **********  RESET

		resetBut = params[RESET_BUT_PARAM].getValue();
		if (resetBut)
			resetValue = 1;
		else
			resetValue = inputs[RESET_INPUT].getVoltage();

		lights[RESET_BUT_LIGHT].setBrightness(resetValue);

		if (resetValue >= 1 && prevResetValue < 1) {

			outputs[CLOCK_OUTPUT].setVoltage(0.f);
			for (int d = 0; d < 6; d++) {
				divPulse[d] = false;
				divClockSample[d] = 1.0;
				divMaxSample[d][0] = 0.0;
				divMaxSample[d][1] = 0.0;
				outputs[DIVMULT_OUTPUT+d].setVoltage(0.f);
			}

			resetPulse = true;
			resetPulseTime = oneMsTime;
		}
		prevResetValue = resetValue;


		// ************************* INTERNAL CLOCK

		if (!extConn) {

			// *********** BPM CALC
			
			bpm = (double)params[BPM_KNOB_PARAM].getValue()/10;

			// **************   RUN PROCESS   ***************

			if (runSetting && resetPulseTime <= 0) {

				// ****** CLOCK PULSE WIDTH

				maxPulseSample = clockMaxSample * (double)params[PW_KNOB_PARAM].getValue();
				if (clockSample > maxPulseSample)
					outputs[CLOCK_OUTPUT].setVoltage(0.f);

				// ************ DIV / MULT

				for (int d = 0; d < 6; d++) {

					if (!divSwing[d]) {

						//if (params[DIVMULT_KNOB_PARAM+d].getValue() > 21 && divClockSample[d] > divMaxSample[d][0]) {
						if (params[DIVMULT_KNOB_PARAM+d].getValue() > 22 && divClockSample[d] > divMaxSample[d][0]) {
							// ***** CLOCK MULTIPLIER *****
							divClockSample[d] = 1.0;
							divPulse[d] = true;
							outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
						}

						// ***** CLOCK MULTIPLIER/DIVIDER   PULSE WIDTH OFF
						if (divPulse[d] && divClockSample[d] > divMaxSample[d][0] * params[DIVPW_KNOB_PARAM+d].getValue()) {
							divPulse[d] = false;
							outputs[DIVMULT_OUTPUT+d].setVoltage(0.f);
						}
						
					} else {

						//if (params[DIVMULT_KNOB_PARAM+d].getValue() > 21 && divClockSample[d] > divMaxSample[d][divOddCounter[d]]) {
						if (params[DIVMULT_KNOB_PARAM+d].getValue() > 22 && divClockSample[d] > divMaxSample[d][divOddCounter[d]]) {
							// ***** CLOCK MULTIPLIER *****
							divPulse[d] = true;
							divPulseTime[d] = oneMsTime;
							outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
	
							if (divOddCounter[d] == 0) {
								divClockSample[d] = 1.0;
								divOddCounter[d] = 1;
							} else {
								divClockSample[d] = 1.0 + divMaxSample[d][1] - divMaxSample[d][0];
								divOddCounter[d] = 0;
							}
						}
					}
				}

				//	*************************  INTERNAL CLOCK  ******************

				clockMaxSample = sampleRateCoeff / bpm;
				
				if (clockSample > clockMaxSample || resetStart)  {

					if (resetStart) {
						clockSample = 1.0;
						resetStart = false;
					} else
						clockSample -= clockMaxSample;
					
					for (int d = 0; d < 6; d++) {
						
						if (!divSwing[d]) {

							//if (params[DIVMULT_KNOB_PARAM+d].getValue() > 21) {
							if (params[DIVMULT_KNOB_PARAM+d].getValue() > 22) {
								// ***** CLOCK MULTIPLIER *****
								divMaxSample[d][0] = clockMaxSample / (divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]);
								divMaxSample[d][1] = divMaxSample[d][0];
								divClockSample[d] = 1.0;
								divPulse[d] = true;
								outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
							} else {
								// ***** CLOCK DIVIDER *****
								divMaxSample[d][0] = clockMaxSample * (divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]);
								divMaxSample[d][1] = divMaxSample[d][0];
								divCount[d]++;
								if (divCount[d] > divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]) {
									divClockSample[d] = 1.0;
									divCount[d] = 1;
									divPulse[d] = true;
									outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
								}
							}
						} else {
							
							//if (params[DIVMULT_KNOB_PARAM+d].getValue() > 21) {
							if (params[DIVMULT_KNOB_PARAM+d].getValue() > 22) {
								// ***** CLOCK MULTIPLIER *****
								divMaxSample[d][0] = clockMaxSample / (divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]);
								divMaxSample[d][1] = divMaxSample[d][0] + (divMaxSample[d][0] * params[DIVPW_KNOB_PARAM+d].getValue());
								divOddCounter[d] = 1;
								divClockSample[d] = 1.0;
								divPulse[d] = true;
								divPulseTime[d] = oneMsTime;
								outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
							} else {
								// ***** CLOCK DIVIDER *****
								divMaxSample[d][0] = clockMaxSample * (divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]);
								divMaxSample[d][1] = divMaxSample[d][0];
								divCount[d]++;
								if (divCount[d] > divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]) {
									divOddCounter[d] = 1;

									divClockSample[d] = 1.0;
									divCount[d] = 1;
									divPulse[d] = true;
									divPulseTime[d] = oneMsTime;
									outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
								}
							}
						}
					}
					
					outputs[CLOCK_OUTPUT].setVoltage(10.f);
					
				}

				clockSample++;
				divClockSample[0]++;
				divClockSample[1]++;
				divClockSample[2]++;
				divClockSample[3]++;
				divClockSample[4]++;
				divClockSample[5]++;
				
			}

		} else {

			// ************************************************ EXTERNAL CLOCK

			// ********** EXTERNAL SYNC
			
			extTrigValue = inputs[EXTCLOCK_INPUT].getVoltage();
				
			if (extTrigValue >= 1 && prevExtTrigValue < 1) {

				pulseNr++;
				if (extSync) {

					/*
					if (pulseNr > ppqnComparison) {
						pulseNr = 0;
						clockMaxSample = clockSample;
						midBeatMaxSample = clockMaxSample / 2;
						clockSample = 0.0;
						
						if (runSetting)
							extBeat = true;

						// calculate bpms
						bpm = round(sampleRateCoeff / clockMaxSample);
						if (bpm > 999)
							bpm = 999;
					}
					*/

					//clockMaxSample = clockSample * (ppqnValue2 - pulseNr);

					if (pulseNr > ppqnComparison) {
						pulseNr = 0;
						clockMaxSample = clockSample;
						clockSample = 0.0;
						
						if (runSetting)
							extBeat = true;

						// calculate bpms
						bpm = round(sampleRateCoeff / clockMaxSample);
						if (bpm > 999)
							bpm = 999;
					}

				} else {
					bpm = 0.0;
					extSync = true;
					clockSample = 1.0;
					pulseNr = 0;

					if (runSetting)
						extBeat = true;
				}

			} else {
				extBeat = false;
			}
			prevExtTrigValue = extTrigValue;

			//extClockSample++;
		

			// **************   RUN PROCESS   ***************

			if (runSetting && resetPulseTime <= 0) {

				// ****** CLOCK PULSE WIDTH

				maxPulseSample = clockMaxSample * (double)params[PW_KNOB_PARAM].getValue();
				if (clockSample > maxPulseSample)
					outputs[CLOCK_OUTPUT].setVoltage(0.f);

				// ************ DIV / MULT

				for (int d = 0; d < 6; d++) {

					if(!divSwing[d]) {

						//if (params[DIVMULT_KNOB_PARAM+d].getValue() > 21 && divClockSample[d] > divMaxSample[d][0]) {
						if (params[DIVMULT_KNOB_PARAM+d].getValue() > 22 && divClockSample[d] > divMaxSample[d][0]) {
							// ***** CLOCK MULTIPLIER *****
							divClockSample[d] = 1.0;
							divPulse[d] = true;
							outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
						}

						// ***** CLOCK MULTIPLIER/DIVIDER   PULSE WIDTH OFF
						if (divPulse[d] && divClockSample[d] > divMaxSample[d][0] * params[DIVPW_KNOB_PARAM+d].getValue()) {
							divPulse[d] = false;
							outputs[DIVMULT_OUTPUT+d].setVoltage(0.f);
						}

					} else {
						//if (params[DIVMULT_KNOB_PARAM+d].getValue() > 21 && divClockSample[d] > divMaxSample[d][divOddCounter[d]]) {
						if (params[DIVMULT_KNOB_PARAM+d].getValue() > 22 && divClockSample[d] > divMaxSample[d][divOddCounter[d]]) {
							// ***** CLOCK MULTIPLIER *****
							divPulse[d] = true;
							divPulseTime[d] = oneMsTime;
							outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
	
							if (divOddCounter[d] == 0) {
								divClockSample[d] = 1.0;
								divOddCounter[d] = 1;
							}	else {
								divClockSample[d] = 1.0 + divMaxSample[d][1] - divMaxSample[d][0];
								divOddCounter[d] = 0;
							}
						}
					}
				}

				// ************************ EXTERNAL CLOCK ******************

				if (extBeat) {

					if (extSync) {

						// ********** SYNCED BEAT

						for (int d = 0; d < 6; d++) {

							if (!divSwing[d]) {
								//if (params[DIVMULT_KNOB_PARAM+d].getValue() > 21) {
								if (params[DIVMULT_KNOB_PARAM+d].getValue() > 22) {
									// ***** CLOCK MULTIPLIER *****
									divMaxSample[d][0] = clockMaxSample / (divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]);
									divMaxSample[d][1] = divMaxSample[d][0];
									divClockSample[d] = 1.0;
									divPulse[d] = true;
									outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
								} else {
									// ***** CLOCK DIVIDER *****
									divMaxSample[d][0] = clockMaxSample * (divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]);
									divMaxSample[d][1] = divMaxSample[d][1];
									divCount[d]++;
									if (divCount[d] > divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]) {
										divClockSample[d] = 1.0;
										divCount[d] = 1;
										divPulse[d] = true;
										outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
									}
								}
							} else {
								//if (params[DIVMULT_KNOB_PARAM+d].getValue() > 21) {
								if (params[DIVMULT_KNOB_PARAM+d].getValue() > 22) {
									// ***** CLOCK MULTIPLIER *****
									divMaxSample[d][0] = clockMaxSample / (divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]);
									divMaxSample[d][1] = divMaxSample[d][0] + (divMaxSample[d][0] * params[DIVPW_KNOB_PARAM+d].getValue());
									divOddCounter[d] = 1;
									divClockSample[d] = 1.0;
									divPulse[d] = true;
									divPulseTime[d] = oneMsTime;
									outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
								} else {
									// ***** CLOCK DIVIDER *****
									divMaxSample[d][0] = clockMaxSample * (divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]);
									divMaxSample[d][1] = divMaxSample[d][0];
									divCount[d]++;
									if (divCount[d] > divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]) {
										divOddCounter[d] = 1;
										divClockSample[d] = 1.0;
										divCount[d] = 1;
										divPulse[d] = true;
										divPulseTime[d] = oneMsTime;
										outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
									}
								}
							}
						}

						outputs[CLOCK_OUTPUT].setVoltage(10.f);
					
					}
				}
			}
				
			clockSample++;
			divClockSample[0]++;
			divClockSample[1]++;
			divClockSample[2]++;
			divClockSample[3]++;
			divClockSample[4]++;
			divClockSample[5]++;			
			
		}

		// ***************************** COMMON PROCESS

		//	********** RESET AND DIV PULSES

		if (resetPulse) {
			resetPulseTime--;
			if (resetPulseTime < 0) {
				resetPulse = false;
				outputs[RESET_OUTPUT].setVoltage(0.f);
			} else
				outputs[RESET_OUTPUT].setVoltage(10.f);
		}

		for (int d = 0; d < 6; d++) {
			if (divSwing[d] && divPulse[d]) {
				divPulseTime[d]--;
				if (divPulseTime[d] < 0) {
					divPulse[d] = false;
					outputs[DIVMULT_OUTPUT+d].setVoltage(0.f);
				} else
					outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
			}
		}
	}
};

struct Clocker2DisplayTempo : TransparentWidget {
	Clocker2 *module;
	int frame = 0;
	Clocker2DisplayTempo() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 13);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(0xdd, 0xdd, 0x33, 0xff)); 
				
				int tempBpmInteger;
				std::string tempBpm;
				std::string tempBpmInt;
				std::string tempBpmDec;

				if (!module->inputs[module->EXTCLOCK_INPUT].isConnected()) {
					tempBpmInteger = int(module->bpm * 10 + .5);

					tempBpm = to_string(tempBpmInteger);
					tempBpmInt = tempBpm.substr(0, tempBpm.size()-1);
					tempBpmDec = tempBpm.substr(tempBpm.size() - 1);
					tempBpm = tempBpmInt+"."+tempBpmDec;

					if (tempBpmInteger < 1000)
						nvgTextBox(args.vg, 14.5, 16.3, 60, tempBpm.c_str(), NULL);
					else
						nvgTextBox(args.vg, 4, 16.3, 60, tempBpm.c_str(), NULL);
				} else {
					
					tempBpmInteger = int(module->bpm);
					tempBpm = to_string(tempBpmInteger)+".X";
					if (tempBpmInteger < 100)
						nvgTextBox(args.vg, 14.5, 16.3, 60, tempBpm.c_str(), NULL);
					else
						nvgTextBox(args.vg, 4, 16.3, 60, tempBpm.c_str(), NULL);
					
				}
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct Clocker2DisplayDiv1 : TransparentWidget {
	Clocker2 *module;
	int frame = 0;
	Clocker2DisplayDiv1() {
	}

	void onButton(const event::Button &e) override {

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 12);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				int tempValue = int(module->params[module->DIVMULT_KNOB_PARAM+0].getValue());
				float tempXpos = 3;
				if (tempValue > 13 && tempValue < 31)
					tempXpos = 12.8;

				if (tempValue < 22)
					nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
				else
					nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff)); 

				nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		Clocker2 *module = dynamic_cast<Clocker2 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker2* module;
				int valueNr;
				void onAction(const event::Action& e) override {
					module->params[module->DIVMULT_KNOB_PARAM+0].setValue(float(valueNr));
				}
			};

			const std::string menuNames[45] = {"/256", "/128", "/64", "/48", "/32", "/24", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "x1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x24", "x32", "x48", "x64", "x128", "x256"};
			for (int i = 0; i < 45; i++) {
				ThisItem* thisItem = createMenuItem<ThisItem>(menuNames[i]);
				thisItem->rightText = CHECKMARK(int(module->params[module->DIVMULT_KNOB_PARAM+0].getValue()) == i);
				thisItem->module = module;
				thisItem->valueNr = i;
				menu->addChild(thisItem);
			}
		}
	}
};

struct Clocker2DisplayDiv2 : TransparentWidget {
	Clocker2 *module;
	int frame = 0;
	Clocker2DisplayDiv2() {
	}

	void onButton(const event::Button &e) override {

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 12);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				int tempValue = int(module->params[module->DIVMULT_KNOB_PARAM+1].getValue());
				float tempXpos = 3;
				if (tempValue > 13 && tempValue < 31)
					tempXpos = 12.8;

				if (tempValue < 22)
					nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
				else
					nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff)); 

				nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		Clocker2 *module = dynamic_cast<Clocker2 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker2* module;
				int valueNr;
				void onAction(const event::Action& e) override {
					module->params[module->DIVMULT_KNOB_PARAM+1].setValue(float(valueNr));
				}
			};

			const std::string menuNames[45] = {"/256", "/128", "/64", "/48", "/32", "/24", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "x1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x24", "x32", "x48", "x64", "x128", "x256"};
			for (int i = 0; i < 45; i++) {
				ThisItem* thisItem = createMenuItem<ThisItem>(menuNames[i]);
				thisItem->rightText = CHECKMARK(int(module->params[module->DIVMULT_KNOB_PARAM+1].getValue()) == i);
				thisItem->module = module;
				thisItem->valueNr = i;
				menu->addChild(thisItem);
			}
		}
	}
};

struct Clocker2DisplayDiv3 : TransparentWidget {
	Clocker2 *module;
	int frame = 0;
	Clocker2DisplayDiv3() {
	}

	void onButton(const event::Button &e) override {

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 12);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				int tempValue = int(module->params[module->DIVMULT_KNOB_PARAM+2].getValue());
				float tempXpos = 3;
				if (tempValue > 13 && tempValue < 31)
					tempXpos = 12.8;

				if (tempValue < 22)
					nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
				else
					nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff)); 

				nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		Clocker2 *module = dynamic_cast<Clocker2 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker2* module;
				int valueNr;
				void onAction(const event::Action& e) override {
					module->params[module->DIVMULT_KNOB_PARAM+2].setValue(float(valueNr));
				}
			};

			const std::string menuNames[45] = {"/256", "/128", "/64", "/48", "/32", "/24", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "x1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x24", "x32", "x48", "x64", "x128", "x256"};
			for (int i = 0; i < 45; i++) {
				ThisItem* thisItem = createMenuItem<ThisItem>(menuNames[i]);
				thisItem->rightText = CHECKMARK(int(module->params[module->DIVMULT_KNOB_PARAM+2].getValue()) == i);
				thisItem->module = module;
				thisItem->valueNr = i;
				menu->addChild(thisItem);
			}
		}
	}
};

struct Clocker2DisplayDiv4 : TransparentWidget {
	Clocker2 *module;
	int frame = 0;
	Clocker2DisplayDiv4() {
	}

	void onButton(const event::Button &e) override {

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 12);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				int tempValue = int(module->params[module->DIVMULT_KNOB_PARAM+3].getValue());
				float tempXpos = 3;
				if (tempValue > 13 && tempValue < 31)
					tempXpos = 12.8;

				if (tempValue < 22)
					nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
				else
					nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff)); 

				nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		Clocker2 *module = dynamic_cast<Clocker2 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker2* module;
				int valueNr;
				void onAction(const event::Action& e) override {
					module->params[module->DIVMULT_KNOB_PARAM+3].setValue(float(valueNr));
				}
			};

			const std::string menuNames[45] = {"/256", "/128", "/64", "/48", "/32", "/24", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "x1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x24", "x32", "x48", "x64", "x128", "x256"};
			for (int i = 0; i < 45; i++) {
				ThisItem* thisItem = createMenuItem<ThisItem>(menuNames[i]);
				thisItem->rightText = CHECKMARK(int(module->params[module->DIVMULT_KNOB_PARAM+3].getValue()) == i);
				thisItem->module = module;
				thisItem->valueNr = i;
				menu->addChild(thisItem);
			}
		}
	}
};

struct Clocker2DisplayDiv5 : TransparentWidget {
	Clocker2 *module;
	int frame = 0;
	Clocker2DisplayDiv5() {
	}

	void onButton(const event::Button &e) override {

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 12);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				int tempValue = int(module->params[module->DIVMULT_KNOB_PARAM+4].getValue());
				float tempXpos = 3;
				if (tempValue > 13 && tempValue < 31)
					tempXpos = 12.8;

				if (tempValue < 22)
					nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
				else
					nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff)); 

				nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		Clocker2 *module = dynamic_cast<Clocker2 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker2* module;
				int valueNr;
				void onAction(const event::Action& e) override {
					module->params[module->DIVMULT_KNOB_PARAM+4].setValue(float(valueNr));
				}
			};

			const std::string menuNames[45] = {"/256", "/128", "/64", "/48", "/32", "/24", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "x1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x24", "x32", "x48", "x64", "x128", "x256"};
			for (int i = 0; i < 45; i++) {
				ThisItem* thisItem = createMenuItem<ThisItem>(menuNames[i]);
				thisItem->rightText = CHECKMARK(int(module->params[module->DIVMULT_KNOB_PARAM+4].getValue()) == i);
				thisItem->module = module;
				thisItem->valueNr = i;
				menu->addChild(thisItem);
			}
		}
	}
};

struct Clocker2DisplayDiv6 : TransparentWidget {
	Clocker2 *module;
	int frame = 0;
	Clocker2DisplayDiv6() {
	}

	void onButton(const event::Button &e) override {

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 12);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				int tempValue = int(module->params[module->DIVMULT_KNOB_PARAM+5].getValue());
				float tempXpos = 3;
				if (tempValue > 13 && tempValue < 31)
					tempXpos = 12.8;

				if (tempValue < 22)
					nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
				else
					nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff)); 

				nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		Clocker2 *module = dynamic_cast<Clocker2 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker2* module;
				int valueNr;
				void onAction(const event::Action& e) override {
					module->params[module->DIVMULT_KNOB_PARAM+5].setValue(float(valueNr));
				}
			};

			const std::string menuNames[45] = {"/256", "/128", "/64", "/48", "/32", "/24", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "x1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x24", "x32", "x48", "x64", "x128", "x256"};
			for (int i = 0; i < 45; i++) {
				ThisItem* thisItem = createMenuItem<ThisItem>(menuNames[i]);
				thisItem->rightText = CHECKMARK(int(module->params[module->DIVMULT_KNOB_PARAM+5].getValue()) == i);
				thisItem->module = module;
				thisItem->valueNr = i;
				menu->addChild(thisItem);
			}
		}
	}
};
/*
struct Clocker2DebugDisplay : TransparentWidget {
	Clocker2 *module;
	int frame = 0;
	Clocker2DebugDisplay() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-bold.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff)); 
				

				nvgTextBox(args.vg, 9, 0,120, module->debugDisplay.c_str(), NULL);
				nvgTextBox(args.vg, 9, 10,120, module->debugDisplay2.c_str(), NULL);
				nvgTextBox(args.vg, 9, 20,120, module->debugDisplay3.c_str(), NULL);
				nvgTextBox(args.vg, 9, 30,120, module->debugDisplay4.c_str(), NULL);
				nvgTextBox(args.vg, 9, 40,120, module->debugDisplay5.c_str(), NULL);
				nvgTextBox(args.vg, 9, 50,120, module->debugDisplay6.c_str(), NULL);
				nvgTextBox(args.vg, 9, 60,120, module->debugDisplay7.c_str(), NULL);
				
			}
		}
		Widget::drawLayer(args, layer);
	}
};
*/

struct Clocker2Widget : ModuleWidget {
	Clocker2Widget(Clocker2 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Clocker2.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH))); 

		/*		
		{
			Clocker2DebugDisplay *display = new Clocker2DebugDisplay();
			display->box.pos = Vec(0, 10);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/	

		{
			Clocker2DisplayTempo *display = new Clocker2DisplayTempo();
			display->box.pos = mm2px(Vec(13.222, 17.5));
			display->box.size = mm2px(Vec(16.8, 6.5));
			display->module = module;
			addChild(display);
		}

		{
			Clocker2DisplayDiv1 *display = new Clocker2DisplayDiv1();
			display->box.pos = mm2px(Vec(15.3, 80.2 - 23));
			display->box.size = mm2px(Vec(15, 6.3));
			display->module = module;
			addChild(display);
		}

		{
			Clocker2DisplayDiv2 *display = new Clocker2DisplayDiv2();
			display->box.pos = mm2px(Vec(15.3, 91.2 - 23));
			display->box.size = mm2px(Vec(15, 6.3));
			display->module = module;
			addChild(display);
		}

		{
			Clocker2DisplayDiv3 *display = new Clocker2DisplayDiv3();
			display->box.pos = mm2px(Vec(15.3, 102.2 - 23));
			display->box.size = mm2px(Vec(15, 6.3));
			display->module = module;
			addChild(display);
		}

		{
			Clocker2DisplayDiv4 *display = new Clocker2DisplayDiv4();
			display->box.pos = mm2px(Vec(15.3, 113.2 - 23));
			display->box.size = mm2px(Vec(15, 6.3));
			display->module = module;
			addChild(display);
		}

		{
			Clocker2DisplayDiv5 *display = new Clocker2DisplayDiv5();
			display->box.pos = mm2px(Vec(15.3, 124.2 - 23));
			display->box.size = mm2px(Vec(15, 6.3));
			display->module = module;
			addChild(display);
		}

		{
			Clocker2DisplayDiv6 *display = new Clocker2DisplayDiv6();
			display->box.pos = mm2px(Vec(15.3, 135.2 - 23));
			display->box.size = mm2px(Vec(15, 6.3));
			display->module = module;
			addChild(display);
		}


		const float xExtClock = 7.5f;
		const float xResetIn = 36.f;
		const float xResetBut = 36.f;

		const float xRun = 7.5f;
		const float xBpmKnob = 22.f;
		const float xPwKnob = 36.f;
		const float xDivLg = 40.f;

		const float xDivKnob = 8.7f;
		
		const float xDivOut = 49.1f;

		// -------------------------------

		const float yExtClock = 18.5f;
		const float yRunBut = 33.5f;
		const float yRunIn = 42.5f;
		
		const float yBpmKob = 34.5f;

		const float yRstBut = 17.5f;
		const float yRstIn = 26.5f;

		const float yPwClOut = 43.f;

		const float yDivKn1 = 83.5f - 23;
		const float yDivKn2 = 94.5f - 23;
		const float yDivKn3 = 105.5f - 23;
		const float yDivKn4 = 116.5f - 23;
		const float yDivKn5 = 127.5f - 23;
		const float yDivKn6 = 138.5f - 23;

		const float yDivLg1 = 80.5f - 23;
		const float yDivLg2 = 91.5f - 23;
		const float yDivLg3 = 102.5f - 23;
		const float yDivLg4 = 113.5f - 23;
		const float yDivLg5 = 124.5f - 23;
		const float yDivLg6 = 135.5f - 23;

		const float yClockOut = 19.5f;
		const float yResetOut = 36.f;		

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xExtClock, yExtClock)), module, Clocker2::EXTCLOCK_INPUT));
		
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xResetIn, yRstIn)), module, Clocker2::RESET_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(xResetBut, yRstBut)), module, Clocker2::RESET_BUT_PARAM, Clocker2::RESET_BUT_LIGHT));
		

		addParam(createLightParamCentered<VCVLightBezelLatch<BlueLight>>(mm2px(Vec(xRun, yRunBut)), module, Clocker2::RUN_BUT_PARAM, Clocker2::RUN_BUT_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRun, yRunIn)), module, Clocker2::RUN_INPUT));

		addParam(createParamCentered<SickoBigKnob>(mm2px(Vec(xBpmKnob, yBpmKob)), module, Clocker2::BPM_KNOB_PARAM));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPwKnob, yPwClOut)), module, Clocker2::PW_KNOB_PARAM));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xDivKnob, yDivKn1)), module, Clocker2::DIVMULT_KNOB_PARAM+0));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xDivKnob, yDivKn2)), module, Clocker2::DIVMULT_KNOB_PARAM+1));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xDivKnob, yDivKn3)), module, Clocker2::DIVMULT_KNOB_PARAM+2));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xDivKnob, yDivKn4)), module, Clocker2::DIVMULT_KNOB_PARAM+3));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xDivKnob, yDivKn5)), module, Clocker2::DIVMULT_KNOB_PARAM+4));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xDivKnob, yDivKn6)), module, Clocker2::DIVMULT_KNOB_PARAM+5));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPwKnob, yDivKn1)), module, Clocker2::DIVPW_KNOB_PARAM+0));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPwKnob, yDivKn2)), module, Clocker2::DIVPW_KNOB_PARAM+1));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPwKnob, yDivKn3)), module, Clocker2::DIVPW_KNOB_PARAM+2));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPwKnob, yDivKn4)), module, Clocker2::DIVPW_KNOB_PARAM+3));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPwKnob, yDivKn5)), module, Clocker2::DIVPW_KNOB_PARAM+4));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPwKnob, yDivKn6)), module, Clocker2::DIVPW_KNOB_PARAM+5));

		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(xDivLg, yDivLg1)), module, Clocker2::DIVSWING_LIGHT+0));
		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(xDivLg, yDivLg2)), module, Clocker2::DIVSWING_LIGHT+1));
		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(xDivLg, yDivLg3)), module, Clocker2::DIVSWING_LIGHT+2));
		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(xDivLg, yDivLg4)), module, Clocker2::DIVSWING_LIGHT+3));
		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(xDivLg, yDivLg5)), module, Clocker2::DIVSWING_LIGHT+4));
		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(xDivLg, yDivLg6)), module, Clocker2::DIVSWING_LIGHT+5));
		
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yClockOut)), module, Clocker2::CLOCK_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yResetOut)), module, Clocker2::RESET_OUTPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yDivKn1)), module, Clocker2::DIVMULT_OUTPUT+0));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yDivKn2)), module, Clocker2::DIVMULT_OUTPUT+1));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yDivKn3)), module, Clocker2::DIVMULT_OUTPUT+2));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yDivKn4)), module, Clocker2::DIVMULT_OUTPUT+3));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yDivKn5)), module, Clocker2::DIVMULT_OUTPUT+4));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yDivKn6)), module, Clocker2::DIVMULT_OUTPUT+5));
	}

	void appendContextMenu(Menu *menu) override {
	   	Clocker2 *module = dynamic_cast<Clocker2*>(this->module);
			assert(module);
		
		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Trig/Swing on Div #1", "", &module->divSwing[0]));
		menu->addChild(createBoolPtrMenuItem("Trig/Swing on Div #2", "", &module->divSwing[1]));
		menu->addChild(createBoolPtrMenuItem("Trig/Swing on Div #3", "", &module->divSwing[2]));
		menu->addChild(createBoolPtrMenuItem("Trig/Swing on Div #4", "", &module->divSwing[3]));
		menu->addChild(createBoolPtrMenuItem("Trig/Swing on Div #5", "", &module->divSwing[4]));
		menu->addChild(createBoolPtrMenuItem("Trig/Swing on Div #6", "", &module->divSwing[5]));

		menu->addChild(new MenuSeparator());

		struct PpqnItem : MenuItem {
			Clocker2* module;
			int ppqn;
			void onAction(const event::Action& e) override {
				module->tempPpqn = ppqn;
				module->ppqnChange = true;
			}
		};

		menu->addChild(createMenuLabel("External Clock"));
		std::string ppqnNames[7] = {"1 PPQN", "2 PPQN", "4 PPQN", "8 PPQN", "12 PPQN", "16 PPQN", "24 PPQN"};
		menu->addChild(createSubmenuItem("Resolution", ppqnNames[module->ppqn], [=](Menu * menu) {
			for (int i = 0; i < 7; i++) {
				PpqnItem* ppqnItem = createMenuItem<PpqnItem>(ppqnNames[i]);
				ppqnItem->rightText = CHECKMARK(module->ppqn == i);
				ppqnItem->module = module;
				ppqnItem->ppqn = i;
				menu->addChild(ppqnItem);
			}
		}));

		/*
		// experimental: EXTERNAL CLOCK AUTO STOP
		struct SmoothItem : MenuItem {
			Clocker2* module;
			int smooth;
			void onAction(const event::Action& e) override {
				module->tempSmooth = smooth;
				module->smoothChange = true;
			}
		};

		std::string smoothNames[4] = {"None", "Low", "Medium", "High"};
		menu->addChild(createSubmenuItem("Smoothing", smoothNames[module->smooth], [=](Menu * menu) {
			for (int i = 0; i < 4; i++) {
				SmoothItem* smoothItem = createMenuItem<SmoothItem>(smoothNames[i]);
				smoothItem->rightText = CHECKMARK(module->smooth == i);
				smoothItem->module = module;
				smoothItem->smooth = i;
				menu->addChild(smoothItem);
			}
		}));
		*/

		/*
		// experimental: EXTERNAL CLOCK AUTO STOP
		struct ExtStopItem : MenuItem {
			Clocker2* module;
			int extStop;
			void onAction(const event::Action& e) override {
				module->extStop = extStop;
				module->extStopValue = module->extStopTable[extStop];
			}
		};

		std::string extStopNames[4] = {"Off", "Low", "Medium", "High"};
		menu->addChild(createSubmenuItem("Auto Stop sensitivity", extStopNames[module->extStop], [=](Menu * menu) {
			for (int i = 0; i < 4; i++) {
				ExtStopItem* extStopItem = createMenuItem<ExtStopItem>(extStopNames[i]);
				extStopItem->rightText = CHECKMARK(module->extStop == i);
				extStopItem->module = module;
				extStopItem->extStop = i;
				menu->addChild(extStopItem);
			}
		}));
		*/

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("On Run", "", [=](Menu* menu) {
			menu->addChild(createBoolPtrMenuItem("Reset Bar", "", &module->resetOnRun));
			menu->addChild(createBoolPtrMenuItem("Pulse to RST out", "", &module->resetPulseOnRun));
		}));
		menu->addChild(createSubmenuItem("On Stop", "", [=](Menu* menu) {
			menu->addChild(createBoolPtrMenuItem("Reset Bar", "", &module->resetOnStop));
			menu->addChild(createBoolPtrMenuItem("Pulse to RST out", "", &module->resetPulseOnStop));
		}));
	}
};

Model *modelClocker2 = createModel<Clocker2, Clocker2Widget>("Clocker2");
