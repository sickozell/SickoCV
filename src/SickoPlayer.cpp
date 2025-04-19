#define LEFT 0
#define RIGHT 1
#define STOP_STAGE 0
#define ATTACK_STAGE 1
#define DECAY_STAGE 2
#define SUSTAIN_STAGE 3
#define RELEASE_STAGE 4
#define GATE_MODE 0
#define TRIG_MODE 1
#define START_STOP 0
#define START_RESTART 1
#define PLAY_PAUSE 2
#define NO_INTERP 0
#define LINEAR1_INTERP 1
#define LINEAR2_INTERP 2
#define HERMITE_INTERP 3
#define MONOPHONIC 0
#define POLYPHONIC 1
#define NO_FADE 0
#define CROSS_FADE 1
#define FADE_OUT 2
#define PINGPONG_FADE 3
#define FORWARD 0
#define REVERSE 1

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

struct SickoPlayer : Module {
	enum ParamIds {
		VOL_PARAM,
		VOLATNV_PARAM,
		TUNE_PARAM,
		TUNEATNV_PARAM,
		CUESTART_PARAM,
		CUEEND_PARAM,
		LOOPSTART_PARAM,
		LOOPEND_PARAM,
		ATTACK_PARAM,
		ATTACKATNV_PARAM,
		DECAY_PARAM,
		DECAYATNV_PARAM,
		SUSTAIN_PARAM,
		SUSTAINATNV_PARAM,
		RELEASE_PARAM,
		RELEASEATNV_PARAM,
		TRIGGATEMODE_SWITCH,
		TRIGMODE_SWITCH,
		LIMIT_SWITCH,
		LOOP_PARAM,
		REV_PARAM,
		PINGPONG_PARAM,
		XFADE_PARAM,
		PREVSAMPLE_PARAM,
		NEXTSAMPLE_PARAM,
		TRIGBUT_PARAM,
		STOPBUT_PARAM,
		NUM_PARAMS 
	};
	enum InputIds {
		TRIG_INPUT,
		STOP_INPUT,
		VOL_INPUT,
		TUNE_INPUT,
		VO_INPUT,
		ATTACK_INPUT,
		DECAY_INPUT,
		SUSTAIN_INPUT,
		RELEASE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUT_OUTPUT,2),
		EOC_OUTPUT,
		EOR_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LOOP_LIGHT,
		REV_LIGHT,
		PINGPONG_LIGHT,
		CLIPPING_LIGHT,
		TRIGBUT_LIGHT,
		STOPBUT_LIGHT,
		NUM_LIGHTS
	};
  
	unsigned int channels;
	unsigned int sampleRate;
	drwav_uint64 totalSampleC = 0;
	drwav_uint64 totalSamples = 0;

	const unsigned int minSamplesToLoad = 124;

	vector<float> playBuffer[2][2];
	vector<double> displayBuff;
	int currentDisplay = 0;
	float voctDisplay = 100.f;

	bool fileLoaded = false;

	bool play[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool playPauseToStop[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	double samplePos[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double sampleCoeff;
	double currentSpeed = 0.0;
	double distancePos[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	double cueStartPos;
	double cueEndPos;
	double loopStartPos;
	double loopEndPos;

	float knobCueStartPos;
	float knobCueEndPos;
	float prevKnobCueStartPos = -1.f;
	float prevKnobCueEndPos = 2.f;

	float knobLoopStartPos;
	float knobLoopEndPos;
	float prevKnobLoopStartPos = -1.f;
	float prevKnobLoopEndPos = 2.f;

	float tune = 0.f;
	float prevTune = -1.f;
	
	float voct[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevVoct[16] = {11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f};
	float speedVoct[16];

	bool searchingCueStartPhase = false;
	bool searchingCueEndPhase = false;
	double scanCueStartSample;
	double scanCueEndSample;

	bool searchingLoopStartPhase = false;
	bool searchingLoopEndPhase = false;
	double scanLoopStartSample;
	double scanLoopEndSample;

	double prevSampleWeight[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double currSampleWeight[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double prevSamplePos[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	std::string storedPath = "";
	std::string fileDescription = "--none--";
	std::string fileDisplay = "";
	std::string channelsDisplay = "";
	std::string timeDisplay = "";
	std::string samplerateDisplay = "";

	std::string userFolder = "";
	std::string currentFolder = "";
	vector <std::string> currentFolderV;
	int currentFile = 0;

	bool rootFound = false;
	bool fileFound = false;

	std::string tempDir = "";
	vector<vector<std::string>> folderTreeData;
	vector<vector<std::string>> folderTreeDisplay;
	vector<std::string> tempTreeData;
	vector<std::string> tempTreeDisplay;

	int seconds;
	int minutes;

	float trigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float trigButValue = 0;
	float stopButValue = 0;
	float prevTrigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float stopValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevStopValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	bool loop = false;

	int interpolationMode = HERMITE_INTERP;
	int antiAlias = 1;
	int polyOuts = POLYPHONIC;
	int polyMaster = POLYPHONIC;
	bool phaseScan = true;
	bool prevPhaseScan = false;
	bool firstLoad = true;
	bool resetCursorsOnLoad = true;
	bool disableNav = false;
	bool sampleInPatch = true;
	
	bool loadFromPatch = false;
	bool restoreLoadFromPatch = false;

	float fadeCoeff = 0.f;

	int fadingType[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float fadingValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float fadeDecrement = 0;
	double fadedPosition[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double fadeSamples = 0;
	float xFade = 0.f;
	float prevXfade = -1.f;

	float currentOutput;
	float currentOutputR;
	float sumOutput;
	float sumOutputR;

	//std::string debugDisplay = "X";
	//std::string debugDisplay2 = "X";
	//int debugInt = 0;

	double a0, a1, a2, b1, b2, z1, z2, z1r, z2r;

	int trigMode;
	int trigType;
	int stage[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float stageLevel[16];
	
	bool eoc[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool eor[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	float eocTime[16];
	float eorTime[16];
	float oneMsSamples = (APP->engine->getSampleRate()) / 1000;

	bool eocFromTrg = false;
	bool eocFromStop = false;
	bool eocFromCueEnd = true;
	bool eocFromCueStart = true;
	bool eocFromLoopEnd = true;
	bool eocFromLoopStart = true;
	bool eocFromPing = true;
	bool eocFromPong = true;

	int chan;

	float attackValue;
	float decayValue;
	float sustainValue;
	float releaseValue;
	float stageCoeff[16];
	float masterLevel[16];
	int limiter;

	bool reverseStart = false;
	int reversePlaying[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int pingpong;

	float clippingValue = 0;
	float clippingCoeff = 5 / (APP->engine->getSampleRate());	// nr of samples of 200 ms (1/0.2)
	bool clipping = fadeSamples;

	bool nextSample = false;
	bool prevNextSample = false;
	bool prevSample = false;
	bool prevPrevSample = false;

	bool unlimitedRecording = false;
#if defined(METAMODULE)
	const drwav_uint64 recordingLimit = 48000 * 2 * 60; // 60 sec limit on MM = 5.5MB
#else
	const drwav_uint64 recordingLimit = 52428800 * 2; // set memory allocation limit to 200Mb for samples (~18mins at 48.000khz MONO)
	// const drwav_uint64 recordingLimit = 480000; // 10 sec for test purposes
#endif

	drwav_uint64 currentRecordingLimit = recordingLimit;

	static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	const float maxAdsrTime = 10.f;
	const float minAdsrTime = 0.001f;

	SickoPlayer() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configSwitch(PREVSAMPLE_PARAM, 0.f, 1.f, 0.f, "Previous Sample");
		configSwitch(NEXTSAMPLE_PARAM, 0.f, 1.f, 0.f, "Next Sample");

		configSwitch(TRIGGATEMODE_SWITCH, 0.f, 1.f, 1.f, "Mode", {"Gate", "Trig"});
		configSwitch(TRIGMODE_SWITCH, 0.f, 2.f, 0.f, "Trig Type", {"Start/Stop", "Restart", "Play/Pause"});

		configInput(TRIG_INPUT,"Play");
		configSwitch(TRIGBUT_PARAM, 0.f, 1.f, 0.f, "Play", {"OFF", "ON"});
		configInput(STOP_INPUT,"Stop");
		configSwitch(STOPBUT_PARAM, 0.f, 1.f, 0.f, "Stop", {"OFF", "ON"});

		//******************************************************************************

		configParam(CUESTART_PARAM, 0.f, 1.0f, 0.f, "Cue Start", "%", 0, 100);
		configParam(CUEEND_PARAM, 0.f, 1.0f, 1.f, "Cue End", "%", 0, 100);

		configParam(LOOPSTART_PARAM, 0.f, 1.0f, 0.f, "Loop Start", "%", 0, 100);
		configParam(LOOPEND_PARAM, 0.f, 1.0f, 1.0f, "Loop End", "%", 0, 100);

		configSwitch(REV_PARAM, 0.f, 1.f, 0.f, "Playback Start", {"Forward", "Reverse"});
		configParam(XFADE_PARAM, 0.f, 1.f, 0.f, "Crossfade", "ms", maxStageTime / minStageTime, minStageTime);
		
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});
		configSwitch(PINGPONG_PARAM, 0.f, 1.f, 0.f, "PingPong", {"Off", "On"});

		//******************************************************************************

		configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Attack", "ms", maxStageTime / minStageTime, minStageTime);
		configInput(ATTACK_INPUT,"Attack");
		configParam(ATTACKATNV_PARAM, -1.0f, 1.0f, 0.0f, "Attack CV","%", 0, 100);

		configParam(DECAY_PARAM, 0.f, 1.f, 0.f, "Decay", "ms", maxStageTime / minStageTime, minStageTime);
		configInput(DECAY_INPUT,"Decay");
		configParam(DECAYATNV_PARAM, -1.0f, 1.0f, 0.0f, "Decay CV","%", 0, 100);

		configParam(SUSTAIN_PARAM, 0.f, 1.0f, 1.0f, "Sustain","%", 0, 100);
		configInput(SUSTAIN_INPUT,"Sustain");
		configParam(SUSTAINATNV_PARAM, -1.0f, 1.0f, 0.0f, "Sustain CV","%", 0, 100);

		configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "Release", "ms", maxStageTime / minStageTime, minStageTime);
		configInput(RELEASE_INPUT,"Release ");
		configParam(RELEASEATNV_PARAM, -1.0f, 1.0f, 0.0f, "Release CV","%", 0, 100);

		//******************************************************************************
		
		configInput(VO_INPUT,"V/Oct");

		configParam(TUNE_PARAM, -2.f, 2.f, 0.f, "Tune", " semitones", 0, 12);
		configInput(TUNE_INPUT,"Tune");
		configParam(TUNEATNV_PARAM, -1.0f, 1.0f, 0.0f, "Tune CV","%", 0, 100);

		configParam(VOL_PARAM, 0.f, 2.0f, 1.0f, "Master Volume", "%", 0, 100);
		configInput(VOL_INPUT,"Master Volume");
		configParam(VOLATNV_PARAM, -1.f, 1.0f, 0.f, "Master Volume CV", "%", 0, 100);

		configSwitch(LIMIT_SWITCH, 0.f, 1.f, 0.f, "Limit", {"Off", "±5v"});

		//******************************************************************************
		
		configOutput(OUT_OUTPUT,"Left");
		configOutput(OUT_OUTPUT+1,"Right");
		configOutput(EOC_OUTPUT,"End of Cycle");
		configOutput(EOR_OUTPUT,"End of Release");

		playBuffer[0][0].resize(0);
		playBuffer[0][1].resize(0);
		playBuffer[1][0].resize(0);
		playBuffer[1][1].resize(0);
	}

	static float convertCVToSeconds(float cv) {		
		return minStageTime * std::pow(maxStageTime / minStageTime, cv) / 1000;
	}

	void onReset(const ResetEvent &e) override {
		for (int i = 0; i < 16; i++) {
			play[i] = false;
			fadingType[i] = NO_FADE;
			stage[i] = STOP_STAGE;
			stageLevel[i] = 0;
			voct[i] = 0.f;
			prevVoct[i] = 11.f;
			reversePlaying[i] = FORWARD;
		}
		clearSlot();
		interpolationMode = HERMITE_INTERP;
		antiAlias = 1;
		polyOuts = POLYPHONIC;
		polyMaster = POLYPHONIC;
		phaseScan = true;
		prevPhaseScan = false;
		firstLoad = true;
		resetCursorsOnLoad = true;
		eocFromTrg = false;
		eocFromStop = false;
		eocFromCueEnd = true;
		eocFromCueStart = true;
		eocFromLoopEnd = true;
		eocFromLoopStart = true;
		eocFromPing = true;
		eocFromPong = true;
		disableNav = false;
		sampleInPatch = true;
		prevKnobCueStartPos = -1.f;
		prevKnobCueEndPos = 2.f;
		prevKnobLoopStartPos = -1.f;
		prevKnobLoopEndPos = 2.f;
		prevTune = -1.f;
		reverseStart = false;
		totalSampleC = 0;
		totalSamples = 0;
		prevXfade = -1.f;
		system::removeRecursively(getPatchStorageDirectory().c_str());
		Module::onReset(e);
	}

	void onSampleRateChange() override {
		oneMsSamples = (APP->engine->getSampleRate())/1000;			// number of samples for 1 ms used for output triggers
		clippingCoeff = 5 / (APP->engine->getSampleRate());	// decrement for 200 ms clipping light (1/0.2)
		if (fileLoaded)
			sampleCoeff = sampleRate / (APP->engine->getSampleRate());			// the % distance between samples at speed 1x
	}

	void onAdd(const AddEvent& e) override {
		if (!fileLoaded) {
			std::string patchFile = system::join(getPatchStorageDirectory(), "sample.wav");
			loadFromPatch = true;
			loadSample(patchFile);
		}		
		Module::onAdd(e);
	}

	void onSave(const SaveEvent& e) override {
		system::removeRecursively(getPatchStorageDirectory().c_str());
		if (fileLoaded) {
			if (sampleInPatch) {
				std::string patchFile = system::join(createPatchStorageDirectory(), "sample.wav");
				saveSample(patchFile);
			}
		}
		Module::onSave(e);
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Interpolation", json_integer(interpolationMode));
		json_object_set_new(rootJ, "AntiAlias", json_integer(antiAlias));
		json_object_set_new(rootJ, "PolyOuts", json_integer(polyOuts));
		json_object_set_new(rootJ, "PolyMaster", json_integer(polyMaster));
		json_object_set_new(rootJ, "PhaseScan", json_boolean(phaseScan));
		json_object_set_new(rootJ, "EocFromTrg", json_boolean(eocFromTrg));
		json_object_set_new(rootJ, "EocFromStop", json_boolean(eocFromStop));
		json_object_set_new(rootJ, "EocFromCueEnd", json_boolean(eocFromCueEnd));
		json_object_set_new(rootJ, "EocFromCueStart", json_boolean(eocFromCueStart));
		json_object_set_new(rootJ, "EocFromLoopEnd", json_boolean(eocFromLoopEnd));
		json_object_set_new(rootJ, "EocFromLoopStart", json_boolean(eocFromLoopStart));
		json_object_set_new(rootJ, "EocFromPing", json_boolean(eocFromPing));
		json_object_set_new(rootJ, "EocFromPong", json_boolean(eocFromPong));
		json_object_set_new(rootJ, "DisableNav", json_boolean(disableNav));
		json_object_set_new(rootJ, "sampleInPatch", json_boolean(sampleInPatch));
		json_object_set_new(rootJ, "unlimitedRecording", json_boolean(unlimitedRecording));
		json_object_set_new(rootJ, "ResetCursorsOnLoad", json_boolean(resetCursorsOnLoad));
		json_object_set_new(rootJ, "Slot", json_string(storedPath.c_str()));
		json_object_set_new(rootJ, "UserFolder", json_string(userFolder.c_str()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t* interpolationJ = json_object_get(rootJ, "Interpolation");
		if (interpolationJ)
			interpolationMode = json_integer_value(interpolationJ);
		json_t* antiAliasJ = json_object_get(rootJ, "AntiAlias");
		if (antiAliasJ)
			antiAlias = json_integer_value(antiAliasJ);
		json_t* polyOutsJ = json_object_get(rootJ, "PolyOuts");
		if (polyOutsJ)
			polyOuts = json_integer_value(polyOutsJ);
		json_t* polyMasterJ = json_object_get(rootJ, "PolyMaster");
		if (polyMasterJ)
			polyMaster = json_integer_value(polyMasterJ);
		json_t* phaseScanJ = json_object_get(rootJ, "PhaseScan");
		if (phaseScanJ)
			phaseScan = json_boolean_value(phaseScanJ);
		json_t* eocFromTrgJ = json_object_get(rootJ, "EocFromTrg");
		if (eocFromTrgJ)
			eocFromTrg = json_boolean_value(eocFromTrgJ);
		json_t* eocFromStopJ = json_object_get(rootJ, "EocFromStop");
		if (eocFromStopJ)
			eocFromStop = json_boolean_value(eocFromStopJ);
		json_t* eocFromCueEndJ = json_object_get(rootJ, "EocFromCueEnd");
		if (eocFromCueEndJ)
			eocFromCueEnd = json_boolean_value(eocFromCueEndJ);
		json_t* eocFromCueStartJ = json_object_get(rootJ, "EocFromCueStart");
		if (eocFromCueStartJ)
			eocFromCueStart = json_boolean_value(eocFromCueStartJ);
		json_t* eocFromLoopEndJ = json_object_get(rootJ, "EocFromLoopEnd");
		if (eocFromLoopEndJ)
			eocFromLoopEnd = json_boolean_value(eocFromLoopEndJ);
		json_t* eocFromLoopStartJ = json_object_get(rootJ, "EocFromLoopStart");
		if (eocFromLoopStartJ)
			eocFromLoopStart = json_boolean_value(eocFromLoopStartJ);
		json_t* eocFromPingJ = json_object_get(rootJ, "EocFromPing");
		if (eocFromPingJ)
			eocFromPing = json_boolean_value(eocFromPingJ);
		json_t* eocFromPongJ = json_object_get(rootJ, "EocFromPong");
		if (eocFromPongJ)
			eocFromPong = json_boolean_value(eocFromPongJ);
		json_t* disableNavJ = json_object_get(rootJ, "DisableNav");
		if (disableNavJ)
			disableNav = json_boolean_value(disableNavJ);
		json_t* sampleInPatchJ = json_object_get(rootJ, "sampleInPatch");
		if (sampleInPatchJ)
			sampleInPatch = json_boolean_value(sampleInPatchJ);
		json_t* unlimitedRecordingJ = json_object_get(rootJ, "unlimitedRecording");
		if (unlimitedRecordingJ)
			unlimitedRecording = json_boolean_value(unlimitedRecordingJ);
		json_t* resetCursorsOnLoadJ = json_object_get(rootJ, "ResetCursorsOnLoad");
		if (resetCursorsOnLoadJ)
			resetCursorsOnLoad = json_boolean_value(resetCursorsOnLoadJ);
		json_t *slotJ = json_object_get(rootJ, "Slot");
		if (slotJ) {
			storedPath = json_string_value(slotJ);
			if (storedPath != "") {
				loadSample(storedPath);
			} else
				firstLoad = false;
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

	float biquadLpf2(float in) {
		double out = in * a0 + z1r;
		z1r = in * a1 + z2r - b1 * out;
		z2r = in * a2 - b2 * out;
		return out;
	}

	/*
	double hermiteInterpol(double x0, double x1, double x2, double x3, double t)	{
		double c0 = x1;
		double c1 = .5F * (x2 - x0);
		double c2 = x0 - (2.5F * x1) + (2 * x2) - (.5F * x3);
		double c3 = (.5F * (x3 - x0)) + (1.5F * (x1 - x2));
		return (((((c3 * t) + c2) * t) + c1) * t) + c0;
	}
	*/

	void selectRootFolder() {
		const char* prevFolder = userFolder.c_str();
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN_DIR, prevFolder, NULL, NULL, [this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN_DIR, prevFolder, NULL, NULL);
#endif
		if (path) {
			folderTreeData.clear();
			folderTreeDisplay.clear();
			userFolder = std::string(path);
			createFolder(userFolder);
			if (rootFound) {
				folderTreeData.push_back(tempTreeData);
				folderTreeDisplay.push_back(tempTreeDisplay);
			}
		}
		free(path);
#if defined(METAMODULE)
		});
#endif
	};

	void refreshRootFolder() {
		folderTreeData.clear();
		folderTreeDisplay.clear();
		createFolder(userFolder);
		if (rootFound) {
			folderTreeData.push_back(tempTreeData);
			folderTreeDisplay.push_back(tempTreeDisplay);
		}
	}

	void createFolder(std::string dir_path) {
		vector <std::string> browserList;
		vector <std::string> browserListDisplay;
		vector <std::string> browserDir;
		vector <std::string> browserDirDisplay;
		vector <std::string> browserFiles;
		vector <std::string> browserFilesDisplay;

		tempTreeData.clear();
		tempTreeDisplay.clear();

		std::string lastChar = dir_path.substr(dir_path.length()-1,dir_path.length()-1);
		if (lastChar != "/")
			dir_path += "/";

		DIR *dir = opendir(dir_path.c_str());

		if (dir) {
			rootFound = true;
			struct dirent *d;
			while ((d = readdir(dir))) {
				std::string filename = d->d_name;
				if (filename != "." && filename != "..") {
					std::string filepath = std::string(dir_path) + filename;
					struct stat statbuf;
					if (stat(filepath.c_str(), &statbuf) == 0 && (statbuf.st_mode & S_IFMT) == S_IFDIR) {
						//browserDir.push_back(filepath + "/");
						browserDir.push_back(filepath);
						browserDirDisplay.push_back(filename);
					} else {
						std::size_t found = filename.find(".wav",filename.length()-5);
						if (found==std::string::npos)
							found = filename.find(".WAV",filename.length()-5);
						if (found!=std::string::npos) {
							browserFiles.push_back(filepath);
							browserFilesDisplay.push_back(filename.substr(0, filename.length()-4));
						}
					}
				}
	   		}
	   		closedir(dir);

			sort(browserDir.begin(), browserDir.end());
			sort(browserDirDisplay.begin(), browserDirDisplay.end());
			sort(browserFiles.begin(), browserFiles.end());
			sort(browserFilesDisplay.begin(), browserFilesDisplay.end());
			
			// this adds "/" to browserDir after sorting to avoid wrong sorting with foldernames with spaces
			int dirSize = (int)browserDir.size();
			for (int i=0; i < dirSize; i++)
				browserDir[i] += "/";
			
			tempTreeData.push_back(dir_path);
			tempTreeDisplay.push_back(dir_path);

			for (unsigned int i = 0; i < browserDir.size(); i++) {
				tempTreeData.push_back(browserDir[i]);
				tempTreeDisplay.push_back(browserDirDisplay[i]);
			}
			for (unsigned int i = 0; i < browserFiles.size(); i++) {
				tempTreeData.push_back(browserFiles[i]);
				tempTreeDisplay.push_back(browserFilesDisplay[i]);
			}
		} else {
			rootFound = false;
		}
	};

	void createCurrentFolder(std::string dir_path) {
		vector <std::string> browserList;
		vector <std::string> browserFiles;

		tempTreeData.clear();

		std::string lastChar = dir_path.substr(dir_path.length()-1,dir_path.length()-1);
		if (lastChar != "/")
			dir_path += "/";

		DIR *dir = opendir(dir_path.c_str());
		struct dirent *d;
		while ((d = readdir(dir))) {
			std::string filename = d->d_name;
			if (filename != "." && filename != "..") {
				std::string filepath = std::string(dir_path) + filename;

					std::size_t found = filename.find(".wav",filename.length()-5);
					if (found==std::string::npos)
						found = filename.find(".WAV",filename.length()-5);
					if (found!=std::string::npos) {
						browserFiles.push_back(filepath);
					}

			}
   		}
   		closedir(dir);

		sort(browserFiles.begin(), browserFiles.end());
		
		for (unsigned int i = 0; i < browserFiles.size(); i++) {
			tempTreeData.push_back(browserFiles[i]);
		}
	};

//	-----------------------------------------------------------------------------------------------

	float* LoadWavFileF32(const std::string& path, uint32_t* channels, uint32_t* sampleRate, uint64_t* totalSampleCount) {
	    drwav wav;
	    if (!drwav_init_file(&wav, path.c_str(), nullptr)) {
	        return nullptr;
	    }

	    if (channels) *channels = wav.channels;
	    if (sampleRate) *sampleRate = wav.sampleRate;

	    uint64_t frameCount = wav.totalPCMFrameCount;
	    uint64_t sampleCount = frameCount * wav.channels;

	    float* pSampleData = (float*)malloc((size_t)sampleCount * sizeof(float));
	    if (!pSampleData) {
	        drwav_uninit(&wav);
	        return nullptr;
	    }

	    uint64_t framesRead = drwav_read_pcm_frames_f32(&wav, frameCount, pSampleData);
	    drwav_uninit(&wav);

	    if (totalSampleCount) *totalSampleCount = framesRead * wav.channels;

	    return pSampleData;
	}

// -------------------------------------------------------------------------------------------------------------------------------

	bool SaveWavFileF32(const std::string& path, const std::vector<float>& data, uint32_t sampleRate, uint32_t channels) {
	    drwav_data_format format;
	    format.container = drwav_container_riff;      // Standard WAV
	    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;    // Float 32-bit
	    format.channels = channels;
	    format.sampleRate = sampleRate;
	    format.bitsPerSample = 32;

	    drwav wav;
	    if (!drwav_init_file_write(&wav, path.c_str(), &format, nullptr)) {
	        return false;
	    }

	    drwav_uint64 framesToWrite = data.size() / channels;

	    // Scrivi i frame
	    drwav_uint64 framesWritten = drwav_write_pcm_frames(&wav, framesToWrite, data.data());

	    drwav_uninit(&wav);

	    return framesWritten == framesToWrite;
	}

// -------------------------------------------------------------------------------------------------------------------------------


	void menuLoadSample() {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV;All files (*.*):*.*";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		fileLoaded = false;
		restoreLoadFromPatch = false;
		if (path) {
			loadFromPatch = false;
			loadSample(path);
			storedPath = std::string(path);
		} else {
			restoreLoadFromPatch = true;
			fileLoaded = true;
		}
		if (storedPath == "" || fileFound == false) {
			fileLoaded = false;
		}
		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadSample(std::string fromPath) {
		std::string path = fromPath;
		z1 = 0; z2 = 0; z1r = 0; z2r = 0;
		//unsigned int c;
		//unsigned int sr;
		//drwav_uint64 tsc;
		uint32_t c;
		uint32_t sr;
		uint64_t tsc;
		//float* pSampleData;
		//pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &tsc);
		float* pSampleData = LoadWavFileF32(path.c_str(), &c, &sr, &tsc);	// new dr_wav lib

		if (pSampleData != NULL && tsc > minSamplesToLoad * c) {
			fileFound = true;
			channels = c;
			sampleRate = sr * 2;
			calcBiquadLpf(20000.0, sampleRate, 1);
			
			for (int c=0; c < 16; c++)
				samplePos[c] = 0;
			playBuffer[LEFT][0].clear();
			playBuffer[LEFT][1].clear();
			playBuffer[RIGHT][0].clear();
			playBuffer[RIGHT][1].clear();
			displayBuff.clear();

			/*
			if (tsc > 52428800)
				tsc = 52428800;	// set memory allocation limit to 200Mb for samples (~18mins at 48.000khz MONO)
			*/
			if (!unlimitedRecording) {
				if (tsc > recordingLimit / 2)
					tsc = recordingLimit / 2;	// set memory allocation limit to 200Mb for samples (~18mins at 48.000khz MONO)
			}


			for (unsigned int i=0; i < tsc; i = i + c) {
				playBuffer[LEFT][0].push_back(pSampleData[i] * 5);
				playBuffer[LEFT][0].push_back(0);
				if (channels == 2) {
					playBuffer[RIGHT][0].push_back(pSampleData[i+1] * 5);
					playBuffer[RIGHT][0].push_back(0);
				}
			}
			totalSampleC = playBuffer[LEFT][0].size();
			totalSamples = totalSampleC-1;
//			drwav_free(pSampleData);
			for (unsigned int i = 1; i < totalSamples; i = i+2) {		// averaging oversampled vector
				playBuffer[LEFT][0][i] = playBuffer[LEFT][0][i-1] * .5f + playBuffer[LEFT][0][i+1] * .5f;
				if (channels == 2)
					playBuffer[RIGHT][0][i] = playBuffer[RIGHT][0][i-1] * .5f + playBuffer[RIGHT][0][i+1] * .5f;
			}
			
			playBuffer[LEFT][0][totalSamples] = playBuffer[LEFT][0][totalSamples-1] * .5f; // halve the last sample
			if (channels == 2)
				playBuffer[RIGHT][0][totalSamples] = playBuffer[RIGHT][0][totalSamples-1] * .5f;

			for (unsigned int i = 0; i < totalSampleC; i++) {	// populating filtered vector
				playBuffer[LEFT][1].push_back(biquadLpf(playBuffer[LEFT][0][i]));
				if (channels == 2)
					playBuffer[RIGHT][1].push_back(biquadLpf2(playBuffer[RIGHT][0][i]));
			}

			sampleCoeff = sampleRate / (APP->engine->getSampleRate());			// the % distance between samples at 1x speed

			prevKnobCueStartPos = -1.f;
			prevKnobCueEndPos = 2.f;
			prevKnobLoopStartPos = -1.f;
			prevKnobLoopEndPos = 2.f;

			vector<double>().swap(displayBuff);		// creating the display vector
			for (int i = 0; i < floor(totalSampleC); i = i + floor(totalSampleC/240))
				displayBuff.push_back(playBuffer[0][0][i]);

			seconds = totalSampleC / sampleRate;
			minutes = (seconds / 60) % 60;
			seconds = seconds % 60;

			timeDisplay = std::to_string(minutes) + ":";
			if (seconds < 10)
				timeDisplay += "0";
			timeDisplay += std::to_string(seconds);

			if (loadFromPatch)
				path = storedPath;

			char* pathDup = strdup(path.c_str());
			fileDescription = basename(pathDup);

			if (loadFromPatch)
				fileDescription = "(!)"+fileDescription;

			// *** CHARs CHECK according to font
			std::string tempFileDisplay = fileDescription.substr(0, fileDescription.size()-4);
			char tempFileChar;
			fileDisplay = "";
			for (int i = 0; i < int(tempFileDisplay.length()); i++) {
				tempFileChar = tempFileDisplay.at(i);
				if ( (int(tempFileChar) > 47 && int(tempFileChar) < 58)
					|| (int(tempFileChar) > 64 && int(tempFileChar) < 123)
					|| int(tempFileChar) == 45 || int(tempFileChar) == 46 || int(tempFileChar) == 95 )
					fileDisplay += tempFileChar;
			}

			fileDisplay = fileDisplay.substr(0, 20);
			samplerateDisplay = std::to_string(int(sampleRate * .5));
			channelsDisplay = std::to_string(channels) + "Ch";

			free(pathDup);
			storedPath = path;

			if (!loadFromPatch) {
				currentFolder = system::getDirectory(path);
				createCurrentFolder(currentFolder);
				currentFolderV.clear();
				currentFolderV = tempTreeData;
				for (unsigned int i = 0; i < currentFolderV.size(); i++) {
					if (system::getFilename(path) == system::getFilename(currentFolderV[i])) {
						currentFile = i;
						i = currentFolderV.size();
					}
				}
			}

			if (!firstLoad) {
				prevKnobCueStartPos = -1.f;
				prevKnobCueEndPos = 2.f;
				prevKnobLoopStartPos = -1.f;
				prevKnobLoopEndPos = 2.f;	

				if (resetCursorsOnLoad) {
					params[CUESTART_PARAM].setValue(0.f);
					params[CUEEND_PARAM].setValue(1.f);
					params[LOOPSTART_PARAM].setValue(0.f);
					params[LOOPEND_PARAM].setValue(1.f);
					knobCueStartPos = 0.f;
					knobCueEndPos = 1.f;
					knobLoopStartPos = 0.f;
					knobLoopEndPos = 1.f;
				}
			}

			firstLoad = false;

			fileLoaded = true;
			
		} else {
			fileFound = false;
			fileLoaded = false;
			//storedPath = path;
			if (loadFromPatch)
				path = storedPath;

			fileDescription = "(!)"+path;
			fileDisplay = "";
			timeDisplay = "";
			channelsDisplay = "";
		}
	};

	void saveSample(std::string path) {
		drwav_uint64 samples;

		samples = playBuffer[LEFT][0].size();

		std::vector<float> data;

		for (unsigned int i = 0; i <= playBuffer[LEFT][0].size(); i = i + 2) {
			data.push_back(playBuffer[LEFT][0][i] / 5);
			if (channels == 2)
				data.push_back(playBuffer[RIGHT][0][i] / 5);
		}

		drwav_data_format format;
		format.container = drwav_container_riff;
		format.format = DR_WAVE_FORMAT_IEEE_FLOAT;

		format.channels = channels;

		if (channels == 1)
			samples /= 2;

		format.sampleRate = sampleRate / 2;

		format.bitsPerSample = 32;

		//if (path.substr(path.size() - 4) != ".wav" and path.substr(path.size() - 4) != ".WAV")
		if (path.substr(path.size() - 4) != ".wav" && path.substr(path.size() - 4) != ".WAV")
			path += ".wav";

		/*
		drwav *pWav = drwav_open_file_write(path.c_str(), &format);
		drwav_write(pWav, samples, data.data());
		drwav_close(pWav);
		*/
		bool ok = SaveWavFileF32(path.c_str(), data, format.sampleRate, format.channels);
		if (!ok) {
		    // std::cerr << "Errore durante il salvataggio WAV" << std::endl;
		    INFO("ERROR WRITING");
		}

		data.clear();
		
	}

	void clearSlot() {
		fileLoaded = false;
		fileFound = false;
		storedPath = "";
		fileDescription = "--none--";
		fileDisplay = "";
		timeDisplay = "";
		channelsDisplay = "";
		loadFromPatch = false;
		restoreLoadFromPatch = false;
		playBuffer[LEFT][0].clear();
		playBuffer[RIGHT][0].clear();
		playBuffer[LEFT][1].clear();
		playBuffer[RIGHT][1].clear();
		totalSampleC = 0;
		totalSamples = 0;
	}

	void resetCursors() {
		params[CUESTART_PARAM].setValue(0);
		params[CUEEND_PARAM].setValue(1);
		params[LOOPSTART_PARAM].setValue(0);
		params[LOOPEND_PARAM].setValue(1);
	}

	void copyCueToLoop() {
		loopStartPos = cueStartPos;
		loopEndPos = cueEndPos;
		params[LOOPSTART_PARAM].setValue(params[CUESTART_PARAM].getValue());
		params[LOOPEND_PARAM].setValue(params[CUEEND_PARAM].getValue());
		prevKnobLoopStartPos = params[CUESTART_PARAM].getValue();
		prevKnobLoopEndPos = params[CUEEND_PARAM].getValue();
		knobLoopStartPos = params[CUESTART_PARAM].getValue();
		knobLoopEndPos = params[CUEEND_PARAM].getValue();
	}
	
	void copyLoopToCue() {
		cueStartPos = loopStartPos;
		cueEndPos = loopEndPos;
		params[CUESTART_PARAM].setValue(params[LOOPSTART_PARAM].getValue());
		params[CUEEND_PARAM].setValue(params[LOOPEND_PARAM].getValue());
		prevKnobCueStartPos = params[LOOPSTART_PARAM].getValue();
		prevKnobCueEndPos = params[LOOPEND_PARAM].getValue();
		knobCueStartPos = params[LOOPSTART_PARAM].getValue();
		knobCueEndPos = params[LOOPEND_PARAM].getValue();
	}

	void setPreset(int presetNo) {
		switch (presetNo) {
			case 0:	// wavetable
				phaseScan = false;
				params[TRIGGATEMODE_SWITCH].setValue(GATE_MODE);
				
				params[REV_PARAM].setValue(0.f);
				params[XFADE_PARAM].setValue(0.f);
				params[LOOP_PARAM].setValue(1.f);
				params[PINGPONG_PARAM].setValue(0.f);

				params[CUESTART_PARAM].setValue(0.f);
				params[CUEEND_PARAM].setValue(1.f);
				params[LOOPSTART_PARAM].setValue(0.f);
				params[LOOPEND_PARAM].setValue(1.f);
				prevKnobCueStartPos = -1.f;
				prevKnobCueEndPos = 2.f;
				prevKnobLoopStartPos = -1.f;
				prevKnobLoopEndPos = 2.f;
			break;

			case 1:	// trig with envelope
				phaseScan = true;
				params[TRIGGATEMODE_SWITCH].setValue(TRIG_MODE);
				params[TRIGMODE_SWITCH].setValue(START_STOP);

				params[XFADE_PARAM].setValue(0.f);
				params[LOOP_PARAM].setValue(0.f);
				params[PINGPONG_PARAM].setValue(0.f);

				params[ATTACK_PARAM].setValue(0.33f);
				params[DECAY_PARAM].setValue(0.f);
				params[SUSTAIN_PARAM].setValue(1.f);
				params[RELEASE_PARAM].setValue(0.33f);

				prevKnobCueStartPos = -1.f;
				prevKnobCueEndPos = 2.f;
				prevKnobLoopStartPos = -1.f;
				prevKnobLoopEndPos = 2.f;
			break;

			case 2:	// drum player
				phaseScan = false;
				params[TRIGGATEMODE_SWITCH].setValue(TRIG_MODE);
				params[TRIGMODE_SWITCH].setValue(START_RESTART);

				params[REV_PARAM].setValue(0.f);
				params[XFADE_PARAM].setValue(0.0001f);
				params[LOOP_PARAM].setValue(0.f);
				params[PINGPONG_PARAM].setValue(0.f);

				params[ATTACK_PARAM].setValue(0.f);
				params[DECAY_PARAM].setValue(0.f);
				params[SUSTAIN_PARAM].setValue(1.f);
				params[RELEASE_PARAM].setValue(0.f);

				params[CUESTART_PARAM].setValue(0.f);
				params[CUEEND_PARAM].setValue(1.f);
				params[LOOPSTART_PARAM].setValue(0.f);
				params[LOOPEND_PARAM].setValue(1.f);
				prevKnobCueStartPos = -1.f;
				prevKnobCueEndPos = 2.f;
				prevKnobLoopStartPos = -1.f;
				prevKnobLoopEndPos = 2.f;
			break;
		}
	}

	bool isPolyOuts() {
		return polyOuts;
	}

	void setPolyOuts(bool poly) {
		if (poly) 
			polyOuts = 1;
		else {
			polyOuts = 0;
			outputs[OUT_OUTPUT].setChannels(1);
			outputs[OUT_OUTPUT+1].setChannels(1);
			outputs[EOC_OUTPUT].setChannels(1);
			outputs[EOR_OUTPUT].setChannels(1);
		}
	}
	
	void process(const ProcessArgs &args) override {

		//if (!disableNav) {
		if (!disableNav && !loadFromPatch) {
			nextSample = params[NEXTSAMPLE_PARAM].getValue();
			if (fileLoaded && nextSample && !prevNextSample) {
				for (int i = 0; i < 16; i++)
					play[i] = false;
				currentFile++;
				if (currentFile >= int(currentFolderV.size()))
					currentFile = 0;
				loadSample(currentFolderV[currentFile]);
			}
			prevNextSample = nextSample;

			prevSample = params[PREVSAMPLE_PARAM].getValue();
			if (fileLoaded && prevSample && !prevPrevSample) {
				for (int i = 0; i < 16; i++)
					play[i] = false;
				currentFile--;
				if (currentFile < 0)
					currentFile = currentFolderV.size()-1;
				loadSample(currentFolderV[currentFile]);
			}
			prevPrevSample = prevSample;
		}

		reverseStart = params[REV_PARAM].getValue();
		lights[REV_LIGHT].setBrightness(reverseStart);

		pingpong = params[PINGPONG_PARAM].getValue();
		lights[PINGPONG_LIGHT].setBrightness(pingpong);

		loop = params[LOOP_PARAM].getValue();
		lights[LOOP_LIGHT].setBrightness(loop);
		chan = std::max(1, inputs[VO_INPUT].getChannels());

		trigButValue = params[TRIGBUT_PARAM].getValue();
		lights[TRIGBUT_LIGHT].setBrightness(trigButValue);

		stopButValue = params[STOPBUT_PARAM].getValue();
		lights[STOPBUT_LIGHT]. setBrightness(stopButValue);
		
		if (!fileLoaded) {

			for (int c = 0; c < chan; c++) {
				play[c] = false;
				fadingType[c] = NO_FADE;
				stage[c] = STOP_STAGE;
				stageLevel[c] = 0;
				voct[c] = 0.f;
				prevVoct[c] = 11.f;
				outputs[OUT_OUTPUT].setVoltage(0, c);
				outputs[OUT_OUTPUT+1].setVoltage(0, c);
			}
			
		} else {
			
			knobCueEndPos = params[CUEEND_PARAM].getValue();
			if (knobCueEndPos != prevKnobCueEndPos) {
				if (knobCueEndPos < prevKnobCueEndPos)
					cueEndPos = floor(totalSamples * knobCueEndPos);
				else
					cueEndPos = ceil(totalSamples * knobCueEndPos);
				prevKnobCueEndPos = knobCueEndPos;
				searchingCueEndPhase = true;
				scanCueEndSample = cueEndPos;
				if (cueEndPos < cueStartPos)
					cueEndPos = cueStartPos;
			}
			knobCueStartPos = params[CUESTART_PARAM].getValue();
			if (knobCueStartPos != prevKnobCueStartPos) {
				if (knobCueStartPos < prevKnobCueStartPos)
					cueStartPos = floor(totalSamples * knobCueStartPos);
				else
					cueStartPos = ceil(totalSamples * knobCueStartPos);
				prevKnobCueStartPos = knobCueStartPos;
				searchingCueStartPhase = true;
				scanCueStartSample = cueStartPos;
				if (cueStartPos > cueEndPos)
					cueStartPos = cueEndPos;
			}
			knobLoopEndPos = params[LOOPEND_PARAM].getValue();
			if (knobLoopEndPos != prevKnobLoopEndPos) {
				if (knobLoopEndPos < prevKnobLoopEndPos)
					loopEndPos = floor(totalSamples * knobLoopEndPos);
				else
					loopEndPos = ceil(totalSamples * knobLoopEndPos);
				prevKnobLoopEndPos = knobLoopEndPos;
				searchingLoopEndPhase = true;
				scanLoopEndSample = loopEndPos;
				if (loopEndPos < loopStartPos)
					loopEndPos = loopStartPos;
			}
			knobLoopStartPos = params[LOOPSTART_PARAM].getValue();
			if (knobLoopStartPos != prevKnobLoopStartPos) {
				if (knobLoopStartPos < prevKnobLoopStartPos)
					loopStartPos = floor(totalSamples * knobLoopStartPos);
				else
					loopStartPos = ceil(totalSamples * knobLoopStartPos);
				prevKnobLoopStartPos = knobLoopStartPos;
				searchingLoopStartPhase = true;
				scanLoopStartSample = loopStartPos;
				if (loopStartPos > loopEndPos)
					loopStartPos = loopEndPos;
			}

			if (phaseScan && !prevPhaseScan) {
				prevPhaseScan = true;
				searchingCueEndPhase = true;
				searchingCueStartPhase = true;
				searchingLoopEndPhase = true;
				searchingLoopStartPhase = true;
			}
			
			if (phaseScan) {
				float tempKnob;
				if (searchingCueEndPhase) {
					if (playBuffer[LEFT][antiAlias][scanCueEndSample-1] <= 0 && playBuffer[LEFT][antiAlias][scanCueEndSample] >= 0) {
						cueEndPos = scanCueEndSample;
						searchingCueEndPhase = false;
						tempKnob = cueEndPos/totalSamples;
						params[CUEEND_PARAM].setValue(tempKnob);
						prevKnobCueEndPos = tempKnob;
					} else {
						scanCueEndSample--;
						if (scanCueEndSample < cueStartPos) {
							cueEndPos = cueStartPos;
							searchingCueEndPhase = false;
							tempKnob = cueEndPos/totalSamples;
							params[CUEEND_PARAM].setValue(tempKnob);
							prevKnobCueEndPos = tempKnob;
						}
					}
				}
				if (searchingCueStartPhase) {
					if (playBuffer[LEFT][antiAlias][scanCueStartSample+1] >= 0 && playBuffer[LEFT][antiAlias][scanCueStartSample] <= 0) {
						cueStartPos = scanCueStartSample;
						searchingCueStartPhase = false;
						tempKnob = cueStartPos/totalSamples;
						params[CUESTART_PARAM].setValue(tempKnob);
						prevKnobCueStartPos = tempKnob;
					} else {
						scanCueStartSample++;
						if (scanCueStartSample > cueEndPos) {
							cueStartPos = cueEndPos;
							searchingCueStartPhase = false;
							tempKnob = cueStartPos/totalSamples;
							params[CUESTART_PARAM].setValue(tempKnob);
							prevKnobCueStartPos = tempKnob;
						}
					}
				}
				if (searchingLoopEndPhase) {
					if (playBuffer[LEFT][antiAlias][scanLoopEndSample-1] <= 0 && playBuffer[LEFT][antiAlias][scanLoopEndSample] >= 0) {
						loopEndPos = scanLoopEndSample;
						searchingLoopEndPhase = false;
						tempKnob = loopEndPos/totalSamples;
						params[LOOPEND_PARAM].setValue(tempKnob);
						prevKnobLoopEndPos = tempKnob;
					} else {
						scanLoopEndSample--;
						if (scanLoopEndSample < loopStartPos) {
							loopEndPos = loopStartPos;
							searchingLoopEndPhase = false;
							tempKnob = loopEndPos/totalSamples;
							params[LOOPEND_PARAM].setValue(tempKnob);
							prevKnobLoopEndPos = tempKnob;
						}
					}
				}
				if (searchingLoopStartPhase) {
					if (playBuffer[LEFT][antiAlias][scanLoopStartSample+1] >= 0 && playBuffer[LEFT][antiAlias][scanLoopStartSample] <= 0) {
						loopStartPos = scanLoopStartSample;
						searchingLoopStartPhase = false;
						tempKnob = loopStartPos/totalSamples;
						params[LOOPSTART_PARAM].setValue(tempKnob);
						prevKnobLoopStartPos = tempKnob;
					} else {
						scanLoopStartSample++;
						if (scanLoopStartSample > loopEndPos) {
							loopStartPos = loopEndPos;
							searchingLoopStartPhase = false;
							tempKnob = loopStartPos/totalSamples;
							params[LOOPSTART_PARAM].setValue(tempKnob);
							prevKnobLoopStartPos = tempKnob;
						}
					}
				}
				
			} else {
				prevPhaseScan = false;
			}

			trigMode = params[TRIGGATEMODE_SWITCH].getValue();
			trigType = params[TRIGMODE_SWITCH].getValue();
			
			sustainValue = params[SUSTAIN_PARAM].getValue() + (inputs[SUSTAIN_INPUT].getVoltage() * params[SUSTAINATNV_PARAM].getValue() * 0.1);
			if (sustainValue > 1)
				sustainValue = 1;
			else if (sustainValue < 0)
				sustainValue = 0;
			
			limiter = params[LIMIT_SWITCH].getValue();

			tune = params[TUNE_PARAM].getValue() + (inputs[TUNE_INPUT].getVoltage() * params[TUNEATNV_PARAM].getValue() * 0.2);
			if (tune != prevTune) {
				prevTune = tune;
				currentSpeed = double(powf(2,tune));
				if (currentSpeed > 4)
					currentSpeed = 4;
				else if (currentSpeed < 0.25)
					currentSpeed = 0.25;
			}

			sumOutput = 0;
			sumOutputR = 0;

			xFade = params[XFADE_PARAM].getValue();

			if (xFade != prevXfade) {
				if (xFade == 0)
					fadeSamples = 0;
				else
					fadeSamples = floor(convertCVToSeconds(xFade) * args.sampleRate); // number of samples before starting fade

				prevXfade = xFade;
			}

			// START CHANNEL MANAGEMENT

			voctDisplay = 100.f;

			for (int c = 0; c < chan; c++) {
				
				if (polyMaster) 
					masterLevel[c] = params[VOL_PARAM].getValue() + (inputs[VOL_INPUT].getVoltage(c) * params[VOLATNV_PARAM].getValue() * 0.2);
				else
					masterLevel[c] = params[VOL_PARAM].getValue() + (inputs[VOL_INPUT].getVoltage(0) * params[VOLATNV_PARAM].getValue() * 0.2);
				
				if (masterLevel[c] > 2)
					masterLevel[c] = 2;
				else if (masterLevel[c] < 0)
					masterLevel[c] = 0;

				if (c == 0 && trigButValue)
					trigValue[c] = 1;
				else
					trigValue[c] = inputs[TRIG_INPUT].getVoltage(c);

				switch (trigMode) {
					case GATE_MODE:												// ***** GATE MODE *****
						if (trigValue[c] >= 1) {
							if (!play[c]) {
								play[c] = true;
								if (reverseStart) {
									reversePlaying[c] = REVERSE;
									samplePos[c] = floor(cueEndPos-1);
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = floor(cueEndPos);
									prevSampleWeight[c] = 0;
								} else {
									reversePlaying[c] = FORWARD;
									samplePos[c] = floor(cueStartPos+1);
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = floor(cueStartPos);
									prevSampleWeight[c] = 0;
								}
								stage[c] = ATTACK_STAGE;
								attackValue =  convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
								if (attackValue > maxAdsrTime) {
									attackValue = maxAdsrTime;
								} else if (attackValue < minAdsrTime) {
									attackValue = minAdsrTime;
								}
								stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);
							} else {
								if (stage[c] == RELEASE_STAGE) {
									stage[c] = ATTACK_STAGE;
									attackValue =  convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
									if (attackValue > maxAdsrTime) {
										attackValue = maxAdsrTime;
									} else if (attackValue < minAdsrTime) {
										attackValue = minAdsrTime;
									}
									stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);
								}
							}
						} else {
							if (play[c]) {
								if (stage[c] != RELEASE_STAGE) {
									stage[c]=RELEASE_STAGE;
									releaseValue =  convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
									if (releaseValue > maxAdsrTime) {
										releaseValue = maxAdsrTime;
									} else 	if (releaseValue < minAdsrTime) {
										releaseValue = minAdsrTime;
									}
									stageCoeff[c] = stageLevel[c] / (args.sampleRate * releaseValue);

									if (eocFromTrg) {
										if (polyOuts) {
											eoc[c] = true;
											eocTime[c] = oneMsSamples;
										} else {
											if (c == currentDisplay) {
												eoc[0] = true;
												eocTime[0] = oneMsSamples;
											}
										}
									}
								}
							}
						}
					break;

					case TRIG_MODE:												// ***** TRIG MODE *****
						if (trigValue[c] >= 1 && prevTrigValue[c] < 1){
							switch (trigType) {
								case START_STOP:									// trig type: Start/Stop
									if (play[c]) {
										if (!inputs[STOP_INPUT].isConnected()) {
											if (stage[c] != RELEASE_STAGE) {
												stage[c]=RELEASE_STAGE;
												releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
												if (releaseValue > maxAdsrTime) {
													releaseValue = maxAdsrTime;
												} else 	if (releaseValue < minAdsrTime) {
													releaseValue = minAdsrTime;
												}
												stageCoeff[c] = stageLevel[c] / (args.sampleRate * releaseValue);
												
												if (eocFromTrg) {
													if (polyOuts) {
														eoc[c] = true;
														eocTime[c] = oneMsSamples;
													} else {
														if (c == currentDisplay) {
															eoc[0] = true;
															eocTime[0] = oneMsSamples;
														}
													}
												}
											} else {
												stage[c] = ATTACK_STAGE;
												attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
												if (attackValue > maxAdsrTime) {
													attackValue = maxAdsrTime;
												} else if (attackValue < minAdsrTime) {
													attackValue = minAdsrTime;
												}
												stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);
											}
										}
									} else {
										play[c] = true;
										if (reverseStart) {
											reversePlaying[c] = REVERSE;
											samplePos[c] = floor(cueEndPos-1);
											currSampleWeight[c] = sampleCoeff;
											prevSamplePos[c] = floor(cueEndPos);
											prevSampleWeight[c] = 0;
										} else {
											reversePlaying[c] = FORWARD;
											samplePos[c] = floor(cueStartPos+1);
											currSampleWeight[c] = sampleCoeff;
											prevSamplePos[c] = floor(cueStartPos);
											prevSampleWeight[c] = 0;
										}
										stage[c] = ATTACK_STAGE;
										attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
										if (attackValue > maxAdsrTime) {
											attackValue = maxAdsrTime;
										} else if (attackValue < minAdsrTime) {
											attackValue = minAdsrTime;
										}
										stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);
									}
								break;

								case START_RESTART:									// trig type: START RESTART
									if (!play[c]) {
										play[c] = true;
										if (reverseStart) {
											reversePlaying[c] = REVERSE;
											samplePos[c] = floor(cueEndPos-1);
											currSampleWeight[c] = sampleCoeff;
											prevSamplePos[c] = floor(cueEndPos);
											prevSampleWeight[c] = 0;
										} else {
											reversePlaying[c] = FORWARD;
											samplePos[c] = floor(cueStartPos+1);
											currSampleWeight[c] = sampleCoeff;
											prevSamplePos[c] = floor(cueStartPos);
											prevSampleWeight[c] = 0;
										}
										stage[c] = ATTACK_STAGE;
										attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
										if (attackValue > maxAdsrTime) {
											attackValue = maxAdsrTime;
										} else if (attackValue < minAdsrTime) {
											attackValue = minAdsrTime;
										}
										stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);

									} else {
										fadingValue[c] = 1.f;
										fadedPosition[c] = samplePos[c];
										if (fadeSamples)
											fadeCoeff = 1 / convertCVToSeconds(xFade) / args.sampleRate;
										else
											fadeCoeff = 1;
										fadingType[c] = CROSS_FADE;
										if (reverseStart) {
											reversePlaying[c] = REVERSE;
											samplePos[c] = floor(cueEndPos-1);
											currSampleWeight[c] = sampleCoeff;
											prevSamplePos[c] = floor(cueEndPos);
											prevSampleWeight[c] = 0;
										} else {
											reversePlaying[c] = FORWARD;
											samplePos[c] = floor(cueStartPos+1);
											currSampleWeight[c] = sampleCoeff;
											prevSamplePos[c] = floor(cueStartPos);
											prevSampleWeight[c] = 0;
										}
										stage[c] = ATTACK_STAGE;
										attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
										if (attackValue > maxAdsrTime) {
											attackValue = maxAdsrTime;
										} else if (attackValue < minAdsrTime) {
											attackValue = minAdsrTime;
										}
										stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);

									}
								break;
								
								case PLAY_PAUSE:									// trig type: Play/Pause
									if (play[c]) {
										if (stage[c] != RELEASE_STAGE) {
											stage[c] = RELEASE_STAGE;
											releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
											if (releaseValue > maxAdsrTime) {
												releaseValue = maxAdsrTime;
											} else 	if (releaseValue < minAdsrTime) {
												releaseValue = minAdsrTime;
											}
											stageCoeff[c] = stageLevel[c] / (args.sampleRate * releaseValue);

											if (eocFromTrg) {
												if (polyOuts) {
													eoc[c] = true;
													eocTime[c] = oneMsSamples;
												} else {
													if (c == currentDisplay) {
														eoc[0] = true;
														eocTime[0] = oneMsSamples;
													}
												}
											}
										} else {
											stage[c] = ATTACK_STAGE;
											attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
											if (attackValue > maxAdsrTime) {
												attackValue = maxAdsrTime;
											} else if (attackValue < minAdsrTime) {
												attackValue = minAdsrTime;
											}
											stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);
										}
									} else {
										play[c] = true;
										if (reverseStart) {
											reversePlaying[c] = REVERSE;
											if (samplePos[c] < cueStartPos || samplePos[c] > cueEndPos) {
												samplePos[c] = floor(cueEndPos-1);
												currSampleWeight[c] = sampleCoeff;
												prevSamplePos[c] = floor(cueEndPos);
												prevSampleWeight[c] = 0;
											}
										} else {
											reversePlaying[c] = FORWARD;
											if (samplePos[c] < cueStartPos || samplePos[c] > cueEndPos) {
												samplePos[c] = floor(cueStartPos+1);
												currSampleWeight[c] = sampleCoeff;
												prevSamplePos[c] = floor(cueStartPos);
												prevSampleWeight[c] = 0;
											}
										}
										stage[c] = ATTACK_STAGE;
										attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
										if (attackValue > maxAdsrTime) {
											attackValue = maxAdsrTime;
										} else if (attackValue < minAdsrTime) {
											attackValue = minAdsrTime;
										}
										stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);
									}
								break;
							}
						}

						// ************************************************* STOP INPUT MANAGEMENT

						if (c == 0 && stopButValue)
							stopValue[c] = 1;
						else
							stopValue[c] = inputs[STOP_INPUT].getVoltage(c);							

						if (stopValue[c] >= 1 && prevStopValue[c] < 1 && trigMode) {

							if (trigMode == TRIG_MODE && trigType == PLAY_PAUSE) {
								if (play[c]) {
									playPauseToStop[c] = true;
								} else {
									if (reverseStart == FORWARD) {
										samplePos[c] = floor(cueStartPos)+1;
										currSampleWeight[c] = sampleCoeff;
										prevSamplePos[c] = floor(cueStartPos);
										prevSampleWeight[c] = 0;
									} else {
										samplePos[c] = floor(cueEndPos)-1;
										currSampleWeight[c] = sampleCoeff;
										prevSamplePos[c] = floor(cueEndPos);
										prevSampleWeight[c] = 0;
									}
								}
							}

							if (stage[c] != RELEASE_STAGE) {
								stage[c] = RELEASE_STAGE;
								releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
								if (releaseValue > maxAdsrTime) {
									releaseValue = maxAdsrTime;
								} else 	if (releaseValue < minAdsrTime) {
									releaseValue = minAdsrTime;
								}
								stageCoeff[c] = stageLevel[c] / (args.sampleRate * releaseValue);
							}
							if (eocFromStop && play[c]) {
								if (polyOuts) {
									eoc[c] = true;
									eocTime[c] = oneMsSamples;
								} else {
									if (c == currentDisplay) {
										eoc[0] = true;
										eocTime[0] = oneMsSamples;
									}
								}
							}
						}
						prevStopValue[c] = stopValue[c];

					break;
				}
				prevTrigValue[c] = trigValue[c];

				currentOutput = 0;
				currentOutputR = 0;

				if (play[c]) {
					if (inputs[VO_INPUT].isConnected()) {
						voct[c] = inputs[VO_INPUT].getVoltage(c);
						if (voct[c] != prevVoct[c]) {
							speedVoct[c] = pow(2,voct[c]);
							prevVoct[c] = voct[c];
						}
						distancePos[c] = currentSpeed * sampleCoeff * speedVoct[c];
					} else
						distancePos[c] = currentSpeed * sampleCoeff;

					//if (play[c] && voct[c] < voctDisplay) {
					if (voct[c] < voctDisplay) {
						currentDisplay = c;
						voctDisplay = voct[c];
					}

					switch (reversePlaying[c]) {
						case FORWARD:		// FORWARD PLAYBACK
							if (loop && samplePos[c] > floor(loopEndPos - (fadeSamples * distancePos[c]))) {		// *** REACHED END OF LOOP ***
								fadingValue[c] = 1.f;
								fadedPosition[c] = samplePos[c];
								if (fadeSamples)
									fadeCoeff = 1 / convertCVToSeconds(xFade) / args.sampleRate;
								else
									fadeCoeff = 1;

								if (pingpong) {
									fadingType[c] = PINGPONG_FADE;
									reversePlaying[c] = REVERSE;
									samplePos[c] = floor(loopEndPos)-1;
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = floor(loopEndPos);
									prevSampleWeight[c] = 0;
									if (eocFromPing) {
										if (polyOuts) {
											eoc[c] = true;
											eocTime[c] = oneMsSamples;
										} else {
											if (c == currentDisplay) {
												eoc[0] = true;
												eocTime[0] = oneMsSamples;
											}
										}
									}
								} else {
									fadingType[c] = CROSS_FADE;
									samplePos[c] = floor(loopStartPos)+1;
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = floor(loopStartPos);
									prevSampleWeight[c] = 0;
									if (eocFromLoopEnd) {
										if (polyOuts) {
											eoc[c] = true;
											eocTime[c] = oneMsSamples;
										} else {
											if (c == currentDisplay) {
												eoc[0] = true;
												eocTime[0] = oneMsSamples;
											}
										}
									}
								}

							} else if (!fadingType[c] && floor(samplePos[c]) > (totalSamples - (fadeSamples * distancePos[c]))) {
								fadingType[c] = FADE_OUT;
								fadingValue[c] = 1.f;
								fadedPosition[c] = samplePos[c];
								if (fadeSamples)
									fadeCoeff = 1 / convertCVToSeconds(xFade) / args.sampleRate;
								else
									fadeCoeff = 1;
	
							} else if (floor(samplePos[c]) > totalSamples) {	// *** REACHED END OF SAMPLE ***
								if (eocFromCueEnd) {
									if (polyOuts) {
										eoc[c] = true;
										eocTime[c] = oneMsSamples;
									} else {
										if (c == currentDisplay) {
											eoc[0] = true;
											eocTime[0] = oneMsSamples;
										}
									}
								}
								play[c] = false;

							} else if (samplePos[c] > cueEndPos) {				// *** REACHED CUE END ***
								if (stage[c] != RELEASE_STAGE) {
									stage[c] = RELEASE_STAGE;
									releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
									if (releaseValue > maxAdsrTime) {
										releaseValue = maxAdsrTime;
									} else 	if (releaseValue < minAdsrTime) {
										releaseValue = minAdsrTime;
									}
									stageCoeff[c] = stageLevel[c] / (args.sampleRate * releaseValue);
									if (eocFromCueEnd) {
										if (polyOuts) {
											eoc[c] = true;
											eocTime[c] = oneMsSamples;
										} else {
											if (c == currentDisplay) {
												eoc[0] = true;
												eocTime[0] = oneMsSamples;
											}
										}
									}
								}
								if (trigMode == GATE_MODE) {
									if (pingpong) {
										reversePlaying[c] = REVERSE;
										samplePos[c] = floor(cueEndPos)-1;
										currSampleWeight[c] = sampleCoeff;
										prevSamplePos[c] = floor(cueEndPos);
										prevSampleWeight[c] = 0;
										if (eocFromPing) {
											if (polyOuts) {
												eoc[c] = true;
												eocTime[c] = oneMsSamples;
											} else {
												if (c == currentDisplay) {
													eoc[0] = true;
													eocTime[0] = oneMsSamples;
												}
											}
										}
									} else {
										fadingType[c] = CROSS_FADE;
										fadingValue[c] = 1.f;
										samplePos[c] = floor(cueStartPos)+1;
										currSampleWeight[c] = sampleCoeff;
										prevSamplePos[c] = floor(cueStartPos);
										prevSampleWeight[c] = 0;
										if (eocFromCueEnd) {
											if (polyOuts) {
												eoc[c] = true;
												eocTime[c] = oneMsSamples;
											} else {
												if (c == currentDisplay) {
													eoc[0] = true;
													eocTime[0] = oneMsSamples;
												}
											}
										}
									}
								}
							} 
						break;

						case REVERSE:		// REVERSE PLAYBACK
							if (loop && samplePos[c] < floor(loopStartPos + (fadeSamples * distancePos[c]))) {	// *** REACHED BEGIN OF LOOP ***
								fadingValue[c] = 1.f;
								fadedPosition[c] = samplePos[c];
								if (fadeSamples)
									fadeCoeff = 1 / convertCVToSeconds(xFade) / args.sampleRate;
								else
									fadeCoeff = 1;

								if (pingpong) {
									fadingType[c] = PINGPONG_FADE;
									reversePlaying[c] = FORWARD;
									samplePos[c] = floor(loopStartPos)+1;
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = floor(loopStartPos);
									prevSampleWeight[c] = 0;
									if (eocFromPong) {
										if (polyOuts) {
											eoc[c] = true;
											eocTime[c] = oneMsSamples;
										} else {
											if (c == currentDisplay) {
												eoc[0] = true;
												eocTime[0] = oneMsSamples;
											}
										}
									}
								} else {
									fadingType[c] = CROSS_FADE;
									samplePos[c] = floor(loopEndPos)-1;
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = floor(loopEndPos);
									prevSampleWeight[c] = 0;
									if (eocFromLoopStart) {
										if (polyOuts) {
											eoc[c] = true;
											eocTime[c] = oneMsSamples;
										} else {
											if (c == currentDisplay) {
												eoc[0] = true;
												eocTime[0] = oneMsSamples;
											}
										}
									}
								}

							} else if (!fadingType[c] && floor(samplePos[c]) < (fadeSamples * distancePos[c])) {
								fadingType[c] = FADE_OUT;
								fadingValue[c] = 1.f;
								fadedPosition[c] = samplePos[c];
								if (fadeSamples)
									fadeCoeff = 1 / convertCVToSeconds(xFade) / args.sampleRate;
								else
									fadeCoeff = 1;

							} else if (floor(samplePos[c]) < 0) {				// *** REACHED START OF SAMPLE ***
								if (eocFromCueEnd) {
									if (polyOuts) {
										eoc[c] = true;
										eocTime[c] = oneMsSamples;
									} else {
										if (c == currentDisplay) {
											eoc[0] = true;
											eocTime[0] = oneMsSamples;
										}
									}
								}
								play[c] = false;

							} else if (samplePos[c] < cueStartPos) {			// *** REACHED CUE START ***
								if (stage[c] != RELEASE_STAGE) {
									stage[c] = RELEASE_STAGE;
									releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
									if (releaseValue > maxAdsrTime) {
										releaseValue = maxAdsrTime;
									} else 	if (releaseValue < minAdsrTime) {
										releaseValue = minAdsrTime;
									}
									stageCoeff[c] = stageLevel[c] / (args.sampleRate * releaseValue);
									if (eocFromCueStart) {
										if (polyOuts) {
											eoc[c] = true;
											eocTime[c] = oneMsSamples;
										} else {
											if (c == currentDisplay) {
												eoc[0] = true;
												eocTime[0] = oneMsSamples;
											}
										}
									}
								}
								if (trigMode == GATE_MODE) {
									if (pingpong) {
										reversePlaying[c] = FORWARD;
										samplePos[c] = floor(cueStartPos)+1;
										currSampleWeight[c] = sampleCoeff;
										prevSamplePos[c] = floor(cueStartPos);
										prevSampleWeight[c] = 0;
										if (eocFromPong) {
											if (polyOuts) {
												eoc[c] = true;
												eocTime[c] = oneMsSamples;
											} else {
												if (c == currentDisplay) {
													eoc[0] = true;
													eocTime[0] = oneMsSamples;
												}
											}
										}
									} else {
										fadingType[c] = CROSS_FADE;
										fadingValue[c] = 1.f;
										fadedPosition[c] = samplePos[c];
										samplePos[c] = floor(cueEndPos)-1;
										currSampleWeight[c] = sampleCoeff;
										prevSamplePos[c] = floor(cueEndPos);
										prevSampleWeight[c] = 0;
										if (eocFromCueStart) {
											if (polyOuts) {
												eoc[c] = true;
												eocTime[c] = oneMsSamples;
											} else {
												if (c == currentDisplay) {
													eoc[0] = true;
													eocTime[0] = oneMsSamples;
												}
											}
										}
									}
								}
							}
						break;
					}

					if (play[c]) {									// it's false only if end of sample has reached, see above

						switch (interpolationMode) {
							case NO_INTERP:
								currentOutput = playBuffer[LEFT][antiAlias][floor(samplePos[c])];
								if (channels == 2)
									currentOutputR = playBuffer[RIGHT][antiAlias][floor(samplePos[c])];
							break;

							case LINEAR1_INTERP:
								if (currSampleWeight[c] == 0) {	// if no distance between samples, it means that speed is 1 and samplerates match -> no interpolation
									currentOutput = playBuffer[LEFT][antiAlias][floor(samplePos[c])];
									if (channels == 2)
										currentOutputR = playBuffer[RIGHT][antiAlias][floor(samplePos[c])];
								} else {							// weighted average of the actual previous and next sample to get the value of theoretical one
									currentOutput = (playBuffer[LEFT][antiAlias][floor(samplePos[c])] * (1-currSampleWeight[c])) +
													(playBuffer[LEFT][antiAlias][floor(samplePos[c])+1] * currSampleWeight[c]);
									if (channels == 2)
										currentOutputR = (playBuffer[RIGHT][antiAlias][floor(samplePos[c])] * (1-currSampleWeight[c])) +
													(playBuffer[RIGHT][antiAlias][floor(samplePos[c])+1] * currSampleWeight[c]);
								}

							break;

							case LINEAR2_INTERP:
								if (currSampleWeight[c] == 0) {	// if no distance between samples, it means that speed is 1 and samplerates match -> no interpolation
									currentOutput = playBuffer[LEFT][antiAlias][floor(samplePos[c])];
									if (channels == 2)
										currentOutputR = playBuffer[RIGHT][antiAlias][floor(samplePos[c])];
								} else {						// average of the weighted average of previous played sample and the weighted average of the current sample
									currentOutput = (
													(playBuffer[LEFT][antiAlias][floor(prevSamplePos[c])] * (1-prevSampleWeight[c])) +
													(playBuffer[LEFT][antiAlias][floor(prevSamplePos[c])+1] * prevSampleWeight[c]) +
													(playBuffer[LEFT][antiAlias][floor(samplePos[c])] * (1-currSampleWeight[c])) +
													(playBuffer[LEFT][antiAlias][floor(samplePos[c])+1] * currSampleWeight[c])
												) / 2;
									if (channels == 2)
										currentOutputR = (
														(playBuffer[RIGHT][antiAlias][floor(prevSamplePos[c])] * (1-prevSampleWeight[c])) +
														(playBuffer[RIGHT][antiAlias][floor(prevSamplePos[c])+1] * prevSampleWeight[c]) +
														(playBuffer[RIGHT][antiAlias][floor(samplePos[c])] * (1-currSampleWeight[c])) +
														(playBuffer[RIGHT][antiAlias][floor(samplePos[c])+1] * currSampleWeight[c])
													) / 2;
								}

							break;

							case HERMITE_INTERP:
								if (currSampleWeight[c] == 0) {	// if no distance between samples, it means that speed is 1 and samplerates match -> no interpolation
									currentOutput = playBuffer[LEFT][antiAlias][floor(samplePos[c])];
									if (channels == 2)
										currentOutputR = playBuffer[RIGHT][antiAlias][floor(samplePos[c])];
								} else {
									if (floor(samplePos[c]) > 0 && floor(samplePos[c]) < totalSamples - 1) {
										/*
										currentOutput = hermiteInterpol(playBuffer[i][antiAlias][floor(samplePos[i])-1],
																		playBuffer[i][antiAlias][floor(samplePos[i])],
																		playBuffer[i][antiAlias][floor(samplePos[i])+1],
																		playBuffer[i][antiAlias][floor(samplePos[i])+2],
																		currSampleWeight[i]);
										*/
										// below is translation of the above function
										double a1 = .5F * (playBuffer[LEFT][antiAlias][floor(samplePos[c])+1] - playBuffer[LEFT][antiAlias][floor(samplePos[c])-1]);
										double a2 = playBuffer[LEFT][antiAlias][floor(samplePos[c])-1] - (2.5F * playBuffer[LEFT][antiAlias][floor(samplePos[c])]) + (2 * playBuffer[LEFT][antiAlias][floor(samplePos[c])+1]) - (.5F * playBuffer[LEFT][antiAlias][floor(samplePos[c])+2]);
										double a3 = (.5F * (playBuffer[LEFT][antiAlias][floor(samplePos[c])+2] - playBuffer[LEFT][antiAlias][floor(samplePos[c])-1])) + (1.5F * (playBuffer[LEFT][antiAlias][floor(samplePos[c])] - playBuffer[LEFT][antiAlias][floor(samplePos[c])+1]));
										currentOutput = (((((a3 * currSampleWeight[c]) + a2) * currSampleWeight[c]) + a1) * currSampleWeight[c]) + playBuffer[LEFT][antiAlias][floor(samplePos[c])];
										if (channels == 2) {
											a1 = .5F * (playBuffer[RIGHT][antiAlias][floor(samplePos[c])+1] - playBuffer[RIGHT][antiAlias][floor(samplePos[c])-1]);
											a2 = playBuffer[RIGHT][antiAlias][floor(samplePos[c])-1] - (2.5F * playBuffer[RIGHT][antiAlias][floor(samplePos[c])]) + (2 * playBuffer[RIGHT][antiAlias][floor(samplePos[c])+1]) - (.5F * playBuffer[RIGHT][antiAlias][floor(samplePos[c])+2]);
											a3 = (.5F * (playBuffer[RIGHT][antiAlias][floor(samplePos[c])+2] - playBuffer[RIGHT][antiAlias][floor(samplePos[c])-1])) + (1.5F * (playBuffer[RIGHT][antiAlias][floor(samplePos[c])] - playBuffer[RIGHT][antiAlias][floor(samplePos[c])+1]));
											currentOutputR = (((((a3 * currSampleWeight[c]) + a2) * currSampleWeight[c]) + a1) * currSampleWeight[c]) + playBuffer[RIGHT][antiAlias][floor(samplePos[c])];
										}

									} else { // if playing sample is the first or one of the last 3 -> no interpolation
										currentOutput = playBuffer[LEFT][antiAlias][floor(samplePos[c])];
										if (channels == 2)
											currentOutputR = playBuffer[RIGHT][antiAlias][floor(samplePos[c])];
									}
								}
							break;
						}

						prevSamplePos[c] = samplePos[c];
						
						if (reversePlaying[c])
							samplePos[c] -= distancePos[c];
						else
							samplePos[c] += distancePos[c];

						if (interpolationMode > NO_INTERP) {
							prevSampleWeight[c] = currSampleWeight[c];
							currSampleWeight[c] = samplePos[c] - floor(samplePos[c]);
						}

						switch (stage[c]) {
							case ATTACK_STAGE:
								stageLevel[c] += stageCoeff[c];
								if (stageLevel[c] > 1) {
									stageLevel[c] = 1;
									stage[c] = DECAY_STAGE;
									decayValue = convertCVToSeconds(params[DECAY_PARAM].getValue()) + (inputs[DECAY_INPUT].getVoltage() * params[DECAYATNV_PARAM].getValue());
									if (decayValue > maxAdsrTime) {
										decayValue = maxAdsrTime;
									} else 	if (decayValue < minAdsrTime) {
										decayValue = minAdsrTime;
									}
									stageCoeff[c] = (1-sustainValue) / (args.sampleRate * decayValue);
								}
							break;

							case DECAY_STAGE:
								stageLevel[c] -= stageCoeff[c];
								if (stageLevel[c] <= sustainValue) {
									stageLevel[c] = sustainValue;
									stage[c] = SUSTAIN_STAGE;
								}
							break;

							case SUSTAIN_STAGE:
								stageLevel[c] = sustainValue;
							break;

							case RELEASE_STAGE:
								stageLevel[c] -= stageCoeff[c];
								if (stageLevel[c] < 0) {
									stageLevel[c] = 0;
									stage[c] = STOP_STAGE;
									play[c] = false;

									if (polyOuts) {
										eor[c] = true;
										eorTime[c] = oneMsSamples;
									} else {
										if (c == currentDisplay) {
											eor[0] = true;
											eorTime[0] = oneMsSamples;
										}
									}

									if (trigMode == TRIG_MODE && trigType == PLAY_PAUSE) {
										if (playPauseToStop[c]) {
											playPauseToStop[c] = false;
											if (reversePlaying[c] == FORWARD) {
												samplePos[c] = floor(cueStartPos)+1;
												currSampleWeight[c] = sampleCoeff;
												prevSamplePos[c] = floor(cueStartPos);
												prevSampleWeight[c] = 0;
											} else {
												samplePos[c] = floor(cueEndPos)-1;
												currSampleWeight[c] = sampleCoeff;
												prevSamplePos[c] = floor(cueEndPos);
												prevSampleWeight[c] = 0;
											}
										}
									}
								}
							break;
						}

						if (reversePlaying[c]) {
							currentOutput *= stageLevel[c] * masterLevel[c] * -1;
							if (channels == 2)
								currentOutputR *= stageLevel[c] * masterLevel[c] * -1;
						} else {
							currentOutput *= stageLevel[c] * masterLevel[c];
							if (channels == 2)
								currentOutputR *= stageLevel[c] * masterLevel[c];
						}

						switch (fadingType[c]) {
							case NO_FADE:
							break;

							case CROSS_FADE:
								if (fadingValue[c] > 0) {
									fadingValue[c] -= fadeCoeff;
									switch (reversePlaying[c]) {
										case FORWARD:
											currentOutput *= 1 - fadingValue[c];
											currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c]);
											if (channels == 2) {
												currentOutputR *= 1 - fadingValue[c];
												currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c]);
											}
											fadedPosition[c] += distancePos[c];
											if (fadedPosition[c] > totalSamples)
												fadingType[c] = NO_FADE;
										break;
										case REVERSE:
											currentOutput *= 1 - fadingValue[c];
											currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c] * -1);
											if (channels == 2) {
												currentOutputR *= 1 - fadingValue[c];
												currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c] * -1);
											}
											fadedPosition[c] -= distancePos[c];
											if (fadedPosition[c] < 0)
												fadingType[c] = NO_FADE;
										break;
									}
								} else
									fadingType[c] = NO_FADE;
							break;

							case FADE_OUT:
								if (fadingValue[c] > 0) {
									fadingValue[c] -= fadeCoeff;
									currentOutput *= fadingValue[c];
									if (channels == 2)
										currentOutputR *= 1 - fadingValue[c];
								} else
									fadingType[c] = NO_FADE;
							break;

							case PINGPONG_FADE:
								if (fadingValue[c] > 0) {
									fadingValue[c] -= fadeCoeff;
									switch (reversePlaying[c]) {
										case FORWARD:
											currentOutput *= 1 - fadingValue[c];
											currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c] * -1);
											if (channels == 2) {
												currentOutputR *= 1 - fadingValue[c];
												currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c] * -1);
											}
											fadedPosition[c] -= distancePos[c];
											if (fadedPosition[c] < 0)
												fadingType[c] = NO_FADE;
										break;
										case REVERSE:
											currentOutput *= 1 - fadingValue[c];
											currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c]);
											if (channels == 2) {
												currentOutputR *= 1 - fadingValue[c];
												currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c]);
											}
											fadedPosition[c] += distancePos[c];
											if (fadedPosition[c] > totalSamples)
												fadingType[c] = NO_FADE;
										break;
									}
								} else
									fadingType[c] = NO_FADE;
							break;
						}
					}

				} else {
					play[c] = false;
					fadingType[c] = NO_FADE;
				}

				switch (polyOuts) {
					case MONOPHONIC:										// monophonic CABLES
						sumOutput += currentOutput;
						if (channels == 2)
							sumOutputR += currentOutputR;
					break;

					case POLYPHONIC:										// polyphonic CABLES
						if (limiter) {					// *** hard clip functionality ***
							if (currentOutput > 5)
								currentOutput = 5;
							else if (currentOutput < -5)
								currentOutput = -5;
							if (channels == 2) {
								if (currentOutputR > 5)
									currentOutputR = 5;
								else if (currentOutputR < -5)
									currentOutputR = -5;
							}
						}

						// *** CLIPPING LIGHT ***
						if (currentOutput < -5 || currentOutput > 5) {
							clipping = true;
							clippingValue = 0;
						}
						if (channels == 2) {
							if (currentOutputR < -5 || currentOutputR > 5) {
								clipping = true;
								clippingValue = 0;
							}
						}
						if (clipping && clippingValue < 1)
							clippingValue += clippingCoeff;
						else {
							clipping = false;
							clippingValue = 1;
						}

						if (outputs[OUT_OUTPUT].isConnected())
							outputs[OUT_OUTPUT].setVoltage(currentOutput, c);

						if (outputs[OUT_OUTPUT+1].isConnected()) {
							if (channels == 2)
								outputs[OUT_OUTPUT+1].setVoltage(currentOutputR, c);
							else
								outputs[OUT_OUTPUT+1].setVoltage(currentOutput, c);
						}
					break;
				}

				if (polyOuts) {
					if (eoc[c]) {
						eocTime[c]--;
						if (eocTime[c] < 0) {
							eoc[c] = false;
							outputs[EOC_OUTPUT].setVoltage(0.f, c);
						} else
							outputs[EOC_OUTPUT].setVoltage(10.f, c);
					}

					if (eor[c]) {
						eorTime[c]--;
						if (eorTime[c] < 0) {
							eor[c] = false;
							outputs[EOR_OUTPUT].setVoltage(0.f, c);
						} else
							outputs[EOR_OUTPUT].setVoltage(10.f, c);
					}
				}

			} // END OF CHANNEL MANAGEMENT

			switch (polyOuts) {
				case MONOPHONIC:			// monophonic CABLES
					if (limiter) {					// *** hard clip functionality ***
						if (sumOutput > 5)
							sumOutput = 5;
						else if (sumOutput < -5)
							sumOutput = -5;
						if (channels == 2) {
							if (sumOutputR > 5)
								sumOutputR = 5;
							else if (sumOutputR < -5)
								sumOutputR = -5;
						}
					}

					// *** CLIPPING LIGHT ***
					if (sumOutput < -5 || sumOutput > 5) {
						clipping = true;
						clippingValue = 0;
					}
					if (channels == 2) {
						if (sumOutputR < -5 || sumOutputR > 5) {
							clipping = true;
							clippingValue = 0;
						}
					}
					if (clipping && clippingValue < 1)
						clippingValue += clippingCoeff;
					else {
						clipping = false;
						clippingValue = 1;
					}

					if (outputs[OUT_OUTPUT].isConnected()) {
						outputs[OUT_OUTPUT].setVoltage(sumOutput);
					}
					if (outputs[OUT_OUTPUT+1].isConnected()) {
						if (channels == 2)
							outputs[OUT_OUTPUT+1].setVoltage(sumOutputR);
						else
							outputs[OUT_OUTPUT+1].setVoltage(sumOutput);
					}

				break;

				case POLYPHONIC:			// polyphonic CABLES
					outputs[OUT_OUTPUT].setChannels(chan);
					outputs[OUT_OUTPUT+1].setChannels(chan);
				break;
			}

			if (polyOuts) {
				outputs[EOC_OUTPUT].setChannels(chan);
				outputs[EOR_OUTPUT].setChannels(chan);
			} else {
				if (eoc[0]) {
					eocTime[0]--;
					if (eocTime[0] < 0) {
						eoc[0] = false;
						outputs[EOC_OUTPUT].setVoltage(0.f, 0);
					} else
						outputs[EOC_OUTPUT].setVoltage(10.f, 0);
				}

				if (eor[0]) {
					eorTime[0]--;
					if (eorTime[0] < 0) {
						eor[0] = false;
						outputs[EOR_OUTPUT].setVoltage(0.f, 0);
					} else
						outputs[EOR_OUTPUT].setVoltage(10.f, 0);
				}
			}
		}

	}
};

