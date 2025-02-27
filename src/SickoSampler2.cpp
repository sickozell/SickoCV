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
#define MONOPHONIC 0
#define POLYPHONIC 1
#define SAVE_FULL 0
#define SAVE_CUE 1
#define SAVE_LOOP 2
#define NO_FADE 0
#define CROSS_FADE 1
#define FADE_OUT 2
#define PINGPONG_FADE 3
#define FORWARD 0
#define REVERSE 1

#define recOutChan 0
#define sampleCoeff 2

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

struct SickoSampler2 : Module {
	enum ParamIds {
		VOL_PARAM,
		CUESTART_PARAM,
		CUEEND_PARAM,
		LOOPSTART_PARAM,
		LOOPEND_PARAM,
		XFADE_PARAM,
		ATTACK_PARAM,
		DECAY_PARAM,
		SUSTAIN_PARAM,
		RELEASE_PARAM,
		RELEASEATNV_PARAM,
		TRIGGATEMODE_SWITCH,
		TRIGMODE_SWITCH,
		LOOP_PARAM,
		REV_PARAM,
		PINGPONG_PARAM,
		REC_PARAM,
		GAIN_PARAM,
		MONITOR_SWITCH,
		PREVSAMPLE_PARAM,
		NEXTSAMPLE_PARAM,
		TRIGBUT_PARAM,
		PHASESCAN_SWITCH,
		STRETCH_PARAM,
		STR_SIZE_PARAM,
		NUM_PARAMS 
	};
	enum InputIds {
		TRIG_INPUT,
		VOL_INPUT,
		VO_INPUT,
		ENUMS(IN_INPUT,2),
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
		REC_LIGHT,
		TRIGBUT_LIGHT,
		PHASESCAN_LIGHT,
		MONITOR_LIGHT,
		TRIGMODE_LIGHT,
		NUM_LIGHTS
	};
  
	unsigned int fileChannels;
	unsigned int channels;
	unsigned int sampleRate;
	drwav_uint64 totalSampleC = 0;
	drwav_uint64 totalSamples = 0;

	const unsigned int minSamplesToLoad = 124;

	vector<float> playBuffer[2][2];
	
	//metamodule change
	//vector<float> tempBuffer[2];

	vector<double> displayBuff;
	int currentDisplay = 0;
	float voctDisplay = 100.f;

	bool fileLoaded = false;

