#define BEAT 0
#define BAR 1

#define COLOR_RED 0xff, 0x00, 0x00, 0xff
//#define COLOR_DARK_RED 0xaf, 0xa0, 0x00, 0xff
#define COLOR_GREEN 0x00, 0xff, 0x00, 0xff
#define COLOR_DARK_GREEN 0x00, 0xaf, 0x00, 0xff
#define COLOR_BLUE 0x00, 0x00, 0xff, 0xff
#define COLOR_YELLOW 0xff, 0xff, 0x00, 0xff
#define COLOR_DARK_YELLOW 0xaf, 0xaf, 0x00, 0xff
#define COLOR_LCD_RED 0xdd, 0x33, 0x33, 0xff

#define LEFT 0
#define RIGHT 1

#define FORWARD 0
#define REVERSE 1

#define NO_PULSE 0
#define SLOW_PULSE 1
#define FAST_PULSE 2

#define EMPTY 0
// EMPTY is for trackStatus
#define NOTHING 0
// NOTHING is for nextStatus
#define IDLE 1
#define PREROLLING 2
#define PLAYING 3
#define RECORDING 4
#define OVERDUBBING 5

#define PLAY_STOP 0
#define REC_PLAY_OVERDUB 1
#define REC_OVERDUB_PLAY 2

#define START_ALL 1
#define STOP_ALL 2

#define CLICK_STANDARD 0
#define CLICK_CUSTOM 3

#define sampleCoeff 2

#include "plugin.hpp"
#include "SickoLooper1Exp.hpp"
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

struct tpSignatureSl1 : ParamQuantity {
	std::string getDisplayValueString() override {
		const std::string valueDisplay[17] = {"2/4", "3/4", "4/4", "5/4", "6/4", "7/4", "5/8", "6/8", "7/8", "8/8", "9/8", "10/8", "11/8", "12/8", "13/8", "14/8", "15/8"};
		return valueDisplay[int(getValue())];
	}
};

struct SickoLooper1 : Module {
	enum ParamIds {
		CLOCK_RST_SWITCH,
		BPM_KNOB_PARAM,
		SIGNATURE_KNOB_PARAM,
		CLICK_BUT_PARAM,
		CLICKVOL_KNOB_PARAM,
		PREROLL_BUT_PARAM,
		PREROLL_SWITCH,
		ALLSTARTSTOP_BUT_PARAM,
		SOURCELVL_KNOB_PARAM,
		MUTE_SWITCH,
		MEAS_KNOB_PARAM,
		LOOPSYNC_SWITCH,
		STARTIMM_SWITCH,
		STOPIMM_SWITCH,
		ONESHOT_SWITCH,
		REV_SWITCH,
		XFADE_KNOB_PARAM,
		PAN_KNOB_PARAM,
		VOLTRACK_KNOB_PARAM,
		REC_BUT_PARAM,
		PLAY_BUT_PARAM,
		STOP_BUT_PARAM,
		ERASE_BUT_PARAM,
		SRC_TO_TRACK_SWITCH,
		NUM_PARAMS 
	};
	enum InputIds {
		LEFT_INPUT,
		RIGHT_INPUT,
		SOURCELVL_INPUT,
		REC_TRIG_INPUT,
		PLAY_TRIG_INPUT,
		STOP_TRIG_INPUT,
		ERASE_TRIG_INPUT,
		VOCT_INPUT,
		REV_INPUT,
		PAN_INPUT,
		VOLTRACK_INPUT,
		EXTCLOCK_INPUT,
		CLOCKRESET_INPUT,
		ALLSTARTSTOP_TRIG_INPUT,
		ALLSTOP_TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		TRACK_LEFT_OUTPUT,
		TRACK_RIGHT_OUTPUT,
		EOL_OUTPUT,
		CLOCK_OUTPUT,
		RESET_OUTPUT,
		CLICK_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		CLOCK_RST_LIGHT,
		CLICK_BUT_LIGHT,
		PREROLL_BUT_LIGHT,
		ALLSTARTSTOP_BUT_LIGHT,
		MUTE_LIGHT,
		REC_BUT_LIGHT,
		PLAY_BUT_LIGHT,
		STOP_BUT_LIGHT,
		ERASE_BUT_LIGHT,
		LOOPSYNC_LIGHT,
		TEMPOSYNC_LIGHT,
		STARTIMM_LIGHT,
		STOPIMM_LIGHT,
		ONESHOT_LIGHT,
		REV_LIGHT,
		SRC_TO_TRACK_LIGHT,
		NUM_LIGHTS
	};

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

	//**************************************************************
	// EXPANDER variables

	bool connectedToMaster = true;
	int expConnected = 0;
	int receivedRunFromExp = 0;
	int sendRunToExp = 0;
	int sendSsAllToExp = 0;
	bool sendReset = false;

	Sl1ExpMsg expInputMessage[2][1];
	Sl1ExpMsg expOutputMessage[2][1];

	bool waitingExpResp = false;


	//**************************************************************
	// ANTIALIAS variables

	double a0, a1, a2, b1, b2, z1, z2, z1r, z2r;

	//**************************************************************
	// COMMON variables

	unsigned int fileChannels;
	unsigned int sampleRate = APP->engine->getSampleRate();
	unsigned int prevSampleRate = APP->engine->getSampleRate();

	//vector<float> undoBuffer[2];
	
	// metamodule change
	//vector<float> tempBuffer[2];

	int globalStatus = IDLE;
	int recordedTracks = 0;
	int busyTracks = 0;
	int lagBusyTracks = 0;
	bool busyTrack = false;
	bool preRoll = false;
	int preRollCount = 0;

	bool barReached = false;
	bool barChecked = false;
	bool loopEnd = false;
	bool stopNow = false;
	bool startNow = false;

	float allSSTrig = 0.f;
	float prevAllSSTrig = 0.f;

	float allStopTrig = 0.f;
	float prevAllStopTrig = 0.f;
	bool allStop = false;

	const std::string signatureDisplay[17] = {"2/4", "3/4", "4/4", "5/4", "6/4", "7/4", "5/8", "6/8", "7/8", "8/8", "9/8", "10/8", "11/8", "12/8", "13/8", "14/8", "15/8"};

	float extraRecMaxSamples = 0.f;

	float panKnobValue = 0.f;
	float panLeftCoeff = 1.f;
	float panRightCoeff = 1.f;

	//**************************************************************
	// TRACKS
	
	int trackStatus = EMPTY;
	int nextStatus = NOTHING;
	bool trackRecorded = false;
	vector<float> trackBuffer[2];

	drwav_uint64 totalSampleC = 0;
	drwav_uint64 totalSamples = 0;

	double samplePos = 0.0;

	double distancePos = 2.0;
	double currSampleWeight = 0.0;
	double currSampleWeightXtra = 0.0;

	int playingDirection = 0;

	float playTrig = 0.f;
	float prevPlayTrig = 0.f;
	float recTrig = 0.f;
	float prevRecTrig = 0.f;
	float stopTrig = 0.f;
	float prevStopTrig = 0.f;
	float eraseTrig = 0.f;
	float prevEraseTrig = 0.f;

	float inputValue[2];
	float volTrack = 0.f;

	float revTrig = 0.f;
	float prevRevTrig = 0.f;

	drwav_uint64 extraRecPos = 0;
	double extraPlayPos = 0;
	int extraRecCount = 0;
	int extraRecDirection = 0;
	int extraPlayDirection = 0;

	float mute = 1.f;
	float prevMute = 1.f;
	bool muteFade = false;
	float muteValue = 0.f;
	float muteDelta = 0.f;

	float srcToTrack = 1.f;
	float prevSrcToTrack = 0.f;
	bool srcToTrackFade = false;
	float srcToTrackValue = 0.f;
	float srcToTrackDelta = 0.f;

	float xFadeValue = 0.f;
	float xFadeDelta = 1.f;

	bool fadeIn = false;
	float fadeInValue = 0.f;
	float fadeInDelta = 1.f;

	bool recFade = false;
	float recFadeValue = 1.f;
	float recFadeDelta = 0.f;

	bool fadeTail = false;
	float fadeTailValue = 1.f;
	double tailEnd = 0;

	bool eraseWait = false;
	float eraseTime = 0;
	float eraseSamples = APP->engine->getSampleRate() / 1.5f;

	bool eolPulse = false;
	float eolPulseTime = 0.f;
	bool notEolPulse = false;

	int trackLoopMeas = 0;
	int loopCount = 0;

	bool extraRecording = false;
	bool extraPlaying = false;
	bool extraPlayingFadeOut = false;

	bool playTail = false;

	float voct = 0.f;
	float prevVoct = 99.f;
	float speedVoct = 0;

	// *************************************************************
	// OUTPUTS

	float currentOutput[2] = {0.f, 0.f};

	//**************************************************************
	// FILE MANAGEMENT

	bool fileLoaded = false;
	bool fileFound = false;

	// *************************************************************
	// JSON variables

	bool eolPulseOnStop = false;
	int playSequence = 0;
	bool instantStop = false;
	bool overdubAfterRec = false;
	bool fadeInOnPlay = false;
	bool extraSamples = true;
	bool playFullTail = false;
	int panRange = 0;

	// ***************************************************************************************************
	// PANEL settings

	float play_but = 0.f;
	float rec_but = 0.f;
	float stop_but = 0.f;
	float erase_but = 0.f;
	float loopSync_setting = 0.f;
	float startImm_setting = 0.f;
	float stopImm_setting = 0.f;
	float oneShot_setting = 0.f;
	float rev_setting = 0.f;
	bool ledLight = false;

	// ***************************************************************************************************
	// ***************************************************************************************************
	//                                         C   L  O  C  K
	// ***************************************************************************************************
	// ***************************************************************************************************

	const int beatMaxPerBar[17] = {2, 3, 4, 5, 6, 7, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	int currentBeatMaxPerBar = 4;

	float rstBarBut = 0.f;
	float prevRstBarBut = 1.f;

	bool rstPulse = false;
	float rstPulseTime = 0;

	float rstIn = 0.f;
	float prevRstIn = 0.f;

	unsigned int clickSampleRate[2];

	double sampleRateCoeff = (double)APP->engine->getSampleRate() * 60;

	double bpm = 0.1;

	drwav_uint64 clickTotalSampleC[2];

	const unsigned int minSamplesToLoad = 9;

	int clickSelect = CLICK_STANDARD;

	vector<float> clickPlayBuffer[2];
	vector<float> clickTempBuffer;
	vector<float> clickTempBuffer2;

	bool clickFileLoaded[2] = {false, false};
	bool clickPlay[2] = {false, false};
	double clickSamplePos[2] = {0,0};

	std::string clickStoredPath[2] = {"",""};
	std::string clickFileDescription[2] = {"--none--","--none--"};

	float extTrigValue = 0.f;
	float prevExtTrigValue = 0.f;

	float clickOutput;

	float resetBut = 0;
	float resetValue = 0;
	float prevResetValue = 0;
	bool resetStart = true;

	int runSetting = 0;
	bool internalClockAlwaysOn = false;

	double clockSample = 1.0;
	double clockMaxSample = 0.0;
	double barSample = -1;
	
	double midBeatMaxSample = 0.0;
	bool midBeatPlayed = false;

	//int beatCounter = 1;
	int beatCounter = 20;	// this hack let stars clock immediately on bar

	//float fiveMsSamples = (APP->engine->getSampleRate()) / 200;		// samples in 5ms
	//float sixMsSamples = (APP->engine->getSampleRate()) / 166.67f;	// samples in 6ms
	//float halfSecondSamples = (APP->engine->getSampleRate()) / 2;		// samples in half second

	//float minTimeSamples = (APP->engine->getSampleRate()) / 5; // use this for fade testing (200ms)
	float minTimeSamples = (APP->engine->getSampleRate()) / 125.f;		// samples in 8ms
	float minTimeSamplesOvs = minTimeSamples * 2;
	float tailSamples = APP->engine->getSampleRate() * 2; // samples in one second, oversampled
	float fadeTailDelta = 1.f / minTimeSamples;
	float oneMsSamples = (APP->engine->getSampleRate()) / 1000;			// samples in 1ms

	float fastPulseTime = (APP->engine->getSampleRate()) / 50;
	float slowPulseTime = (APP->engine->getSampleRate()) / 5;

	int playButtonPulse = NO_PULSE;
	float playButtonPulseTime = 0.f;

	int recButtonPulse = NO_PULSE;
	float recButtonPulseTime = 0.f;

	bool clockPulse = false;
	float clockPulseTime = 0.f;

	bool extSync = false;
	bool extConn = false;
	bool prevExtConn = true;
	bool extBeat = false;

#if defined(METAMODULE)
	const drwav_uint64 recordingLimit = 48000 * 60; // 60 sec limit on MM = 5.5MB
#else
	const drwav_uint64 recordingLimit = 52428800;
	// const drwav_uint64 recordingLimit = 480000; // 10 sec for test purposes
#endif
	
	// ***************************************************************************************************
	// ***************************************************************************************************

	SickoLooper1() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configInput(LEFT_INPUT,"Left");
		configInput(RIGHT_INPUT,"Right");

		configInput(EXTCLOCK_INPUT,"External Clock");
		
		configInput(CLOCKRESET_INPUT,"Reset");
		configSwitch(CLOCK_RST_SWITCH, 0.f, 1.f, 0.f, "Bar Reset", {"OFF", "ON"});
		
		configParam(BPM_KNOB_PARAM, 300.f, 3000.f, 1200.f, "Tempo", " bpm", 0, 0.1);
		paramQuantities[BPM_KNOB_PARAM]->snapEnabled = true;

		configParam<tpSignatureSl1>(SIGNATURE_KNOB_PARAM, 0.f, 16.0f, 2.f, "Time Signature");
		paramQuantities[SIGNATURE_KNOB_PARAM]->snapEnabled = true;

		configInput(ALLSTARTSTOP_TRIG_INPUT,"All Start/Stop Trig");
		configSwitch(ALLSTARTSTOP_BUT_PARAM, 0.f, 1.f, 0.f, "All Start/Stop", {"OFF", "ON"});
		configInput(ALLSTOP_TRIG_INPUT,"All Stop Trig");

		configSwitch(CLICK_BUT_PARAM, 0.f, 1.f, 1.f, "Click", {"OFF", "ON"});
		configParam(CLICKVOL_KNOB_PARAM, 0.f, 2.0f, 1.f, "Click Volume", "%", 0, 100);

		configSwitch(PREROLL_BUT_PARAM, 0.f, 1.f, 0.f, "Pre-Roll", {"OFF", "ON"});
		configSwitch(PREROLL_SWITCH, 1.f, 2.f, 1.f, "Pre-Roll", {"1 bar", "2 bar"});

		configOutput(CLOCK_OUTPUT,"Clock");
		configOutput(RESET_OUTPUT,"Reset");
		configOutput(CLICK_OUTPUT,"Click");

		configInput(SOURCELVL_INPUT,"Source Level");
		configParam(SOURCELVL_KNOB_PARAM, 0.f, 2.0f, 1.f, "Source Level", "%", 0, 100);
		
		configSwitch(MUTE_SWITCH, 0.f, 1.f, 1.f, "Mute", {"Mute OFF", "Mute ON"});

		configParam(MEAS_KNOB_PARAM, 1.f, 16.f, 1.f, "Loop Measure");
		paramQuantities[MEAS_KNOB_PARAM]->snapEnabled = true;

		configInput(PLAY_TRIG_INPUT,"Play Trig");
		configSwitch(PLAY_BUT_PARAM, 0.f, 1.f, 0.f, "Play", {"OFF", "ON"});

		configInput(REC_TRIG_INPUT,"Rec Trig");
		configSwitch(REC_BUT_PARAM, 0.f, 1.f, 0.f, "Rec", {"OFF", "ON"});

		configInput(STOP_TRIG_INPUT,"Stop Trig");
		configSwitch(STOP_BUT_PARAM, 0.f, 1.f, 0.f, "Stop", {"OFF", "ON"});

		configInput(ERASE_TRIG_INPUT,"Erase Trig");
		configSwitch(ERASE_BUT_PARAM, 0.f, 1.f, 0.f, "Erase", {"OFF", "ON"});

		configSwitch(LOOPSYNC_SWITCH, 0.f, 1.f, 1.f, "Loop Sync", {"Off", "On"});

		configSwitch(STARTIMM_SWITCH, 0.f, 1.f, 0.f, "Start Immediately", {"Off", "On"});
		configSwitch(STOPIMM_SWITCH, 0.f, 1.f, 0.f, "Stop Immediately", {"Off", "On"});

		configInput(VOCT_INPUT,"V/Oct");

		configInput(REV_INPUT,"Reverse Trig");
		configSwitch(REV_SWITCH, 0.f, 1.f, 0.f, "Reverse", {"Off", "On"});

		configSwitch(ONESHOT_SWITCH, 0.f, 1.f, 0.f, "1 shot", {"Off", "On"});

		configParam(XFADE_KNOB_PARAM, 0.f, 1000.f, 8.f, "Crossfade", "ms");
	
		configInput(PAN_INPUT,"Pan CV");
		configParam(PAN_KNOB_PARAM, -1.f, 1.f, 0.f, "Pan");

		configInput(VOLTRACK_INPUT,"Volume CV");
		configParam(VOLTRACK_KNOB_PARAM, 0.f, 1.f, 1.f, "Volume", "%", 0, 100);

		configOutput(EOL_OUTPUT,"End of Loop");

		configSwitch(SRC_TO_TRACK_SWITCH, 0.f, 1.f, 1.f, "Source to Out", {"Off", "On"});

		configOutput(TRACK_LEFT_OUTPUT,"Left");
		configOutput(TRACK_RIGHT_OUTPUT,"Right");

		trackBuffer[0].resize(0);
		trackBuffer[1].resize(0);
		
		// metamodule change
		//tempBuffer[0].resize(0);
		//tempBuffer[1].resize(0);

		setClick(0);

		rightExpander.producerMessage = &expOutputMessage[0];
		rightExpander.consumerMessage = &expOutputMessage[1];

	}


//																											░░░░░██╗░██████╗░█████╗░███╗░░██╗
//																											░░░░░██║██╔════╝██╔══██╗████╗░██║
//																											░░░░░██║╚█████╗░██║░░██║██╔██╗██║
//																											██╗░░██║░╚═══██╗██║░░██║██║╚████║
//																											╚█████╔╝██████╔╝╚█████╔╝██║░╚███║
//																											░╚════╝░╚═════╝░░╚════╝░╚═╝░░╚══╝


	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "EolPulseOnStop", json_boolean(eolPulseOnStop));
		json_object_set_new(rootJ, "playSequence", json_integer(playSequence));
		json_object_set_new(rootJ, "InstantStop", json_boolean(instantStop));
		json_object_set_new(rootJ, "overdubAfterRec", json_boolean(overdubAfterRec));
		json_object_set_new(rootJ, "panRange", json_integer(panRange));
		json_object_set_new(rootJ, "extraSamples", json_boolean(extraSamples));
		json_object_set_new(rootJ, "playFullTail", json_boolean(playFullTail));
		json_object_set_new(rootJ, "fadeInOnPlay", json_boolean(fadeInOnPlay));
		json_object_set_new(rootJ, "internalClockAlwaysOn", json_boolean(internalClockAlwaysOn));
		json_object_set_new(rootJ, "ClickSlot1", json_string(clickStoredPath[0].c_str()));
		json_object_set_new(rootJ, "ClickSlot2", json_string(clickStoredPath[1].c_str()));
		json_object_set_new(rootJ, "clickSelect", json_integer(clickSelect));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {

		json_t* eolPulseOnStopJ = json_object_get(rootJ, "EolPulseOnStop");
		if (eolPulseOnStopJ)
			eolPulseOnStop = json_boolean_value(eolPulseOnStopJ);
		json_t* playSequenceJ = json_object_get(rootJ, "playSequence");
		if (playSequenceJ) {
			playSequence = json_integer_value(playSequenceJ);
			if (playSequence < 0 || playSequence > 2)
				playSequence = 0;
		}
		json_t* instantStopJ = json_object_get(rootJ, "InstantStop");
		if (instantStopJ)
			instantStop = json_boolean_value(instantStopJ);
		json_t* overdubAfterRecJ = json_object_get(rootJ, "overdubAfterRec");
		if (overdubAfterRecJ)
			overdubAfterRec = json_boolean_value(overdubAfterRecJ);

		json_t* panRangeJ = json_object_get(rootJ, "panRange");
		if (panRangeJ) {
			panRange = json_integer_value(panRangeJ);
			if (panRange < 0 || panRange > 2)
				panRange = 0;
		}

		json_t* extraSamplesJ = json_object_get(rootJ, "extraSamples");
		if (extraSamplesJ)
			extraSamples = json_boolean_value(extraSamplesJ);

		json_t* playFullTailJ = json_object_get(rootJ, "playFullTail");
		if (playFullTailJ)
			playFullTail = json_boolean_value(playFullTailJ);

		json_t* fadeInOnPlayJ = json_object_get(rootJ, "fadeInOnPlay");
		if (fadeInOnPlayJ)
			fadeInOnPlay = json_boolean_value(fadeInOnPlayJ);

		json_t* internalClockAlwaysOnJ = json_object_get(rootJ, "internalClockAlwaysOn");
		if (internalClockAlwaysOnJ) {
			internalClockAlwaysOn = json_boolean_value(internalClockAlwaysOnJ);
			setInternalClock(internalClockAlwaysOn);
		}

		json_t *clickSlot1J = json_object_get(rootJ, "ClickSlot1");
		if (clickSlot1J) {
			clickStoredPath[0] = json_string_value(clickSlot1J);
			if (clickStoredPath[0] == "")
				clickClearSlot(0);
			else
				clickLoadSample(clickStoredPath[0], 0, true);
		}
		json_t *clickSlot2J = json_object_get(rootJ, "ClickSlot2");
		if (clickSlot2J) {
			clickStoredPath[1] = json_string_value(clickSlot2J);
			if (clickStoredPath[1] == "")
				clickClearSlot(1);
			else
				clickLoadSample(clickStoredPath[1], 1, true);
		}

		json_t* clickSelectJ = json_object_get(rootJ, "clickSelect");
		if (clickSelectJ) {
			clickSelect = json_integer_value(clickSelectJ);
			if (clickSelect < 0 || clickSelect > 3)
				clickSelect = CLICK_STANDARD;
			setClick(clickSelect);
		}

	}

	void onReset(const ResetEvent &e) override {

		system::removeRecursively(getPatchStorageDirectory().c_str());

		eolPulseOnStop = false;
		playSequence = 0;
		instantStop = false;
		overdubAfterRec = false;
		panRange = 0;
		recordedTracks = 0;

		extraSamples = true;
		playTail = false;
		playFullTail = false;
		fadeTail = false;
		fadeInOnPlay = false;
		trackBuffer[LEFT].clear();
		trackBuffer[RIGHT].clear();

		// metamodule change
		vector<float>().swap(trackBuffer[LEFT]);
		vector<float>().swap(trackBuffer[RIGHT]);

		trackRecorded = false;
		trackStatus = EMPTY;
		setEmptyLed();

		busyTracks = 0;
		lagBusyTracks = 0;
		waitingExpResp = false;
		busyTrack = false;
		receivedRunFromExp = 0;
		sendRunToExp = 0;
		sendSsAllToExp = 0;
		internalClockAlwaysOn = false;
		ledLight = false;
		expConnected = 0;

		Module::onReset(e);
	}

	void onAdd(const AddEvent& e) override {
		std::string path ;

		path = system::join(getPatchStorageDirectory(), "track.wav");
		loadSample(path);
		if (fileLoaded) {
			trackRecorded = true;
			//recordedTracks++;
		}

		Module::onAdd(e);
	}

	void onSave(const SaveEvent& e) override {
		std::string path;
		system::removeRecursively(getPatchStorageDirectory().c_str());

		if (trackStatus != EMPTY) {
			path = system::join(createPatchStorageDirectory(), "track.wav");
			saveSample(path);
		}

		Module::onSave(e);
	}


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


		if (prevSampleRate != APP->engine->getSampleRate()) {
			//WARN("SICKOLOOPER -> CHANGING SAMPLERATE");
			
			sampleRate = APP->engine->getSampleRate();

			sampleRateCoeff = (double)sampleRate * 60;
			oneMsSamples = sampleRate / 1000;
			//minTimeSamples = sampleRate / 166.67f;	// samples in 6ms
			minTimeSamples = sampleRate / 125.f;		// samples in 8ms
			minTimeSamplesOvs = minTimeSamples * 2;
			//minTimeSamples = (APP->engine->getSampleRate()) / 5;	// use this for fade testing
			tailSamples = sampleRate * 2;	// oversampled 1 sec
			fadeTailDelta = 1.f / minTimeSamples;
			
			eraseSamples = sampleRate / 1.5f;
			fastPulseTime = sampleRate / 50;
			slowPulseTime = sampleRate / 5;
			
			/*
			for (int i = 0; i < 2; i++) {
				if (clickFileLoaded[i]) {
					clickPlay[i] = false;
					clickLoadSample(clickStoredPath[i],i);
				}
			}
			*/
			setClick(clickSelect);

			if (trackStatus != EMPTY) {
				double resampleCoeff;

				z1 = 0; z2 = 0; z1r = 0; z2r = 0;

				// metamodule change
				//tempBuffer[0].clear();
				//tempBuffer[1].clear();
				vector<float> tempBuffer[2];

				for (unsigned int i=0; i < trackBuffer[LEFT].size(); i = i + 2) {
					tempBuffer[LEFT].push_back(trackBuffer[LEFT][i]);
					tempBuffer[LEFT].push_back(0);
					tempBuffer[RIGHT].push_back(trackBuffer[RIGHT][i]);
					tempBuffer[RIGHT].push_back(0);
				}

				drwav_uint64 tempSampleC = tempBuffer[LEFT].size();
				drwav_uint64 tempSamples = tempSampleC-1;					// *****   DA VERIFICARE se è -2 ********************************************
				
				
				for (unsigned int i = 1; i < tempSamples; i = i + 2) {
					tempBuffer[LEFT][i] = tempBuffer[LEFT][i-1] * .5f + tempBuffer[LEFT][i+1] * .5f;
					tempBuffer[RIGHT][i] = tempBuffer[RIGHT][i-1] * .5f + tempBuffer[RIGHT][i+1] * .5f;
				}

				tempBuffer[LEFT][tempSamples] = tempBuffer[LEFT][tempSamples-1] * .5f; // halve the last sample
				tempBuffer[RIGHT][tempSamples] = tempBuffer[RIGHT][tempSamples-1] * .5f;
				

				// ***************************************************************************

				trackBuffer[LEFT].clear();
				trackBuffer[RIGHT].clear();

				// metamodule change
				vector<float>().swap(trackBuffer[LEFT]);
				vector<float>().swap(trackBuffer[RIGHT]);

				resampleCoeff = double(prevSampleRate) / double(sampleRate);
				
				double currResamplePos = 0;
				double floorCurrResamplePos = 0;

				trackBuffer[LEFT].push_back(tempBuffer[LEFT][0]);
				trackBuffer[RIGHT].push_back(tempBuffer[RIGHT][0]);

				currResamplePos += resampleCoeff;
				floorCurrResamplePos = floor(currResamplePos);
				int temp;

				while ( floorCurrResamplePos < 1 ) {
					temp = tempBuffer[LEFT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[LEFT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					trackBuffer[LEFT].push_back(temp);
					temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
							tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					trackBuffer[RIGHT].push_back(temp);

					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				} 

				while ( currResamplePos < tempSamples - 1) {
					trackBuffer[LEFT].push_back(	hermiteInterpol(tempBuffer[LEFT][floorCurrResamplePos - 1],
																	tempBuffer[LEFT][floorCurrResamplePos],
																	tempBuffer[LEFT][floorCurrResamplePos + 1],
																	tempBuffer[LEFT][floorCurrResamplePos + 2],
																	currResamplePos - floorCurrResamplePos)
													);

					trackBuffer[RIGHT].push_back( hermiteInterpol(tempBuffer[RIGHT][floorCurrResamplePos - 1],
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
					trackBuffer[LEFT].push_back(temp);

					temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
							tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					trackBuffer[RIGHT].push_back(temp);

					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				}

				tempBuffer[LEFT].clear();
				tempBuffer[RIGHT].clear();

				totalSampleC = trackBuffer[LEFT].size();
				totalSamples = totalSampleC-1;

				calcBiquadLpf(20000.0, sampleRate * 2, 1);
				for (unsigned int i = 0; i < totalSampleC; i ++) {
					tempBuffer[LEFT].push_back(biquadLpf(trackBuffer[LEFT][i]));
					tempBuffer[RIGHT].push_back(biquadLpf2(trackBuffer[RIGHT][i]));
				}

				tempSampleC = tempBuffer[LEFT].size();
				tempSamples = tempSampleC-1;

				trackBuffer[LEFT].clear();
				trackBuffer[RIGHT].clear();
				// metamodule change
				vector<float>().swap(trackBuffer[LEFT]);
				vector<float>().swap(trackBuffer[RIGHT]);

				for (unsigned int i = 0; i < tempSampleC; i++) {
					trackBuffer[LEFT].push_back(tempBuffer[LEFT][i]);
					trackBuffer[RIGHT].push_back(tempBuffer[RIGHT][i]);
				}
				totalSampleC = trackBuffer[LEFT].size();
				totalSamples = totalSampleC-1;

				if (extraSamples)
					totalSampleC = trackBuffer[LEFT].size() - tailSamples;
				else
					totalSampleC = trackBuffer[LEFT].size();
				totalSamples = totalSampleC-1;
			}
		}
		prevSampleRate = sampleRate;
	}
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


//
//																							░██████╗░█████╗░██╗░░░██╗███████╗
//																							██╔════╝██╔══██╗██║░░░██║██╔════╝
//																							╚█████╗░███████║╚██╗░██╔╝█████╗░░
//																							░╚═══██╗██╔══██║░╚████╔╝░██╔══╝░░
//																							██████╔╝██║░░██║░░╚██╔╝░░███████╗
//																							╚═════╝░╚═╝░░╚═╝░░░╚═╝░░░╚══════╝

	void menuSaveSample() {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path)
			saveSample(path);

		free(path);
#if defined(METAMODULE)
		});
#endif
	};

	void saveSample(std::string path) {
		drwav_uint64 samples;

		//samples = trackBuffer[LEFT].size();
		samples = trackBuffer[LEFT].size() / 2;

		std::vector<float> data;

		//for (unsigned int i = 0; i <= trackBuffer[LEFT].size(); i++) {
		for (unsigned int i = 0; i <= trackBuffer[LEFT].size(); i = i + 2) {
			data.push_back(trackBuffer[LEFT][i] / 5);
			data.push_back(trackBuffer[RIGHT][i] / 5);
		}

		drwav_data_format format;
		format.container = drwav_container_riff;
		format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
	
		format.channels = 2;
		samples *= 2;

		format.sampleRate = sampleRate;

		format.bitsPerSample = 32;

		if (path.substr(path.size() - 4) != ".wav" and path.substr(path.size() - 4) != ".WAV")
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
		if (path)
			loadSample(path);

		if (fileLoaded) {
			if (!trackRecorded) {
				trackRecorded = true;
				//recordedTracks++;
			}
		}

		free(path);

		fileLoaded = false;
#if defined(METAMODULE)
		});
