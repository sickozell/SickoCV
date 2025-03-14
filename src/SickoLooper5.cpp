#define MAX_TRACKS 5
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

#define MONOPHONIC 0
#define POLYPHONIC 1

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

struct tpSignatureSl5 : ParamQuantity {
	std::string getDisplayValueString() override {
		const std::string valueDisplay[17] = {"2/4", "3/4", "4/4", "5/4", "6/4", "7/4", "5/8", "6/8", "7/8", "8/8", "9/8", "10/8", "11/8", "12/8", "13/8", "14/8", "15/8"};
		return valueDisplay[int(getValue())];
	}
};

struct SickoLooper5 : Module {
	enum ParamIds {
		CLOCK_RST_SWITCH,
		BPM_KNOB_PARAM,
		SIGNATURE_KNOB_PARAM,
		CLICK_BUT_PARAM,
		CLICKVOL_KNOB_PARAM,
		CLICKTOMASTER_SWITCH,
		PREROLL_BUT_PARAM,
		PREROLL_SWITCH,
		ALLSTARTSTOP_BUT_PARAM,
		//UNDOREDO_BUT_PARAM,
		EARVOL_KNOB_PARAM,
		MASTERVOL_KNOB_PARAM,

		ENUMS(SOURCELVL_KNOB_PARAM, MAX_TRACKS),
		ENUMS(MUTE_SWITCH, MAX_TRACKS),
		ENUMS(SOURCE_KNOB_PARAM, MAX_TRACKS),
		ENUMS(MEAS_KNOB_PARAM, MAX_TRACKS),
		ENUMS(LOOPSYNC_SWITCH, MAX_TRACKS),
		ENUMS(STARTIMM_SWITCH, MAX_TRACKS),
		ENUMS(STOPIMM_SWITCH, MAX_TRACKS),
		ENUMS(SOLO_SWITCH, MAX_TRACKS),
		ENUMS(ONESHOT_SWITCH, MAX_TRACKS),
		ENUMS(REV_SWITCH, MAX_TRACKS),
		ENUMS(XFADE_KNOB_PARAM, MAX_TRACKS),
		ENUMS(PAN_KNOB_PARAM, MAX_TRACKS),
		ENUMS(VOLTRACK_KNOB_PARAM, MAX_TRACKS),
		ENUMS(REC_BUT_PARAM, MAX_TRACKS),
		ENUMS(PLAY_BUT_PARAM, MAX_TRACKS),
		ENUMS(STOP_BUT_PARAM, MAX_TRACKS),
		ENUMS(ERASE_BUT_PARAM, MAX_TRACKS),
		ENUMS(SRC_TO_TRACK_SWITCH, MAX_TRACKS),
		NUM_PARAMS 
	};
	enum InputIds {
		ENUMS(LEFT_INPUT, MAX_TRACKS),
		ENUMS(RIGHT_INPUT, MAX_TRACKS),
		ENUMS(REC_TRIG_INPUT, MAX_TRACKS),
		ENUMS(PLAY_TRIG_INPUT, MAX_TRACKS),
		ENUMS(STOP_TRIG_INPUT, MAX_TRACKS),
		ENUMS(ERASE_TRIG_INPUT, MAX_TRACKS),
		EXTCLOCK_INPUT,
		ALLSTARTSTOP_TRIG_INPUT,
		ALLSTOP_TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(TRACK_LEFT_OUTPUT, MAX_TRACKS),
		ENUMS(TRACK_RIGHT_OUTPUT, MAX_TRACKS),
		ENUMS(EOL_OUTPUT, MAX_TRACKS),
		CLOCK_OUTPUT,
		EAR_LEFT_OUTPUT,
		EAR_RIGHT_OUTPUT,
		MASTER_LEFT_OUTPUT,
		MASTER_RIGHT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		CLOCK_RST_LIGHT,
		CLICK_BUT_LIGHT,
		CLICKTOMASTER_LIGHT,
		PREROLL_BUT_LIGHT,
		ALLSTARTSTOP_BUT_LIGHT,
		//UNDOREDO_BUT_LIGHT,
		ENUMS(MUTE_LIGHT, MAX_TRACKS),
		ENUMS(SOURCE_BUT_LIGHT, MAX_TRACKS),
		ENUMS(REC_BUT_LIGHT, MAX_TRACKS),
		ENUMS(PLAY_BUT_LIGHT, MAX_TRACKS),
		ENUMS(STOP_BUT_LIGHT, MAX_TRACKS),
		ENUMS(ERASE_BUT_LIGHT, MAX_TRACKS),
		ENUMS(LOOPSYNC_LIGHT, MAX_TRACKS),
		ENUMS(TEMPOSYNC_LIGHT, MAX_TRACKS),
		ENUMS(STARTIMM_LIGHT, MAX_TRACKS),
		ENUMS(STOPIMM_LIGHT, MAX_TRACKS),
		ENUMS(SOLO_LIGHT, MAX_TRACKS),
		ENUMS(ONESHOT_LIGHT, MAX_TRACKS),
		ENUMS(REV_LIGHT, MAX_TRACKS),
		ENUMS(SRC_TO_TRACK_LIGHT, MAX_TRACKS),
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
	bool preRoll = false;
	int preRollCount = 0;

	bool barReached = false;
	bool barChecked[5] = {false, false, false, false, false};
	bool loopEnd[5] = {false, false, false, false, false};
	bool stopNow[5] = {false, false, false, false, false};
	bool startNow[5] = {false, false, false, false, false};

	float allSSTrig = 0.f;
	float prevAllSSTrig = 0.f;

	float allStopTrig = 0.f;
	float prevAllStopTrig = 0.f;
	bool allStop = false;
	
	const std::string signatureDisplay[17] = {"2/4", "3/4", "4/4", "5/4", "6/4", "7/4", "5/8", "6/8", "7/8", "8/8", "9/8", "10/8", "11/8", "12/8", "13/8", "14/8", "15/8"};

	float extraRecMaxSamples = 0.f;

	int currentSoloTrack = -1;
	int nextSoloTrack = -1;
	bool startNewSolo = false;

	float panKnobValue = 0.f;
	float panLeftCoeff = 1.f;
	float panRightCoeff = 1.f;

	//**************************************************************
	// TRACKS
	
	int trackStatus[5] = {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY};
	int nextStatus[5] = {NOTHING, NOTHING, NOTHING, NOTHING, NOTHING};
	bool trackRecorded[5] = {false, false, false, false, false};
	vector<float> trackBuffer[5][2];

	drwav_uint64 totalSampleC[5] = {0, 0, 0, 0, 0};
	drwav_uint64 totalSamples[5] = {0, 0, 0, 0, 0};