	bool play[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	double samplePos[16] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	double distancePos[16] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

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

	float voct[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
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

	double currSampleWeight[16] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

	std::string storedPath = "";
	std::string fileDescription = "--none--";
	std::string fileDisplay = "";
	std::string channelsDisplay = "";
	std::string timeDisplay = "";
	std::string recTimeDisplay = "";
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
	int recSeconds;
	int recMinutes;

	drwav_uint64 recSamples = 0;

	float trigValue[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	int trigButValue = 0;
	float prevTrigValue[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

	bool loop = false;

	bool trimOnSave = true;
	int antiAlias = 1;
	int polyOuts = POLYPHONIC;
	int polyMaster = POLYPHONIC;
	bool phaseScan = true;
	bool prevPhaseScan = false;
	bool firstLoad = true;
	bool resetCursorsOnLoad = true;
	bool disableNav = false;
	bool saveOversampled = false;
	bool autoMonOff = true;

	bool sampleInPatch = true;

	bool loadFromPatch = false;
	bool restoreLoadFromPatch = false;
	
	float fadeCoeff = 0.f;

	int fadingType[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float fadingValue[16] = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};
	float fadeDecrement = 0.f;
	double fadedPosition[16] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	double fadeSamples = 0.0;
	float xFade = 0.f;
	float prevXfade = -1.f;

	float currentOutput = 0.f;
	float currentOutputR = 0.f;
	float sumOutput = 0.f;
	float sumOutputR = 0.f;

	/*
	std::string debugDisplay = "X";
	std::string debugDisplay2 = "X";
	std::string debugDisplay3 = "X";
	std::string debugDisplay4 = "X";
	int debugInt = 0;
	bool debugBool = false;
	*/

	double a0, a1, a2, b1, b2, z1, z2, z1r, z2r;

	int trigMode;
	int trigType;
	int stage[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float stageLevel[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	
	bool releaseNew[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	bool eoc[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool eor[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	float eocTime[16];
	float eorTime[16];
	float oneMsSamples  = (APP->engine->getSampleRate()) / 1000;	// number of samples in 1ms

	bool eocFromTrg = false;
	//bool eocFromStop = false;
	bool eocFromCueEnd = true;
	bool eocFromCueStart = true;
	bool eocFromLoopEnd = true;
	bool eocFromLoopStart = true;
	bool eocFromPing = true;
	bool eocFromPong = true;

	int chan;

	float attackValue = 0.f;
	float decayValue = 0.f;
	float sustainValue = 0.f;
	float releaseValue = 0.f;
	float stageCoeff[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float masterLevel[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

	bool reverseStart = false;
	int reversePlaying[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int pingpong;

	int recButton = 0;
	int prevRecButton = 0;
	float recTrig = 0.f;
	float prevRecTrig = 0.f;
	float recStopTrig = 0.f;
	float prevRecStopTrig = 0.f;
	int recordingState = 0;

	drwav_uint64 currRecPosition = 0;

	int recChannels = 1;
	float currRecValue[2] = {0.f, 0.f};
	float prevRecValue[2] = {0.f, 0.f};
	
	float recFadeValue = 0.f;
	bool recFadeIn = false;
	bool recFadeOut = false;

	int fileSampleRate = 0;
	bool resampled = false;
	bool toSave = false;
	std::string infoToSave= "";

	int monitorSwitch = 0;
	int prevMonitorSwitch = 0;
	bool monitorFade = false;
	float monitorFadeValue = 0.f;
	float monitorFadeCoeff = 10 / (APP->engine->getSampleRate());

	int saveMode = 0;

	bool nextSample = false;
	bool prevNextSample = false;
	bool prevSample = false;
	bool prevPrevSample = false;

	bool unlimitedRecording = false;

#if defined(METAMODULE)
	const drwav_uint64 recordingLimit = 48000 * 2 * 60; // 60 sec limit on MM = 5.5MB
#else
	const drwav_uint64 recordingLimit = 52428800 * 2;
	// const drwav_uint64 recordingLimit = 480000 * 2; // 10 sec for test purposes
#endif
	
	drwav_uint64 currentRecordingLimit = recordingLimit;

	static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	const float maxAdsrTime = 10.f;
	const float minAdsrTime = 0.001f;
	const float noEnvTime = 0.00101f;

	int grainCount[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double grainSampleCount[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double stretchMaxPos = 0;
	//drwav_uint64 grainPos[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double grainPos[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float grainFadeValue[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float grainFadeCoeff[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	bool grainFade[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	float stretchKnob = 0.f;
	float cycleKnob = 0.f;

	SickoSampler2() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configSwitch(PREVSAMPLE_PARAM, 0.f, 1.f, 0.f, "Previous Sample");
		configSwitch(NEXTSAMPLE_PARAM, 0.f, 1.f, 0.f, "Next Sample");

		configSwitch(TRIGGATEMODE_SWITCH, 0.f, 1.f, 1.f, "Mode", {"Gate", "Trig"});
		configSwitch(TRIGMODE_SWITCH, 0.f, 1.f, 0.f, "Trig Type", {"Start/Stop", "Restart"});

		configInput(TRIG_INPUT,"Play");
		configSwitch(TRIGBUT_PARAM, 0.f, 1.f, 0.f, "Play", {"OFF", "ON"});

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
		configParam(DECAY_PARAM, 0.f, 1.f, 0.f, "Decay", "ms", maxStageTime / minStageTime, minStageTime);
		configParam(SUSTAIN_PARAM, 0.f, 1.0f, 1.0f, "Sustain","%", 0, 100);
		configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "Release", "ms", maxStageTime / minStageTime, minStageTime);

		//******************************************************************************
		
		configInput(VO_INPUT,"V/Oct");

		configParam(STRETCH_PARAM, 0.01f, 9.99f, 1.f, "Time Stretch", "%", 0, 100);
		configParam(STR_SIZE_PARAM, 1.f, 99.f, 48.f, "Cycle Size", "ms");

		configParam(VOL_PARAM, 0.f, 1.0f, 1.0f, "Master Volume", "%", 0, 100);
		configInput(VOL_INPUT,"Master Volume CV");

		//******************************************************************************
		
		configOutput(OUT_OUTPUT,"Left");
		configOutput(OUT_OUTPUT+1,"Right");
		configOutput(EOC_OUTPUT,"End of Cycle");
		configOutput(EOR_OUTPUT,"End of Release");

		//******************************************************************************
		configSwitch(PHASESCAN_SWITCH, 0.f, 1.f, 1.f, "Phase Scan", {"Off", "On"});
		configInput(IN_INPUT,"Left");
		configInput(IN_INPUT+1,"Right");
		configSwitch(REC_PARAM, 0.f, 1.f, 0.f, "REC", {"OFF", "ON"});
		configParam(GAIN_PARAM, 0.f, 2.0f, 1.0f, "REC Gain", "%", 0, 100);
		
		configSwitch(MONITOR_SWITCH, 0.f, 1.f, 0.f, "Monitor", {"Off", "On"});

		playBuffer[0][0].resize(0);
		playBuffer[0][1].resize(0);
		playBuffer[1][0].resize(0);
		playBuffer[1][1].resize(0);
		// metamodule change
		//tempBuffer[0].resize(0);
		//tempBuffer[1].resize(0);

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
		trimOnSave = true;
		antiAlias = 1;
		polyOuts = POLYPHONIC;
		polyMaster = POLYPHONIC;
		prevPhaseScan = false;
		autoMonOff = true;
		eocFromTrg = false;
		eocFromCueEnd = true;
		eocFromCueStart = true;
		eocFromLoopEnd = true;
		eocFromLoopStart = true;
		eocFromPing = true;
		eocFromPong = true;
		firstLoad = true;
		resetCursorsOnLoad = true;
		saveOversampled = false;

		disableNav = false;
		sampleInPatch = true;
		unlimitedRecording = false;
		
		prevKnobCueStartPos = -1.f;
		prevKnobCueEndPos = 2.f;
		prevKnobLoopStartPos = -1.f;
		prevKnobLoopEndPos = 2.f;
		reverseStart = false;
		totalSampleC = 0;
		totalSamples = 0;
		recordingState = 0;
		recButton = 0;
		prevMonitorSwitch = 0;
		prevXfade = -1.f;
		system::removeRecursively(getPatchStorageDirectory().c_str());
		Module::onReset(e);
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
				saveMode = SAVE_FULL;
				loadFromPatch = true;
				saveSample(patchFile);
			}
		}

		Module::onSave(e);
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "TrimOnSave", json_boolean(trimOnSave));
		json_object_set_new(rootJ, "SaveOversampled", json_boolean(saveOversampled));
		json_object_set_new(rootJ, "AntiAlias", json_integer(antiAlias));
		json_object_set_new(rootJ, "PolyOuts", json_integer(polyOuts));
		json_object_set_new(rootJ, "PolyMaster", json_integer(polyMaster));
		json_object_set_new(rootJ, "AutoMonOff", json_integer(autoMonOff));
		json_object_set_new(rootJ, "EocFromTrg", json_boolean(eocFromTrg));
		json_object_set_new(rootJ, "EocFromCueEnd", json_boolean(eocFromCueEnd));
		json_object_set_new(rootJ, "EocFromCueStart", json_boolean(eocFromCueStart));
		json_object_set_new(rootJ, "EocFromLoopEnd", json_boolean(eocFromLoopEnd));
		json_object_set_new(rootJ, "EocFromLoopStart", json_boolean(eocFromLoopStart));
		json_object_set_new(rootJ, "EocFromPing", json_boolean(eocFromPing));
		json_object_set_new(rootJ, "EocFromPong", json_boolean(eocFromPong));
		json_object_set_new(rootJ, "ResetCursorsOnLoad", json_boolean(resetCursorsOnLoad));
		json_object_set_new(rootJ, "DisableNav", json_boolean(disableNav));
		json_object_set_new(rootJ, "sampleInPatch", json_boolean(sampleInPatch));
		json_object_set_new(rootJ, "unlimitedRecording", json_boolean(unlimitedRecording));
		json_object_set_new(rootJ, "Slot", json_string(storedPath.c_str()));
		json_object_set_new(rootJ, "UserFolder", json_string(userFolder.c_str()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t* trimOnSaveJ = json_object_get(rootJ, "TrimOnSave");
		if (trimOnSaveJ)
			trimOnSave = json_boolean_value(trimOnSaveJ);
		json_t* saveOversampledJ = json_object_get(rootJ, "SaveOversampled");
		if (saveOversampledJ)
			saveOversampled = json_boolean_value(saveOversampledJ);
		json_t* antiAliasJ = json_object_get(rootJ, "AntiAlias");
		if (antiAliasJ)
			antiAlias = json_integer_value(antiAliasJ);
		json_t* polyOutsJ = json_object_get(rootJ, "PolyOuts");
		if (polyOutsJ)
			polyOuts = json_integer_value(polyOutsJ);
		json_t* polyMasterJ = json_object_get(rootJ, "PolyMaster");
		if (polyMasterJ)
			polyMaster = json_integer_value(polyMasterJ);
		json_t* autoMonOffJ = json_object_get(rootJ, "AutoMonOff");
		if (autoMonOffJ)
			autoMonOff = json_integer_value(autoMonOffJ);
		json_t* eocFromTrgJ = json_object_get(rootJ, "EocFromTrg");
		if (eocFromTrgJ)
			eocFromTrg = json_boolean_value(eocFromTrgJ);
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
		json_t* resetCursorsOnLoadJ = json_object_get(rootJ, "ResetCursorsOnLoad");
		if (resetCursorsOnLoadJ)
			resetCursorsOnLoad = json_boolean_value(resetCursorsOnLoadJ);
		json_t* disableNavJ = json_object_get(rootJ, "DisableNav");
		if (disableNavJ)
			disableNav = json_boolean_value(disableNavJ);
		json_t* sampleInPatchJ = json_object_get(rootJ, "sampleInPatch");
		if (sampleInPatchJ)
			sampleInPatch = json_boolean_value(sampleInPatchJ);
		json_t* unlimitedRecordingJ = json_object_get(rootJ, "unlimitedRecording");
		if (unlimitedRecordingJ)
			unlimitedRecording = json_boolean_value(unlimitedRecordingJ);
		json_t *slotJ = json_object_get(rootJ, "Slot");
		if (slotJ) {
			storedPath = json_string_value(slotJ);
			if (storedPath != "")
				loadSample(storedPath);
			else
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

	
	double hermiteInterpol(double x0, double x1, double x2, double x3, double t)	{
		double c0 = x1;
		double c1 = .5F * (x2 - x0);
		double c2 = x0 - (2.5F * x1) + (2 * x2) - (.5F * x3);
		double c3 = (.5F * (x3 - x0)) + (1.5F * (x1 - x2));
		return (((((c3 * t) + c2) * t) + c1) * t) + c0;
	}

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

/*

																██████╗░███████╗░██████╗░█████╗░███╗░░░███╗██████╗░██╗░░░░░██╗███╗░░██╗░██████╗░
																██╔══██╗██╔════╝██╔════╝██╔══██╗████╗░████║██╔══██╗██║░░░░░██║████╗░██║██╔════╝░
																██████╔╝█████╗░░╚█████╗░███████║██╔████╔██║██████╔╝██║░░░░░██║██╔██╗██║██║░░██╗░
																██╔══██╗██╔══╝░░░╚═══██╗██╔══██║██║╚██╔╝██║██╔═══╝░██║░░░░░██║██║╚████║██║░░╚██╗
																██║░░██║███████╗██████╔╝██║░░██║██║░╚═╝░██║██║░░░░░███████╗██║██║░╚███║╚██████╔╝
																╚═╝░░╚═╝╚══════╝╚═════╝░╚═╝░░╚═╝╚═╝░░░░░╚═╝╚═╝░░░░░╚══════╝╚═╝╚═╝░░╚══╝░╚═════╝░
*/
// https://fsymbols.com/generators/carty/

	void onSampleRateChange() override {
		monitorFadeCoeff = 10 / (APP->engine->getSampleRate()); // 100ms monitor fade
		oneMsSamples = (APP->engine->getSampleRate())/1000;			// number of samples for 1 ms used for output triggers
		prevXfade = -1.f;

		if (fileLoaded && APP->engine->getSampleRate() != sampleRate/2) {
			double resampleCoeff;

			fileLoaded = false;

			z1 = 0; z2 = 0; z1r = 0; z2r = 0;

			// metamodule change
			//tempBuffer[0].clear();
			//tempBuffer[1].clear();
			vector<float> tempBuffer[2];

			for (unsigned int i=0; i < totalSampleC; i++) {
				tempBuffer[LEFT].push_back(playBuffer[LEFT][0][i]);
				if (channels == 2) {
					tempBuffer[RIGHT].push_back(playBuffer[RIGHT][0][i]);
				}
			}
			
			playBuffer[LEFT][0].clear();
			playBuffer[LEFT][1].clear();
			playBuffer[RIGHT][0].clear();
			playBuffer[RIGHT][1].clear();

			// metamodule change
			vector<float>().swap(playBuffer[LEFT][0]);
			vector<float>().swap(playBuffer[RIGHT][0]);
			vector<float>().swap(playBuffer[LEFT][1]);
			vector<float>().swap(playBuffer[RIGHT][1]);

			drwav_uint64 tempSampleC = totalSampleC;
			drwav_uint64 tempSamples = totalSamples;
	
			resampleCoeff = sampleRate / (APP->engine->getSampleRate()) / 2;
			
			double currResamplePos = 0;
			double floorCurrResamplePos = 0;

			playBuffer[LEFT][0].push_back(tempBuffer[LEFT][0]);
			if (channels == 2)
				playBuffer[RIGHT][0].push_back(tempBuffer[RIGHT][0]);

			currResamplePos += resampleCoeff;
			floorCurrResamplePos = floor(currResamplePos);
			int temp;

			while ( floorCurrResamplePos < 1 ) {
				temp = tempBuffer[LEFT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
							tempBuffer[LEFT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
				playBuffer[LEFT][0].push_back(temp);
				if (channels == 2) {
					temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
							tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					playBuffer[RIGHT][0].push_back(temp);
				}
				currResamplePos += resampleCoeff;
				floorCurrResamplePos = floor(currResamplePos);
			} 

			while ( currResamplePos < tempSamples - 1) {
				playBuffer[LEFT][0].push_back(	hermiteInterpol(tempBuffer[LEFT][floorCurrResamplePos - 1],
																tempBuffer[LEFT][floorCurrResamplePos],
																tempBuffer[LEFT][floorCurrResamplePos + 1],
																tempBuffer[LEFT][floorCurrResamplePos + 2],
																currResamplePos - floorCurrResamplePos)
											);
				if (channels == 2)
					playBuffer[RIGHT][0].push_back(	hermiteInterpol(tempBuffer[RIGHT][floorCurrResamplePos - 1],
																	tempBuffer[RIGHT][floorCurrResamplePos],
																	tempBuffer[RIGHT][floorCurrResamplePos + 1],
																	tempBuffer[RIGHT][floorCurrResamplePos + 2],
																	currResamplePos - floorCurrResamplePos)
												);
				currResamplePos += resampleCoeff;
				floorCurrResamplePos = floor(currResamplePos);
			}

			while ( floorCurrResamplePos < tempSampleC ) {
				temp = tempBuffer[LEFT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
							tempBuffer[LEFT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
				playBuffer[LEFT][0].push_back(temp);
				if (channels == 2) {
					temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
							tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					playBuffer[RIGHT][0].push_back(temp);
				}
				currResamplePos += resampleCoeff;
				floorCurrResamplePos = floor(currResamplePos);
			}

			// metamodule change
			//tempBuffer[LEFT].clear();
			//tempBuffer[RIGHT].clear();

			// ***************************************************************************
			totalSampleC = playBuffer[LEFT][0].size();
			totalSamples = totalSampleC-1;					// *****   DA VERIFICARE se è -2 ********************************************
			sampleRate = APP->engine->getSampleRate() * 2;
			// *************************************************************

			calcBiquadLpf(20000.0, sampleRate, 1);
			for (unsigned int i = 0; i < totalSampleC; i++) {
				playBuffer[LEFT][1].push_back(biquadLpf(playBuffer[LEFT][0][i]));
				if (channels == 2)
					playBuffer[RIGHT][1].push_back(biquadLpf2(playBuffer[RIGHT][0][i]));
			}

			prevKnobCueStartPos = -1.f;
			prevKnobCueEndPos = 2.f;
			prevKnobLoopStartPos = -1.f;
			prevKnobLoopEndPos = 2.f;

			vector<double>().swap(displayBuff);
			for (int i = 0; i < floor(totalSampleC); i = i + floor(totalSampleC/140))
				displayBuff.push_back(playBuffer[0][0][i]);

			resampled = true;
			recSamples = 0;
			fileLoaded = true;
		}
	}	
/*

																							░██████╗░█████╗░██╗░░░██╗███████╗
																							██╔════╝██╔══██╗██║░░░██║██╔════╝
																							╚█████╗░███████║╚██╗░██╔╝█████╗░░
																							░╚═══██╗██╔══██║░╚████╔╝░██╔══╝░░
																							██████╔╝██║░░██║░░╚██╔╝░░███████╗
																							╚═════╝░╚═╝░░╚═╝░░░╚═╝░░░╚══════╝
*/
	void menuSaveSample(int mode) {
		fileChannels = channels;
		fileLoaded = false;
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			saveMode = mode;
			fileDescription = basename(path);
			saveSample(path);
			//storedPath = std::string(path);
			std::string newPath = std::string(path);
			if (newPath.substr(newPath.size() - 4) != ".wav" && newPath.substr(newPath.size() - 4) != ".WAV")
				newPath += ".wav";
			storedPath = newPath;
			
		}
		channels = fileChannels;
		fileLoaded = true;
		free(path);
#if defined(METAMODULE)
		});
#endif
	};

	void saveSample(std::string path) {
		drwav_uint64 samples;
		drwav_uint64 saveStartSamplePos = 0;
		drwav_uint64 saveEndSamplePos = 0;

		int overSampleCoeff;
		if (saveOversampled)
			overSampleCoeff = 1;
		else
			overSampleCoeff = 2;

		switch (saveMode) {
			case SAVE_FULL:
				saveStartSamplePos = 0;
				saveEndSamplePos = floor(totalSampleC/overSampleCoeff);
			break;
			case SAVE_CUE:
				saveStartSamplePos = floor(cueStartPos/overSampleCoeff);
				saveEndSamplePos = floor(cueEndPos/overSampleCoeff);
			break;
			case SAVE_LOOP:
				saveStartSamplePos = floor(loopStartPos/overSampleCoeff);
				saveEndSamplePos = floor(loopEndPos/overSampleCoeff);
			break;
		}

		if (!saveOversampled) {
			if (saveStartSamplePos % 2 != 0 && saveStartSamplePos > 0)
				saveStartSamplePos--;

			if (saveEndSamplePos % 2 != 0 && saveEndSamplePos < totalSamples)
				saveEndSamplePos++;
		}

		samples = floor(saveEndSamplePos - saveStartSamplePos);

		std::vector<float> data;

		switch (fileChannels) {
			case 1:
				for (unsigned int i = saveStartSamplePos; i <= saveEndSamplePos; i++)
					data.push_back(playBuffer[LEFT][0][i*overSampleCoeff] / 5);
			break;

			case 2:
				for (unsigned int i = saveStartSamplePos; i <= saveEndSamplePos; i++) {
					data.push_back(playBuffer[LEFT][0][i*overSampleCoeff] / 5);
					data.push_back(playBuffer[RIGHT][0][i*overSampleCoeff] / 5);
				}
			break;
		}

		drwav_data_format format;
		format.container = drwav_container_riff;
		format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
	
		if (fileChannels == 1) 
			format.channels = 1;
		else {
			format.channels = 2;
			samples *= 2;
		}

		if (saveOversampled)
			format.sampleRate = sampleRate;
		else
			format.sampleRate = sampleRate / 2;

		format.bitsPerSample = 32;

		if (path.substr(path.size() - 4) != ".wav" && path.substr(path.size() - 4) != ".WAV")
			path += ".wav";

		drwav *pWav = drwav_open_file_write(path.c_str(), &format);
		drwav_write(pWav, samples, data.data());
		drwav_close(pWav);
		toSave = false;
		infoToSave = "";

		char* pathDup = strdup(path.c_str());
		//fileDescription = basename(pathDup);
		if (!loadFromPatch) {
			toSave = false;
			infoToSave = "";
			fileDescription = basename(pathDup);
		} else {
			if (fileDescription != "_unknown_.wav")
				fileDescription = basename(pathDup);
		}

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

		fileDisplay = fileDisplay.substr(0, 12);

		free(pathDup);
		data.clear();

		//storedPath = path;
		if (!loadFromPatch) {
			storedPath = path;
		
			if (trimOnSave && saveMode == SAVE_LOOP) {
				// set position knob to 0 and 1
				knobCueStartPos = 0;
				knobCueEndPos = 1;
				knobLoopStartPos = 0;
				knobLoopEndPos = 1;
				params[CUESTART_PARAM].setValue(0);
				params[CUEEND_PARAM].setValue(1);
				params[LOOPSTART_PARAM].setValue(0);
				params[LOOPEND_PARAM].setValue(1);

				loadSample(path);
			} else if (trimOnSave && saveMode == SAVE_CUE) {
				float tempKnobLoopStartPos = (knobLoopStartPos - knobCueStartPos ) / (knobCueEndPos - knobCueStartPos);
				float tempKnobLoopEndPos = (knobLoopEndPos - knobCueStartPos ) / (knobCueEndPos - knobCueStartPos);
				loadSample(path);

				params[CUESTART_PARAM].setValue(0);
				params[CUEEND_PARAM].setValue(1);

				if (tempKnobLoopStartPos < 0)
					params[LOOPSTART_PARAM].setValue(0);
				else				
					params[LOOPSTART_PARAM].setValue(tempKnobLoopStartPos);
				if (tempKnobLoopEndPos > 1)
					params[LOOPEND_PARAM].setValue(1);
				else
					params[LOOPEND_PARAM].setValue(tempKnobLoopEndPos);
			}
		}

		fileLoaded = true;
		fileFound = true;
		channels = fileChannels;
	}
/*

																					██╗░░░░░░█████╗░░█████╗░██████╗░
																					██║░░░░░██╔══██╗██╔══██╗██╔══██╗
																					██║░░░░░██║░░██║███████║██║░░██║
																					██║░░░░░██║░░██║██╔══██║██║░░██║
																					███████╗╚█████╔╝██║░░██║██████╔╝
																					╚══════╝░╚════╝░╚═╝░░╚═╝╚═════╝░
*/
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
			loadSample(path);
			storedPath = std::string(path);
		} else {
			restoreLoadFromPatch = true;
			fileLoaded = true;
		}
		if ((storedPath == "" || fileFound == false) && !toSave) {
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

		// metamodule change
		//tempBuffer[0].clear();
		//tempBuffer[1].clear();
		vector<float> tempBuffer[2];

		unsigned int c;
		unsigned int sr;
		drwav_uint64 tsc;
		float* pSampleData;
		pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &tsc);

		if (pSampleData != NULL && tsc > minSamplesToLoad * c) {

			fileFound = true;
			fileChannels = c;
			sampleRate = sr * 2;
			fileSampleRate = sr;

			for (int c=0; c < 16; c++)
				samplePos[c] = 0;
			
			playBuffer[LEFT][0].clear();
			playBuffer[LEFT][1].clear();
			playBuffer[RIGHT][0].clear();
			playBuffer[RIGHT][1].clear();
			tempBuffer[LEFT].clear();
			tempBuffer[RIGHT].clear();
			displayBuff.clear();

			// metamodule change
			vector<float>().swap(playBuffer[LEFT][0]);
			vector<float>().swap(playBuffer[RIGHT][0]);
			vector<float>().swap(playBuffer[LEFT][1]);
			vector<float>().swap(playBuffer[RIGHT][1]);

			if (!unlimitedRecording) {
				if (tsc > recordingLimit / 2)
					tsc = recordingLimit / 2;	// set memory allocation limit to 200Mb for samples (~18mins at 48.000khz MONO)
			}

			if (sr == APP->engine->getSampleRate()) {			//  **************************   NO RESAMPLE   ************************
				for (unsigned int i=0; i < tsc; i = i + c) {
					playBuffer[LEFT][0].push_back(pSampleData[i] * 5);
					playBuffer[LEFT][0].push_back(0);
					if (fileChannels == 2) {
						playBuffer[RIGHT][0].push_back(pSampleData[i+1] * 5);
						playBuffer[RIGHT][0].push_back(0);
					}
				}
				totalSampleC = playBuffer[LEFT][0].size();
				totalSamples = totalSampleC-1;					// *****   DA VERIFICARE se è -2 ********************************************
				drwav_free(pSampleData);

				for (unsigned int i = 1; i < totalSamples; i = i+2) { // **************** tempSamples  o tempSampleC ????
					playBuffer[LEFT][0][i] = playBuffer[LEFT][0][i-1] * .5f + playBuffer[LEFT][0][i+1] * .5f;
					if (fileChannels == 2)
						playBuffer[RIGHT][0][i] = playBuffer[RIGHT][0][i-1] * .5f + playBuffer[RIGHT][0][i+1] * .5f;
				}

				playBuffer[LEFT][0][totalSamples] = playBuffer[LEFT][0][totalSamples-1] * .5f; // halve the last sample
				if (fileChannels == 2)
					playBuffer[RIGHT][0][totalSamples] = playBuffer[RIGHT][0][totalSamples-1] * .5f;

				resampled = false;

			} else if (sr == APP->engine->getSampleRate() * 2) {	// ***** LOAD DIRECTLY OVERSAMPLED, NO RESAMPLE *****
				for (unsigned int i=0; i < tsc; i = i + c) {
					playBuffer[LEFT][0].push_back(pSampleData[i] * 5);
					if (fileChannels == 2) {
						playBuffer[RIGHT][0].push_back(pSampleData[i+1] * 5);
					}
				}
				totalSampleC = playBuffer[LEFT][0].size();
				totalSamples = totalSampleC-1;					// *****   DA VERIFICARE se è -2 ********************************************
				drwav_free(pSampleData);

				resampled = false;

				sampleRate = APP->engine->getSampleRate() * 2;

			} else {											// ***************** RESAMPLE ****************************************
				for (unsigned int i=0; i < tsc; i = i + c) {
					tempBuffer[LEFT].push_back(pSampleData[i] * 5);
					tempBuffer[LEFT].push_back(0);
					if (fileChannels == 2) {
						tempBuffer[RIGHT].push_back(pSampleData[i+1] * 5);
						tempBuffer[RIGHT].push_back(0);
					}
				}

				drwav_free(pSampleData);

				drwav_uint64 tempSampleC = tempBuffer[LEFT].size();
				drwav_uint64 tempSamples = tempSampleC-1;					// *****   DA VERIFICARE se è -2 ********************************************
				

				for (unsigned int i = 1; i < tempSamples; i = i+2) { // **************** tempSamples  o tempSampleC ????
					tempBuffer[LEFT][i] = tempBuffer[LEFT][i-1] * .5f + tempBuffer[LEFT][i+1] * .5f;
					if (fileChannels == 2)
						tempBuffer[RIGHT][i] = tempBuffer[RIGHT][i-1] * .5f + tempBuffer[RIGHT][i+1] * .5f;
				}

				tempBuffer[LEFT][tempSamples] = tempBuffer[LEFT][tempSamples-1] * .5f; // halve the last sample
				if (fileChannels == 2)
					tempBuffer[RIGHT][tempSamples] = tempBuffer[RIGHT][tempSamples-1] * .5f;

				// ***************************************************************************

				double resampleCoeff = sampleRate * .5 / (APP->engine->getSampleRate());
				double currResamplePos = 0;
				int floorCurrResamplePos = 0;

				playBuffer[LEFT][0].push_back(tempBuffer[LEFT][0]);
				if (fileChannels == 2)
					playBuffer[RIGHT][0].push_back(tempBuffer[RIGHT][0]);

				currResamplePos += resampleCoeff;
				floorCurrResamplePos = floor(currResamplePos);
				int temp;

				while ( floorCurrResamplePos < 1 ) {
					temp = tempBuffer[LEFT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[LEFT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					playBuffer[LEFT][0].push_back(temp);
					if (fileChannels == 2) {
						temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
						playBuffer[RIGHT][0].push_back(temp);
					}
					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				} 

				while ( currResamplePos < tempSamples - 1) {
					playBuffer[LEFT][0].push_back(	hermiteInterpol(tempBuffer[LEFT][floorCurrResamplePos - 1],
																	tempBuffer[LEFT][floorCurrResamplePos],
																	tempBuffer[LEFT][floorCurrResamplePos + 1],
																	tempBuffer[LEFT][floorCurrResamplePos + 2],
																	currResamplePos - floorCurrResamplePos)
												);
					if (fileChannels == 2)
						playBuffer[RIGHT][0].push_back(	hermiteInterpol(tempBuffer[RIGHT][floorCurrResamplePos - 1],
																		tempBuffer[RIGHT][floorCurrResamplePos],
																		tempBuffer[RIGHT][floorCurrResamplePos + 1],
																		tempBuffer[RIGHT][floorCurrResamplePos + 2],
																		currResamplePos - floorCurrResamplePos)
													);
					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				}

				while ( floorCurrResamplePos < double(tempSampleC) ) {
					temp = tempBuffer[LEFT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[LEFT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					playBuffer[LEFT][0].push_back(temp);
					if (fileChannels == 2) {
						temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
						playBuffer[RIGHT][0].push_back(temp);
					}
					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				}

				// metamodule change
				//tempBuffer[LEFT].clear();
				//tempBuffer[RIGHT].clear();

				// ***************************************************************************

				totalSampleC = playBuffer[LEFT][0].size();
				totalSamples = totalSampleC-1;					// *****   DA VERIFICARE se è -2 ********************************************

				sampleRate = APP->engine->getSampleRate() * 2;

				resampled = true;
			}

			calcBiquadLpf(20000.0, sampleRate, 1);
			for (unsigned int i = 0; i < totalSampleC; i++) {
				playBuffer[LEFT][1].push_back(biquadLpf(playBuffer[LEFT][0][i]));
				if (fileChannels == 2)
					playBuffer[RIGHT][1].push_back(biquadLpf2(playBuffer[RIGHT][0][i]));
			}

			vector<double>().swap(displayBuff);
			for (int i = 0; i < floor(totalSampleC); i = i + floor(totalSampleC/140))
				displayBuff.push_back(playBuffer[0][0][i]);

			seconds = totalSampleC * 0.5 / (APP->engine->getSampleRate());
			minutes = (seconds / 60) % 60;
			seconds = seconds % 60;

			timeDisplay = std::to_string(minutes) + ":";
			if (seconds < 10)
				timeDisplay += "0";
			timeDisplay += std::to_string(seconds);

			if (loadFromPatch) {
				if (storedPath != "")
					path = storedPath;
				else
					path = "_unknown_.wav";
			}

			char* pathDup = strdup(path.c_str());
			fileDescription = basename(pathDup);

			if (loadFromPatch) {
				if (storedPath != "")
					fileDescription = "(!)"+fileDescription;
			}

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

			fileDisplay = fileDisplay.substr(0, 12);
			samplerateDisplay = std::to_string(fileSampleRate);
			channelsDisplay = std::to_string(fileChannels) + "Ch";
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

				if (!firstLoad || toSave) {
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

				toSave = false;
				infoToSave = "";
			
			} else {
				knobCueStartPos = params[CUESTART_PARAM].getValue();
				knobCueEndPos = params[CUEEND_PARAM].getValue();
				prevKnobCueStartPos = knobCueStartPos;
				prevKnobCueEndPos = knobCueEndPos;
				knobLoopStartPos = params[LOOPSTART_PARAM].getValue();
				knobLoopEndPos = params[LOOPEND_PARAM].getValue();
				prevKnobLoopStartPos = knobLoopStartPos;
				prevKnobLoopEndPos = knobLoopEndPos;

				cueStartPos = floor(totalSamples * knobCueStartPos);
				cueEndPos = ceil(totalSamples * knobCueEndPos);
				loopStartPos = floor(totalSamples * knobLoopStartPos);
				loopEndPos = ceil(totalSamples * knobLoopEndPos);

				scanCueStartSample = cueStartPos;
				scanCueEndSample = cueEndPos;
				scanLoopStartSample = loopStartPos;
				scanLoopEndSample = loopEndPos;

				toSave = true;
				infoToSave = "S";
			}

			recSeconds = 0;
			recMinutes = 0;
			
			firstLoad = false;

			fileLoaded = true;
			channels = fileChannels;

			//debugDisplay2 = to_string(totalSamples);

		} else {
			fileFound = false;
			fileLoaded = false;
			//storedPath = path;
			if (loadFromPatch)
				path = storedPath;
			fileDescription = "(!)"+path;
			fileDisplay = "";
			timeDisplay = "";
			recSamples = 0;
			recTimeDisplay = "";
			channelsDisplay = "";
		}
	};
	
	void clearSlot() {
		fileLoaded = false;
		fileFound = false;
		loadFromPatch = false;
		restoreLoadFromPatch = false;
		storedPath = "";
		fileDescription = "--none--";
		fileDisplay = "";
		timeDisplay = "";
		recSamples = 0;
		recTimeDisplay = "";
		channelsDisplay = "";
		playBuffer[LEFT][0].clear();
		playBuffer[RIGHT][0].clear();
		playBuffer[LEFT][1].clear();
		playBuffer[RIGHT][1].clear();
		displayBuff.clear();

		// metamodule change
		vector<float>().swap(playBuffer[LEFT][0]);
		vector<float>().swap(playBuffer[RIGHT][0]);
		vector<float>().swap(playBuffer[LEFT][1]);
		vector<float>().swap(playBuffer[RIGHT][1]);

		totalSampleC = 0;
		totalSamples = 0;
		resampled = false;
		toSave = false;
		infoToSave= "";
		for (int i=0; i < 16; i++)
			fadingValue[i] = 1;
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
				params[PHASESCAN_SWITCH].setValue(0);
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
				params[PHASESCAN_SWITCH].setValue(1);
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
				params[PHASESCAN_SWITCH].setValue(0);
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

/*
																				██████╗░██████╗░░█████╗░░█████╗░███████╗░██████╗░██████╗
																				██╔══██╗██╔══██╗██╔══██╗██╔══██╗██╔════╝██╔════╝██╔════╝
																				██████╔╝██████╔╝██║░░██║██║░░╚═╝█████╗░░╚█████╗░╚█████╗░
																				██╔═══╝░██╔══██╗██║░░██║██║░░██╗██╔══╝░░░╚═══██╗░╚═══██╗
																				██║░░░░░██║░░██║╚█████╔╝╚█████╔╝███████╗██████╔╝██████╔╝
																				╚═╝░░░░░╚═╝░░╚═╝░╚════╝░░╚════╝░╚══════╝╚═════╝░╚═════╝░
*/
	void process(const ProcessArgs &args) override {

		//if (!disableNav) {
		if (!disableNav && !loadFromPatch) {
			nextSample = params[NEXTSAMPLE_PARAM].getValue();
			if (fileLoaded && fileFound && recordingState == 0 && nextSample && !prevNextSample) {
				for (int i = 0; i < 16; i++)
					play[i] = false;
				currentFile++;
				if (currentFile >= int(currentFolderV.size()))
					currentFile = 0;
				loadSample(currentFolderV[currentFile]);
			}
			prevNextSample = nextSample;

			prevSample = params[PREVSAMPLE_PARAM].getValue();
			if (fileLoaded && fileFound && recordingState == 0 && prevSample && !prevPrevSample) {
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
		
		phaseScan = params[PHASESCAN_SWITCH].getValue();
		lights[PHASESCAN_LIGHT].setBrightness(phaseScan);

		lights[MONITOR_LIGHT].setBrightness(params[MONITOR_SWITCH].getValue());

		lights[TRIGMODE_LIGHT].setBrightness(params[TRIGMODE_SWITCH].getValue());

		xFade = params[XFADE_PARAM].getValue();

		if (recButton == 0)
			chan = std::max(1, inputs[VO_INPUT].getChannels());
		else
			chan = 1;

		sumOutput = 0;
		sumOutputR = 0;

		trigButValue = params[TRIGBUT_PARAM].getValue();
		lights[TRIGBUT_LIGHT].setBrightness(trigButValue);

		stretchKnob = params[STRETCH_PARAM].getValue();
		cycleKnob = params[STR_SIZE_PARAM].getValue();
		//stretchMaxPos = oneMsSamples * params[STR_SIZE_PARAM].getValue();
		stretchMaxPos = oneMsSamples * cycleKnob;

		// *********************************************************************************************** PLAY SECTION *******************************		

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
						knobCueEndPos = tempKnob;
					} else {
						scanCueEndSample--;
						if (scanCueEndSample < cueStartPos) {
							cueEndPos = cueStartPos;
							searchingCueEndPhase = false;
							tempKnob = cueEndPos/totalSamples;
							params[CUEEND_PARAM].setValue(tempKnob);
							knobCueEndPos = tempKnob;

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

			sustainValue = params[SUSTAIN_PARAM].getValue();

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
					masterLevel[c] = params[VOL_PARAM].getValue() + (inputs[VOL_INPUT].getVoltage(c) * 0.1);
				else
					masterLevel[c] = params[VOL_PARAM].getValue() + (inputs[VOL_INPUT].getVoltage(0) * 0.1);
				
				if (masterLevel[c] > 1)
					masterLevel[c] = 1;
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

								grainSampleCount[c] = 0;
								grainCount[c] = 1;
								grainFade[c] = false;

								if (reverseStart) {
									reversePlaying[c] = REVERSE;
									samplePos[c] = floor(cueEndPos-1);
									currSampleWeight[c] = sampleCoeff;
								} else {
									reversePlaying[c] = FORWARD;
									samplePos[c] = floor(cueStartPos+1);
									currSampleWeight[c] = sampleCoeff;
								}
								stage[c] = ATTACK_STAGE;
								attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue());

								if (attackValue > maxAdsrTime) {
									attackValue = maxAdsrTime;
								} else if (attackValue < minAdsrTime) {
									attackValue = minAdsrTime;
								}
								stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);
							} else {
								if (stage[c] == RELEASE_STAGE) {
									stage[c] = ATTACK_STAGE;
									attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue());
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
									stage[c] = RELEASE_STAGE;
									releaseNew[c] = true;
									releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue());
									if (releaseValue > maxAdsrTime) {
										releaseValue = maxAdsrTime;
									} else 	if (releaseValue < minAdsrTime) {
										releaseValue = minAdsrTime;
									}

									if (stageLevel[c] != 0)
										stageCoeff[c] = stageLevel[c] / (args.sampleRate * releaseValue);
									else
										stageCoeff[c] = 1;

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

						if ((trigValue[c] >= 1 && prevTrigValue[c] < 1)) {
						
							switch (trigType) {
								case START_STOP:									// trig type: Start/Stop

									if (play[c]) {

										if (stage[c] != RELEASE_STAGE) {

											stage[c] = RELEASE_STAGE;
											releaseNew[c] = true;
											releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue());
											if (releaseValue > maxAdsrTime) {
												releaseValue = maxAdsrTime;
											} else 	if (releaseValue < minAdsrTime) {
												releaseValue = minAdsrTime;
											}

											if (stageLevel[c] != 0)
												stageCoeff[c] = stageLevel[c] / (args.sampleRate * releaseValue);
											else
												stageCoeff[c] = 1;

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
											attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue());
											if (attackValue > maxAdsrTime) {
												attackValue = maxAdsrTime;
											} else if (attackValue < minAdsrTime) {
												attackValue = minAdsrTime;
											}
											stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);
										}
										
									} else {
										play[c] = true;

										grainSampleCount[c] = 0;
										grainCount[c] = 1;
										grainFade[c] = false;
										
										if (reverseStart) {
											reversePlaying[c] = REVERSE;
											samplePos[c] = floor(cueEndPos-1);
											currSampleWeight[c] = sampleCoeff;
										} else {
											reversePlaying[c] = FORWARD;
											samplePos[c] = floor(cueStartPos+1);
											currSampleWeight[c] = sampleCoeff;
										}
										stage[c] = ATTACK_STAGE;
										attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue());
										if (attackValue > maxAdsrTime) {
											attackValue = maxAdsrTime;
										} else if (attackValue < minAdsrTime) {
											attackValue = minAdsrTime;
										}
										stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);
									}
								break;

								case START_RESTART:									// trig type: START RESTART
									if (!play[c]) {		// ******* START PLAYBACK
										
										play[c] = true;

										grainSampleCount[c] = 0;
										grainCount[c] = 1;
										grainFade[c] = false;
										
										if (reverseStart) {
											reversePlaying[c] = REVERSE;
											samplePos[c] = floor(cueEndPos-1);
											currSampleWeight[c] = sampleCoeff;
										} else {
											reversePlaying[c] = FORWARD;
											samplePos[c] = floor(cueStartPos+1);
											currSampleWeight[c] = sampleCoeff;
										}
										stage[c] = ATTACK_STAGE;
										attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue());
										if (attackValue > maxAdsrTime) {
											attackValue = maxAdsrTime;
										} else if (attackValue < minAdsrTime) {
											attackValue = minAdsrTime;
										}
										stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);

									} else {	// ******* RESTART PLAYBACK

										grainSampleCount[c] = 0;
										grainCount[c] = 1;
										grainFade[c] = false;

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
										} else {
											reversePlaying[c] = FORWARD;
											samplePos[c] = floor(cueStartPos+1);
											currSampleWeight[c] = sampleCoeff;
										}
										stage[c] = ATTACK_STAGE;
										attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue());
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
						prevTrigValue[c] = trigValue[c];

					break;
				}

				currentOutput = 0;
				currentOutputR = 0;

				if (play[c]) {
					if (inputs[VO_INPUT].isConnected()) {
						voct[c] = inputs[VO_INPUT].getVoltage(c);
						if (voct[c] != prevVoct[c]) {
							speedVoct[c] = pow(2,voct[c]);
							prevVoct[c] = voct[c];
						}
						distancePos[c] = sampleCoeff * speedVoct[c];
					} else
						distancePos[c] = sampleCoeff;

					//if (play[c] && voct[c] < voctDisplay) {
					if (voct[c] < voctDisplay) {
						currentDisplay = c;
						voctDisplay = voct[c];
					}



					switch (reversePlaying[c]) {
						case FORWARD:		// ********************************************************************************************  FORWARD PLAYBACK   ***********

							if (loop && samplePos[c] > floor(loopEndPos - (fadeSamples * distancePos[c]))) {		// *** REACHED END OF LOOP ***

								grainSampleCount[c] = 0;
								grainCount[c] = 1;
								grainFade[c] = false;

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
									releaseNew[c] = true;
									releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue());
									if (releaseValue > maxAdsrTime) {
										releaseValue = maxAdsrTime;
									} else 	if (releaseValue < minAdsrTime) {
										releaseValue = minAdsrTime;
									}
									if (stageLevel[c] != 0)
										stageCoeff[c] = stageLevel[c] / (args.sampleRate * releaseValue);
									else
										stageCoeff[c] = 1;

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
										grainSampleCount[c] = 0;
										grainCount[c] = 1;
										grainFade[c] = false;

										reversePlaying[c] = REVERSE;
										samplePos[c] = floor(cueEndPos)-1;
										currSampleWeight[c] = sampleCoeff;

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
										grainSampleCount[c] = 0;
										grainCount[c] = 1;
										grainFade[c] = false;

										fadingType[c] = CROSS_FADE;
										fadingValue[c] = 1.f;
										fadedPosition[c] = samplePos[c];
										samplePos[c] = floor(cueStartPos)+1;
										currSampleWeight[c] = sampleCoeff;

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

						case REVERSE:		// ********************************************************************************************  REVERSE PLAYBACK   ***********

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
									
									grainSampleCount[c] = 0;
									grainCount[c] = 1;
									grainFade[c] = false;

									samplePos[c] = floor(loopStartPos)+1;
									currSampleWeight[c] = sampleCoeff;

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

									grainSampleCount[c] = 0;
									grainCount[c] = 1;
									grainFade[c] = false;

									samplePos[c] = floor(loopEndPos)-1;
									currSampleWeight[c] = sampleCoeff;

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
									releaseNew[c] = true;
									releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue());
									if (releaseValue > maxAdsrTime) {
										releaseValue = maxAdsrTime;
									} else 	if (releaseValue < minAdsrTime) {
										releaseValue = minAdsrTime;
									}

									if (stageLevel[c] != 0)
										stageCoeff[c] = stageLevel[c] / (args.sampleRate * releaseValue);
									else
										stageCoeff[c] = 1;

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

										grainSampleCount[c] = 0;
										grainCount[c] = 1;
										grainFade[c] = false;

										samplePos[c] = floor(cueStartPos)+1;
										currSampleWeight[c] = sampleCoeff;

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

										grainSampleCount[c] = 0;
										grainCount[c] = 1;
										grainFade[c] = false;

										samplePos[c] = floor(cueEndPos)-1;
										currSampleWeight[c] = sampleCoeff;

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

						/*
						// debuggrain
						if (samplePos[c] < 0 || samplePos[c] > totalSamples) {
							DEBUG(("samplePos " + to_string(samplePos[c]) + " / " + to_string(totalSamples)).c_str());
						}
						*/

						// *** SICKOSAMPLER USES HERMITE INTERPOLATION ONLY ***
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
								/*
								currentOutput = playBuffer[LEFT][antiAlias][floor(samplePos[c])];
								if (channels == 2)
									currentOutputR = playBuffer[RIGHT][antiAlias][floor(samplePos[c])];
								*/
								if (floor(samplePos[c]) >= 0 && floor(samplePos[c]) < totalSampleC) {
									currentOutput = playBuffer[LEFT][antiAlias][floor(samplePos[c])];
									if (channels == 2)
										currentOutputR = playBuffer[RIGHT][antiAlias][floor(samplePos[c])];	
								}
							}
						}

						
						// ******************************************************** G R A I N    F A D E ***********************************************
						
						if (grainFade[c]) {
							grainFadeValue[c] -= grainFadeCoeff[c];
							if (grainFadeValue[c] < 0) {
								grainFade[c] = false;
							} else {
								float tempGrainFadeOutput;

								/*
								// debuggrain
								if (grainPos[c] < 0 || grainPos[c] > totalSamples) {
									DEBUG(("GRAIN " + to_string(grainPos[c]) + " / " + to_string(totalSamples)).c_str());
								}
								*/
								
								if (floor(grainPos[c]) > 0 && floor(grainPos[c]) < totalSamples - 1) {

									tempGrainFadeOutput = hermiteInterpol(playBuffer[LEFT][antiAlias][floor(grainPos[c])-1],
																playBuffer[LEFT][antiAlias][floor(grainPos[c])],
																playBuffer[LEFT][antiAlias][floor(grainPos[c])+1],
																playBuffer[LEFT][antiAlias][floor(grainPos[c])+2],
																currSampleWeight[c]);
									currentOutput = (currentOutput * (1-grainFadeValue[c])) + (tempGrainFadeOutput * grainFadeValue[c]);
									if (channels == 2) {
										tempGrainFadeOutput = hermiteInterpol(playBuffer[RIGHT][antiAlias][floor(grainPos[c])-1],
																playBuffer[RIGHT][antiAlias][floor(grainPos[c])],
																playBuffer[RIGHT][antiAlias][floor(grainPos[c])+1],
																playBuffer[RIGHT][antiAlias][floor(grainPos[c])+2],
																currSampleWeight[c]);
										currentOutputR = (currentOutputR * (1-grainFadeValue[c])) + (tempGrainFadeOutput * grainFadeValue[c]);
									}

								} else { // if playing sample is the first or one of the last 3 -> no interpolation
									/*
									tempGrainFadeOutput = playBuffer[LEFT][antiAlias][floor(grainPos[c])];
									currentOutput = (currentOutput * (1-grainFadeValue[c])) + (tempGrainFadeOutput * grainFadeValue[c]);
									if (channels == 2) {
										tempGrainFadeOutput = playBuffer[RIGHT][antiAlias][floor(grainPos[c])];
										currentOutputR = (currentOutputR * (1-grainFadeValue[c])) + (tempGrainFadeOutput * grainFadeValue[c]);
									}
									*/
									if (floor(grainPos[c]) >= 0 && floor(grainPos[c]) < totalSampleC) {
										tempGrainFadeOutput = playBuffer[LEFT][antiAlias][floor(grainPos[c])];
										currentOutput = (currentOutput * (1-grainFadeValue[c])) + (tempGrainFadeOutput * grainFadeValue[c]);
										if (channels == 2) {
											tempGrainFadeOutput = playBuffer[RIGHT][antiAlias][floor(grainPos[c])];
											currentOutputR = (currentOutputR * (1-grainFadeValue[c])) + (tempGrainFadeOutput * grainFadeValue[c]);
										}
									}
								}


								if (!reversePlaying[c])
									grainPos[c] += distancePos[c];
								else
									grainPos[c] -= distancePos[c];
							}
						}
						
						// --------------------- A D S R

						switch (stage[c]) {
							case ATTACK_STAGE:
								stageLevel[c] += stageCoeff[c];
								if (stageLevel[c] > 1) {
									stageLevel[c] = 1;
									stage[c] = DECAY_STAGE;
									decayValue = convertCVToSeconds(params[DECAY_PARAM].getValue());
									if (decayValue > maxAdsrTime) {
										decayValue = maxAdsrTime;
									} else if (decayValue < minAdsrTime) {
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
								if (stageLevel[c] < 0) {	// if release has ended
									stageLevel[c] = 0;

									stage[c] = STOP_STAGE;
									play[c] = false;
									
									if (releaseNew[c]) {
										releaseNew[c] = false;
										if (polyOuts) {
											eor[c] = true;
											eorTime[c] = oneMsSamples;
										} else {
											if (c == currentDisplay) {
												eor[0] = true;
												eocTime[0] = oneMsSamples;
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

/*
																													███████╗░█████╗░██████╗░███████╗
																													██╔════╝██╔══██╗██╔══██╗██╔════╝
																													█████╗░░███████║██║░░██║█████╗░░
																													██╔══╝░░██╔══██║██║░░██║██╔══╝░░
																													██║░░░░░██║░░██║██████╔╝███████╗
																													╚═╝░░░░░╚═╝░░╚═╝╚═════╝░╚══════╝
*/
						
						switch (fadingType[c]) {
							case NO_FADE:
							break;

							case CROSS_FADE:
								if (fadingValue[c] > 0) {
									fadingValue[c] -= fadeCoeff;
									switch (reversePlaying[c]) {
										case FORWARD:
											
											if (floor(fadedPosition[c]) < totalSampleC) {
												currentOutput *= 1 - fadingValue[c];
												currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c]);
												if (channels == 2) {
													currentOutputR *= 1 - fadingValue[c];
													currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c]);
												}
											} else {
												fadingType[c] = NO_FADE;
											}
											
											fadedPosition[c] += distancePos[c];
										break;
										case REVERSE:
											
											if (floor(fadedPosition[c]) >= 0) {
												currentOutput *= 1 - fadingValue[c];
												currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c] * -1);
												if (channels == 2) {
													currentOutputR *= 1 - fadingValue[c];
													currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c] * -1);
												}
											} else {
												fadingType[c] = NO_FADE;
											}
											
											fadedPosition[c] -= distancePos[c];
											
										break;
									}
								} else {
									fadingType[c] = NO_FADE;
								}
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
											
											if (fadedPosition[c] >= 0) {
												currentOutput *= 1 - fadingValue[c];
												currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c] * -1);
												if (channels == 2) {
													currentOutputR *= 1 - fadingValue[c];
													currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c] * -1);
												}
											} else {
												fadingType[c] = NO_FADE;
											}
											
											fadedPosition[c] -= distancePos[c];
										break;
										case REVERSE:
											
											if (fadedPosition[c] < totalSampleC) {
												currentOutput *= 1 - fadingValue[c];
												currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c]);
												if (channels == 2) {
													currentOutputR *= 1 - fadingValue[c];
													currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel[c] * stageLevel[c]);
												}
											} else {
												fadingType[c] = NO_FADE;
											}
											
											fadedPosition[c] += distancePos[c];
										break;
									}
								} else
									fadingType[c] = NO_FADE;
							break;
						}

