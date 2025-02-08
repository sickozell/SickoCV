#define BEAT 0
#define BAR 1

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

#define CLICK_STANDARD 0
#define CLICK_CUSTOM 3

#include "plugin.hpp"
#include "osdialog.h"
#if defined(METAMODULE)
#include "async_filebrowser.hh"
#endif
//#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <vector>
#include "cmath"
#include <dirent.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

struct tpSignature : ParamQuantity {
	std::string getDisplayValueString() override {
		const std::string valueDisplay[17] = {"2/4", "3/4", "4/4", "5/4", "6/4", "7/4", "5/8", "6/8", "7/8", "8/8", "9/8", "10/8", "11/8", "12/8", "13/8", "14/8", "15/8"};
		return valueDisplay[int(getValue())];
	}
};

struct tpDivMult : ParamQuantity {
	std::string getDisplayValueString() override {
		const std::string valueDisplay[45] = {"/256", "/128", "/64", "/48", "/32", "/24", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "x1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x24", "x32", "x48", "x64", "x128", "x256"};
		return valueDisplay[int(getValue())];
	}
};

struct Clocker : Module {
	enum ParamIds {
		BPM_KNOB_PARAM,
		SIGNATURE_KNOB_PARAM,
		CLICK_BUT_PARAM,
		CLICKVOL_KNOB_PARAM,
		PW_KNOB_PARAM,
		RUN_BUT_PARAM,
		RESET_BUT_PARAM,
		ENUMS(DIVMULT_KNOB_PARAM, 4),
		ENUMS(DIVPW_KNOB_PARAM, 4),
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
		ENUMS(DIVMULT_OUTPUT, 4),
		BEATPULSE_OUTPUT,
		BARPULSE_OUTPUT,
		MASTER_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		CLICK_BUT_LIGHT,
		RUN_BUT_LIGHT,
		RESET_BUT_LIGHT,
		ENUMS(DIVSWING_LIGHT, 4),
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

	const std::string signatureDisplay[17] = {"2/4", "3/4", "4/4", "5/4", "6/4", "7/4", "5/8", "6/8", "7/8", "8/8", "9/8", "10/8", "11/8", "12/8", "13/8", "14/8", "15/8"};