	double samplePos[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
	int currentSource[5] = {0, 0, 0, 0, 0};
	int playingDirection[5] = {0, 0, 0, 0, 0};

	float playTrig[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	float prevPlayTrig[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	float recTrig[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	float prevRecTrig[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	float stopTrig[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	float prevStopTrig[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	float eraseTrig[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	float prevEraseTrig[5] = {0.f, 0.f, 0.f, 0.f, 0.f};

	float inputValue[5][2];
	float volTrack[5] = {0.f, 0.f, 0.f, 0.f, 0.f};

	drwav_uint64 extraRecPos[5] = {0, 0, 0, 0, 0};
	drwav_uint64 extraPlayPos[5] = {0, 0, 0, 0, 0};
	int extraRecCount[5] = {0, 0, 0, 0, 0};
	int extraRecDirection[5] = {0, 0, 0, 0, 0};
	int extraPlayDirection[5] = {0, 0, 0, 0, 0};

	float mute[5] = {1.f, 1.f, 1.f, 1.f, 1.f};
	float prevMute[5] = {1.f, 1.f, 1.f, 1.f, 1.f};
	bool muteFade[5] = {false, false, false, false, false};
	float muteValue[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	float muteDelta[5] = {0.f, 0.f, 0.f, 0.f, 0.f};

	float srcToTrack[5] = {1.f, 1.f, 1.f, 1.f, 1.f};
	float prevSrcToTrack[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	bool srcToTrackFade[5] = {false, false, false, false, false};
	float srcToTrackValue[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	float srcToTrackDelta[5] = {0.f, 0.f, 0.f, 0.f, 0.f};

	float xFadeValue[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	float xFadeDelta[5] = {1.f, 1.f, 1.f, 1.f, 1.f};

	bool fadeIn[5] = {false, false, false, false, false};
	float fadeInValue[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	float fadeInDelta[5] = {1.f, 1.f, 1.f, 1.f, 1.f};

	bool recFade[5] = {false, false, false, false, false};
	float recFadeValue[5] = {1.f, 1.f, 1.f, 1.f, 1.f};
	float recFadeDelta[5] = {0.f, 0.f, 0.f, 0.f, 0.f};

	bool fadeTail[5] = {false, false, false, false, false};
	float fadeTailValue[5] = {1.f, 1.f, 1.f, 1.f, 1.f};
	double tailEnd[5] = {0, 0, 0, 0, 0};

	bool eraseWait[5] = {false, false, false, false, false};
	float eraseTime[5] = {0, 0, 0, 0, 0};
	float eraseSamples = APP->engine->getSampleRate() / 1.5f;

	bool eolPulse[5] = {false, false, false, false, false};
	float eolPulseTime[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
	bool notEolPulse[5] = {false, false, false, false, false};

	int trackLoopMeas[5] = {0, 0, 0, 0, 0};
	int loopCount[5] = {0, 0, 0, 0, 0};

	bool extraRecording[5] = {false, false, false, false, false};
	bool extraPlaying[5] = {false, false, false, false, false};
	bool extraPlayingFadeOut[5] = {false, false, false, false, false};

	bool playTail[5] = {false, false, false, false, false};

	// *************************************************************
	// OUTPUTS

	float earLevel = 0;
	float masterLevel = 0.f;

	float currentOutput[5][2] = {{0.f, 0.f},{0.f, 0.f},{0.f, 0.f},{0.f, 0.f},{0.f, 0.f},};
	float sumOutput[2] = {0.f, 0.f};
	float earOutput[2] = {0.f, 0.f};

	//**************************************************************
	// FILE MANAGEMENT

	bool fileLoaded = false;
	bool fileFound = false;

	// *************************************************************
	// JSON variables

	bool srcToMaster = true;
	bool onlyClickOnEar = false;
	bool eolPulseOnStop = false;
	int playSequence = 0;
	bool instantStop = false;
	bool overdubAfterRec = false;
	bool fadeInOnPlay[5] = {false, false, false, false, false};
	bool extraSamples[5] = {true, true, true, true, true};
	bool playFullTail[5] = {false, false, false, false, false};

	// ***************************************************************************************************
	// exponential time knkobs

	//static constexpr float minStageTime = 1.f;  // in milliseconds
	//static constexpr float maxStageTime = 10000.f;  // in milliseconds

	// ***************************************************************************************************
	// PANEL settings

	float play_but[5] = {0.f,0.f,0.f,0.f,0.f};
	float rec_but[5] = {0.f,0.f,0.f,0.f,0.f};
	float stop_but[5] = {0.f,0.f,0.f,0.f,0.f};
	float erase_but[5] = {0.f,0.f,0.f,0.f,0.f};
	float loopSync_setting[5] = {0.f,0.f,0.f,0.f,0.f};
	float startImm_setting[5] = {0.f,0.f,0.f,0.f,0.f};
	float stopImm_setting[5] = {0.f,0.f,0.f,0.f,0.f};
	float solo_setting[5] = {0.f,0.f,0.f,0.f,0.f};
	float prevSolo_setting[5] = {-1.f, -1.f, -1.f, -1.f, -1.f};
	float oneShot_setting[5] = {0.f,0.f,0.f,0.f,0.f};
	float rev_setting[5] = {0.f,0.f,0.f,0.f,0.f};

	// ***************************************************************************************************
	// ***************************************************************************************************
	//                                         C   L  O  C  K
	// ***************************************************************************************************
	// ***************************************************************************************************

	const int beatMaxPerBar[17] = {2, 3, 4, 5, 6, 7, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	int currentBeatMaxPerBar = 4;

	float rstBarBut = 0.f;
	float prevRstBarBut = 1.f;

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

	//float minTimeSamples = (APP->engine->getSampleRate()) / 5;	// use this for fade testing (200ms)
	float minTimeSamples = (APP->engine->getSampleRate()) / 125.f;		// samples in 8ms
	float tailSamples = APP->engine->getSampleRate();					// samples in one second
	float fadeTailDelta = 1.f / minTimeSamples;
	float oneMsSamples = (APP->engine->getSampleRate()) / 1000;			// samples in 1ms

	float fastPulseTime = (APP->engine->getSampleRate()) / 50;
	float slowPulseTime = (APP->engine->getSampleRate()) / 5;

	int playButtonPulse[5] = {NO_PULSE, NO_PULSE, NO_PULSE, NO_PULSE, NO_PULSE};
	float playButtonPulseTime[5] = {0.f, 0.f, 0.f, 0.f, 0.f};

	int recButtonPulse[5] = {NO_PULSE, NO_PULSE, NO_PULSE, NO_PULSE, NO_PULSE};
	float recButtonPulseTime[5] = {0.f, 0.f, 0.f, 0.f, 0.f};

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

	SickoLooper5() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configInput(LEFT_INPUT+0,"Left #1");
		configInput(RIGHT_INPUT+0,"Right #1");
		configInput(LEFT_INPUT+1,"Left #2");
		configInput(RIGHT_INPUT+1,"Right #2");
		configInput(LEFT_INPUT+2,"Left #3");
		configInput(RIGHT_INPUT+2,"Right #3");
		configInput(LEFT_INPUT+3,"Left #4");
		configInput(RIGHT_INPUT+3,"Right #4");
		configInput(LEFT_INPUT+4,"Left #5");
		configInput(RIGHT_INPUT+4,"Right #5");

		configInput(EXTCLOCK_INPUT,"External Clock");
		configSwitch(CLOCK_RST_SWITCH, 0.f, 1.f, 0.f, "Bar Reset", {"OFF", "ON"});
		configOutput(CLOCK_OUTPUT,"Clock");

		configParam(BPM_KNOB_PARAM, 300.f, 3000.f, 1200.f, "Tempo", " bpm", 0, 0.1);
		paramQuantities[BPM_KNOB_PARAM]->snapEnabled = true;

		configParam<tpSignatureSl5>(SIGNATURE_KNOB_PARAM, 0.f, 16.0f, 2.f, "Time Signature");
		paramQuantities[SIGNATURE_KNOB_PARAM]->snapEnabled = true;

		configInput(ALLSTARTSTOP_TRIG_INPUT,"All Start/Stop Trig");
		configSwitch(ALLSTARTSTOP_BUT_PARAM, 0.f, 1.f, 0.f, "All Start/Stop", {"OFF", "ON"});
		configInput(ALLSTOP_TRIG_INPUT,"All Stop Trig");
		//configSwitch(UNDOREDO_BUT_PARAM, 0.f, 1.f, 0.f, "Undo/Redo", {"OFF", "ON"});

		configSwitch(CLICK_BUT_PARAM, 0.f, 1.f, 1.f, "Click", {"OFF", "ON"});
		configParam(CLICKVOL_KNOB_PARAM, 0.f, 2.0f, 1.f, "Click Volume", "%", 0, 100);
		configSwitch(CLICKTOMASTER_SWITCH, 0.f, 1.f, 0.f, "Click to Master", {"Off", "On"});

		configSwitch(PREROLL_BUT_PARAM, 0.f, 1.f, 0.f, "Pre-Roll", {"OFF", "ON"});
		configSwitch(PREROLL_SWITCH, 1.f, 2.f, 1.f, "Pre-Roll", {"1 bar", "2 bar"});
		
		configParam(EARVOL_KNOB_PARAM, 0.f, 1.0f, 1.f, "Ear Volume", "%", 0, 100);
		configOutput(EAR_LEFT_OUTPUT,"Ear Left");
		configOutput(EAR_RIGHT_OUTPUT,"Ear Right");
		configParam(MASTERVOL_KNOB_PARAM, 0.f, 1.0f, 1.f, "Master Volume", "%", 0, 100);
		configOutput(MASTER_LEFT_OUTPUT,"Master Left");
		configOutput(MASTER_RIGHT_OUTPUT,"Master Right");

		configParam(SOURCE_KNOB_PARAM, 1.f, 5.f, 1.f, "Source Input");
		paramQuantities[SOURCE_KNOB_PARAM]->snapEnabled = true;
		configParam(SOURCE_KNOB_PARAM+1, 1.f, 5.f, 2.f, "Source Input");
		paramQuantities[SOURCE_KNOB_PARAM+1]->snapEnabled = true;
		configParam(SOURCE_KNOB_PARAM+2, 1.f, 5.f, 3.f, "Source Input");
		paramQuantities[SOURCE_KNOB_PARAM+2]->snapEnabled = true;
		configParam(SOURCE_KNOB_PARAM+3, 1.f, 5.f, 4.f, "Source Input");
		paramQuantities[SOURCE_KNOB_PARAM+3]->snapEnabled = true;
		configParam(SOURCE_KNOB_PARAM+4, 1.f, 5.f, 5.f, "Source Input");
		paramQuantities[SOURCE_KNOB_PARAM+4]->snapEnabled = true;

		for (int i=0; i < MAX_TRACKS; i++) {

			configParam(SOURCELVL_KNOB_PARAM+i, 0.f, 2.0f, 1.f, "Source Level", "%", 0, 100);
			configSwitch(MUTE_SWITCH+i, 0.f, 1.f, 1.f, "Mute", {"Mute OFF", "Mute ON"});

			configParam(MEAS_KNOB_PARAM+i, 1.f, 16.f, 1.f, "Loop Measure");
			paramQuantities[MEAS_KNOB_PARAM+i]->snapEnabled = true;

			configInput(PLAY_TRIG_INPUT+i,"Play Trig");
			configSwitch(PLAY_BUT_PARAM+i, 0.f, 1.f, 0.f, "Play", {"OFF", "ON"});

			configInput(REC_TRIG_INPUT+i,"Rec Trig");
			configSwitch(REC_BUT_PARAM+i, 0.f, 1.f, 0.f, "Rec", {"OFF", "ON"});

			configInput(STOP_TRIG_INPUT+i,"Stop Trig");
			configSwitch(STOP_BUT_PARAM+i, 0.f, 1.f, 0.f, "Stop", {"OFF", "ON"});

			configInput(ERASE_TRIG_INPUT+i,"Erase Trig");
			configSwitch(ERASE_BUT_PARAM+i, 0.f, 1.f, 0.f, "Erase", {"OFF", "ON"});

			configSwitch(LOOPSYNC_SWITCH+i, 0.f, 1.f, 1.f, "Loop Sync", {"Off", "On"});

			configSwitch(STARTIMM_SWITCH+i, 0.f, 1.f, 0.f, "Start Immediately", {"Off", "On"});
			configSwitch(STOPIMM_SWITCH+i, 0.f, 1.f, 0.f, "Stop Immediately", {"Off", "On"});

			configSwitch(SOLO_SWITCH+i, 0.f, 1.f, 0.f, "S/M", {"Multi", "Solo"});

			configSwitch(REV_SWITCH+i, 0.f, 1.f, 0.f, "Reverse", {"Off", "On"});

			configSwitch(ONESHOT_SWITCH+i, 0.f, 1.f, 0.f, "1 shot", {"Off", "On"});

			configParam(XFADE_KNOB_PARAM+i, 0.f, 1000.f, 8.f, "Crossfade", "ms");
		
			configParam(PAN_KNOB_PARAM+i, -1.f, 1.f, 0.f, "Pan");

			configParam(VOLTRACK_KNOB_PARAM+i, 0.f, 1.f, 1.f, "Track Volume", "%", 0, 100);

			configOutput(EOL_OUTPUT+i,"End of Loop");

			configSwitch(SRC_TO_TRACK_SWITCH+i, 0.f, 1.f, 1.f, "Source to Track Out", {"Off", "On"});

			configOutput(TRACK_LEFT_OUTPUT+i,"Track Left");
			configOutput(TRACK_RIGHT_OUTPUT+i,"Track Right");

		}

		trackBuffer[0][0].resize(0);
		trackBuffer[0][1].resize(0);
		trackBuffer[1][0].resize(0);
		trackBuffer[1][1].resize(0);
		trackBuffer[2][0].resize(0);
		trackBuffer[2][1].resize(0);
		trackBuffer[3][0].resize(0);
		trackBuffer[3][1].resize(0);
		trackBuffer[4][0].resize(0);
		trackBuffer[4][1].resize(0);
		//undoBuffer[0].resize(0);
		//undoBuffer[1].resize(0);
		
		// metamodule change
		//tempBuffer[0].resize(0);
		//tempBuffer[1].resize(0);

		setClick(0);

	}

	/*
	static float convertCVToSeconds(float cv) {		
		return minStageTime * std::pow(maxStageTime / minStageTime, cv) / 1000;
	}
	*/

//																											░░░░░██╗░██████╗░█████╗░███╗░░██╗
//																											░░░░░██║██╔════╝██╔══██╗████╗░██║
//																											░░░░░██║╚█████╗░██║░░██║██╔██╗██║
//																											██╗░░██║░╚═══██╗██║░░██║██║╚████║
//																											╚█████╔╝██████╔╝╚█████╔╝██║░╚███║
//																											░╚════╝░╚═════╝░░╚════╝░╚═╝░░╚══╝


	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "SrcToMaster", json_boolean(srcToMaster));
		json_object_set_new(rootJ, "OnlyClickOnEar", json_boolean(onlyClickOnEar));
		json_object_set_new(rootJ, "EolPulseOnStop", json_boolean(eolPulseOnStop));
		json_object_set_new(rootJ, "playSequence", json_integer(playSequence));
		json_object_set_new(rootJ, "InstantStop", json_boolean(instantStop));
		json_object_set_new(rootJ, "overdubAfterRec", json_boolean(overdubAfterRec));
		json_object_set_new(rootJ, "extraSamples0", json_boolean(extraSamples[0]));
		json_object_set_new(rootJ, "extraSamples1", json_boolean(extraSamples[1]));
		json_object_set_new(rootJ, "extraSamples2", json_boolean(extraSamples[2]));
		json_object_set_new(rootJ, "extraSamples3", json_boolean(extraSamples[3]));
		json_object_set_new(rootJ, "extraSamples4", json_boolean(extraSamples[4]));
		json_object_set_new(rootJ, "playFullTail0", json_boolean(playFullTail[0]));
		json_object_set_new(rootJ, "playFullTail1", json_boolean(playFullTail[1]));
		json_object_set_new(rootJ, "playFullTail2", json_boolean(playFullTail[2]));
		json_object_set_new(rootJ, "playFullTail3", json_boolean(playFullTail[3]));
		json_object_set_new(rootJ, "playFullTail4", json_boolean(playFullTail[4]));
		json_object_set_new(rootJ, "fadeInOnPlay0", json_boolean(fadeInOnPlay[0]));
		json_object_set_new(rootJ, "fadeInOnPlay1", json_boolean(fadeInOnPlay[1]));
		json_object_set_new(rootJ, "fadeInOnPlay2", json_boolean(fadeInOnPlay[2]));
		json_object_set_new(rootJ, "fadeInOnPlay3", json_boolean(fadeInOnPlay[3]));
		json_object_set_new(rootJ, "fadeInOnPlay4", json_boolean(fadeInOnPlay[4]));
		json_object_set_new(rootJ, "internalClockAlwaysOn", json_boolean(internalClockAlwaysOn));
		json_object_set_new(rootJ, "ClickSlot1", json_string(clickStoredPath[0].c_str()));
		json_object_set_new(rootJ, "ClickSlot2", json_string(clickStoredPath[1].c_str()));
		json_object_set_new(rootJ, "clickSelect", json_integer(clickSelect));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {

		json_t* srcToMasterJ = json_object_get(rootJ, "SrcToMaster");
		if (srcToMasterJ)
			srcToMaster = json_boolean_value(srcToMasterJ);
		json_t* onlyClickOnEarJ = json_object_get(rootJ, "OnlyClickOnEar");
		if (onlyClickOnEarJ)
			onlyClickOnEar = json_boolean_value(onlyClickOnEarJ);
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

		json_t* extraSamples0J = json_object_get(rootJ, "extraSamples0");
		if (extraSamples0J)
			extraSamples[0] = json_boolean_value(extraSamples0J);
		json_t* extraSamples1J = json_object_get(rootJ, "extraSamples1");
		if (extraSamples1J)
			extraSamples[1] = json_boolean_value(extraSamples1J);
		json_t* extraSamples2J = json_object_get(rootJ, "extraSamples2");
		if (extraSamples2J)
			extraSamples[2] = json_boolean_value(extraSamples2J);
		json_t* extraSamples3J = json_object_get(rootJ, "extraSamples3");
		if (extraSamples3J)
			extraSamples[3] = json_boolean_value(extraSamples3J);
		json_t* extraSamples4J = json_object_get(rootJ, "extraSamples4");
		if (extraSamples4J)
			extraSamples[4] = json_boolean_value(extraSamples4J);

		json_t* playFullTail0J = json_object_get(rootJ, "playFullTail0");
		if (playFullTail0J)
			playFullTail[0] = json_boolean_value(playFullTail0J);
		json_t* playFullTail1J = json_object_get(rootJ, "playFullTail1");
		if (playFullTail1J)
			playFullTail[1] = json_boolean_value(playFullTail1J);
		json_t* playFullTail2J = json_object_get(rootJ, "playFullTail2");
		if (playFullTail2J)
			playFullTail[2] = json_boolean_value(playFullTail2J);

		json_t* playFullTail3J = json_object_get(rootJ, "playFullTail3");
		if (playFullTail3J)
			playFullTail[3] = json_boolean_value(playFullTail3J);
		json_t* playFullTail4J = json_object_get(rootJ, "playFullTail4");
		if (playFullTail4J)
			playFullTail[4] = json_boolean_value(playFullTail4J);

		json_t* fadeInOnPlay0J = json_object_get(rootJ, "fadeInOnPlay0");
		if (fadeInOnPlay0J)
			fadeInOnPlay[0] = json_boolean_value(fadeInOnPlay0J);
		json_t* fadeInOnPlay1J = json_object_get(rootJ, "fadeInOnPlay1");
		if (fadeInOnPlay1J)
			fadeInOnPlay[1] = json_boolean_value(fadeInOnPlay1J);
		json_t* fadeInOnPlay2J = json_object_get(rootJ, "fadeInOnPlay2");
		if (fadeInOnPlay2J)
			fadeInOnPlay[2] = json_boolean_value(fadeInOnPlay2J);
		json_t* fadeInOnPlay3J = json_object_get(rootJ, "fadeInOnPlay3");
		if (fadeInOnPlay3J)
			fadeInOnPlay[3] = json_boolean_value(fadeInOnPlay3J);
		json_t* fadeInOnPlay4J = json_object_get(rootJ, "fadeInOnPlay4");
		if (fadeInOnPlay4J)
			fadeInOnPlay[4] = json_boolean_value(fadeInOnPlay4J);

		json_t* internalClockAlwaysOnJ = json_object_get(rootJ, "internalClockAlwaysOn");
		if (internalClockAlwaysOnJ) {
			internalClockAlwaysOn = json_boolean_value(internalClockAlwaysOnJ);
			setInternalClock(internalClockAlwaysOn);
		}

		/*
		json_t *clickSlot1J = json_object_get(rootJ, "ClickSlot1");
		if (clickSlot1J) {
			clickStoredPath[0] = json_string_value(clickSlot1J);
			clickLoadSample(clickStoredPath[0], 0);
		}
		json_t *clickSlot2J = json_object_get(rootJ, "ClickSlot2");
		if (clickSlot2J) {
			clickStoredPath[1] = json_string_value(clickSlot2J);
			clickLoadSample(clickStoredPath[1], 1);
		}
		*/
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

		srcToMaster = true;
		onlyClickOnEar = false;
		eolPulseOnStop = false;
		playSequence = 0;
		instantStop = false;
		overdubAfterRec = false;
		recordedTracks = 0;
		for (int track = 0; track < MAX_TRACKS; track++) {
			extraSamples[track] = true;
			playTail[track] = false;
			playFullTail[track] = false;
			fadeTail[track] = false;
			fadeInOnPlay[track] = false;
			trackBuffer[track][LEFT].clear();
			trackBuffer[track][RIGHT].clear();

			// metamodule change
			vector<float>().swap(trackBuffer[track][LEFT]);
			vector<float>().swap(trackBuffer[track][RIGHT]);

			trackRecorded[track] = false;
			trackStatus[track] = EMPTY;
			setEmptyLed(track);
		}

		busyTracks = 0;
		currentSoloTrack = -1;
		nextSoloTrack = -1;
		startNewSolo = false;
		internalClockAlwaysOn = false;

		Module::onReset(e);
	}

	void onAdd(const AddEvent& e) override {
		std::string path ;
		for (int track = 0; track < MAX_TRACKS; track++) {
			path = system::join(getPatchStorageDirectory(), ("track"+to_string(track+1)+".wav").c_str());
			loadSample(track, path);
			if (fileLoaded) {
				trackRecorded[track] = true;
				recordedTracks++;
			}
		}
		
		Module::onAdd(e);
	}

	void onSave(const SaveEvent& e) override {
		std::string path;
		system::removeRecursively(getPatchStorageDirectory().c_str());

		for (int track = 0; track < MAX_TRACKS; track++) {
			if (trackStatus[track] != EMPTY) {
				path = system::join(createPatchStorageDirectory(), ("track"+to_string(track+1)+".wav").c_str());
				saveSample(track, path);
			}
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

		prevSampleRate = sampleRate;
		sampleRate = APP->engine->getSampleRate();

		sampleRateCoeff = (double)sampleRate * 60;
		oneMsSamples = sampleRate / 1000;
		//minTimeSamples = sampleRate / 166.67f;	// samples in 6ms
		minTimeSamples = sampleRate / 125.f;		// samples in 8ms
		//minTimeSamples = (APP->engine->getSampleRate()) / 5;	// use this for fade testing
		tailSamples = sampleRate;
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

		for (int track = 0; track < MAX_TRACKS; track++) {

			if (trackStatus[track] != EMPTY) {
				double resampleCoeff;

				z1 = 0; z2 = 0; z1r = 0; z2r = 0;

				// metamodule change
				//tempBuffer[0].clear();
				//tempBuffer[1].clear();
				vector<float> tempBuffer[2];

				for (unsigned int i=0; i < trackBuffer[track][LEFT].size(); i++) {
					tempBuffer[LEFT].push_back(trackBuffer[track][LEFT][i]);
					tempBuffer[LEFT].push_back(0);
					tempBuffer[RIGHT].push_back(trackBuffer[track][RIGHT][i]);
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

				trackBuffer[track][LEFT].clear();
				trackBuffer[track][RIGHT].clear();

				// metamodule change
				vector<float>().swap(trackBuffer[track][LEFT]);
				vector<float>().swap(trackBuffer[track][RIGHT]);

				resampleCoeff = double(prevSampleRate) / double(sampleRate);
				
				double currResamplePos = 0;
				double floorCurrResamplePos = 0;

				trackBuffer[track][LEFT].push_back(tempBuffer[LEFT][0]);
				trackBuffer[track][RIGHT].push_back(tempBuffer[RIGHT][0]);

				currResamplePos += resampleCoeff;
				floorCurrResamplePos = floor(currResamplePos);
				int temp;

				while ( floorCurrResamplePos < 1 ) {
					temp = tempBuffer[LEFT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[LEFT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					trackBuffer[track][LEFT].push_back(temp);
					temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
							tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					trackBuffer[track][RIGHT].push_back(temp);

					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				} 

				while ( currResamplePos < tempSamples - 1) {
					trackBuffer[track][LEFT].push_back(	hermiteInterpol(tempBuffer[LEFT][floorCurrResamplePos - 1],
																	tempBuffer[LEFT][floorCurrResamplePos],
																	tempBuffer[LEFT][floorCurrResamplePos + 1],
																	tempBuffer[LEFT][floorCurrResamplePos + 2],
																	currResamplePos - floorCurrResamplePos)
													);

					trackBuffer[track][RIGHT].push_back( hermiteInterpol(tempBuffer[RIGHT][floorCurrResamplePos - 1],
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
					trackBuffer[track][LEFT].push_back(temp);

					temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
							tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					trackBuffer[track][RIGHT].push_back(temp);

					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				}

				tempBuffer[LEFT].clear();
				tempBuffer[RIGHT].clear();

				totalSampleC[track] = trackBuffer[track][LEFT].size();
				totalSamples[track] = totalSampleC[track]-1;

				calcBiquadLpf(20000.0, sampleRate * 2, 1);
				for (unsigned int i = 0; i < totalSampleC[track]; i = i + 2) {
					tempBuffer[LEFT].push_back(biquadLpf(trackBuffer[track][LEFT][i]));
					tempBuffer[RIGHT].push_back(biquadLpf2(trackBuffer[track][RIGHT][i]));
				}

				tempSampleC = tempBuffer[LEFT].size();
				tempSamples = tempSampleC-1;

				trackBuffer[track][LEFT].clear();
				trackBuffer[track][RIGHT].clear();

				// metamodule change
				vector<float>().swap(trackBuffer[track][LEFT]);
				vector<float>().swap(trackBuffer[track][RIGHT]);

				for (unsigned int i = 0; i < tempSampleC; i++) {
					trackBuffer[track][LEFT].push_back(tempBuffer[LEFT][i]);
					trackBuffer[track][RIGHT].push_back(tempBuffer[RIGHT][i]);
				}
				totalSampleC[track] = trackBuffer[track][LEFT].size();
				totalSamples[track] = totalSampleC[track]-1;

				if (extraSamples[track])
					totalSampleC[track] = trackBuffer[track][LEFT].size() - tailSamples;
				else
					totalSampleC[track] = trackBuffer[track][LEFT].size();
				totalSamples[track] = totalSampleC[track]-1;
			}
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
	void menuSaveSample(int track) {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path)
			saveSample(track, path);

		free(path);
#if defined(METAMODULE)
		});
#endif
	};

	void saveSample(int track, std::string path) {
		drwav_uint64 samples;

		samples = trackBuffer[track][LEFT].size(); // da rivedere se è questo o diminuito di 1

		std::vector<float> data;

		for (unsigned int i = 0; i <= trackBuffer[track][LEFT].size(); i++) {
			data.push_back(trackBuffer[track][LEFT][i] / 5);
			data.push_back(trackBuffer[track][RIGHT][i] / 5);
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

		drwav *pWav = drwav_open_file_write(path.c_str(), &format);
		drwav_write(pWav, samples, data.data());
		drwav_close(pWav);

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


	void menuLoadSample(int track) {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV;All files (*.*):*.*";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		if (path)
			loadSample(track, path);

		if (fileLoaded) {
			if (!trackRecorded[track]) {
				trackRecorded[track] = true;
				recordedTracks++;
			}
		}

		free(path);

		fileLoaded = false;
#if defined(METAMODULE)
		});
#endif
	}

	void loadSample(int track, std::string path) {
		z1 = 0; z2 = 0; z1r = 0; z2r = 0;

		fileLoaded = false;

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

			// stop track
			if ((trackStatus[track] != IDLE && trackStatus[track] != EMPTY) || nextStatus[track] != NOTHING)
				busyTracks--;
			if (track == nextSoloTrack)
				nextSoloTrack = -1;
			if (track == currentSoloTrack)
				currentSoloTrack = -1;
			trackStatus[track] = EMPTY;
			nextStatus[track] = NOTHING;
			samplePos[track] = 0;
			setEmptyLed(track);

			fileChannels = c;

			unsigned int fileSampleRate = sr;

			samplePos[track] = 0;

			trackBuffer[track][LEFT].clear();
			trackBuffer[track][RIGHT].clear();
			tempBuffer[LEFT].clear();
			tempBuffer[RIGHT].clear();

			// metamodule change
			vector<float>().swap(trackBuffer[track][LEFT]);
			vector<float>().swap(trackBuffer[track][RIGHT]);

			/*
			if (tsc > 52428800)
				tsc = 52428800;	// set memory allocation limit to 200Mb for samples (~18mins at 48.000khz MONO)
			*/

			if (tsc > recordingLimit)
				tsc = recordingLimit;

			if (fileSampleRate == sampleRate) {			//  **************************   NO RESAMPLE   ************************
				for (unsigned int i=0; i < tsc; i = i + c) {
					trackBuffer[track][LEFT].push_back(pSampleData[i] * 5);
					if (fileChannels == 2)
						trackBuffer[track][RIGHT].push_back(pSampleData[i+1] * 5);
					else
						trackBuffer[track][RIGHT].push_back(pSampleData[i] * 5);
				}

				if (extraSamples[track])
					totalSampleC[track] = trackBuffer[track][LEFT].size() - tailSamples;
				else
					totalSampleC[track] = trackBuffer[track][LEFT].size();
				totalSamples[track] = totalSampleC[track]-1;
				drwav_free(pSampleData);

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

				drwav_free(pSampleData);

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

				trackBuffer[track][LEFT].push_back(tempBuffer[LEFT][0]);
				trackBuffer[track][RIGHT].push_back(tempBuffer[RIGHT][0]);

				currResamplePos += resampleCoeff;
				floorCurrResamplePos = floor(currResamplePos);
				int temp;

				//INFO("[ sickoCV ] ARRIVATO QUI 0 %s\n",(to_string(resampleCoeff)).c_str());

				while ( floorCurrResamplePos < 1 ) {
					temp = tempBuffer[LEFT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[LEFT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					trackBuffer[track][LEFT].push_back(temp);

					temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
							tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					trackBuffer[track][RIGHT].push_back(temp);
					
					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				} 

				while ( currResamplePos < tempSamples - 1) {
					trackBuffer[track][LEFT].push_back(	hermiteInterpol(tempBuffer[LEFT][floorCurrResamplePos - 1],
																	tempBuffer[LEFT][floorCurrResamplePos],
																	tempBuffer[LEFT][floorCurrResamplePos + 1],
																	tempBuffer[LEFT][floorCurrResamplePos + 2],
																	currResamplePos - floorCurrResamplePos)
													);

					trackBuffer[track][RIGHT].push_back( hermiteInterpol(tempBuffer[RIGHT][floorCurrResamplePos - 1],
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
					trackBuffer[track][LEFT].push_back(temp);

					temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
							tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					trackBuffer[track][RIGHT].push_back(temp);

					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				}

				tempBuffer[LEFT].clear();
				tempBuffer[RIGHT].clear();

				// ***************************************************************************

				totalSampleC[track] = trackBuffer[track][LEFT].size();
				totalSamples[track] = totalSampleC[track]-1;

				calcBiquadLpf(20000.0, sampleRate * 2, 1);
				for (unsigned int i = 0; i < totalSampleC[track]; i = i + 2) {
					tempBuffer[LEFT].push_back(biquadLpf(trackBuffer[track][LEFT][i]));
					tempBuffer[RIGHT].push_back(biquadLpf2(trackBuffer[track][RIGHT][i]));
				}

				tempSampleC = tempBuffer[LEFT].size();
				tempSamples = tempSampleC-1;

				trackBuffer[track][LEFT].clear();
				trackBuffer[track][RIGHT].clear();

				// metamodule change
				vector<float>().swap(trackBuffer[track][LEFT]);
				vector<float>().swap(trackBuffer[track][RIGHT]);

				for (unsigned int i = 0; i < tempSampleC; i++) {
					trackBuffer[track][LEFT].push_back(tempBuffer[LEFT][i]);
					trackBuffer[track][RIGHT].push_back(tempBuffer[RIGHT][i]);
				}
				totalSampleC[track] = trackBuffer[track][LEFT].size();
				totalSamples[track] = totalSampleC[track]-1;
			}

			if (extraSamples[track])
				totalSampleC[track] = trackBuffer[track][LEFT].size() - tailSamples;
			else
				totalSampleC[track] = trackBuffer[track][LEFT].size();
			totalSamples[track] = totalSampleC[track]-1;

			fileLoaded = true;
			trackStatus[track] = IDLE;
			setIdleLed(track);
		} 
	};


//
//																							██████╗░██████╗░███████╗░██████╗███████╗████████╗
//																							██╔══██╗██╔══██╗██╔════╝██╔════╝██╔════╝╚══██╔══╝
//																							██████╔╝██████╔╝█████╗░░╚█████╗░█████╗░░░░░██║░░░
//																							██╔═══╝░██╔══██╗██╔══╝░░░╚═══██╗██╔══╝░░░░░██║░░░
//																							██║░░░░░██║░░██║███████╗██████╔╝███████╗░░░██║░░░
//																							╚═╝░░░░░╚═╝░░╚═╝╚══════╝╚═════╝░╚══════╝░░░╚═╝░░░

	json_t *presetToJson() {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "bpm", json_integer(int(bpm*10)));
		json_object_set_new(rootJ, "meter", json_integer(int(params[SIGNATURE_KNOB_PARAM].getValue())));
		json_object_set_new(rootJ, "srcToMaster", json_boolean(srcToMaster));
		json_object_set_new(rootJ, "onlyClickOnEar", json_boolean(onlyClickOnEar));
		json_object_set_new(rootJ, "eolPulseOnStop", json_boolean(eolPulseOnStop));
		json_object_set_new(rootJ, "playSequence", json_integer(playSequence));
		json_object_set_new(rootJ, "instantStop", json_boolean(instantStop));
		json_object_set_new(rootJ, "overdubAfterRec", json_boolean(overdubAfterRec));
		json_object_set_new(rootJ, "internalClockAlwaysOn", json_boolean(internalClockAlwaysOn));
		json_object_set_new(rootJ, "clickSlot1", json_string(clickStoredPath[0].c_str()));
		json_object_set_new(rootJ, "clickSlot2", json_string(clickStoredPath[1].c_str()));
		json_object_set_new(rootJ, "clickSelect", json_integer(clickSelect));
		json_object_set_new(rootJ, "click", json_integer(int(params[CLICK_BUT_PARAM].getValue())));
		json_object_set_new(rootJ, "clickVol", json_real(params[CLICKVOL_KNOB_PARAM].getValue()));
		json_object_set_new(rootJ, "clickToMaster", json_integer(int(params[CLICKTOMASTER_SWITCH].getValue())));
		json_object_set_new(rootJ, "preRoll", json_boolean(preRoll));
		json_object_set_new(rootJ, "preRollBars", json_integer((int)params[PREROLL_SWITCH].getValue()));
		json_object_set_new(rootJ, "earLevel", json_real(earLevel));
		json_object_set_new(rootJ, "masterLevel", json_real(masterLevel));
		for (int track = 0; track < MAX_TRACKS; track++){
			json_object_set_new(rootJ, ("sourceLvl"+to_string(track)).c_str(), json_real(params[SOURCELVL_KNOB_PARAM+track].getValue()));
			json_object_set_new(rootJ, ("sourceMute"+to_string(track)).c_str(), json_integer(int(mute[track])));
			json_object_set_new(rootJ, ("currentSource"+to_string(track)).c_str(), json_integer(currentSource[track]));
			json_object_set_new(rootJ, ("trackLoopMeas"+to_string(track)).c_str(), json_integer(trackLoopMeas[track]));
			json_object_set_new(rootJ, ("startImm"+to_string(track)).c_str(), json_integer(int(startImm_setting[track])));
			json_object_set_new(rootJ, ("stopImm"+to_string(track)).c_str(), json_integer(int(stopImm_setting[track])));
			json_object_set_new(rootJ, ("loopSync"+to_string(track)).c_str(), json_integer(int(loopSync_setting[track])));
			json_object_set_new(rootJ, ("oneShot"+to_string(track)).c_str(), json_integer(int(oneShot_setting[track])));
			json_object_set_new(rootJ, ("rev"+to_string(track)).c_str(), json_integer(int(rev_setting[track])));
			json_object_set_new(rootJ, ("solo"+to_string(track)).c_str(), json_integer(int(solo_setting[track])));
			json_object_set_new(rootJ, ("xFade"+to_string(track)).c_str(), json_real(params[XFADE_KNOB_PARAM+track].getValue()));
			json_object_set_new(rootJ, ("pan"+to_string(track)).c_str(), json_real(params[PAN_KNOB_PARAM+track].getValue()));
			json_object_set_new(rootJ, ("volTrack"+to_string(track)).c_str(), json_real(volTrack[track]));
			json_object_set_new(rootJ, ("srcToTrack"+to_string(track)).c_str(), json_integer(int(srcToTrack[track])));
			json_object_set_new(rootJ, ("extraSamples"+to_string(track)).c_str(), json_boolean(extraSamples[track]));
			json_object_set_new(rootJ, ("playFullTail"+to_string(track)).c_str(), json_boolean(playFullTail[track]));
			json_object_set_new(rootJ, ("fadeInOnPlay"+to_string(track)).c_str(), json_boolean(fadeInOnPlay[track]));
		}
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
		json_t *srcToMasterJ = json_object_get(rootJ, "srcToMaster");
		if (srcToMasterJ) {
			srcToMaster = json_boolean_value(srcToMasterJ);
		}
		json_t *onlyClickOnEarJ = json_object_get(rootJ, "onlyClickOnEar");
		if (onlyClickOnEarJ) {
			onlyClickOnEar = json_boolean_value(onlyClickOnEarJ);
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
		json_t* overdubAfterRecJ = json_object_get(rootJ, "overdubAfterRec");
		if (overdubAfterRecJ)
			overdubAfterRec = json_boolean_value(overdubAfterRecJ);
		json_t *internalClockAlwaysOnJ = json_object_get(rootJ, "internalClockAlwaysOn");
		if (internalClockAlwaysOnJ) {
			internalClockAlwaysOn = json_boolean_value(internalClockAlwaysOnJ);
			setInternalClock(internalClockAlwaysOn);
		}
		/*
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
		*/
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

		json_t *clickJ = json_object_get(rootJ, "click");
		if (clickJ) {
			params[CLICK_BUT_PARAM].setValue(json_integer_value(clickJ));
		}
		json_t *clickVolJ = json_object_get(rootJ, "clickVol");
		if (clickVolJ) {
			params[CLICKVOL_KNOB_PARAM].setValue(json_real_value(clickVolJ));
		}
		json_t *clickToMasterJ = json_object_get(rootJ, "clickToMaster");
		if (clickToMasterJ) {
			params[CLICKTOMASTER_SWITCH].setValue(json_integer_value(clickToMasterJ));
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
		json_t *earLevelJ = json_object_get(rootJ, "earLevel");
		if (earLevelJ) {
			params[EARVOL_KNOB_PARAM].setValue(json_real_value(earLevelJ));
		}
		json_t *masterLevelJ = json_object_get(rootJ, "masterLevel");
		if (masterLevelJ) {
			params[MASTERVOL_KNOB_PARAM].setValue(json_real_value(masterLevelJ));
		}

		// ************    T R A C K    0    ***************
		json_t *sourceLvl0J = json_object_get(rootJ, "sourceLvl0");
		if (sourceLvl0J)
			params[SOURCELVL_KNOB_PARAM+0].setValue(json_real_value(sourceLvl0J));
		json_t *sourceMute0J = json_object_get(rootJ, "sourceMute0");
		if (sourceMute0J)
			params[MUTE_SWITCH+0].setValue(json_integer_value(sourceMute0J));
		json_t *currentSource0J = json_object_get(rootJ, "currentSource0");
		if (currentSource0J)
			params[SOURCE_KNOB_PARAM+0].setValue(json_integer_value(currentSource0J)+1);
		json_t *trackLoopMeas0J = json_object_get(rootJ, "trackLoopMeas0");
		if (trackLoopMeas0J)
			params[MEAS_KNOB_PARAM+0].setValue(json_integer_value(trackLoopMeas0J));
		json_t *startImm0J = json_object_get(rootJ, "startImm0");
		if (startImm0J)
			params[STARTIMM_SWITCH+0].setValue(json_integer_value(startImm0J));
		json_t *stopImm0J = json_object_get(rootJ, "stopImm0");
		if (stopImm0J)
			params[STOPIMM_SWITCH+0].setValue(json_integer_value(stopImm0J));
		json_t *oneShot0J = json_object_get(rootJ, "oneShot0");
		if (oneShot0J)
			params[ONESHOT_SWITCH+0].setValue(json_integer_value(oneShot0J));
		json_t *loopSync0J = json_object_get(rootJ, "loopSync0");
		if (loopSync0J)
			params[LOOPSYNC_SWITCH+0].setValue(json_integer_value(loopSync0J));
		json_t *rev0J = json_object_get(rootJ, "rev0");
		if (rev0J)
			params[REV_SWITCH+0].setValue(json_integer_value(rev0J));
		json_t *solo0J = json_object_get(rootJ, "solo0");
		if (solo0J)
			params[SOLO_SWITCH+0].setValue(json_integer_value(solo0J));
		json_t *xFade_setting0J = json_object_get(rootJ, "xFade0");
		if (xFade_setting0J)
			params[XFADE_KNOB_PARAM+0].setValue(json_real_value(xFade_setting0J));
		json_t *pan0J = json_object_get(rootJ, "pan0");
		if (pan0J)
			params[PAN_KNOB_PARAM+0].setValue(json_real_value(pan0J));
		json_t *volTrack0J = json_object_get(rootJ, "volTrack0");
		if (volTrack0J)
			params[VOLTRACK_KNOB_PARAM+0].setValue(json_real_value(volTrack0J));
		json_t *srcToTrack0J = json_object_get(rootJ, "srcToTrack0");
		if (srcToTrack0J)
			params[SRC_TO_TRACK_SWITCH+0].setValue(json_integer_value(srcToTrack0J));
		json_t *extraSamples0J = json_object_get(rootJ, "extraSamples0");
		if (extraSamples0J)
			extraSamples[0] = json_boolean_value(extraSamples0J);
		json_t *playFullTail0J = json_object_get(rootJ, "playFullTail0");
		if (playFullTail0J)
			playFullTail[0] = json_boolean_value(playFullTail0J);
		json_t *fadeInOnPlay0J = json_object_get(rootJ, "fadeInOnPlay0");
		if (fadeInOnPlay0J)
			fadeInOnPlay[0] = json_boolean_value(fadeInOnPlay0J);
	
		// ************    T R A C K    1    ***************
		json_t *sourceLvl1J = json_object_get(rootJ, "sourceLvl1");
		if (sourceLvl1J)
			params[SOURCELVL_KNOB_PARAM+1].setValue(json_real_value(sourceLvl1J));
		json_t *sourceMute1J = json_object_get(rootJ, "sourceMute1");
		if (sourceMute1J)
			params[MUTE_SWITCH+1].setValue(json_integer_value(sourceMute1J));
		json_t *currentSource1J = json_object_get(rootJ, "currentSource1");
		if (currentSource1J)
			params[SOURCE_KNOB_PARAM+1].setValue(json_integer_value(currentSource1J)+1);
		json_t *trackLoopMeas1J = json_object_get(rootJ, "trackLoopMeas1");
		if (trackLoopMeas1J)
			params[MEAS_KNOB_PARAM+1].setValue(json_integer_value(trackLoopMeas1J));
		json_t *startImm1J = json_object_get(rootJ, "startImm1");
		if (startImm1J)
			params[STARTIMM_SWITCH+1].setValue(json_integer_value(startImm1J));
		json_t *stopImm1J = json_object_get(rootJ, "stopImm1");
		if (stopImm1J)
			params[STOPIMM_SWITCH+1].setValue(json_integer_value(stopImm1J));
		json_t *oneShot1J = json_object_get(rootJ, "oneShot1");
		if (oneShot1J)
			params[ONESHOT_SWITCH+1].setValue(json_integer_value(oneShot1J));
		json_t *loopSync1J = json_object_get(rootJ, "loopSync1");
		if (loopSync1J)
			params[LOOPSYNC_SWITCH+1].setValue(json_integer_value(loopSync1J));
		json_t *rev1J = json_object_get(rootJ, "rev1");
		if (rev1J)
			params[REV_SWITCH+1].setValue(json_integer_value(rev1J));
		json_t *solo1J = json_object_get(rootJ, "solo1");
		if (solo1J)
			params[SOLO_SWITCH+1].setValue(json_integer_value(solo1J));
		json_t *xFade_setting1J = json_object_get(rootJ, "xFade1");
		if (xFade_setting1J)
			params[XFADE_KNOB_PARAM+1].setValue(json_real_value(xFade_setting1J));
		json_t *pan1J = json_object_get(rootJ, "pan1");
		if (pan1J)
			params[PAN_KNOB_PARAM+1].setValue(json_real_value(pan1J));
		json_t *volTrack1J = json_object_get(rootJ, "volTrack1");
		if (volTrack1J)
			params[VOLTRACK_KNOB_PARAM+1].setValue(json_real_value(volTrack1J));
		json_t *srcToTrack1J = json_object_get(rootJ, "srcToTrack1");
		if (srcToTrack1J)
			params[SRC_TO_TRACK_SWITCH+1].setValue(json_integer_value(srcToTrack1J));
		json_t *extraSamples1J = json_object_get(rootJ, "extraSamples1");
		if (extraSamples1J)
			extraSamples[1] = json_boolean_value(extraSamples1J);
		json_t *playFullTail1J = json_object_get(rootJ, "playFullTail1");
		if (playFullTail1J)
			playFullTail[1] = json_boolean_value(playFullTail1J);
		json_t *fadeInOnPlay1J = json_object_get(rootJ, "fadeInOnPlay1");
		if (fadeInOnPlay1J)
			fadeInOnPlay[1] = json_boolean_value(fadeInOnPlay1J);

		// ************    T R A C K    2    ***************
		json_t *sourceLvl2J = json_object_get(rootJ, "sourceLvl2");
		if (sourceLvl2J)
			params[SOURCELVL_KNOB_PARAM+2].setValue(json_real_value(sourceLvl2J));
		json_t *sourceMute2J = json_object_get(rootJ, "sourceMute2");
		if (sourceMute2J)
			params[MUTE_SWITCH+2].setValue(json_integer_value(sourceMute2J));
		json_t *currentSource2J = json_object_get(rootJ, "currentSource2");
		if (currentSource2J)
			params[SOURCE_KNOB_PARAM+2].setValue(json_integer_value(currentSource2J)+1);
		json_t *trackLoopMeas2J = json_object_get(rootJ, "trackLoopMeas2");
		if (trackLoopMeas2J)
			params[MEAS_KNOB_PARAM+2].setValue(json_integer_value(trackLoopMeas2J));
		json_t *startImm2J = json_object_get(rootJ, "startImm2");
		if (startImm2J)
			params[STARTIMM_SWITCH+2].setValue(json_integer_value(startImm2J));
		json_t *stopImm2J = json_object_get(rootJ, "stopImm2");
		if (stopImm2J)
			params[STOPIMM_SWITCH+2].setValue(json_integer_value(stopImm2J));
		json_t *oneShot2J = json_object_get(rootJ, "oneShot2");
		if (oneShot2J)
			params[ONESHOT_SWITCH+2].setValue(json_integer_value(oneShot2J));
		json_t *loopSync2J = json_object_get(rootJ, "loopSync2");
		if (loopSync2J)
			params[LOOPSYNC_SWITCH+2].setValue(json_integer_value(loopSync2J));
		json_t *rev2J = json_object_get(rootJ, "rev2");
		if (rev2J)
			params[REV_SWITCH+2].setValue(json_integer_value(rev2J));
		json_t *solo2J = json_object_get(rootJ, "solo2");
		if (solo2J)
			params[SOLO_SWITCH+2].setValue(json_integer_value(solo2J));
		json_t *xFade_setting2J = json_object_get(rootJ, "xFade2");
		if (xFade_setting2J)
			params[XFADE_KNOB_PARAM+2].setValue(json_real_value(xFade_setting2J));
		json_t *pan2J = json_object_get(rootJ, "pan2");
		if (pan2J)
			params[PAN_KNOB_PARAM+2].setValue(json_real_value(pan2J));
		json_t *volTrack2J = json_object_get(rootJ, "volTrack2");
		if (volTrack2J)
			params[VOLTRACK_KNOB_PARAM+2].setValue(json_real_value(volTrack2J));
		json_t *srcToTrack2J = json_object_get(rootJ, "srcToTrack2");
		if (srcToTrack2J)
			params[SRC_TO_TRACK_SWITCH+2].setValue(json_integer_value(srcToTrack2J));
		json_t *extraSamples2J = json_object_get(rootJ, "extraSamples2");
		if (extraSamples2J)
			extraSamples[2] = json_boolean_value(extraSamples2J);
		json_t *playFullTail2J = json_object_get(rootJ, "playFullTail2");
		if (playFullTail2J)
			playFullTail[2] = json_boolean_value(playFullTail2J);
		json_t *fadeInOnPlay2J = json_object_get(rootJ, "fadeInOnPlay2");
		if (fadeInOnPlay2J)
			fadeInOnPlay[2] = json_boolean_value(fadeInOnPlay2J);

		// ************    T R A C K    3    ***************
		json_t *sourceLvl3J = json_object_get(rootJ, "sourceLvl3");
		if (sourceLvl3J)
			params[SOURCELVL_KNOB_PARAM+3].setValue(json_real_value(sourceLvl3J));
		json_t *sourceMute3J = json_object_get(rootJ, "sourceMute3");
		if (sourceMute3J)
			params[MUTE_SWITCH+3].setValue(json_integer_value(sourceMute3J));
		json_t *currentSource3J = json_object_get(rootJ, "currentSource3");
		if (currentSource3J)
			params[SOURCE_KNOB_PARAM+3].setValue(json_integer_value(currentSource3J)+1);
		json_t *trackLoopMeas3J = json_object_get(rootJ, "trackLoopMeas3");
		if (trackLoopMeas3J)
			params[MEAS_KNOB_PARAM+3].setValue(json_integer_value(trackLoopMeas3J));
		json_t *startImm3J = json_object_get(rootJ, "startImm3");
		if (startImm3J)
			params[STARTIMM_SWITCH+3].setValue(json_integer_value(startImm3J));
		json_t *stopImm3J = json_object_get(rootJ, "stopImm3");
		if (stopImm3J)
			params[STOPIMM_SWITCH+3].setValue(json_integer_value(stopImm3J));
		json_t *oneShot3J = json_object_get(rootJ, "oneShot3");
		if (oneShot3J)
			params[ONESHOT_SWITCH+3].setValue(json_integer_value(oneShot3J));
		json_t *loopSync3J = json_object_get(rootJ, "loopSync3");
		if (loopSync3J)
			params[LOOPSYNC_SWITCH+3].setValue(json_integer_value(loopSync3J));
		json_t *rev3J = json_object_get(rootJ, "rev3");
		if (rev3J)
			params[REV_SWITCH+3].setValue(json_integer_value(rev3J));
		json_t *solo3J = json_object_get(rootJ, "solo3");
		if (solo3J)
			params[SOLO_SWITCH+3].setValue(json_integer_value(solo3J));
		json_t *xFade_setting3J = json_object_get(rootJ, "xFade3");
		if (xFade_setting3J)
			params[XFADE_KNOB_PARAM+3].setValue(json_real_value(xFade_setting3J));
		json_t *pan3J = json_object_get(rootJ, "pan3");
		if (pan3J)
			params[PAN_KNOB_PARAM+3].setValue(json_real_value(pan3J));
		json_t *volTrack3J = json_object_get(rootJ, "volTrack3");
		if (volTrack3J)
			params[VOLTRACK_KNOB_PARAM+3].setValue(json_real_value(volTrack3J));
		json_t *srcToTrack3J = json_object_get(rootJ, "srcToTrack3");
		if (srcToTrack3J)
			params[SRC_TO_TRACK_SWITCH+3].setValue(json_integer_value(srcToTrack3J));
		json_t *extraSamples3J = json_object_get(rootJ, "extraSamples3");
		if (extraSamples3J)
			extraSamples[3] = json_boolean_value(extraSamples3J);
		json_t *playFullTail3J = json_object_get(rootJ, "playFullTail3");
		if (playFullTail3J)
			playFullTail[3] = json_boolean_value(playFullTail3J);
		json_t *fadeInOnPlay3J = json_object_get(rootJ, "fadeInOnPlay3");
		if (fadeInOnPlay3J)
			fadeInOnPlay[3] = json_boolean_value(fadeInOnPlay3J);

		// ************    T R A C K    4    ***************
		json_t *sourceLvl4J = json_object_get(rootJ, "sourceLvl4");
		if (sourceLvl4J)
			params[SOURCELVL_KNOB_PARAM+4].setValue(json_real_value(sourceLvl4J));
		json_t *sourceMute4J = json_object_get(rootJ, "sourceMute4");
		if (sourceMute4J)
			params[MUTE_SWITCH+4].setValue(json_integer_value(sourceMute4J));
		json_t *currentSource4J = json_object_get(rootJ, "currentSource4");
		if (currentSource4J)
			params[SOURCE_KNOB_PARAM+4].setValue(json_integer_value(currentSource4J)+1);
		json_t *trackLoopMeas4J = json_object_get(rootJ, "trackLoopMeas4");
		if (trackLoopMeas4J)
			params[MEAS_KNOB_PARAM+4].setValue(json_integer_value(trackLoopMeas4J));
		json_t *startImm4J = json_object_get(rootJ, "startImm4");
		if (startImm4J)
			params[STARTIMM_SWITCH+4].setValue(json_integer_value(startImm4J));
		json_t *stopImm4J = json_object_get(rootJ, "stopImm4");
		if (stopImm4J)
			params[STOPIMM_SWITCH+4].setValue(json_integer_value(stopImm4J));
		json_t *oneShot4J = json_object_get(rootJ, "oneShot4");
		if (oneShot4J)
			params[ONESHOT_SWITCH+4].setValue(json_integer_value(oneShot4J));
		json_t *loopSync4J = json_object_get(rootJ, "loopSync4");
		if (loopSync4J)
			params[LOOPSYNC_SWITCH+4].setValue(json_integer_value(loopSync4J));
		json_t *rev4J = json_object_get(rootJ, "rev4");
		if (rev4J)
			params[REV_SWITCH+4].setValue(json_integer_value(rev4J));
		json_t *solo4J = json_object_get(rootJ, "solo4");
		if (solo4J)
			params[SOLO_SWITCH+4].setValue(json_integer_value(solo4J));
		json_t *xFade_setting4J = json_object_get(rootJ, "xFade4");
		if (xFade_setting4J)
			params[XFADE_KNOB_PARAM+4].setValue(json_real_value(xFade_setting4J));
		json_t *pan4J = json_object_get(rootJ, "pan4");
		if (pan4J)
			params[PAN_KNOB_PARAM+4].setValue(json_real_value(pan4J));
		json_t *volTrack4J = json_object_get(rootJ, "volTrack4");
		if (volTrack4J)
			params[VOLTRACK_KNOB_PARAM+4].setValue(json_real_value(volTrack4J));
		json_t *srcToTrack4J = json_object_get(rootJ, "srcToTrack4");
		if (srcToTrack4J)
			params[SRC_TO_TRACK_SWITCH+4].setValue(json_integer_value(srcToTrack4J));
		json_t *extraSamples4J = json_object_get(rootJ, "extraSamples4");
		if (extraSamples4J)
			extraSamples[4] = json_boolean_value(extraSamples4J);
		json_t *playFullTail4J = json_object_get(rootJ, "playFullTail4");
		if (playFullTail4J)
			playFullTail[4] = json_boolean_value(playFullTail4J);
		json_t *fadeInOnPlay4J = json_object_get(rootJ, "fadeInOnPlay4");
		if (fadeInOnPlay4J)
			fadeInOnPlay[4] = json_boolean_value(fadeInOnPlay4J);
	}

	void menuLoadPreset() {
		static const char FILE_FILTERS[] = "sickolooper preset (.slp):slp,SLP";
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

			for (int track = 0; track < MAX_TRACKS; track++) {

				std::string trackPath = path.substr(0, path.length() - 4) + "_track" + to_string(track+1) + ".wav";
				loadSample(track, trackPath);

				if (fileLoaded) {
					fileLoaded = false;
					if (!trackRecorded[track]) {
						trackRecorded[track] = true;
						recordedTracks++;
					}
				} else {
					trackRecorded[track] = false;
					trackStatus[track] = EMPTY;
					setEmptyLed(track);
				}

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
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".slp" and strPath.substr(strPath.size() - 4) != ".SLP")
				strPath += ".slp";
			path = strcpy(new char[strPath.length() + 1], strPath.c_str());
			savePreset(path, presetToJson(), loops);
		}

		free(path);
#if defined(METAMODULE)
		});
#endif
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
					for (int track = 0; track < MAX_TRACKS; track++) {
						std::string trackPath = path.substr(0, path.length() - 4) + "_track" + to_string(track+1);
						system::remove((trackPath+".wav").c_str());
						system::remove((trackPath+".WAV").c_str());
						if (trackRecorded[track])
							saveSample(track, trackPath);
					}
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
			clickLoadSample(path, slot);
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

		unsigned int c;
		unsigned int sr;
		drwav_uint64 tsc;
		float* pSampleData;
		pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &tsc);

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

				drwav_free(pSampleData);

				clickSampleRate[slot] = APP->engine->getSampleRate();

			} else {											// ***************** RESAMPLE ****************************************
				for (unsigned int i=0; i < tsc; i = i + c) {
					clickTempBuffer.push_back(pSampleData[i] * 5);
					clickTempBuffer.push_back(0);
				}

				drwav_free(pSampleData);

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
			/*
			clickFileDescription[slot] = basename(pathDup);

			free(pathDup);
			clickStoredPath[slot] = path;

			clickFileLoaded[slot] = true;
			*/
			if (customClick) {
				clickFileDescription[slot] = basename(pathDup);

				clickStoredPath[slot] = path;
	
			}
			clickFileLoaded[slot] = true;
			free(pathDup);

		} else {
			clickFileLoaded[slot] = false;
			clickStoredPath[slot] = path;
			clickFileDescription[slot] = "(!)"+path;
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


	void inline setEmptyLed(int track) {
		playButtonPulse[track] = NO_PULSE;
		lights[PLAY_BUT_LIGHT+track].setBrightness(0.f);
		recButtonPulse[track] = NO_PULSE;
		lights[REC_BUT_LIGHT+track].setBrightness(0.f);
	}

	void inline setIdleLed(int track) {
		playButtonPulse[track] = NO_PULSE;
		lights[PLAY_BUT_LIGHT+track].setBrightness(1.f);
		recButtonPulse[track] = NO_PULSE;
		lights[REC_BUT_LIGHT+track].setBrightness(0.f);
	}

	void inline setPlayLed(int track) {
		playButtonPulse[track] = SLOW_PULSE;
		playButtonPulseTime[track] = slowPulseTime;
		lights[PLAY_BUT_LIGHT+track].setBrightness(1.f);
		recButtonPulse[track] = NO_PULSE;
		lights[REC_BUT_LIGHT+track].setBrightness(0.f);
	}

	void inline setFastPlayLed(int track) {
		playButtonPulse[track] = FAST_PULSE;
		playButtonPulseTime[track] = fastPulseTime;
		lights[PLAY_BUT_LIGHT+track].setBrightness(1.f);
		recButtonPulse[track] = NO_PULSE;
		lights[REC_BUT_LIGHT+track].setBrightness(0.f);
	}

	void inline setRecLed(int track) {
		playButtonPulse[track] = NO_PULSE;
		lights[PLAY_BUT_LIGHT+track].setBrightness(0.f);
		recButtonPulse[track] = SLOW_PULSE;
		recButtonPulseTime[track] = slowPulseTime;
		lights[REC_BUT_LIGHT+track].setBrightness(1.f);	
	}

	void inline setFastRecLed(int track) {
		playButtonPulse[track] = NO_PULSE;
		lights[PLAY_BUT_LIGHT+track].setBrightness(0.f);
		recButtonPulse[track] = FAST_PULSE;
		recButtonPulseTime[track] = fastPulseTime;
		lights[REC_BUT_LIGHT+track].setBrightness(1.f);	
	}

	void inline setOverdubLed(int track) {
		playButtonPulse[track] = SLOW_PULSE;
		playButtonPulseTime[track] = slowPulseTime;
		lights[PLAY_BUT_LIGHT+track].setBrightness(1.f);
		recButtonPulse[track] = SLOW_PULSE;
		recButtonPulseTime[track] = slowPulseTime;
		lights[REC_BUT_LIGHT+track].setBrightness(1.f);	
	}

	void inline setFastOverdubLed(int track) {
		playButtonPulse[track] = FAST_PULSE;
		playButtonPulseTime[track] = fastPulseTime;
		lights[PLAY_BUT_LIGHT+track].setBrightness(1.f);
		recButtonPulse[track] = FAST_PULSE;
		recButtonPulseTime[track] = fastPulseTime;
		lights[REC_BUT_LIGHT+track].setBrightness(1.f);	
	}

	void inline setPlayToRecLed(int track) {
		playButtonPulse[track] = SLOW_PULSE;
		playButtonPulseTime[track] = slowPulseTime;
		lights[PLAY_BUT_LIGHT+track].setBrightness(1.f);
		recButtonPulse[track] = FAST_PULSE;
		recButtonPulseTime[track] = fastPulseTime;
		lights[REC_BUT_LIGHT+track].setBrightness(1.f);	
	}

	void inline setRecToPlayLed(int track) {
		playButtonPulse[track] = FAST_PULSE;
		playButtonPulseTime[track] = fastPulseTime;
		lights[PLAY_BUT_LIGHT+track].setBrightness(1.f);
		recButtonPulse[track] = SLOW_PULSE;
		recButtonPulseTime[track] = fastPulseTime;
		lights[REC_BUT_LIGHT+track].setBrightness(1.f);	
	}

	void inline xFadePlay(int track) {
		extraPlaying[track] = true;
		extraPlayPos[track] = samplePos[track];
		extraPlayDirection[track] = playingDirection[track];
		xFadeValue[track] = 1.f;
		if (!stopNow[track])
			xFadeDelta[track] = 1000 / (params[XFADE_KNOB_PARAM+track].getValue() * APP->engine->getSampleRate());
		else
			xFadeDelta[track] = 1 / minTimeSamples;
	}

	bool isExtraSamples(int track) {
		return extraSamples[track];
	}

	void setExtraSamples(int track, bool poly) {
		if (poly) {
			if (totalSampleC[track] > tailSamples) {
				extraSamples[track] = true;
				totalSampleC[track] = trackBuffer[track][LEFT].size() - tailSamples;
				totalSamples[track] = totalSampleC[track]-1;
			} else {
				extraSamples[track] = false;
				totalSampleC[track] = trackBuffer[track][LEFT].size();
				totalSamples[track] = totalSampleC[track]-1;
			}
		} else {
			extraSamples[track] = false;
			totalSampleC[track] = trackBuffer[track][LEFT].size();
			totalSamples[track] = totalSampleC[track]-1;
		}
	}

	void detectTempo(int track) {
		if (beatMaxPerBar[int(params[SIGNATURE_KNOB_PARAM].getValue())] < 6)
			params[BPM_KNOB_PARAM].setValue(int(600 * (double)sampleRate * beatMaxPerBar[int(params[SIGNATURE_KNOB_PARAM].getValue())] / totalSampleC[track]) * params[MEAS_KNOB_PARAM+track].getValue());
		else
			params[BPM_KNOB_PARAM].setValue(int(300 * (double)sampleRate * beatMaxPerBar[int(params[SIGNATURE_KNOB_PARAM].getValue())] / totalSampleC[track]) * params[MEAS_KNOB_PARAM+track].getValue());
	}

	void inline cancelNextStatus(int track) {
		if (trackStatus[track] == PLAYING)
			setPlayLed(track);
		else if (trackStatus[track] == RECORDING)
			setRecLed(track);
		else if (trackStatus[track] == OVERDUBBING)
			setOverdubLed(track);
		nextStatus[track] = NOTHING;
	}

	bool isInternalClockAlwaysOn() {
		return internalClockAlwaysOn;
	}

	void setInternalClock(bool internalClock) {
		if (internalClock) {
			internalClockAlwaysOn = true;
			runSetting = 1;
		} else {
			internalClockAlwaysOn = false;
			if (busyTracks == 0) {
				globalStatus = IDLE;
				
				//if (!extConn) {
				if (!extConn && !internalClockAlwaysOn) {
					runSetting = 0;
					clockSample = 1.0;
					resetStart = true;
					beatCounter = 20;	// this hack makes the click be audible on next runSetting=1
				}
			}
		}
	}

	void inline resetIdleEmptyStatus(int track) {
		if (trackStatus[track] == EMPTY)
			setEmptyLed(track);
		else
			setIdleLed(track);
		nextStatus[track] = NOTHING;
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

		sumOutput[LEFT] = 0.f;
		sumOutput[RIGHT] = 0.f;
		earOutput[LEFT] = 0.f;
		earOutput[RIGHT] = 0.f;

		masterLevel = params[MASTERVOL_KNOB_PARAM].getValue();
		earLevel = params[EARVOL_KNOB_PARAM].getValue();

		lights[CLOCK_RST_LIGHT].setBrightness(params[CLOCK_RST_SWITCH].getValue());

		lights[CLICK_BUT_LIGHT].setBrightness(params[CLICK_BUT_PARAM].getValue());

		lights[CLICKTOMASTER_LIGHT].setBrightness(params[CLICKTOMASTER_SWITCH].getValue());

		if (params[PREROLL_BUT_PARAM].getValue())
			preRoll = true;
		else
			preRoll = false;

		lights[PREROLL_BUT_LIGHT].setBrightness(preRoll);

		lights[ALLSTARTSTOP_BUT_LIGHT].setBrightness(params[ALLSTARTSTOP_BUT_PARAM].getValue());

		// ***************************************************************************************************************************
		// ************************************************************************     L O A D     I N P U T S      *****************
		// ***************************************************************************************************************************
		// ******************************************************************* INPUTs AUDIO COLLECTING
		for (int track = 0; track < MAX_TRACKS; track++) {
			inputValue[track][LEFT] = 0.f;
			inputValue[track][RIGHT] = 0.f;
			mute[track] = params[MUTE_SWITCH+track].getValue();
			lights[MUTE_LIGHT+track].setBrightness(mute[track]);
			if (mute[track] != prevMute[track]) {
				muteFade[track] = true;
				if (mute[track]){
					muteValue[track] = 1;
					muteDelta[track] = -1.f / minTimeSamples;
				} else {
					muteValue[track] = 0;
					muteDelta[track] = 1.f / minTimeSamples;
				}
				prevMute[track] = mute[track];
			}

			if (muteFade[track]) {
				muteValue[track] += muteDelta[track];
				if (muteValue[track] < 0) {
					muteValue[track] = 0;
					muteFade[track] = false;
				} else if (muteValue[track] > 1) {
					muteValue[track] = 1;
					muteFade[track] = false;
				}
			}

			if (inputs[LEFT_INPUT+track].isConnected()) {
				//inputValue[track][LEFT] = inputs[LEFT_INPUT+track].getVoltage() * params[SOURCELVL_KNOB_PARAM+track].getValue() * muteValue[track];
				inputValue[track][LEFT] = inputs[LEFT_INPUT+track].getVoltageSum() * params[SOURCELVL_KNOB_PARAM+track].getValue() * muteValue[track];
				if (inputValue[track][LEFT] > 10.f)
					inputValue[track][LEFT] = 10.f;
				else if (inputValue[track][LEFT] < -10.f)
					inputValue[track][LEFT] = -10.f;
				if (inputs[RIGHT_INPUT+track].isConnected()) {
					//inputValue[track][RIGHT] = inputs[RIGHT_INPUT+track].getVoltage() * params[SOURCELVL_KNOB_PARAM+track].getValue() * muteValue[track];
					inputValue[track][RIGHT] = inputs[RIGHT_INPUT+track].getVoltageSum() * params[SOURCELVL_KNOB_PARAM+track].getValue() * muteValue[track];
					if (inputValue[track][RIGHT] > 10.f)
						inputValue[track][RIGHT] = 10.f;
					else if (inputValue[track][RIGHT] < -10.f)
						inputValue[track][RIGHT] = -10.f;
				} else
					inputValue[track][RIGHT] = inputValue[track][LEFT];
			} else {
				if (inputs[RIGHT_INPUT+track].isConnected()) {
					//inputValue[track][RIGHT] = inputs[RIGHT_INPUT+track].getVoltage() * params[SOURCELVL_KNOB_PARAM+track].getValue() * muteValue[track];
					inputValue[track][RIGHT] = inputs[RIGHT_INPUT+track].getVoltageSum() * params[SOURCELVL_KNOB_PARAM+track].getValue() * muteValue[track];
					if (inputValue[track][RIGHT] > 10.f)
						inputValue[track][RIGHT] = 10.f;
					else if (inputValue[track][RIGHT] < -10.f)
						inputValue[track][RIGHT] = -10.f;
					inputValue[track][LEFT] = inputValue[track][RIGHT];
				}
			}

			// ********************************************************************************************* other initializations

			startNow[track] = false;	// these are initialized here to be ready for next ALL PLAY STOP check
			stopNow[track] = false;

			if (params[SOLO_SWITCH+track].getValue() != solo_setting[track] && trackStatus[track] < 2 && nextStatus[track] < 2) {	// this avoids changing solo setting during play/rec/ovdub
				solo_setting[track] = params[SOLO_SWITCH+track].getValue();
				lights[SOLO_LIGHT+track].setBrightness(solo_setting[track]);
			} else {
				params[SOLO_SWITCH+track].setValue(solo_setting[track]);
			}
			
		}

//																						██████╗░██╗░░░░░░█████╗░██╗░░░██╗  ░█████╗░██╗░░░░░██╗░░░░░
//																						██╔══██╗██║░░░░░██╔══██╗╚██╗░██╔╝  ██╔══██╗██║░░░░░██║░░░░░
//																						██████╔╝██║░░░░░███████║░╚████╔╝░  ███████║██║░░░░░██║░░░░░
//																						██╔═══╝░██║░░░░░██╔══██║░░╚██╔╝░░  ██╔══██║██║░░░░░██║░░░░░
//																						██║░░░░░███████╗██║░░██║░░░██║░░░  ██║░░██║███████╗███████╗
//																						╚═╝░░░░░╚══════╝╚═╝░░╚═╝░░░╚═╝░░░  ╚═╝░░╚═╝╚══════╝╚══════╝
		
		/*
		if (inputs[ALLSTARTSTOP_TRIG_INPUT].getVoltage() > 1.f)
			allSSTrig = 1;
		else
			allSSTrig = 0;
		*/
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
				for (int track = 0; track < MAX_TRACKS; track++) {

					if (trackStatus[track] != EMPTY && (!solo_setting[track] || (solo_setting[track] && nextSoloTrack < 0))) {

						if (solo_setting[track])
							nextSoloTrack = track;

						nextStatus[track] = PLAYING;
						busyTracks++;
						setFastPlayLed(track);
						loopCount[track] = trackLoopMeas[track];

					}
				}
				
				if (preRoll) {
					globalStatus = PREROLLING;
					preRollCount = 0;
				} else
					globalStatus = PLAYING;	

				runSetting = 1; // this starts clock

			} else if (globalStatus == PLAYING) {
				for (int track = 0; track < MAX_TRACKS; track++) {
					switch (trackStatus[track]) {
						case EMPTY:
						case IDLE:
						break;

						case PLAYING:
							setFastPlayLed(track);
							if (stopImm_setting[track])
								stopNow[track] = true;
						break;

						case RECORDING:
							setFastRecLed(track);
							if (!loopSync_setting[track])
								stopNow[track] = true;
						break;

						case OVERDUBBING:
							setFastOverdubLed(track);
							if (stopImm_setting[track])
								stopNow[track] = true;
						break;
					}
					nextStatus[track] = IDLE;
				}
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
			for (int track = 0; track < MAX_TRACKS; track++) {
				switch (trackStatus[track]) {
					case EMPTY:
					case IDLE:
					break;

					case PLAYING:
						setFastPlayLed(track);
						if (stopImm_setting[track])
							stopNow[track] = true;
					break;

					case RECORDING:
						setFastRecLed(track);
						if (!loopSync_setting[track])
							stopNow[track] = true;
					break;

					case OVERDUBBING:
						setFastOverdubLed(track);
						if (stopImm_setting[track])
							stopNow[track] = true;
					break;
				}
				nextStatus[track] = IDLE;
			}
		}

		// ***************************************************************************************************************************
		// **********************************************************************   T R A C K    M A N A G E M E N T   ***************
		// ***************************************************************************************************************************

		for (int track = 0; track < MAX_TRACKS; track++) {

			currentSource[track] = int(params[SOURCE_KNOB_PARAM+track].getValue())-1;

			trackLoopMeas[track] = int(params[MEAS_KNOB_PARAM+track].getValue());

			volTrack[track] = params[VOLTRACK_KNOB_PARAM+track].getValue();

			play_but[track] = params[PLAY_BUT_PARAM+track].getValue();

			rec_but[track] = params[REC_BUT_PARAM+track].getValue();

			stop_but[track] = params[STOP_BUT_PARAM+track].getValue();
			lights[STOP_BUT_LIGHT+track].setBrightness(stop_but[track]);

			erase_but[track] = params[ERASE_BUT_PARAM+track].getValue();
			lights[ERASE_BUT_LIGHT+track].setBrightness(erase_but[track]);	

			loopSync_setting[track] = params[LOOPSYNC_SWITCH+track].getValue();
			lights[LOOPSYNC_LIGHT+track].setBrightness(loopSync_setting[track]);

			startImm_setting[track] = params[STARTIMM_SWITCH+track].getValue();
			lights[STARTIMM_LIGHT+track].setBrightness(startImm_setting[track]);

			stopImm_setting[track] = params[STOPIMM_SWITCH+track].getValue();
			lights[STOPIMM_LIGHT+track].setBrightness(stopImm_setting[track]);

			oneShot_setting[track] = params[ONESHOT_SWITCH+track].getValue();
			lights[ONESHOT_LIGHT+track].setBrightness(oneShot_setting[track]);

			rev_setting[track] = params[REV_SWITCH+track].getValue();
			lights[REV_LIGHT+track].setBrightness(rev_setting[track]);

			srcToTrack[track] = params[SRC_TO_TRACK_SWITCH+track].getValue();
			lights[SRC_TO_TRACK_LIGHT+track].setBrightness(srcToTrack[track]);

			if (srcToTrack[track] != prevSrcToTrack[track]) {
				srcToTrackFade[track] = true;
				if (srcToTrack[track]){
					srcToTrackValue[track] = 0;
					srcToTrackDelta[track] = 1.f / minTimeSamples;
				} else {
					srcToTrackValue[track] = 1;
					srcToTrackDelta[track] = -1.f / minTimeSamples;
				}
				prevSrcToTrack[track] = srcToTrack[track];
			}

			if (srcToTrackFade[track]) {
				srcToTrackValue[track] += srcToTrackDelta[track];
				if (srcToTrackValue[track] < 0) {
					srcToTrackValue[track] = 0;
					srcToTrackFade[track] = false;
				} else if (srcToTrackValue[track] > 1) {
					srcToTrackValue[track] = 1;
					srcToTrackFade[track] = false;
				}
			}
			
			if (inputs[PLAY_TRIG_INPUT+track].getVoltage() > 1 || play_but[track])
				playTrig[track] = 1;
			else
				playTrig[track] = 0;

			if (inputs[REC_TRIG_INPUT+track].getVoltage() > 1 || rec_but[track])
				recTrig[track] = 1;
			else
				recTrig[track] = 0;

			if (inputs[STOP_TRIG_INPUT+track].getVoltage() > 1 || stop_but[track])
				stopTrig[track] = 1;
			else
				stopTrig[track] = 0;

			if (inputs[ERASE_TRIG_INPUT+track].getVoltage() > 1) {
				eraseTrig[track] = 1;
				eraseWait[track] = true;
				eraseTime[track] = 0;
			} else if (erase_but[track]) {
				eraseTrig[track] = 1;
				if (eraseWait[track] == false) {
					eraseWait[track] = true;
					eraseTime[track] = eraseSamples;
				}
			} else
				eraseTrig[track] = 0;			

			// *****************************************************  CHECK TRIGGERS


//																		███████╗██████╗░░█████╗░░██████╗███████╗  ████████╗██████╗░██╗░██████╗░
//																		██╔════╝██╔══██╗██╔══██╗██╔════╝██╔════╝  ╚══██╔══╝██╔══██╗██║██╔════╝░
//																		█████╗░░██████╔╝███████║╚█████╗░█████╗░░  ░░░██║░░░██████╔╝██║██║░░██╗░
//																		██╔══╝░░██╔══██╗██╔══██║░╚═══██╗██╔══╝░░  ░░░██║░░░██╔══██╗██║██║░░╚██╗
//																		███████╗██║░░██║██║░░██║██████╔╝███████╗  ░░░██║░░░██║░░██║██║╚██████╔╝
//																		╚══════╝╚═╝░░╚═╝╚═╝░░╚═╝╚═════╝░╚══════╝  ░░░╚═╝░░░╚═╝░░╚═╝╚═╝░╚═════╝░			

			

			if (eraseTrig[track] >= 1 && prevEraseTrig[track] < 1) {	// ***** ERASE TRIG

				if (trackStatus[track] == IDLE && eraseWait[track] && eraseTime[track] != eraseSamples) {
					eraseWait[track] = false;
					trackBuffer[track][LEFT].resize(0);
					trackBuffer[track][RIGHT].resize(0);
					totalSamples[track] = 0;
					totalSampleC[track] = 0;
					trackStatus[track] = EMPTY;
					nextStatus[track] = NOTHING;
					trackRecorded[track] = false;
					recordedTracks--;
					lights[PLAY_BUT_LIGHT+track].setBrightness(0.f);
				}
			}
			
			if (eraseWait[track]) {
				eraseTime[track]--;
				if (eraseTime[track] < 0)
					eraseWait[track] = false;
			}



//																					░██████╗████████╗░█████╗░██████╗░  ████████╗██████╗░██╗░██████╗░
//																					██╔════╝╚══██╔══╝██╔══██╗██╔══██╗  ╚══██╔══╝██╔══██╗██║██╔════╝░
//																					╚█████╗░░░░██║░░░██║░░██║██████╔╝  ░░░██║░░░██████╔╝██║██║░░██╗░
//																					░╚═══██╗░░░██║░░░██║░░██║██╔═══╝░  ░░░██║░░░██╔══██╗██║██║░░╚██╗
//																					██████╔╝░░░██║░░░╚█████╔╝██║░░░░░  ░░░██║░░░██║░░██║██║╚██████╔╝
//																					╚═════╝░░░░╚═╝░░░░╚════╝░╚═╝░░░░░  ░░░╚═╝░░░╚═╝░░╚═╝╚═╝░╚═════╝░

			if (stopTrig[track] >= 1 && prevStopTrig[track] < 1) {	// ***** STOP TRIG

				switch (trackStatus[track]) {

					case IDLE:
					case EMPTY:
						if (nextStatus[track] != NOTHING) {
							if (solo_setting[track]) {
								if (currentSoloTrack != track)
									cancelNextStatus(currentSoloTrack);

								nextSoloTrack = -1;
							}
							nextStatus[track] = NOTHING;
							resetIdleEmptyStatus(track);
							busyTracks--;
						}
					break;

					case PLAYING:
						if (instantStop) {
							if (solo_setting[track])
								nextSoloTrack = -1;
							nextStatus[track] = IDLE;
							stopNow[track] = true;
						} else {
							if (nextStatus[track] != IDLE) {
								nextStatus[track] = IDLE;
								setFastPlayLed(track);

								if (solo_setting[track])
									nextSoloTrack = -1;
								if (stopImm_setting[track])
									stopNow[track] = true;
							} else {	// undo
								nextStatus[track] = NOTHING;
								setPlayLed(track);
							}
						}
					break;

					case RECORDING:
						if (nextStatus[track] != IDLE) {	// undo stop recording
							nextStatus[track] = IDLE;
							if (loopSync_setting[track])
								setFastRecLed(track);
							else
								stopNow[track] = true;

							if (solo_setting[track])
								nextSoloTrack = -1;
						} else {	// undo
							nextStatus[track] = NOTHING;
							setRecLed(track);
							}
					break;

					case OVERDUBBING:
						if (instantStop) {
							if (solo_setting[track])
								nextSoloTrack = -1;
							nextStatus[track] = IDLE;
							stopNow[track] = true;
						} else {
							if (nextStatus[track] != IDLE) {

								nextStatus[track] = IDLE;

								if (loopSync_setting[track])
									setFastOverdubLed(track);
								else
									stopNow[track] = true;

								if (solo_setting[track])
									nextSoloTrack = -1;
								if (stopImm_setting[track])
									stopNow[track] = true;

							} else { // undo
								nextStatus[track] = NOTHING;
								setOverdubLed(track);
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
			
			if (playTrig[track] >= 1 && prevPlayTrig[track] < 1) {	// ******* PLAY TRIG

				switch (trackStatus[track]) {
					case EMPTY:											// play press & track is EMPTY -> do nothing
						if (playSequence != PLAY_STOP)
							recTrig[track] = 1;
					break;

					case IDLE:
						
						if (playSequence == REC_OVERDUB_PLAY) {
							recTrig[track] = 1;

						} else {

							if (globalStatus == IDLE) {						// play press & globalStatus is IDLE

								if (preRoll) {
									globalStatus = PREROLLING;
									preRollCount = 0;
								} else {
									globalStatus = PLAYING;
								}

								loopCount[track] = trackLoopMeas[track];
								
								runSetting = 1;
								busyTracks++;
								nextStatus[track] = PLAYING;
								if (solo_setting[track])
									nextSoloTrack = track;

								if (!loopSync_setting[track]) {
									if (!rev_setting[track]) {
										playingDirection[track] = FORWARD;
										samplePos[track] = 0;
									} else {
										playingDirection[track] = REVERSE;
										samplePos[track] = totalSamples[track];
									}
								}
								
								setFastPlayLed(track);

							} else if (globalStatus == PREROLLING) {		// play press & globalStatus is PREROLLING
								
								if (nextStatus[track] == NOTHING) {	// undo

									if (!solo_setting[track]) {
										busyTracks++;
									} else {
										if (nextSoloTrack < 0) {
											busyTracks++;
										} else {
											nextStatus[nextSoloTrack] = NOTHING;
											setIdleLed(nextSoloTrack);
										}
										nextSoloTrack = track;
									}

									if (!loopSync_setting[track]) {
										if (!rev_setting[track]) {
											playingDirection[track] = FORWARD;
											samplePos[track] = 0;
										} else {
											playingDirection[track] = REVERSE;
											samplePos[track] = totalSamples[track];
										}
									}

									nextStatus[track] = PLAYING;
									loopCount[track] = trackLoopMeas[track];

									setFastPlayLed(track);

								} else {	// undo or switch from overdub to play

									if (nextStatus[track] == PLAYING) {
										nextStatus[track] = NOTHING;
										busyTracks--;
										if (nextSoloTrack == track)
											nextSoloTrack = -1;
										setIdleLed(track);
									} else {
										nextStatus[track] = PLAYING;
										setFastPlayLed(track);
									}
								}
								
							} else {

								if (nextStatus[track] == NOTHING) {		// play press & track is IDLE -> go PLAYING
									
									if (solo_setting[track]) {

										if (nextSoloTrack < 0) {	// se non ci sono solo track prenotate

											if (currentSoloTrack >= 0)	// se è in funzione un'altra solo track, ne prenota lo stop
												nextStatus[currentSoloTrack] = IDLE;	// così alla fine del loop inizierà questa
											else	// se non è in funzione un'altra solo track, allora ne prenota subito il play
												loopCount[track] = trackLoopMeas[track];
										
										} else if (nextSoloTrack != track) {	// se era già prenotata un'altra solo track, allora si annulla la precedente prenotata
											resetIdleEmptyStatus(nextSoloTrack);
											busyTracks--;
										}

										nextSoloTrack = track;
										
									} else {	// se non è una solo track
										loopCount[track] = trackLoopMeas[track];
									}

									busyTracks++;
									nextStatus[track] = PLAYING;

									if (startImm_setting[track]) {
										startNow[track] = true;
										loopCount[track] = 1;
									}

									if (!loopSync_setting[track]) {
										if (!rev_setting[track]) {
											playingDirection[track] = FORWARD;
											samplePos[track] = 0;
										} else {
											playingDirection[track] = REVERSE;
											samplePos[track] = totalSamples[track];
										}
									}

									setFastPlayLed(track);

									
								} else {	// undo play press

									if (nextStatus[track] == PLAYING) {
										busyTracks--;
										
										resetIdleEmptyStatus(track);

										if (solo_setting[track] && nextSoloTrack == track)
											nextSoloTrack = -1;

										if (currentSoloTrack >= 0)
											cancelNextStatus(currentSoloTrack);

									} else {	// se il nextStatus è OVERDUBBING passa a PLAYING
										nextStatus[track] = PLAYING;
										setFastPlayLed(track);
									}
								}
							}
						}
					break;

					case PLAYING:							// play press & track is PLAYING -> go IDLE
						if (playSequence == PLAY_STOP) {

							if (nextStatus[track] != IDLE) {
								nextStatus[track] = IDLE;

								if (solo_setting[track]) {
									if (nextSoloTrack == track)	// se quella che stai premendo e la nextSoloTrack è la stessa
										nextSoloTrack = -1;	// dovrebbe equivalere a non far niente
									else if (nextSoloTrack >= 0) {	// se la nextSoloTrack è un'altra riportala in IDLE
										resetIdleEmptyStatus(nextSoloTrack);

										nextSoloTrack = -1;
										busyTracks--;
									}
								}
								
								if (stopImm_setting[track])
									stopNow[track] = true;

								setFastPlayLed(track);

							} else {	// undo play press
								nextStatus[track] = NOTHING;

								if (solo_setting[track]) {
									
									if (nextSoloTrack == track)
										nextSoloTrack = -1;
									else if (nextSoloTrack >= 0) {
										resetIdleEmptyStatus(nextSoloTrack);
										nextSoloTrack = -1;
										busyTracks--;
									}
								}

								setPlayLed(track);
								
							}
						} else {
							recTrig[track] = 1;
						}
					break;

					case RECORDING:						// play press & track is RECORDING -> go PLAYING

						if (playSequence == PLAY_STOP) {
							if (nextStatus[track] != PLAYING) {
								
								nextStatus[track] = PLAYING;
								
								if (!loopSync_setting[track])				// if not loopSync
									stopNow[track] = true;

								setRecToPlayLed(track);
							} else {	// undo play press

								nextStatus[track] = NOTHING;

								setRecLed(track);
							}
						} else {
							recTrig[track] = 1;
						}

					break;

					case OVERDUBBING:				// play press & track is OVERDUBBING -> go PLAYING

						if (playSequence == PLAY_STOP) {

							if (nextStatus[track] != PLAYING) {
								nextStatus[track] = PLAYING;

								if (solo_setting[track]) {
									if (nextSoloTrack == track)
										nextSoloTrack = -1;
									else if (nextSoloTrack >= 0) {
										resetIdleEmptyStatus(nextSoloTrack);
										nextSoloTrack = -1;
										busyTracks--;
									}
								}

								if (startImm_setting[track])
									startNow[track] = true;

								setRecToPlayLed(track);
							} else {	// undo play press

								if (solo_setting[track]) {
									
									if (nextSoloTrack == track)
										nextSoloTrack = -1;
									else if (nextSoloTrack >= 0) {
										resetIdleEmptyStatus(nextSoloTrack);
										nextSoloTrack = -1;
										busyTracks--;
									}
								}
								nextStatus[track] = NOTHING;
								setOverdubLed(track);
							}
						} else {
							recTrig[track] = 1;
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

			if (recTrig[track] >= 1 && prevRecTrig[track] < 1) {	// ***** REC TRIG

				if (globalStatus == IDLE) {									// rec press & globalStatus is IDLE
					
					runSetting = 1;
					busyTracks++;

					if (preRoll) {
						globalStatus = PREROLLING;
						preRollCount = 0;
					} else {
						globalStatus = PLAYING;
					}

					if (trackStatus[track] == EMPTY) {
						nextStatus[track] = RECORDING;
						setFastRecLed(track);
					} else {
						nextStatus[track] = OVERDUBBING;
						setFastOverdubLed(track);
					}

					loopCount[track] = trackLoopMeas[track];

					if (solo_setting[track])
						nextSoloTrack = track;

					if (!loopSync_setting[track]) {
						if (!rev_setting[track]) {
							playingDirection[track] = FORWARD;
							samplePos[track] = 0;
						} else {
							playingDirection[track] = REVERSE;
							samplePos[track] = totalSamples[track];
						}
					}

				} else if (globalStatus == PREROLLING) {				// rec press & globalStatus is PREROLLING

					if (nextStatus[track] == NOTHING ) {

						if (!solo_setting[track]) {
							busyTracks++;

						} else {
							if (nextSoloTrack < 0) {
								busyTracks++;
							} else {
								resetIdleEmptyStatus(nextSoloTrack);
							}
							nextSoloTrack = track;
						} 

						if (trackStatus[track] == EMPTY) {
							nextStatus[track] = RECORDING;
							setFastRecLed(track);
						} else {
							nextStatus[track] = OVERDUBBING;
							setFastOverdubLed(track);
						}

						if (!loopSync_setting[track]) {
							if (!rev_setting[track]) {
								playingDirection[track] = FORWARD;
								samplePos[track] = 0;
							} else {
								playingDirection[track] = REVERSE;
								samplePos[track] = totalSamples[track];
							}
						}

						loopCount[track] = trackLoopMeas[track];

					
					} else {	// undo
						
						if (nextStatus[track] == PLAYING) {

							nextStatus[track] = OVERDUBBING;
							setFastOverdubLed(track);
						} else {

							if (nextStatus[track] == RECORDING)
								setEmptyLed(track);
							else
								setIdleLed(track);

							nextStatus[track] = NOTHING;
							busyTracks--;
							if (nextSoloTrack == track)
								nextSoloTrack = -1;
						}
					}
			
				} else {

					switch (trackStatus[track]) {
						case EMPTY:									// rec press & track is empty -> go recording

							if (nextStatus[track] == NOTHING) {

								if (solo_setting[track]) {

									if (nextSoloTrack < 0) {	// se non ci sono solo track prenotate

										if (currentSoloTrack >= 0)	// se è in funzione un'altra solo track, ne prenota lo stop
											nextStatus[currentSoloTrack] = IDLE;
										else	// se non è in funzione un'altra solo track, allora ne prenota subito l'overdub
											loopCount[track] = trackLoopMeas[track];

									} else if (nextSoloTrack != track) {	// se era già prenotata un'altra solo track, allora si annulla la precedente prenotata
										resetIdleEmptyStatus(nextSoloTrack);
										busyTracks--;
									}
									nextSoloTrack = track;

								} else {	// if not solo setting
									loopCount[track] = trackLoopMeas[track];
								}

								nextStatus[track] = RECORDING;
								busyTracks++;
								setFastRecLed(track);
							} else {	// UNDO REC PRESS
								nextStatus[track] = NOTHING;
								busyTracks--;

								if (solo_setting[track] && nextSoloTrack == track)
									nextSoloTrack = -1;

								if (currentSoloTrack >= 0)
									cancelNextStatus(currentSoloTrack);

								setEmptyLed(track);
							}
						break;

						case IDLE:										// rec press & track is IDLE -> go overdubbinig
							if (nextStatus[track] == NOTHING) {

								if (solo_setting[track]) {

									if (nextSoloTrack < 0) {	// se non ci sono solo track prenotate

										if (currentSoloTrack >= 0)	// se è in funzione un'altra solo track, ne prenota lo stop
											nextStatus[currentSoloTrack] = IDLE;
										else	// se non è in funzione un'altra solo track, allora ne prenota subito l'overdub
											loopCount[track] = trackLoopMeas[track];

									} else if (nextSoloTrack != track) {	// se era già prenotata un'altra solo track, allora si annulla la precedente prenotata
										resetIdleEmptyStatus(nextSoloTrack);
										busyTracks--;
									}

									nextSoloTrack = track;
									
								} else {	// se non è una solo track
									loopCount[track] = trackLoopMeas[track];
								}

								if (startImm_setting[track]) {
									startNow[track] = true;
									loopCount[track] = 1;
								}
								nextStatus[track] = OVERDUBBING;
								busyTracks++;

								if (!loopSync_setting[track]) {
									if (!rev_setting[track]) {
										playingDirection[track] = FORWARD;
										samplePos[track] = 0;
									} else {
										playingDirection[track] = REVERSE;
										samplePos[track] = totalSamples[track];
									}
								}

								setFastOverdubLed(track);

							} else { // UNDO REC PRESS

								if (nextStatus[track] == OVERDUBBING) {
									nextStatus[track] = NOTHING;
									busyTracks--;

									if (solo_setting[track] && nextSoloTrack == track)
										nextSoloTrack = -1;

									if (currentSoloTrack >= 0)
										cancelNextStatus(currentSoloTrack);

									setIdleLed(track);
								} else {	// se il nextStatus è PLAYING vai a OVERDUBBING
									nextStatus[track] = OVERDUBBING;
									setFastOverdubLed(track);
								}
							}
						break;

						case PLAYING:									// rec press & track is PLAYING -> go OVERDUBBING
							if (nextStatus[track] != OVERDUBBING) {
								
								nextStatus[track] = OVERDUBBING;

								if (solo_setting[track]) {

									if (nextSoloTrack == track)	// se quella che stai premendo e la nextSoloTrack è la stessa
										nextSoloTrack = -1;	// dovrebbe equivalere a non far niente
									else if (nextSoloTrack >= 0) {	// se la nextSoloTrack è un'altra riportala in IDLE
										resetIdleEmptyStatus(nextSoloTrack);

										nextSoloTrack = -1;
										busyTracks--;
									}
								
								} 
								
								if (startImm_setting[track])
									startNow[track] = true;

								setPlayToRecLed(track);

							} else {	// UNDO REC PRESS

								if (currentSoloTrack >= 0)
									cancelNextStatus(currentSoloTrack);

								nextStatus[track] = NOTHING;
								setPlayLed(track);

							}
						break;

						case RECORDING:									// rec press & track is RECORDING -> go PLAYING
							if (loopSync_setting[track]) {
								if (nextStatus[track] != PLAYING) {
									nextStatus[track] = PLAYING;
									setRecToPlayLed(track);
								} else {
									nextStatus[track] = NOTHING;
									setRecLed(track);
								}
							} else {

								nextStatus[track] = OVERDUBBING;	// if not loopSync then go OVERDUBBING instead of PLAYING
								stopNow[track] = true;
							}
						break;

						case OVERDUBBING:								// rec press & track is OVERDUBBING -> go PLAYINIG
							if (nextStatus[track] != PLAYING) {

								nextStatus[track] = PLAYING;

								if (solo_setting[track]) {

									if (nextSoloTrack < 0) {	// se non ci sono solo track prenotate

										if (currentSoloTrack != track) {	// se la traccia premuta è un'altra
											cancelNextStatus(currentSoloTrack);
											nextSoloTrack = track;
										}

									} else if (nextSoloTrack != track) {	// se era già prenotata un'altra solo track, allora si annulla la precedente prenotata
										resetIdleEmptyStatus(nextSoloTrack);
										busyTracks--;
										nextSoloTrack = track;
									}
									
								} 

								/*
								if (playSequence == PLAY_STOP) {	// nella playSequence PLAY_STOP
									if (stopImm_setting[track])	// premendo rec in overdubbing con lo stopImm_setting fa partire subito il play
										startNow[track] = true;	// (se invece si preme play in overdubbing partirà il play con lo startImm_setting)
								} else {
									if (startImm_setting[track] || stopImm_setting[track])	// nelle altre playSequence parte il play
										startNow[track] = true;								//  anche con lo startImm_setting
								}
								*/

								if (stopImm_setting[track])	// premendo rec in overdubbing con lo stopImm_setting fa partire subito il play
									startNow[track] = true;	// (se invece si preme play in overdubbing partirà il play con lo startImm_setting)

								setRecToPlayLed(track);
							} else {	// UNDO REC PRESS
								
								if (solo_setting[track]) {
									if (currentSoloTrack == track) {
										cancelNextStatus(currentSoloTrack);
										nextSoloTrack = -1;
									} else if (nextSoloTrack >= 0) {
										resetIdleEmptyStatus(nextSoloTrack);
										nextSoloTrack = -1;
										busyTracks--;
									}
								}

								nextStatus[track] = NOTHING;
								setOverdubLed(track);
							}
						break;
					}

				}

			}

			prevPlayTrig[track] = playTrig[track];
			prevRecTrig[track] = recTrig[track];
			prevStopTrig[track] = stopTrig[track];
			prevEraseTrig[track] = eraseTrig[track];
		}

/*

																								░█████╗░██╗░░░░░░█████╗░░█████╗░██╗░░██╗
																								██╔══██╗██║░░░░░██╔══██╗██╔══██╗██║░██╔╝
																								██║░░╚═╝██║░░░░░██║░░██║██║░░╚═╝█████═╝░
																								██║░░██╗██║░░░░░██║░░██║██║░░██╗██╔═██╗░
																								╚█████╔╝███████╗╚█████╔╝╚█████╔╝██║░╚██╗
																								░╚════╝░╚══════╝░╚════╝░░╚════╝░╚═╝░░╚═╝
*/

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
					barSample++;
				}

			} else {

				// ************************************************ EXTERNAL CLOCK

				// Reset
				rstBarBut = params[CLOCK_RST_SWITCH].getValue();
				if (rstBarBut >= 1.f && prevRstBarBut < 1.f) {
					beatCounter = currentBeatMaxPerBar;
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
				barSample++;
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

/*
																	██╗░░░░░░█████╗░░█████╗░██████╗░  ░██████╗████████╗░█████╗░██████╗░████████╗
																	██║░░░░░██╔══██╗██╔══██╗██╔══██╗  ██╔════╝╚══██╔══╝██╔══██╗██╔══██╗╚══██╔══╝
																	██║░░░░░██║░░██║██║░░██║██████╔╝  ╚█████╗░░░░██║░░░███████║██████╔╝░░░██║░░░
																	██║░░░░░██║░░██║██║░░██║██╔═══╝░  ░╚═══██╗░░░██║░░░██╔══██║██╔══██╗░░░██║░░░
																	███████╗╚█████╔╝╚█████╔╝██║░░░░░  ██████╔╝░░░██║░░░██║░░██║██║░░██║░░░██║░░░
																	╚══════╝░╚════╝░░╚════╝░╚═╝░░░░░  ╚═════╝░░░░╚═╝░░░╚═╝░░╚═╝╚═╝░░╚═╝░░░╚═╝░░░
*/


		for (int track = 0; track < MAX_TRACKS; track++) {

			// **********************************************************************************************************************
			// ************************  LOOP  START CHECK  ************  AKA  ************* NEXT STATUS  CHECK *********************
			// **********************************************************************************************************************
			
			barChecked[track] = false;

			if (globalStatus == PLAYING) {

				switch (nextStatus[track]) {

					case NOTHING:
					case IDLE:
					break;

					case PLAYING:						// NEXT STATUS PLAY

						if (trackStatus[track] != RECORDING) {
							if (barReached) {
								loopCount[track]++;
								barChecked[track] = true;
							}

							if (!solo_setting[track] || (solo_setting[track] && currentSoloTrack < 0)) {

								if (loopSync_setting[track] && startNow[track]) {				// ********** if loopSync && start now

									if (trackStatus[track] == IDLE) {
										if (!rev_setting[track]) {
											playingDirection[track] = FORWARD;
											samplePos[track] = barSample;
										} else {
											playingDirection[track] = REVERSE;
											samplePos[track] = totalSamples[track] - barSample;
										}

										fadeInValue[track] = 0.f;
										fadeIn[track] = true;
										fadeInDelta[track] = 1.f / minTimeSamples;
										
									} else {		// if it's OVERDUBBING & startNow -> rec fade out
										// rec fade out
										recFade[track] = true;
										recFadeValue[track] = 1.f;
										recFadeDelta[track] = -1 / minTimeSamples;

										extraRecording[track] = true;
										extraRecDirection[track] = playingDirection[track];
										extraRecCount[track] = 0;
										extraRecMaxSamples = minTimeSamples;
										extraRecPos[track] = samplePos[track];

									}

									nextStatus[track] = NOTHING;
									trackStatus[track] = PLAYING;

									if (solo_setting[track]) {
										currentSoloTrack = track;
										nextSoloTrack = -1;
									}

									setPlayLed(track);

								} else if (loopSync_setting[track] && loopCount[track] > trackLoopMeas[track]) {	// if loopSync && end of loop reached

									if (trackStatus[track] == OVERDUBBING && playingDirection[track] == FORWARD) {

										extraRecording[track] = true;
										extraSamples[track] = true;
										extraRecDirection[track] = playingDirection[track];
										extraRecCount[track] = 0;
										extraRecMaxSamples = tailSamples;
										extraRecPos[track] = samplePos[track];

										xFadePlay(track);
									} else {

										if (fadeInOnPlay[track]) {
											fadeInValue[track] = 0.f;
											fadeIn[track] = true;
											fadeInDelta[track] = 1000 / (params[XFADE_KNOB_PARAM+track].getValue() * sampleRate);
										}
									}

									loopCount[track] = 1;

									if (!rev_setting[track]) {
										playingDirection[track] = FORWARD;
										samplePos[track] = 0;
									} else {
										playingDirection[track] = REVERSE;
										samplePos[track] = totalSamples[track];
									}

									nextStatus[track] = NOTHING;
									trackStatus[track] = PLAYING;

									setPlayLed(track);

									if (solo_setting[track]) {
										currentSoloTrack = track;
										nextSoloTrack = -1;
									}
								
								} else if (!loopSync_setting[track]) {	// if NOT loopSync

									if (trackStatus[track] == IDLE) {
										nextStatus[track] = NOTHING;
										trackStatus[track] = PLAYING;

										if (solo_setting[track]) {
											currentSoloTrack = track;
											nextSoloTrack = -1;
										}

										if (fadeInOnPlay[track]) {
											fadeInValue[track] = 0.f;
											fadeIn[track] = true;
											fadeInDelta[track] = 1000 / (params[XFADE_KNOB_PARAM+track].getValue() * sampleRate);
										}

										setPlayLed(track);

									} else if (startNow[track]) {	// if trackStatus is OVERDUBBING and startNow

										if (trackStatus[track] == OVERDUBBING) {
											recFade[track] = true;					// rec fade out
											recFadeValue[track] = 1.f;
											recFadeDelta[track] = -1 / minTimeSamples;

											extraRecording[track] = true;
											extraRecDirection[track] = playingDirection[track];
											extraRecCount[track] = 0;
											extraRecMaxSamples = minTimeSamples;
											extraRecPos[track] = samplePos[track];

										}

										trackStatus[track] = PLAYING;
										nextStatus[track] = NOTHING;
										setPlayLed(track);
									}
								}
							}
						}

					break;


					case RECORDING:

						if (barReached) {
							loopCount[track]++;
							barChecked[track] = true;
						}

						if (!solo_setting[track] || (solo_setting[track] && currentSoloTrack < 0)) {
							if ((loopSync_setting[track] && barReached) || !loopSync_setting[track]) {
								loopCount[track] = 1;
								playingDirection[track] = FORWARD;
								samplePos[track] = 0;

								trackRecorded[track] = true;
								recordedTracks++;

								trackStatus[track] = RECORDING;
								nextStatus[track] = NOTHING;
								recFadeValue[track] = 1.f;

								if (solo_setting[track]) {
									currentSoloTrack = track;
									nextSoloTrack = -1;
								}

								setRecLed(track);
							}
						}
						
					break;
					
					case OVERDUBBING:

						if (barReached) {
							loopCount[track]++;
							barChecked[track] = true;
						}


						if (!solo_setting[track] || (solo_setting[track] && currentSoloTrack < 0)) {

							if (loopSync_setting[track] && startNow[track]) {				// ********** if loopSync && start now

								if (trackStatus[track] == IDLE) {
									if (!rev_setting[track]) {
										playingDirection[track] = FORWARD;
										samplePos[track] = barSample;
									} else {
										playingDirection[track] = REVERSE;
										samplePos[track] = totalSamples[track] - barSample;
									}

									fadeInValue[track] = 0.f;
									fadeIn[track] = true;
									fadeInDelta[track] = 1.f / minTimeSamples;
								}

								recFade[track] = true;
								recFadeValue[track] = 0.f;
								recFadeDelta[track] = 1.f / minTimeSamples;

								trackStatus[track] = OVERDUBBING;
								nextStatus[track] = NOTHING;

								if (solo_setting[track]) {
									currentSoloTrack = track;
									nextSoloTrack = -1;
								}

								setOverdubLed(track);
								
							} else if (loopSync_setting[track] && loopCount[track] > trackLoopMeas[track]) {	// if loopSync && end of loop reached

								loopCount[track] = 1;

								eolPulse[track] = true;
								eolPulseTime[track] = oneMsSamples;		// eol pulse when an overdubbing is starting

								if (!rev_setting[track]) {
									if (trackStatus[track] == PLAYING) 
										xFadePlay(track);
									samplePos[track] = 0;
									playingDirection[track] = FORWARD;
								} else {
									samplePos[track] = totalSamples[track];
									playingDirection[track] = REVERSE;
								}

								trackStatus[track] = OVERDUBBING;
								nextStatus[track] = NOTHING;

								if (solo_setting[track]) {
									currentSoloTrack = track;
									nextSoloTrack = -1;
								}

								setOverdubLed(track);
								
							} else if (!loopSync_setting[track]) {	// if NOT loopSync
								
								if (trackStatus[track] == IDLE) {
									trackStatus[track] = OVERDUBBING;
									nextStatus[track] = NOTHING;

									if (solo_setting[track]) {
										currentSoloTrack = track;
										nextSoloTrack = -1;
									}

									if (fadeInOnPlay[track]) {
										fadeInValue[track] = 0.f;
										fadeIn[track] = true;
										fadeInDelta[track] = 1000 / (params[XFADE_KNOB_PARAM+track].getValue() * sampleRate);
									}

									setOverdubLed(track);

								} else if (startNow[track]) {	// if it's PLAYING & startNow

									trackStatus[track] = OVERDUBBING;
									nextStatus[track] = NOTHING;

									// recFadeIn
									recFade[track] = true;
									recFadeValue[track] = 0.f;
									recFadeDelta[track] = 1 / minTimeSamples;

									setOverdubLed(track);
								}
							}
						}

					break;

				}
			}
		
			// **********************************************************************************************************
			// **************  LOOP  END CHECK  ************  AKA  ********* TRACK STATUS CHECK *************************
			// **********************************************************************************************************

/*
																		██╗░░░░░░█████╗░░█████╗░██████╗░  ███████╗███╗░░██╗██████╗░
																		██║░░░░░██╔══██╗██╔══██╗██╔══██╗  ██╔════╝████╗░██║██╔══██╗
																		██║░░░░░██║░░██║██║░░██║██████╔╝  █████╗░░██╔██╗██║██║░░██║
																		██║░░░░░██║░░██║██║░░██║██╔═══╝░  ██╔══╝░░██║╚████║██║░░██║
																		███████╗╚█████╔╝╚█████╔╝██║░░░░░  ███████╗██║░╚███║██████╔╝
																		╚══════╝░╚════╝░░╚════╝░╚═╝░░░░░  ╚══════╝╚═╝░░╚══╝╚═════╝░
*/

			loopEnd[track] = false;  //   initialize loopEnd seeking

			if (trackStatus[track] != EMPTY && trackStatus[track] != IDLE) {

				if (stopNow[track]) {

					loopEnd[track] = true;
					if (!eolPulseOnStop)
						notEolPulse[track] = true;

				} else {

					if (loopSync_setting[track]) {

						if (barReached) {
							if (!barChecked[track])
								loopCount[track]++;
							if (loopCount[track] > trackLoopMeas[track]) {
								loopEnd[track] = true;
								loopCount[track] = 1;
							}
						}

					} else {	// if not loop sync

						switch (trackStatus[track]) {
							case IDLE:
								switch (nextStatus[track]) {
									case PLAYING:
									case OVERDUBBING:
										loopEnd[track] = true;
									break;

								}
							break;

							case PLAYING:
							case OVERDUBBING:
								if ((samplePos[track] > totalSamples[track] && playingDirection[track] == FORWARD) ||
									(samplePos[track] < 0 && playingDirection[track] == REVERSE))
										loopEnd[track] = true;
							break;
						}
					}
				}

				if (loopEnd[track]) {

					loopEnd[track] = false;

					if (!notEolPulse[track]) {
						eolPulse[track] = true;
						eolPulseTime[track] = oneMsSamples;	
					} else {
						notEolPulse[track] = false;
					}

					switch (trackStatus[track]) {

						case PLAYING:

							if (oneShot_setting[track] && !stopNow[track]) {

								if (extraPlayingFadeOut[track])
									extraPlayingFadeOut[track] = false;
								else
									xFadePlay(track);

								if (nextStatus[track] == OVERDUBBING) {
									trackStatus[track] = OVERDUBBING;
									nextStatus[track] = NOTHING;

									if (!rev_setting[track]) { 
										playingDirection[track] = FORWARD;
										samplePos[track] = 0;
									} else {
										playingDirection[track] = REVERSE;
										samplePos[track] = totalSamples[track];
									}

									setOverdubLed(track);
								} else {
									busyTracks--;
									trackStatus[track] = IDLE;
									nextStatus[track] = NOTHING;

									if (solo_setting[track]) {
										startNewSolo = true;
										currentSoloTrack = -1;
										/*
										if (nextSoloTrack < 0) {
											playTail[track] = true;
											tailEnd[track] = samplePos[track] + tailSamples;
										}
										*/
										if (nextSoloTrack < 0 && playFullTail[track]) {
											playTail[track] = true;
											tailEnd[track] = samplePos[track] + tailSamples;
										}
									} else if (!rev_setting[track] && playFullTail[track]) {
										playTail[track] = true;
										tailEnd[track] = samplePos[track] + tailSamples;
									}

									setIdleLed(track);
								}
								
							} else {
								switch (nextStatus[track]) {
									case IDLE:
										busyTracks--;
										trackStatus[track] = IDLE;
										nextStatus[track] = NOTHING;

										setIdleLed(track);

										if (solo_setting[track]) {
											startNewSolo = true;
											currentSoloTrack = -1;
											/*
											if (nextSoloTrack < 0) {
												playTail[track] = true;
												tailEnd[track] = samplePos[track] + tailSamples;
											}
											*/
											if (nextSoloTrack < 0 && playFullTail[track]) {
												playTail[track] = true;
												tailEnd[track] = samplePos[track] + tailSamples;
											}
										} else if (!rev_setting[track] && playFullTail[track] && !stopNow[track]) {
											playTail[track] = true;
											tailEnd[track] = samplePos[track] + tailSamples;
										}
									break;

									case OVERDUBBING:
										trackStatus[track] = OVERDUBBING;
										nextStatus[track] = NOTHING;

										setOverdubLed(track);
									break;
								}

								if (!rev_setting[track]) { 
									playingDirection[track] = FORWARD;
									if (extraPlayingFadeOut[track])
										extraPlayingFadeOut[track] = false;
									else
										xFadePlay(track);

									samplePos[track] = 0;
								} else {
									playingDirection[track] = REVERSE;
									if (stopNow[track]) {
										if (extraPlayingFadeOut[track])
											extraPlayingFadeOut[track] = false;
										else
											xFadePlay(track);
									}
									samplePos[track] = totalSamples[track];
								}
							}
					
						break;

						case RECORDING:
							totalSamples[track] = samplePos[track]-2;
							totalSampleC[track] = samplePos[track]-1;
							extraRecording[track] = true;
							extraSamples[track] = true;
							extraRecDirection[track] = FORWARD;
							extraRecCount[track] = 0;
							extraRecMaxSamples = tailSamples;
							extraRecPos[track] = samplePos[track];

							if (!loopSync_setting[track] && recordedTracks == 1) {
								detectTempo(track);
								if (!extConn) {
									clockSample = 1.0;
									resetStart = true;
									beatCounter = 20;
								}
							}
							
							switch (nextStatus[track]) {
								case IDLE:
									busyTracks--;
									trackStatus[track] = IDLE;
									nextStatus[track] = NOTHING;

									if (solo_setting[track]) {
										startNewSolo = true;
										currentSoloTrack = -1;
									}

									setIdleLed(track);
								break;

								case PLAYING:
									trackStatus[track] = PLAYING;
									nextStatus[track] = NOTHING;

									setPlayLed(track);
								break;

								
								case OVERDUBBING:	// this is possible only when is NOT loopSync 
									trackStatus[track] = OVERDUBBING;
									nextStatus[track] = NOTHING;

									setOverdubLed(track);
								break;
								

								case NOTHING:
									if (!oneShot_setting[track]) {
										nextStatus[track] = NOTHING;

										if (overdubAfterRec) {
											trackStatus[track] = OVERDUBBING;
											setOverdubLed(track);
										} else {
											trackStatus[track] = PLAYING;
											setPlayLed(track);
										}
										
									} else {
										busyTracks--;
										trackStatus[track] = IDLE;
										nextStatus[track] = NOTHING;

										if (solo_setting[track]) {
											startNewSolo = true;
											currentSoloTrack = -1;
										}

										setIdleLed(track);
									}
									
								break;
							}

							if (!rev_setting[track]) { 
								playingDirection[track] = FORWARD;
								samplePos[track] = 0;
							} else {
								playingDirection[track] = REVERSE;
								samplePos[track] = totalSamples[track];
							}	
					
						break;
						
						case OVERDUBBING:
							
							if (playingDirection[track] == FORWARD) {

								extraRecording[track] = true;
								extraRecDirection[track] = FORWARD;
								extraRecCount[track] = 0;
								if (!stopNow[track]) {
									extraRecMaxSamples = tailSamples;
									extraSamples[track] = true;
								} else {
									extraRecMaxSamples = minTimeSamples;
									recFade[track] = true;
									recFadeValue[track] = 0.f;
									recFadeDelta[track] = -1 / minTimeSamples;
								}
									
								extraRecPos[track] = samplePos[track];

								if (extraPlayingFadeOut[track])
									extraPlayingFadeOut[track] = false;
								else
									xFadePlay(track);

							} else if (stopNow[track]) {	// if it's REVERSE and it's stopNow
								recFade[track] = true;
								recFadeValue[track] = 0.f;
								recFadeDelta[track] = -1 / minTimeSamples;

								extraRecording[track] = true;
								extraRecDirection[track] = REVERSE;
								extraRecCount[track] = 0;
								extraRecMaxSamples = minTimeSamples;
								extraRecPos[track] = samplePos[track];

								if (extraPlayingFadeOut[track])
									extraPlayingFadeOut[track] = false;
								else
									xFadePlay(track);
							}
							
							if (oneShot_setting[track] && !stopNow[track]) {

								if (nextStatus[track] == PLAYING) {
									trackStatus[track] = PLAYING;
									nextStatus[track] = NOTHING;

									if (!rev_setting[track]) { 
										if (extraPlayingFadeOut[track])
											extraPlayingFadeOut[track] = false;
										else
											xFadePlay(track);

										playingDirection[track] = FORWARD;
										samplePos[track] = 0;
									} else {
										playingDirection[track] = REVERSE;
										samplePos[track] = totalSamples[track];
									}
									
									setPlayLed(track);

								} else {
									busyTracks--;								
									trackStatus[track] = IDLE;
									nextStatus[track] = NOTHING;

									if (solo_setting[track]) {
										startNewSolo = true;
										currentSoloTrack = -1;
										/*
										if (nextSoloTrack < 0) {
											playTail[track] = true;
											tailEnd[track] = samplePos[track] + tailSamples;
										}
										*/
										if (nextSoloTrack < 0 && playFullTail[track]) {
											playTail[track] = true;
											tailEnd[track] = samplePos[track] + tailSamples;
										}
									} else if (!rev_setting[track] && playFullTail[track]) {
										playTail[track] = true;
										tailEnd[track] = samplePos[track] + tailSamples;
									}

									setIdleLed(track);
								}
							} else {
								switch (nextStatus[track]) {
									case IDLE:
										busyTracks--;
										trackStatus[track] = IDLE;
										nextStatus[track] = NOTHING;

										if (solo_setting[track]) {
											startNewSolo = true;
											currentSoloTrack = -1;
											/*
											if (nextSoloTrack < 0) {
												playTail[track] = true;
												tailEnd[track] = samplePos[track] + tailSamples;
											}
											*/
											if (nextSoloTrack < 0 && playFullTail[track]) {
												playTail[track] = true;
												tailEnd[track] = samplePos[track] + tailSamples;
											}
										} else if (!rev_setting[track] && playFullTail[track] && !stopNow[track]) {
											playTail[track] = true;
											tailEnd[track] = samplePos[track] + tailSamples;
										}

										setIdleLed(track);
									break;

									case PLAYING:
										trackStatus[track] = PLAYING;
										nextStatus[track] = NOTHING;

										setPlayLed(track);

									break;
								}
								
								if (!rev_setting[track]) { 
									if (extraPlayingFadeOut[track])
										extraPlayingFadeOut[track] = false;
									else
										xFadePlay(track);
									playingDirection[track] = FORWARD;
									samplePos[track] = 0;
								} else {
									if (stopNow[track]) {
										if (extraPlayingFadeOut[track])
											extraPlayingFadeOut[track] = false;
										else
											xFadePlay(track);
									}
									playingDirection[track] = REVERSE;
									samplePos[track] = totalSamples[track];
								}
								
							}
						break;
					}


					if (startNewSolo) {
						startNewSolo = false;
						if (nextSoloTrack < 0)
							currentSoloTrack = -1;
						else if (!loopSync_setting[track] && loopSync_setting[nextSoloTrack]) {
							loopCount[nextSoloTrack] = trackLoopMeas[nextSoloTrack];
							currentSoloTrack = -1;
						} else {
							switch (nextStatus[nextSoloTrack]) {
								case PLAYING:
									trackStatus[nextSoloTrack] = PLAYING;
									nextStatus[nextSoloTrack] = NOTHING;

									if (fadeInOnPlay[nextSoloTrack]) {
										if (!extraPlaying[nextSoloTrack])
											fadeInValue[nextSoloTrack] = 0.f;
										else {
											extraPlaying[nextSoloTrack] = false;
											fadeInValue[nextSoloTrack] = 1-xFadeValue[nextSoloTrack];
										}
										fadeIn[nextSoloTrack] = true;
										fadeInDelta[nextSoloTrack] = 1000 / (params[XFADE_KNOB_PARAM+nextSoloTrack].getValue() * sampleRate);
									}

									setPlayLed(nextSoloTrack);
									if (nextSoloTrack > track)
										loopCount[nextSoloTrack] = 0;	// tested
									else
										loopCount[nextSoloTrack] = 1;	// tested

									if (!rev_setting[nextSoloTrack]) {
										playingDirection[nextSoloTrack] = FORWARD;
										samplePos[nextSoloTrack] = 0;
									} else {
										playingDirection[nextSoloTrack] = REVERSE;
										samplePos[nextSoloTrack] = totalSamples[nextSoloTrack];
									}
								break;

								case RECORDING:
									trackStatus[nextSoloTrack] = RECORDING;
									nextStatus[nextSoloTrack] = NOTHING;

									recordedTracks++;
									trackRecorded[nextSoloTrack] = true;

									setRecLed(nextSoloTrack);

									if (nextSoloTrack > track)
										loopCount[nextSoloTrack] = 0;
									else
										loopCount[nextSoloTrack] = 1;

									playingDirection[nextSoloTrack] = FORWARD;
									samplePos[nextSoloTrack] = 0;
								break;

								case OVERDUBBING:
									trackStatus[nextSoloTrack] = OVERDUBBING;
									nextStatus[nextSoloTrack] = NOTHING;

									setOverdubLed(nextSoloTrack);

									if (nextSoloTrack > track)
										loopCount[nextSoloTrack] = 0;
									else
										loopCount[nextSoloTrack] = 1;

									if (!rev_setting[nextSoloTrack]) {
										playingDirection[nextSoloTrack] = FORWARD;
										samplePos[nextSoloTrack] = 0;
									} else {
										playingDirection[nextSoloTrack] = REVERSE;
										samplePos[nextSoloTrack] = totalSamples[nextSoloTrack];
									}
								break;
							}
							currentSoloTrack = nextSoloTrack;
							nextSoloTrack = -1;
						}
					}
				}

			}	// end LOOP END if

			// EOL check

			if (eolPulse[track]) {
				eolPulseTime[track]--;
				if (eolPulseTime[track] < 0) {
					eolPulse[track] = false;
					outputs[EOL_OUTPUT+track].setVoltage(0.f);
				} else
					outputs[EOL_OUTPUT+track].setVoltage(10.f);
			}
			// ******************************************************************** PLAY / REC / OVERDUB
			currentOutput[track][LEFT] = 0.f;
			currentOutput[track][RIGHT] = 0.f;

			switch (trackStatus[track]) {
				case PLAYING:

					if (samplePos[track] > totalSamples[track] && !extraPlayingFadeOut[track]) {
						extraPlayingFadeOut[track] = true;
						xFadePlay(track);
						samplePos[track] = totalSampleC[track];
					}

					if (samplePos[track] < totalSampleC[track] && samplePos[track] >= 0) {

						if (!fadeIn[track]) {
							currentOutput[track][LEFT] = trackBuffer[track][LEFT][samplePos[track]];
							currentOutput[track][RIGHT] = trackBuffer[track][RIGHT][samplePos[track]];
						} else {
							fadeInValue[track] += fadeInDelta[track];
							if (fadeInValue[track] > 1.f) {
								fadeIn[track] = false;
								fadeInValue[track] = 1.f;
							}
							currentOutput[track][LEFT] = trackBuffer[track][LEFT][samplePos[track]] * fadeInValue[track];
							currentOutput[track][RIGHT] = trackBuffer[track][RIGHT][samplePos[track]] * fadeInValue[track];
						}
					
						if (playingDirection[track] == FORWARD)
							samplePos[track]++;
						else
							samplePos[track]--;
					}
				break;

				case RECORDING:
					trackBuffer[track][LEFT].push_back(inputValue[currentSource[track]][LEFT]);
					trackBuffer[track][RIGHT].push_back(inputValue[currentSource[track]][RIGHT]);
					samplePos[track]++;
				break;

				case OVERDUBBING:

					if (samplePos[track] >= trackBuffer[track][LEFT].size()) {
						trackBuffer[track][LEFT].push_back(0.f);
						trackBuffer[track][RIGHT].push_back(0.f);
					}

					if (samplePos[track] >= 0) {
						
						if (!fadeIn[track]) {
							currentOutput[track][LEFT] = trackBuffer[track][LEFT][samplePos[track]];
							currentOutput[track][RIGHT] = trackBuffer[track][RIGHT][samplePos[track]];
						} else {
							fadeInValue[track] += fadeInDelta[track];
							if (fadeInValue[track] > 1.f) {
								fadeIn[track] = false;
								fadeInValue[track] = 1.f;
							}
							currentOutput[track][LEFT] = trackBuffer[track][LEFT][samplePos[track]] * fadeInValue[track];
							currentOutput[track][RIGHT] = trackBuffer[track][RIGHT][samplePos[track]] * fadeInValue[track];
						}
						
						if (recFade[track]) {
							recFadeValue[track] += recFadeDelta[track];
							if (recFadeValue[track] < 0) {
								recFadeValue[track] = 0;
								recFade[track] = false;
							} else if ( recFadeValue[track] > 1) {
								recFadeValue[track] = 1;
								recFade[track] = false;
							}
						}

						trackBuffer[track][LEFT][samplePos[track]] += inputValue[currentSource[track]][LEFT] * recFadeValue[track];
						trackBuffer[track][RIGHT][samplePos[track]] += inputValue[currentSource[track]][RIGHT] * recFadeValue[track];

						if (playingDirection[track] == FORWARD)
							samplePos[track]++;
						else
							samplePos[track]--;
					}

				break;
			}

			if (extraPlaying[track]) {
				if (!playTail[track]) {
					xFadeValue[track] -= xFadeDelta[track];
					if (xFadeValue[track] < 0) {
						extraPlaying[track] = false;
					} else {
						if (extraPlayPos[track] < trackBuffer[track][LEFT].size()) {
							currentOutput[track][LEFT] *= 1-xFadeValue[track];
							currentOutput[track][RIGHT] *= 1-xFadeValue[track];

							currentOutput[track][LEFT] += trackBuffer[track][LEFT][extraPlayPos[track]] * xFadeValue[track];
							currentOutput[track][RIGHT] += trackBuffer[track][RIGHT][extraPlayPos[track]] * xFadeValue[track];

							if (extraPlayDirection[track] == FORWARD)
								extraPlayPos[track]++;
							else
								extraPlayPos[track]--;
						} else {
							extraPlaying[track] = false;
						}
					}
				} else {	// if it's playing full tail, only if direction is FORWARD
					if (extraPlayPos[track] < tailEnd[track] - minTimeSamples) {
						currentOutput[track][LEFT] += trackBuffer[track][LEFT][extraPlayPos[track]];
						currentOutput[track][RIGHT] += trackBuffer[track][RIGHT][extraPlayPos[track]];

						extraPlayPos[track]++;

					} else {
						if (!fadeTail[track]) {
							fadeTail[track] = true;
							fadeTailValue[track] = 1;
						}
						if (extraPlayPos[track] < tailEnd[track]) {
							fadeTailValue[track] -= fadeTailDelta;
							currentOutput[track][LEFT] += trackBuffer[track][LEFT][extraPlayPos[track]] * fadeTailValue[track];
							currentOutput[track][RIGHT] += trackBuffer[track][RIGHT][extraPlayPos[track]] * fadeTailValue[track];
							extraPlayPos[track]++;
						} else {
							extraPlaying[track] = false;
							playTail[track] = false;
							fadeTail[track] = false;
						}
					}
				}
			}

			if (extraRecording[track]) {
				extraRecCount[track]++;

				if (extraRecCount[track] > extraRecMaxSamples) {
					extraRecording[track] = false;

				} else {
					if (extraRecPos[track] >= trackBuffer[track][LEFT].size()) {
						trackBuffer[track][LEFT].push_back(0.f);
						trackBuffer[track][RIGHT].push_back(0.f);
					}

					if (recFade[track]) {
						recFadeValue[track] += recFadeDelta[track];
						if (recFadeValue[track] < 0) {
							recFadeValue[track] = 0;
							recFade[track] = false;
						} else if ( recFadeValue[track] > 1) {
							recFadeValue[track] = 1;
							recFade[track] = false;
						}
					}

					//if (extraRecPos[track] >= 0) {	// extraRecPos is unsigned. this condition means nothing
						trackBuffer[track][LEFT][extraRecPos[track]] += inputValue[currentSource[track]][LEFT] * recFadeValue[track];
						trackBuffer[track][RIGHT][extraRecPos[track]] += inputValue[currentSource[track]][RIGHT] * recFadeValue[track];
					//}
					
					if (extraRecDirection[track] == FORWARD)
						extraRecPos[track]++;
					else
						extraRecPos[track]--;
				}
			}
			

			// ************************ TRACK OUTPUT MANAGEMENT

			panKnobValue = params[PAN_KNOB_PARAM+track].getValue();

			if (panKnobValue < 0.f) {
				panLeftCoeff = 1.f;
				panRightCoeff = 1.f + panKnobValue;
			} else {
				panLeftCoeff = 1.f - panKnobValue;
				panRightCoeff = 1.f; 
			}

			if (!srcToTrack[track]) {
				if (!srcToTrackFade[track]) {
					currentOutput[track][LEFT] *= volTrack[track] * panLeftCoeff;
					currentOutput[track][RIGHT] *= volTrack[track] * panRightCoeff;
					
					sumOutput[LEFT] += currentOutput[track][LEFT];
					sumOutput[RIGHT] += currentOutput[track][RIGHT];
				} else {	// residuo di fade out dell'input
					sumOutput[LEFT] += volTrack[track]  * currentOutput[track][LEFT] * panLeftCoeff;
					sumOutput[RIGHT] += volTrack[track] * currentOutput[track][RIGHT] * panRightCoeff;

					currentOutput[track][LEFT] += volTrack[track] * srcToTrackValue[track] * inputValue[currentSource[track]][LEFT] * panLeftCoeff;
					currentOutput[track][RIGHT] += volTrack[track] * srcToTrackValue[track] * inputValue[currentSource[track]][RIGHT] * panRightCoeff;
				}

			} else {
				if (!srcToTrackFade[track]) {
					sumOutput[LEFT] += volTrack[track]  * currentOutput[track][LEFT] * panLeftCoeff;
					sumOutput[RIGHT] += volTrack[track] * currentOutput[track][RIGHT] * panRightCoeff;

					currentOutput[track][LEFT] += inputValue[currentSource[track]][LEFT];
					currentOutput[track][RIGHT] += inputValue[currentSource[track]][RIGHT];

					currentOutput[track][LEFT] *= volTrack[track] * panLeftCoeff;
					currentOutput[track][RIGHT] *= volTrack[track] * panRightCoeff;

				} else {
					sumOutput[LEFT] += volTrack[track]  * currentOutput[track][LEFT] * panLeftCoeff;
					sumOutput[RIGHT] += volTrack[track] * currentOutput[track][RIGHT] * panRightCoeff;

					currentOutput[track][LEFT] += srcToTrackValue[track] * inputValue[currentSource[track]][LEFT];
					currentOutput[track][RIGHT] += srcToTrackValue[track] * inputValue[currentSource[track]][RIGHT];

					currentOutput[track][LEFT] *= volTrack[track] * panLeftCoeff;
					currentOutput[track][RIGHT] *= volTrack[track] * panRightCoeff;	

				}
			}

			if (currentOutput[track][LEFT] > 10.f)
				currentOutput[track][LEFT] = 10.f;
			else if (currentOutput[track][LEFT] < -10.f)
				currentOutput[track][LEFT] = -10.f;

			if (currentOutput[track][RIGHT] > 10.f)
				currentOutput[track][RIGHT] = 10.f;
			else if (currentOutput[track][RIGHT] < -10.f)
				currentOutput[track][RIGHT] = -10.f;

			outputs[TRACK_LEFT_OUTPUT+track].setVoltage(currentOutput[track][LEFT]);
			outputs[TRACK_RIGHT_OUTPUT+track].setVoltage(currentOutput[track][RIGHT]);


			// ********************************************* BUTTON LIGHT MANAGEMENT ****************************************
			

			if (playButtonPulse[track] == SLOW_PULSE) {
				if (runSetting == 1) {
					if (clockSample < midBeatMaxSample)
						lights[PLAY_BUT_LIGHT+track].setBrightness(1.f);
					else
						lights[PLAY_BUT_LIGHT+track].setBrightness(0.f);
				} else {
					playButtonPulseTime[track]--;
					if (playButtonPulseTime[track] < 0) {
						playButtonPulseTime[track] = slowPulseTime;
						lights[PLAY_BUT_LIGHT+track].setBrightness(!lights[PLAY_BUT_LIGHT+track].getBrightness());
					}

				}
			} else if (playButtonPulse[track] == FAST_PULSE) {
				playButtonPulseTime[track]--;
				if (playButtonPulseTime[track] < 0) {
					playButtonPulseTime[track] = fastPulseTime;
					lights[PLAY_BUT_LIGHT+track].setBrightness(!lights[PLAY_BUT_LIGHT+track].getBrightness());
				}					
			}

			if (recButtonPulse[track] == SLOW_PULSE) {
				if (runSetting == 1) {
					if (clockSample < midBeatMaxSample)
						lights[REC_BUT_LIGHT+track].setBrightness(1.f);
					else
						lights[REC_BUT_LIGHT+track].setBrightness(0.f);
				} else {
					recButtonPulseTime[track]--;
					if (recButtonPulseTime[track] < 0) {
						recButtonPulseTime[track] = slowPulseTime;
						lights[REC_BUT_LIGHT+track].setBrightness(!lights[REC_BUT_LIGHT+track].getBrightness());
					}

				}
			} else if (recButtonPulse[track] == FAST_PULSE) {
				recButtonPulseTime[track]--;
				if (recButtonPulseTime[track] < 0) {
					recButtonPulseTime[track] = fastPulseTime;
					lights[REC_BUT_LIGHT+track].setBrightness(!lights[REC_BUT_LIGHT+track].getBrightness());
				}					
			}

		}	// END FOR/NEXT TRACKS

/*		

																								░██████╗░██╗░░░░░░█████╗░██████╗░░█████╗░██╗░░░░░
																								██╔════╝░██║░░░░░██╔══██╗██╔══██╗██╔══██╗██║░░░░░
																								██║░░██╗░██║░░░░░██║░░██║██████╦╝███████║██║░░░░░
																								██║░░╚██╗██║░░░░░██║░░██║██╔══██╗██╔══██║██║░░░░░
																								╚██████╔╝███████╗╚█████╔╝██████╦╝██║░░██║███████╗
																								░╚═════╝░╚══════╝░╚════╝░╚═════╝░╚═╝░░╚═╝╚══════╝
*/

		// ****************************************************************************** GLOBAL STATUS

		if (busyTracks == 0) {
			globalStatus = IDLE;
			
			//if (!extConn) {
			if (!extConn && !internalClockAlwaysOn) {
				runSetting = 0;
				clockSample = 1.0;
				resetStart = true;
				beatCounter = 20;	// this hack makes the click be audible on next runSetting=1
			}
		}


		//debugDisplay = "recTracks " + to_string(recordedTracks); 
		//debugDisplay3 = "curr stat " + to_string(trackStatus[0]);
		//debugDisplay4 = "next stat " + to_string(nextStatus[0]);

		//debugDisplay2 = "busy " + to_string(busyTracks); 
		//debugDisplay3 = "curr solo " + to_string(currentSoloTrack);
		//debugDisplay4 = "next solo " + to_string(nextSoloTrack);

/*

																							░█████╗░██╗░░░██╗████████╗██████╗░██╗░░░██╗████████╗
																							██╔══██╗██║░░░██║╚══██╔══╝██╔══██╗██║░░░██║╚══██╔══╝
																							██║░░██║██║░░░██║░░░██║░░░██████╔╝██║░░░██║░░░██║░░░
																							██║░░██║██║░░░██║░░░██║░░░██╔═══╝░██║░░░██║░░░██║░░░
																							╚█████╔╝╚██████╔╝░░░██║░░░██║░░░░░╚██████╔╝░░░██║░░░
																							░╚════╝░░╚═════╝░░░░╚═╝░░░╚═╝░░░░░░╚═════╝░░░░╚═╝░░░
*/

		float tempSourcesLeft = inputValue[0][LEFT] + inputValue[1][LEFT] + inputValue[2][LEFT] + inputValue[3][LEFT] + inputValue[4][LEFT];
		float tempSourcesRight = inputValue[0][RIGHT] + inputValue[1][RIGHT] + inputValue[2][RIGHT] + inputValue[3][RIGHT] + inputValue[4][RIGHT];
		
		earOutput[LEFT] = clickOutput;
		earOutput[RIGHT] = clickOutput;

		if (!onlyClickOnEar) {
			earOutput[LEFT] += sumOutput[LEFT] + tempSourcesLeft;
			earOutput[RIGHT] += sumOutput[RIGHT] + tempSourcesRight;
		}

		if (srcToMaster) {
			sumOutput[LEFT] += tempSourcesLeft;
			sumOutput[RIGHT] += tempSourcesRight;
		}

		if (params[CLICKTOMASTER_SWITCH].getValue()) {
			sumOutput[LEFT] += clickOutput;
			sumOutput[RIGHT] += clickOutput;
		}

		sumOutput[LEFT] *= masterLevel;
		sumOutput[RIGHT] *= masterLevel;

		if (sumOutput[LEFT] > 10.f)
			sumOutput[LEFT] = 10.f;
		else if (sumOutput[LEFT] < -10.f)
			sumOutput[LEFT] = -10.f;

		if (sumOutput[RIGHT] > 10.f)
			sumOutput[RIGHT] = 10.f;
		else if (sumOutput[RIGHT] < -10.f)
			sumOutput[RIGHT] = -10.f;

		outputs[MASTER_LEFT_OUTPUT].setVoltage(sumOutput[LEFT]);
		outputs[MASTER_RIGHT_OUTPUT].setVoltage(sumOutput[RIGHT]);

		earOutput[LEFT] *= earLevel;
		earOutput[RIGHT] *= earLevel;

		if (earOutput[LEFT] > 10.f)
			earOutput[LEFT] = 10.f;
		else if (earOutput[LEFT] < -10.f)
			earOutput[LEFT] = -10.f;

		if (earOutput[RIGHT] > 10.f)
			earOutput[RIGHT] = 10.f;
		else if (earOutput[RIGHT] < -10.f)
			earOutput[RIGHT] = -10.f;

		outputs[EAR_LEFT_OUTPUT].setVoltage(earOutput[LEFT]);
		outputs[EAR_RIGHT_OUTPUT].setVoltage(earOutput[RIGHT]);

	}
	
};




//
//															███████╗███╗░░██╗██████╗░  ██████╗░██████╗░░█████╗░░█████╗░███████╗░██████╗░██████╗
//															██╔════╝████╗░██║██╔══██╗  ██╔══██╗██╔══██╗██╔══██╗██╔══██╗██╔════╝██╔════╝██╔════╝
//															█████╗░░██╔██╗██║██║░░██║  ██████╔╝██████╔╝██║░░██║██║░░╚═╝█████╗░░╚█████╗░╚█████╗░
//															██╔══╝░░██║╚████║██║░░██║  ██╔═══╝░██╔══██╗██║░░██║██║░░██╗██╔══╝░░░╚═══██╗░╚═══██╗
//															███████╗██║░╚███║██████╔╝  ██║░░░░░██║░░██║╚█████╔╝╚█████╔╝███████╗██████╔╝██████╔╝
//															╚══════╝╚═╝░░╚══╝╚═════╝░  ╚═╝░░░░░╚═╝░░╚═╝░╚════╝░░╚════╝░╚══════╝╚═════╝░╚═════╝░



struct SickoLooper5DisplaySrc1 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplaySrc1() {
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

struct SickoLooper5DisplayMeas1 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplayMeas1() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED));					
				if (module->trackLoopMeas[0] > 9)
					nvgTextBox(args.vg, 3, 17, 60, to_string(module->trackLoopMeas[0]).c_str(), NULL);
				else
					nvgTextBox(args.vg, 11, 17, 60, to_string(module->trackLoopMeas[0]).c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct SickoLooper5DisplayLoop1 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplayLoop1() {
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
				switch (module->trackStatus[0]) {
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
							if (module->samplePos[0] < module->totalSamples[0]) {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_GREEN));
								nvgLineTo(args.vg, 9, 76-(75*module->samplePos[0]/module->totalSamples[0]));
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
							if (module->samplePos[0] < module->totalSamples[0]) {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_YELLOW));
								nvgLineTo(args.vg, 9, 76-(75*module->samplePos[0]/module->totalSamples[0]));
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
		SickoLooper5 *module = dynamic_cast<SickoLooper5 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			const int track = 0;

			menu->addChild(createMenuLabel(("TRACK "+to_string(track+1)).c_str()));
			menu->addChild(createBoolPtrMenuItem("Fade IN on playback", "", &module->fadeInOnPlay[track]));
			menu->addChild(createBoolPtrMenuItem("Play Full Tail on Stop", "", &module->playFullTail[track]));
			
			menu->addChild(new MenuSeparator());
			menu->addChild(createBoolMenuItem("Extra samples Tail (1sec)", "", [=]() {
					return module->isExtraSamples(track);
				}, [=](bool xtraSamples) {
					module->setExtraSamples(track, xtraSamples);
			}));
			if (module->trackStatus[track] != EMPTY)
				menu->addChild(createMenuItem("Detect tempo and set bpm", "", [=]() {module->detectTempo(track);}));
			else
				menu->addChild(createMenuLabel("Detect tempo and set bpm"));

			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Import Wav", "", [=]() {module->menuLoadSample(track);}));
			if (module->trackStatus[track] != EMPTY)
				menu->addChild(createMenuItem("Export Wav", "", [=]() {module->menuSaveSample(track);}));
			else
				menu->addChild(createMenuLabel("Export Wav"));
		}
	}
};
// ------------------------------------------------------------------------------------------------------------------

struct SickoLooper5DisplaySrc2 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplaySrc2() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED)); 
				nvgTextBox(args.vg, 7, 17, 60, to_string(int(module->params[module->SOURCE_KNOB_PARAM+1].getValue())).c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}

};

struct SickoLooper5DisplayMeas2 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplayMeas2() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED)); 
				if (module->trackLoopMeas[1] > 9)
					nvgTextBox(args.vg, 3, 17, 60, to_string(module->trackLoopMeas[1]).c_str(), NULL);
				else
					nvgTextBox(args.vg, 11, 17, 60, to_string(module->trackLoopMeas[1]).c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct SickoLooper5DisplayLoop2 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplayLoop2() {
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
				switch (module->trackStatus[1]) {
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
							if (module->samplePos[1] < module->totalSamples[1]) {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_GREEN));
								nvgLineTo(args.vg, 9, 76-(75*module->samplePos[1]/module->totalSamples[1]));
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
							if (module->samplePos[1] < module->totalSamples[1]) {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_YELLOW));
								nvgLineTo(args.vg, 9, 76-(75*module->samplePos[1]/module->totalSamples[1]));
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
		SickoLooper5 *module = dynamic_cast<SickoLooper5 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			const int track = 1;

			menu->addChild(createMenuLabel(("TRACK "+to_string(track+1)).c_str()));
			menu->addChild(createBoolPtrMenuItem("Fade IN on playback", "", &module->fadeInOnPlay[track]));
			menu->addChild(createBoolPtrMenuItem("Play Full Tail on Stop", "", &module->playFullTail[track]));
			
			menu->addChild(new MenuSeparator());
			menu->addChild(createBoolMenuItem("Extra samples Tail (1sec)", "", [=]() {
					return module->isExtraSamples(track);
				}, [=](bool xtraSamples) {
					module->setExtraSamples(track, xtraSamples);
			}));
			if (module->trackStatus[track] != EMPTY)
				menu->addChild(createMenuItem("Detect tempo and set bpm", "", [=]() {module->detectTempo(track);}));
			else
				menu->addChild(createMenuLabel("Detect tempo and set bpm"));

			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Import Wav", "", [=]() {module->menuLoadSample(track);}));
			if (module->trackStatus[track] != EMPTY)
				menu->addChild(createMenuItem("Export Wav", "", [=]() {module->menuSaveSample(track);}));
			else
				menu->addChild(createMenuLabel("Export Wav"));
		}
	}
};
// ------------------------------------------------------------------------------------------------------------------

struct SickoLooper5DisplaySrc3 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplaySrc3() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED)); 
				nvgTextBox(args.vg, 7, 17, 60, to_string(int(module->params[module->SOURCE_KNOB_PARAM+2].getValue())).c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct SickoLooper5DisplayMeas3 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplayMeas3() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED)); 
				if (module->trackLoopMeas[2] > 9)
					nvgTextBox(args.vg, 3, 17, 60, to_string(module->trackLoopMeas[2]).c_str(), NULL);
				else
					nvgTextBox(args.vg, 11, 17, 60, to_string(module->trackLoopMeas[2]).c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct SickoLooper5DisplayLoop3 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplayLoop3() {
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
				switch (module->trackStatus[2]) {
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
							if (module->samplePos[2] < module->totalSamples[2]) {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_GREEN));
								nvgLineTo(args.vg, 9, 76-(75*module->samplePos[2]/module->totalSamples[2]));
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
							if (module->samplePos[2] < module->totalSamples[2]) {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_YELLOW));
								nvgLineTo(args.vg, 9, 76-(75*module->samplePos[2]/module->totalSamples[2]));
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
		SickoLooper5 *module = dynamic_cast<SickoLooper5 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			const int track = 2;

			menu->addChild(createMenuLabel(("TRACK "+to_string(track+1)).c_str()));
			menu->addChild(createBoolPtrMenuItem("Fade IN on playback", "", &module->fadeInOnPlay[track]));
			menu->addChild(createBoolPtrMenuItem("Play Full Tail on Stop", "", &module->playFullTail[track]));
			
			menu->addChild(new MenuSeparator());
			menu->addChild(createBoolMenuItem("Extra samples Tail (1sec)", "", [=]() {
					return module->isExtraSamples(track);
				}, [=](bool xtraSamples) {
					module->setExtraSamples(track, xtraSamples);
			}));
			if (module->trackStatus[track] != EMPTY)
				menu->addChild(createMenuItem("Detect tempo and set bpm", "", [=]() {module->detectTempo(track);}));
			else
				menu->addChild(createMenuLabel("Detect tempo and set bpm"));

			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Import Wav", "", [=]() {module->menuLoadSample(track);}));
			if (module->trackStatus[track] != EMPTY)
				menu->addChild(createMenuItem("Export Wav", "", [=]() {module->menuSaveSample(track);}));
			else
				menu->addChild(createMenuLabel("Export Wav"));
		}
	}
};
// ------------------------------------------------------------------------------------------------------------------

struct SickoLooper5DisplaySrc4 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplaySrc4() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED)); 
				nvgTextBox(args.vg, 7, 17, 60, to_string(int(module->params[module->SOURCE_KNOB_PARAM+3].getValue())).c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct SickoLooper5DisplayMeas4 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplayMeas4() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED)); 
				if (module->trackLoopMeas[3] > 9)
					nvgTextBox(args.vg, 3, 17, 60, to_string(module->trackLoopMeas[3]).c_str(), NULL);
				else
					nvgTextBox(args.vg, 11, 17, 60, to_string(module->trackLoopMeas[3]).c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct SickoLooper5DisplayLoop4 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplayLoop4() {
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
				switch (module->trackStatus[3]) {
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
							if (module->samplePos[3] < module->totalSamples[0]) {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_GREEN));
								nvgLineTo(args.vg, 9, 76-(75*module->samplePos[3]/module->totalSamples[3]));
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
							if (module->samplePos[3] < module->totalSamples[3]) {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_YELLOW));
								nvgLineTo(args.vg, 9, 76-(75*module->samplePos[3]/module->totalSamples[3]));
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
		SickoLooper5 *module = dynamic_cast<SickoLooper5 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			const int track = 3;

			menu->addChild(createMenuLabel(("TRACK "+to_string(track+1)).c_str()));
			menu->addChild(createBoolPtrMenuItem("Fade IN on playback", "", &module->fadeInOnPlay[track]));
			menu->addChild(createBoolPtrMenuItem("Play Full Tail on Stop", "", &module->playFullTail[track]));
						
			menu->addChild(new MenuSeparator());
			menu->addChild(createBoolMenuItem("Extra samples Tail (1sec)", "", [=]() {
					return module->isExtraSamples(track);
				}, [=](bool xtraSamples) {
					module->setExtraSamples(track, xtraSamples);
			}));
			if (module->trackStatus[track] != EMPTY)
				menu->addChild(createMenuItem("Detect tempo and set bpm", "", [=]() {module->detectTempo(track);}));
			else
				menu->addChild(createMenuLabel("Detect tempo and set bpm"));

			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Import Wav", "", [=]() {module->menuLoadSample(track);}));
			if (module->trackStatus[track] != EMPTY)
				menu->addChild(createMenuItem("Export Wav", "", [=]() {module->menuSaveSample(track);}));
			else
				menu->addChild(createMenuLabel("Export Wav"));
		}
	}
};
// ------------------------------------------------------------------------------------------------------------------

struct SickoLooper5DisplaySrc5 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplaySrc5() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED)); 
				nvgTextBox(args.vg, 7, 17, 60, to_string(int(module->params[module->SOURCE_KNOB_PARAM+4].getValue())).c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct SickoLooper5DisplayMeas5 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplayMeas5() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED)); 
				if (module->trackLoopMeas[4] > 9)
					nvgTextBox(args.vg, 3, 17, 60, to_string(module->trackLoopMeas[4]).c_str(), NULL);
				else
					nvgTextBox(args.vg, 11, 17, 60, to_string(module->trackLoopMeas[4]).c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct SickoLooper5DisplayLoop5 : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplayLoop5() {
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
				switch (module->trackStatus[4]) {
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
							if (module->samplePos[4] < module->totalSamples[4]) {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_GREEN));
								nvgLineTo(args.vg, 9, 76-(75*module->samplePos[4]/module->totalSamples[4]));
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
							if (module->samplePos[4] < module->totalSamples[4]) {
								nvgStrokeColor(args.vg, nvgRGBA(COLOR_YELLOW));
								nvgLineTo(args.vg, 9, 76-(75*module->samplePos[4]/module->totalSamples[4]));
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
		SickoLooper5 *module = dynamic_cast<SickoLooper5 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			const int track = 4;

			menu->addChild(createMenuLabel(("TRACK "+to_string(track+1)).c_str()));
			menu->addChild(createBoolPtrMenuItem("Fade IN on playback", "", &module->fadeInOnPlay[track]));
			menu->addChild(createBoolPtrMenuItem("Play Full Tail on Stop", "", &module->playFullTail[track]));
			
			menu->addChild(new MenuSeparator());
			menu->addChild(createBoolMenuItem("Extra samples Tail (1sec)", "", [=]() {
					return module->isExtraSamples(track);
				}, [=](bool xtraSamples) {
					module->setExtraSamples(track, xtraSamples);
			}));
			if (module->trackStatus[track] != EMPTY)
				menu->addChild(createMenuItem("Detect tempo and set bpm", "", [=]() {module->detectTempo(track);}));
			else
				menu->addChild(createMenuLabel("Detect tempo and set bpm"));

			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Import Wav", "", [=]() {module->menuLoadSample(track);}));
			if (module->trackStatus[track] != EMPTY)
				menu->addChild(createMenuItem("Export Wav", "", [=]() {module->menuSaveSample(track);}));
			else
				menu->addChild(createMenuLabel("Export Wav"));
		}
	}
};
// ------------------------------------------------------------------------------------------------------------------

struct SickoLooper5DisplayTempo : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplayTempo() {
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

struct SickoLooper5DisplayBeat : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DisplayBeat() {
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
					nvgTextBox(args.vg, 2, 17, 60, module->signatureDisplay[tempValue].c_str(), NULL);
				else
					nvgTextBox(args.vg, 10, 17, 60, module->signatureDisplay[tempValue].c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		SickoLooper5 *module = dynamic_cast<SickoLooper5 *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				SickoLooper5* module;
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
struct SickoLooper5DebugDisplay : TransparentWidget {
	SickoLooper5 *module;
	int frame = 0;
	SickoLooper5DebugDisplay() {
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

struct SickoLooper5Widget : ModuleWidget {
	SickoLooper5Widget(SickoLooper5 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SickoLooper5.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		const float xTrackShift = 44;

		{
			SickoLooper5DisplaySrc1 *display = new SickoLooper5DisplaySrc1();
			display->box.pos = mm2px(Vec(13.1, 31.2));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}

		{
			SickoLooper5DisplayMeas1 *display = new SickoLooper5DisplayMeas1();
			display->box.pos = mm2px(Vec(32.5, 31.2));
			display->box.size = mm2px(Vec(10, 8));
			display->module = module;
			addChild(display);
		}

		{
			SickoLooper5DisplayLoop1 *display = new SickoLooper5DisplayLoop1();
			display->box.pos = mm2px(Vec(23, 78));
			display->box.size = mm2px(Vec(6, 26));
			display->module = module;
			addChild(display);
		}
		
		// ----------------------------------------------------------------------------
		{
			SickoLooper5DisplaySrc2 *display = new SickoLooper5DisplaySrc2();
			display->box.pos = mm2px(Vec(13.1 + (xTrackShift), 31.2));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}

		{
			SickoLooper5DisplayMeas2 *display = new SickoLooper5DisplayMeas2();
			display->box.pos = mm2px(Vec(32.5 + (xTrackShift), 31.2));
			display->box.size = mm2px(Vec(10, 8));
			display->module = module;
			addChild(display);
		}
		
		{
			SickoLooper5DisplayLoop2 *display = new SickoLooper5DisplayLoop2();
			display->box.pos = mm2px(Vec(23 + (xTrackShift), 78));
			display->box.size = mm2px(Vec(6, 26));
			display->module = module;
			addChild(display);
		}
		
		// ----------------------------------------------------------------------------
		{
			SickoLooper5DisplaySrc3 *display = new SickoLooper5DisplaySrc3();
			display->box.pos = mm2px(Vec(13.1 + (xTrackShift*2), 31.2));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}

		{
			SickoLooper5DisplayMeas3 *display = new SickoLooper5DisplayMeas3();
			display->box.pos = mm2px(Vec(32.5 + (xTrackShift*2), 31.2));
			display->box.size = mm2px(Vec(10, 8));
			display->module = module;
			addChild(display);
		}
		
		{
			SickoLooper5DisplayLoop3 *display = new SickoLooper5DisplayLoop3();
			display->box.pos = mm2px(Vec(23 + (xTrackShift*2), 78));
			display->box.size = mm2px(Vec(6, 26));
			display->module = module;
			addChild(display);
		}
		
		// ----------------------------------------------------------------------------
		{
			SickoLooper5DisplaySrc4 *display = new SickoLooper5DisplaySrc4();
			display->box.pos = mm2px(Vec(13.1 + (xTrackShift*3), 31.2));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}

		{
			SickoLooper5DisplayMeas4 *display = new SickoLooper5DisplayMeas4();
			display->box.pos = mm2px(Vec(32.5 + (xTrackShift*3), 31.2));
			display->box.size = mm2px(Vec(10, 8));
			display->module = module;
			addChild(display);
		}
		
		{
			SickoLooper5DisplayLoop4 *display = new SickoLooper5DisplayLoop4();
			display->box.pos = mm2px(Vec(23 + (xTrackShift*3), 78));
			display->box.size = mm2px(Vec(6, 26));
			display->module = module;
			addChild(display);
		}
		
		// ----------------------------------------------------------------------------
		{
			SickoLooper5DisplaySrc5 *display = new SickoLooper5DisplaySrc5();
			display->box.pos = mm2px(Vec(13.1 + (xTrackShift*4), 31.2));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}

		{
			SickoLooper5DisplayMeas5 *display = new SickoLooper5DisplayMeas5();
			display->box.pos = mm2px(Vec(32.5 + (xTrackShift*4), 31.2));
			display->box.size = mm2px(Vec(10, 8));
			display->module = module;
			addChild(display);
		}
		
		{
			SickoLooper5DisplayLoop5 *display = new SickoLooper5DisplayLoop5();
			display->box.pos = mm2px(Vec(23 + (xTrackShift*4), 78));
			display->box.size = mm2px(Vec(6, 26));
			display->module = module;
			addChild(display);
		}

		// ----------------------------------------------------------------------------
		{
			SickoLooper5DisplayTempo *display = new SickoLooper5DisplayTempo();
			display->box.pos = mm2px(Vec(242.4, 28));
			display->box.size = mm2px(Vec(13.6, 7.9));
			display->module = module;
			addChild(display);
		}

		{
			SickoLooper5DisplayBeat *display = new SickoLooper5DisplayBeat();
			display->box.pos = mm2px(Vec(242.4, 37.5));
			display->box.size = mm2px(Vec(13.6, 7.9));
			display->module = module;
			addChild(display);
		}

		/*
		{
			SickoLooper5DebugDisplay *display = new SickoLooper5DebugDisplay();
			display->box.pos = Vec(23, 3);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/

		const float xInL = 7;
		const float xInR = 17;
		const float xInLvl = 28;
		const float xMute = 38.5;
		const float yIn = 18;

		const float xSrc = 7;
		const float ySrc = 35;

		const float xMeasKnob = 27.5;
		const float yMeasKnob = 35;

		const float xStartImm = 9;
		const float xStopImm = 23;
		const float xSyncBut = 36.4;
		const float ySyncBut = 46.9;
		const float yStartImm = 46.9;

		const float xRevBut = 26.5;
		const float yRevBut = 58.5;

		const float xSoloBut = 37;
		const float ySoloBut = 58.5;

		const float xOneShotBut = 26.5;
		const float yOneShotBut = 72.5;

		const float xPlayTrig = 7;
		const float xPlayBut = 17;
		const float yPlay = 58.5;
		const float yRec = 72.5;
		const float yStop = 86.5;
		const float yErase = 100;


		const float xXFade = 36.7;
		const float yXFade = 72;

		const float xPan = 36.7;
		const float yPan = 85;

		const float xVol = 36.7;
		const float yVol = 99;


		const float xEol = 8;
		const float xSrcToTrack = 17.3;
		const float xOutL = 27.3;
		const float xOutR = 37.1;

		const float yOutput = 117.f;

		// ******************************
		
		const float xBpm = 236.6;
		const float yBpm = 31.6;
		const float xBeat = 236.7;
		const float yBeat = 41.7;

		const float xExtClock = 228.5;
		const float yExtClock = 18;
		const float xClockRst = 239;
		const float yClockRst = 18;
		const float xClockOut = 252;
		const float yClockOut = 18;

		const float xClick = 228;
		const float yClick = 57.5;
		const float xClickVol = 239.7;
		const float yClickVol = 57.7;
		const float xClickMst = 251;

		const float xPrerollBut = 239.2;
		const float xPrerollSwitch = 250;
		const float yPreroll = 66;

		const float yAll = 82.6;
		const float xAllStartTrig = 228.4;
		const float xAllStartBut = 238.4;
		const float xAllStop = 251.4;

		const float xEarVol = 228.6;
		const float yEarVol = 99.4;
		const float xEarL = 241.4;
		const float xEarR = 251.4;
		const float yEar = 99.5;
		const float xMastVol = 228.7;
		const float yMastVol = 116;
		const float xMastL = 241.4;
		const float xMastR = 251.4;
		const float yMast = 117;

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xBpm, yBpm)), module, SickoLooper5::BPM_KNOB_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xBeat, yBeat)), module, SickoLooper5::SIGNATURE_KNOB_PARAM));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xExtClock, yExtClock)), module, SickoLooper5::EXTCLOCK_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(xClockRst, yClockRst)), module, SickoLooper5::CLOCK_RST_SWITCH, SickoLooper5::CLOCK_RST_LIGHT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xClockOut, yClockOut)), module, SickoLooper5::CLOCK_OUTPUT));

		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xClick, yClick)), module, SickoLooper5::CLICK_BUT_PARAM, SickoLooper5::CLICK_BUT_LIGHT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xClickVol, yClickVol)), module, SickoLooper5::CLICKVOL_KNOB_PARAM));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xClickMst, yClickVol)), module, SickoLooper5::CLICKTOMASTER_SWITCH, SickoLooper5::CLICKTOMASTER_LIGHT));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xPrerollBut, yPreroll)), module, SickoLooper5::PREROLL_BUT_PARAM, SickoLooper5::PREROLL_BUT_LIGHT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(xPrerollSwitch, yPreroll)), module, SickoLooper5::PREROLL_SWITCH));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xAllStartTrig, yAll)), module, SickoLooper5::ALLSTARTSTOP_TRIG_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xAllStartBut, yAll)), module, SickoLooper5::ALLSTARTSTOP_BUT_PARAM, SickoLooper5::ALLSTARTSTOP_BUT_LIGHT));

		//addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xAllStop, yAll)), module, SickoLooper5::UNDOREDO_BUT_PARAM, SickoLooper5::UNDOREDO_BUT_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xAllStop, yAll)), module, SickoLooper5::ALLSTOP_TRIG_INPUT));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xEarVol, yEarVol)), module, SickoLooper5::EARVOL_KNOB_PARAM));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xEarL, yEar)), module, SickoLooper5::EAR_LEFT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xEarR, yEar)), module, SickoLooper5::EAR_RIGHT_OUTPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xMastVol, yMastVol)), module, SickoLooper5::MASTERVOL_KNOB_PARAM));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xMastL, yMast)), module, SickoLooper5::MASTER_LEFT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xMastR, yMast)), module, SickoLooper5::MASTER_RIGHT_OUTPUT));

		for (int i = 0; i < MAX_TRACKS; i++) {

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInL+(xTrackShift*i), yIn)), module, SickoLooper5::LEFT_INPUT+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInR+(xTrackShift*i), yIn)), module, SickoLooper5::RIGHT_INPUT+i));
			addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xInLvl+(xTrackShift*i), yIn)), module, SickoLooper5::SOURCELVL_KNOB_PARAM+i));
			addParam(createLightParamCentered<VCVLightLatch<LargeSimpleLight<RedLight>>>(mm2px(Vec(xMute+(xTrackShift*i), yIn)), module, SickoLooper5::MUTE_SWITCH+i, SickoLooper5::MUTE_LIGHT+i));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xSrc+(xTrackShift*i), ySrc)), module, SickoLooper5::SOURCE_KNOB_PARAM+i));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xMeasKnob+(xTrackShift*i), yMeasKnob)), module, SickoLooper5::MEAS_KNOB_PARAM+i));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xPlayTrig+(xTrackShift*i), yPlay)), module, SickoLooper5::PLAY_TRIG_INPUT+i));
			addParam(createLightParamCentered<VCVLightBezel<GreenLight>>(mm2px(Vec(xPlayBut+(xTrackShift*i), yPlay)), module, SickoLooper5::PLAY_BUT_PARAM+i, SickoLooper5::PLAY_BUT_LIGHT+i));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xPlayTrig+(xTrackShift*i), yRec)), module, SickoLooper5::REC_TRIG_INPUT+i));
			addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xPlayBut+(xTrackShift*i), yRec)), module, SickoLooper5::REC_BUT_PARAM+i, SickoLooper5::REC_BUT_LIGHT+i));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xPlayTrig+(xTrackShift*i), yStop)), module, SickoLooper5::STOP_TRIG_INPUT+i));
			addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xPlayBut+(xTrackShift*i), yStop)), module, SickoLooper5::STOP_BUT_PARAM+i, SickoLooper5::STOP_BUT_LIGHT+i));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xPlayTrig+(xTrackShift*i), yErase)), module, SickoLooper5::ERASE_TRIG_INPUT+i));
			addParam(createLightParamCentered<VCVLightBezel<YellowLight>>(mm2px(Vec(xPlayBut+(xTrackShift*i), yErase)), module, SickoLooper5::ERASE_BUT_PARAM+i, SickoLooper5::ERASE_BUT_LIGHT+i));

			addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xStartImm+(xTrackShift*i), yStartImm)), module, SickoLooper5::STARTIMM_SWITCH+i, SickoLooper5::STARTIMM_LIGHT+i));
			addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(xStopImm+(xTrackShift*i), yStartImm)), module, SickoLooper5::STOPIMM_SWITCH+i, SickoLooper5::STOPIMM_LIGHT+i));
			addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xSyncBut+(xTrackShift*i), ySyncBut)), module, SickoLooper5::LOOPSYNC_SWITCH+i, SickoLooper5::LOOPSYNC_LIGHT+i));

			addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xOneShotBut+(xTrackShift*i), yOneShotBut)), module, SickoLooper5::ONESHOT_SWITCH+i, SickoLooper5::ONESHOT_LIGHT+i));
			addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xRevBut+(xTrackShift*i), yRevBut)), module, SickoLooper5::REV_SWITCH+i, SickoLooper5::REV_LIGHT+i));

			addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xSoloBut+(xTrackShift*i), ySoloBut)), module, SickoLooper5::SOLO_SWITCH+i, SickoLooper5::SOLO_LIGHT+i));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xXFade+(xTrackShift*i), yXFade)), module, SickoLooper5::XFADE_KNOB_PARAM+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPan+(xTrackShift*i), yPan)), module, SickoLooper5::PAN_KNOB_PARAM+i));
			addParam(createParamCentered<SickoKnob>(mm2px(Vec(xVol+(xTrackShift*i), yVol)), module, SickoLooper5::VOLTRACK_KNOB_PARAM+i));

			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xEol+(xTrackShift*i), yOutput)), module, SickoLooper5::EOL_OUTPUT+i));
			addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xSrcToTrack+(xTrackShift*i), yOutput)), module, SickoLooper5::SRC_TO_TRACK_SWITCH+i, SickoLooper5::SRC_TO_TRACK_LIGHT+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOutL+(xTrackShift*i), yOutput)), module, SickoLooper5::TRACK_LEFT_OUTPUT+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOutR+(xTrackShift*i), yOutput)), module, SickoLooper5::TRACK_RIGHT_OUTPUT+i));
		}
	}


	void appendContextMenu(Menu *menu) override {
	   	SickoLooper5 *module = dynamic_cast<SickoLooper5*>(this->module);
			assert(module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("SOURCEs to MASTER out", "", &module->srcToMaster));
		menu->addChild(createBoolPtrMenuItem("Only Click on EAR", "", &module->onlyClickOnEar));
		menu->addChild(createBoolPtrMenuItem("EOL pulse on stop", "", &module->eolPulseOnStop));
		
		menu->addChild(new MenuSeparator());

		struct ModeItem : MenuItem {
			SickoLooper5* module;
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

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Tracks settings", "", [=](Menu * menu) {
			for (int track = 0; track < MAX_TRACKS; track++) {
				menu->addChild(createSubmenuItem(("Track "+to_string(track+1)).c_str(), "", [=](Menu * menu) {
					menu->addChild(createMenuLabel(("TRACK "+to_string(track+1)).c_str()));
					
					menu->addChild(createBoolPtrMenuItem("Fade IN on playback", "", &module->fadeInOnPlay[track]));
					menu->addChild(createBoolPtrMenuItem("Play Full Tail on Stop", "", &module->playFullTail[track]));
					
					menu->addChild(new MenuSeparator());
					menu->addChild(createBoolMenuItem("Extra samples Tail (1sec)", "", [=]() {
							return module->isExtraSamples(track);
						}, [=](bool xtraSamples) {
							module->setExtraSamples(track, xtraSamples);
					}));

					if (module->trackStatus[track] != EMPTY)
						menu->addChild(createMenuItem("Detect tempo and set bpm", "", [=]() {module->detectTempo(track);}));
					else
						menu->addChild(createMenuLabel("Detect tempo and set bpm"));

					menu->addChild(new MenuSeparator());
					menu->addChild(createMenuItem("Import Wav", "", [=]() {module->menuLoadSample(track);}));
					if (module->trackStatus[track] != EMPTY)
						menu->addChild(createMenuItem("Export Wav", "", [=]() {module->menuSaveSample(track);}));
					else
						menu->addChild(createMenuLabel("Export Wav"));
				}));
			}
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Load preset (+loops)", "", [=]() {module->menuLoadPreset();}));
		menu->addChild(createMenuItem("Save preset", "", [=]() {module->menuSavePreset(false);}));
		menu->addChild(createMenuItem("Save preset + loops", "", [=]() {module->menuSavePreset(true);}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolMenuItem("Internal Clock Always ON", "", [=]() {
				return module->isInternalClockAlwaysOn();
			}, [=](bool internalClockAlwaysOn) {
				module->setInternalClock(internalClockAlwaysOn);
		}));
		/*
		menu->addChild(createSubmenuItem("Click Settings", "", [=](Menu * menu) {
			menu->addChild(createSubmenuItem("Click Presets", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("Standard", "", [=]() {module->setClick(0);}));
				menu->addChild(createMenuItem("Click1", "", [=]() {module->setClick(1);}));
				menu->addChild(createMenuItem("Click2", "", [=]() {module->setClick(2);}));
			}));

			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuItem("Load BEAT click", "", [=]() {module->clickMenuLoadSample(0);}));
			menu->addChild(createMenuItem("File: " + module->clickFileDescription[0], "", [=]() {module->clickMenuLoadSample(0);}));
			menu->addChild(createMenuItem("", "Clear", [=]() {module->clickClearSlot(0);}));
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Load BAR click", "", [=]() {module->clickMenuLoadSample(1);}));
			menu->addChild(createMenuItem("File: " + module->clickFileDescription[1], "", [=]() {module->clickMenuLoadSample(1);}));
			menu->addChild(createMenuItem("", "Clear", [=]() {module->clickClearSlot(1);}));
		}));	
		*/
		struct ClickItem : MenuItem {
			SickoLooper5* module;
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

			menu->addChild(createMenuItem("Custom BEAT click", "", [=]() {module->clickMenuLoadSample(0);}));
			menu->addChild(createMenuItem("File: " + module->clickFileDescription[0], "", [=]() {module->clickMenuLoadSample(0);}));
			menu->addChild(createMenuItem("", "Clear", [=]() {module->clickClearSlot(0);}));
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Custom BAR click", "", [=]() {module->clickMenuLoadSample(1);}));
			menu->addChild(createMenuItem("File: " + module->clickFileDescription[1], "", [=]() {module->clickMenuLoadSample(1);}));
			menu->addChild(createMenuItem("", "Clear", [=]() {module->clickClearSlot(1);}));
		}));

	}
};

Model *modelSickoLooper5 = createModel<SickoLooper5, SickoLooper5Widget>("SickoLooper5");
