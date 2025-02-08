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

struct SickoSampler : Module {
	enum ParamIds {
		VOL_PARAM,
		VOLATNV_PARAM,
		TUNE_PARAM,
		TUNEATNV_PARAM,
		CUESTART_PARAM,
		CUEEND_PARAM,
		LOOPSTART_PARAM,
		LOOPEND_PARAM,
		XFADE_PARAM,
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
		REC_PARAM,
		GAIN_PARAM,
		RECFADE_PARAM,
		XTND_SWITCH,
		OVERDUB_SWITCH,
		REC_REL_SWITCH,
		REARM_SWITCH,
		ULE_SWITCH,
		UCE_SWITCH,
		PLAYONREC_SWITCH,
		STOPONREC_SWITCH,
		MONITOR_SWITCH,
		PREVSAMPLE_PARAM,
		NEXTSAMPLE_PARAM,
		TRIGBUT_PARAM,
		STOPBUT_PARAM,
		PHASESCAN_SWITCH,
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
		ENUMS(IN_INPUT,2),
		REC_INPUT,
		RECSTOP_INPUT,
		CLEAR_INPUT,
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
		OVERDUB_LIGHT,
		REC_REL_LIGHT,
		REARM_LIGHT,
		ULE_LIGHT,
		UCE_LIGHT,
		PLAYONREC_LIGHT,
		STOPONREC_LIGHT,
		XTND_LIGHT,
		CLIPPING_LIGHT,
		TRIGBUT_LIGHT,
		STOPBUT_LIGHT,
		PHASESCAN_LIGHT,
		NUM_LIGHTS
	};
  
	unsigned int fileChannels;
	unsigned int channels;
	unsigned int sampleRate;
	drwav_uint64 totalSampleC = 0;
	drwav_uint64 totalSamples = 0;

	const unsigned int minSamplesToLoad = 124;

	vector<float> playBuffer[2][2];
	vector<float> tempBuffer[2];

	vector<double> displayBuff;
	float recCoeffDisplay;
	drwav_uint64 prevRecTotalSampleC;
	int currentDisplay = 0;
	float voctDisplay = 100.f;

	bool fileLoaded = false;