	const int beatMaxPerBar[17] = {2, 3, 4, 5, 6, 7, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	int currentBeatMaxPerBar = 4;

	const std::string divMultDisplay[45] = {"/256", "/128", "/64", "/48", "/32", "/24", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "x1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x24", "x32", "x48", "x64", "x128", "x256"};

	const float divMult[45] = {256, 128, 64, 48, 32, 24, 17, 16, 15, 14, 13, 12, 11, 10, 9 , 8, 7, 6, 5, 4, 3, 2, 1,
							2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 24, 32, 48, 64, 128, 256};

	//**************************************************************
	//  	

	unsigned int sampleRate[2];

	double sampleRateCoeff = (double)APP->engine->getSampleRate() * 60;

	double bpm = 0.1;

	drwav_uint64 totalSampleC[2];
	drwav_uint64 totalSamples[2];

	const unsigned int minSamplesToLoad = 9;

	vector<float> playBuffer[2];
	vector<float> tempBuffer;
	vector<float> tempBuffer2;

	bool fileLoaded[2] = {false, false};
	bool fileFound[2] = {false, false};
	bool play[2] = {false, false};
	double samplePos[2] = {0,0};

	std::string storedPath[2] = {"",""};
	std::string fileDescription[2] = {"--none--","--none--"};

	float extTrigValue = 0.f;
	float prevExtTrigValue = 0.f;

	float clickOutput;

	int clickSelect = CLICK_STANDARD;

	double a0, a1, a2, b1, b2, z1, z2;

	int click_setting;
	
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
	
	double midBeatMaxSample = 0.0;
	bool midBeatPlayed = false;

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
	double avgPulse = 999999999.0;

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

	//int beatCounter = 1;
	int beatCounter = 20;	// thise ensure that module starts on a new bar

	float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	//float oneMsTime = (APP->engine->getSampleRate()) / 10;	// for testing purposes
	bool resetPulse = false;
	float resetPulseTime = 0.f;
	bool beatPulse = false;
	float beatPulseTime = 0.f;
	bool barPulse = false;
	float barPulseTime = 0.f;

	bool beatOnBar = false;

	bool resetOnRun = true;
	bool resetPulseOnRun = false;
	bool resetOnStop = false;
	bool resetPulseOnStop = false;

	bool extSync = false;
	bool extConn = false;
	bool prevExtConn = true;
	bool extBeat = false;
	
	double divClockSample[4] = {1.0, 1.0, 1.0, 1.0};
	double divMaxSample[4][2] = {{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}};
	int divOddCounter[4] = {0, 0, 0, 0};
	bool divPulse[4] = {false, false, false, false};
	float divPulseTime[4] = {0.0, 0.0, 0.0, 0.0};
	int divCount[4] = {1, 1, 1, 1};

	bool divSwing[4] = {false, false, false, false};

	Clocker() {
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
		configSwitch(CLICK_BUT_PARAM, 0.f, 1.f, 0.f, "Click", {"OFF", "ON"});
		configParam(CLICKVOL_KNOB_PARAM, 0.f, 2.f, 1.0f, "Click Level", "%", 0, 100);
		
		configParam<tpSignature>(SIGNATURE_KNOB_PARAM, 0.f, 16.0f, 2.f, "Time Signature");
		paramQuantities[SIGNATURE_KNOB_PARAM]->snapEnabled = true;

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

		configOutput(DIVMULT_OUTPUT+0,"Div/Mult #1");
		configOutput(DIVMULT_OUTPUT+1,"Div/Mult #2");
		configOutput(DIVMULT_OUTPUT+2,"Div/Mult #3");
		configOutput(DIVMULT_OUTPUT+3,"Div/Mult #4");

		configOutput(CLOCK_OUTPUT,"Clock");
		configOutput(MASTER_OUTPUT,"Metronome");
		configOutput(BEATPULSE_OUTPUT,"Beat Pulse");
		configOutput(BARPULSE_OUTPUT,"Bar Pulse");

		playBuffer[0].resize(0);
		playBuffer[1].resize(0);

		setClick(0);
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

		//beatCounter = 1;
		beatCounter = 20;
		clockSample = 1.0;
		beatOnBar = false;
		resetOnRun = true;
		resetPulseOnRun = false;
		resetOnStop = false;
		resetPulseOnStop = false;

		bpm = 0.1;

		for (int d = 0; d < 4; d++) {
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
		
		/*
		for (int i = 0; i < 2; i++) {
			clearSlot(i);
			play[i] = false;
		}
		*/
		setClick(0);

		Module::onReset(e);
	}

	void onSampleRateChange() override {
		sampleRateCoeff = (double)APP->engine->getSampleRate() * 60;
		oneMsTime = (APP->engine->getSampleRate()) / 1000;
		//oneMsTime = (APP->engine->getSampleRate()) / 10; // for testing purposes

		/*
		for (int i = 0; i < 2; i++) {
			if (fileLoaded[i]) {
				play[i] = false;
				loadSample(storedPath[i],i);
			}
		}
		*/
		setClick(clickSelect);
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "ppqn", json_integer(ppqn));
		//json_object_set_new(rootJ, "smooth", json_integer(smooth));		// experimental: EXTERNAL CLOCK SMOOTH
		//json_object_set_new(rootJ, "extStop", json_integer(extStop));		// expertimental: EXTERNAL CLOCK AUTO STOP
		json_object_set_new(rootJ, "BeatOnBar", json_boolean(beatOnBar));
		json_object_set_new(rootJ, "ResetOnRun", json_boolean(resetOnRun));
		json_object_set_new(rootJ, "ResetPulseOnRun", json_boolean(resetPulseOnRun));
		json_object_set_new(rootJ, "ResetOnStop", json_boolean(resetOnStop));
		json_object_set_new(rootJ, "ResetPulseOnStop", json_boolean(resetPulseOnStop));
		json_object_set_new(rootJ, "Swing1", json_boolean(divSwing[0]));
		json_object_set_new(rootJ, "Swing2", json_boolean(divSwing[1]));
		json_object_set_new(rootJ, "Swing3", json_boolean(divSwing[2]));
		json_object_set_new(rootJ, "Swing4", json_boolean(divSwing[3]));
		json_object_set_new(rootJ, "Slot1", json_string(storedPath[0].c_str()));
		json_object_set_new(rootJ, "Slot2", json_string(storedPath[1].c_str()));
		json_object_set_new(rootJ, "clickSelect", json_integer(clickSelect));
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

		json_t* beatOnBarJ = json_object_get(rootJ, "BeatOnBar");
		if (beatOnBarJ)
			beatOnBar = json_boolean_value(beatOnBarJ);

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

		/*
		json_t *slot1J = json_object_get(rootJ, "Slot1");
		if (slot1J) {
			storedPath[0] = json_string_value(slot1J);
			loadSample(storedPath[0], 0);
		}
		json_t *slot2J = json_object_get(rootJ, "Slot2");
		if (slot2J) {
			storedPath[1] = json_string_value(slot2J);
			loadSample(storedPath[1], 1);
		}
		*/
		json_t *clickSlot1J = json_object_get(rootJ, "Slot1");
		if (clickSlot1J) {
			storedPath[0] = json_string_value(clickSlot1J);
			if (storedPath[0] == "")
				clearSlot(0);
			else
				loadSample(storedPath[0], 0, true);
		}
		json_t *clickSlot2J = json_object_get(rootJ, "Slot2");
		if (clickSlot2J) {
			storedPath[1] = json_string_value(clickSlot2J);
			if (storedPath[1] == "")
				clearSlot(1);
			else
				loadSample(storedPath[1], 1, true);
		}

		json_t* clickSelectJ = json_object_get(rootJ, "clickSelect");
		if (clickSelectJ) {
			clickSelect = json_integer_value(clickSelectJ);
			if (clickSelect < 0 || clickSelect > 3)
				clickSelect = CLICK_STANDARD;
			setClick(clickSelect);
		}
	}

	void calcBiquadLpf(double frequency, double samplerate, double Q) {
	    z1 = z2 = 0.0;
	    double Fc = frequency / samplerate;
	    double norm;
	    double K = tan(M_PI * Fc);
        norm = 1 / (1 + K / Q + K * K);
        a0 = K * K * norm;
        a1 = 2 * a0;
        a2 = a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / Q + K * K) * norm;
    }