						// ------------------------------------------------------- U P D A T E    S A M P L E   P O S I T I O N -----------------
						
						//if (params[STRETCH_PARAM].getValue() != 1) {
						if (stretchKnob != 1) {
							grainSampleCount[c]++;
							//if (params[STRETCH_PARAM].getValue() > 1) {
							if (stretchKnob > 1) {

								if (grainSampleCount[c] > stretchMaxPos ) {		//
									grainCount[c]++;
									grainPos[c] = samplePos[c];
									grainFadeValue[c] = 1;

									//grainFadeCoeff[c] = 10 / (params[STR_SIZE_PARAM].getValue() * oneMsSamples);
									grainFadeCoeff[c] = 10 / (cycleKnob * oneMsSamples);
									grainFade[c] = true;

									//if (grainCount[c] > (params[STRETCH_PARAM].getValue())) {
									if (grainCount[c] > stretchKnob) {
										//double tempStretch1 = params[STRETCH_PARAM].getValue()-floor(params[STRETCH_PARAM].getValue());
										double tempStretch1 = stretchKnob-floor(stretchKnob);
										if (!reversePlaying[c])
											samplePos[c] -= stretchMaxPos * tempStretch1 * distancePos[c];
										else
											samplePos[c] += stretchMaxPos* tempStretch1 * distancePos[c];
										grainSampleCount[c] = -stretchMaxPos * tempStretch1;
										grainCount[c] = 1;
									} else {
										if (!reversePlaying[c])
											samplePos[c] -= stretchMaxPos * distancePos[c];
										else
											samplePos[c] += stretchMaxPos * distancePos[c];
										grainSampleCount[c] = grainSampleCount[c] - stretchMaxPos;
									}
								}

							} else {

								if (grainSampleCount[c] > stretchMaxPos) {
									
									grainPos[c] = samplePos[c];
									grainFadeValue[c] = 1;
									
									//grainFadeCoeff[c] = 10 / (params[STR_SIZE_PARAM].getValue() * oneMsSamples);
									grainFadeCoeff[c] = 10 / (cycleKnob * oneMsSamples);
									grainFade[c] = true;
									
									if (!reversePlaying[c])
										//samplePos[c] += ((stretchMaxPos / params[STRETCH_PARAM].getValue()) - stretchMaxPos) * distancePos[c];
										samplePos[c] += ((stretchMaxPos / stretchKnob) - stretchMaxPos) * distancePos[c];
									else
										//samplePos[c] -= ((stretchMaxPos / params[STRETCH_PARAM].getValue()) - stretchMaxPos) * distancePos[c];
										samplePos[c] -= ((stretchMaxPos / stretchKnob) - stretchMaxPos) * distancePos[c];

									grainSampleCount[c] = grainSampleCount[c] - stretchMaxPos;
								}
							}
						}
						