struct SickoPlayerDisplay : TransparentWidget {
	SickoPlayer *module;
	int frame = 0;
	SickoPlayerDisplay() {
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

				if (module->clipping)
					module->lights[module->CLIPPING_LIGHT].setBrightness(1);
				else
					module->lights[module->CLIPPING_LIGHT].setBrightness(0);

				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
				nvgTextBox(args.vg, 7, 16,247, module->fileDisplay.c_str(), NULL);
				
				nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff)); 
				nvgTextBox(args.vg, 186, 16,97, module->timeDisplay.c_str(), NULL);

				nvgFillColor(args.vg, nvgRGBA(0x88, 0xaa, 0xff, 0xff)); 
				nvgTextBox(args.vg, 218, 16,97, module->channelsDisplay.c_str(), NULL);

				//nvgTextBox(args.vg, 9, 26,120, module->debugDisplay.c_str(), NULL);
				//nvgTextBox(args.vg, 109, 26,120, module->debugDisplay2.c_str(), NULL);

				// Zero line
				nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x40));
				{
					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, 7, 58.5);
					nvgLineTo(args.vg, 242, 58.5);
					nvgClosePath(args.vg);
				}
				nvgStroke(args.vg);
		
				if (module->fileLoaded) {
					int xLine;
					// Playback line
					nvgStrokeColor(args.vg, nvgRGBA(0xf5, 0xf5, 0xf5, 0xff));
					nvgStrokeWidth(args.vg, 0.8);
					{
						nvgBeginPath(args.vg);
						xLine = 7 + floor(module->samplePos[module->currentDisplay] * 235 / module->totalSampleC);
						nvgMoveTo(args.vg, xLine, 21);
						nvgLineTo(args.vg, xLine, 96);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Cue Start line
					nvgStrokeColor(args.vg, nvgRGBA(0x00, 0xf0, 0x00, 0xf0));
					nvgStrokeWidth(args.vg, 1);
					{
						nvgBeginPath(args.vg);
						xLine = 7 + floor(module->cueStartPos * 235 / module->totalSampleC);
						nvgMoveTo(args.vg, xLine , 21);
						nvgLineTo(args.vg, xLine , 96);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Cue End line
					nvgStrokeColor(args.vg, nvgRGBA(0xf0, 0x00, 0x00, 0xf0));
					nvgStrokeWidth(args.vg, 1);
					{
						nvgBeginPath(args.vg);
						xLine = 7 + floor(module->cueEndPos * 235 / module->totalSampleC);
						nvgMoveTo(args.vg, xLine , 21);
						nvgLineTo(args.vg, xLine , 96);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Loop Start line
					nvgStrokeColor(args.vg, nvgRGBA(0xf8, 0xec, 0x2e, 0xf0));
					nvgStrokeWidth(args.vg, 1);
					{
						nvgBeginPath(args.vg);
						xLine = 7 + floor(module->loopStartPos * 235 / module->totalSampleC);
						nvgMoveTo(args.vg, xLine , 21);
						nvgLineTo(args.vg, xLine , 96);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Loop End line
					nvgStrokeColor(args.vg, nvgRGBA(0xea, 0x79, 0x26, 0xf0));
					nvgStrokeWidth(args.vg, 1);
					{
						nvgBeginPath(args.vg);
						xLine = 7 + floor(module->loopEndPos * 235 / module->totalSampleC);
						nvgMoveTo(args.vg, xLine , 21);
						nvgLineTo(args.vg, xLine , 96);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Waveform
					nvgStrokeColor(args.vg, nvgRGBA(0x22, 0x44, 0xc9, 0xc0));
					nvgSave(args.vg);
					Rect b = Rect(Vec(7, 22), Vec(235, 73));
					nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
					nvgBeginPath(args.vg);
					for (unsigned int i = 0; i < module->displayBuff.size(); i++) {
						float x, y;
						x = (float)i / (module->displayBuff.size() - 1);
						y = module->displayBuff[i] / 10.0 + 0.5;
						Vec p;
						p.x = b.pos.x + b.size.x * x;
						p.y = b.pos.y + b.size.y * (1.0 - y);
						if (i == 0)
							nvgMoveTo(args.vg, p.x, p.y);
						else
							nvgLineTo(args.vg, p.x, p.y);
					}
					nvgLineCap(args.vg, NVG_ROUND);
					nvgMiterLimit(args.vg, 2.0);
					nvgStrokeWidth(args.vg, 1.5);
					nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
					nvgStroke(args.vg);			
					nvgResetScissor(args.vg);
					nvgRestore(args.vg);	
				}
			}
		}
		Widget::drawLayer(args, layer);
	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		SickoPlayer *module = dynamic_cast<SickoPlayer*>(this->module);
			assert(module);
		std::string currentDir = path;
		int tempIndex = 1;
		if (module->folderTreeData.size() < 2) {
			module->createFolder(currentDir.substr(0,currentDir.length()-1));
			module->folderTreeData.push_back(module->tempTreeData);
			module->folderTreeDisplay.push_back(module->tempTreeDisplay);
			module->folderTreeData[1][0] = currentDir;
			module->folderTreeDisplay[1][0] = currentDir;
		} else {
			bool exited = false;
			for (unsigned int i = 1 ; i < module->folderTreeData.size(); i++) {
				if (module->folderTreeData[i][0] == currentDir) {
					tempIndex = i;
					i = module->folderTreeData.size();
					exited = true;
				} 
			}
			if (!exited) {
				module->createFolder(currentDir);
				module->folderTreeData.push_back(module->tempTreeData);
				module->folderTreeDisplay.push_back(module->tempTreeDisplay);
				tempIndex = module->folderTreeData.size()-1;
			}	
		}
		if (module->folderTreeData[tempIndex].size() > 1) {
			for (unsigned int i = 1; i < module->folderTreeData[tempIndex].size(); i++) {
				if (module->folderTreeData[tempIndex][i].substr(module->folderTreeData[tempIndex][i].length()-1,module->folderTreeData[tempIndex][i].length()-1) == "/")  {
					module->tempDir = module->folderTreeData[tempIndex][i];
					menu->addChild(createSubmenuItem(module->folderTreeDisplay[tempIndex][i], "", [=](Menu* menu) {
						loadSubfolder(menu, module->folderTreeData[tempIndex][i]);
					}));
				} else {
					menu->addChild(createMenuItem(module->folderTreeDisplay[tempIndex][i], "", [=]() {module->loadFromPatch = false;module->loadSample(module->folderTreeData[tempIndex][i]);}));
				}
			}
		}
	}

	void createContextMenu() {
		SickoPlayer *module = dynamic_cast<SickoPlayer *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

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

			if (module->fileLoaded) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription));
				menu->addChild(createMenuLabel(" " + module->samplerateDisplay + " - " + std::to_string(module->channels) + "ch"));
				menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot();}));
			}

			menu->addChild(new MenuSeparator());
			menu->addChild(createBoolPtrMenuItem("Phase scan", "", &module->phaseScan));

			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Reset Cursors", "", [=]() {module->resetCursors();}));
			menu->addChild(createMenuItem("Copy Cue to Loop", "", [=]() {module->copyCueToLoop();}));
			menu->addChild(createMenuItem("Copy Loop to Cue", "", [=]() {module->copyLoopToCue();}));

			menu->addChild(createSubmenuItem("Presets", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("Wavetable", "", [=]() {module->setPreset(0);}));
				menu->addChild(createMenuItem("Triggered Sample with Envelope", "", [=]() {module->setPreset(1);}));
				menu->addChild(createMenuItem("Drum Player", "", [=]() {module->setPreset(2);}));
			}));
		}
	}
};