#endif
	}

	void loadSample(std::string path) {
		z1 = 0; z2 = 0; z1r = 0; z2r = 0;

		fileLoaded = false;

		// metamodule change
		//tempBuffer[0].clear();
		//tempBuffer[1].clear();
		vector<float> tempBuffer[2];

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

			// stop track
			if ((trackStatus != IDLE && trackStatus != EMPTY) || nextStatus != NOTHING) {
				//busyTracks--;
				busyTrack = false;
			}

			trackStatus = EMPTY;
			nextStatus = NOTHING;
			samplePos = 0;
			setEmptyLed();

			fileChannels = c;

			unsigned int fileSampleRate = sr;

			samplePos = 0;

			trackBuffer[LEFT].clear();
			trackBuffer[RIGHT].clear();
			tempBuffer[LEFT].clear();
			tempBuffer[RIGHT].clear();

			// metamodule change
			vector<float>().swap(trackBuffer[LEFT]);
			vector<float>().swap(trackBuffer[RIGHT]);

			/*
			if (tsc > 52428800)
				tsc = 52428800;	// set memory allocation limit to 200Mb for samples (~18mins at 48.000khz MONO)
			*/

			if (tsc > recordingLimit)
				tsc = recordingLimit;

			if (fileSampleRate == sampleRate) {			//  **************************   NO RESAMPLE   ************************
				for (unsigned int i=0; i < tsc; i = i + c) {
					trackBuffer[LEFT].push_back(pSampleData[i] * 5);
					trackBuffer[LEFT].push_back(0);
					if (fileChannels == 2) {
						trackBuffer[RIGHT].push_back(pSampleData[i+1] * 5);
					} else {
						trackBuffer[RIGHT].push_back(pSampleData[i] * 5);
					}
					trackBuffer[RIGHT].push_back(0);

					if (i > 0) {
						trackBuffer[LEFT][i-1] = trackBuffer[LEFT][i-2] * .5 + trackBuffer[LEFT][i] * .5;
						trackBuffer[RIGHT][i-1] = trackBuffer[RIGHT][i-2] * .5 + trackBuffer[RIGHT][i] * .5;
					}
				}
				trackBuffer[LEFT].push_back(trackBuffer[LEFT][trackBuffer[LEFT].size()-2] * .5);
				trackBuffer[RIGHT].push_back(trackBuffer[RIGHT][trackBuffer[RIGHT].size()-2] * .5);

//				drwav_free(pSampleData);

			} else if (fileSampleRate == APP->engine->getSampleRate() * 2) {	// ***** LOAD DIRECTLY OVERSAMPLED, NO RESAMPLE *****
				for (unsigned int i=0; i < tsc; i = i + c) {
					trackBuffer[LEFT].push_back(pSampleData[i] * 5);
					if (fileChannels == 2) {
						trackBuffer[RIGHT].push_back(pSampleData[i+1] * 5);
					} else {
						trackBuffer[RIGHT].push_back(pSampleData[i] * 5);
					}
				}
//				drwav_free(pSampleData);

				//resampled = false;

				//sampleRate = APP->engine->getSampleRate() * 2;

			} else {											// ***************** RESAMPLE ****************************************
				
				for (unsigned int i=0; i < tsc; i = i + c) {
					tempBuffer[LEFT].push_back(pSampleData[i] * 5);
					tempBuffer[LEFT].push_back(0);
					if (fileChannels == 2)
						tempBuffer[RIGHT].push_back(pSampleData[i+1] * 5);
					else
						tempBuffer[RIGHT].push_back(pSampleData[i] * 5);
					tempBuffer[RIGHT].push_back(0);
				}

//				drwav_free(pSampleData);

				drwav_uint64 tempSampleC = tempBuffer[LEFT].size();
				drwav_uint64 tempSamples = tempSampleC-1;					// *****   DA VERIFICARE se è -2 ********************************************
				
				for (unsigned int i = 1; i < tempSamples; i = i + 2) { // **************** tempSamples  o tempSampleC ????
					tempBuffer[LEFT][i] = tempBuffer[LEFT][i-1] * .5f + tempBuffer[LEFT][i+1] * .5f;
					tempBuffer[RIGHT][i] = tempBuffer[RIGHT][i-1] * .5f + tempBuffer[RIGHT][i+1] * .5f;
				}

				tempBuffer[LEFT][tempSamples] = tempBuffer[LEFT][tempSamples-1] * .5f; // halve the last sample
				tempBuffer[RIGHT][tempSamples] = tempBuffer[RIGHT][tempSamples-1] * .5f;

				// ***************************************************************************

				double resampleCoeff = double(fileSampleRate) / double(sampleRate);
				double currResamplePos = 0;
				int floorCurrResamplePos = 0;

				trackBuffer[LEFT].push_back(tempBuffer[LEFT][0]);
				trackBuffer[RIGHT].push_back(tempBuffer[RIGHT][0]);

				currResamplePos += resampleCoeff;
				floorCurrResamplePos = floor(currResamplePos);
				int temp;

				//INFO("[ sickoCV ] ARRIVATO QUI 0 %s\n",(to_string(resampleCoeff)).c_str());

				while ( floorCurrResamplePos < 1 ) {
					temp = tempBuffer[LEFT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[LEFT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					trackBuffer[LEFT].push_back(temp);

					temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
							tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					trackBuffer[RIGHT].push_back(temp);
					
					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				} 

				while ( currResamplePos < tempSamples - 1) {
					trackBuffer[LEFT].push_back(	hermiteInterpol(tempBuffer[LEFT][floorCurrResamplePos - 1],
																	tempBuffer[LEFT][floorCurrResamplePos],
																	tempBuffer[LEFT][floorCurrResamplePos + 1],
																	tempBuffer[LEFT][floorCurrResamplePos + 2],
																	currResamplePos - floorCurrResamplePos)
													);

					trackBuffer[RIGHT].push_back( hermiteInterpol(tempBuffer[RIGHT][floorCurrResamplePos - 1],
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
					trackBuffer[LEFT].push_back(temp);

					temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
							tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					trackBuffer[RIGHT].push_back(temp);

					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				}

				tempBuffer[LEFT].clear();
				tempBuffer[RIGHT].clear();

				// ***************************************************************************

				totalSampleC = trackBuffer[LEFT].size();
				totalSamples = totalSampleC-1;

				calcBiquadLpf(20000.0, sampleRate * 2, 1);
				//for (unsigned int i = 0; i < totalSampleC; i = i + 2) {	// ****************************** forse si può saltare un passaggio
				for (unsigned int i = 0; i < totalSampleC; i++) {
					tempBuffer[LEFT].push_back(biquadLpf(trackBuffer[LEFT][i]));
					tempBuffer[RIGHT].push_back(biquadLpf2(trackBuffer[RIGHT][i]));
				}

				tempSampleC = tempBuffer[LEFT].size();
				tempSamples = tempSampleC-1;

				trackBuffer[LEFT].clear();
				trackBuffer[RIGHT].clear();

				// metamodule change
				vector<float>().swap(trackBuffer[LEFT]);
				vector<float>().swap(trackBuffer[RIGHT]);

				for (unsigned int i = 0; i < tempSampleC; i++) {
					trackBuffer[LEFT].push_back(tempBuffer[LEFT][i]);
					trackBuffer[RIGHT].push_back(tempBuffer[RIGHT][i]);
				}
				totalSampleC = trackBuffer[LEFT].size();
				totalSamples = totalSampleC-1;
				//	******************************************************************** fine dell'eventuale salto di un passaggio
			}

			if (extraSamples)
				totalSampleC = trackBuffer[LEFT].size() - tailSamples;
			else
				totalSampleC = trackBuffer[LEFT].size();
			totalSamples = totalSampleC-1;

			fileLoaded = true;
			trackStatus = IDLE;
			setIdleLed();
		} 
	};


//
//																							██████╗░██████╗░███████╗░██████╗███████╗████████╗
//																							██╔══██╗██╔══██╗██╔════╝██╔════╝██╔════╝╚══██╔══╝
//																							██████╔╝██████╔╝█████╗░░╚█████╗░█████╗░░░░░██║░░░
//																							██╔═══╝░██╔══██╗██╔══╝░░░╚═══██╗██╔══╝░░░░░██║░░░
//																							██║░░░░░██║░░██║███████╗██████╔╝███████╗░░░██║░░░
//																							╚═╝░░░░░╚═╝░░╚═╝╚══════╝╚═════╝░╚══════╝░░░╚═╝░░░

/*
	json_t *presetToJson() {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "bpm", json_integer(int(bpm*10)));
		json_object_set_new(rootJ, "meter", json_integer(int(params[SIGNATURE_KNOB_PARAM].getValue())));

		json_object_set_new(rootJ, "eolPulseOnStop", json_boolean(eolPulseOnStop));
		json_object_set_new(rootJ, "playSequence", json_integer(playSequence));
		json_object_set_new(rootJ, "instantStop", json_boolean(instantStop));
		json_object_set_new(rootJ, "overdubAfterRec", json_boolean(overdubAfterRec));
		json_object_set_new(rootJ, "panRange", json_integer(panRange));
		json_object_set_new(rootJ, "internalClockAlwaysOn", json_boolean(internalClockAlwaysOn));
		json_object_set_new(rootJ, "clickSlot1", json_string(clickStoredPath[0].c_str()));
		json_object_set_new(rootJ, "clickSlot2", json_string(clickStoredPath[1].c_str()));
		json_object_set_new(rootJ, "click", json_integer(int(params[CLICK_BUT_PARAM].getValue())));
		json_object_set_new(rootJ, "clickVol", json_real(params[CLICKVOL_KNOB_PARAM].getValue()));

		json_object_set_new(rootJ, "sourceLvl", json_real(params[SOURCELVL_KNOB_PARAM].getValue()));
		json_object_set_new(rootJ, "sourceMute", json_integer(int(mute)));
		json_object_set_new(rootJ, "trackLoopMeas", json_integer(trackLoopMeas));
		json_object_set_new(rootJ, "startImm", json_integer(int(startImm_setting)));
		json_object_set_new(rootJ, "stopImm", json_integer(int(stopImm_setting)));
		json_object_set_new(rootJ, "loopSync", json_integer(int(loopSync_setting)));
		json_object_set_new(rootJ, "oneShot", json_integer(int(oneShot_setting)));
		json_object_set_new(rootJ, "rev", json_integer(int(rev_setting)));
		json_object_set_new(rootJ, "xFade", json_real(params[XFADE_KNOB_PARAM].getValue()));
		json_object_set_new(rootJ, "pan", json_real(params[PAN_KNOB_PARAM].getValue()));
		json_object_set_new(rootJ, "volTrack", json_real(volTrack));
		json_object_set_new(rootJ, "srcToTrack", json_integer(int(srcToTrack)));
		json_object_set_new(rootJ, "extraSamples", json_boolean(extraSamples));
		json_object_set_new(rootJ, "playFullTail", json_boolean(playFullTail));
		json_object_set_new(rootJ, "fadeInOnPlay", json_boolean(fadeInOnPlay));
		return rootJ;
	}

	void presetFromJson(json_t *rootJ) {
		json_t *bpmJ = json_object_get(rootJ, "bpm");
		if (bpmJ) {
			params[BPM_KNOB_PARAM].setValue(json_integer_value(bpmJ));
		}
		json_t *meterJ = json_object_get(rootJ, "meter");
		if (meterJ) {
			params[SIGNATURE_KNOB_PARAM].setValue(json_integer_value(meterJ));
		}
		json_t *eolPulseOnStopJ = json_object_get(rootJ, "eolPulseOnStop");
		if (eolPulseOnStopJ) {
			eolPulseOnStop = json_boolean_value(eolPulseOnStopJ);
		}
		json_t *playSequenceJ = json_object_get(rootJ, "playSequence");
		if (playSequenceJ) {
			playSequence = json_integer_value(playSequenceJ);
			if (playSequence < 0 || playSequence > 2)
				playSequence = 0;
		}
		json_t *instantStopJ = json_object_get(rootJ, "instantStop");
		if (instantStopJ) {
			instantStop = json_boolean(instantStopJ);
		}
		json_t *overdubAfterRecJ = json_object_get(rootJ, "overdubAfterRec");
		if (overdubAfterRecJ)
			overdubAfterRec = json_boolean_value(overdubAfterRecJ);
		json_t *panRangeJ = json_object_get(rootJ, "panRange");
		if (panRangeJ) {
			panRange = json_integer_value(panRangeJ);
			if (panRange < 0 || panRange > 2)
				panRange = 0;
		}
		json_t *internalClockAlwaysOnJ = json_object_get(rootJ, "internalClockAlwaysOn");
		if (internalClockAlwaysOnJ) {
			internalClockAlwaysOn = json_boolean_value(internalClockAlwaysOnJ);
			setInternalClock(internalClockAlwaysOn);
		}
		json_t *clickSlot1J = json_object_get(rootJ, "clickSlot1");
		if (clickSlot1J) {
			clickStoredPath[0] = json_string_value(clickSlot1J);
			clickLoadSample(clickStoredPath[0], 0);
		}
		json_t *clickSlot2J = json_object_get(rootJ, "clickSlot2");
		if (clickSlot2J) {
			clickStoredPath[1] = json_string_value(clickSlot2J);
			clickLoadSample(clickStoredPath[1], 1);
		}
		json_t *clickJ = json_object_get(rootJ, "click");
		if (clickJ) {
			params[CLICK_BUT_PARAM].setValue(json_integer_value(clickJ));
		}
		json_t *clickVolJ = json_object_get(rootJ, "clickVol");
		if (clickVolJ) {
			params[CLICKVOL_KNOB_PARAM].setValue(json_real_value(clickVolJ));
		}

		json_t *preRollJ = json_object_get(rootJ, "preRoll");
		if (preRollJ) {
			preRoll = json_boolean_value(preRollJ);
			params[PREROLL_BUT_PARAM].setValue(preRoll);
		}
		json_t *preRollBarsJ = json_object_get(rootJ, "preRollBars");
		if (preRollBarsJ) {
			params[PREROLL_SWITCH].setValue(json_integer_value(preRollBarsJ));
		}

		// ************    T R A C K    0    ***************
		json_t *sourceLvlJ = json_object_get(rootJ, "sourceLvl");
		if (sourceLvlJ)
			params[SOURCELVL_KNOB_PARAM].setValue(json_real_value(sourceLvlJ));
		json_t *sourceMuteJ = json_object_get(rootJ, "sourceMute");
		if (sourceMuteJ)
			params[MUTE_SWITCH].setValue(json_integer_value(sourceMuteJ));

		json_t *trackLoopMeasJ = json_object_get(rootJ, "trackLoopMeas");
		if (trackLoopMeasJ)
			params[MEAS_KNOB_PARAM].setValue(json_integer_value(trackLoopMeasJ));
		json_t *startImmJ = json_object_get(rootJ, "startImm");
		if (startImmJ)
			params[STARTIMM_SWITCH].setValue(json_integer_value(startImmJ));
		json_t *stopImmJ = json_object_get(rootJ, "stopImm");
		if (stopImmJ)
			params[STOPIMM_SWITCH].setValue(json_integer_value(stopImmJ));
		json_t *oneShotJ = json_object_get(rootJ, "oneShot");
		if (oneShotJ)
			params[ONESHOT_SWITCH].setValue(json_integer_value(oneShotJ));
		json_t *loopSyncJ = json_object_get(rootJ, "loopSync");
		if (loopSyncJ)
			params[LOOPSYNC_SWITCH].setValue(json_integer_value(loopSyncJ));
		json_t *revJ = json_object_get(rootJ, "rev");
		if (revJ)
			params[REV_SWITCH].setValue(json_integer_value(revJ));

		json_t *xFade_settingJ = json_object_get(rootJ, "xFade");
		if (xFade_settingJ)
			params[XFADE_KNOB_PARAM].setValue(json_real_value(xFade_settingJ));
		json_t *panJ = json_object_get(rootJ, "pan");
		if (panJ)
			params[PAN_KNOB_PARAM].setValue(json_real_value(panJ));
		json_t *volTrackJ = json_object_get(rootJ, "volTrack");
		if (volTrackJ)
			params[VOLTRACK_KNOB_PARAM].setValue(json_real_value(volTrackJ));
		
		json_t *srcToTrackJ = json_object_get(rootJ, "srcToTrack");
		if (srcToTrackJ)
			params[SRC_TO_TRACK_SWITCH].setValue(json_integer_value(srcToTrackJ));
		
		json_t *extraSamplesJ = json_object_get(rootJ, "extraSamples");
		if (extraSamplesJ)
			extraSamples = json_boolean_value(extraSamplesJ);
		json_t *playFullTailJ = json_object_get(rootJ, "playFullTail");
		if (playFullTailJ)
			playFullTail = json_boolean_value(playFullTailJ);
		json_t *fadeInOnPlayJ = json_object_get(rootJ, "fadeInOnPlay");
		if (fadeInOnPlayJ)
			fadeInOnPlay = json_boolean_value(fadeInOnPlayJ);
	
	}

	void menuLoadPreset() {
		static const char FILE_FILTERS[] = "sickolooper preset (.slp):slp,SLP";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);

		if (path)
			loadPreset(path);

		free(path);

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

			std::string trackPath = path.substr(0, path.length() - 4) + "_track1.wav";
			loadSample(trackPath);

			if (fileLoaded) {
				fileLoaded = false;
				if (!trackRecorded) {
					trackRecorded = true;
					//recordedTracks++;
				}
			} else {
				trackRecorded = false;
				trackStatus = EMPTY;
				setEmptyLed();
			}

		} else {
			WARN("problem loading preset json file");
			//return;
		}
		
	}

	void menuSavePreset(bool loops) {

		static const char FILE_FILTERS[] = "sickolooper preset (.slp):slp,SLP";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".slp" and strPath.substr(strPath.size() - 4) != ".SLP")
				strPath += ".slp";
			path = strcpy(new char[strPath.length() + 1], strPath.c_str());
			savePreset(path, presetToJson(), loops);
		}

		free(path);
	}

	void savePreset(std::string path, json_t *rootJ, bool loops) {

		if (rootJ) {
			FILE *file = fopen(path.c_str(), "w");
			if (!file) {
				WARN("[ SickoCV ] cannot open '%s' to write\n", path.c_str());
				//return;
			} else {
				json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
				json_decref(rootJ);
				fclose(file);
				if (loops) {

					std::string trackPath = path.substr(0, path.length() - 4) + "_track1";
					system::remove((trackPath+".wav").c_str());
					system::remove((trackPath+".WAV").c_str());
					if (trackRecorded)
						saveSample(trackPath);

				}
			}
		}
	}
*/

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



//																											░█████╗░██╗░░░░░██╗░█████╗░██╗░░██╗
//																											██╔══██╗██║░░░░░██║██╔══██╗██║░██╔╝
//																											██║░░╚═╝██║░░░░░██║██║░░╚═╝█████═╝░
//																											██║░░██╗██║░░░░░██║██║░░██╗██╔═██╗░
//																											╚█████╔╝███████╗██║╚█████╔╝██║░╚██╗
//																											░╚════╝░╚══════╝╚═╝░╚════╝░╚═╝░░╚═╝


	void clickMenuLoadSample(int slot) {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV;All files (*.*):*.*";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		clickFileLoaded[slot] = false;
		if (path) {
			/*
			if (clickSelect == CLICK_CUSTOM)
				clickLoadSample(path, slot, true);
			clickStoredPath[slot] = std::string(path);
			*/
			clickLoadSample(path, slot, true);
			clickStoredPath[slot] = std::string(path);
			if (clickSelect != CLICK_CUSTOM)
				setClick(clickSelect);

		} else {
			clickFileLoaded[slot] = true;
		}
		if (clickStoredPath[slot] == "") {
			clickFileLoaded[slot] = false;
		}
		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void clickLoadSample(std::string path, int slot, bool customClick) {
		z1 = 0; z2 = 0;

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

			clickSampleRate[slot] = sr * 2;
			
			clickPlayBuffer[slot].clear();

			clickTempBuffer.clear();
			clickTempBuffer2.clear();

			if (tsc > 96000)
				tsc = 96000;	// set memory allocation limit to 96000 samples*/

			if (sr == APP->engine->getSampleRate()) {			//  **************************   NO RESAMPLE   ************************
				for (unsigned int i=0; i < tsc; i = i + c) {
					clickPlayBuffer[slot].push_back(pSampleData[i] * 5);
				}
				clickTotalSampleC[slot] = clickPlayBuffer[slot].size();

//				drwav_free(pSampleData);

				clickSampleRate[slot] = APP->engine->getSampleRate();

			} else {											// ***************** RESAMPLE ****************************************
				for (unsigned int i=0; i < tsc; i = i + c) {
					clickTempBuffer.push_back(pSampleData[i] * 5);
					clickTempBuffer.push_back(0);
				}

//s				drwav_free(pSampleData);

				drwav_uint64 clickTempSampleC = clickTempBuffer.size();
				drwav_uint64 clickTempSamples = clickTempSampleC-1;
				
				for (unsigned int i = 1; i < clickTempSamples; i = i+2)
					clickTempBuffer[i] = clickTempBuffer[i-1] * .5f + clickTempBuffer[i+1] * .5f;

				clickTempBuffer[clickTempSamples] = clickTempBuffer[clickTempSamples-1] * .5f; // halve the last sample

				// ***************************************************************************

				double clickResampleCoeff = clickSampleRate[slot] * .5 / (APP->engine->getSampleRate());
				double clickCurrResamplePos = 0;
				int clickFloorCurrResamplePos = 0;

				clickTempBuffer2.push_back(clickTempBuffer[0]);

				clickCurrResamplePos += clickResampleCoeff;
				clickFloorCurrResamplePos = floor(clickCurrResamplePos);
				int temp;

				while ( clickFloorCurrResamplePos < 1 ) {
					temp = clickTempBuffer[clickFloorCurrResamplePos]* (1-(clickCurrResamplePos - clickFloorCurrResamplePos)) + 
								clickTempBuffer[clickFloorCurrResamplePos+1]*(clickCurrResamplePos - clickFloorCurrResamplePos);

					clickTempBuffer2.push_back(temp);

					clickCurrResamplePos += clickResampleCoeff;
					clickFloorCurrResamplePos = floor(clickCurrResamplePos);
				} 

				while ( clickCurrResamplePos < clickTempSamples - 1) {

					clickTempBuffer2.push_back(	hermiteInterpol(clickTempBuffer[clickFloorCurrResamplePos - 1],
															clickTempBuffer[clickFloorCurrResamplePos],
															clickTempBuffer[clickFloorCurrResamplePos + 1],
															clickTempBuffer[clickFloorCurrResamplePos + 2],
															clickCurrResamplePos - clickFloorCurrResamplePos)
											);

					clickCurrResamplePos += clickResampleCoeff;
					clickFloorCurrResamplePos = floor(clickCurrResamplePos);
				}

				while ( clickFloorCurrResamplePos < double(clickTempSampleC) ) {
					temp = clickTempBuffer[clickFloorCurrResamplePos]* (1-(clickCurrResamplePos - clickFloorCurrResamplePos)) + 
								clickTempBuffer[clickFloorCurrResamplePos+1]*(clickCurrResamplePos - clickFloorCurrResamplePos);
					clickTempBuffer2.push_back(temp);

					clickCurrResamplePos += clickResampleCoeff;
					clickFloorCurrResamplePos = floor(clickCurrResamplePos);
				}

				// ***************************************************************************

				clickTempBuffer.clear();
				clickTotalSampleC[slot] = clickTempBuffer2.size();

				calcBiquadLpf(20000.0, clickSampleRate[slot], 1);
				for (unsigned int i = 0; i < clickTotalSampleC[slot]; i++)
					clickTempBuffer.push_back(biquadLpf(clickTempBuffer2[i]));

				for (unsigned int i = 0; i < clickTotalSampleC[slot]; i = i + 2)
					clickPlayBuffer[slot].push_back(clickTempBuffer[i]);

				clickTotalSampleC[slot] = clickPlayBuffer[slot].size();
				clickSampleRate[slot] = APP->engine->getSampleRate();

			}

			clickTempBuffer.clear();
			clickTempBuffer2.clear();

			char* pathDup = strdup(path.c_str());

			//if (clickSelect == CLICK_CUSTOM) {
			if (customClick) {
				clickFileDescription[slot] = basename(pathDup);

				clickStoredPath[slot] = path;
	
			}
			clickFileLoaded[slot] = true;
			free(pathDup);

		} else {
			clickFileLoaded[slot] = false;
			if (clickSelect == CLICK_CUSTOM) {
				clickStoredPath[slot] = path;
				clickFileDescription[slot] = "(!)"+path;
			}
		}
	};
	

	void clickClearSlot(int slot) {
		clickStoredPath[slot] = "";
		clickFileDescription[slot] = "--none--";
		if (clickSelect == CLICK_CUSTOM) {
			clickFileLoaded[slot] = false;
			clickPlayBuffer[slot].clear();
			clickTotalSampleC[slot] = 0;
		}
	}

	void setClick(int clickNo) {
		switch (clickNo) {
			case 0:
				clickLoadSample(asset::plugin(pluginInstance, "res/clicks/click0_beat.wav"), 0, false);
				clickLoadSample(asset::plugin(pluginInstance, "res/clicks/click0_bar.wav"), 1, false);
			break;

			case 1:
				clickLoadSample(asset::plugin(pluginInstance, "res/clicks/click1_beat.wav"), 0, false);
				clickLoadSample(asset::plugin(pluginInstance, "res/clicks/click1_bar.wav"), 1, false);
			break;

			case 2:
				clickLoadSample(asset::plugin(pluginInstance, "res/clicks/click2_beat.wav"), 0, false);
				clickLoadSample(asset::plugin(pluginInstance, "res/clicks/click2_bar.wav"), 1, false);
			break;

			case 3:
				if (clickStoredPath[0] != "")
					clickLoadSample(clickStoredPath[0], 0, true);
				else
					clickClearSlot(0);
				if (clickStoredPath[1] != "")
					clickLoadSample(clickStoredPath[1], 1, true);
				else clickClearSlot(1);
			break;
		}
	}


//																													██╗░░░░░███████╗██████╗░
//																													██║░░░░░██╔════╝██╔══██╗
//																													██║░░░░░█████╗░░██║░░██║
//																													██║░░░░░██╔══╝░░██║░░██║
//																													███████╗███████╗██████╔╝
//																													╚══════╝╚══════╝╚═════╝░


	void inline setEmptyLed() {
		playButtonPulse = NO_PULSE;
		lights[PLAY_BUT_LIGHT].setBrightness(0.f);
		recButtonPulse = NO_PULSE;
		lights[REC_BUT_LIGHT].setBrightness(0.f);
	}

	void inline setIdleLed() {
		playButtonPulse = NO_PULSE;
		lights[PLAY_BUT_LIGHT].setBrightness(1.f);
		recButtonPulse = NO_PULSE;
		lights[REC_BUT_LIGHT].setBrightness(0.f);
	}

	void inline setPlayLed() {
		playButtonPulse = SLOW_PULSE;
		playButtonPulseTime = slowPulseTime;
		lights[PLAY_BUT_LIGHT].setBrightness(1.f);
		recButtonPulse = NO_PULSE;
		lights[REC_BUT_LIGHT].setBrightness(0.f);
	}

	void inline setFastPlayLed() {
		playButtonPulse = FAST_PULSE;
		playButtonPulseTime = fastPulseTime;
		lights[PLAY_BUT_LIGHT].setBrightness(1.f);
		recButtonPulse = NO_PULSE;
		lights[REC_BUT_LIGHT].setBrightness(0.f);
	}

	void inline setRecLed() {
		playButtonPulse = NO_PULSE;
		lights[PLAY_BUT_LIGHT].setBrightness(0.f);
		recButtonPulse = SLOW_PULSE;
		recButtonPulseTime= slowPulseTime;
		lights[REC_BUT_LIGHT].setBrightness(1.f);	
	}

	void inline setFastRecLed() {
		playButtonPulse = NO_PULSE;
		lights[PLAY_BUT_LIGHT].setBrightness(0.f);
		recButtonPulse = FAST_PULSE;
		recButtonPulseTime = fastPulseTime;
		lights[REC_BUT_LIGHT].setBrightness(1.f);	
	}

	void inline setOverdubLed() {
		playButtonPulse = SLOW_PULSE;
		playButtonPulseTime = slowPulseTime;
		lights[PLAY_BUT_LIGHT].setBrightness(1.f);
		recButtonPulse = SLOW_PULSE;
		recButtonPulseTime = slowPulseTime;
		lights[REC_BUT_LIGHT].setBrightness(1.f);	
	}

	void inline setFastOverdubLed() {
		playButtonPulse = FAST_PULSE;
		playButtonPulseTime = fastPulseTime;
		lights[PLAY_BUT_LIGHT].setBrightness(1.f);
		recButtonPulse = FAST_PULSE;
		recButtonPulseTime = fastPulseTime;
		lights[REC_BUT_LIGHT].setBrightness(1.f);	
	}

	void inline setPlayToRecLed() {
		playButtonPulse = SLOW_PULSE;
		playButtonPulseTime = slowPulseTime;
		lights[PLAY_BUT_LIGHT].setBrightness(1.f);
		recButtonPulse = FAST_PULSE;
		recButtonPulseTime = fastPulseTime;
		lights[REC_BUT_LIGHT].setBrightness(1.f);	
	}

	void inline setRecToPlayLed() {
		playButtonPulse = FAST_PULSE;
		playButtonPulseTime = fastPulseTime;
		lights[PLAY_BUT_LIGHT].setBrightness(1.f);
		recButtonPulse = SLOW_PULSE;
		recButtonPulseTime = fastPulseTime;
		lights[REC_BUT_LIGHT].setBrightness(1.f);	
	}

	void inline xFadePlay() {
		extraPlaying = true;
		extraPlayPos = samplePos;
		currSampleWeightXtra = extraPlayPos - floor(extraPlayPos);
		extraPlayDirection = playingDirection;
		xFadeValue = 1.f;
		if (!stopNow)
			xFadeDelta = 1000 / (params[XFADE_KNOB_PARAM].getValue() * APP->engine->getSampleRate());
		else
			xFadeDelta = 1 / minTimeSamples;
	}

	bool isExtraSamples() {
		return extraSamples;
	}

	void setExtraSamples(bool poly) {
		if (poly) {
			if (totalSampleC > tailSamples) {
				extraSamples = true;
				totalSampleC = trackBuffer[LEFT].size() - tailSamples;
				totalSamples = totalSampleC-1;
			} else {
				extraSamples = false;
				totalSampleC = trackBuffer[LEFT].size();
				totalSamples = totalSampleC-1;
			}
		} else {
			extraSamples = false;
			totalSampleC = trackBuffer[LEFT].size();
			totalSamples = totalSampleC-1;
		}
	}

	void detectTempo() {
		if (beatMaxPerBar[int(params[SIGNATURE_KNOB_PARAM].getValue())] < 6)
			params[BPM_KNOB_PARAM].setValue(int(600 * (double)sampleRate * beatMaxPerBar[int(params[SIGNATURE_KNOB_PARAM].getValue())] / totalSampleC * 2) * params[MEAS_KNOB_PARAM].getValue());
		else
			params[BPM_KNOB_PARAM].setValue(int(300 * (double)sampleRate * beatMaxPerBar[int(params[SIGNATURE_KNOB_PARAM].getValue())] / totalSampleC * 2) * params[MEAS_KNOB_PARAM].getValue());
	}

	bool isInternalClockAlwaysOn() {
		return internalClockAlwaysOn;
	}

	void setInternalClock(bool internalClock) {
		if (internalClock) {
			internalClockAlwaysOn = true;
			runSetting = 1;
			sendRunToExp = 1;
		} else {
			internalClockAlwaysOn = false;
			if (busyTracks == 0) {
				globalStatus = IDLE;
				
				//if (!extConn) {
				if (!extConn && !internalClockAlwaysOn) {
					runSetting = 0;
					sendRunToExp = 0;
					clockSample = 1.0;
					resetStart = true;
					beatCounter = 20;	// this hack makes the click be audible on next runSetting=1
				}
			}
		}
	}

	void inline cancelNextStatus() {
		if (trackStatus == PLAYING)
			setPlayLed();
		else if (trackStatus == RECORDING)
			setRecLed();
		else if (trackStatus == OVERDUBBING)
			setOverdubLed();
		nextStatus = NOTHING;
	}

	void inline resetIdleEmptyStatus() {
		if (trackStatus == EMPTY)
			setEmptyLed();
		else
			setIdleLed();
		nextStatus = NOTHING;
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

/*
																				██████╗░██████╗░░█████╗░░█████╗░███████╗░██████╗░██████╗
																				██╔══██╗██╔══██╗██╔══██╗██╔══██╗██╔════╝██╔════╝██╔════╝
																				██████╔╝██████╔╝██║░░██║██║░░╚═╝█████╗░░╚█████╗░╚█████╗░
																				██╔═══╝░██╔══██╗██║░░██║██║░░██╗██╔══╝░░░╚═══██╗░╚═══██╗
																				██║░░░░░██║░░██║╚█████╔╝╚█████╔╝███████╗██████╔╝██████╔╝
																				╚═╝░░░░░╚═╝░░╚═╝░╚════╝░░╚════╝░╚══════╝╚═════╝░╚═════╝░
*/
	void process(const ProcessArgs &args) override {

		barReached = false;
		sendReset = false;
		busyTracks = 0;
		recordedTracks = 0;
		receivedRunFromExp = 0;
		expConnected = 0;

		//sendRunToExp = 0;

		sendSsAllToExp = 0;

		if (params[PREROLL_BUT_PARAM].getValue())
			preRoll = true;
		else
			preRoll = false;

		lights[PREROLL_BUT_LIGHT].setBrightness(preRoll);


		// RECEIVING DATA FROM RIGHT
		if (rightExpander.module && rightExpander.module->model == modelSickoLooper1Exp) {
			
			Sl1ExpMsg *msgFromModule = (Sl1ExpMsg *)(rightExpander.consumerMessage);

			expConnected = msgFromModule->expConnected;

			if (msgFromModule->runSetting == 1)
				receivedRunFromExp = 1;
			else
				receivedRunFromExp = 0;

			if (msgFromModule->globalStatus == PLAYING) {
				if (globalStatus == IDLE) {
					if (preRoll) {
						globalStatus = PREROLLING;
						preRollCount = 0;
					} else {
						globalStatus = PLAYING;
					}

					loopCount = trackLoopMeas;
					
					runSetting = 1;
					sendRunToExp = 1;
				}
			}

			if (msgFromModule->recordedTracks)
				recordedTracks = msgFromModule->recordedTracks;

			if (msgFromModule->busyTracks)
				busyTracks = msgFromModule->busyTracks;

		}

		if (trackRecorded)
			recordedTracks++;

		if (busyTrack)
			busyTracks++;


		lights[CLOCK_RST_LIGHT].setBrightness(params[CLOCK_RST_SWITCH].getValue());

		lights[CLICK_BUT_LIGHT].setBrightness(params[CLICK_BUT_PARAM].getValue());


		lights[ALLSTARTSTOP_BUT_LIGHT].setBrightness(params[ALLSTARTSTOP_BUT_PARAM].getValue());

		// ***************************************************************************************************************************
		// ************************************************************************     L O A D     I N P U T S      *****************
		// ***************************************************************************************************************************
		
		inputValue[LEFT] = 0.f;
		inputValue[RIGHT] = 0.f;
		mute = params[MUTE_SWITCH].getValue();
		lights[MUTE_LIGHT].setBrightness(mute);
		if (mute != prevMute) {
			muteFade = true;
			if (mute){
				muteValue = 1;
				muteDelta = -1.f / minTimeSamples;
			} else {
				muteValue = 0;
				muteDelta = 1.f / minTimeSamples;
			}
			prevMute = mute;
		}

		if (muteFade) {
			muteValue += muteDelta;
			if (muteValue < 0) {
				muteValue = 0;
				muteFade = false;
			} else if (muteValue > 1) {
				muteValue = 1;
				muteFade = false;
			}
		}

		if (inputs[LEFT_INPUT].isConnected()) {
			inputValue[LEFT] = inputs[LEFT_INPUT].getVoltageSum() * params[SOURCELVL_KNOB_PARAM].getValue() * muteValue;
			if (inputValue[LEFT] > 10.f)
				inputValue[LEFT] = 10.f;
			else if (inputValue[LEFT] < -10.f)
				inputValue[LEFT] = -10.f;
			if (inputs[RIGHT_INPUT].isConnected()) {
				inputValue[RIGHT] = inputs[RIGHT_INPUT].getVoltageSum() * params[SOURCELVL_KNOB_PARAM].getValue() * muteValue;
				if (inputValue[RIGHT] > 10.f)
					inputValue[RIGHT] = 10.f;
				else if (inputValue[RIGHT] < -10.f)
					inputValue[RIGHT] = -10.f;
			} else
				inputValue[RIGHT] = inputValue[LEFT];
		} else {
			if (inputs[RIGHT_INPUT].isConnected()) {
				inputValue[RIGHT] = inputs[RIGHT_INPUT].getVoltageSum() * params[SOURCELVL_KNOB_PARAM].getValue() * muteValue;
				if (inputValue[RIGHT] > 10.f)
					inputValue[RIGHT] = 10.f;
				else if (inputValue[RIGHT] < -10.f)
					inputValue[RIGHT] = -10.f;
				inputValue[LEFT] = inputValue[RIGHT];
			}
		}

		// ********************************************************************************************* other initializations

		startNow = false;	// these are initialized here to be ready for next ALL PLAY STOP check
		stopNow = false;


//																						██████╗░██╗░░░░░░█████╗░██╗░░░██╗  ░█████╗░██╗░░░░░██╗░░░░░
//																						██╔══██╗██║░░░░░██╔══██╗╚██╗░██╔╝  ██╔══██╗██║░░░░░██║░░░░░
//																						██████╔╝██║░░░░░███████║░╚████╔╝░  ███████║██║░░░░░██║░░░░░
//																						██╔═══╝░██║░░░░░██╔══██║░░╚██╔╝░░  ██╔══██║██║░░░░░██║░░░░░
//																						██║░░░░░███████╗██║░░██║░░░██║░░░  ██║░░██║███████╗███████╗
//																						╚═╝░░░░░╚══════╝╚═╝░░╚═╝░░░╚═╝░░░  ╚═╝░░╚═╝╚══════╝╚══════╝

		if (inputs[ALLSTARTSTOP_TRIG_INPUT].getVoltage() > 1.f) {
			if (!inputs[ALLSTOP_TRIG_INPUT].isConnected()) {
				allSSTrig = 1;
			} else if (globalStatus == IDLE) {
				allSSTrig = 1;
			} else {
				allSSTrig = 0;
			}
		} else {
			allSSTrig = 0;
		}

		allSSTrig += params[ALLSTARTSTOP_BUT_PARAM].getValue();
		if (allSSTrig >= 1 && prevAllSSTrig == 0) {
			if (globalStatus == IDLE && recordedTracks > 0) {

				if (trackStatus != EMPTY) {

					nextStatus = PLAYING;
					busyTracks++;
					busyTrack = true;
					setFastPlayLed();
					loopCount = trackLoopMeas;

				}
				
				if (preRoll) {
					globalStatus = PREROLLING;
					preRollCount = 0;
				} else
					globalStatus = PLAYING;	

				runSetting = 1; // this starts clock
				sendRunToExp = 1;

				sendSsAllToExp = START_ALL;

				if (expConnected > 0) {
					waitingExpResp = true;
					lagBusyTracks = 0;
				}

			} else if (globalStatus == PLAYING) {

				switch (trackStatus) {
					case EMPTY:
					case IDLE:
					break;

					case PLAYING:
						setFastPlayLed();
						if (stopImm_setting)
							stopNow = true;
					break;

					case RECORDING:
						setFastRecLed();
						if (!loopSync_setting)
							stopNow = true;
					break;

					case OVERDUBBING:
						setFastOverdubLed();
						if (stopImm_setting)
							stopNow = true;
					break;
				}
				nextStatus = IDLE;

				sendSsAllToExp = STOP_ALL;

			}
			
		}
		prevAllSSTrig = allSSTrig;

		allStopTrig = inputs[ALLSTOP_TRIG_INPUT].getVoltage();
		if (allStopTrig > 1.f && prevAllStopTrig <= 1.f) {
			allStop = true;
		} else {
			allStop = false;
		}
		prevAllStopTrig = allStopTrig;

		if (allStop && globalStatus == PLAYING) {

			switch (trackStatus) {
				case EMPTY:
				case IDLE:
				break;

				case PLAYING:
					setFastPlayLed();
					if (stopImm_setting)
						stopNow = true;
				break;

				case RECORDING:
					setFastRecLed();
					if (!loopSync_setting)
						stopNow = true;
				break;

				case OVERDUBBING:
					setFastOverdubLed();
					if (stopImm_setting)
						stopNow = true;
				break;
			}
			nextStatus = IDLE;

			sendSsAllToExp = STOP_ALL;

		}

		// ***************************************************************************************************************************
		// **********************************************************************   T R A C K    M A N A G E M E N T   ***************
		// ***************************************************************************************************************************

		trackLoopMeas = int(params[MEAS_KNOB_PARAM].getValue());

		volTrack = params[VOLTRACK_KNOB_PARAM].getValue() + (inputs[VOLTRACK_INPUT].getVoltage() / 10.f);
		if (volTrack > 1)
			volTrack = 1;
		else if (volTrack < 0)
			volTrack = 0;

		play_but = params[PLAY_BUT_PARAM].getValue();

		rec_but = params[REC_BUT_PARAM].getValue();

		stop_but = params[STOP_BUT_PARAM].getValue();
		lights[STOP_BUT_LIGHT].setBrightness(stop_but);

		erase_but = params[ERASE_BUT_PARAM].getValue();
		lights[ERASE_BUT_LIGHT].setBrightness(erase_but);	

		loopSync_setting = params[LOOPSYNC_SWITCH].getValue();
		lights[LOOPSYNC_LIGHT].setBrightness(loopSync_setting);

		startImm_setting = params[STARTIMM_SWITCH].getValue();
		lights[STARTIMM_LIGHT].setBrightness(startImm_setting);

		stopImm_setting = params[STOPIMM_SWITCH].getValue();
		lights[STOPIMM_LIGHT].setBrightness(stopImm_setting);

		oneShot_setting = params[ONESHOT_SWITCH].getValue();
		lights[ONESHOT_LIGHT].setBrightness(oneShot_setting);

		revTrig = inputs[REV_INPUT].getVoltage();
		if (revTrig >= 1.f && prevRevTrig < 1.f) {
			params[REV_SWITCH].setValue(!params[REV_SWITCH].getValue());
		}
		prevRevTrig = revTrig;
		rev_setting = params[REV_SWITCH].getValue();

		lights[REV_LIGHT].setBrightness(rev_setting);

		srcToTrack = params[SRC_TO_TRACK_SWITCH].getValue();
		lights[SRC_TO_TRACK_LIGHT].setBrightness(srcToTrack);

		if (srcToTrack != prevSrcToTrack) {
			srcToTrackFade = true;
			if (srcToTrack){
				srcToTrackValue = 0;
				srcToTrackDelta = 1.f / minTimeSamples;
			} else {
				srcToTrackValue = 1;
				srcToTrackDelta = -1.f / minTimeSamples;
			}
			prevSrcToTrack = srcToTrack;
		}

		if (srcToTrackFade) {
			srcToTrackValue += srcToTrackDelta;
			if (srcToTrackValue < 0) {
				srcToTrackValue = 0;
				srcToTrackFade = false;
			} else if (srcToTrackValue > 1) {
				srcToTrackValue = 1;
				srcToTrackFade = false;
			}
		}
		
		if (inputs[PLAY_TRIG_INPUT].getVoltage() > 1 || play_but)
			playTrig = 1;
		else
			playTrig = 0;

		if (inputs[REC_TRIG_INPUT].getVoltage() > 1 || rec_but)
			recTrig = 1;
		else
			recTrig = 0;

		if (inputs[STOP_TRIG_INPUT].getVoltage() > 1 || stop_but)
			stopTrig = 1;
		else
			stopTrig = 0;

		if (inputs[ERASE_TRIG_INPUT].getVoltage() > 1) {
			eraseTrig = 1;
			eraseWait = true;
			eraseTime = 0;
		} else if (erase_but) {
			eraseTrig = 1;
			if (eraseWait == false) {
				eraseWait = true;
				eraseTime = eraseSamples;
			}
		} else
			eraseTrig = 0;			

		// *****************************************************  CHECK TRIGGERS


//																		███████╗██████╗░░█████╗░░██████╗███████╗  ████████╗██████╗░██╗░██████╗░
//																		██╔════╝██╔══██╗██╔══██╗██╔════╝██╔════╝  ╚══██╔══╝██╔══██╗██║██╔════╝░
//																		█████╗░░██████╔╝███████║╚█████╗░█████╗░░  ░░░██║░░░██████╔╝██║██║░░██╗░
//																		██╔══╝░░██╔══██╗██╔══██║░╚═══██╗██╔══╝░░  ░░░██║░░░██╔══██╗██║██║░░╚██╗
//																		███████╗██║░░██║██║░░██║██████╔╝███████╗  ░░░██║░░░██║░░██║██║╚██████╔╝
//																		╚══════╝╚═╝░░╚═╝╚═╝░░╚═╝╚═════╝░╚══════╝  ░░░╚═╝░░░╚═╝░░╚═╝╚═╝░╚═════╝░			

		

		if (eraseTrig >= 1 && prevEraseTrig < 1) {	// ***** ERASE TRIG

			if (trackStatus == IDLE && eraseWait && eraseTime != eraseSamples) {
				eraseWait = false;
				trackBuffer[LEFT].resize(0);
				trackBuffer[RIGHT].resize(0);
				totalSamples = 0;
				totalSampleC = 0;
				trackStatus = EMPTY;
				nextStatus = NOTHING;
				trackRecorded = false;
				recordedTracks--;
				lights[PLAY_BUT_LIGHT].setBrightness(0.f);
			}
		}
		
		if (eraseWait) {
			eraseTime--;
			if (eraseTime < 0)
				eraseWait = false;
		}



//																					░██████╗████████╗░█████╗░██████╗░  ████████╗██████╗░██╗░██████╗░
//																					██╔════╝╚══██╔══╝██╔══██╗██╔══██╗  ╚══██╔══╝██╔══██╗██║██╔════╝░
//																					╚█████╗░░░░██║░░░██║░░██║██████╔╝  ░░░██║░░░██████╔╝██║██║░░██╗░
//																					░╚═══██╗░░░██║░░░██║░░██║██╔═══╝░  ░░░██║░░░██╔══██╗██║██║░░╚██╗
//																					██████╔╝░░░██║░░░╚█████╔╝██║░░░░░  ░░░██║░░░██║░░██║██║╚██████╔╝
//																					╚═════╝░░░░╚═╝░░░░╚════╝░╚═╝░░░░░  ░░░╚═╝░░░╚═╝░░╚═╝╚═╝░╚═════╝░

		if (stopTrig >= 1 && prevStopTrig < 1) {	// ***** STOP TRIG

			switch (trackStatus) {

				case IDLE:
				case EMPTY:
					if (nextStatus != NOTHING) {
						nextStatus = NOTHING;
						resetIdleEmptyStatus();
						busyTracks--;
						busyTrack = false;
					}
				break;

				case PLAYING:
					if (instantStop) {
						nextStatus = IDLE;
						stopNow = true;
					} else {
						if (nextStatus != IDLE) {
							nextStatus = IDLE;
							setFastPlayLed();

							if (stopImm_setting)
								stopNow = true;
						} else {	// undo
							nextStatus = NOTHING;
							setPlayLed();
						}
					}
				break;

				case RECORDING:
					if (nextStatus != IDLE) {	// undo stop recording
						nextStatus = IDLE;
						if (loopSync_setting)
							setFastRecLed();
						else
							stopNow = true;

					} else {	// undo
						nextStatus = NOTHING;
						setRecLed();
						}
				break;

				case OVERDUBBING:
					if (instantStop) {
						nextStatus = IDLE;
						stopNow = true;
					} else {
						if (nextStatus != IDLE) {

							nextStatus = IDLE;

							if (loopSync_setting)
								setFastOverdubLed();
							else
								stopNow = true;

							if (stopImm_setting)
								stopNow = true;

						} else { // undo
							nextStatus = NOTHING;
							setOverdubLed();
						}
					}
				break;
			}
		}

//
//																					██████╗░██╗░░░░░░█████╗░██╗░░░██╗  ████████╗██████╗░██╗░██████╗░
//																					██╔══██╗██║░░░░░██╔══██╗╚██╗░██╔╝  ╚══██╔══╝██╔══██╗██║██╔════╝░
//																					██████╔╝██║░░░░░███████║░╚████╔╝░  ░░░██║░░░██████╔╝██║██║░░██╗░
//																					██╔═══╝░██║░░░░░██╔══██║░░╚██╔╝░░  ░░░██║░░░██╔══██╗██║██║░░╚██╗
//																					██║░░░░░███████╗██║░░██║░░░██║░░░  ░░░██║░░░██║░░██║██║╚██████╔╝
//																					╚═╝░░░░░╚══════╝╚═╝░░╚═╝░░░╚═╝░░░  ░░░╚═╝░░░╚═╝░░╚═╝╚═╝░╚═════╝░
		
		if (playTrig >= 1 && prevPlayTrig < 1) {	// ******* PLAY TRIG

			switch (trackStatus) {
				case EMPTY:											// play press & track is EMPTY -> do nothing
					if (playSequence != PLAY_STOP)
						recTrig = 1;
				break;

				case IDLE:
					
					if (playSequence == REC_OVERDUB_PLAY) {
						recTrig = 1;

					} else {

						if (globalStatus == IDLE) {						// play press & globalStatus is IDLE

							if (preRoll) {
								globalStatus = PREROLLING;
								preRollCount = 0;
							} else {
								globalStatus = PLAYING;
							}

							loopCount = trackLoopMeas;
							
							runSetting = 1;
							sendRunToExp = 1;
							busyTracks++;
							busyTrack = true;
							nextStatus = PLAYING;

							if (!loopSync_setting) {
								if (!rev_setting) {
									playingDirection = FORWARD;
									samplePos = 0;
								} else {
									playingDirection = REVERSE;
									samplePos = totalSamples;
								}
							}
							
							setFastPlayLed();

						} else if (globalStatus == PREROLLING) {		// play press & globalStatus is PREROLLING
							
							if (nextStatus == NOTHING) {	// undo

								busyTracks++;
								busyTrack = true;

								if (!loopSync_setting) {
									if (!rev_setting) {
										playingDirection = FORWARD;
										samplePos = 0;
									} else {
										playingDirection = REVERSE;
										samplePos = totalSamples;
									}
								}

								nextStatus = PLAYING;
								loopCount = trackLoopMeas;

								setFastPlayLed();

							} else {	// undo or switch from overdub to play

								if (nextStatus == PLAYING) {
									nextStatus = NOTHING;
									busyTracks--;
									busyTrack = false;
									setIdleLed();
								} else {
									nextStatus = PLAYING;
									setFastPlayLed();
								}
							}
							
						} else {

							if (nextStatus == NOTHING) {		// play press & track is IDLE -> go PLAYING
								
								loopCount = trackLoopMeas;

								busyTracks++;
								busyTrack = true;
								nextStatus = PLAYING;

								if (startImm_setting) {
									startNow = true;
									loopCount = 1;
								}

								if (!loopSync_setting) {
									if (!rev_setting) {
										playingDirection = FORWARD;
										samplePos = 0;
									} else {
										playingDirection = REVERSE;
										samplePos = totalSamples;
									}
								}

								setFastPlayLed();

								
							} else {	// undo play press

								if (nextStatus == PLAYING) {
									busyTracks--;
									busyTrack = false;
									
									resetIdleEmptyStatus();

								} else {	// se il nextStatus è OVERDUBBING passa a PLAYING
									nextStatus = PLAYING;
									setFastPlayLed();
								}
							}
						}
					}
				break;

				case PLAYING:							// play press & track is PLAYING -> go IDLE
					if (playSequence == PLAY_STOP) {

						if (nextStatus != IDLE) {
							nextStatus = IDLE;
							
							if (stopImm_setting)
								stopNow = true;

							setFastPlayLed();

						} else {	// undo play press
							nextStatus = NOTHING;

							setPlayLed();
							
						}
					} else {
						recTrig = 1;
					}
				break;

				case RECORDING:						// play press & track is RECORDING -> go PLAYING

					if (playSequence == PLAY_STOP) {
						if (nextStatus != PLAYING) {
							
							nextStatus = PLAYING;
							
							if (!loopSync_setting)					// if not loopSync
								stopNow = true;

							setRecToPlayLed();
						} else {	// undo play press

							nextStatus = NOTHING;

							setRecLed();
						}
					} else {
						recTrig = 1;
					}

				break;

				case OVERDUBBING:				// play press & track is OVERDUBBING -> go PLAYING

					if (playSequence == PLAY_STOP) {

						if (nextStatus != PLAYING) {
							nextStatus = PLAYING;

							if (startImm_setting)
								startNow = true;

							setRecToPlayLed();
						} else {	// undo play press

							nextStatus = NOTHING;
							setOverdubLed();
						}
					} else {
						recTrig = 1;
					}
				break;
			}

		}

//
//																						██████╗░███████╗░█████╗░  ████████╗██████╗░██╗░██████╗░
//																						██╔══██╗██╔════╝██╔══██╗  ╚══██╔══╝██╔══██╗██║██╔════╝░
//																						██████╔╝█████╗░░██║░░╚═╝  ░░░██║░░░██████╔╝██║██║░░██╗░
//																						██╔══██╗██╔══╝░░██║░░██╗  ░░░██║░░░██╔══██╗██║██║░░╚██╗
//																						██║░░██║███████╗╚█████╔╝  ░░░██║░░░██║░░██║██║╚██████╔╝
//																						╚═╝░░╚═╝╚══════╝░╚════╝░  ░░░╚═╝░░░╚═╝░░╚═╝╚═╝░╚═════╝░

		if (recTrig >= 1 && prevRecTrig < 1) {	// ***** REC TRIG

			if (globalStatus == IDLE) {									// rec press & globalStatus is IDLE
				
				runSetting = 1;
				sendRunToExp = 1;
				busyTracks++;
				busyTrack = true;

				if (preRoll) {
					globalStatus = PREROLLING;
					preRollCount = 0;
				} else {
					globalStatus = PLAYING;
				}

				if (trackStatus == EMPTY) {
					nextStatus = RECORDING;
					setFastRecLed();
				} else {
					nextStatus = OVERDUBBING;
					setFastOverdubLed();
				}

				loopCount = trackLoopMeas;

				if (!loopSync_setting) {
					if (!rev_setting) {
						playingDirection = FORWARD;
						samplePos = 0;
					} else {
						playingDirection = REVERSE;
						samplePos = totalSamples;
					}
				}

			} else if (globalStatus == PREROLLING) {				// rec press & globalStatus is PREROLLING

				if (nextStatus == NOTHING ) {

					busyTracks++;
					busyTrack = true;

					if (trackStatus == EMPTY) {
						nextStatus = RECORDING;
						setFastRecLed();
					} else {
						nextStatus = OVERDUBBING;
						setFastOverdubLed();
					}

					if (!loopSync_setting) {
						if (!rev_setting) {
							playingDirection = FORWARD;
							samplePos = 0;
						} else {
							playingDirection = REVERSE;
							samplePos = totalSamples;
						}
					}

					loopCount = trackLoopMeas;

				
				} else {	// undo
					
					if (nextStatus == PLAYING) {

						nextStatus = OVERDUBBING;
						setFastOverdubLed();
					} else {

						if (nextStatus == RECORDING)
							setEmptyLed();
						else
							setIdleLed();

						nextStatus = NOTHING;
						busyTracks--;
						busyTrack = false;

					}
				}
		
			} else {

				switch (trackStatus) {
					case EMPTY:									// rec press & track is empty -> go recording

						if (nextStatus == NOTHING) {

							loopCount = trackLoopMeas;

							nextStatus = RECORDING;
							busyTracks++;
							busyTrack = true;
							setFastRecLed();
						} else {	// UNDO REC PRESS
							nextStatus = NOTHING;
							busyTracks--;
							busyTrack = false;

							setEmptyLed();
						}
					break;

					case IDLE:										// rec press & track is IDLE -> go overdubbinig
						if (nextStatus == NOTHING) {

							loopCount = trackLoopMeas;

							if (startImm_setting) {
								startNow = true;
								loopCount = 1;
							}
							nextStatus = OVERDUBBING;
							busyTracks++;
							busyTrack = true;

							if (!loopSync_setting) {
								if (!rev_setting) {
									playingDirection = FORWARD;
									samplePos = 0;
								} else {
									playingDirection = REVERSE;
									samplePos = totalSamples;
								}
							}

							setFastOverdubLed();

						} else { // UNDO REC PRESS

							if (nextStatus == OVERDUBBING) {
								nextStatus = NOTHING;
								busyTracks--;
								busyTrack = false;

								setIdleLed();
							} else {	// se il nextStatus è PLAYING vai a OVERDUBBING
								nextStatus = OVERDUBBING;
								setFastOverdubLed();
							}
						}
					break;

					case PLAYING:									// rec press & track is PLAYING -> go OVERDUBBING
						if (nextStatus != OVERDUBBING) {
							
							nextStatus = OVERDUBBING;
							
							if (startImm_setting)
								startNow = true;

							setPlayToRecLed();

						} else {	// UNDO REC PRESS

							nextStatus = NOTHING;
							setPlayLed();

						}
					break;

					case RECORDING:									// rec press & track is RECORDING -> go PLAYING
						if (loopSync_setting) {
							if (nextStatus != PLAYING) {
								nextStatus = PLAYING;
								setRecToPlayLed();
							} else {
								nextStatus = NOTHING;
								setRecLed();
							}
						} else {

							nextStatus = OVERDUBBING;	// if not loopSync then go OVERDUBBING instead of PLAYING
							stopNow = true;
						}
					break;

					case OVERDUBBING:								// rec press & track is OVERDUBBING -> go PLAYINIG
						if (nextStatus != PLAYING) {

							nextStatus = PLAYING;

							if (stopImm_setting)	// premendo rec in overdubbing con lo stopImm_setting fa partire subito il play
								startNow = true;	// (se invece si preme play in overdubbing partirà il play con lo startImm_setting)

							setRecToPlayLed();
						} else {	// UNDO REC PRESS

							nextStatus = NOTHING;
							setOverdubLed();
						}
					break;
				}

			}

		}

		prevPlayTrig = playTrig;
		prevRecTrig = recTrig;
		prevStopTrig = stopTrig;
		prevEraseTrig = eraseTrig;



//
//																								░█████╗░██╗░░░░░░█████╗░░█████╗░██╗░░██╗
//																								██╔══██╗██║░░░░░██╔══██╗██╔══██╗██║░██╔╝
//																								██║░░╚═╝██║░░░░░██║░░██║██║░░╚═╝█████═╝░
//																								██║░░██╗██║░░░░░██║░░██║██║░░██╗██╔═██╗░
//																								╚█████╔╝███████╗╚█████╔╝╚█████╔╝██║░╚██╗
//																								░╚════╝░╚══════╝░╚════╝░░╚════╝░╚═╝░░╚═╝


		{
			clickOutput = 0.f;

			lights[CLICK_BUT_LIGHT].setBrightness(params[CLICK_BUT_PARAM].getValue());

			// ********* EXTERNAL CONNECTION

			extConn = inputs[EXTCLOCK_INPUT].isConnected();
			if (extConn && !prevExtConn) {
				extSync = false;
				bpm = 0.0;
			}
			prevExtConn = extConn;

			// ************************* INTERNAL CLOCK

			if (!extConn) {

				// Reset
				rstBarBut = params[CLOCK_RST_SWITCH].getValue();

				rstIn = inputs[CLOCKRESET_INPUT].getVoltage();
				if (rstIn >= 1.f && prevRstIn < 1.f) {
					rstBarBut = 1.f;
				}
				prevRstIn = rstIn;

				if (rstBarBut >= 1.f && prevRstBarBut < 1.f) {
					beatCounter = currentBeatMaxPerBar;
					rstPulse = true;
					rstPulseTime = oneMsSamples;
					resetStart = true;
					sendReset = true;
					if (trackRecorded) {
						if (!rev_setting)
							samplePos = 0;
						else
							samplePos = totalSamples;
					}
				}
				prevRstBarBut = rstBarBut;


				// *********** BPM CALC
				
				bpm = (double)params[BPM_KNOB_PARAM].getValue()/10;

				if (bpm > 999)
					bpm = 999;

				// **************   RUN PROCESS   ***************

				if (runSetting) {
					
					currentBeatMaxPerBar = beatMaxPerBar[int(params[SIGNATURE_KNOB_PARAM].getValue())];
					
					// ***********  MID BEAT PULSES WHEN USING TEMPOS WITH EIGHTH NOTES
				
					if (params[SIGNATURE_KNOB_PARAM].getValue() > 5 && !midBeatPlayed && clockSample > midBeatMaxSample)  {
						beatCounter++;
						if (beatCounter > currentBeatMaxPerBar) {
							barReached = true;
							beatCounter = 1;
							clickSamplePos[BAR] = 0;
							clickPlay[BAR] = true;
							clickPlay[BEAT] = false;
						} else {
							clickSamplePos[BEAT] = 0;
							clickPlay[BEAT] = true;
							clickPlay[BAR] = false;
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

						if (beatCounter > currentBeatMaxPerBar) {

							barReached = true;
							beatCounter = 1;
							clickSamplePos[BAR] = 0;
							clickPlay[BAR] = true;
							clickPlay[BEAT] = false;

							barSample = -1;

						} else {
							clickSamplePos[BEAT] = 0;
							clickPlay[BEAT] = true;
							clickPlay[BAR] = false;
						}
						
						clockPulse = true;
						clockPulseTime = oneMsSamples;
						
					}

					clockSample++;
					//barSample++;
					barSample += sampleCoeff;
				}

			} else {

				// ************************************************ EXTERNAL CLOCK

				// Reset
				/*
				rstBarBut = params[CLOCK_RST_SWITCH].getValue();

				rstIn = inputs[CLOCKRESET_INPUT].getVoltage();
				if (rstIn >= 1.f && prevRstIn < 1.f) {
					rstBarBut = 1.f;
				}
				prevRstIn = rstIn;

				if (rstBarBut >= 1.f && prevRstBarBut < 1.f) {
					beatCounter = currentBeatMaxPerBar;
					rstPulse = true;
					rstPulseTime = oneMsSamples;
					sendReset = true;
				}
				prevRstBarBut = rstBarBut;
				*/

				rstBarBut = params[CLOCK_RST_SWITCH].getValue();

				rstIn = inputs[CLOCKRESET_INPUT].getVoltage();
				if (rstIn >= 1.f && prevRstIn < 1.f) {
					rstBarBut = 1.f;
				}
				prevRstIn = rstIn;

				if (rstBarBut >= 1.f && prevRstBarBut < 1.f) {
					beatCounter = currentBeatMaxPerBar;
					rstPulse = true;
					rstPulseTime = oneMsSamples;
					if (rstIn >= 1.f)
						sendReset = true;
				}
				prevRstBarBut = rstBarBut;


				// ********** EXTERNAL SYNC

				extTrigValue = inputs[EXTCLOCK_INPUT].getVoltage();
					
				if (extTrigValue >= 1 && prevExtTrigValue < 1) {

					if (extSync) {

						clockMaxSample = clockSample;
						midBeatMaxSample = clockMaxSample / 2;
						
						clockSample = 1.0;

						// calculate bpms
						bpm = round(sampleRateCoeff / clockMaxSample);
						if (bpm > 999)
							bpm = 999;

					} else {
						bpm = 0.0;
						extSync = true;
						clockSample = 1.0;
						runSetting = 1;
						sendRunToExp = 1;
					}

					if (runSetting)
						extBeat = true;

				} else {
					extBeat = false;
				}
				prevExtTrigValue = extTrigValue;

				// **************   RUN PROCESS   ***************

				if (runSetting) {

					currentBeatMaxPerBar = beatMaxPerBar[int(params[SIGNATURE_KNOB_PARAM].getValue())];

					// ***********  MID BEAT PULSES WHEN USING TEMPOS WITH EIGHTH NOTES

					if (params[SIGNATURE_KNOB_PARAM].getValue() > 5 && !midBeatPlayed && clockSample > midBeatMaxSample)  {
						beatCounter++;
						if (beatCounter > currentBeatMaxPerBar) {
							barReached = true;
							beatCounter = 1;
							clickSamplePos[BAR] = 0;
							clickPlay[BAR] = true;
							clickPlay[BEAT] = false;
						} else {
							clickSamplePos[BEAT] = 0;
							clickPlay[BEAT] = true;
							clickPlay[BAR] = false;
						}
						midBeatPlayed = true;
					}

					// ************************ EXTERNAL CLOCK ******************

					if (extBeat) {

						midBeatPlayed = false;
						beatCounter++;

						if (extSync) {

							// ********** SYNCED BEAT

							if (beatCounter > currentBeatMaxPerBar) {
								// ***** BAR DETECTED *****
								barReached = true;
								beatCounter = 1;
								clickSamplePos[BAR] = 0;
								clickPlay[BAR] = true;
								clickPlay[BEAT] = false;

								barSample = -1;

							} else {
								// ***** BEAT DETECTED *****
								clickSamplePos[BEAT] = 0;
								clickPlay[BEAT] = true;
								clickPlay[BAR] = false;
							}
							
							clockPulse = true;
							clockPulseTime = oneMsSamples;
						
						} else {

							//	************ UNSYNCED BEAT
						
							beatCounter++;
							clickSamplePos[BEAT] = 0;
							clickPlay[BEAT] = true;
							clickPlay[BAR] = false;
						}
					}
				}
					
				clockSample++;
				//barSample++;
				barSample += sampleCoeff;
			}

			// ***************************** COMMON PROCESS

			//	************ AUDIO CLICK

			if (params[CLICK_BUT_PARAM].getValue()) {
				for (int i = 0; i < 2; i++) {
					if (clickFileLoaded[i] && clickPlay[i] && floor(clickSamplePos[i]) < clickTotalSampleC[i]) {
						clickOutput = clickPlayBuffer[i][clickSamplePos[i]] * params[CLICKVOL_KNOB_PARAM].getValue();
						clickSamplePos[i]++;
					} else {
						clickPlay[i] = false;
					}
				}
			} else {
				clickPlay[BEAT] = false;
				clickPlay[BAR] = false;
			}
			
			//	********** CLOCK  PULSE
			if (clockPulse) {
				clockPulseTime--;

				if (clockPulseTime < 0) {
					clockPulse = false;
					outputs[CLOCK_OUTPUT].setVoltage(0.f);
				} else
					outputs[CLOCK_OUTPUT].setVoltage(10.f);

			}
			
		}


		// **********************************************************************************************************************
		// **********************************************************************************************************************


		// ************* PREROLL CHECK ***************

		if (globalStatus == PREROLLING) {
			if (barReached) {
				preRollCount++;
				if (preRollCount > (int)params[PREROLL_SWITCH].getValue()) {
					globalStatus = PLAYING;
					preRollCount = 0;
				}
			}
		}


//																	██╗░░░░░░█████╗░░█████╗░██████╗░  ░██████╗████████╗░█████╗░██████╗░████████╗
//																	██║░░░░░██╔══██╗██╔══██╗██╔══██╗  ██╔════╝╚══██╔══╝██╔══██╗██╔══██╗╚══██╔══╝
//																	██║░░░░░██║░░██║██║░░██║██████╔╝  ╚█████╗░░░░██║░░░███████║██████╔╝░░░██║░░░
//																	██║░░░░░██║░░██║██║░░██║██╔═══╝░  ░╚═══██╗░░░██║░░░██╔══██║██╔══██╗░░░██║░░░
//																	███████╗╚█████╔╝╚█████╔╝██║░░░░░  ██████╔╝░░░██║░░░██║░░██║██║░░██║░░░██║░░░
//																	╚══════╝░╚════╝░░╚════╝░╚═╝░░░░░  ╚═════╝░░░░╚═╝░░░╚═╝░░╚═╝╚═╝░░╚═╝░░░╚═╝░░░




		// **********************************************************************************************************************
		// ************************  LOOP  START CHECK  ************  AKA  ************* NEXT STATUS  CHECK *********************
		// **********************************************************************************************************************
		
		barChecked = false;

		if (globalStatus == PLAYING) {

			switch (nextStatus) {

				case NOTHING:
				case IDLE:
				break;

				case PLAYING:						// NEXT STATUS PLAY

					if (trackStatus != RECORDING) {
						if (barReached) {
							loopCount++;
							barChecked = true;
						}

						if (loopSync_setting && startNow) {				// ********** if loopSync && start now

							if (trackStatus == IDLE) {
								if (!rev_setting) {
									playingDirection = FORWARD;
									samplePos = barSample;
								} else {
									playingDirection = REVERSE;
									samplePos = totalSamples - barSample;
								}

								fadeInValue = 0.f;
								fadeIn = true;
								fadeInDelta = 1.f / minTimeSamples;
								
							} else {		// if it's OVERDUBBING & startNow -> rec fade out
								// rec fade out
								recFade = true;
								recFadeValue = 1.f;
								recFadeDelta = -1 / minTimeSamples;

								extraRecording = true;
								extraRecDirection = playingDirection;
								extraRecCount = 0;
								extraRecMaxSamples = minTimeSamplesOvs;
								extraRecPos = samplePos;

							}

							nextStatus = NOTHING;
							trackStatus = PLAYING;

							setPlayLed();

						} else if (loopSync_setting && loopCount > trackLoopMeas) {	// if loopSync && end of loop reached

							if (trackStatus == OVERDUBBING && playingDirection == FORWARD) {

								extraRecording = true;
								extraSamples = true;
								extraRecDirection = playingDirection;
								extraRecCount = 0;
								extraRecMaxSamples = tailSamples;
								extraRecPos = samplePos;

								xFadePlay();
							} else {

								if (fadeInOnPlay) {
									fadeInValue = 0.f;
									fadeIn = true;
									fadeInDelta = 1000 / (params[XFADE_KNOB_PARAM].getValue() * sampleRate);
								}
							}

							loopCount = 1;

							if (!rev_setting) {
								playingDirection = FORWARD;
								samplePos = 0;
							} else {
								playingDirection = REVERSE;
								samplePos = totalSamples;
							}

							nextStatus = NOTHING;
							trackStatus = PLAYING;

							setPlayLed();
						
						} else if (!loopSync_setting) {	// if NOT loopSync

							if (trackStatus == IDLE) {
								nextStatus = NOTHING;
								trackStatus = PLAYING;

								if (fadeInOnPlay) {
									fadeInValue = 0.f;
									fadeIn = true;
									fadeInDelta = 1000 / (params[XFADE_KNOB_PARAM].getValue() * sampleRate);
								}

								setPlayLed();

							} else if (startNow) {	// if trackStatus is OVERDUBBING and startNow

								if (trackStatus == OVERDUBBING) {
									recFade = true;					// rec fade out
									recFadeValue = 1.f;
									recFadeDelta = -1 / minTimeSamples;

									extraRecording = true;
									extraRecDirection = playingDirection;
									extraRecCount = 0;
									extraRecMaxSamples = minTimeSamplesOvs;
									extraRecPos = samplePos;

								}

								trackStatus = PLAYING;
								nextStatus = NOTHING;
								setPlayLed();
							}
						}
					}

				break;


				case RECORDING:

					if (barReached) {
						loopCount++;
						barChecked = true;
					}

					if ((loopSync_setting && barReached) || !loopSync_setting) {
						loopCount = 1;
						playingDirection = FORWARD;
						samplePos = 0;

						trackRecorded = true;

						trackStatus = RECORDING;
						nextStatus = NOTHING;
						recFadeValue = 1.f;

						setRecLed();
					}
				
				break;
				
				case OVERDUBBING:

					if (barReached) {
						loopCount++;
						barChecked = true;
					}

					if (loopSync_setting && startNow) {				// ********** if loopSync && start now

						if (trackStatus == IDLE) {
							if (!rev_setting) {
								playingDirection = FORWARD;
								samplePos = barSample;
							} else {
								playingDirection = REVERSE;
								samplePos = totalSamples - barSample;
							}

							fadeInValue = 0.f;
							fadeIn = true;
							fadeInDelta = 1.f / minTimeSamples;
						}

						recFade = true;
						recFadeValue = 0.f;
						recFadeDelta = 1.f / minTimeSamples;

						trackStatus = OVERDUBBING;
						nextStatus = NOTHING;

						setOverdubLed();
						
					} else if (loopSync_setting && loopCount > trackLoopMeas) {	// if loopSync && end of loop reached

						loopCount = 1;

						eolPulse = true;
						eolPulseTime = oneMsSamples;		// eol pulse when an overdubbing is starting

						if (!rev_setting) {
							if (trackStatus == PLAYING) 
								xFadePlay();
							samplePos = 0;
							playingDirection = FORWARD;
						} else {
							samplePos = totalSamples;
							playingDirection = REVERSE;
						}

						trackStatus = OVERDUBBING;
						nextStatus = NOTHING;

						setOverdubLed();
						
					} else if (!loopSync_setting) {	// if NOT loopSync
						
						if (trackStatus == IDLE) {
							trackStatus = OVERDUBBING;
							nextStatus = NOTHING;

							if (fadeInOnPlay) {
								fadeInValue = 0.f;
								fadeIn = true;
								fadeInDelta = 1000 / (params[XFADE_KNOB_PARAM].getValue() * sampleRate);
							}

							setOverdubLed();

						} else if (startNow) {	// if it's PLAYING & startNow

							trackStatus = OVERDUBBING;
							nextStatus = NOTHING;

							// recFadeIn
							recFade = true;
							recFadeValue = 0.f;
							recFadeDelta = 1 / minTimeSamples;

							setOverdubLed();
						}
					}
				break;

			}
		}
	
		// **********************************************************************************************************
		// **************  LOOP  END CHECK  ************  AKA  ********* TRACK STATUS CHECK *************************
		// **********************************************************************************************************


//																		██╗░░░░░░█████╗░░█████╗░██████╗░  ███████╗███╗░░██╗██████╗░
//																		██║░░░░░██╔══██╗██╔══██╗██╔══██╗  ██╔════╝████╗░██║██╔══██╗
//																		██║░░░░░██║░░██║██║░░██║██████╔╝  █████╗░░██╔██╗██║██║░░██║
//																		██║░░░░░██║░░██║██║░░██║██╔═══╝░  ██╔══╝░░██║╚████║██║░░██║
//																		███████╗╚█████╔╝╚█████╔╝██║░░░░░  ███████╗██║░╚███║██████╔╝
//																		╚══════╝░╚════╝░░╚════╝░╚═╝░░░░░  ╚══════╝╚═╝░░╚══╝╚═════╝░


		loopEnd = false;  //   initialize loopEnd seeking

		if (trackStatus != EMPTY && trackStatus != IDLE) {

			if (stopNow) {

				loopEnd = true;
				if (!eolPulseOnStop)
					notEolPulse = true;

			} else {

				if (loopSync_setting) {

					if (barReached) {
						if (!barChecked)
							loopCount++;
						if (loopCount > trackLoopMeas) {
							loopEnd = true;
							loopCount = 1;
						}
					}

				} else {	// if not loop sync

					switch (trackStatus) {
						case IDLE:
							switch (nextStatus) {
								case PLAYING:
								case OVERDUBBING:
									loopEnd = true;
								break;

							}
						break;

						case PLAYING:
						case OVERDUBBING:
							if ((samplePos > totalSamples && playingDirection == FORWARD) ||
								(samplePos < 0 && playingDirection == REVERSE))
									loopEnd = true;
						break;
					}
				}
			}

			if (loopEnd) {

				loopEnd = false;

				if (!notEolPulse) {
					eolPulse = true;
					eolPulseTime = oneMsSamples;	
				} else {
					notEolPulse = false;
				}

				switch (trackStatus) {

					case PLAYING:

						if (oneShot_setting && !stopNow) {

							if (extraPlayingFadeOut)
								extraPlayingFadeOut = false;
							else
								xFadePlay();

							if (nextStatus == OVERDUBBING) {
								trackStatus = OVERDUBBING;
								nextStatus = NOTHING;

								if (!rev_setting) { 
									playingDirection = FORWARD;
									samplePos = 0;
								} else {
									playingDirection = REVERSE;
									samplePos = totalSamples;
								}

								setOverdubLed();
							} else {
								busyTracks--;
								busyTrack = false;
								trackStatus = IDLE;
								nextStatus = NOTHING;

								if (!rev_setting && playFullTail) {
									playTail = true;
									tailEnd = samplePos + tailSamples;
								}

								setIdleLed();
							}
							
						} else {
							switch (nextStatus) {
								case IDLE:
									busyTracks--;
									busyTrack = false;
									trackStatus = IDLE;
									nextStatus = NOTHING;

									setIdleLed();

									if (!rev_setting && playFullTail && !stopNow) {
										playTail = true;
										tailEnd = samplePos + tailSamples;
									}
								break;

								case OVERDUBBING:
									trackStatus = OVERDUBBING;
									nextStatus = NOTHING;

									setOverdubLed();
								break;
							}

							if (!rev_setting) { 
								playingDirection = FORWARD;
								if (extraPlayingFadeOut)
									extraPlayingFadeOut = false;
								else
									xFadePlay();

								samplePos = 0;
							} else {
								playingDirection = REVERSE;
								if (stopNow) {
									if (extraPlayingFadeOut)
										extraPlayingFadeOut = false;
									else
										xFadePlay();
								}
								samplePos = totalSamples;
							}
						}
				
					break;

					case RECORDING:
						//totalSampleC = samplePos - 1;
						//totalSamples = totalSampleC - 1;
						totalSampleC = samplePos - 2;
						totalSamples = totalSampleC - 1;
						extraRecording = true;
						extraSamples = true;
						extraRecDirection = FORWARD;
						extraRecCount = 0;
						extraRecMaxSamples = tailSamples;
						extraRecPos = samplePos;

						if (!loopSync_setting && recordedTracks == 1) {
							detectTempo();
							if (!extConn) {
								clockSample = 1.0;
								resetStart = true;
								beatCounter = 20;
							}
						}
						
						switch (nextStatus) {
							case IDLE:
								busyTracks--;
								busyTrack = false;
								trackStatus = IDLE;
								nextStatus = NOTHING;

								setIdleLed();
							break;

							case PLAYING:
								trackStatus = PLAYING;
								nextStatus = NOTHING;

								setPlayLed();
							break;

							
							case OVERDUBBING:	// this is possible only when is NOT loopSync 
								trackStatus = OVERDUBBING;
								nextStatus = NOTHING;

								setOverdubLed();
							break;
							

							case NOTHING:
								if (!oneShot_setting) {
									nextStatus = NOTHING;

									if (overdubAfterRec) {
										trackStatus = OVERDUBBING;
										setOverdubLed();
									} else {
										trackStatus = PLAYING;
										setPlayLed();
									}
									
								} else {
									busyTracks--;
									busyTrack = false;
									trackStatus = IDLE;
									nextStatus = NOTHING;

									setIdleLed();
								}
								
							break;
						}

						if (!rev_setting) { 
							playingDirection = FORWARD;
							samplePos = 0;
						} else {
							playingDirection = REVERSE;
							samplePos = totalSamples;
						}	
				
					break;
					
					case OVERDUBBING:
						
						if (playingDirection == FORWARD) {

							extraRecording = true;
							extraRecDirection = FORWARD;
							extraRecCount = 0;
							if (!stopNow) {
								extraRecMaxSamples = tailSamples;
								extraSamples = true;
							} else {
								extraRecMaxSamples = minTimeSamplesOvs;
								recFade = true;
								recFadeValue = 0.f;
								recFadeDelta = -1 / minTimeSamples;
							}
								
							extraRecPos = samplePos;

							if (extraPlayingFadeOut)
								extraPlayingFadeOut = false;
							else
								xFadePlay();

						} else if (stopNow) {	// if it's REVERSE and it's stopNow
							recFade = true;
							recFadeValue = 0.f;
							recFadeDelta = -1 / minTimeSamples;

							extraRecording = true;
							extraRecDirection = REVERSE;
							extraRecCount = 0;
							extraRecMaxSamples = minTimeSamplesOvs;
							extraRecPos = samplePos;

							if (extraPlayingFadeOut)
								extraPlayingFadeOut = false;
							else
								xFadePlay();
						}
						
						if (oneShot_setting && !stopNow) {

							if (nextStatus == PLAYING) {
								trackStatus = PLAYING;
								nextStatus = NOTHING;

								if (!rev_setting) { 
									if (extraPlayingFadeOut)
										extraPlayingFadeOut = false;
									else
										xFadePlay();

									playingDirection = FORWARD;
									samplePos = 0;
								} else {
									playingDirection = REVERSE;
									samplePos = totalSamples;
								}
								
								setPlayLed();

							} else {
								busyTracks--;								
								busyTrack = false;
								trackStatus = IDLE;
								nextStatus = NOTHING;

								if (!rev_setting && playFullTail) {
									playTail = true;
									tailEnd = samplePos + tailSamples;
								}

								setIdleLed();
							}
						} else {
							switch (nextStatus) {
								case IDLE:
									busyTracks--;
									busyTrack = false;
									trackStatus = IDLE;
									nextStatus = NOTHING;

									if (!rev_setting && playFullTail && !stopNow) {
										playTail = true;
										tailEnd = samplePos + tailSamples;
									}

									setIdleLed();
								break;

								case PLAYING:
									trackStatus = PLAYING;
									nextStatus = NOTHING;

									setPlayLed();

								break;
							}
							
							if (!rev_setting) { 
								if (extraPlayingFadeOut)
									extraPlayingFadeOut = false;
								else
									xFadePlay();
								playingDirection = FORWARD;
								samplePos = 0;
							} else {
								if (stopNow) {
									if (extraPlayingFadeOut)
										extraPlayingFadeOut = false;
									else
										xFadePlay();
								}
								playingDirection = REVERSE;
								samplePos = totalSamples;
							}
							
						}
					break;
				}

			}

		}	// end LOOP END if

		// EOL check

		if (eolPulse) {
			eolPulseTime--;
			if (eolPulseTime < 0) {
				eolPulse = false;
				outputs[EOL_OUTPUT].setVoltage(0.f);
			} else
				outputs[EOL_OUTPUT].setVoltage(10.f);
		}
		// ******************************************************************** PLAY / REC / OVERDUB
		currentOutput[LEFT] = 0.f;
		currentOutput[RIGHT] = 0.f;

		if (inputs[VOCT_INPUT].isConnected()) {
			voct = inputs[VOCT_INPUT].getVoltage();
			if (voct != prevVoct) {
				speedVoct = pow(2,voct);
				prevVoct = voct;
			}
			distancePos = sampleCoeff * speedVoct;
		} else
			distancePos = sampleCoeff;


		switch (trackStatus) {
			case PLAYING:

				if (samplePos > totalSamples && !extraPlayingFadeOut) {
					extraPlayingFadeOut = true;
					xFadePlay();
					samplePos = totalSampleC;
				}

				if (samplePos < totalSampleC && samplePos >= 0) {

					if (!fadeIn) {
						/*
						currentOutput[LEFT] = trackBuffer[LEFT][samplePos];
						currentOutput[RIGHT] = trackBuffer[RIGHT][samplePos];
						*/

						// *** SICKOLOOPER1 USES HERMITE INTERPOLATION ONLY ***

						if (floor(samplePos) > 0 && floor(samplePos) < totalSamples - 1) {
							/*
							currentOutput = hermiteInterpol(playBuffer[i][antiAlias][floor(samplePos[i])-1],
															playBuffer[i][antiAlias][floor(samplePos[i])],
															playBuffer[i][antiAlias][floor(samplePos[i])+1],
															playBuffer[i][antiAlias][floor(samplePos[i])+2],
															currSampleWeight[i]);
							*/
							// below is translation of the above function
							double a1 = .5F * (trackBuffer[LEFT][floor(samplePos)+1] - trackBuffer[LEFT][floor(samplePos)-1]);
							double a2 = trackBuffer[LEFT][floor(samplePos)-1] - (2.5F * trackBuffer[LEFT][floor(samplePos)]) + (2 * trackBuffer[LEFT][floor(samplePos)+1]) - (.5F * trackBuffer[LEFT][floor(samplePos)+2]);
							double a3 = (.5F * (trackBuffer[LEFT][floor(samplePos)+2] - trackBuffer[LEFT][floor(samplePos)-1])) + (1.5F * (trackBuffer[LEFT][floor(samplePos)] - trackBuffer[LEFT][floor(samplePos)+1]));
							currentOutput[LEFT] = (((((a3 * currSampleWeight) + a2) * currSampleWeight) + a1) * currSampleWeight) + trackBuffer[LEFT][floor(samplePos)];

							a1 = .5F * (trackBuffer[RIGHT][floor(samplePos)+1] - trackBuffer[RIGHT][floor(samplePos)-1]);
							a2 = trackBuffer[RIGHT][floor(samplePos)-1] - (2.5F * trackBuffer[RIGHT][floor(samplePos)]) + (2 * trackBuffer[RIGHT][floor(samplePos)+1]) - (.5F * trackBuffer[RIGHT][floor(samplePos)+2]);
							a3 = (.5F * (trackBuffer[RIGHT][floor(samplePos)+2] - trackBuffer[RIGHT][floor(samplePos)-1])) + (1.5F * (trackBuffer[RIGHT][floor(samplePos)] - trackBuffer[RIGHT][floor(samplePos)+1]));
							currentOutput[RIGHT] = (((((a3 * currSampleWeight) + a2) * currSampleWeight) + a1) * currSampleWeight) + trackBuffer[RIGHT][floor(samplePos)];
							

						} else { // if playing sample is the first or one of the last 3 -> no interpolation
							currentOutput[LEFT] = trackBuffer[LEFT][floor(samplePos)];
							currentOutput[RIGHT] = trackBuffer[RIGHT][floor(samplePos)];
						}



					} else {
						fadeInValue += fadeInDelta;
						if (fadeInValue > 1.f) {
							fadeIn = false;
							fadeInValue = 1.f;
						}
						/*
						currentOutput[LEFT] = trackBuffer[LEFT][samplePos] * fadeInValue;
						currentOutput[RIGHT] = trackBuffer[RIGHT][samplePos] * fadeInValue;
						*/
						// below is translation of the above function
						double a1 = .5F * (trackBuffer[LEFT][floor(samplePos)+1] - trackBuffer[LEFT][floor(samplePos)-1]);
						double a2 = trackBuffer[LEFT][floor(samplePos)-1] - (2.5F * trackBuffer[LEFT][floor(samplePos)]) + (2 * trackBuffer[LEFT][floor(samplePos)+1]) - (.5F * trackBuffer[LEFT][floor(samplePos)+2]);
						double a3 = (.5F * (trackBuffer[LEFT][floor(samplePos)+2] - trackBuffer[LEFT][floor(samplePos)-1])) + (1.5F * (trackBuffer[LEFT][floor(samplePos)] - trackBuffer[LEFT][floor(samplePos)+1]));
						currentOutput[LEFT] = fadeInValue * ( (((((a3 * currSampleWeight) + a2) * currSampleWeight) + a1) * currSampleWeight) + trackBuffer[LEFT][floor(samplePos)] );

						a1 = .5F * (trackBuffer[RIGHT][floor(samplePos)+1] - trackBuffer[RIGHT][floor(samplePos)-1]);
						a2 = trackBuffer[RIGHT][floor(samplePos)-1] - (2.5F * trackBuffer[RIGHT][floor(samplePos)]) + (2 * trackBuffer[RIGHT][floor(samplePos)+1]) - (.5F * trackBuffer[RIGHT][floor(samplePos)+2]);
						a3 = (.5F * (trackBuffer[RIGHT][floor(samplePos)+2] - trackBuffer[RIGHT][floor(samplePos)-1])) + (1.5F * (trackBuffer[RIGHT][floor(samplePos)] - trackBuffer[RIGHT][floor(samplePos)+1]));
						currentOutput[RIGHT] = fadeInValue * ( (((((a3 * currSampleWeight) + a2) * currSampleWeight) + a1) * currSampleWeight) + trackBuffer[RIGHT][floor(samplePos)] );
						
					}
				
					if (playingDirection == FORWARD)
						samplePos += distancePos;
					else
						samplePos -= distancePos;

					currSampleWeight = samplePos - floor(samplePos);
				}
			break;

			case RECORDING:
				trackBuffer[LEFT].push_back(inputValue[LEFT]);
				trackBuffer[LEFT].push_back(0);
				trackBuffer[RIGHT].push_back(inputValue[RIGHT]);
				trackBuffer[RIGHT].push_back(0);
				if (samplePos > 0) {
					trackBuffer[LEFT][samplePos-1] = (trackBuffer[LEFT][samplePos-2] + inputValue[LEFT]) / 2;
					trackBuffer[RIGHT][samplePos-1] = (trackBuffer[RIGHT][samplePos-2] + inputValue[RIGHT]) / 2;
				}
				samplePos += sampleCoeff;
			break;

			case OVERDUBBING:

				if (samplePos >= trackBuffer[LEFT].size()) {
					trackBuffer[LEFT].push_back(0.f);
					trackBuffer[LEFT].push_back(0.f);
					trackBuffer[RIGHT].push_back(0.f);
					trackBuffer[RIGHT].push_back(0.f);
				}

				if (samplePos >= 0) {
					
					if (!fadeIn) {
						currentOutput[LEFT] = trackBuffer[LEFT][samplePos];
						currentOutput[RIGHT] = trackBuffer[RIGHT][samplePos];
					} else {
						fadeInValue += fadeInDelta;
						if (fadeInValue > 1.f) {
							fadeIn = false;
							fadeInValue = 1.f;
						}
						currentOutput[LEFT] = trackBuffer[LEFT][samplePos] * fadeInValue;
						currentOutput[RIGHT] = trackBuffer[RIGHT][samplePos] * fadeInValue;
					}
					
					if (recFade) {
						recFadeValue += recFadeDelta;
						if (recFadeValue < 0) {
							recFadeValue = 0;
							recFade = false;
						} else if ( recFadeValue > 1) {
							recFadeValue = 1;
							recFade = false;
						}
					}

					trackBuffer[LEFT][samplePos] += inputValue[LEFT] * recFadeValue;
					trackBuffer[RIGHT][samplePos] += inputValue[RIGHT] * recFadeValue;

					/*
					if (samplePos > 0) {
						trackBuffer[LEFT][samplePos-1] = (trackBuffer[LEFT][samplePos-2] + trackBuffer[LEFT][samplePos]) / 2;
						trackBuffer[RIGHT][samplePos-1] = (trackBuffer[RIGHT][samplePos-2] + trackBuffer[RIGHT][samplePos]) / 2;
					}

					if (playingDirection == FORWARD)
						samplePos += sampleCoeff;
					else
						samplePos -= sampleCoeff;
					*/
					if (playingDirection == FORWARD) {
						if (samplePos != 0) {
							trackBuffer[LEFT][samplePos-1] = (trackBuffer[LEFT][samplePos-2] + trackBuffer[LEFT][samplePos]) / 2;
							trackBuffer[RIGHT][samplePos-1] = (trackBuffer[RIGHT][samplePos-2] + trackBuffer[RIGHT][samplePos]) / 2;
						}
						samplePos += sampleCoeff;
					} else {
						if (samplePos != 0) {
							trackBuffer[LEFT][samplePos+1] = (trackBuffer[LEFT][samplePos+2] + trackBuffer[LEFT][samplePos]) / 2;
							trackBuffer[RIGHT][samplePos+1] = (trackBuffer[RIGHT][samplePos+2] + trackBuffer[RIGHT][samplePos]) / 2;
						}
						samplePos -= sampleCoeff;
					}
				}

			break;
		}

		if (extraPlaying) {

			if (!playTail) {
				xFadeValue -= xFadeDelta;
				if (xFadeValue < 0) {
					extraPlaying = false;
				} else {
					if (extraPlayPos >= 0 && extraPlayPos < trackBuffer[LEFT].size()) {
						currentOutput[LEFT] *= 1-xFadeValue;
						currentOutput[RIGHT] *= 1-xFadeValue;
						
						/*
						currentOutput[LEFT] += trackBuffer[LEFT][extraPlayPos] * xFadeValue;
						currentOutput[RIGHT] += trackBuffer[RIGHT][extraPlayPos] * xFadeValue;
						*/

						double a1 = .5F * (trackBuffer[LEFT][floor(extraPlayPos)+1] - trackBuffer[LEFT][floor(extraPlayPos)-1]);
						double a2 = trackBuffer[LEFT][floor(extraPlayPos)-1] - (2.5F * trackBuffer[LEFT][floor(extraPlayPos)]) + (2 * trackBuffer[LEFT][floor(extraPlayPos)+1]) - (.5F * trackBuffer[LEFT][floor(extraPlayPos)+2]);
						double a3 = (.5F * (trackBuffer[LEFT][floor(extraPlayPos)+2] - trackBuffer[LEFT][floor(extraPlayPos)-1])) + (1.5F * (trackBuffer[LEFT][floor(extraPlayPos)] - trackBuffer[LEFT][floor(extraPlayPos)+1]));
						currentOutput[LEFT] += xFadeValue * ( (((((a3 * currSampleWeightXtra) + a2) * currSampleWeightXtra) + a1) * currSampleWeightXtra) + trackBuffer[LEFT][floor(extraPlayPos)] );

						a1 = .5F * (trackBuffer[RIGHT][floor(extraPlayPos)+1] - trackBuffer[RIGHT][floor(extraPlayPos)-1]);
						a2 = trackBuffer[RIGHT][floor(extraPlayPos)-1] - (2.5F * trackBuffer[RIGHT][floor(extraPlayPos)]) + (2 * trackBuffer[RIGHT][floor(extraPlayPos)+1]) - (.5F * trackBuffer[RIGHT][floor(extraPlayPos)+2]);
						a3 = (.5F * (trackBuffer[RIGHT][floor(extraPlayPos)+2] - trackBuffer[RIGHT][floor(extraPlayPos)-1])) + (1.5F * (trackBuffer[RIGHT][floor(extraPlayPos)] - trackBuffer[RIGHT][floor(extraPlayPos)+1]));
						currentOutput[RIGHT] += xFadeValue * ( (((((a3 * currSampleWeightXtra) + a2) * currSampleWeightXtra) + a1) * currSampleWeightXtra) + trackBuffer[RIGHT][floor(extraPlayPos)] );
						
						if (extraPlayDirection == FORWARD)
							extraPlayPos += distancePos;
						else
							extraPlayPos -= distancePos;

						currSampleWeightXtra = extraPlayPos - floor(extraPlayPos);
					} else {
						extraPlaying = false;
					}
				}
			} else {	// if it's playing full tail, only if direction is FORWARD

				if (extraPlayPos < tailEnd - minTimeSamplesOvs) {

					/*
					currentOutput[LEFT] += trackBuffer[LEFT][extraPlayPos];
					currentOutput[RIGHT] += trackBuffer[RIGHT][extraPlayPos];
					*/

					double a1 = .5F * (trackBuffer[LEFT][floor(extraPlayPos)+1] - trackBuffer[LEFT][floor(extraPlayPos)-1]);
					double a2 = trackBuffer[LEFT][floor(extraPlayPos)-1] - (2.5F * trackBuffer[LEFT][floor(extraPlayPos)]) + (2 * trackBuffer[LEFT][floor(extraPlayPos)+1]) - (.5F * trackBuffer[LEFT][floor(extraPlayPos)+2]);
					double a3 = (.5F * (trackBuffer[LEFT][floor(extraPlayPos)+2] - trackBuffer[LEFT][floor(extraPlayPos)-1])) + (1.5F * (trackBuffer[LEFT][floor(extraPlayPos)] - trackBuffer[LEFT][floor(extraPlayPos)+1]));
					currentOutput[LEFT] += (((((a3 * currSampleWeightXtra) + a2) * currSampleWeightXtra) + a1) * currSampleWeightXtra) + trackBuffer[LEFT][floor(extraPlayPos)];

					a1 = .5F * (trackBuffer[RIGHT][floor(extraPlayPos)+1] - trackBuffer[RIGHT][floor(extraPlayPos)-1]);
					a2 = trackBuffer[RIGHT][floor(extraPlayPos)-1] - (2.5F * trackBuffer[RIGHT][floor(extraPlayPos)]) + (2 * trackBuffer[RIGHT][floor(extraPlayPos)+1]) - (.5F * trackBuffer[RIGHT][floor(extraPlayPos)+2]);
					a3 = (.5F * (trackBuffer[RIGHT][floor(extraPlayPos)+2] - trackBuffer[RIGHT][floor(extraPlayPos)-1])) + (1.5F * (trackBuffer[RIGHT][floor(extraPlayPos)] - trackBuffer[RIGHT][floor(extraPlayPos)+1]));
					currentOutput[RIGHT] += (((((a3 * currSampleWeightXtra) + a2) * currSampleWeightXtra) + a1) * currSampleWeightXtra) + trackBuffer[RIGHT][floor(extraPlayPos)];

					extraPlayPos += distancePos;
					currSampleWeightXtra = extraPlayPos - floor(extraPlayPos);

				} else {
					if (!fadeTail) {
						fadeTail = true;
						fadeTailValue = 1;
					}
					if (extraPlayPos < tailEnd) {
						fadeTailValue -= fadeTailDelta;
						//currentOutput[LEFT] += trackBuffer[LEFT][extraPlayPos] * fadeTailValue;
						//currentOutput[RIGHT] += trackBuffer[RIGHT][extraPlayPos] * fadeTailValue;

						double a1 = .5F * (trackBuffer[LEFT][floor(extraPlayPos)+1] - trackBuffer[LEFT][floor(extraPlayPos)-1]);
						double a2 = trackBuffer[LEFT][floor(extraPlayPos)-1] - (2.5F * trackBuffer[LEFT][floor(extraPlayPos)]) + (2 * trackBuffer[LEFT][floor(extraPlayPos)+1]) - (.5F * trackBuffer[LEFT][floor(extraPlayPos)+2]);
						double a3 = (.5F * (trackBuffer[LEFT][floor(extraPlayPos)+2] - trackBuffer[LEFT][floor(extraPlayPos)-1])) + (1.5F * (trackBuffer[LEFT][floor(extraPlayPos)] - trackBuffer[LEFT][floor(extraPlayPos)+1]));
						currentOutput[LEFT] += fadeTailValue * ( (((((a3 * currSampleWeightXtra) + a2) * currSampleWeightXtra) + a1) * currSampleWeightXtra) + trackBuffer[LEFT][floor(extraPlayPos)] );

						a1 = .5F * (trackBuffer[RIGHT][floor(extraPlayPos)+1] - trackBuffer[RIGHT][floor(extraPlayPos)-1]);
						a2 = trackBuffer[RIGHT][floor(extraPlayPos)-1] - (2.5F * trackBuffer[RIGHT][floor(extraPlayPos)]) + (2 * trackBuffer[RIGHT][floor(extraPlayPos)+1]) - (.5F * trackBuffer[RIGHT][floor(extraPlayPos)+2]);
						a3 = (.5F * (trackBuffer[RIGHT][floor(extraPlayPos)+2] - trackBuffer[RIGHT][floor(extraPlayPos)-1])) + (1.5F * (trackBuffer[RIGHT][floor(extraPlayPos)] - trackBuffer[RIGHT][floor(extraPlayPos)+1]));
						currentOutput[RIGHT] += fadeTailValue * ( (((((a3 * currSampleWeightXtra) + a2) * currSampleWeightXtra) + a1) * currSampleWeightXtra) + trackBuffer[RIGHT][floor(extraPlayPos)] );
						
						extraPlayPos += distancePos;
						currSampleWeightXtra = extraPlayPos - floor(extraPlayPos);														
					} else {
						extraPlaying = false;
						playTail = false;
						fadeTail = false;
					}
				}
			}
		}

		if (extraRecording) {

			extraRecCount += sampleCoeff;

			if (extraRecCount > extraRecMaxSamples) {
				extraRecording = false;

			} else {
				if (extraRecPos >= trackBuffer[LEFT].size()) {
					trackBuffer[LEFT].push_back(0.f);
					trackBuffer[LEFT].push_back(0.f);
					trackBuffer[RIGHT].push_back(0.f);
					trackBuffer[RIGHT].push_back(0.f);
				}

				if (recFade) {
					recFadeValue += recFadeDelta;
					if (recFadeValue < 0) {
						recFadeValue = 0;
						recFade = false;
					} else if ( recFadeValue > 1) {
						recFadeValue = 1;
						recFade = false;
					}
				}

				//if (extraRecPos >= 0) {	// extraRecPos is unsigned. this condition means nothing
					trackBuffer[LEFT][extraRecPos] += inputValue[LEFT] * recFadeValue;
					trackBuffer[RIGHT][extraRecPos] += inputValue[RIGHT] * recFadeValue;
				//}

				/*
				if (samplePos > 0) {
					trackBuffer[LEFT][extraRecPos-1] = (trackBuffer[LEFT][extraRecPos-2] + trackBuffer[LEFT][extraRecPos]) / 2;
					trackBuffer[RIGHT][extraRecPos-1] = (trackBuffer[RIGHT][extraRecPos-2] + trackBuffer[RIGHT][extraRecPos]) / 2;
				}
				
				if (extraRecDirection == FORWARD)
					extraRecPos += sampleCoeff;
				else
					extraRecPos -= sampleCoeff;
				*/
				if (extraRecDirection == FORWARD) {
					if (samplePos > 0) {
						trackBuffer[LEFT][extraRecPos-1] = (trackBuffer[LEFT][extraRecPos-2] + trackBuffer[LEFT][extraRecPos]) / 2;
						trackBuffer[RIGHT][extraRecPos-1] = (trackBuffer[RIGHT][extraRecPos-2] + trackBuffer[RIGHT][extraRecPos]) / 2;
					}
					extraRecPos += sampleCoeff;
				} else {
					trackBuffer[LEFT][extraRecPos+1] = (trackBuffer[LEFT][extraRecPos+2] + trackBuffer[LEFT][extraRecPos]) / 2;
					trackBuffer[RIGHT][extraRecPos+1] = (trackBuffer[RIGHT][extraRecPos+2] + trackBuffer[RIGHT][extraRecPos]) / 2;
					extraRecPos -= sampleCoeff;
				}
			}
		}
		

		// ************************ TRACK OUTPUT MANAGEMENT

		if (!inputs[PAN_INPUT].isConnected())
			panKnobValue = params[PAN_KNOB_PARAM].getValue();
		else {
			if (panRange == 0)
				panKnobValue = params[PAN_KNOB_PARAM].getValue() + (inputs[PAN_INPUT].getVoltage() / 5.f) - 1;
			else if (panRange == 1)
				panKnobValue = params[PAN_KNOB_PARAM].getValue() + inputs[PAN_INPUT].getVoltage() / 5.f;
			else if (panRange == 2)
				panKnobValue = params[PAN_KNOB_PARAM].getValue() + inputs[PAN_INPUT].getVoltage() / 10.f;

			if (panKnobValue > 1)
				panKnobValue = 1;
			else if (panKnobValue < -1)
				panKnobValue = -1;
		}

		if (panKnobValue < 0.f) {
			panLeftCoeff = 1.f;
			panRightCoeff = 1.f + panKnobValue;
		} else {
			panLeftCoeff = 1.f - panKnobValue;
			panRightCoeff = 1.f; 
		}

		if (!srcToTrack) {
			if (!srcToTrackFade) {
				currentOutput[LEFT] *= volTrack * panLeftCoeff;
				currentOutput[RIGHT] *= volTrack * panRightCoeff;
			} else {	// residuo di fade out dell'input
				currentOutput[LEFT] += volTrack * srcToTrackValue * inputValue[LEFT] * panLeftCoeff;
				currentOutput[RIGHT] += volTrack * srcToTrackValue * inputValue[RIGHT] * panRightCoeff;
			}

		} else {
			if (!srcToTrackFade) {
				currentOutput[LEFT] += inputValue[LEFT];
				currentOutput[RIGHT] += inputValue[RIGHT];

				currentOutput[LEFT] *= volTrack * panLeftCoeff;
				currentOutput[RIGHT] *= volTrack * panRightCoeff;

			} else {
				currentOutput[LEFT] += srcToTrackValue * inputValue[LEFT];
				currentOutput[RIGHT] += srcToTrackValue * inputValue[RIGHT];

				currentOutput[LEFT] *= volTrack * panLeftCoeff;
				currentOutput[RIGHT] *= volTrack * panRightCoeff;	

			}
		}

		if (currentOutput[LEFT] > 10.f)
			currentOutput[LEFT] = 10.f;
		else if (currentOutput[LEFT] < -10.f)
			currentOutput[LEFT] = -10.f;

		if (currentOutput[RIGHT] > 10.f)
			currentOutput[RIGHT] = 10.f;
		else if (currentOutput[RIGHT] < -10.f)
			currentOutput[RIGHT] = -10.f;

		outputs[TRACK_LEFT_OUTPUT].setVoltage(currentOutput[LEFT]);
		outputs[TRACK_RIGHT_OUTPUT].setVoltage(currentOutput[RIGHT]);


		// ********************************************* BUTTON LIGHT MANAGEMENT ****************************************
		
		if (clockSample < midBeatMaxSample)
			ledLight = true;
		else
			ledLight = false;

		if (playButtonPulse == SLOW_PULSE) {
			if (runSetting == 1) {
				if (ledLight)
					lights[PLAY_BUT_LIGHT].setBrightness(1.f);
				else
					lights[PLAY_BUT_LIGHT].setBrightness(0.f);
			} else {
				playButtonPulseTime--;
				if (playButtonPulseTime < 0) {
					playButtonPulseTime = slowPulseTime;
					lights[PLAY_BUT_LIGHT].setBrightness(!lights[PLAY_BUT_LIGHT].getBrightness());
				}

			}
		} else if (playButtonPulse == FAST_PULSE) {
			playButtonPulseTime--;
			if (playButtonPulseTime < 0) {
				playButtonPulseTime = fastPulseTime;
				lights[PLAY_BUT_LIGHT].setBrightness(!lights[PLAY_BUT_LIGHT].getBrightness());
			}					
		}

		if (recButtonPulse == SLOW_PULSE) {
			if (runSetting == 1) {
				if (ledLight)
					lights[REC_BUT_LIGHT].setBrightness(1.f);
				else
					lights[REC_BUT_LIGHT].setBrightness(0.f);
			} else {
				recButtonPulseTime--;
				if (recButtonPulseTime < 0) {
					recButtonPulseTime = slowPulseTime;
					lights[REC_BUT_LIGHT].setBrightness(!lights[REC_BUT_LIGHT].getBrightness());
				}

			}
		} else if (recButtonPulse == FAST_PULSE) {
			recButtonPulseTime--;
			if (recButtonPulseTime < 0) {
				recButtonPulseTime = fastPulseTime;
				lights[REC_BUT_LIGHT].setBrightness(!lights[REC_BUT_LIGHT].getBrightness());
			}					
		}

	
//
//																								░██████╗░██╗░░░░░░█████╗░██████╗░░█████╗░██╗░░░░░
//																								██╔════╝░██║░░░░░██╔══██╗██╔══██╗██╔══██╗██║░░░░░
//																								██║░░██╗░██║░░░░░██║░░██║██████╦╝███████║██║░░░░░
//																								██║░░╚██╗██║░░░░░██║░░██║██╔══██╗██╔══██║██║░░░░░
//																								╚██████╔╝███████╗╚█████╔╝██████╦╝██║░░██║███████╗
//																								░╚═════╝░╚══════╝░╚════╝░╚═════╝░╚═╝░░╚═╝╚══════╝


		// ****************************************************************************** GLOBAL STATUS

		/*
		if (busyTracks == 0) {

			globalStatus = IDLE;

			if (!extConn && !internalClockAlwaysOn) {
				runSetting = 0;
				sendRunToExp = 0;
				clockSample = 1.0;
				resetStart = true;
				beatCounter = 20;	// this hack makes the click be audible on next runSetting=1
			}
		}
		*/
		
		if (!waitingExpResp) {
			if (busyTracks == 0) {

				globalStatus = IDLE;

				if (!extConn && !internalClockAlwaysOn) {
					runSetting = 0;
					sendRunToExp = 0;
					clockSample = 1.0;
					resetStart = true;
					beatCounter = 20;	// this hack makes the click be audible on next runSetting=1
				}
			}
		} else {
			if (busyTracks == 0) {
				if (lagBusyTracks > expConnected * 2) {
					//lagBusyTracks = 0;
					waitingExpResp = false;
					globalStatus = IDLE;

					if (!extConn && !internalClockAlwaysOn) {
						runSetting = 0;
						sendRunToExp = 0;
						clockSample = 1.0;
						resetStart = true;
						beatCounter = 20;	// this hack makes the click be audible on next runSetting=1
					}
				}
				lagBusyTracks++;

			}
		}

		/*
		debugDisplay2 = "lag " + to_string(lagBusyTracks);
		if (globalStatus == IDLE) {
			debugDisplay3 = "IDLE";
		} else if (globalStatus == PLAYING)
			debugDisplay3 = "PLAYING";
		else
			debugDisplay3 = "PREROLL";
		debugDisplay4 = "busy " + to_string(busyTracks);
		*/

//
//																							░█████╗░██╗░░░██╗████████╗██████╗░██╗░░░██╗████████╗
//																							██╔══██╗██║░░░██║╚══██╔══╝██╔══██╗██║░░░██║╚══██╔══╝
//																							██║░░██║██║░░░██║░░░██║░░░██████╔╝██║░░░██║░░░██║░░░
//																							██║░░██║██║░░░██║░░░██║░░░██╔═══╝░██║░░░██║░░░██║░░░
//																							╚█████╔╝╚██████╔╝░░░██║░░░██║░░░░░╚██████╔╝░░░██║░░░
//																							░╚════╝░░╚═════╝░░░░╚═╝░░░╚═╝░░░░░░╚═════╝░░░░╚═╝░░░


		if (clickOutput > 10.f)
			clickOutput = 10.f;
		else if (clickOutput < -10.f)
			clickOutput = -10.f;
		outputs[CLICK_OUTPUT].setVoltage(clickOutput);


		//	--------------------- reset pulse
		if (rstPulse) {
			rstPulseTime--;
			if (rstPulseTime < 0) {
				rstPulse = false;
				outputs[RESET_OUTPUT].setVoltage(0.f);
			} else
				outputs[RESET_OUTPUT].setVoltage(10.f);
		}

		// --------------------------------------------------------------------------------------------------------  E X P A N D E R

		if (rightExpander.module && rightExpander.module->model == modelSickoLooper1Exp) {
			Sl1ExpMsg *msgToModule = (Sl1ExpMsg *)(rightExpander.module->leftExpander.producerMessage);

			msgToModule->connectedToMaster = connectedToMaster;

			if (barReached && globalStatus == PLAYING)
				msgToModule->barPulse = true;
			else
				msgToModule->barPulse = false;

			if (sendRunToExp)
				msgToModule->runSetting = 1;
			else
				msgToModule->runSetting = 0;

			if (globalStatus == PLAYING)
				msgToModule->globalStatus = PLAYING;
			else
				msgToModule->globalStatus = IDLE;

			msgToModule->ledLight = ledLight;

			msgToModule->ssAll = sendSsAllToExp;

			msgToModule->reset = sendReset;

			msgToModule->barSample = barSample;

			rightExpander.module->leftExpander.messageFlipRequested = true;
		}


		//debugDisplay = "exp " + to_string(expConnected); 

		/*
		debugDisplay = "busy " + to_string(busyTracks); 
		debugDisplay2 = "RCV RUN " + to_string(receivedRunFromExp); 
		debugDisplay3 = "SEND RUN " + to_string(sendRunToExp); 

		if (receivedRunFromExp)
			debugDisplay4 = "RCV " + to_string(debugInt++); 
		*/
	}

	
};




//
//															███████╗███╗░░██╗██████╗░  ██████╗░██████╗░░█████╗░░█████╗░███████╗░██████╗░██████╗
//															██╔════╝████╗░██║██╔══██╗  ██╔══██╗██╔══██╗██╔══██╗██╔══██╗██╔════╝██╔════╝██╔════╝
//															█████╗░░██╔██╗██║██║░░██║  ██████╔╝██████╔╝██║░░██║██║░░╚═╝█████╗░░╚█████╗░╚█████╗░
//															██╔══╝░░██║╚████║██║░░██║  ██╔═══╝░██╔══██╗██║░░██║██║░░██╗██╔══╝░░░╚═══██╗░╚═══██╗
//															███████╗██║░╚███║██████╔╝  ██║░░░░░██║░░██║╚█████╔╝╚█████╔╝███████╗██████╔╝██████╔╝
//															╚══════╝╚═╝░░╚══╝╚═════╝░  ╚═╝░░░░░╚═╝░░╚═╝░╚════╝░░╚════╝░╚══════╝╚═════╝░╚═════╝░


/*
struct SickoLooper1DisplaySrc1 : TransparentWidget {
	SickoLooper1 *module;
	int frame = 0;
	SickoLooper1DisplaySrc1() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED)); 
				nvgTextBox(args.vg, 7, 17, 60, to_string(int(module->params[module->SOURCE_KNOB_PARAM].getValue())).c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}
};
*/

struct SickoLooper1DisplayMeas1 : TransparentWidget {
	SickoLooper1 *module;
	int frame = 0;
	SickoLooper1DisplayMeas1() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED));					
				if (module->trackLoopMeas > 9)
					nvgTextBox(args.vg, 3, 17, 60, to_string(module->trackLoopMeas).c_str(), NULL);
				else
					nvgTextBox(args.vg, 11, 17, 60, to_string(module->trackLoopMeas).c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct SickoLooper1DisplayLoop1 : TransparentWidget {
	SickoLooper1 *module;
	int frame = 0;
	SickoLooper1DisplayLoop1() {
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
				switch (module->trackStatus) {
					case IDLE:
						nvgStrokeColor(args.vg, nvgRGBA(COLOR_BLUE));
						nvgStrokeWidth(args.vg, 15);
						{
							nvgBeginPath(args.vg);
							nvgMoveTo(args.vg, 9, 76);
							nvgLineTo(args.vg, 9, 1);
							nvgClosePath(args.vg);
						}
						nvgStroke(args.vg);
					break;

					case PLAYING:
						nvgStrokeWidth(args.vg, 15);
						{
							nvgBeginPath(args.vg);
							nvgMoveTo(args.vg, 9, 76);
							if (module->samplePos < module->totalSamples) {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_GREEN));
								nvgLineTo(args.vg, 9, 76-(75*module->samplePos/module->totalSamples));
							} else {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_DARK_GREEN));
								nvgLineTo(args.vg, 9, 1);
							}
							nvgClosePath(args.vg);
						}
						nvgStroke(args.vg);
					break;

					case RECORDING:
						nvgStrokeColor(args.vg, nvgRGBA(COLOR_RED));
						nvgStrokeWidth(args.vg, 15);
						{
							nvgBeginPath(args.vg);
							nvgMoveTo(args.vg, 9, 76);
							nvgLineTo(args.vg, 9, 1);
							nvgClosePath(args.vg);
						}
						nvgStroke(args.vg);
					break;

					case OVERDUBBING:
						nvgStrokeWidth(args.vg, 15);
						{
							nvgBeginPath(args.vg);
							nvgMoveTo(args.vg, 9, 76);
							if (module->samplePos < module->totalSamples) {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_YELLOW));
								nvgLineTo(args.vg, 9, 76-(75*module->samplePos/module->totalSamples));
							} else {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_DARK_YELLOW));
								nvgLineTo(args.vg, 9, 1);
							}
							nvgClosePath(args.vg);
						}
						nvgStroke(args.vg);
					break;
				}
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		SickoLooper1 *module = dynamic_cast<SickoLooper1 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			//const int track = 0;

			menu->addChild(createMenuLabel("TRACK"));
			menu->addChild(createBoolPtrMenuItem("Fade IN on playback", "", &module->fadeInOnPlay));
			menu->addChild(createBoolPtrMenuItem("Play Full Tail on Stop", "", &module->playFullTail));
			
			menu->addChild(new MenuSeparator());
			menu->addChild(createBoolMenuItem("Extra samples Tail (1sec)", "", [=]() {
					return module->isExtraSamples();
				}, [=](bool xtraSamples) {
					module->setExtraSamples(xtraSamples);
			}));
			if (module->trackStatus != EMPTY)
				menu->addChild(createMenuItem("Detect tempo and set bpm", "", [=]() {module->detectTempo();}));
			else
				menu->addChild(createMenuLabel("Detect tempo and set bpm"));

			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Import Wav", "", [=]() {module->menuLoadSample();}));
			if (module->trackStatus != EMPTY)
				menu->addChild(createMenuItem("Export Wav", "", [=]() {module->menuSaveSample();}));
			else
				menu->addChild(createMenuLabel("Export Wav"));
		}
	}
};