						if (reversePlaying[c])
							samplePos[c] -= distancePos[c];
						else
							samplePos[c] += distancePos[c];

						currSampleWeight[c] = samplePos[c] - floor(samplePos[c]);

						// ------------------------------------------------------------ END POSITION UPDATE

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
						// *** HARD CLIP ***
						if (currentOutput > 10)
							currentOutput = 10;
						else if (currentOutput < -10)
							currentOutput = -10;
						if (channels == 2) {
							if (currentOutputR > 10)
								currentOutputR = 10;
							else if (currentOutputR < -10)
								currentOutputR = -10;
						}

						if (outputs[OUT_OUTPUT].isConnected()) {
							outputs[OUT_OUTPUT].setVoltage(currentOutput, c);
						}

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

					if (outputs[OUT_OUTPUT].isConnected()) {
						outputs[OUT_OUTPUT].setVoltage(sumOutput);
						outputs[OUT_OUTPUT].setChannels(1);
					}
					if (outputs[OUT_OUTPUT+1].isConnected()) {
						if (channels == 2)
							outputs[OUT_OUTPUT+1].setVoltage(sumOutputR);
						else
							outputs[OUT_OUTPUT+1].setVoltage(sumOutput);
						outputs[OUT_OUTPUT+1].setChannels(1);
					}