	bool play[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool playPauseToStop[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	double samplePos[16] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	//const int sampleCoeff = 2;
	double currentSpeed = 0.0;
	double distancePos[16] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

	double cueStartPos;
	double cueEndPos;
	double loopStartPos;
	double loopEndPos;

	//float knobCueStartPos;
	//float knobCueEndPos;
	float knobCueStartPos = 0.f;
	float knobCueEndPos = 1.f;
	float prevKnobCueStartPos = -1.f;
	float prevKnobCueEndPos = 2.f;
	
	//float knobLoopStartPos;
	//float knobLoopEndPos;
	float knobLoopStartPos = 0.f;
	float knobLoopEndPos = 1.f;
	float prevKnobLoopStartPos = -1.f;
	float prevKnobLoopEndPos = 2.f;

	float recKnobCueEndPos;
	float recKnobLoopEndPos;

	float tune = 0.f;
	float prevTune = -1.f;

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
	float stopButValue = 0.f;
	float prevTrigValue[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float stopValue[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float prevStopValue[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

	bool loop = false;

	bool trimOnSave = true;
	int antiAlias = 1;
	int polyOuts = POLYPHONIC;
	int polyMaster = POLYPHONIC;
	bool phaseScan = true;
	bool prevPhaseScan = false;
	bool firstLoad = true;
	bool resetCursorsOnLoad = true;
	bool recordRelease = false;
	bool disableNav = false;
	bool saveOversampled = false;

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
	float oneMsSamples = (APP->engine->getSampleRate()) / 1000;  // number of samples in 1ms

	bool eocFromTrg = false;
	bool eocFromStop = false;
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
	int limiter;

	bool reverseStart = false;
	int reversePlaying[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int pingpong;

	float clippingValue = 0;
	float clippingCoeff = 5 / (APP->engine->getSampleRate());	// decrement for 200 ms clipping light (1/0.2(1/0.2)
	bool clipping = false;

	int recButton = 0;
	int prevRecButton = 0;
	float recTrig = 0.f;
	float prevRecTrig = 0.f;
	float recStopTrig = 0.f;
	float prevRecStopTrig = 0.f;
	float clearTrig = 0.f;
	float prevClearTrig = 0.f;
	int recordingState = 0;
	bool newRecording = true;
	bool armRec = false;
	bool recRearm = false;
	bool recRearmAfter = false;
	bool playOnRec = false;
	bool stopOnRec = false;
	bool rearmSetting = false;
	bool startPlayOnRec = false;
	bool startStopOnRec = false;

	double currRecPosition = 0.0;
	double prevRecPosition = 0.0;
	double startRecPosition = 0.0;
	double lastRecordedSample = 0.0;
	int recChannels = 1;
	float currRecValue[2] = {0.f, 0.f};
	float prevRecValue[2] = {0.f, 0.f};
	float avgRecCoeff = 0.f;
	float recValue[2][5] = {{0.f, 0.f, 0.f, 0.f, 0.f},{0.f, 0.f, 0.f, 0.f, 0.f}};
	float recValInterp[2] = {0.f, 0.f};
	double recWeight[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
	double recPosition[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
	int distanceRec = 0;
	bool xtndRec = false;
	bool extendedRec = false;
	int reverseRecording = false;
	bool waitingRecEdge = false;
	bool stopRecNow = false;
	bool firstSampleToRecord = false;
	bool replayNewRecording = false;
	bool sorRecRelException = false;
	
	float recFadeValue = 0.f;
	bool recFadeIn = false;
	bool recFadeOut = false;
	bool overdub = false;
	bool crossRecFade = false;
	bool updateLoopEnd = false;
	bool updateCueEnd = false;
	bool updateAlsoStart = true;

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

	SickoSampler() {
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
		configInput(RELEASE_INPUT,"Release");
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

		//******************************************************************************
		configSwitch(PHASESCAN_SWITCH, 0.f, 1.f, 1.f, "Phase Scan", {"Off", "On"});
		configInput(IN_INPUT,"Left");
		configInput(IN_INPUT+1,"Right");
		configSwitch(REC_PARAM, 0.f, 1.f, 0.f, "REC", {"OFF", "ON"});
		configInput(REC_INPUT,"Rec Toggle");
		configInput(RECSTOP_INPUT,"Rec Stop");
		configParam(GAIN_PARAM, 0.f, 2.0f, 1.0f, "REC Gain", "%", 0, 100);
		configParam(RECFADE_PARAM, 0.f, 1.f, 0.f, "REC Fade in/out", " ms", maxStageTime / minStageTime, minStageTime);
		
		configSwitch(OVERDUB_SWITCH, 0.f, 1.f, 0.f, "OVerDub", {"Off", "On"});
		configSwitch(XTND_SWITCH, 0.f, 1.f, 0.f, "Extend Recording", {"Off", "On"});
		configSwitch(REC_REL_SWITCH, 0.f, 1.f, 0.f, "Record Release", {"Off", "On"});
		configSwitch(REARM_SWITCH, 0.f, 1.f, 0.f, "REC Re-Arm", {"Off", "On"});
		configSwitch(UCE_SWITCH, 0.f, 1.f, 0.f, "Update Cue End", {"Off", "On"});
		configSwitch(ULE_SWITCH, 0.f, 1.f, 0.f, "Update Loop End", {"Off", "On"});
		configSwitch(PLAYONREC_SWITCH, 0.f, 1.f, 0.f, "Play on REC", {"Off", "On"});
		configSwitch(STOPONREC_SWITCH, 0.f, 1.f, 0.f, "Stop on REC", {"Off", "On"});
		
		configSwitch(MONITOR_SWITCH, 0.f, 2.f, 0.f, "Monitor", {"Off", "Rec Only", "Always"});

		configInput(CLEAR_INPUT,"Clear Sample");

		playBuffer[0][0].resize(0);
		playBuffer[0][1].resize(0);
		playBuffer[1][0].resize(0);
		playBuffer[1][1].resize(0);
		tempBuffer[0].resize(0);
		tempBuffer[1].resize(0);

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
		eocFromTrg = false;
		eocFromStop = false;
		eocFromCueEnd = true;
		eocFromCueStart = true;
		eocFromLoopEnd = true;
		eocFromLoopStart = true;
		eocFromPing = true;
		eocFromPong = true;
		updateAlsoStart = true;
		crossRecFade = false;
		firstLoad = true;
		firstSampleToRecord = false;
		resetCursorsOnLoad = true;
		saveOversampled = false;

		startPlayOnRec = false;
		startStopOnRec = false;
		stopRecNow = false;
		disableNav = false;
		sampleInPatch = true;
		unlimitedRecording = false;

		prevKnobCueStartPos = -1.f;
		prevKnobCueEndPos = 2.f;
		prevKnobLoopStartPos = -1.f;
		prevKnobLoopEndPos = 2.f;
		prevTune = -1.f;
		reverseStart = false;
		totalSampleC = 0;
		totalSamples = 0;
		recordingState = 0;
		recButton = 0;
		prevRecButton = 0;
		newRecording = true;
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
		json_object_set_new(rootJ, "EocFromTrg", json_boolean(eocFromTrg));
		json_object_set_new(rootJ, "EocFromStop", json_boolean(eocFromStop));
		json_object_set_new(rootJ, "EocFromCueEnd", json_boolean(eocFromCueEnd));
		json_object_set_new(rootJ, "EocFromCueStart", json_boolean(eocFromCueStart));
		json_object_set_new(rootJ, "EocFromLoopEnd", json_boolean(eocFromLoopEnd));
		json_object_set_new(rootJ, "EocFromLoopStart", json_boolean(eocFromLoopStart));
		json_object_set_new(rootJ, "EocFromPing", json_boolean(eocFromPing));
		json_object_set_new(rootJ, "EocFromPong", json_boolean(eocFromPong));
		json_object_set_new(rootJ, "ResetCursorsOnLoad", json_boolean(resetCursorsOnLoad));
		json_object_set_new(rootJ, "UpdateAlsoStart", json_boolean(updateAlsoStart));
		json_object_set_new(rootJ, "CrossRecFade", json_boolean(crossRecFade));
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
		json_t* resetCursorsOnLoadJ = json_object_get(rootJ, "ResetCursorsOnLoad");
		if (resetCursorsOnLoadJ)
			resetCursorsOnLoad = json_boolean_value(resetCursorsOnLoadJ);
		json_t* updateAlsoStartJ = json_object_get(rootJ, "UpdateAlsoStart");
		if (updateAlsoStartJ)
			updateAlsoStart = json_boolean_value(updateAlsoStartJ);
		json_t* crossRecFadeJ = json_object_get(rootJ, "CrossRecFade");
		if (crossRecFadeJ)
			crossRecFade = json_boolean_value(crossRecFadeJ);
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
		//INFO (" [SickoCV] JSON LOADED");
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
		clippingCoeff = 5 / (APP->engine->getSampleRate());	// decrement for 200 ms clipping light (1/0.2)
		prevXfade = -1.f;

		if (fileLoaded && APP->engine->getSampleRate() != sampleRate/2) {
			/*
			INFO ("[ SickoCV ] resampling %s\n", (to_string(APP->engine->getSampleRate())).c_str());
			INFO ("[ SickoCV ] resampling %s\n", (to_string(sampleRate/2)).c_str());
			*/
			double resampleCoeff;

			fileLoaded = false;

			z1 = 0; z2 = 0; z1r = 0; z2r = 0;

			tempBuffer[0].clear();
			tempBuffer[1].clear();

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

			tempBuffer[LEFT].clear();
			tempBuffer[RIGHT].clear();

			totalSampleC = playBuffer[LEFT][0].size();
			totalSamples = totalSampleC-1;
			sampleRate = APP->engine->getSampleRate() * 2;

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
			for (int i = 0; i < floor(totalSampleC); i = i + floor(totalSampleC/240))
				displayBuff.push_back(playBuffer[0][0][i]);

			resampled = true;
			recSamples = 0;
			fileLoaded = true;

		}
		/*
		INFO ("[ SickoCV ] cueStartPos %s\n", (to_string(knobCueStartPos)).c_str());
		INFO ("[ SickoCV ] cueEndPos %s\n", (to_string(knobCueEndPos)).c_str());
		INFO ("[ SickoCV ] loopStartPos %s\n", (to_string(knobLoopStartPos)).c_str());
		INFO ("[ SickoCV ] loopEndPos %s\n", (to_string(knobLoopEndPos)).c_str());
		INFO ("[ SickoCV ] RESAMPLED");
		*/
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

		char* pathDup = strdup(path.c_str());

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

		fileDisplay = fileDisplay.substr(0, 15);

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

		//INFO (" [SickoCV] FILE SAVED");
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

		tempBuffer[0].clear();
		tempBuffer[1].clear();

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
				totalSamples = totalSampleC-1;
				drwav_free(pSampleData);

				for (unsigned int i = 1; i < totalSamples; i = i+2) {
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
				totalSamples = totalSampleC-1;
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
				drwav_uint64 tempSamples = tempSampleC-1;
				

				for (unsigned int i = 1; i < tempSamples; i = i+2) {
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

				tempBuffer[LEFT].clear();
				tempBuffer[RIGHT].clear();
				// ***************************************************************************

				totalSampleC = playBuffer[LEFT][0].size();
				totalSamples = totalSampleC-1;

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
			for (int i = 0; i < floor(totalSampleC); i = i + floor(totalSampleC/240))
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

			fileDisplay = fileDisplay.substr(0, 15);
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

				//INFO ("[ SickoCV ] Loaded from FILE");
			
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
				/*
				INFO ("[ SickoCV ] cueStartPos %s\n", (to_string(knobCueStartPos)).c_str());
				INFO ("[ SickoCV ] cueEndPos %s\n", (to_string(knobCueEndPos)).c_str());
				INFO ("[ SickoCV ] loopStartPos %s\n", (to_string(knobLoopStartPos)).c_str());
				INFO ("[ SickoCV ] loopEndPos %s\n", (to_string(knobLoopEndPos)).c_str());
				INFO ("[ SickoCV ] Loaded from Patch");
				*/
			}

			newRecording = false;
			recSeconds = 0;
			recMinutes = 0;

			firstLoad = false;

			fileLoaded = true;
			channels = fileChannels;

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
			//INFO ("[ SickoCV ] FILE NOT FOUND");
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
		totalSampleC = 0;
		totalSamples = 0;
		newRecording = true;
		resampled = false;
		toSave = false;
		infoToSave= "";
		for (int i=0; i < 16; i++)
			fadingValue[i] = 1;
		
		//INFO ("[ SickoCV ] Slot Cleared");

	}

	void resetCursors() {
		params[CUESTART_PARAM].setValue(0);
		params[CUEEND_PARAM].setValue(1);
		params[LOOPSTART_PARAM].setValue(0);
		params[LOOPEND_PARAM].setValue(1);
		//INFO ("[ SickoCV ] Cursors resetted");
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

		overdub = params[OVERDUB_SWITCH].getValue();
		lights[OVERDUB_LIGHT].setBrightness(overdub);

		xtndRec = params[XTND_SWITCH].getValue();
		lights[XTND_LIGHT].setBrightness(xtndRec);

		rearmSetting = params[REARM_SWITCH].getValue();
		lights[REARM_LIGHT].setBrightness(rearmSetting);

		recordRelease = params[REC_REL_SWITCH].getValue();
		lights[REC_REL_LIGHT].setBrightness(recordRelease);

		updateCueEnd = params[UCE_SWITCH].getValue();
		lights[UCE_LIGHT].setBrightness(updateCueEnd);

		updateLoopEnd = params[ULE_SWITCH].getValue();
		lights[ULE_LIGHT].setBrightness(updateLoopEnd);

		playOnRec = params[PLAYONREC_SWITCH].getValue();
		lights[PLAYONREC_LIGHT].setBrightness(playOnRec);

		stopOnRec = params[STOPONREC_SWITCH].getValue();
		lights[STOPONREC_LIGHT].setBrightness(stopOnRec);

		if (recButton == 0)
			chan = std::max(1, inputs[VO_INPUT].getChannels());
		else
			chan = 1;

		sumOutput = 0;
		sumOutputR = 0;

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

		trigButValue = params[TRIGBUT_PARAM].getValue();
		lights[TRIGBUT_LIGHT].setBrightness(trigButValue);

		stopButValue = params[STOPBUT_PARAM].getValue();
		lights[STOPBUT_LIGHT]. setBrightness(stopButValue);

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

			clearTrig = inputs[CLEAR_INPUT].getVoltage();
			if (clearTrig >= 1 && prevClearTrig < 1)
				clearSlot();
			prevClearTrig = clearTrig;

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

			sustainValue = params[SUSTAIN_PARAM].getValue() + (inputs[SUSTAIN_INPUT].getVoltage() * params[SUSTAINATNV_PARAM].getValue() * 0.1);
			if (sustainValue > 1)
				sustainValue = 1;
			else if (sustainValue < 0)
				sustainValue = 0;

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
								
								if (recButton)
									armRec = true;

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
								attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
								if (attackValue > maxAdsrTime) {
									attackValue = maxAdsrTime;
								} else if (attackValue < minAdsrTime) {
									attackValue = minAdsrTime;
								}
								stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);
							} else {
								if (stage[c] == RELEASE_STAGE) {
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
							if (play[c]) {
								if (recordingState == 1 && !recordRelease) {
									recButton = 0;
									params[REC_PARAM].setValue(0);
									if (rearmSetting) {
										recRearm = false;
										recRearmAfter = true;
									}
								}
								if (stage[c] != RELEASE_STAGE) {
									stage[c] = RELEASE_STAGE;
									releaseNew[c] = true;
									releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
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

						if ((trigValue[c] >= 1 && prevTrigValue[c] < 1) || startPlayOnRec || startStopOnRec || replayNewRecording) {
						
							switch (trigType) {
								case START_STOP:									// trig type: Start/Stop

									if (play[c]) {
																						
										if (!inputs[STOP_INPUT].isConnected() || (stopOnRec && !inputs[RECSTOP_INPUT].isConnected())) {

											if (stage[c] != RELEASE_STAGE) {
												if (recordingState == 1 && !recordRelease) {
													recButton = 0;
													params[REC_PARAM].setValue(0);
													if (rearmSetting && !playOnRec) {
														recRearm = false;
														recRearmAfter = true;
													}
													
												}
												startStopOnRec = false;

												stage[c] = RELEASE_STAGE;
												releaseNew[c] = true;
												releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
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

										replayNewRecording = false;
										
										if (recButton) {
											armRec = true;
											startPlayOnRec = false;
										}

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
									if (!play[c]) {		// ******* START PLAYBACK
										
										play[c] = true;
										
										replayNewRecording = false;

										if (recButton) {
											armRec = true;
											startPlayOnRec = false;
										}
										
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
										attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
										if (attackValue > maxAdsrTime) {
											attackValue = maxAdsrTime;
										} else if (attackValue < minAdsrTime) {
											attackValue = minAdsrTime;
										}
										stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);

									} else {	// ******* RESTART PLAYBACK
										if (!startStopOnRec) {		// if startStopOnRec it must be handled by STOP MANAGEMENT
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
											attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
											if (attackValue > maxAdsrTime) {
												attackValue = maxAdsrTime;
											} else if (attackValue < minAdsrTime) {
												attackValue = minAdsrTime;
											}
											stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);

											if (recordingState) {
												if (reversePlaying[recOutChan] == FORWARD) {

													startRecPosition = floor(samplePos[recOutChan]) - distancePos[recOutChan];
													prevRecPosition = startRecPosition - distancePos[recOutChan];
													currRecPosition = startRecPosition;
													
												} else {

													startRecPosition = ceil(samplePos[recOutChan]) + distancePos[recOutChan] + 1;
													prevRecPosition = startRecPosition + distancePos[recOutChan];
													currRecPosition = startRecPosition;

												}

												if (prevRecPosition < 0 || prevRecPosition > totalSamples)
													firstSampleToRecord = true;
											}
										} 

									}
								break;
								
								case PLAY_PAUSE:									// trig type: PLAY/PAUSE
									
									if (play[c]) {		// STOP PLAYING

										if (stage[c] != RELEASE_STAGE) {
											
											if (recordingState && !recordRelease) {
												params[REC_PARAM].setValue(0);
												if (rearmSetting && !playOnRec) {
													recRearm = false;
													recRearmAfter = true;
												}
												prevRecButton = 0;
											}
											
											stage[c] = RELEASE_STAGE;
											releaseNew[c] = true;
											releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
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
											attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
											if (attackValue > maxAdsrTime) {
												attackValue = maxAdsrTime;
											} else if (attackValue < minAdsrTime) {
												attackValue = minAdsrTime;
											}
											stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);
										}
									} else {				// START PLAYING
										play[c] = true;
										playPauseToStop[c] = false;

										replayNewRecording = false;
										
										if (recButton) {
											armRec = true;
											startPlayOnRec = false;
										}

										stage[c] = ATTACK_STAGE;
										attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
										if (attackValue > maxAdsrTime) {
											attackValue = maxAdsrTime;
										} else if (attackValue < minAdsrTime) {
											attackValue = minAdsrTime;
										}
										stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);

										if (reverseStart == FORWARD) {
											reversePlaying[c] = FORWARD;
											if (samplePos[c] < cueStartPos || samplePos[c] > cueEndPos) {
												samplePos[c] = floor(cueStartPos)+1;
												currSampleWeight[c] = sampleCoeff;
											}
										} else {
											reversePlaying[c] = REVERSE;
											if (samplePos[c] < cueStartPos || samplePos[c] > cueEndPos) {
												samplePos[c] = floor(cueEndPos)-1;
												currSampleWeight[c] = sampleCoeff;
											}
										}
									}
								break;
							}
						}
						prevTrigValue[c] = trigValue[c];

						// ************************************************* STOP INPUT MANAGEMENT
						
						if (stopButValue || startStopOnRec) {	// **** startStopRec here handles stopRecButton when in StartRestart mode
							stopValue[c] = 1;
							startStopOnRec = false;
						} else
							stopValue[c] = inputs[STOP_INPUT].getVoltage(c);

						if (stopValue[c] >= 1 && prevStopValue[c] < 1) {

							if (trigMode == TRIG_MODE && trigType == PLAY_PAUSE) {

								if (play[c]) {
									playPauseToStop[c] = true;
								} else {
									if (reverseStart == FORWARD) {
										samplePos[c] = floor(cueStartPos)+1;
										currSampleWeight[c] = sampleCoeff;
									} else {
										samplePos[c] = floor(cueEndPos)-1;
										currSampleWeight[c] = sampleCoeff;
									}
								}
							}

							if (play[c]) {

								if (stage[c] != RELEASE_STAGE) {

									if (recordingState == 1 && !recordRelease) {
										recButton = 0;
										params[REC_PARAM].setValue(0);
										if (rearmSetting && !playOnRec) {
											recRearm = false;
											recRearmAfter = true;
										}
										startStopOnRec = false;
										prevRecButton = 0;			// this avoids to continue playing if SOR button is ON
									}

									stage[c] = RELEASE_STAGE;
									releaseNew[c] = true;
									releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
									if (releaseValue > maxAdsrTime) {
										releaseValue = maxAdsrTime;
									} else 	if (releaseValue < minAdsrTime) {
										releaseValue = minAdsrTime;
									}

									if (stageLevel[c] != 0)
										stageCoeff[c] = stageLevel[c] / (args.sampleRate * releaseValue);
									else
										stageCoeff[c] = 1;

								}

								if (eocFromStop) {
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
						prevStopValue[c] = stopValue[c];

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
						distancePos[c] = currentSpeed * sampleCoeff * speedVoct[c];
					} else
						distancePos[c] = currentSpeed * sampleCoeff;

					//if (play[c] && voct[c] < voctDisplay) {
					if (voct[c] < voctDisplay) {
						currentDisplay = c;
						voctDisplay = voct[c];
					}

					switch (reversePlaying[c]) {
						case FORWARD:		// ********************************************************************************************  FORWARD PLAYBACK   ***********

							if (recordingState == 1 && loop && samplePos[c] > floor(loopEndPos - distancePos[c]) && !xtndRec) {
								waitingRecEdge = true;

								// below it's the same as not recording (code in the next "if")
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

							} else if (recordingState == 0 && loop && samplePos[c] > floor(loopEndPos - (fadeSamples * distancePos[c]))) {		// *** REACHED END OF LOOP ***

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
								if (recRearmAfter) {
									recButton = 1;
									params[REC_PARAM].setValue(1);
								} else if (recordingState == 1 && !xtndRec) {
									waitingRecEdge = true;
									if (rearmSetting && !playOnRec) {
										recRearmAfter = true;
									}
								}
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
								if (recordingState == 1) {
									
									if (!xtndRec) {
										waitingRecEdge = true;
										if (stage[c] != RELEASE_STAGE) {
											stage[c] = RELEASE_STAGE;
											releaseNew[c] = true;

											if (!recordRelease) {
												recButton = 0;
												params[REC_PARAM].setValue(0);

												if (rearmSetting && !playOnRec) {
													recRearm = false;
													recRearmAfter = true;
												}
												startStopOnRec = false;
											}


											releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
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
								} else {
									if (stage[c] != RELEASE_STAGE) {
										stage[c] = RELEASE_STAGE;
										releaseNew[c] = true;
										releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
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
							} 
						break;

						case REVERSE:		// ********************************************************************************************  REVERSE PLAYBACK   ***********

							if (recordingState == 1  && loop && samplePos[c] < floor(loopStartPos + distancePos[c]) && !xtndRec) {
								waitingRecEdge = true;

								// below it's the same as not recording (code in the next "if")
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

							} else if (recordingState == 0 && loop && samplePos[c] < floor(loopStartPos + (fadeSamples * distancePos[c]))) {	// *** REACHED BEGIN OF LOOP ***

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
								if (recordingState == 1)
									waitingRecEdge = true;

								fadingType[c] = FADE_OUT;
								fadingValue[c] = 1.f;
								fadedPosition[c] = samplePos[c];
								if (fadeSamples)
									fadeCoeff = 1 / convertCVToSeconds(xFade) / args.sampleRate;
								else
									fadeCoeff = 1;

							} else if (floor(samplePos[c]) < 0) {				// *** REACHED START OF SAMPLE ***
								if (recRearmAfter) {
									recButton = 1;
									params[REC_PARAM].setValue(1);
								} else if (recordingState == 1) {
									waitingRecEdge = true;
									if (rearmSetting && !playOnRec)
										recRearmAfter = true;
								}
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
								if (recordingState == 1) {

									if (!xtndRec) {
										waitingRecEdge = true;
										if (stage[c] != RELEASE_STAGE) {
											stage[c] = RELEASE_STAGE;
											releaseNew[c] = true;
											
											if (!recordRelease) {
												recButton = 0;
												params[REC_PARAM].setValue(0);
												if (rearmSetting && !playOnRec) {
													recRearm = false;
													recRearmAfter = true;
												}
												startStopOnRec = false;
											}

											releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
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
								} else {
									if (stage[c] != RELEASE_STAGE) {
										stage[c] = RELEASE_STAGE;
										releaseNew[c] = true;
										releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
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
							}
						
						break;
					}

					if (play[c]) {									// it's false only if end of sample has reached, see above

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
								currentOutput = playBuffer[LEFT][antiAlias][floor(samplePos[c])];
								if (channels == 2)
									currentOutputR = playBuffer[RIGHT][antiAlias][floor(samplePos[c])];
							}
						}

						if (reversePlaying[c])
							samplePos[c] -= distancePos[c];
						else
							samplePos[c] += distancePos[c];

						currSampleWeight[c] = samplePos[c] - floor(samplePos[c]);

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
								if (stageLevel[c] < 0) {	// Il release has ended
									stageLevel[c] = 0;

									if (recordingState == 1 && recordRelease) {
										recButton = 0;
										params[REC_PARAM].setValue(0);
										prevRecButton = 0;			// this avoids to continue playing if SOR button is ON

										if (rearmSetting && ((trigMode == TRIG_MODE && !playOnRec) || (trigMode == GATE_MODE)) )
											recRearm = true;

										startStopOnRec = false;
									}
									sorRecRelException = false;
									
									if (!recordingState) {		// If not recording

										stage[c] = STOP_STAGE;
										play[c] = false;

										if (recRearm) {
											recRearm = false;
											recRearmAfter = true;
										}
									} else {					// if recording

										if (recFadeValue <= 0) {	// if release has ended and recfade out
											stage[c] = STOP_STAGE;
											play[c] = false;
											releaseNew[c] = true;
										}
									}
									
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

										if (playPauseToStop[c] && !recordingState) {
											playPauseToStop[c] = false;
											if (reversePlaying[c] == FORWARD) {
												samplePos[c] = floor(cueStartPos)+1;
												currSampleWeight[c] = sampleCoeff;
											} else {
												samplePos[c] = floor(cueEndPos)-1;
												currSampleWeight[c] = sampleCoeff;
											}
										}
										if (recRearmAfter && !recordingState) {
											recRearmAfter = false;
											recRearm = false;
											params[REC_PARAM].setValue(1);
											recButton = 1;
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
						// *** HARD CLIP ***
						if (limiter) {
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
					// *** HARD CLIP ***
					if (limiter) {
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

		if (!newRecording) {
			recButton = params[REC_PARAM].getValue();
			recTrig = inputs[REC_INPUT].getVoltage();
			recStopTrig = inputs[RECSTOP_INPUT].getVoltage();

			if (recTrig > 1 && prevRecTrig <= 0 && !recButton) {
				recButton = 1;
				params[REC_PARAM].setValue(1);
			} else if (!inputs[RECSTOP_INPUT].isConnected() && recTrig > 1 && prevRecTrig <= 0 && recButton) {
				recButton = 0;
				params[REC_PARAM].setValue(0);
			} else if (recStopTrig > 1 && prevRecStopTrig <= 0 && recButton) {
				recButton = 0;
				params[REC_PARAM].setValue(0);
				if (stopOnRec)
					startStopOnRec = true;
			}

			if (play[recOutChan] && recordingState == 0 && prevRecButton == 0 && recButton == 1)
				armRec = true;

			if (!play[recOutChan] && playOnRec && prevRecButton == 0 && recButton == 1 && trigMode == TRIG_MODE)
				startPlayOnRec = true;

			if (play[recOutChan] && recordingState && stopOnRec && prevRecButton && !recButton && !sorRecRelException) {
				startStopOnRec = true;
				if (recordRelease)
					sorRecRelException = true;
			}

			lights[REC_LIGHT].setBrightness(recButton);
			prevRecButton = recButton;
			prevRecTrig = recTrig;
			prevRecStopTrig = recStopTrig;

		} else {	// ********* NEW RECORDING **** START STOP COMMANDS

			recButton = params[REC_PARAM].getValue();
			recTrig = inputs[REC_INPUT].getVoltage();
			recStopTrig = inputs[RECSTOP_INPUT].getVoltage();

			if (recTrig > 1 && prevRecTrig <= 0 && !recButton) {
				recButton = 1;
				params[REC_PARAM].setValue(1);
			} else if (!inputs[RECSTOP_INPUT].isConnected() && recTrig > 1 && prevRecTrig <= 0 && recButton) {
				recButton = 0;
				params[REC_PARAM].setValue(0);
			} else if (recStopTrig > 1 && prevRecStopTrig <= 0 && recButton) {
				recButton = 0;
				params[REC_PARAM].setValue(0);
				if (stopOnRec)
					startStopOnRec = true;
			}

			if (recordingState == 0 && playOnRec && recButton == 1 && trigMode == TRIG_MODE)
				startPlayOnRec = true;

			if (stopOnRec && prevRecButton && !recButton)
				startStopOnRec = true;

			lights[REC_LIGHT].setBrightness(recButton);
			prevRecButton = recButton;
			prevRecTrig = recTrig;
			prevRecStopTrig = recStopTrig;

			masterLevel[recOutChan] = params[VOL_PARAM].getValue() + (inputs[VOL_INPUT].getVoltage(recOutChan) * params[VOLATNV_PARAM].getValue() * 0.2);
			if (recButton || startStopOnRec) {

				trigMode = params[TRIGGATEMODE_SWITCH].getValue();
				trigType = params[TRIGMODE_SWITCH].getValue();
				
				trigValue[recOutChan] = inputs[TRIG_INPUT].getVoltage(recOutChan);

				if (trigButValue || (trigMode == TRIG_MODE && (startPlayOnRec || startStopOnRec || (trigValue[recOutChan] >= 1 && prevTrigValue[recOutChan] < 1 ) ))) {
					trigValue[recOutChan] = 1;
					if (startPlayOnRec)
						startPlayOnRec = false;
					if (startStopOnRec)
						startStopOnRec = false;
				}
				
				switch (trigMode) {
					case GATE_MODE:												// ***** GATE MODE *****
						if (trigValue[recOutChan] >= 1 && recordingState == 0) {
							armRec = true;
						} else if (trigValue[recOutChan] < 1 && recordingState == 1) {
							recButton = 0;
							params[REC_PARAM].setValue(0);
							if (rearmSetting && !playOnRec)
								recRearm = true;
						}
					break;
					case TRIG_MODE:
						if (stopButValue)
							stopValue[recOutChan] = 1;
						else
							stopValue[recOutChan] = inputs[STOP_INPUT].getVoltage(recOutChan);

						if (stopValue[recOutChan] >= 1 && prevStopValue[recOutChan] < 1 && recordingState == 1) {
							recButton = 0;
							params[REC_PARAM].setValue(0);
						}
						prevStopValue[recOutChan] = stopValue[recOutChan];

						if (trigValue[recOutChan] >= 1 && prevTrigValue[recOutChan] < 1) {
							
							if (recordingState == 0) {
								armRec = true;
							} else {
								recButton = 0;
								params[REC_PARAM].setValue(0);
							}
						}
					break;
				}
				prevTrigValue[recOutChan] = trigValue[recOutChan];
			}
		}

		if (recButton && recordingState == 0 && armRec) {

			if (inputs[IN_INPUT].isConnected()){

				armRec = false;
				recRearm = false;
				recRearmAfter = false;
				waitingRecEdge = false;
				firstSampleToRecord = false;
				lastRecordedSample = 0.0;
				replayNewRecording = false;

				if (newRecording) {								// ******* NEW RECORDING ******

					z1 = 0; z2 = 0; z1r = 0; z2r = 0;

					recordingState = 1;
					recSamples = 0;
					extendedRec = false;
					sorRecRelException = false;

					reverseRecording = FORWARD;

					if (inputs[IN_INPUT+1].isConnected())
						recChannels = 2;
					else
						recChannels = 1;

					currentRecordingLimit = recordingLimit / recChannels;

					fileChannels = recChannels;

					sampleRate = args.sampleRate * 2;
					calcBiquadLpf(20000.0, sampleRate, 1);

					playBuffer[LEFT][0].clear();
					playBuffer[LEFT][1].clear();
					if (recChannels == 2) {
						playBuffer[RIGHT][0].clear();
						playBuffer[RIGHT][1].clear();
					}

					prevRecPosition = 0;
					currRecPosition = floor(currentSpeed * sampleCoeff);
					distanceRec = currRecPosition;

					for (int i=0; i <= distanceRec+2; i++) {
						playBuffer[LEFT][0].push_back(0);
						playBuffer[LEFT][1].push_back(0);
						if (recChannels == 2) {
							playBuffer[RIGHT][0].push_back(0);
							playBuffer[RIGHT][1].push_back(0);
						}
					}

					totalSamples = distanceRec+2;
					totalSampleC = totalSamples+1;

					// set position knob to 0 and 1
					knobCueStartPos = 0;
					knobLoopStartPos = 0;
					knobCueEndPos = 1;
					knobLoopEndPos = 1;

					params[CUESTART_PARAM].setValue(0);
					params[CUEEND_PARAM].setValue(1);
					params[LOOPSTART_PARAM].setValue(0);
					params[LOOPEND_PARAM].setValue(1);

					recKnobCueEndPos = 1;
					recKnobLoopEndPos = 1;

					if (params[RECFADE_PARAM].getValue() > 0) {
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

					startRecPosition = 0;
					recFadeOut = false;
					
				} else {								// ************* IF ALREADY RECORDED

					z1 = 0; z2 = 0; z1r = 0; z2r = 0;

					recordingState = 1;
					recSamples = 0;
					extendedRec = false;
					sorRecRelException = false;

					currRecValue[LEFT] = 0;
					currRecValue[RIGHT] = 0;
					prevRecValue[LEFT] = 0;
					prevRecValue[RIGHT] = 0;

					if (play[recOutChan]) {

						recChannels = channels;
						currentRecordingLimit = recordingLimit / recChannels;

						recKnobCueEndPos = knobCueEndPos;
						recKnobLoopEndPos = knobLoopEndPos;

						if (reversePlaying[recOutChan] == FORWARD) {
							reverseRecording = FORWARD;

							startRecPosition = floor(samplePos[recOutChan]) - distancePos[recOutChan];
							prevRecPosition = startRecPosition - distancePos[recOutChan];
							currRecPosition = startRecPosition;
							
						} else {
							reverseRecording = REVERSE;

							startRecPosition = ceil(samplePos[recOutChan]) + distancePos[recOutChan] + 1;
							prevRecPosition = startRecPosition + distancePos[recOutChan];
							currRecPosition = startRecPosition;

						}

						if (prevRecPosition < 0 || prevRecPosition > totalSamples)
							firstSampleToRecord = true;

						prevRecTotalSampleC = totalSampleC;

						if (params[RECFADE_PARAM].getValue() > 0) {
							recFadeValue = 0;
							recFadeIn = true;
						} else {
							recFadeIn = false;
							recFadeValue = 1;
						}

						recFadeOut = false;
					} 
				}

				if (monitorSwitch == 1 && params[RECFADE_PARAM].getValue() <= 0) {
					monitorFadeValue = 0;
					monitorFade = true;
				}
				
			} else {				// **** if rec is pressed or triggered and no left cable is plugged, do nothing
				armRec = false;
				params[REC_PARAM].setValue(0);
				recButton = 0;
			}

		} else if (recButton == 0 && recordingState == 1 && recFadeOut == false && !sorRecRelException && !startStopOnRec)  {			// ********** S T O P   R E C O R D I N G ****************

			if (recFadeValue > 0) {
				recFadeOut = true;

				if (params[RECFADE_PARAM].getValue() <= 0) {
					recFadeValue = 0;
					
					if (monitorSwitch == 1) {
						monitorFadeValue = 0;
						monitorFade = true;
					}

				}
			}

			if (recFadeValue <= 0) {
				recordingState = 0;

				if (reverseRecording == FORWARD) {
					if (currRecPosition >= totalSamples) { 	// <- lasts sample interpolation if it's in forward recording and it's at the end of the sample
						int directionCoeff = 1;
						int fillRecDistance = totalSampleC - lastRecordedSample;
						prevRecPosition = lastRecordedSample+1;
						if (fillRecDistance > 0) {
							int rc = 0;
							currRecValue[rc] = playBuffer[rc][0][lastRecordedSample];
							avgRecCoeff = currRecValue[rc] / fillRecDistance;
							for (int i=0; i < fillRecDistance; i++) {
								if (overdub)
									playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += currRecValue[rc] - (avgRecCoeff * i);
								else if (recFadeIn || recFadeOut)
									playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
								else
									playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i);
								
								playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector
							}
							// ************ RIGHT CHANNEL ******************
							if (recChannels == 2) {
								rc = 1;
								currRecValue[rc] = playBuffer[rc][0][lastRecordedSample];
								avgRecCoeff = currRecValue[rc] / fillRecDistance;
								for (int i=0; i < fillRecDistance; i++) {
									if (overdub)
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += currRecValue[rc] - (avgRecCoeff * i);
									else if (recFadeIn || recFadeOut)
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
									else
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i);
									
									playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf2(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector
								}
							}
						}
						// end filling
					}
				} else {

					if (currRecPosition < 0) { 		// <- firsts sample interpolation if it's in reverse recording and it's at the beginning of the sample
						int directionCoeff = -1;
						int fillRecDistance = lastRecordedSample;
						prevRecPosition = lastRecordedSample-1;

						if (fillRecDistance > 0) {
							int rc = 0;
							avgRecCoeff = currRecValue[rc] / fillRecDistance;
							for (int i=0; i < fillRecDistance; i++) {
								if (overdub)
									playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += currRecValue[rc] - (avgRecCoeff * i);
								else if (recFadeIn || recFadeOut)
									playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
								else
									playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i);
								
								playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector
							}
							// ************ RIGHT CHANNEL ******************
							if (recChannels == 2) {
								rc = 1;
								avgRecCoeff = currRecValue[rc] / fillRecDistance;
								for (int i=0; i < fillRecDistance; i++) {
									if (overdub)
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += currRecValue[rc] - (avgRecCoeff * i);
									else if (recFadeIn || recFadeOut)
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
									else
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i);
									
									playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf2(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector
								}
							}
						}
						// end filling
					}
				}
				
				if (extendedRec) {
					totalSamples = lastRecordedSample;
					totalSampleC = totalSamples+1;
					playBuffer[LEFT][0].resize(totalSampleC);
					playBuffer[LEFT][1].resize(totalSampleC);
					if (recChannels == 2) {
						playBuffer[RIGHT][0].resize(totalSampleC);
						playBuffer[RIGHT][1].resize(totalSampleC);
					}

					if (rearmSetting && !playOnRec) {
						recRearm = false;
						recRearmAfter = true;
						play[recOutChan] = false;
					}

					if (loop) {						// this avoids to play release stage at the beginning of the sample when recoring in gate mode
						play[recOutChan] = false;
						stage[recOutChan] = STOP_STAGE;
						stageLevel[recOutChan] = 0;
					}
				} 

				if (newRecording) {
					newRecording = false;
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

					if (loop)
						replayNewRecording = true;

					if (rearmSetting && !playOnRec) {
						recRearm = true;
						recRearmAfter = false;
					}
					
				} else {

					if (updateCueEnd) {
						float tempKnob;
						if (reverseRecording == FORWARD) {
							cueEndPos = lastRecordedSample;
							tempKnob = cueEndPos/totalSampleC;
							params[CUEEND_PARAM].setValue(tempKnob);
							knobCueEndPos = tempKnob;
							if (updateAlsoStart) {
								cueStartPos = startRecPosition;
								tempKnob = cueStartPos/totalSampleC;
								params[CUESTART_PARAM].setValue(tempKnob);
								knobCueStartPos = tempKnob;
							}

						} else {
							cueStartPos = lastRecordedSample;
							tempKnob = cueStartPos/totalSampleC;
							params[CUESTART_PARAM].setValue(tempKnob);
							knobCueStartPos = tempKnob;
							if (updateAlsoStart) {
								cueEndPos = startRecPosition;
								tempKnob = cueEndPos/totalSampleC;
								params[CUEEND_PARAM].setValue(tempKnob);
								knobCueEndPos = tempKnob;
							}
						}
					}

					if (updateLoopEnd) {
						float tempKnob;
						if (reverseRecording == FORWARD) {
							loopEndPos = lastRecordedSample;
							tempKnob = loopEndPos/totalSampleC;
							params[LOOPEND_PARAM].setValue(tempKnob);
							knobLoopEndPos = tempKnob;
							if (updateAlsoStart) {
								loopStartPos = startRecPosition;
								tempKnob = loopStartPos/totalSampleC;
								params[LOOPSTART_PARAM].setValue(tempKnob);
								knobLoopStartPos = tempKnob;
							}
						} else {
							loopStartPos = lastRecordedSample;
							tempKnob = loopStartPos/totalSampleC;
							params[LOOPSTART_PARAM].setValue(tempKnob);
							knobLoopStartPos = tempKnob;
							if (updateAlsoStart) {
								loopEndPos = startRecPosition;
								tempKnob = loopEndPos/totalSampleC;
								params[LOOPEND_PARAM].setValue(tempKnob);
								knobLoopEndPos = tempKnob;
							}
						}
					}
				}

				if (phaseScan) {
					prevKnobCueStartPos = -1.f;
					prevKnobCueEndPos = 2.f;
					prevKnobLoopStartPos = -1.f;
					prevKnobLoopEndPos = 2.f;
				}
				
				vector<double>().swap(displayBuff);
				for (int i = 0; i < floor(totalSampleC); i = i + floor(totalSampleC/240))
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

				if (recRearm) {					// *** this rearms REC BUTTON in PLAY/PAUSE Trig Mode
					recRearm = false;
					
					if (!playOnRec) {
						params[REC_PARAM].setValue(1);
						recButton = 1;
					}

				}

				if (recRearmAfter && !play[recOutChan]) {
					recRearmAfter = false;
					recRearm = false;
					params[REC_PARAM].setValue(1);
					recButton = 1;
				}

				if (playPauseToStop[recOutChan] && !play[recOutChan]) {
					playPauseToStop[recOutChan] = false;
					if (reversePlaying[recOutChan] == FORWARD) {
						samplePos[recOutChan] = floor(cueStartPos)+1;
						currSampleWeight[recOutChan] = sampleCoeff;
					} else {
						samplePos[recOutChan] = floor(cueEndPos)-1;
						currSampleWeight[recOutChan] = sampleCoeff;
					}
				}

				startPlayOnRec = false;	// these two lines avoid restart playing when switching from gate to trigger mode
				startStopOnRec = false;

			}
		}
		
		if (recordingState) {

			// ******************************************************************************* R E C S A M P L E    C H E C K   S E C T I O N *****************

			if (newRecording) {														// ***********************  N E W    R E C O R D I N G   ****************
				if (currRecPosition > totalSamples) {
					extendedRec = true;
					for (int i=1; i <= distanceRec; i++) {
						playBuffer[LEFT][0].push_back(0);
						playBuffer[LEFT][1].push_back(0);
						if (recChannels == 2) {
							playBuffer[RIGHT][0].push_back(0);
							playBuffer[RIGHT][1].push_back(0);
						}
					}

					totalSampleC += distanceRec;
					totalSamples += distanceRec;

					if (distanceRec == 0) {					// se la distance is 0, adds another sample
						playBuffer[LEFT][0].push_back(0);
						playBuffer[LEFT][1].push_back(0);
						if (recChannels == 2) {
							playBuffer[RIGHT][0].push_back(0);
							playBuffer[RIGHT][1].push_back(0);
						}
						totalSampleC++;
						totalSamples++;
					}
				}
			} else {															// ***********************  2 N D    R E C O R D I N G   ****************

				if (xtndRec) {	// ********************************************************************** if EXTEND REC BUTTON IS ON

					if (samplePos[recOutChan] > totalSamples && reverseRecording == FORWARD) {	// ********** FORWARD  reached LAST SAMPLE
					
						extendedRec = true;
						
						for (int i=1; i <= distanceRec; i++) {
							playBuffer[LEFT][0].push_back(0);
							playBuffer[LEFT][1].push_back(0);
							if (recChannels == 2) {
								playBuffer[RIGHT][0].push_back(0);
								playBuffer[RIGHT][1].push_back(0);
							}
						}

						totalSampleC += distanceRec;
						totalSamples += distanceRec;

						if (distanceRec % 2 != 0 || distanceRec == 0) {				// if the distance is odd, add another sample
							playBuffer[LEFT][0].push_back(0);
							playBuffer[LEFT][1].push_back(0);
							if (recChannels == 2) {
								playBuffer[RIGHT][0].push_back(0);
								playBuffer[RIGHT][1].push_back(0);
							}
							totalSampleC++;
							totalSamples++;
						}

						// update cursors position
						if (!newRecording) {
							float tempKnob = cueStartPos / totalSampleC;
							params[CUESTART_PARAM].setValue(tempKnob);
							prevKnobCueStartPos = tempKnob;
							tempKnob = loopStartPos / totalSampleC;
							params[LOOPSTART_PARAM].setValue(tempKnob);
							prevKnobLoopStartPos = tempKnob;

							tempKnob = cueEndPos / totalSampleC;
							params[CUEEND_PARAM].setValue(tempKnob);
							prevKnobCueEndPos = tempKnob;
							tempKnob = loopEndPos / totalSampleC;
							params[LOOPEND_PARAM].setValue(tempKnob);
							prevKnobLoopEndPos = tempKnob;
						}
					} else 	if (samplePos[recOutChan] < 0 && reverseRecording == REVERSE && waitingRecEdge) {	// ***************** REVERSE   reached FIRST SAMPLE
						// *********************************** TO DO (???): calculate last samples to record
						params[REC_PARAM].setValue(0);
						recButton = 0;
						stopRecNow = true;
					}

				} else {	//  *************************************************************************************    if EXTEND REC BUTTON IS OFF

					if (waitingRecEdge) {

						if (reverseRecording == FORWARD) {

							if (loop && currRecPosition > loopEndPos) {	// ********* LOOP FORWARD  reached LOOP END
								waitingRecEdge = false;
								// filling last samples before loop end
								int directionCoeff = 1;
								int fillRecDistance = ceil(loopEndPos) - floor(prevRecPosition);
								if (fillRecDistance > 0) {
									int rc = 0;
									avgRecCoeff = currRecValue[rc] / fillRecDistance;
									for (int i=0; i < fillRecDistance; i++) {
										if (overdub)
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += currRecValue[rc] - (avgRecCoeff * i);
										else if (recFadeIn || recFadeOut)
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
										else
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i);
										
										playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector
									}
									// ************ RIGHT CHANNEL ******************
									if (recChannels == 2) {
										rc = 1;
										avgRecCoeff = currRecValue[rc] / fillRecDistance;
										for (int i=0; i < fillRecDistance; i++) {
											if (overdub)
												playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += currRecValue[rc] - (avgRecCoeff * i);
											else if (recFadeIn || recFadeOut)
												playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
											else
												playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i);
											
											playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf2(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector
										}
									}
								}
								// end filling

								if (pingpong) {
									prevRecPosition = floor(loopEndPos);
									currRecPosition = floor(loopEndPos) - 1;
									reverseRecording = REVERSE;
								} else {
									prevRecPosition = floor(loopStartPos);
									currRecPosition = floor(loopStartPos) + distancePos[recOutChan];
								}

							} else if (currRecPosition - distanceRec > totalSamples) {	// ********** FORWARD  reached LAST SAMPLE

								waitingRecEdge = false;

								// filling last samples before sample end
								int directionCoeff = 1;
								prevRecPosition -= distanceRec;
								currRecPosition -= distanceRec;
								int fillRecDistance = totalSamples - floor(prevRecPosition);
								
								
								if (fillRecDistance > 0) {
									int rc = 0;
									avgRecCoeff = currRecValue[rc] / fillRecDistance;
									for (int i=0; i < fillRecDistance; i++) {
										if (overdub)
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += currRecValue[rc] - (avgRecCoeff * i);
										else if (recFadeIn || recFadeOut)
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
										else
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i);
										
										playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector
									}
									// ************ RIGHT CHANNEL ******************
									if (recChannels == 2) {
										rc = 1;
										avgRecCoeff = currRecValue[rc] / fillRecDistance;
										for (int i=0; i < fillRecDistance; i++) {
											if (overdub)
												playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += currRecValue[rc] - (avgRecCoeff * i);
											else if (recFadeIn || recFadeOut)
												playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
											else
												playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i);
											
											playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf2(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector
										}
									}
								}
								// end filling

								params[REC_PARAM].setValue(0);
								recButton = 0;
								stopRecNow = true;
							} 

						} else {			// ****************** REVERSE RECORDING


						// **************************************************************

							if (loop && currRecPosition < loopStartPos) {	// *** LOOP REVERSE  reached LOOP START
								waitingRecEdge = false;
								// filling last samples before loop start
								int directionCoeff = -1;
								int fillRecDistance = ceil(prevRecPosition) - floor(loopStartPos);
								if (fillRecDistance > 0) {
									int rc = 0;
									avgRecCoeff = currRecValue[rc] / fillRecDistance;
									for (int i=0; i < fillRecDistance; i++) {
										if (overdub)
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += currRecValue[rc] - (avgRecCoeff * i);
										else if (recFadeIn || recFadeOut)
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
										else
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i);
										
										playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector
									}
									// ************ RIGHT CHANNEL ******************
									if (recChannels == 2) {
										rc = 1;
										avgRecCoeff = currRecValue[rc] / fillRecDistance;
										for (int i=0; i < fillRecDistance; i++) {
											if (overdub)
												playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += currRecValue[rc] - (avgRecCoeff * i);
											else if (recFadeIn || recFadeOut)
												playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
											else
												playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i);
											
											playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf2(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector
										}
									}
								}
								// end filling

								if (pingpong) {
									prevRecPosition = floor(loopStartPos);
									currRecPosition = floor(loopStartPos) + 1;
									reverseRecording = FORWARD;
								} else {
									prevRecPosition = floor(loopEndPos);
									currRecPosition = floor(loopEndPos) - 1;
								}

							} else if (currRecPosition < 0) {	// ********** REVERSE  reached FIRST SAMPLE
								waitingRecEdge = false;

								// filling last samples before sample begin
								int directionCoeff = -1;
								int fillRecDistance = floor(prevRecPosition);

								int rc = 0;
								avgRecCoeff = currRecValue[rc] / (fillRecDistance + 1);
								
								for (int i=0; i <= fillRecDistance; i++) {
									if (overdub)
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += currRecValue[rc] - (avgRecCoeff * i);
									else if (recFadeIn || recFadeOut)
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
									else
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i);
									
									playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector
								}
								// ************ RIGHT CHANNEL ******************
								if (recChannels == 2) {
									rc = 1;
									avgRecCoeff = currRecValue[rc] / fillRecDistance;
									for (int i=0; i <= fillRecDistance; i++) {
										if (overdub)
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += currRecValue[rc] - (avgRecCoeff * i);
										else if (recFadeIn || recFadeOut)
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
										else
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = currRecValue[rc] - (avgRecCoeff * i);
										
										playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf2(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector
									}
								}

								// end filling

								params[REC_PARAM].setValue(0);
								recButton = 0;
								stopRecNow = true;

							} 
						}
					}
				} 
			}

			// ****************************************************************************************** R E C O R D I N G     S E C T I O N *****************

			if (stopRecNow) {
				stopRecNow = false;
				recButton = 0;
				recFadeOut = false;
				recFadeValue = 0;
				
				if (rearmSetting && !playOnRec) {
					recRearm = false;
					recRearmAfter = true;
				}

			} else {
					
				if (recFadeIn) {
					if (recFadeValue < 1)
						recFadeValue += 1 / (convertCVToSeconds(params[RECFADE_PARAM].getValue()) * args.sampleRate);
					if (recFadeValue >= 1) {
						recFadeValue = 1;
						recFadeIn = false;
					}

				} else if (recFadeOut) {

					if (recFadeValue > 0)
						recFadeValue -= 1 / (convertCVToSeconds(params[RECFADE_PARAM].getValue()) * args.sampleRate);
					if (recFadeValue <= 0) {
						recFadeValue = 0;
						recFadeOut = false;
						
						if (recRearm) {
							recRearm = false;
							recRearmAfter = true;
						}
					}
				}

				int directionCoeff;
				if (reverseRecording)
					directionCoeff = -1;
				else
					directionCoeff = 1;

				int rc = 0;
				
				prevRecValue[rc] = currRecValue[rc];
				currRecValue[rc] = inputs[IN_INPUT+rc].getVoltage() * params[GAIN_PARAM].getValue() * recFadeValue;
				avgRecCoeff = abs(currRecValue[rc] - prevRecValue[rc]) / distanceRec;

				if (currRecValue[rc] < prevRecValue[rc])
					avgRecCoeff *= -1;

				if (currRecPosition >= 0 && currRecPosition <= totalSamples && prevRecPosition >= 0 && prevRecPosition <= totalSamples) {

					if (firstSampleToRecord) {
						firstSampleToRecord = false;
						if (reverseRecording == FORWARD) {
							prevRecPosition = 0;
							distanceRec = currRecPosition;
						} else {
							prevRecPosition = totalSamples;
							distanceRec = totalSampleC - currRecPosition;
						}
					}

					if (distanceRec > 0) {
						
						lastRecordedSample = floor(prevRecPosition) + distanceRec - 1;

						for (int i=0; i < distanceRec; i++) {
							
							if (recFadeIn || recFadeOut)
								if (overdub) 
									playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += prevRecValue[rc] + (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
								else
									if (crossRecFade)
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = prevRecValue[rc] + (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
									else
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = prevRecValue[rc] + (avgRecCoeff * i);
							else
								if (overdub)
									playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += prevRecValue[rc] + (avgRecCoeff * i);
								else
									playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = prevRecValue[rc] + (avgRecCoeff * i);

							
							// *** HARD CLIP ***
							if (limiter) {
								if (playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] > 5)
									playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] = 5;
								else if (playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] < -5)
									playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] = -5;
							} else {
								if (playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] > 10)
									playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] = 10;
								else if (playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] < -10)
									playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] = -10;
							}
							
							playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector

						}

						// ************ RIGHT CHANNEL ******************
						if (recChannels == 2) {
							rc = 1;
							prevRecValue[rc] = currRecValue[rc];
							currRecValue[rc] = inputs[IN_INPUT+rc].getVoltage() * params[GAIN_PARAM].getValue() * recFadeValue;
							avgRecCoeff = abs(currRecValue[rc] - prevRecValue[rc]) / distanceRec;

							if (currRecValue[rc] < prevRecValue[rc])
								avgRecCoeff *= -1;

							for (int i=1; i <= distanceRec; i++) {

								if (recFadeIn || recFadeOut)
									if (overdub) 
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += prevRecValue[rc] + (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
									else
										if (crossRecFade)
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = prevRecValue[rc] + (avgRecCoeff * i) + ((1-recFadeValue) * playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]);
										else
											playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = prevRecValue[rc] + (avgRecCoeff * i);
								else
									if (overdub)
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] += prevRecValue[rc] + (avgRecCoeff * i);
									else
										playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)] = prevRecValue[rc] + (avgRecCoeff * i);
								
								// *** HARD CLIP ***
								if (limiter) {
									if (playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] > 5)
										playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] = 5;
									else if (playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] < -5)
										playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] = -5;
								} else {
									if (playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] > 10)
										playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] = 10;
									else if (playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] < -10)
										playBuffer[rc][0][prevRecPosition+(i*directionCoeff)] = -10;
								}
								
								playBuffer[rc][1][floor(prevRecPosition)+(i*directionCoeff)] = biquadLpf2(playBuffer[rc][0][floor(prevRecPosition)+(i*directionCoeff)]); // filtered vector

							}
						}

						// *** CLIPPING LIGHT ***
						if (playBuffer[LEFT][0][currRecPosition] < -5 || playBuffer[LEFT][0][currRecPosition] > 5) {
							clipping = true;
							clippingValue = 0;
						}
						if (recChannels == 2) {
							if (playBuffer[RIGHT][0][currRecPosition] < -5 || playBuffer[RIGHT][0][currRecPosition] > 5) {
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
					}
				}

				// ***********************************************************************************************    U P D A T E    R E C    P O S I T I O N

				prevRecPosition = currRecPosition;

				//	Update speed also with v/oct in new recording
				if (newRecording) {
					if (inputs[VO_INPUT].isConnected()) {
						voct[recOutChan] = inputs[VO_INPUT].getVoltage(recOutChan);
						if (voct[recOutChan] != prevVoct[recOutChan]) {
							speedVoct[recOutChan] = pow(2,voct[recOutChan]);
							prevVoct[recOutChan] = voct[recOutChan];
						}
						distancePos[recOutChan] = currentSpeed * sampleCoeff * speedVoct[recOutChan];
					} else
						distancePos[recOutChan] = currentSpeed * sampleCoeff;

					prevVoct[recOutChan] = voct[recOutChan];

					currRecPosition += distancePos[recOutChan];	
				
				} else {
					if (reverseRecording) {
						currRecPosition -= distancePos[recOutChan];	
					} else {
						currRecPosition += distancePos[recOutChan];	
					}
				}
				
				distanceRec = abs(floor(currRecPosition) - floor(prevRecPosition));
				
				//if (!unlimitedRecording && currRecPosition * recChannels > recordingLimit)
				if (!unlimitedRecording && currRecPosition > currentRecordingLimit)
					params[REC_PARAM].setValue(0);

				recSamples++;

			} // end if stopRecNow


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
			clipping = false;
			monitorFadeValue = 0;
			monitorFade = true;
			if (monitorSwitch == 2 && recordingState == 1) {
				monitorFadeValue = 1;
				monitorFade = false;
			} else if (monitorSwitch == 1 && recordingState == 0) {
				monitorFadeValue = 1;
				monitorFade = false;
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
							if (limiter) {
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
							} else {
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
							if (limiter) {
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
							} else {
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

			case 1:											// *** ONLY RECORDING MONITORING ***
				if (recordingState == 1) {
					switch (polyOuts) {
						case MONOPHONIC:										// monophonic CABLES
							//sumOutput += inputs[IN_INPUT].getVoltage() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0] * recFadeValue;
							sumOutput += inputs[IN_INPUT].getVoltageSum() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0] * recFadeValue;
							if (channels == 2)
								//sumOutputR += inputs[IN_INPUT+1].getVoltage() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0] * recFadeValue;
								sumOutputR += inputs[IN_INPUT+1].getVoltageSum() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0] * recFadeValue;
							
							// *** HARD CLIP ***
							if (limiter) {
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
							} else {
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
							//currentOutput = outputs[OUT_OUTPUT].getVoltage(recOutChan) + (inputs[IN_INPUT].getVoltage() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0] * recFadeValue);
							//currentOutputR = outputs[OUT_OUTPUT+1].getVoltage(recOutChan) + (inputs[IN_INPUT+1].getVoltage() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0] * recFadeValue);
							currentOutput = outputs[OUT_OUTPUT].getVoltage(recOutChan) + (inputs[IN_INPUT].getVoltageSum() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0] * recFadeValue);
							currentOutputR = outputs[OUT_OUTPUT+1].getVoltage(recOutChan) + (inputs[IN_INPUT+1].getVoltageSum() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0] * recFadeValue);
							
							// *** HARD CLIP ***
							if (limiter) {
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
							} else {
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

				} else if (monitorFade && params[RECFADE_PARAM].getValue() <= 0) {
					switch (polyOuts) {
						case MONOPHONIC:										// monophonic CABLES
							//sumOutput += inputs[IN_INPUT].getVoltage() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue) * masterLevel[0];
							sumOutput += inputs[IN_INPUT].getVoltageSum() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue) * masterLevel[0];
							if (channels == 2)
								//sumOutputR += inputs[IN_INPUT+1].getVoltage() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue) * masterLevel[0];
								sumOutputR += inputs[IN_INPUT+1].getVoltageSum() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue) * masterLevel[0];
							
							// *** HARD CLIP ***
							if (limiter) {
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
							} else {
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
							if (limiter) {
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
							} else {
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
			case 2:								// *** ALWAYS MONITORING ***
				switch (polyOuts) {
					case MONOPHONIC:										// monophonic CABLES
						//sumOutput += inputs[IN_INPUT].getVoltage() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0];
						sumOutput += inputs[IN_INPUT].getVoltageSum() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0];
						if (channels == 2)
							//sumOutputR += inputs[IN_INPUT+1].getVoltage() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0];
							sumOutputR += inputs[IN_INPUT+1].getVoltageSum() * params[GAIN_PARAM].getValue() * monitorFadeValue * masterLevel[0];
						
						// *** HARD CLIP ***
						if (limiter) {
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
						} else {
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
						if (limiter) {
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
						} else {
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

struct SickoSamplerDisplay : TransparentWidget {
	SickoSampler *module;
	int frame = 0;
	SickoSamplerDisplay() {
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

				if (module->fileLoaded || module->recordingState) {
					module->recSeconds = module->recSamples / (APP->engine->getSampleRate());
					module->recMinutes = (module->recSeconds / 60) % 60;
					module->recSeconds = module->recSeconds % 60;
					module->recTimeDisplay = std::to_string(module->recMinutes) + ":";
					if (module->recSeconds < 10)
						module->recTimeDisplay += "0";
					module->recTimeDisplay += std::to_string(module->recSeconds);
				}

				nvgTextBox(args.vg, 176, 16,97, module->recTimeDisplay.c_str(), NULL);

				nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff)); 
				nvgTextBox(args.vg, 142, 16,97, module->timeDisplay.c_str(), NULL);
	
				nvgFillColor(args.vg, nvgRGBA(0x88, 0xaa, 0xff, 0xff)); 
				nvgTextBox(args.vg, 208, 16,97, module->channelsDisplay.c_str(), NULL);

				nvgFillColor(args.vg, nvgRGBA(0xee, 0xee, 0x22, 0xff)); 
				nvgTextBox(args.vg, 237, 16,97, module->infoToSave.c_str(), NULL);

				/*
				nvgTextBox(args.vg, 9, 26,120, module->debugDisplay.c_str(), NULL);
				nvgTextBox(args.vg, 9, 36,120, module->debugDisplay2.c_str(), NULL);
				nvgTextBox(args.vg, 129, 26,120, module->debugDisplay3.c_str(), NULL);
				nvgTextBox(args.vg, 129, 36,120, module->debugDisplay4.c_str(), NULL);
				*/

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
					if (module->recordingState)
						nvgStrokeColor(args.vg, nvgRGBA(0xf5, 0x00, 0xf5, 0xff));
					else
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


					if (module->recordingState && !module->extendedRec) {
						for (int i = 0; i < 240; i++)
							module->displayBuff[i] = module->playBuffer[0][0][i*floor(module->totalSampleC/240)];
					}

					// Waveform
					nvgStrokeColor(args.vg, nvgRGBA(0x22, 0x44, 0xc9, 0xc0));
					nvgSave(args.vg);
					Rect b = Rect(Vec(7, 22), Vec(235, 73));
					nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
					nvgBeginPath(args.vg);
					for (unsigned int i = 0; i < module->displayBuff.size(); i++) {
						float x, y;

						if (module->recordingState && module->extendedRec)
							x = ((float)i * (float(module->prevRecTotalSampleC) / float(module->totalSampleC)) / (module->displayBuff.size() - 1)) ;
						else
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
		SickoSampler *module = dynamic_cast<SickoSampler*>(this->module);
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
		SickoSampler *module = dynamic_cast<SickoSampler *>(this->module);
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

struct SickoSamplerWidget : ModuleWidget {
	SickoSamplerWidget(SickoSampler *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SickoSampler.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		{
			SickoSamplerDisplay *display = new SickoSamplerDisplay();
			display->box.pos = Vec(3, 24);
			display->box.size = Vec(307, 100);
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

		const float yStartRec = 54;

		addParam(createParamCentered<VCVButton>(mm2px(Vec(18.7, 4)), module, SickoSampler::PREVSAMPLE_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(88.1, 4)), module, SickoSampler::NEXTSAMPLE_PARAM));

		addParam(createParamCentered<CKSS>(mm2px(Vec(xTrig1, yTrig1)), module, SickoSampler::TRIGGATEMODE_SWITCH));
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(xTrig2, yTrig1+1)), module, SickoSampler::TRIGMODE_SWITCH));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xTrig1-5, yTrig2)), module, SickoSampler::TRIG_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xTrig1+4.6, yTrig2)), module, SickoSampler::TRIGBUT_PARAM, SickoSampler::TRIGBUT_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xTrig2-4.5, yTrig2)), module, SickoSampler::STOP_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xTrig2+5.1, yTrig2)), module, SickoSampler::STOPBUT_PARAM, SickoSampler::STOPBUT_LIGHT));

		//----------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStart1, yStart1)), module, SickoSampler::CUESTART_PARAM));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStart2, yStart1)), module, SickoSampler::CUEEND_PARAM));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStart1, yStart2)), module, SickoSampler::LOOPSTART_PARAM));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStart2, yStart2)), module, SickoSampler::LOOPEND_PARAM));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xStart2+9.8, yStart1-1.5)), module, SickoSampler::REV_PARAM, SickoSampler::REV_LIGHT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xStart2 + 18, yStart1-1)), module, SickoSampler::XFADE_PARAM));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xStart2+10, yStart2-2)), module, SickoSampler::LOOP_PARAM, SickoSampler::LOOP_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xStart2+18, yStart2+0.5)), module, SickoSampler::PINGPONG_PARAM, SickoSampler::PINGPONG_LIGHT));
		
		//----------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xEnv1, yEnv1)), module, SickoSampler::ATTACK_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xEnv2, yEnv2)), module, SickoSampler::ATTACK_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv2+xEnv2Add, yEnv2)), module, SickoSampler::ATTACKATNV_PARAM));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xEnv1+xEnv1Add, yEnv1)), module, SickoSampler::DECAY_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xEnv2+xEnv2Skip, yEnv2)), module, SickoSampler::DECAY_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv2+xEnv2Add+xEnv2Skip, yEnv2)), module, SickoSampler::DECAYATNV_PARAM));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xEnv1+xEnv1Add*2, yEnv1)), module, SickoSampler::SUSTAIN_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xEnv2+xEnv2Skip*2, yEnv2)), module, SickoSampler::SUSTAIN_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv2+xEnv2Add+xEnv2Skip*2, yEnv2)), module, SickoSampler::SUSTAINATNV_PARAM));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xEnv1+xEnv1Add*3, yEnv1)), module, SickoSampler::RELEASE_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xEnv2+xEnv2Skip*3, yEnv2)), module, SickoSampler::RELEASE_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv2+xEnv2Add+xEnv2Skip*3, yEnv2)), module, SickoSampler::RELEASEATNV_PARAM));

		//----------------------------------------------------------------------------------------------------------------------------

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(9, 105.3)), module, SickoSampler::VO_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(24.5, yTunVol)), module, SickoSampler::TUNE_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(19.5, yTunVol2)), module, SickoSampler::TUNE_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(29.5, yTunVol2)), module, SickoSampler::TUNEATNV_PARAM));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(46, yTunVol)), module, SickoSampler::VOL_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(41, yTunVol2)), module, SickoSampler::VOL_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(51, yTunVol2)), module, SickoSampler::VOLATNV_PARAM));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(53.5, yTunVol-4.5)), module, SickoSampler::CLIPPING_LIGHT));

		addParam(createParamCentered<CKSS>(mm2px(Vec(59.8, 113)), module, SickoSampler::LIMIT_SWITCH));

		//----------------------------------------------------------------------------------------------------------------------------
		
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(70.3, 105.3)), module, SickoSampler::OUT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(80.2, 105.3)), module, SickoSampler::OUT_OUTPUT+1));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(70.3, 117.5)), module, SickoSampler::EOC_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(80.2, 117.5)), module, SickoSampler::EOR_OUTPUT));

		//----------------------------------------------------------------------------------------------------------------------------

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(100.4, 12)), module, SickoSampler::PHASESCAN_SWITCH, SickoSampler::PHASESCAN_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(91.2, 26)), module, SickoSampler::IN_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(100.5, 26)), module, SickoSampler::IN_INPUT+1));
		
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(91.88, yStartRec-15)), module, SickoSampler::GAIN_PARAM));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(100.88, yStartRec-11.65)), module, SickoSampler::RECFADE_PARAM));
		
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(91.1, yStartRec+0.5)), module, SickoSampler::REC_INPUT));
		addParam(createLightParamCentered<VCVLightBezelLatch<RedLight>>(mm2px(Vec(100.8, yStartRec+0.5)), module, SickoSampler::REC_PARAM, SickoSampler::REC_LIGHT));
		
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(100.8, yStartRec+10.1)), module, SickoSampler::RECSTOP_INPUT));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(91.5, yStartRec+22)), module, SickoSampler::OVERDUB_SWITCH, SickoSampler::OVERDUB_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(100.4, yStartRec+22)), module, SickoSampler::XTND_SWITCH, SickoSampler::XTND_LIGHT));
		
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(91.5, yStartRec+32)), module, SickoSampler::REARM_SWITCH, SickoSampler::REARM_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(100.4, yStartRec+32)), module, SickoSampler::REC_REL_SWITCH, SickoSampler::REC_REL_LIGHT));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(91.5, yStartRec+42)), module, SickoSampler::UCE_SWITCH, SickoSampler::UCE_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(100.4, yStartRec+42)), module, SickoSampler::ULE_SWITCH, SickoSampler::ULE_LIGHT));
		
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(91.5, yStartRec+52)), module, SickoSampler::PLAYONREC_SWITCH, SickoSampler::PLAYONREC_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(100.4, yStartRec+52)), module, SickoSampler::STOPONREC_SWITCH, SickoSampler::STOPONREC_LIGHT));

		addParam(createParamCentered<CKSSThree>(mm2px(Vec(94, yStartRec+62.3)), module, SickoSampler::MONITOR_SWITCH));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(9, yTunVol2+0.3)), module, SickoSampler::CLEAR_INPUT));

	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		SickoSampler *module = dynamic_cast<SickoSampler*>(this->module);
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
	   	SickoSampler *module = dynamic_cast<SickoSampler*>(this->module);
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
		menu->addChild(createBoolPtrMenuItem("UCE/ULE update also Start", "", &module->updateAlsoStart));
		menu->addChild(createBoolPtrMenuItem("Crossfade while Rec Fading", "", &module->crossRecFade));
		
		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Reset cursors on Load", "", &module->resetCursorsOnLoad));
		menu->addChild(createBoolPtrMenuItem("Disable NAV Buttons", "", &module->disableNav));
		menu->addChild(createBoolPtrMenuItem("Store Sample in Patch", "", &module->sampleInPatch));
#if !defined(METAMODULE)
		menu->addChild(createBoolPtrMenuItem("Unlimited REC (risky)", "", &module->unlimitedRecording));
#endif
		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Presets", "", [=](Menu * menu) {
			menu->addChild(createMenuItem("Wavetable", "", [=]() {module->setPreset(0);}));
			menu->addChild(createMenuItem("Triggered Sample with Envelope", "", [=]() {module->setPreset(1);}));
			menu->addChild(createMenuItem("Drum Player", "", [=]() {module->setPreset(2);}));
		}));
	}
};

Model *modelSickoSampler = createModel<SickoSampler, SickoSamplerWidget>("SickoSampler");