// ------------------------------------------------------------------------------------------------------------------

struct SickoLooper1DisplayTempo : TransparentWidget {
	SickoLooper1 *module;
	int frame = 0;
	SickoLooper1DisplayTempo() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
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
						nvgTextBox(args.vg, 11.3, 16.5, 60, tempBpm.c_str(), NULL);
					else
						nvgTextBox(args.vg, 3, 16.5, 60, tempBpm.c_str(), NULL);
				} else {
					
					tempBpmInteger = int(module->bpm);
					tempBpm = to_string(tempBpmInteger)+".X";
					if (tempBpmInteger < 100)
						nvgTextBox(args.vg, 11.3, 16.5, 60, tempBpm.c_str(), NULL);
					else
						nvgTextBox(args.vg, 3, 16.5, 60, tempBpm.c_str(), NULL);
					
				}
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct SickoLooper1DisplayBeat : TransparentWidget {
	SickoLooper1 *module;
	int frame = 0;
	SickoLooper1DisplayBeat() {
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
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED)); 
				int tempValue = int(module->params[module->SIGNATURE_KNOB_PARAM].getValue());
				if (tempValue > 10)
					nvgTextBox(args.vg, 1.5, 17, 60, module->signatureDisplay[tempValue].c_str(), NULL);
				else
					nvgTextBox(args.vg, 9.8, 17, 60, module->signatureDisplay[tempValue].c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		SickoLooper1 *module = dynamic_cast<SickoLooper1 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				SickoLooper1* module;
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

/*
struct SickoLooper1DebugDisplay : TransparentWidget {
	SickoLooper1 *module;
	int frame = 0;
	SickoLooper1DebugDisplay() {
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
				nvgTextBox(args.vg, 129, 6,120, module->debugDisplay3.c_str(), NULL);
				nvgTextBox(args.vg, 129, 16,120, module->debugDisplay4.c_str(), NULL);

			}
		}
		Widget::drawLayer(args, layer);
	}
};
*/

struct SickoLooper1Widget : ModuleWidget {
	SickoLooper1Widget(SickoLooper1 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SickoLooper1.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		{
			SickoLooper1DisplayTempo *display = new SickoLooper1DisplayTempo();
			display->box.pos = mm2px(Vec(23.8, 33));
			display->box.size = mm2px(Vec(13.6, 7.9));
			display->module = module;
			addChild(display);
		}

		{
			SickoLooper1DisplayBeat *display = new SickoLooper1DisplayBeat();
			display->box.pos = mm2px(Vec(23.8, 44.2));
			display->box.size = mm2px(Vec(13.6, 7.9));
			display->module = module;
			addChild(display);
		}

		{
			SickoLooper1DisplayMeas1 *display = new SickoLooper1DisplayMeas1();
			display->box.pos = mm2px(Vec(53.2, 31.2));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}

		{
			SickoLooper1DisplayLoop1 *display = new SickoLooper1DisplayLoop1();
			display->box.pos = mm2px(Vec(64.5, 64));
			display->box.size = mm2px(Vec(6, 26));
			display->module = module;
			addChild(display);
		}
		
		// ----------------------------------------------------------------------------

		/*
		{
			SickoLooper1DebugDisplay *display = new SickoLooper1DebugDisplay();
			display->box.pos = Vec(23, 3);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/

		const float xInL = 48.1;
		const float xInR = 58.1;
		const float xInLvl = 69.1;
		const float xMute = 79.1;
		const float yIn = 18;

		const float xPlayTrig = 47.7;
		const float xPlayBut = 57.9;
		const float yPlay = 58.5;
		const float yRec = 72.5;
		const float yStop = 86.5;
		const float yErase = 100;


		const float xMeasKnob = 48.1;
		const float yMeasKnob = 35;

		const float xStartImm = 50.1;
		const float xStopImm = 64.1;
		const float yStartImm = 46.4;


		const float xSyncBut = 67.6;
		const float ySyncBut = 35;

		const float xXFade = 77.8;
		const float yXFade = 35;

	
		const float xOneShotBut = 67.6;
		const float yOneShotBut = 58.5;

		const float xVoct = 67.6;
		const float yVoct = 100;

		const float xRev = 77.8;
		const float yRevBut = 46.4;
		const float yRevIn = 54.3;

		const float xPan = 77.8;
		const float yPan = 67.5;
		const float yPanIn = 75.8;

		const float xVol = 77.8;
		const float yVol = 90;
		const float yVolIn = 100;

		const float xEol = 47.9;
		const float xSrcToTrack = 58;
		const float xOutL = 68.7;
		const float xOutR = 79.5;
		const float yOutput = 117.f;

		// ******************************
		
		const float xBpm = 17.358;
		const float yBpm = 36.6;
		const float xBeat = 17.458;
		const float yBeat = 48.4;

		const float xExtClock = 9.958;
		const float yExtClock = 22;
		const float xClockRst = 22.358;
		const float yClockRst = 22;
		const float xClockRstInput = 32.358;
		const float yClockRstInput = 22;

		const float xClick = 13.958;
		const float yClick = 66.5;
		const float xClickVol = 27.658;
		const float yClickVol = 66.7;

		const float xPrerollBut = 20.158;
		const float xPrerollSwitch = 30.958;
		const float yPreroll = 76;

		const float yAll = 99;
		const float xAllStartTrig = 8.358;
		const float xAllStartBut = 19.358;
		const float xAllStop = 33.358;
		
		const float xClockOut = 9.258;
		const float xRstOut = 21.158;
		const float xClickOut = 32.958;

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xBpm, yBpm)), module, SickoLooper1::BPM_KNOB_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xBeat, yBeat)), module, SickoLooper1::SIGNATURE_KNOB_PARAM));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xExtClock, yExtClock)), module, SickoLooper1::EXTCLOCK_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(xClockRst, yClockRst)), module, SickoLooper1::CLOCK_RST_SWITCH, SickoLooper1::CLOCK_RST_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xClockRstInput, yClockRstInput)), module, SickoLooper1::CLOCKRESET_INPUT));

		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xClick, yClick)), module, SickoLooper1::CLICK_BUT_PARAM, SickoLooper1::CLICK_BUT_LIGHT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xClickVol, yClickVol)), module, SickoLooper1::CLICKVOL_KNOB_PARAM));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xPrerollBut, yPreroll)), module, SickoLooper1::PREROLL_BUT_PARAM, SickoLooper1::PREROLL_BUT_LIGHT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(xPrerollSwitch, yPreroll)), module, SickoLooper1::PREROLL_SWITCH));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xAllStartTrig, yAll)), module, SickoLooper1::ALLSTARTSTOP_TRIG_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xAllStartBut, yAll)), module, SickoLooper1::ALLSTARTSTOP_BUT_PARAM, SickoLooper1::ALLSTARTSTOP_BUT_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xAllStop, yAll)), module, SickoLooper1::ALLSTOP_TRIG_INPUT));


		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xClockOut, yOutput)), module, SickoLooper1::CLOCK_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xRstOut, yOutput)), module, SickoLooper1::RESET_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xClickOut, yOutput)), module, SickoLooper1::CLICK_OUTPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInL, yIn)), module, SickoLooper1::LEFT_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInR, yIn)), module, SickoLooper1::RIGHT_INPUT));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xInLvl, yIn)), module, SickoLooper1::SOURCELVL_KNOB_PARAM));
		addParam(createLightParamCentered<VCVLightLatch<LargeSimpleLight<RedLight>>>(mm2px(Vec(xMute, yIn)), module, SickoLooper1::MUTE_SWITCH, SickoLooper1::MUTE_LIGHT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xMeasKnob, yMeasKnob)), module, SickoLooper1::MEAS_KNOB_PARAM));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xPlayTrig, yPlay)), module, SickoLooper1::PLAY_TRIG_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<GreenLight>>(mm2px(Vec(xPlayBut, yPlay)), module, SickoLooper1::PLAY_BUT_PARAM, SickoLooper1::PLAY_BUT_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xPlayTrig, yRec)), module, SickoLooper1::REC_TRIG_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xPlayBut, yRec)), module, SickoLooper1::REC_BUT_PARAM, SickoLooper1::REC_BUT_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xPlayTrig, yStop)), module, SickoLooper1::STOP_TRIG_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xPlayBut, yStop)), module, SickoLooper1::STOP_BUT_PARAM, SickoLooper1::STOP_BUT_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xPlayTrig, yErase)), module, SickoLooper1::ERASE_TRIG_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<YellowLight>>(mm2px(Vec(xPlayBut, yErase)), module, SickoLooper1::ERASE_BUT_PARAM, SickoLooper1::ERASE_BUT_LIGHT));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xStartImm, yStartImm)), module, SickoLooper1::STARTIMM_SWITCH, SickoLooper1::STARTIMM_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(xStopImm, yStartImm)), module, SickoLooper1::STOPIMM_SWITCH, SickoLooper1::STOPIMM_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xSyncBut, ySyncBut)), module, SickoLooper1::LOOPSYNC_SWITCH, SickoLooper1::LOOPSYNC_LIGHT));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xOneShotBut, yOneShotBut)), module, SickoLooper1::ONESHOT_SWITCH, SickoLooper1::ONESHOT_LIGHT));
		
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xRev, yRevBut)), module, SickoLooper1::REV_SWITCH, SickoLooper1::REV_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRev, yRevIn)), module, SickoLooper1::REV_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xVoct, yVoct)), module, SickoLooper1::VOCT_INPUT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xXFade, yXFade)), module, SickoLooper1::XFADE_KNOB_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPan, yPan)), module, SickoLooper1::PAN_KNOB_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xPan, yPanIn)), module, SickoLooper1::PAN_INPUT));
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xVol, yVol)), module, SickoLooper1::VOLTRACK_KNOB_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xVol, yVolIn)), module, SickoLooper1::VOLTRACK_INPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xEol, yOutput)), module, SickoLooper1::EOL_OUTPUT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xSrcToTrack, yOutput)), module, SickoLooper1::SRC_TO_TRACK_SWITCH, SickoLooper1::SRC_TO_TRACK_LIGHT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOutL, yOutput)), module, SickoLooper1::TRACK_LEFT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOutR, yOutput)), module, SickoLooper1::TRACK_RIGHT_OUTPUT));

	}


	void appendContextMenu(Menu *menu) override {
	   	SickoLooper1 *module = dynamic_cast<SickoLooper1*>(this->module);
			assert(module);

		menu->addChild(new MenuSeparator());

		struct ModeItem : MenuItem {
			SickoLooper1* module;
			int playSequence;
			void onAction(const event::Action& e) override {
				module->playSequence = playSequence;
			}
		};

		menu->addChild(createMenuLabel("PLAY Button Sequence"));
		std::string modeNames[3] = {"Play -> Stop", "Rec -> Play -> Overdub", "Rec -> Overdub -> Play"};
		for (int i = 0; i < 3; i++) {
			ModeItem* modeItem = createMenuItem<ModeItem>(modeNames[i]);
			modeItem->rightText = CHECKMARK(module->playSequence == i);
			modeItem->module = module;
			modeItem->playSequence = i;
			menu->addChild(modeItem);
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Instant STOP button", "", &module->instantStop));
		menu->addChild(createBoolPtrMenuItem("OVERDUB after REC", "", &module->overdubAfterRec));
		menu->addChild(createBoolPtrMenuItem("EOL pulse on stop", "", &module->eolPulseOnStop));

		menu->addChild(new MenuSeparator());

		struct PanItem : MenuItem {
			SickoLooper1* module;
			int panRange;
			void onAction(const event::Action& e) override {
				module->panRange = panRange;
			}
		};

		menu->addChild(createMenuLabel("PAN CV range"));
		std::string panNames[3] = {"0/10v", "+/-5v", "+/-10v"};
		for (int i = 0; i < 3; i++) {
			PanItem* panItem = createMenuItem<PanItem>(panNames[i]);
			panItem->rightText = CHECKMARK(module->panRange == i);
			panItem->module = module;
			panItem->panRange = i;
			menu->addChild(panItem);
		}

		menu->addChild(new MenuSeparator());

				menu->addChild(createSubmenuItem("Track settings", "", [=](Menu * menu) {
					
					menu->addChild(createBoolPtrMenuItem("Fade IN on playback", "", &module->fadeInOnPlay));
					menu->addChild(createBoolPtrMenuItem("Play Full Tail on Stop", "", &module->playFullTail));
		
					

					menu->addChild(new MenuSeparator());
					menu->addChild(createBoolMenuItem("Extra samples Tail (1sec)", "", [=]() {
							return module->isExtraSamples();
						}, [=](bool xtraSamples) {
							module->setExtraSamples(xtraSamples);
					}));

					if (module->trackStatus != EMPTY)
						menu->addChild(createMenuItem("Detect tempo and set bpm", "", [=]() {module->detectTempo();}));
					else
						menu->addChild(createMenuLabel("Detect tempo and set bpm"));

					menu->addChild(new MenuSeparator());
					menu->addChild(createMenuItem("Import Wav", "", [=]() {module->menuLoadSample();}));
					if (module->trackStatus != EMPTY)
						menu->addChild(createMenuItem("Export Wav", "", [=]() {module->menuSaveSample();}));
					else
						menu->addChild(createMenuLabel("Export Wav"));

				}));

		/*
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Load preset (+loops)", "", [=]() {module->menuLoadPreset();}));
		menu->addChild(createMenuItem("Save preset", "", [=]() {module->menuSavePreset(false);}));
		menu->addChild(createMenuItem("Save preset + loops", "", [=]() {module->menuSavePreset(true);}));
		*/

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolMenuItem("Internal Clock Always ON", "", [=]() {
				return module->isInternalClockAlwaysOn();
			}, [=](bool internalClockAlwaysOn) {
				module->setInternalClock(internalClockAlwaysOn);
		}));

		struct ClickItem : MenuItem {
			SickoLooper1* module;
			int clickSelect;
			void onAction(const event::Action& e) override {
				module->clickSelect = clickSelect;
				module->setClick(clickSelect);
			}
		};
		menu->addChild(createSubmenuItem("Click Settings", "", [=](Menu * menu) {

			/*
			menu->addChild(createSubmenuItem("Click Presets", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("Standard", "", [=]() {module->setClick(0);}));
				menu->addChild(createMenuItem("Click1", "", [=]() {module->setClick(1);}));
				menu->addChild(createMenuItem("Click2", "", [=]() {module->setClick(2);}));
			}));*/
			

			//menu->addChild(createMenuLabel("PLAY Button Sequence"));
			std::string clickNames[4] = {"Standard", "Click1", "Click2", "Custom"};
			for (int i = 0; i < 4; i++) {
				ClickItem* clickItem = createMenuItem<ClickItem>(clickNames[i]);
				clickItem->rightText = CHECKMARK(module->clickSelect == i);
				clickItem->module = module;
				clickItem->clickSelect = i;
				menu->addChild(clickItem);
			}

			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuItem("Custom BEAT click", "", [=]() {module->clickMenuLoadSample(0);}));
			menu->addChild(createMenuItem("File: " + module->clickFileDescription[0], "", [=]() {module->clickMenuLoadSample(0);}));
			menu->addChild(createMenuItem("", "Clear", [=]() {module->clickClearSlot(0);}));
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Custom BAR click", "", [=]() {module->clickMenuLoadSample(1);}));
			menu->addChild(createMenuItem("File: " + module->clickFileDescription[1], "", [=]() {module->clickMenuLoadSample(1);}));
			menu->addChild(createMenuItem("", "Clear", [=]() {module->clickClearSlot(1);}));
		}));	

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Add Expander", "", [=]() {module->addExpander(modelSickoLooper1Exp, this);}));

	}
};

Model *modelSickoLooper1 = createModel<SickoLooper1, SickoLooper1Widget>("SickoLooper1");