				break;

				case POLYPHONIC:			// polyphonic CABLES
					outputs[OUT_OUTPUT].setChannels(chan);
					outputs[OUT_OUTPUT+1].setChannels(chan);
				break;
			}

		}

/*

																							██████╗░███████╗░█████╗░░█████╗░██████╗░██████╗░
																							██╔══██╗██╔════╝██╔══██╗██╔══██╗██╔══██╗██╔══██╗
																							██████╔╝█████╗░░██║░░╚═╝██║░░██║██████╔╝██║░░██║
																							██╔══██╗██╔══╝░░██║░░██╗██║░░██║██╔══██╗██║░░██║
																							██║░░██║███████╗╚█████╔╝╚█████╔╝██║░░██║██████╔╝
																							╚═╝░░╚═╝╚══════╝░╚════╝░░╚════╝░╚═╝░░╚═╝╚═════╝░
*/

		recTrig = params[REC_PARAM].getValue();

		if (recTrig && !prevRecTrig) {
			if (!fileLoaded) {
				if (recordingState) {
					recButton = 0;
				} else {
					recButton = 1;
				}
				lights[REC_LIGHT].setBrightness(recButton);
			} else {
				lights[REC_LIGHT].setBrightness(0.f);
			}

		}
		prevRecTrig = recTrig;

		if (!fileLoaded) {
			// ********* NEW RECORDING **** START STOP COMMANDS

			masterLevel[recOutChan] = params[VOL_PARAM].getValue() + (inputs[VOL_INPUT].getVoltage(recOutChan) * 0.1);

			if (masterLevel[recOutChan] > 1)
				masterLevel[recOutChan] = 1;
			else if (masterLevel[recOutChan] < 0)
				masterLevel[recOutChan] = 0;
			
			if (recButton && recordingState == 0) {

				if (inputs[IN_INPUT].isConnected()){
					z1 = 0; z2 = 0; z1r = 0; z2r = 0;
					recordingState = 1;
					if (inputs[IN_INPUT+1].isConnected())
						recChannels = 2;
					else
						recChannels = 1;

					currentRecordingLimit = recordingLimit / recChannels;
					//DEBUG("[SickoSampler2] %d", currentRecordingLimit);

					fileChannels = recChannels;

					sampleRate = args.sampleRate * 2;
					calcBiquadLpf(20000.0, sampleRate, 1);

					playBuffer[LEFT][0].clear();
					playBuffer[LEFT][1].clear();
					if (recChannels == 2) {
						playBuffer[RIGHT][0].clear();
						playBuffer[RIGHT][1].clear();
					}

					// metamodule change
					vector<float>().swap(playBuffer[LEFT][0]);
					vector<float>().swap(playBuffer[RIGHT][0]);
					vector<float>().swap(playBuffer[LEFT][1]);
					vector<float>().swap(playBuffer[RIGHT][1]);

					currRecPosition = 1;

					// set position knob to 0 and 1
					knobCueStartPos = 0;
					knobLoopStartPos = 0;
					knobCueEndPos = 1;
					knobLoopEndPos = 1;

					params[CUESTART_PARAM].setValue(0);
					params[CUEEND_PARAM].setValue(1);
					params[LOOPSTART_PARAM].setValue(0);
					params[LOOPEND_PARAM].setValue(1);

					if (xFade > 0) {
						recFadeValue = 0;
						recFadeIn = true;
					} else {
						recFadeIn = false;
						recFadeValue = 1;
					}

					//currRecValue[LEFT] = inputs[IN_INPUT].getVoltage() * params[GAIN_PARAM].getValue() * recFadeValue;
					//currRecValue[RIGHT] = inputs[IN_INPUT+1].getVoltage() * params[GAIN_PARAM].getValue() * recFadeValue;
					currRecValue[LEFT] = inputs[IN_INPUT].getVoltageSum() * params[GAIN_PARAM].getValue() * recFadeValue;
					currRecValue[RIGHT] = inputs[IN_INPUT+1].getVoltageSum() * params[GAIN_PARAM].getValue() * recFadeValue;
					prevRecValue[LEFT] = currRecValue[LEFT];
					prevRecValue[RIGHT] = currRecValue[RIGHT];

					recFadeOut = false;
								
				} else {				// **** if rec is pressed or triggered and no left cable is plugged, do nothing
					params[REC_PARAM].setValue(0);
					lights[REC_LIGHT].setBrightness(0.f);
					recButton = 0;
				}

			} else if (recButton == 0 && recordingState == 1 && recFadeOut == false)  {	// ********** S T O P   R E C O R D I N G ****************
				if (recFadeValue > 0) {
					recFadeOut = true;

					if (xFade <= 0)
						recFadeValue = 0;
				}

				if (recFadeValue <= 0) {
					recordingState = 0;

					channels = recChannels;
					fileDescription = "_unknown_.wav";
					fileDisplay = "_unknown_";
					samplerateDisplay = std::to_string(int(args.sampleRate));
					channelsDisplay = std::to_string(recChannels) + "Ch";

					cueStartPos = 0;
					cueEndPos = totalSamples;
					loopStartPos = 0;
					loopEndPos = totalSamples;
					prevKnobCueStartPos = -1.f;
					prevKnobCueEndPos = 2.f;
					prevKnobLoopStartPos = -1.f;
					prevKnobLoopEndPos = 2.f;

					samplePos[recOutChan] = 0;

					totalSampleC = playBuffer[0][0].size();
					totalSamples = totalSampleC-1;

					vector<double>().swap(displayBuff);

					for (int i = 0; i < floor(totalSampleC); i = i + floor(totalSampleC/140))
						displayBuff.push_back(playBuffer[0][0][i]);

					seconds = totalSampleC * 0.5 / (APP->engine->getSampleRate());
					minutes = (seconds / 60) % 60;
					seconds = seconds % 60;

					timeDisplay = std::to_string(minutes) + ":";
					if (seconds < 10)
						timeDisplay += "0";
					timeDisplay += std::to_string(seconds);

					toSave = true;
					infoToSave ="S";
					fileLoaded = true;

					if (autoMonOff) {
						params[MONITOR_SWITCH].setValue(0.f);
					}

				}
			}
		}

		if (recordingState) {

			// **************************************************************************** R E C O R D I N G     S E C T I O N *****************
					
			if (recFadeIn) {
				if (recFadeValue < 1)
					recFadeValue += 1 / (convertCVToSeconds(xFade) * args.sampleRate);
				if (recFadeValue >= 1) {
					recFadeValue = 1;
					recFadeIn = false;
				}

			} else if (recFadeOut) {

				if (recFadeValue > 0)
					recFadeValue -= 1 / (convertCVToSeconds(xFade) * args.sampleRate);
				if (recFadeValue <= 0) {
					recFadeValue = 0;
					recFadeOut = false;
				}
			}

	
			playBuffer[LEFT][0].push_back(0);
			playBuffer[LEFT][1].push_back(0);
			playBuffer[LEFT][0].push_back(0);
			playBuffer[LEFT][1].push_back(0);
			if (recChannels == 2) {
				playBuffer[RIGHT][0].push_back(0);
				playBuffer[RIGHT][1].push_back(0);
				playBuffer[RIGHT][0].push_back(0);
				playBuffer[RIGHT][1].push_back(0);
			}

			int rc = 0;

			prevRecValue[rc] = currRecValue[rc];
			currRecValue[rc] = inputs[IN_INPUT+rc].getVoltage() * params[GAIN_PARAM].getValue() * recFadeValue;

			playBuffer[rc][0][currRecPosition] = currRecValue[rc];
			playBuffer[rc][0][currRecPosition-1] = (currRecValue[rc] + prevRecValue[rc]) / 2;

			playBuffer[rc][1][currRecPosition-1] = biquadLpf(playBuffer[rc][0][currRecPosition-1]); // filtered vector
			playBuffer[rc][1][currRecPosition] = biquadLpf(playBuffer[rc][0][currRecPosition]); // filtered vector

			if (recChannels == 2) {
				rc = 1;

				prevRecValue[rc] = currRecValue[rc];
				currRecValue[rc] = inputs[IN_INPUT+rc].getVoltage() * params[GAIN_PARAM].getValue() * recFadeValue;

				playBuffer[rc][0][currRecPosition] = currRecValue[rc];
				playBuffer[rc][0][currRecPosition-1] = (currRecValue[rc] + prevRecValue[rc]) / 2;

				playBuffer[rc][1][currRecPosition-1] = biquadLpf2(playBuffer[rc][0][currRecPosition-1]); // filtered vector
				playBuffer[rc][1][currRecPosition] = biquadLpf2(playBuffer[rc][0][currRecPosition]); // filtered vector
			}

			recSamples++;
			currRecPosition = currRecPosition + 2;

			if (!unlimitedRecording && currRecPosition > currentRecordingLimit) {
				params[REC_PARAM].setValue(0);
				lights[REC_LIGHT].setBrightness(0);
				recButton = 0;
			}

		}	// end if RECORDING STATE = true