struct SickoPlayerWidget : ModuleWidget {
	SickoPlayerWidget(SickoPlayer *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SickoPlayer.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		{
			SickoPlayerDisplay *display = new SickoPlayerDisplay();
			display->box.pos = Vec(3, 24);
			display->box.size = Vec(247, 100);
			display->module = module;
			addChild(display);
		}

		const float xTrig1 = 11;
		const float xTrig2 = 29;
		const float yTrig1 = 50;
		const float yTrig2 = 64; 
		const float xStart1 = 50;
		const float xStart2 = 62;
		const float yStart1 = 53.5;
		const float yStart2 = 63.5;

		const float xEnv1 = 11.5f;
		const float xEnv1Add = 21.f;
		const float xEnv2 = 6.5f;
		const float xEnv2Add = 10.f;
		const float xEnv2Skip = 21.f;
		const float yEnv1 = 81.f;
		const float yEnv2 = 90.f;
		
		const float yTunVol = 108;
		const float yTunVol2 = 117.5;

		addParam(createParamCentered<VCVButton>(mm2px(Vec(12, 4)), module, SickoPlayer::PREVSAMPLE_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(74.4, 4)), module, SickoPlayer::NEXTSAMPLE_PARAM));

		addParam(createParamCentered<CKSS>(mm2px(Vec(xTrig1, yTrig1)), module, SickoPlayer::TRIGGATEMODE_SWITCH));
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(xTrig2, yTrig1+1)), module, SickoPlayer::TRIGMODE_SWITCH));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xTrig1-5, yTrig2)), module, SickoPlayer::TRIG_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xTrig1+4.6, yTrig2)), module, SickoPlayer::TRIGBUT_PARAM, SickoPlayer::TRIGBUT_LIGHT));
		
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xTrig2-4.5, yTrig2)), module, SickoPlayer::STOP_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xTrig2+5.1, yTrig2)), module, SickoPlayer::STOPBUT_PARAM, SickoPlayer::STOPBUT_LIGHT));

		//----------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStart1, yStart1)), module, SickoPlayer::CUESTART_PARAM));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStart2, yStart1)), module, SickoPlayer::CUEEND_PARAM));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStart1, yStart2)), module, SickoPlayer::LOOPSTART_PARAM));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStart2, yStart2)), module, SickoPlayer::LOOPEND_PARAM));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xStart2+9.8, yStart1-1.5)), module, SickoPlayer::REV_PARAM, SickoPlayer::REV_LIGHT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xStart2 + 18, yStart1-1)), module, SickoPlayer::XFADE_PARAM));;

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xStart2+10, yStart2-2)), module, SickoPlayer::LOOP_PARAM, SickoPlayer::LOOP_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xStart2+18, yStart2+0.5)), module, SickoPlayer::PINGPONG_PARAM, SickoPlayer::PINGPONG_LIGHT));
		//----------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xEnv1, yEnv1)), module, SickoPlayer::ATTACK_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xEnv2, yEnv2)), module, SickoPlayer::ATTACK_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv2+xEnv2Add, yEnv2)), module, SickoPlayer::ATTACKATNV_PARAM));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xEnv1+xEnv1Add, yEnv1)), module, SickoPlayer::DECAY_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xEnv2+xEnv2Skip, yEnv2)), module, SickoPlayer::DECAY_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv2+xEnv2Add+xEnv2Skip, yEnv2)), module, SickoPlayer::DECAYATNV_PARAM));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xEnv1+xEnv1Add*2, yEnv1)), module, SickoPlayer::SUSTAIN_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xEnv2+xEnv2Skip*2, yEnv2)), module, SickoPlayer::SUSTAIN_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv2+xEnv2Add+xEnv2Skip*2, yEnv2)), module, SickoPlayer::SUSTAINATNV_PARAM));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xEnv1+xEnv1Add*3, yEnv1)), module, SickoPlayer::RELEASE_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xEnv2+xEnv2Skip*3, yEnv2)), module, SickoPlayer::RELEASE_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv2+xEnv2Add+xEnv2Skip*3, yEnv2)), module, SickoPlayer::RELEASEATNV_PARAM));
		//----------------------------------------------------------------------------------------------------------------------------

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(9, yTunVol+2.5)), module, SickoPlayer::VO_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(23, yTunVol)), module, SickoPlayer::TUNE_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(18, yTunVol2)), module, SickoPlayer::TUNE_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(28, yTunVol2)), module, SickoPlayer::TUNEATNV_PARAM));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(45.5, yTunVol)), module, SickoPlayer::VOL_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(40.5, yTunVol2)), module, SickoPlayer::VOL_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(50.5, yTunVol2)), module, SickoPlayer::VOLATNV_PARAM));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(53, yTunVol-4.5)), module, SickoPlayer::CLIPPING_LIGHT));

		addParam(createParamCentered<CKSS>(mm2px(Vec(59.3, 113)), module, SickoPlayer::LIMIT_SWITCH));
		//----------------------------------------------------------------------------------------------------------------------------

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(70.2, 105.3)), module, SickoPlayer::OUT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(80.2, 105.3)), module, SickoPlayer::OUT_OUTPUT+1));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(70.2, 117.5)), module, SickoPlayer::EOC_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(80.2, 117.5)), module, SickoPlayer::EOR_OUTPUT));
	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		SickoPlayer *module = dynamic_cast<SickoPlayer*>(this->module);
			assert(module);
		std::string currentDir = path;
		int tempIndex = 1;
		if (module->folderTreeData.size() < 2) {
			module->createFolder(currentDir.substr(0,currentDir.length()-1));
			module->folderTreeData.push_back(module->tempTreeData);
			module->folderTreeDisplay.push_back(module->tempTreeDisplay);
			module->folderTreeData[1][0] = currentDir;
			module->folderTreeDisplay[1][0] = currentDir;
		} else {
			bool exited = false;
			for (unsigned int i = 1 ; i < module->folderTreeData.size(); i++) {
				if (module->folderTreeData[i][0] == currentDir) {
					tempIndex = i;
					i = module->folderTreeData.size();
					exited = true;
				} 
			}
			if (!exited) {
				module->createFolder(currentDir);
				module->folderTreeData.push_back(module->tempTreeData);
				module->folderTreeDisplay.push_back(module->tempTreeDisplay);
				tempIndex = module->folderTreeData.size()-1;
			}	
		}
		if (module->folderTreeData[tempIndex].size() > 1) {
			for (unsigned int i = 1; i < module->folderTreeData[tempIndex].size(); i++) {
				if (module->folderTreeData[tempIndex][i].substr(module->folderTreeData[tempIndex][i].length()-1,module->folderTreeData[tempIndex][i].length()-1) == "/")  {
					module->tempDir = module->folderTreeData[tempIndex][i];
					menu->addChild(createSubmenuItem(module->folderTreeDisplay[tempIndex][i], "", [=](Menu* menu) {
						loadSubfolder(menu, module->folderTreeData[tempIndex][i]);
					}));
				} else {
					menu->addChild(createMenuItem(module->folderTreeDisplay[tempIndex][i], "", [=]() {module->loadFromPatch = false;module->loadSample(module->folderTreeData[tempIndex][i]);}));
				}
			}
		}
	}

	void appendContextMenu(Menu *menu) override {
	   	SickoPlayer *module = dynamic_cast<SickoPlayer*>(this->module);
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
				menu->addChild(createMenuLabel(" " + module->samplerateDisplay + " - " + std::to_string(module->channels) + "ch"));
			} else {
				menu->addChild(createMenuLabel("Sample ERROR:"));
				menu->addChild(createMenuLabel(module->fileDescription));
			}
			menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot();}));
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Select Samples Root", "", [=]() {module->selectRootFolder();}));
		if (module->userFolder != "") {
			if (module->rootFound) {
				menu->addChild(createMenuLabel(module->userFolder));
				//menu->addChild(createMenuItem("", "Refresh", [=]() {module->refreshRootFolder();}));
			} else {
				menu->addChild(createMenuLabel("(!)"+module->userFolder));
			}
		}

		menu->addChild(new MenuSeparator());
		struct ModeItem : MenuItem {
			SickoPlayer* module;
			int interpolationMode;
			void onAction(const event::Action& e) override {
				module->interpolationMode = interpolationMode;
			}
		};
		std::string modeNames[4] = {"None", "Linear 1", "Linear 2", "Hermite"};
		menu->addChild(createSubmenuItem("Interpolation", (modeNames[module->interpolationMode]), [=](Menu * menu) {
			for (int i = 0; i < 4; i++) {
				ModeItem* modeItem = createMenuItem<ModeItem>(modeNames[i]);
				modeItem->rightText = CHECKMARK(module->interpolationMode == i);
				modeItem->module = module;
				modeItem->interpolationMode = i;
				menu->addChild(modeItem);
			}
		}));
		
		menu->addChild(createBoolPtrMenuItem("Anti-aliasing filter", "", &module->antiAlias));
		menu->addChild(createBoolPtrMenuItem("Phase scan", "", &module->phaseScan));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolMenuItem("Polyphonic OUTs", "", [=]() {
				return module->isPolyOuts();
			}, [=](bool poly) {
				module->setPolyOuts(poly);
		}));
		menu->addChild(createBoolPtrMenuItem("Polyphonic Master IN", "", &module->polyMaster));

		menu->addChild(createSubmenuItem("EOC pulse from", "", [=](Menu* menu) {
			menu->addChild(createBoolPtrMenuItem("TRG/GATE (stop)", "", &module->eocFromTrg));
			menu->addChild(createBoolPtrMenuItem("STOP trig", "", &module->eocFromStop));
			menu->addChild(createBoolPtrMenuItem("CUE END", "", &module->eocFromCueEnd));
			menu->addChild(createBoolPtrMenuItem("CUE START", "", &module->eocFromCueStart));
			menu->addChild(createBoolPtrMenuItem("LOOP END", "", &module->eocFromLoopEnd));
			menu->addChild(createBoolPtrMenuItem("LOOP START", "", &module->eocFromLoopStart));
			menu->addChild(createBoolPtrMenuItem("PING", "", &module->eocFromPing));
			menu->addChild(createBoolPtrMenuItem("PONG", "", &module->eocFromPong));
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Reset cursors on Load", "", &module->resetCursorsOnLoad));
		menu->addChild(createBoolPtrMenuItem("Disable NAV Buttons", "", &module->disableNav));
		menu->addChild(createBoolPtrMenuItem("Store Sample in Patch", "", &module->sampleInPatch));
		menu->addChild(createBoolPtrMenuItem("Unlimited Sample Size (risky)", "", &module->unlimitedRecording));
		
		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Presets", "", [=](Menu * menu) {
			menu->addChild(createMenuItem("Wavetable", "", [=]() {module->setPreset(0);}));
			menu->addChild(createMenuItem("Triggered Sample with Envelope", "", [=]() {module->setPreset(1);}));
			menu->addChild(createMenuItem("Drum Player", "", [=]() {module->setPreset(2);}));
		}));
	}
};

Model *modelSickoPlayer = createModel<SickoPlayer, SickoPlayerWidget>("SickoPlayer");