	float biquadLpf(float in) {
	    double out = in * a0 + z1;
	    z1 = in * a1 + z2 - b1 * out;
	    z2 = in * a2 - b2 * out;
	    return out;
	}

	double hermiteInterpol(double x0, double x1, double x2, double x3, double t)	{
		double c0 = x1;
		double c1 = .5F * (x2 - x0);
		double c2 = x0 - (2.5F * x1) + (2 * x2) - (.5F * x3);
		double c3 = (.5F * (x3 - x0)) + (1.5F * (x1 - x2));
		return (((((c3 * t) + c2) * t) + c1) * t) + c0;
	}
	
	void menuLoadSample(int slot) {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV;All files (*.*):*.*";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		fileLoaded[slot] = false;
		if (path) {
			/*
			loadSample(path, slot);
			storedPath[slot] = std::string(path);
			*/
			loadSample(path, slot, true);
			storedPath[slot] = std::string(path);
			if (clickSelect != CLICK_CUSTOM)
				setClick(clickSelect);
		} else {
			fileLoaded[slot] = true;
		}
		if (storedPath[slot] == "") {
			fileLoaded[slot] = false;
		}
		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadSample(std::string path, int slot, bool customClick) {
		z1 = 0; z2 = 0;

		unsigned int c;
		unsigned int sr;
		drwav_uint64 tsc;
		float* pSampleData;
		pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &tsc);

		if (pSampleData != NULL && tsc > minSamplesToLoad * c) {
			fileFound[slot] = true;
			sampleRate[slot] = sr * 2;
			
			playBuffer[slot].clear();

			tempBuffer.clear();
			tempBuffer2.clear();

			if (tsc > 96000)
				tsc = 96000;	// set memory allocation limit to 96000 samples*/

			if (sr == APP->engine->getSampleRate()) {			//  **************************   NO RESAMPLE   ************************
				for (unsigned int i=0; i < tsc; i = i + c) {
					playBuffer[slot].push_back(pSampleData[i] * 5);
				}
				totalSampleC[slot] = playBuffer[slot].size();
				totalSamples[slot] = totalSampleC[slot]-1;
				drwav_free(pSampleData);

				//sampleRate = APP->engine->getSampleRate() * 2;
				sampleRate[slot] = APP->engine->getSampleRate();

			} else {											// ***************** RESAMPLE ****************************************
				for (unsigned int i=0; i < tsc; i = i + c) {
					tempBuffer.push_back(pSampleData[i] * 5);
					tempBuffer.push_back(0);
				}

				drwav_free(pSampleData);

				drwav_uint64 tempSampleC = tempBuffer.size();
				drwav_uint64 tempSamples = tempSampleC-1;
				
				for (unsigned int i = 1; i < tempSamples; i = i+2)
					tempBuffer[i] = tempBuffer[i-1] * .5f + tempBuffer[i+1] * .5f;

				tempBuffer[tempSamples] = tempBuffer[tempSamples-1] * .5f; // halve the last sample

				// ***************************************************************************

				double resampleCoeff = sampleRate[slot] * .5 / (APP->engine->getSampleRate());
				double currResamplePos = 0;
				int floorCurrResamplePos = 0;

				tempBuffer2.push_back(tempBuffer[0]);

				currResamplePos += resampleCoeff;
				floorCurrResamplePos = floor(currResamplePos);
				int temp;

				while ( floorCurrResamplePos < 1 ) {
					temp = tempBuffer[floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);

					tempBuffer2.push_back(temp);

					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				} 

				while ( currResamplePos < tempSamples - 1) {

					tempBuffer2.push_back(	hermiteInterpol(tempBuffer[floorCurrResamplePos - 1],
															tempBuffer[floorCurrResamplePos],
															tempBuffer[floorCurrResamplePos + 1],
															tempBuffer[floorCurrResamplePos + 2],
															currResamplePos - floorCurrResamplePos)
											);

					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				}

				while ( floorCurrResamplePos < double(tempSampleC) ) {
					temp = tempBuffer[floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					tempBuffer2.push_back(temp);

					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				}

				// ***************************************************************************

				tempBuffer.clear();
				totalSampleC[slot] = tempBuffer2.size();

				calcBiquadLpf(20000.0, sampleRate[slot], 1);
				for (unsigned int i = 0; i < totalSampleC[slot]; i++)
					tempBuffer.push_back(biquadLpf(tempBuffer2[i]));

				for (unsigned int i = 0; i < totalSampleC[slot]; i = i + 2)
					playBuffer[slot].push_back(tempBuffer[i]);



				totalSampleC[slot] = playBuffer[slot].size();
				totalSamples[slot] = totalSampleC[slot]-1;
				sampleRate[slot] = APP->engine->getSampleRate();

			}

			tempBuffer.clear();
			tempBuffer2.clear();

			char* pathDup = strdup(path.c_str());
			/*
			fileDescription[slot] = basename(pathDup);

			free(pathDup);
			storedPath[slot] = path;

			fileLoaded[slot] = true;
			*/
			if (customClick) {
				fileDescription[slot] = basename(pathDup);

				storedPath[slot] = path;
	
			}
			fileLoaded[slot] = true;
			free(pathDup);

		} else {
			fileFound[slot] = false;
			fileLoaded[slot] = false;
			storedPath[slot] = path;
			fileDescription[slot] = "(!)"+path;
		}
	};
	
	void clearSlot(int slot) {
		storedPath[slot] = "";
		fileDescription[slot] = "--none--";
		if (clickSelect == CLICK_CUSTOM) {
			fileFound[slot] = false;
			fileLoaded[slot] = false;
			playBuffer[slot].clear();
			totalSampleC[slot] = 0;
		}
	}

	void setClick(int clickNo) {
		switch (clickNo) {
			case 0:
				loadSample(asset::plugin(pluginInstance, "res/clicks/click0_beat.wav"), 0, false);
				loadSample(asset::plugin(pluginInstance, "res/clicks/click0_bar.wav"), 1, false);
			break;

			case 1:
				loadSample(asset::plugin(pluginInstance, "res/clicks/click1_beat.wav"), 0, false);
				loadSample(asset::plugin(pluginInstance, "res/clicks/click1_bar.wav"), 1, false);
			break;

			case 2:
				loadSample(asset::plugin(pluginInstance, "res/clicks/click2_beat.wav"), 0, false);
				loadSample(asset::plugin(pluginInstance, "res/clicks/click2_bar.wav"), 1, false);
			break;

			case 3:
				if (storedPath[0] != "")
					loadSample(storedPath[0], 0, true);
				else
					clearSlot(0);
				if (storedPath[1] != "")
					loadSample(storedPath[1], 1, true);
				else clearSlot(1);
			break;
		}
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
		
		clickOutput = 0.f;

		click_setting = params[CLICK_BUT_PARAM].getValue();
		lights[CLICK_BUT_LIGHT].setBrightness(click_setting);

		lights[DIVSWING_LIGHT+0].setBrightness(divSwing[0]);
		lights[DIVSWING_LIGHT+1].setBrightness(divSwing[1]);
		lights[DIVSWING_LIGHT+2].setBrightness(divSwing[2]);
		lights[DIVSWING_LIGHT+3].setBrightness(divSwing[3]);

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
				for (int d = 0; d < 4; d++) {
					divPulse[d] = false;
					divClockSample[d] = 1.0;
					divMaxSample[d][0] = 0.0;
					divMaxSample[d][1] = 0.0;
					outputs[DIVMULT_OUTPUT+d].setVoltage(0.f);
				}
				midBeatPlayed = false;
				beatCounter = 20;
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
				for (int d = 0; d < 4; d++) {
					divPulse[d] = false;
					divClockSample[d] = 1.0;
					divMaxSample[d][0] = 0.0;
					divMaxSample[d][1] = 0.0;
					outputs[DIVMULT_OUTPUT+d].setVoltage(0.f);
				}
				midBeatPlayed = false;
				beatCounter = 20;
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
			for (int d = 0; d < 4; d++) {
				divPulse[d] = false;
				divClockSample[d] = 1.0;
				divMaxSample[d][0] = 0.0;
				divMaxSample[d][1] = 0.0;
				outputs[DIVMULT_OUTPUT+d].setVoltage(0.f);
			}
			midBeatPlayed = false;
			
			//beatCounter = 1;
			beatCounter = 20;

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

				for (int d = 0; d < 4; d++) {

					if (!divSwing[d]) {

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

				currentBeatMaxPerBar = beatMaxPerBar[int(params[SIGNATURE_KNOB_PARAM].getValue())];

				// ***********  MID BEAT PULSES WHEN USING TEMPOS WITH EIGHTH NOTES

				if (params[SIGNATURE_KNOB_PARAM].getValue() > 5 && !midBeatPlayed && clockSample > midBeatMaxSample)  {
					beatCounter++;
					if (beatCounter > currentBeatMaxPerBar) {
						beatCounter = 1;
						samplePos[BAR] = 0;
						play[BAR] = true;
						play[BEAT] = false;
						barPulse = true;
						barPulseTime = oneMsTime;
						if (beatOnBar) {
							beatPulse = true;
							beatPulseTime = oneMsTime;
						}
					} else {
						samplePos[BEAT] = 0;
						play[BEAT] = true;
						play[BAR] = false;
						beatPulse = true;
						beatPulseTime = oneMsTime;
					}
					midBeatPlayed = true;
				}
				

				//	*************************  INTERNAL CLOCK  ******************

				clockMaxSample = sampleRateCoeff / bpm;
				midBeatMaxSample = clockMaxSample / 2;
				
				if (clockSample > clockMaxSample || resetStart)  {
					midBeatPlayed = false;

					beatCounter++;

					if (resetStart) {
						clockSample = 1.0;
						resetStart = false;
					} else
						clockSample -= clockMaxSample;
					
					for (int d = 0; d < 4; d++) {
						
						if (!divSwing[d]) {

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

					if (beatCounter > currentBeatMaxPerBar) {
						beatCounter = 1;
						samplePos[BAR] = 0;
						play[BAR] = true;
						play[BEAT] = false;
						barPulse = true;
						barPulseTime = oneMsTime;
						if (beatOnBar) {
							beatPulse = true;
							beatPulseTime = oneMsTime;
						}
					} else {
						samplePos[BEAT] = 0;
						play[BEAT] = true;
						play[BAR] = false;
						beatPulse = true;
						beatPulseTime = oneMsTime;
					}
					
					outputs[CLOCK_OUTPUT].setVoltage(10.f);
					
				}

				clockSample++;
				divClockSample[0]++;
				divClockSample[1]++;
				divClockSample[2]++;
				divClockSample[3]++;
				
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
					//midBeatMaxSample = clockMaxSample / 2;

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

				for (int d = 0; d < 4; d++) {

					if(!divSwing[d]) {

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

				currentBeatMaxPerBar = beatMaxPerBar[int(params[SIGNATURE_KNOB_PARAM].getValue())];

				
				// ***********  MID BEAT PULSES WHEN USING TEMPOS WITH EIGHTH NOTES

				if (params[SIGNATURE_KNOB_PARAM].getValue() > 5 && !midBeatPlayed && clockSample > midBeatMaxSample)  {
					beatCounter++;
					if (beatCounter > currentBeatMaxPerBar) {
						beatCounter = 1;
						samplePos[BAR] = 0;
						play[BAR] = true;
						play[BEAT] = false;
						barPulse = true;
						barPulseTime = oneMsTime;
						if (beatOnBar) {
							beatPulse = true;
							beatPulseTime = oneMsTime;
						}
					} else {
						samplePos[BEAT] = 0;
						play[BEAT] = true;
						play[BAR] = false;
						beatPulse = true;
						beatPulseTime = oneMsTime;
					}
					midBeatPlayed = true;
				}
				

				// ************************ EXTERNAL CLOCK ******************

				if (extBeat) {

					midBeatPlayed = false;
					beatCounter++;

					if (extSync) {

						// ********** SYNCED BEAT

						for (int d = 0; d < 4; d++) {

							if (!divSwing[d]) {
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

						if (beatCounter > currentBeatMaxPerBar) {
							// ***** BAR DETECTED *****
							beatCounter = 1;
							samplePos[BAR] = 0;
							play[BAR] = true;
							play[BEAT] = false;
							barPulse = true;
							barPulseTime = oneMsTime;
							if (beatOnBar) {
								beatPulse = true;
								beatPulseTime = oneMsTime;
							}
						} else {
							// ***** BEAT DETECTED *****
							samplePos[BEAT] = 0;
							play[BEAT] = true;
							play[BAR] = false;
							beatPulse = true;
							beatPulseTime = oneMsTime;
						}
						
						outputs[CLOCK_OUTPUT].setVoltage(10.f);
					
					} else {

						//	************ UNSYNCED BEAT
					
						beatCounter++;
						samplePos[BEAT] = 0;
						play[BEAT] = true;
						play[BAR] = false;
						beatPulse = true;
						beatPulseTime = oneMsTime;

					}
				}
			}
				
			clockSample++;
			divClockSample[0]++;
			divClockSample[1]++;
			divClockSample[2]++;
			divClockSample[3]++;
			
		}

		// ***************************** COMMON PROCESS

		//	************ AUDIO CLICK

		if (click_setting) {
			for (int i = 0; i < 2; i++) {
				if (fileLoaded[i] && play[i] && floor(samplePos[i]) < totalSampleC[i]) {
					clickOutput = playBuffer[i][samplePos[i]] * params[CLICKVOL_KNOB_PARAM].getValue();
					if (clickOutput > 10.f)
						clickOutput = 10;
					else if (clickOutput < -10.f)
						clickOutput = -10;
					samplePos[i]++;
				} else {
					play[i] = false;
				}

				outputs[MASTER_OUTPUT].setVoltage(clickOutput);
			}
		} else {
			play[BEAT] = false;
			play[BAR] = false;
		}

		//	********** RESET AND BEAT AND BAR PULSES

		if (resetPulse) {
			resetPulseTime--;
			if (resetPulseTime < 0) {
				resetPulse = false;
				outputs[RESET_OUTPUT].setVoltage(0.f);
			} else
				outputs[RESET_OUTPUT].setVoltage(10.f);
		}

		if (beatPulse) {
			beatPulseTime--;
			if (beatPulseTime < 0) {
				beatPulse = false;
				outputs[BEATPULSE_OUTPUT].setVoltage(0.f);
			} else
				outputs[BEATPULSE_OUTPUT].setVoltage(10.f);
		}

		if (barPulse) {
			barPulseTime--;
			if (barPulseTime < 0) {
				barPulse = false;
				outputs[BARPULSE_OUTPUT].setVoltage(0.f);
			} else
				outputs[BARPULSE_OUTPUT].setVoltage(10.f);
		}

		for (int d = 0; d < 4; d++) {
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

struct ClockerDisplayTempo : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDisplayTempo() {
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

struct ClockerDisplayBeat : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDisplayBeat() {
	}

	void onButton(const event::Button &e) override {
		/*if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);*/

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
				nvgFillColor(args.vg, nvgRGBA(0xaa, 0xcc, 0xff, 0xe0));

				int tempValue = int(module->params[module->SIGNATURE_KNOB_PARAM].getValue());
				if (tempValue > 10)
					nvgTextBox(args.vg, 0, 15, 60, module->signatureDisplay[tempValue].c_str(), NULL);
				else
					nvgTextBox(args.vg, 10, 15, 60, module->signatureDisplay[tempValue].c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		Clocker *module = dynamic_cast<Clocker *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker* module;
				int valueNr;
				void onAction(const event::Action& e) override {
					module->params[module->SIGNATURE_KNOB_PARAM].setValue(float(valueNr));
				}
			};

			std::string menuNames[17] = {"2/4", "3/4", "4/4", "5/4", "6/4", "7/4", "5/8", "6/8", "7/8", "8/8", "9/8", "10/8", "11/8", "12/8", "13/8", "14/8", "15/8"};
			for (int i = 0; i < 17; i++) {
				ThisItem* thisItem = createMenuItem<ThisItem>(menuNames[i]);
				thisItem->rightText = CHECKMARK(int(module->params[module->SIGNATURE_KNOB_PARAM].getValue()) == i);
				thisItem->module = module;
				thisItem->valueNr = i;
				menu->addChild(thisItem);
			}
		}
	}
};

struct ClockerDisplayDiv1 : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDisplayDiv1() {
	}

	void onButton(const event::Button &e) override {
		/*if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);*/

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
		Clocker *module = dynamic_cast<Clocker *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker* module;
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

struct ClockerDisplayDiv2 : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDisplayDiv2() {
	}

	void onButton(const event::Button &e) override {
		/*if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);*/

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
		Clocker *module = dynamic_cast<Clocker *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker* module;
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

struct ClockerDisplayDiv3 : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDisplayDiv3() {
	}

	void onButton(const event::Button &e) override {
		/*if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);*/

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
		Clocker *module = dynamic_cast<Clocker *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker* module;
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

struct ClockerDisplayDiv4 : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDisplayDiv4() {
	}

	void onButton(const event::Button &e) override {
		/*if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);*/

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
		Clocker *module = dynamic_cast<Clocker *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker* module;
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

/*
struct ClockerDebugDisplay : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDebugDisplay() {
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

struct ClockerWidget : ModuleWidget {
	ClockerWidget(Clocker *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Clocker.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH))); 

		/*		
		{
			ClockerDebugDisplay *display = new ClockerDebugDisplay();
			display->box.pos = Vec(0, 10);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/	

		{
			ClockerDisplayTempo *display = new ClockerDisplayTempo();
			display->box.pos = mm2px(Vec(13.222, 17.5));
			display->box.size = mm2px(Vec(16.8, 6.5));
			display->module = module;
			addChild(display);
		}

		{
			ClockerDisplayBeat *display = new ClockerDisplayBeat();
			display->box.pos = mm2px(Vec(22, 52));
			display->box.size = mm2px(Vec(14.5, 6));
			display->module = module;
			addChild(display);
		}

		{
			ClockerDisplayDiv1 *display = new ClockerDisplayDiv1();
			display->box.pos = mm2px(Vec(15.3, 80.2));
			display->box.size = mm2px(Vec(15, 6.3));
			display->module = module;
			addChild(display);
		}

		{
			ClockerDisplayDiv2 *display = new ClockerDisplayDiv2();
			display->box.pos = mm2px(Vec(15.3, 91.2));
			display->box.size = mm2px(Vec(15, 6.3));
			display->module = module;
			addChild(display);
		}

		{
			ClockerDisplayDiv3 *display = new ClockerDisplayDiv3();
			display->box.pos = mm2px(Vec(15.3, 102.2));
			display->box.size = mm2px(Vec(15, 6.3));
			display->module = module;
			addChild(display);
		}

		{
			ClockerDisplayDiv4 *display = new ClockerDisplayDiv4();
			display->box.pos = mm2px(Vec(15.3, 113.2));
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

		const float xBeatKnob = 10.f;

		const float xCLickBut = 24.5f;
		const float xClickVolKnob = 35.f;

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

		const float yClick1 = 63.f;
		const float yClick2 = 67.f;

		const float yDivKn1 = 83.5f;
		const float yDivKn2 = 94.5f;
		const float yDivKn3 = 105.5f;
		const float yDivKn4 = 116.5f;

		const float yDivLg1 = 80.5f;
		const float yDivLg2 = 91.5f;
		const float yDivLg3 = 102.5f;
		const float yDivLg4 = 113.f;

		const float yClockOut = 17.5f;
		const float yResetOut = 31.5f;

		const float yBeatOut = 48.f;
		const float yBarOut = 60.f;
		const float yClickOut = 72.f;

		const float yDiv1 = 88.5f;
		const float yDiv2 = 98.f;
		const float yDiv3 = 107.5f;
		const float yDiv4 = 117.f;

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xExtClock, yExtClock)), module, Clocker::EXTCLOCK_INPUT));
		
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xResetIn, yRstIn)), module, Clocker::RESET_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(xResetBut, yRstBut)), module, Clocker::RESET_BUT_PARAM, Clocker::RESET_BUT_LIGHT));
		

		addParam(createLightParamCentered<VCVLightBezelLatch<BlueLight>>(mm2px(Vec(xRun, yRunBut)), module, Clocker::RUN_BUT_PARAM, Clocker::RUN_BUT_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRun, yRunIn)), module, Clocker::RUN_INPUT));

		addParam(createParamCentered<SickoBigKnob>(mm2px(Vec(xBpmKnob, yBpmKob)), module, Clocker::BPM_KNOB_PARAM));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPwKnob, yPwClOut)), module, Clocker::PW_KNOB_PARAM));
		
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xBeatKnob, yClick1 + .9f)), module, Clocker::SIGNATURE_KNOB_PARAM));
		
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xCLickBut, yClick2)), module, Clocker::CLICK_BUT_PARAM, Clocker::CLICK_BUT_LIGHT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xClickVolKnob, yClick2 + .3f)), module, Clocker::CLICKVOL_KNOB_PARAM));
		
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xDivKnob, yDivKn1)), module, Clocker::DIVMULT_KNOB_PARAM+0));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xDivKnob, yDivKn2)), module, Clocker::DIVMULT_KNOB_PARAM+1));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xDivKnob, yDivKn3)), module, Clocker::DIVMULT_KNOB_PARAM+2));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xDivKnob, yDivKn4)), module, Clocker::DIVMULT_KNOB_PARAM+3));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPwKnob, yDivKn1)), module, Clocker::DIVPW_KNOB_PARAM+0));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPwKnob, yDivKn2)), module, Clocker::DIVPW_KNOB_PARAM+1));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPwKnob, yDivKn3)), module, Clocker::DIVPW_KNOB_PARAM+2));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPwKnob, yDivKn4)), module, Clocker::DIVPW_KNOB_PARAM+3));

		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(xDivLg, yDivLg1)), module, Clocker::DIVSWING_LIGHT+0));
		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(xDivLg, yDivLg2)), module, Clocker::DIVSWING_LIGHT+1));
		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(xDivLg, yDivLg3)), module, Clocker::DIVSWING_LIGHT+2));
		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(xDivLg, yDivLg4)), module, Clocker::DIVSWING_LIGHT+3));
		
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yClockOut)), module, Clocker::CLOCK_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yResetOut)), module, Clocker::RESET_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yBeatOut)), module, Clocker::BEATPULSE_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yBarOut)), module, Clocker::BARPULSE_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yClickOut)), module, Clocker::MASTER_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yDiv1)), module, Clocker::DIVMULT_OUTPUT+0));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yDiv2)), module, Clocker::DIVMULT_OUTPUT+1));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yDiv3)), module, Clocker::DIVMULT_OUTPUT+2));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xDivOut, yDiv4)), module, Clocker::DIVMULT_OUTPUT+3));
	}

	void appendContextMenu(Menu *menu) override {
	   	Clocker *module = dynamic_cast<Clocker*>(this->module);
			assert(module);
		
		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Trig/Swing on Div #1", "", &module->divSwing[0]));
		menu->addChild(createBoolPtrMenuItem("Trig/Swing on Div #2", "", &module->divSwing[1]));
		menu->addChild(createBoolPtrMenuItem("Trig/Swing on Div #3", "", &module->divSwing[2]));
		menu->addChild(createBoolPtrMenuItem("Trig/Swing on Div #4", "", &module->divSwing[3]));

		menu->addChild(new MenuSeparator());

		struct PpqnItem : MenuItem {
			Clocker* module;
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
			Clocker* module;
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
			Clocker* module;
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


		/*
		menu->addChild(new MenuSeparator());

		menu->addChild(createSubmenuItem("Click Presets", "", [=](Menu * menu) {
			menu->addChild(createMenuItem("Standard", "", [=]() {module->setClick(0);}));
			menu->addChild(createMenuItem("Click1", "", [=]() {module->setClick(1);}));
			menu->addChild(createMenuItem("Click2", "", [=]() {module->setClick(2);}));
		}));

		menu->addChild(new MenuSeparator());

		menu->addChild(createMenuItem("Load BEAT click", "", [=]() {module->menuLoadSample(0);}));
		menu->addChild(createMenuItem("File: " + module->fileDescription[0], "", [=]() {module->menuLoadSample(0);}));
		menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot(0);}));
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Load BAR click", "", [=]() {module->menuLoadSample(1);}));
		menu->addChild(createMenuItem("File: " + module->fileDescription[1], "", [=]() {module->menuLoadSample(1);}));
		menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot(1);}));

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

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Beat pulse also on Bar", "", &module->beatOnBar));

		struct ClickItem : MenuItem {
			Clocker* module;
			int clickSelect;
			void onAction(const event::Action& e) override {
				module->clickSelect = clickSelect;
				module->setClick(clickSelect);
			}
		};

		menu->addChild(createSubmenuItem("Click Settings", "", [=](Menu * menu) {

			std::string clickNames[4] = {"Standard", "Click1", "Click2", "Custom"};
			for (int i = 0; i < 4; i++) {
				ClickItem* clickItem = createMenuItem<ClickItem>(clickNames[i]);
				clickItem->rightText = CHECKMARK(module->clickSelect == i);
				clickItem->module = module;
				clickItem->clickSelect = i;
				menu->addChild(clickItem);
			}

			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuItem("Custom BEAT click", "", [=]() {module->menuLoadSample(0);}));
			menu->addChild(createMenuItem("File: " + module->fileDescription[0], "", [=]() {module->menuLoadSample(0);}));
			menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot(0);}));
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Custom BAR click", "", [=]() {module->menuLoadSample(1);}));
			menu->addChild(createMenuItem("File: " + module->fileDescription[1], "", [=]() {module->menuLoadSample(1);}));
			menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot(1);}));
		}));
	}
};

Model *modelClocker = createModel<Clocker, ClockerWidget>("Clocker");