/*
																						███╗░░░███╗░█████╗░███╗░░██╗██╗████████╗░█████╗░██████╗░
																						████╗░████║██╔══██╗████╗░██║██║╚══██╔══╝██╔══██╗██╔══██╗
																						██╔████╔██║██║░░██║██╔██╗██║██║░░░██║░░░██║░░██║██████╔╝
																						██║╚██╔╝██║██║░░██║██║╚████║██║░░░██║░░░██║░░██║██╔══██╗
																						██║░╚═╝░██║╚█████╔╝██║░╚███║██║░░░██║░░░╚█████╔╝██║░░██║
																						╚═╝░░░░░╚═╝░╚════╝░╚═╝░░╚══╝╚═╝░░░╚═╝░░░░╚════╝░╚═╝░░╚═╝
*/


		if (!fileLoaded && !recordingState) {
			if (inputs[IN_INPUT+1].isConnected())
				channels = 2;
			else
				channels = 1;
		}

		monitorSwitch = int(params[MONITOR_SWITCH].getValue());
		if (monitorSwitch != prevMonitorSwitch) {
			monitorFadeValue = 0;
			monitorFade = true;
			if (monitorSwitch == 1 && recordingState == 1) {
				monitorFadeValue = 0;
				monitorFade = true;
			}
		}

		if (monitorFade) {
			if (monitorFadeValue < 1)
				monitorFadeValue += monitorFadeCoeff;
			else {
				monitorFadeValue = 1;
				monitorFade = false;
			}
		}

		switch (monitorSwitch) {
			case 0:											// *** MONITOR OFF ***
				if (monitorFade) {
					switch (polyOuts) {
						case MONOPHONIC:										// monophonic CABLES
							//sumOutput += inputs[IN_INPUT].getVoltage() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue) * masterLevel[0];
							sumOutput += inputs[IN_INPUT].getVoltageSum() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue) * masterLevel[0];
							if (channels == 2)
								//sumOutputR += inputs[IN_INPUT+1].getVoltage() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue) * masterLevel[0];
								sumOutputR += inputs[IN_INPUT+1].getVoltageSum() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue) * masterLevel[0];
							
							// *** HARD CLIP ***

							if (sumOutput > 10)
								sumOutput = 10;
							else if (sumOutput < -10)
								sumOutput = -10;
							if (channels == 2) {
								if (sumOutputR > 10)
									sumOutputR = 10;
								else if (sumOutputR < -10)
									sumOutputR = -10;
							}

							if (outputs[OUT_OUTPUT].isConnected()){
								outputs[OUT_OUTPUT].setVoltage(sumOutput);
								outputs[OUT_OUTPUT].setChannels(1);
							}

							if (outputs[OUT_OUTPUT+1].isConnected()) {
								if (channels == 2)
									outputs[OUT_OUTPUT+1].setVoltage(sumOutputR);
								else
									outputs[OUT_OUTPUT+1].setVoltage(sumOutput);
								outputs[OUT_OUTPUT+1].setChannels(1);
							}
						break;

						case POLYPHONIC:
							//currentOutput = outputs[OUT_OUTPUT].getVoltage(recOutChan) + (inputs[IN_INPUT].getVoltage() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue) * masterLevel[0]);
							//currentOutputR = outputs[OUT_OUTPUT+1].getVoltage(recOutChan) + (inputs[IN_INPUT+1].getVoltage() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue) * masterLevel[0]);
							currentOutput = outputs[OUT_OUTPUT].getVoltage(recOutChan) + (inputs[IN_INPUT].getVoltageSum() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue) * masterLevel[0]);
							currentOutputR = outputs[OUT_OUTPUT+1].getVoltage(recOutChan) + (inputs[IN_INPUT+1].getVoltageSum() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue) * masterLevel[0]);
							
							// *** HARD CLIP ***

							if (currentOutput > 10)
								currentOutput = 10;
							else if (currentOutput < -10)
								currentOutput = -10;
							if (channels == 2) {
								if (currentOutputR > 10)
									currentOutputR = 10;
								else if (currentOutputR < -10)
									currentOutputR = -10;
							}

							if (outputs[OUT_OUTPUT].isConnected()) {
								outputs[OUT_OUTPUT].setVoltage(currentOutput, recOutChan);
								outputs[OUT_OUTPUT].setChannels(chan);
							}

							if (outputs[OUT_OUTPUT+1].isConnected()) {
								if (channels == 2)
									outputs[OUT_OUTPUT+1].setVoltage(currentOutputR, recOutChan);
								else
									outputs[OUT_OUTPUT+1].setVoltage(currentOutput, recOutChan);
								outputs[OUT_OUTPUT+1].setChannels(chan);
							}
						break;
					}
				}
			break;

			case 1:								// *** ALWAYS MONITORING ***
				switch (polyOuts) {
					case MONOPHONIC:										// monophonic CABLES
						//sumOutput += inputs[IN_INPUT].getVoltage() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0];
						sumOutput += inputs[IN_INPUT].getVoltageSum() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0];
						if (channels == 2)
							//sumOutputR += inputs[IN_INPUT+1].getVoltage() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0];
							sumOutputR += inputs[IN_INPUT+1].getVoltageSum() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0];
						
						// *** HARD CLIP ***
						if (sumOutput > 10)
							sumOutput = 10;
						else if (sumOutput < -10)
							sumOutput = -10;
						if (channels == 2) {
							if (sumOutputR > 10)
								sumOutputR = 10;
							else if (sumOutputR < -10)
								sumOutputR = -10;
						}

						if (outputs[OUT_OUTPUT].isConnected()) {
							outputs[OUT_OUTPUT].setVoltage(sumOutput);
							outputs[OUT_OUTPUT].setChannels(1);
						}

						if (outputs[OUT_OUTPUT+1].isConnected()) {
							if (channels == 2)
								outputs[OUT_OUTPUT+1].setVoltage(sumOutputR);
							else
								outputs[OUT_OUTPUT+1].setVoltage(sumOutput);
							outputs[OUT_OUTPUT+1].setChannels(1);
						}
					break;

					case POLYPHONIC:
						//currentOutput = outputs[OUT_OUTPUT].getVoltage(recOutChan) + (inputs[IN_INPUT].getVoltage() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0]);
						//currentOutputR = outputs[OUT_OUTPUT+1].getVoltage(recOutChan) + (inputs[IN_INPUT+1].getVoltage() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0]);
						currentOutput = outputs[OUT_OUTPUT].getVoltage(recOutChan) + (inputs[IN_INPUT].getVoltageSum() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0]);
						currentOutputR = outputs[OUT_OUTPUT+1].getVoltage(recOutChan) + (inputs[IN_INPUT+1].getVoltageSum() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0]);
						
						// *** HARD CLIP ***
						if (currentOutput > 10)
							currentOutput = 10;
						else if (currentOutput < -10)
							currentOutput = -10;
						if (channels == 2) {
							if (currentOutputR > 10)
								currentOutputR = 10;
							else if (currentOutputR < -10)
								currentOutputR = -10;
						}
	
						if (outputs[OUT_OUTPUT].isConnected()) {
							outputs[OUT_OUTPUT].setVoltage(currentOutput, recOutChan);
							outputs[OUT_OUTPUT].setChannels(chan);
						}

						if (outputs[OUT_OUTPUT+1].isConnected()) {
							if (channels == 2)
								outputs[OUT_OUTPUT+1].setVoltage(currentOutputR, recOutChan);
							else
								outputs[OUT_OUTPUT+1].setVoltage(currentOutput, recOutChan);
							outputs[OUT_OUTPUT+1].setChannels(chan);
						}
					break;
				}
			break;
		}

		prevMonitorSwitch = monitorSwitch;

		// ******************************************************************************************************
		// *****************************  R E C O R D I N G        E N D  ***************************************
		// ******************************************************************************************************

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
};

/*


															███████╗███╗░░██╗██████╗░  ██████╗░██████╗░░█████╗░░█████╗░███████╗░██████╗░██████╗
															██╔════╝████╗░██║██╔══██╗  ██╔══██╗██╔══██╗██╔══██╗██╔══██╗██╔════╝██╔════╝██╔════╝
															█████╗░░██╔██╗██║██║░░██║  ██████╔╝██████╔╝██║░░██║██║░░╚═╝█████╗░░╚█████╗░╚█████╗░
															██╔══╝░░██║╚████║██║░░██║  ██╔═══╝░██╔══██╗██║░░██║██║░░██╗██╔══╝░░░╚═══██╗░╚═══██╗
															███████╗██║░╚███║██████╔╝  ██║░░░░░██║░░██║╚█████╔╝╚█████╔╝███████╗██████╔╝██████╔╝
															╚══════╝╚═╝░░╚══╝╚═════╝░  ╚═╝░░░░░╚═╝░░╚═╝░╚════╝░░╚════╝░╚══════╝╚═════╝░╚═════╝░

*/

struct SickoSampler2Display : TransparentWidget {
	SickoSampler2 *module;
	int frame = 0;
	SickoSampler2Display() {
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

				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 8);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
				nvgTextBox(args.vg, 3, 11,247, module->fileDisplay.c_str(), NULL);

				if (module->fileLoaded) {
					nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff));
					nvgTextBox(args.vg, 90, 11,97, module->timeDisplay.c_str(), NULL);
				} else if (module->recordingState) {
					module->recSeconds = module->recSamples / (APP->engine->getSampleRate());
					module->recMinutes = (module->recSeconds / 60) % 60;
					module->recSeconds = module->recSeconds % 60;
					module->recTimeDisplay = std::to_string(module->recMinutes) + ":";
					if (module->recSeconds < 10)
						module->recTimeDisplay += "0";
					module->recTimeDisplay += std::to_string(module->recSeconds);
					module->timeDisplay = module->recTimeDisplay;
					nvgTextBox(args.vg, 90, 11,97, module->timeDisplay.c_str(), NULL);
				}
	
				nvgFillColor(args.vg, nvgRGBA(0x88, 0xaa, 0xff, 0xff)); 
				nvgTextBox(args.vg, 115, 11,97, module->channelsDisplay.c_str(), NULL);

				nvgFillColor(args.vg, nvgRGBA(0xee, 0xee, 0x22, 0xff)); 
				nvgTextBox(args.vg, 137, 11,97, module->infoToSave.c_str(), NULL);

				
				//nvgTextBox(args.vg, 9, 26,120, module->debugDisplay.c_str(), NULL);
				//nvgTextBox(args.vg, 9, 36,120, module->debugDisplay2.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 26,120, module->debugDisplay3.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 36,120, module->debugDisplay4.c_str(), NULL);
				

				// Zero line
				nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x40));
				{
					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, 4, 31.5);
					nvgLineTo(args.vg, 143, 31.4);
					nvgClosePath(args.vg);
				}
				nvgStroke(args.vg);
		
				if (module->fileLoaded) {
					int xLine;
					// Playback line
					if (module->recordingState)
						nvgStrokeColor(args.vg, nvgRGBA(0xf5, 0x00, 0xf5, 0xff));
					else
						nvgStrokeColor(args.vg, nvgRGBA(0xf5, 0xf5, 0xf5, 0xff));
					nvgStrokeWidth(args.vg, 0.8);
					{
						nvgBeginPath(args.vg);
						xLine = 4 + floor(module->samplePos[module->currentDisplay] * 139 / module->totalSampleC);
						nvgMoveTo(args.vg, xLine, 13);
						nvgLineTo(args.vg, xLine, 50);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Cue Start line
					nvgStrokeColor(args.vg, nvgRGBA(0x00, 0xf0, 0x00, 0xf0));
					nvgStrokeWidth(args.vg, 1);
					{
						nvgBeginPath(args.vg);
						xLine = 4 + floor(module->cueStartPos * 139 / module->totalSampleC);
						nvgMoveTo(args.vg, xLine , 13);
						nvgLineTo(args.vg, xLine , 50);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Cue End line
					nvgStrokeColor(args.vg, nvgRGBA(0xf0, 0x00, 0x00, 0xf0));
					nvgStrokeWidth(args.vg, 1);
					{
						nvgBeginPath(args.vg);
						xLine = 4 + floor(module->cueEndPos * 139 / module->totalSampleC);
						nvgMoveTo(args.vg, xLine , 13);
						nvgLineTo(args.vg, xLine , 50);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Loop Start line
					nvgStrokeColor(args.vg, nvgRGBA(0xf8, 0xec, 0x2e, 0xf0));
					nvgStrokeWidth(args.vg, 1);
					{
						nvgBeginPath(args.vg);
						xLine = 4 + floor(module->loopStartPos * 139 / module->totalSampleC);
						nvgMoveTo(args.vg, xLine , 13);
						nvgLineTo(args.vg, xLine , 50);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Loop End line
					nvgStrokeColor(args.vg, nvgRGBA(0xea, 0x79, 0x26, 0xf0));
					nvgStrokeWidth(args.vg, 1);
					{
						nvgBeginPath(args.vg);
						xLine = 4 + floor(module->loopEndPos * 139 / module->totalSampleC);
						nvgMoveTo(args.vg, xLine , 13);
						nvgLineTo(args.vg, xLine , 50);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Waveform
					nvgStrokeColor(args.vg, nvgRGBA(0x22, 0x44, 0xc9, 0xc0));
					nvgSave(args.vg);
					Rect b = Rect(Vec(4, 13), Vec(139, 37));
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
		SickoSampler2 *module = dynamic_cast<SickoSampler2*>(this->module);
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
		SickoSampler2 *module = dynamic_cast<SickoSampler2 *>(this->module);
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
				std::string tempDisplay = " ";
				if (module->resampled) {
					tempDisplay += "resampled to " + std::to_string(int(APP->engine->getSampleRate()));
					if (module->toSave)
						tempDisplay += " - ";
				}
				if (module->toSave)
					tempDisplay += "to SAVE";
				if (tempDisplay != " ")
					menu->addChild(createMenuLabel(tempDisplay));

				menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot();}));

				menu->addChild(createMenuItem("Save FULL Sample", "", [=]() {module->menuSaveSample(SAVE_FULL);}));
				menu->addChild(createMenuItem("Save CUE Region", "", [=]() {module->menuSaveSample(SAVE_CUE);}));
				menu->addChild(createMenuItem("Save LOOP Region", "", [=]() {module->menuSaveSample(SAVE_LOOP);}));

				menu->addChild(createBoolPtrMenuItem("Trim Sample after Save", "", &module->trimOnSave));
				menu->addChild(createBoolPtrMenuItem("Save Oversampled", "", &module->saveOversampled));
			}

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

struct SickoSampler2Widget : ModuleWidget {
	SickoSampler2Widget(SickoSampler2 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SickoSampler2.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));  

		{
			SickoSampler2Display *display = new SickoSampler2Display();
			display->box.pos = mm2px(Vec(0.6, 23.5));
			display->box.size = mm2px(Vec(49.5, 17.9));
			display->module = module;
			addChild(display);
		}

		const float xInL = 6.f;
		const float xInR = 15.f;
		const float xGain = 25.5f;
		const float yGain = 17.1f;
		const float xRecBut = 36.f;
		const float xMon = 45.f;
		const float yMon = 17.5f;
		const float yInputs = 16.9f;

		const float xTrigIn = 6.5f;
		const float yTrigIn = 78.4f;
		const float xTrigBut = 16.1f;
		const float yTrigBut = 78.4f;

		const float xTrigGate = 26.85f;
		const float yTrigGate = 77.189f;
		const float xTrigMode = 34.903f;
		const float yTrigMode = 78.108f;

		const float xStart = 11.903f;
		const float xEnd = 24.903f;
		const float yCue = 52.647f;
		const float yLoop = 62.647f;

		const float xRev = 35.203f;
		const float yRev = 51.147f;
		const float xXfade = 43.903f;
		const float yXfade = 51.647f;

		const float xLp = 35.403f;
		const float yLp = 60.647f;

		const float xPng = 43.903f;
		const float yPng = 63.147f;

		const float xEnv1 = 8.6f;
		const float xEnv1Add = 12.f;
		const float yEnv1 = 89.7f;
		
		const float xScan = 45.f;
		const float yScan = 79.f;

		const float xStretch = 9.7f;
		const float yStretch = 103.f;

		const float xStretchSiz = 23.f;
		const float yStretchSiz = 103.2f;

		const float xVoct = 5.9f;
		const float yVoct = 117.8f;

		const float xVol = 15.5f;
		const float yVol = 117.8f;

		const float xInVol = 25.2f;
		const float yInVol = 117.8f;

		const float xOut1 = 36.1f;
		const float xOut2 = 45.1f;
		const float yOut1 = 104.4f;
		const float yOut2 = 117.3f;

		addParam(createParamCentered<VCVButton>(mm2px(Vec(4, 4)), module, SickoSampler2::PREVSAMPLE_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(46.8, 4)), module, SickoSampler2::NEXTSAMPLE_PARAM));

		//----------------------------------------------------------------------------------------------------------------------------

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInL, yInputs)), module, SickoSampler2::IN_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInR, yInputs)), module, SickoSampler2::IN_INPUT+1));
		
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xGain, yGain)), module, SickoSampler2::GAIN_PARAM));

		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xRecBut, yInputs)), module, SickoSampler2::REC_PARAM, SickoSampler2::REC_LIGHT));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xMon, yMon)), module, SickoSampler2::MONITOR_SWITCH, SickoSampler2::MONITOR_LIGHT));

		//------------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<CKSS>(mm2px(Vec(xTrigGate, yTrigGate)), module, SickoSampler2::TRIGGATEMODE_SWITCH));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xTrigMode, yTrigMode)), module, SickoSampler2::TRIGMODE_SWITCH, SickoSampler2::TRIGMODE_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xTrigIn, yTrigIn)), module, SickoSampler2::TRIG_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xTrigBut, yTrigBut)), module, SickoSampler2::TRIGBUT_PARAM, SickoSampler2::TRIGBUT_LIGHT));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xScan, yScan)), module, SickoSampler2::PHASESCAN_SWITCH, SickoSampler2::PHASESCAN_LIGHT));

		//----------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStart, yCue)), module, SickoSampler2::CUESTART_PARAM));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xEnd, yCue)), module, SickoSampler2::CUEEND_PARAM));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStart, yLoop)), module, SickoSampler2::LOOPSTART_PARAM));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xEnd, yLoop)), module, SickoSampler2::LOOPEND_PARAM));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xRev, yRev)), module, SickoSampler2::REV_PARAM, SickoSampler2::REV_LIGHT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xXfade, yXfade)), module, SickoSampler2::XFADE_PARAM));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xLp, yLp)), module, SickoSampler2::LOOP_PARAM, SickoSampler2::LOOP_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xPng, yPng)), module, SickoSampler2::PINGPONG_PARAM, SickoSampler2::PINGPONG_LIGHT));
		
		//----------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv1, yEnv1)), module, SickoSampler2::ATTACK_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv1+xEnv1Add, yEnv1)), module, SickoSampler2::DECAY_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv1+xEnv1Add*2, yEnv1)), module, SickoSampler2::SUSTAIN_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv1+xEnv1Add*3, yEnv1)), module, SickoSampler2::RELEASE_PARAM));

		//----------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStretch, yStretch)), module, SickoSampler2::STRETCH_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xStretchSiz, yStretchSiz)), module, SickoSampler2::STR_SIZE_PARAM));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xVoct, yVoct)), module, SickoSampler2::VO_INPUT));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xVol, yVol)), module, SickoSampler2::VOL_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInVol, yInVol)), module, SickoSampler2::VOL_INPUT));

		//----------------------------------------------------------------------------------------------------------------------------
		
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut1, yOut1)), module, SickoSampler2::OUT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut2, yOut1)), module, SickoSampler2::OUT_OUTPUT+1));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut1, yOut2)), module, SickoSampler2::EOC_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut2, yOut2)), module, SickoSampler2::EOR_OUTPUT));

	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		SickoSampler2 *module = dynamic_cast<SickoSampler2*>(this->module);
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
	   	SickoSampler2 *module = dynamic_cast<SickoSampler2*>(this->module);
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

		if (module->fileLoaded) {
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("Current Sample:"));
			menu->addChild(createMenuLabel(module->fileDescription));
			menu->addChild(createMenuLabel(" " + module->samplerateDisplay + " - " + std::to_string(module->channels) + "ch"));
			std::string tempDisplay = " ";
			if (module->resampled) {
				tempDisplay += "resampled to " + std::to_string(int(APP->engine->getSampleRate()));
				if (module->toSave)
					tempDisplay += " - ";
			}
			if (module->toSave)
				tempDisplay += "to SAVE";
			if (tempDisplay != " ") {
				menu->addChild(createMenuLabel(tempDisplay));
			}

			menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot();}));

			menu->addChild(new MenuSeparator());
			
			menu->addChild(createMenuItem("Save FULL Sample", "", [=]() {module->menuSaveSample(SAVE_FULL);}));
			menu->addChild(createMenuItem("Save CUE Region", "", [=]() {module->menuSaveSample(SAVE_CUE);}));
			menu->addChild(createMenuItem("Save LOOP Region", "", [=]() {module->menuSaveSample(SAVE_LOOP);}));

			menu->addChild(createBoolPtrMenuItem("Trim Sample after Save", "", &module->trimOnSave));
			menu->addChild(createBoolPtrMenuItem("Save Oversampled", "", &module->saveOversampled));
		
		} else if (module->storedPath != "" && module->fileFound == false) {
			menu->addChild(createMenuLabel("Sample ERROR:"));
			menu->addChild(createMenuLabel(module->fileDescription));
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

		menu->addChild(createBoolPtrMenuItem("Auto Monitor Off", "", &module->autoMonOff));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("EOC pulse from", "", [=](Menu* menu) {
			menu->addChild(createBoolPtrMenuItem("TRG/GATE (stop)", "", &module->eocFromTrg));
			menu->addChild(createBoolPtrMenuItem("CUE END", "", &module->eocFromCueEnd));
			menu->addChild(createBoolPtrMenuItem("CUE START", "", &module->eocFromCueStart));
			menu->addChild(createBoolPtrMenuItem("LOOP END", "", &module->eocFromLoopEnd));
			menu->addChild(createBoolPtrMenuItem("LOOP START", "", &module->eocFromLoopStart));
			menu->addChild(createBoolPtrMenuItem("PING", "", &module->eocFromPing));
			menu->addChild(createBoolPtrMenuItem("PONG", "", &module->eocFromPong));
		}));
		menu->addChild(createBoolPtrMenuItem("Disable NAV Buttons", "", &module->disableNav));		
		menu->addChild(createMenuItem("Reset Cursors", "", [=]() {module->resetCursors();}));
		menu->addChild(createBoolPtrMenuItem("Reset cursors on Load", "", &module->resetCursorsOnLoad));
		menu->addChild(createSubmenuItem("Presets", "", [=](Menu * menu) {
			menu->addChild(createMenuItem("Wavetable", "", [=]() {module->setPreset(0);}));
			menu->addChild(createMenuItem("Triggered Sample with Envelope", "", [=]() {module->setPreset(1);}));
			menu->addChild(createMenuItem("Drum Player", "", [=]() {module->setPreset(2);}));
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Store Sample in Patch", "", &module->sampleInPatch));
#if !defined(METAMODULE)
		menu->addChild(createBoolPtrMenuItem("Unlimited REC (risky)", "", &module->unlimitedRecording));
#endif
	}
};

Model *modelSickoSampler2 = createModel<SickoSampler2, SickoSampler2Widget>("SickoSampler2");
