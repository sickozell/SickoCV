#define COLOR_LCD_RED 0xff, 0x44, 0x44, 0xff
#define COLOR_LCD_GREEN 0x33, 0xff, 0x33, 0xff
#define COLOR_LCD_BLUE 0xca, 0xca, 0xff, 0xff
#define COLOR_LCD_BLACK 0x00, 0x00, 0x00, 0xff

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

#define LOKEY 0
#define HIKEY 1
#define REFKEY 2

#define recOutChan 0
#define sampleCoeff 2
#define xWav 5
#define wavDisp 138
#define yLineUp 13
#define yLineDn 41
#define yZeroLine 27
#define yInfo 10.8
#define yInfo2 50
#define xSlot 6
#define xMagg 12
#define xRoute 18
#define xTitle 27
#define maxTitleChars 18
#define xTime 118
#define xCh 6
#define xToSave 24
#define xLoVal 33
#define xHiVal 70
#define xRfVal 107
#define xKeyL 32
#define xKeyH 69
#define xKeyR 106
#define yKey 41.7
#define keyW 36
#define keyH 9

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

struct KeySampler : Module {

	#include "notes.hpp"
	#include "shapes.hpp"

	enum ParamIds {
		GAIN_PARAM,
		REC_PARAM,
		MONITOR_SWITCH,
		SLOT_SELECTOR_PARAM,
		RANGE_PARAM,
		RANGE_SELECTOR_PARAM,
		OUT_SELECTOR_PARAM,
		CUESTART_PARAM,
		CUEEND_PARAM,
		LOOPSTART_PARAM,
		LOOPEND_PARAM,
		REV_PARAM,
		XFADE_PARAM,
		LOOP_PARAM,
		PINGPONG_PARAM,
		TRIGGATEMODE_SWITCH,
		TRIGMODE_SWITCH,
		PB_SWITCH,
		PHASESCAN_SWITCH,
		ATTACK_PARAM,
		DECAY_PARAM,
		SUSTAIN_PARAM,
		RELEASE_PARAM,
		CURVE_PARAM,
		STRETCH_PARAM,
		STR_SIZE_PARAM,
		VOL_PARAM,
		PREVSAMPLE_PARAM,
		NEXTSAMPLE_PARAM,
		NUM_PARAMS 
	};
	enum InputIds {
		ENUMS(IN_INPUT, 2),
		GATE_INPUT,
		VO_INPUT,
		VEL_INPUT,
		PB_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(LEFT_OUTPUT, 8),
		ENUMS(RIGHT_OUTPUT, 8),
		NUM_OUTPUTS
	};
	enum LightIds {
		REC_LIGHT,
		MONITOR_LIGHT,
		RANGE_SELECTOR_LIGHT,
		LOOP_LIGHT,
		REV_LIGHT,
		PINGPONG_LIGHT,
		TRIGMODE_LIGHT,
		PB_LIGHT,
		PHASESCAN_LIGHT,
		NUM_LIGHTS
	};

	int currSlot = 0;
	int prevSlot = -1;
  
	unsigned int fileChannels[8];
	unsigned int channels[8];
	unsigned int vcvSampleRate = APP->engine->getSampleRate();
	unsigned int sampleRate = vcvSampleRate * 2;
	drwav_uint64 totalSampleC[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	drwav_uint64 totalSamples[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	const unsigned int minSamplesToLoad = 124;

	vector<float> playBuffer[8][2][2];
	
	//metamodule change
	//vector<float> tempBuffer[2];

	vector<double> displayBuff[8];
	int currentDisplay = 0;
	int displaySlot = 0;
	float voctDisplay = 100.f;
	bool velConnected = false;

	float velKey = 0;
	float vOctKey = 0;
	int vOctNote = 0;
	int vOctDiff = 0;
	float vOctDiffFloat = 0;

	bool fileLoaded[8] = {false, false, false, false, false, false, false, false};

	bool play[8][16] = {{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
						{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
						{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
						{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
						{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
						{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
						{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
						{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
					};

	double samplePos[8][16] = {{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
							};
	double distancePos[8][16] = {{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
							};

	double cueStartPos[8];
	double cueEndPos[8];
	double loopStartPos[8];
	double loopEndPos[8];

	float knobCueStartPos[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float knobCueEndPos[8] = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};
	float prevKnobCueStartPos[8] = {-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f};
	float prevKnobCueEndPos[8] = {2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f};

	float knobLoopStartPos[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float knobLoopEndPos[8] = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};
	float prevKnobLoopStartPos[8] = {-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f};
	float prevKnobLoopEndPos[8] = {2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f};

	float voct[8][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
						};
	float prevVoctDiffFloat[8][16] = {{11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f},
							{11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f},
							{11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f},
							{11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f},
							{11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f},
							{11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f},
							{11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f},
							{11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f}
						};


	float speedVoct[8][16];

	bool searchingCueStartPhase[8] = {false, false, false, false, false, false, false, false};
	bool searchingCueEndPhase[8] = {false, false, false, false, false, false, false, false};
	double scanCueStartSample[8];
	double scanCueEndSample[8];

	bool searchingLoopStartPhase[8] = {false, false, false, false, false, false, false, false};
	bool searchingLoopEndPhase[8] = {false, false, false, false, false, false, false, false};
	double scanLoopStartSample[8];
	double scanLoopEndSample[8];

	double currSampleWeight[8][16] = {{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
							};

	std::string storedPath[8] = {"", "", "", "", "", "", "", ""};
	std::string fileDescription[8] = {"--none--", "--none--", "--none--", "--none--", "--none--", "--none--", "--none--", "--none--"};
	std::string fileDisplay[8] = {"", "", "", "", "", "", "", ""};
	std::string channelsDisplay[8] = {"", "", "", "", "", "", "", ""};
	std::string timeDisplay[8] = {"", "", "", "", "", "", "", ""};
	std::string recTimeDisplay[8] = {"", "", "", "", "", "", "", ""};
	std::string samplerateDisplay[8] = {"", "", "", "", "", "", "", ""};

	std::string userFolder = "";
	std::string currentFolder = "";
	vector <std::string> currentFolderV;
	int currentFile = 0;

	bool rootFound = false;
	bool fileFound[8] = {false, false, false, false, false, false, false, false};

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

	float trigValue[8][16] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
							};

	float prevTrigValue[8][16] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
							};
	

	bool trimOnSave = false;
	int antiAlias = 1;
	bool quantize = false;
	
	bool prevPhaseScan[8] = {false, false, false, false, false, false, false, false};

	bool firstLoad[8] = {true, true, true, true, true, true, true, true};
	bool resetCursorsOnLoad = true;
	bool disableNav = false;
	bool saveOversampled = false;
	bool autoMonOff = true;

	bool sampleInPatch = true;

	bool loadFromPatch[8] = {false, false, false, false, false, false, false, false};
	bool restoreLoadFromPatch[8] = {false, false, false, false, false, false, false, false};
	
	float fadeCoeff[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	int fadingType[8][16] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
							};
	float fadingValue[8][16] = {{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f}
								};

	double fadedPosition[8][16] = {{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
									{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
									{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
									{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
									{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
									{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
									{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
									{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
									};
	double fadeSamples[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	
	float prevXfade[8] = {-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f};

	float currentOutput = 0.f;
	float currentOutputR = 0.f;
	float sumOutput[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float sumOutputR[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

	/*
	std::string debugDisplay[16] = {"X", "X", "X", "X", "X", "X", "X", "X", "X", "X", "X", "X", "X", "X", "X", "X"};
	std::string debugDisplay2 = "X";
	std::string debugDisplay3 = "X";
	std::string debugDisplay4 = "X";
	int debugInt = 0;
	bool debugBool = false;
	*/

	double a0, a1, a2, b1, b2, z1, z2, z1r, z2r;

	float attackValue[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float decayValue[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float sustainValue[8] = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};
	float releaseValue[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

	bool loop[8] = {false, false, false, false, false, false, false, false};
	float xFade[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	bool pingpong[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	bool reverseStart[8] = {false, false, false, false, false, false, false, false};
	int trigMode[8] = {0, 0, 0, 0, 0, 0, 0 ,0};
	int trigType[8] = {0, 0, 0, 0, 0, 0, 0 ,0};
	bool allowPB[8] = {false, false, false, false, false, false, false, false};
	bool phaseScan[8] = {true, true, true, true, true, true, true, true};
	float attackKnob[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float decayKnob[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float releaseKnob[8] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float shape[8] = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
	float stretchKnob[8] = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};
	float cycleKnob[8] = {48.f, 48.f, 48.f, 48.f, 48.f, 48.f, 48.f, 48.f};
	float volumeValue[8] = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};

	int refKey[8][3] =  {{0, 127, 72},
						{0, 127, 72},
						{0, 127, 72},
						{0, 127, 72},
						{0, 127, 72},
						{0, 127, 72},
						{0, 127, 72},
						{0, 127, 72}};

	int selectedRange = 0;
	int outRoute[8] = {0, 1, 2, 3, 4, 5, 6, 7};

	float pitchBend = 0.f;
	float pbValue = 0.f;
	const float pbRangeTable[15] = {60.f, 30.f, 20.f, 15.f, 12.f, 10.f, 8.571428571428571f, 7.5f, 6.666666666666667f, 6.f, 5.454545454545454f, 5.f, 2.5f, 1.666666666666667f, 1.25f};
	
	int pbRange = 1;

	bool pbConnected = false;

	float prevAttackKnob[8] = {-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f};
	float prevDecayKnob[8] = {-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f};
	float prevReleaseKnob[8] = {-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f};

	float trigRange = 0;
	float prevTrigRange = 0;

	int stage[8][16] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
						{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
						{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
						{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
						{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
						{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
						{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
						{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
					};
	float stageLevel[8][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
								};

	float env[8][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
						};

	float refValue[8][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
						{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
						};

	float deltaValue[8][16] = {{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f}
								};

	float slopeCorr[8][16] = {{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
								{1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f}
								};
	
	/* // // RELEASE NEW DEVE SERVIRE SOLO PER GLI EOC ????? SI PUO' TOGLIERE
	bool releaseNew[8][16] = {{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
							{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
							{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
							{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
							{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
							{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
							{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
							{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}
						};	
	*/

	float oneMsSamples  = (APP->engine->getSampleRate()) / 1000;	// number of samples in 1ms
	float envSrCoeff = 1 / APP->engine->getSampleRate();

	int chan;

	
	float stageCoeff[8][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
							};
	float masterLevel[8][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
							};

	
	int reversePlaying[8][16] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
							};
	

	int recSlot = 0;
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

	int fileSampleRate[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	bool resampled[8] = {false, false, false, false, false, false, false, false};
	bool toSave[8] = {false, false, false, false, false, false, false, false};
	std::string infoToSave[8] = {"", "", "", "", "", "", "", ""};

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

	int grainCount[8][16] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
							{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
							{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
							{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
							{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
							{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
							{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
							{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
						};
	double grainSampleCount[8][16] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
										{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
										{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
										{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
										{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
										{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
										{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
										{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
									};
	double stretchMaxPos[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	double grainPos[8][16] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
							};
	float grainFadeValue[8][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
									{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
									{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
									{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
									{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
									{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
									{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
									{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
								};
	float grainFadeCoeff[8][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
							};
	bool grainFade[8][16] = {{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}
							};

	bool learn = false;

	bool waitingLearnEnter = false;
	int waitingLearnEnterTime = 0;
	float learnSamples = APP->engine->getSampleRate();

	bool storedLearn = false;
	int storedLearnTime = 0;
	int maxStoredLearnTime = APP->engine->getSampleRate() * 0.75;

	float keyToStore;
	float learnKey;
	float prevLearnKey = -11.f;

	bool learningButLight = false;
	float learnButLightDelta = 2 / APP->engine->getSampleRate();
	float learnButLightValue = 0.f;
	int learnPulseTime = 0;
	bool learnExitButStatus = false;
	int lightPulseTime = (APP->engine->getSampleRate()) / 10;

	KeySampler() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configSwitch(PREVSAMPLE_PARAM, 0.f, 1.f, 0.f, "Previous Sample");
		configSwitch(NEXTSAMPLE_PARAM, 0.f, 1.f, 0.f, "Next Sample");

		//******************************************************************************

		configInput(IN_INPUT,"Left");
		configInput(IN_INPUT+1,"Right");
		configSwitch(REC_PARAM, 0.f, 1.f, 0.f, "REC", {"OFF", "ON"});
		configParam(GAIN_PARAM, 0.f, 2.0f, 1.0f, "REC Gain", "%", 0, 100);
		
		configSwitch(MONITOR_SWITCH, 0.f, 1.f, 0.f, "Monitor", {"Off", "On"});

		//******************************************************************************

		configParam(SLOT_SELECTOR_PARAM, 1.f, 8.f, 1.f, "Slot");
		paramQuantities[SLOT_SELECTOR_PARAM]->snapEnabled = true;

		configParam(RANGE_PARAM, 0.f, 127.f, 72.f, "Midi");
		paramQuantities[RANGE_PARAM]->snapEnabled = true;

		configSwitch(RANGE_SELECTOR_PARAM, 0.f, 1.f, 0.f, "Key Select");

		configParam(OUT_SELECTOR_PARAM, 1.f, 8.f, 1.f, "Out");
		paramQuantities[OUT_SELECTOR_PARAM]->snapEnabled = true;

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

		configSwitch(TRIGGATEMODE_SWITCH, 0.f, 1.f, 0.f, "Mode", {"Gate", "Trig"});
		configSwitch(TRIGMODE_SWITCH, 0.f, 1.f, 0.f, "Trig Type", {"Start/Stop", "Restart"});

		configSwitch(PB_SWITCH, 0.f, 1.f, 0.f, "Allow PB", {"Off", "On"});

		configSwitch(PHASESCAN_SWITCH, 0.f, 1.f, 1.f, "Phase Scan", {"Off", "On"});

		//******************************************************************************

		configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Attack", "ms", maxStageTime / minStageTime, minStageTime);
		configParam(DECAY_PARAM, 0.f, 1.f, 0.f, "Decay", "ms", maxStageTime / minStageTime, minStageTime);
		configParam(SUSTAIN_PARAM, 0.f, 1.0f, 1.0f, "Sustain","%", 0, 100);
		configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "Release", "ms", maxStageTime / minStageTime, minStageTime);

		configParam(CURVE_PARAM, 0.f, 1.0f, 0.5f, "Shape","", 0, 100);

		//******************************************************************************

		configParam(STRETCH_PARAM, 0.01f, 9.99f, 1.f, "Time Stretch", "%", 0, 100);
		configParam(STR_SIZE_PARAM, 1.f, 99.f, 48.f, "Cycle Size", "ms");

		configParam(VOL_PARAM, 0.f, 1.0f, 1.0f, "Sample Volume", "%", 0, 100);

		//******************************************************************************

		configInput(GATE_INPUT,"Gate/Trig");
		configInput(VO_INPUT,"V/Oct");
		configInput(VEL_INPUT,"Velocity");
		configInput(PB_INPUT,"Pitch Bend");

		//******************************************************************************

		configOutput(LEFT_OUTPUT,"Left #1");
		configOutput(RIGHT_OUTPUT,"Right #1");
		configOutput(LEFT_OUTPUT+1,"Left #2");
		configOutput(RIGHT_OUTPUT+1,"Right #2");
		configOutput(LEFT_OUTPUT+2,"Left #3");
		configOutput(RIGHT_OUTPUT+2,"Right #3");
		configOutput(LEFT_OUTPUT+3,"Left #4");
		configOutput(RIGHT_OUTPUT+3,"Right #4");
		configOutput(LEFT_OUTPUT+4,"Left #5");
		configOutput(RIGHT_OUTPUT+4,"Right #5");
		configOutput(LEFT_OUTPUT+5,"Left #6");
		configOutput(RIGHT_OUTPUT+5,"Right #6");
		configOutput(LEFT_OUTPUT+6,"Left #7");
		configOutput(RIGHT_OUTPUT+6,"Right #7");
		configOutput(LEFT_OUTPUT+7,"Left #8");
		configOutput(RIGHT_OUTPUT+7,"Right #8");

		//configOutput(EOC_OUTPUT,"End of Cycle");
		//configOutput(EOR_OUTPUT,"End of Release");

		//******************************************************************************
		
		playBuffer[0][0][0].resize(0);
		playBuffer[0][0][1].resize(0);
		playBuffer[0][1][0].resize(0);
		playBuffer[0][1][1].resize(0);

		playBuffer[1][0][0].resize(0);
		playBuffer[1][0][1].resize(0);
		playBuffer[1][1][0].resize(0);
		playBuffer[1][1][1].resize(0);

		playBuffer[2][0][0].resize(0);
		playBuffer[2][0][1].resize(0);
		playBuffer[2][1][0].resize(0);
		playBuffer[2][1][1].resize(0);

		playBuffer[3][0][0].resize(0);
		playBuffer[3][0][1].resize(0);
		playBuffer[3][1][0].resize(0);
		playBuffer[3][1][1].resize(0);

		playBuffer[4][0][0].resize(0);
		playBuffer[4][0][1].resize(0);
		playBuffer[4][1][0].resize(0);
		playBuffer[4][1][1].resize(0);

		playBuffer[5][0][0].resize(0);
		playBuffer[5][0][1].resize(0);
		playBuffer[5][1][0].resize(0);
		playBuffer[5][1][1].resize(0);

		playBuffer[6][0][0].resize(0);
		playBuffer[6][0][1].resize(0);
		playBuffer[6][1][0].resize(0);
		playBuffer[6][1][1].resize(0);

		playBuffer[7][0][0].resize(0);
		playBuffer[7][0][1].resize(0);
		playBuffer[7][1][0].resize(0);
		playBuffer[7][1][1].resize(0);

		// metamodule change
		//tempBuffer[0].resize(0);
		//tempBuffer[1].resize(0);

	}

	static float convertCVToSeconds(float cv) {		
		return minStageTime * std::pow(maxStageTime / minStageTime, cv) / 1000;
	}

	void onReset(const ResetEvent &e) override {

		for (int s = 0; s < 8; s++) {

			for (int c = 0; c < 16; c++) {	
				play[s][c] = false;
				fadingType[s][c] = NO_FADE;
				stage[s][c] = STOP_STAGE;	
				stageLevel[s][c] = 0;
				env[s][c] = 0;
				voct[s][c] = 0.f;
				prevVoctDiffFloat[s][c] = 11.f;
				reversePlaying[s][c] = FORWARD;
			}

			clearSlot(s);
			
			firstLoad[s] = true;
			totalSampleC[s] = 0;
			totalSamples[s] = 0;

			prevPhaseScan[s] = false;
			prevXfade[s] = -1.f;

			knobCueStartPos[s] = 0.f;
			knobCueEndPos[s] = 1.f;
			prevKnobCueStartPos[s] = -1.f;
			prevKnobCueEndPos[s] = 2.f;

			knobLoopStartPos[s] = 0.f;
			knobLoopEndPos[s] = 1.f;
			prevKnobLoopStartPos[s] = -1.f;
			prevKnobLoopEndPos[s] = 2.f;

			prevAttackKnob[s] = -1;
			prevDecayKnob[s] = -1;
			prevReleaseKnob[s] = -1;
			
			reverseStart[s] = false;
			xFade[s] = 0;
			loop[s] = false;
			pingpong[s] = false;
			
			attackKnob[s] = 0;
			decayKnob[s] = 0;
			sustainValue[s] = 1;
			releaseKnob[s] = 0;

			trigMode[s] = 0;
			trigType[s] = 0;
			allowPB[s] = false;
			phaseScan[s] = true;
			shape[s] = 0.5;
			stretchKnob[s] = 1;
			cycleKnob[s] = 48;
			volumeValue[s] = 1;

			refKey[s][LOKEY] = 0;
			refKey[s][HIKEY] = 127;
			refKey[s][REFKEY] = 72;

			outRoute[s] = s;
			
		}

		prevSlot = -1;
		
		selectedRange = 0;
		
		trimOnSave = false;
		antiAlias = 1;

		quantize = false;
		pbRange = 1;
		
		autoMonOff = true;
		
		resetCursorsOnLoad = true;
		saveOversampled = false;

		disableNav = false;
		sampleInPatch = true;
		unlimitedRecording = false;
		
		recordingState = 0;
		recButton = 0;
		prevMonitorSwitch = 0;
		
		system::removeRecursively(getPatchStorageDirectory().c_str());
		Module::onReset(e);
	}

	void onAdd(const AddEvent& e) override {
		for (int slot = 0; slot < 8; slot++) {
			if (!fileLoaded[slot]) {
				std::string patchFile = system::join(getPatchStorageDirectory(), ("slot"+to_string(slot+1)+".wav").c_str());
				loadFromPatch[slot] = true;
				loadSample(patchFile, slot);
			}
		}
		Module::onAdd(e);
	}

	void onSave(const SaveEvent& e) override {
		system::removeRecursively(getPatchStorageDirectory().c_str());
		
		for (int slot = 0; slot < 8; slot++) {
			if (fileLoaded[slot]) {
				if (sampleInPatch) {
					std::string patchFile = system::join(createPatchStorageDirectory(), ("slot"+to_string(slot+1)+".wav").c_str());
					saveMode = SAVE_FULL;
					loadFromPatch[slot] = true;
					saveSample(patchFile, slot);
				}
			}
		}
		Module::onSave(e);
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "TrimOnSave", json_boolean(trimOnSave));
		json_object_set_new(rootJ, "SaveOversampled", json_boolean(saveOversampled));
		json_object_set_new(rootJ, "AntiAlias", json_integer(antiAlias));
		json_object_set_new(rootJ, "quantize", json_boolean(quantize));
		json_object_set_new(rootJ, "pbRange", json_integer(pbRange));
		json_object_set_new(rootJ, "AutoMonOff", json_integer(autoMonOff));
		json_object_set_new(rootJ, "ResetCursorsOnLoad", json_boolean(resetCursorsOnLoad));
		json_object_set_new(rootJ, "DisableNav", json_boolean(disableNav));
		json_object_set_new(rootJ, "sampleInPatch", json_boolean(sampleInPatch));
		json_object_set_new(rootJ, "unlimitedRecording", json_boolean(unlimitedRecording));
		json_object_set_new(rootJ, "currSlot", json_integer(currSlot));
		json_object_set_new(rootJ, "selectedRange", json_integer(selectedRange));

		for (int slot = 0; slot < 8; slot++){
			json_object_set_new(rootJ, ("knobCueStartPos"+to_string(slot)).c_str(), json_real(knobCueStartPos[slot]));
			json_object_set_new(rootJ, ("knobCueEndPos"+to_string(slot)).c_str(), json_real(knobCueEndPos[slot]));
			json_object_set_new(rootJ, ("knobLoopStartPos"+to_string(slot)).c_str(), json_real(knobLoopStartPos[slot]));
			json_object_set_new(rootJ, ("knobLoopEndPos"+to_string(slot)).c_str(), json_real(knobLoopEndPos[slot]));
			json_object_set_new(rootJ, ("reverseStart"+to_string(slot)).c_str(), json_boolean(reverseStart[slot]));
			json_object_set_new(rootJ, ("xFade"+to_string(slot)).c_str(), json_real(xFade[slot]));
			json_object_set_new(rootJ, ("loop"+to_string(slot)).c_str(), json_boolean(loop[slot]));
			json_object_set_new(rootJ, ("pingpong"+to_string(slot)).c_str(), json_boolean(pingpong[slot]));
			json_object_set_new(rootJ, ("trigMode"+to_string(slot)).c_str(), json_integer(trigMode[slot]));
			json_object_set_new(rootJ, ("trigType"+to_string(slot)).c_str(), json_integer(trigType[slot]));
			json_object_set_new(rootJ, ("allowPB"+to_string(slot)).c_str(), json_boolean(allowPB[slot]));
			json_object_set_new(rootJ, ("phaseScan"+to_string(slot)).c_str(), json_boolean(phaseScan[slot]));
			json_object_set_new(rootJ, ("attackKnob"+to_string(slot)).c_str(), json_real(attackKnob[slot]));
			json_object_set_new(rootJ, ("decayKnob"+to_string(slot)).c_str(), json_real(decayKnob[slot]));
			json_object_set_new(rootJ, ("sustainValue"+to_string(slot)).c_str(), json_real(sustainValue[slot]));
			json_object_set_new(rootJ, ("releaseKnob"+to_string(slot)).c_str(), json_real(releaseKnob[slot]));
			json_object_set_new(rootJ, ("shape"+to_string(slot)).c_str(), json_real(shape[slot]));
			json_object_set_new(rootJ, ("stretchKnob"+to_string(slot)).c_str(), json_real(stretchKnob[slot]));
			json_object_set_new(rootJ, ("cycleKnob"+to_string(slot)).c_str(), json_real(cycleKnob[slot]));
			json_object_set_new(rootJ, ("volumeValue"+to_string(slot)).c_str(), json_real(volumeValue[slot]));
			json_object_set_new(rootJ, ("loKey"+to_string(slot)).c_str(), json_integer(refKey[slot][LOKEY]));
			json_object_set_new(rootJ, ("hiKey"+to_string(slot)).c_str(), json_integer(refKey[slot][HIKEY]));
			json_object_set_new(rootJ, ("refKey"+to_string(slot)).c_str(), json_integer(refKey[slot][REFKEY]));
			json_object_set_new(rootJ, ("outRoute"+to_string(slot)).c_str(), json_integer(outRoute[slot]));
			json_object_set_new(rootJ, ("storedPath"+to_string(slot)).c_str(), json_string(storedPath[slot].c_str()));
		}

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
		json_t* quantizeJ = json_object_get(rootJ, "quantize");
		if (quantizeJ)
			quantize = json_boolean_value(quantizeJ);
		json_t* pbRangeJ = json_object_get(rootJ, "pbRange");
		if (pbRangeJ)
			pbRange = json_integer_value(pbRangeJ);
		json_t* autoMonOffJ = json_object_get(rootJ, "AutoMonOff");
		if (autoMonOffJ)
			autoMonOff = json_integer_value(autoMonOffJ);
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
		json_t* currSlotJ = json_object_get(rootJ, "currSlot");
		if (currSlotJ)
			currSlot = json_integer_value(currSlotJ);
		json_t* selectedRangeJ = json_object_get(rootJ, "selectedRange");
		if (selectedRangeJ)
			selectedRange = json_integer_value(selectedRangeJ);

		for (int slot = 0; slot < 8; slot++) {
			json_t *knobCueStartPosJ = json_object_get(rootJ, ("knobCueStartPos"+to_string(slot)).c_str());
			if (knobCueStartPosJ)
				knobCueStartPos[slot] = json_real_value(knobCueStartPosJ);
			json_t *knobCueEndPosJ = json_object_get(rootJ, ("knobCueEndPos"+to_string(slot)).c_str());
			if (knobCueEndPosJ)
				knobCueEndPos[slot] = json_real_value(knobCueEndPosJ);
			json_t *knobLoopStartPosJ = json_object_get(rootJ, ("knobLoopStartPos"+to_string(slot)).c_str());
			if (knobLoopStartPosJ)
				knobLoopStartPos[slot] = json_real_value(knobLoopStartPosJ);
			json_t *knobLoopEndPosJ = json_object_get(rootJ, ("knobLoopEndPos"+to_string(slot)).c_str());
			if (knobLoopEndPosJ)
				knobLoopEndPos[slot] = json_real_value(knobLoopEndPosJ);
			json_t* reverseStartJ = json_object_get(rootJ, ("reverseStart"+to_string(slot)).c_str());
			if (reverseStartJ)
				reverseStart[slot] = json_boolean_value(reverseStartJ);
			json_t *xFadeJ = json_object_get(rootJ, ("xFade"+to_string(slot)).c_str());
			if (xFadeJ) {
				xFade[slot] = json_real_value(xFadeJ);
				if (xFade[slot] == 0)
					fadeSamples[slot] = 0;
				else
					fadeSamples[slot] = floor(convertCVToSeconds(xFade[slot]) * APP->engine->getSampleRate()); // number of samples before starting fade			
			}
			json_t* loopJ = json_object_get(rootJ, ("loop"+to_string(slot)).c_str());
			if (loopJ)
				loop[slot] = json_boolean_value(loopJ);
			json_t* pingpongJ = json_object_get(rootJ, ("pingpong"+to_string(slot)).c_str());
			if (pingpongJ)
				pingpong[slot] = json_boolean_value(pingpongJ);
			json_t* trigModeJ = json_object_get(rootJ, ("trigMode"+to_string(slot)).c_str());
			if (trigModeJ)
				trigMode[slot] = json_integer_value(trigModeJ);
			json_t* trigTypeJ = json_object_get(rootJ, ("trigType"+to_string(slot)).c_str());
			if (trigTypeJ)
				trigType[slot] = json_integer_value(trigTypeJ);
			json_t* allowPBJ = json_object_get(rootJ, ("allowPB"+to_string(slot)).c_str());
			if (allowPBJ)
				allowPB[slot] = json_boolean_value(allowPBJ);
			json_t* phaseScanJ = json_object_get(rootJ, ("phaseScan"+to_string(slot)).c_str());
			if (phaseScanJ)
				phaseScan[slot] = json_boolean_value(phaseScanJ);
			json_t *attackKnobJ = json_object_get(rootJ, ("attackKnob"+to_string(slot)).c_str());
			if (attackKnobJ){
				attackKnob[slot] = json_real_value(attackKnobJ);
				calcAttack(slot);
			}
			json_t *decayKnobJ = json_object_get(rootJ, ("decayKnob"+to_string(slot)).c_str());
			if (decayKnobJ) {
				decayKnob[slot] = json_real_value(decayKnobJ);
				calcDecay(slot);
			}
			json_t *sustainValueJ = json_object_get(rootJ, ("sustainValue"+to_string(slot)).c_str());
			if (sustainValueJ)
				sustainValue[slot] = json_real_value(sustainValueJ);
			json_t *releaseKnobJ = json_object_get(rootJ, ("releaseKnob"+to_string(slot)).c_str());
			if (releaseKnobJ) {
				releaseKnob[slot] = json_real_value(releaseKnobJ);
				calcRelease(slot);
			}
			json_t *shapeJ = json_object_get(rootJ, ("shape"+to_string(slot)).c_str());
			if (shapeJ)
				shape[slot] = json_real_value(shapeJ);
			json_t *stretchKnobJ = json_object_get(rootJ, ("stretchKnob"+to_string(slot)).c_str());
			if (stretchKnobJ)
				stretchKnob[slot] = json_real_value(stretchKnobJ);
			json_t *cycleKnobJ = json_object_get(rootJ, ("cycleKnob"+to_string(slot)).c_str());
			if (cycleKnobJ) {
				cycleKnob[slot] = json_real_value(cycleKnobJ);
				stretchMaxPos[slot] = oneMsSamples * cycleKnob[slot];
			}
			json_t *volumeValueJ = json_object_get(rootJ, ("volumeValue"+to_string(slot)).c_str());
			if (volumeValueJ)
				volumeValue[slot] = json_real_value(volumeValueJ);
			json_t* loKeyJ = json_object_get(rootJ, ("loKey"+to_string(slot)).c_str());
			if (loKeyJ)
				refKey[slot][LOKEY] = json_integer_value(loKeyJ);
			json_t* hiKeyJ = json_object_get(rootJ, ("hiKey"+to_string(slot)).c_str());
			if (hiKeyJ)
				refKey[slot][HIKEY] = json_integer_value(hiKeyJ);
			json_t* refKeyJ = json_object_get(rootJ, ("refKey"+to_string(slot)).c_str());
			if (refKeyJ)
				refKey[slot][REFKEY] = json_integer_value(refKeyJ);
			json_t* outRouteJ = json_object_get(rootJ, ("outRoute"+to_string(slot)).c_str());
			if (outRouteJ)
				outRoute[slot] = json_integer_value(outRouteJ);
			json_t *storedPathJ = json_object_get(rootJ, ("storedPath"+to_string(slot)).c_str());
			if (storedPathJ) {
				storedPath[slot] = json_string_value(storedPathJ);
				if (storedPath[slot] != "")
					loadSample(storedPath[slot], slot);
				else
					firstLoad[slot] = false;
			}
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

	inline void calcAttack(int slot) {
		attackValue[slot] = convertCVToSeconds(attackKnob[slot]);
		if (attackValue[slot] > maxAdsrTime)
			attackValue[slot] = maxAdsrTime;
		else if (attackValue[slot] < minAdsrTime)
			attackValue[slot] = minAdsrTime;
		prevAttackKnob[slot] = attackKnob[slot];
	}
	
	inline void calcDecay(int slot) {
		decayValue[slot] = convertCVToSeconds(decayKnob[slot]);
		if (decayValue[slot] > maxAdsrTime)
			decayValue[slot] = maxAdsrTime;
		else if (decayValue[slot] < minAdsrTime)
			decayValue[slot] = minAdsrTime;
		prevDecayKnob[slot] = decayKnob[slot];
	}

	inline void calcRelease(int slot) {
		releaseValue[slot] = convertCVToSeconds(releaseKnob[slot]);
		if (releaseValue[slot] > maxAdsrTime)
			releaseValue[slot] = maxAdsrTime;
		else if (releaseValue[slot] < minAdsrTime)
			releaseValue[slot] = minAdsrTime;
		prevReleaseKnob[slot] = releaseKnob[slot];
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
		learnButLightDelta = 2 / APP->engine->getSampleRate();
		lightPulseTime = (APP->engine->getSampleRate()) / 10;
		learnSamples = APP->engine->getSampleRate();
		maxStoredLearnTime = APP->engine->getSampleRate() * 0.75;

		prevLearnKey = -11.f;

		prevXfade[0] = -1.f;
		prevXfade[1] = -1.f;
		prevXfade[2] = -1.f;
		prevXfade[3] = -1.f;
		prevXfade[4] = -1.f;
		prevXfade[5] = -1.f;
		prevXfade[6] = -1.f;
		prevXfade[7] = -1.f;

		vcvSampleRate = APP->engine->getSampleRate();
		envSrCoeff = 1 / APP->engine->getSampleRate();
		sampleRate = vcvSampleRate * 2;

		double oldSampleRate = sampleRate;
		double newSampleRate = APP->engine->getSampleRate();
		//sampleRate = newSampleRate * 2;

		if (newSampleRate != oldSampleRate/2) {

			for (int slot = 0; slot < 8; slot++) {
				
				if (fileLoaded[slot]) {

					double resampleCoeff;

					fileLoaded[slot] = false;

					z1 = 0; z2 = 0; z1r = 0; z2r = 0;

					// metamodule change
					//tempBuffer[0].clear();
					//tempBuffer[1].clear();
					vector<float> tempBuffer[2];

					for (unsigned int i=0; i < totalSampleC[slot]; i++) {
						tempBuffer[LEFT].push_back(playBuffer[slot][LEFT][0][i]);
						if (channels[slot] == 2) {
							tempBuffer[RIGHT].push_back(playBuffer[slot][RIGHT][0][i]);
						}
					}
					
					playBuffer[slot][LEFT][0].clear();
					playBuffer[slot][LEFT][1].clear();
					playBuffer[slot][RIGHT][0].clear();
					playBuffer[slot][RIGHT][1].clear();

					// metamodule change
					vector<float>().swap(playBuffer[slot][LEFT][0]);
					vector<float>().swap(playBuffer[slot][RIGHT][0]);
					vector<float>().swap(playBuffer[slot][LEFT][1]);
					vector<float>().swap(playBuffer[slot][RIGHT][1]);

					drwav_uint64 tempSampleC = totalSampleC[slot];
					drwav_uint64 tempSamples = totalSamples[slot];
			
					resampleCoeff = oldSampleRate / (newSampleRate) / 2;
					
					double currResamplePos = 0;
					double floorCurrResamplePos = 0;

					playBuffer[slot][LEFT][0].push_back(tempBuffer[LEFT][0]);
					if (channels[slot] == 2)
						playBuffer[slot][RIGHT][0].push_back(tempBuffer[RIGHT][0]);

					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
					int temp;

					while ( floorCurrResamplePos < 1 ) {
						temp = tempBuffer[LEFT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
									tempBuffer[LEFT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
						playBuffer[slot][LEFT][0].push_back(temp);
						if (channels[slot] == 2) {
							temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
									tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
							playBuffer[slot][RIGHT][0].push_back(temp);
						}
						currResamplePos += resampleCoeff;
						floorCurrResamplePos = floor(currResamplePos);
					} 

					while ( currResamplePos < tempSamples - 1) {
						playBuffer[slot][LEFT][0].push_back(	hermiteInterpol(tempBuffer[LEFT][floorCurrResamplePos - 1],
																		tempBuffer[LEFT][floorCurrResamplePos],
																		tempBuffer[LEFT][floorCurrResamplePos + 1],
																		tempBuffer[LEFT][floorCurrResamplePos + 2],
																		currResamplePos - floorCurrResamplePos)
													);
						if (channels[slot] == 2)
							playBuffer[slot][RIGHT][0].push_back(	hermiteInterpol(tempBuffer[RIGHT][floorCurrResamplePos - 1],
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
						playBuffer[slot][LEFT][0].push_back(temp);
						if (channels[slot] == 2) {
							temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
									tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
							playBuffer[slot][RIGHT][0].push_back(temp);
						}
						currResamplePos += resampleCoeff;
						floorCurrResamplePos = floor(currResamplePos);
					}

					// metamodule change
					//tempBuffer[LEFT].clear();
					//tempBuffer[RIGHT].clear();

					// ***************************************************************************
					totalSampleC[slot] = playBuffer[slot][LEFT][0].size();
					totalSamples[slot] = totalSampleC[slot]-1;					// *****   DA VERIFICARE se è -2 ********************************************
					//sampleRate = newSampleRate * 2;  // già impostato all'inizio
					// *************************************************************

					calcBiquadLpf(20000.0, sampleRate, 1);
					for (unsigned int i = 0; i < totalSampleC[slot]; i++) {
						playBuffer[slot][LEFT][1].push_back(biquadLpf(playBuffer[slot][LEFT][0][i]));
						if (channels[slot] == 2)
							playBuffer[slot][RIGHT][1].push_back(biquadLpf2(playBuffer[slot][RIGHT][0][i]));
					}

					prevKnobCueStartPos[slot] = -1.f;
					prevKnobCueEndPos[slot] = 2.f;
					prevKnobLoopStartPos[slot] = -1.f;
					prevKnobLoopEndPos[slot] = 2.f;

					vector<double>().swap(displayBuff[slot]);
					for (int i = 0; i < floor(totalSampleC[slot]); i = i + floor(totalSampleC[slot]/wavDisp))
						displayBuff[slot].push_back(playBuffer[slot][0][0][i]);

					resampled[slot] = true;
					//recSamples = 0;	// *********** messo fuori del loop
					fileLoaded[slot] = true;
				}
			}
			recSamples = 0;
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
	void menuSaveSample(int mode, int slot) {
		fileChannels[slot] = channels[slot];
		fileLoaded[slot] = false;
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
			saveSample(path, slot);
			std::string newPath = std::string(path);
			if (newPath.substr(newPath.size() - 4) != ".wav" && newPath.substr(newPath.size() - 4) != ".WAV")
				newPath += ".wav";
			storedPath[slot] = newPath;
		}
		channels[slot] = fileChannels[slot];
		fileLoaded[slot] = true;
		free(path);
#if defined(METAMODULE)
		});
#endif
	};

	void saveSample(std::string path, int slot) {

		std::string newPath = path;

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
				saveEndSamplePos = floor(totalSampleC[slot]/overSampleCoeff);
			break;
			case SAVE_CUE:
				saveStartSamplePos = floor(cueStartPos[slot]/overSampleCoeff);
				saveEndSamplePos = floor(cueEndPos[slot]/overSampleCoeff);
			break;
			case SAVE_LOOP:
				saveStartSamplePos = floor(loopStartPos[slot]/overSampleCoeff);
				saveEndSamplePos = floor(loopEndPos[slot]/overSampleCoeff);
			break;
		}

		if (!saveOversampled) {
			if (saveStartSamplePos % 2 != 0 && saveStartSamplePos > 0)
				saveStartSamplePos--;

			if (saveEndSamplePos % 2 != 0 && saveEndSamplePos < totalSamples[slot])
				saveEndSamplePos++;
		}

		samples = floor(saveEndSamplePos - saveStartSamplePos);

		std::vector<float> data;

		switch (fileChannels[slot]) {
			case 1:
				for (unsigned int i = saveStartSamplePos; i <= saveEndSamplePos; i++)
					data.push_back(playBuffer[slot][LEFT][0][i*overSampleCoeff] / 5);
			break;

			case 2:
				for (unsigned int i = saveStartSamplePos; i <= saveEndSamplePos; i++) {
					data.push_back(playBuffer[slot][LEFT][0][i*overSampleCoeff] / 5);
					data.push_back(playBuffer[slot][RIGHT][0][i*overSampleCoeff] / 5);
				}
			break;
		}

		drwav_data_format format;
		format.container = drwav_container_riff;
		format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
	
		if (fileChannels[slot] == 1) 
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
		toSave[slot] = false;
		infoToSave[slot] = "";

		char* pathDup = strdup(path.c_str());
		//fileDescription = basename(pathDup);
		if (!loadFromPatch[slot]) {
			toSave[slot] = false;
			infoToSave[slot] = "";
			fileDescription[slot] = basename(pathDup);
		} else {
			/*if (fileDescription[slot] != "_unknown_.wav")
				fileDescription[slot] = basename(pathDup);*/
			if (fileDescription[slot] != "_slot"+to_string(slot+1)+".wav")
				fileDescription[slot] = basename(pathDup);
		}

		// *** CHARs CHECK according to font
		std::string tempFileDisplay = fileDescription[slot].substr(0, fileDescription[slot].size()-4);
		/*
		char tempFileChar;
		fileDisplay[slot] = "";
		for (int i = 0; i < int(tempFileDisplay.length()); i++) {
			tempFileChar = tempFileDisplay.at(i);
			if ( (int(tempFileChar) > 47 && int(tempFileChar) < 58)
					|| (int(tempFileChar) > 64 && int(tempFileChar) < 123)
					|| int(tempFileChar) == 45 || int(tempFileChar) == 46 || int(tempFileChar) == 95 )
				fileDisplay[slot] += tempFileChar;
		}
		fileDisplay[slot] = fileDisplay[slot].substr(0, maxTitleChars);
		*/
		fileDisplay[slot] = tempFileDisplay.substr(0, maxTitleChars);

		free(pathDup);
		data.clear();

		//storedPath[slot] = path;
		if (!loadFromPatch[slot]) {
			storedPath[slot] = path;
		
			if (trimOnSave && saveMode == SAVE_LOOP) {
				// set position knob to 0 and 1
				knobCueStartPos[slot] = 0;
				knobCueEndPos[slot] = 1;
				knobLoopStartPos[slot] = 0;
				knobLoopEndPos[slot] = 1;
				params[CUESTART_PARAM].setValue(0);
				params[CUEEND_PARAM].setValue(1);
				params[LOOPSTART_PARAM].setValue(0);
				params[LOOPEND_PARAM].setValue(1);

				loadSample(path, slot);
			} else if (trimOnSave && saveMode == SAVE_CUE) {
				float tempKnobLoopStartPos = (knobLoopStartPos[slot] - knobCueStartPos[slot] ) / (knobCueEndPos[slot] - knobCueStartPos[slot]);
				float tempKnobLoopEndPos = (knobLoopEndPos[slot] - knobCueStartPos[slot] ) / (knobCueEndPos[slot] - knobCueStartPos[slot]);
				loadSample(path, slot);

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

		fileLoaded[slot] = true;
		fileFound[slot] = true;
		channels[slot] = fileChannels[slot];
	}
/*

																					██╗░░░░░░█████╗░░█████╗░██████╗░
																					██║░░░░░██╔══██╗██╔══██╗██╔══██╗
																					██║░░░░░██║░░██║███████║██║░░██║
																					██║░░░░░██║░░██║██╔══██║██║░░██║
																					███████╗╚█████╔╝██║░░██║██████╔╝
																					╚══════╝░╚════╝░╚═╝░░╚═╝╚═════╝░
*/
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
		restoreLoadFromPatch[slot] = false;
		if (path) {
			//loadFromPatch[slot] = false; // ****************************************** QUESTO C'ERA SOLO SU DRUMPLAYER
			loadSample(path, slot);
			storedPath[slot] = std::string(path);
		} else {
			restoreLoadFromPatch[slot] = true;
			fileLoaded[slot] = true;
		}
		if ((storedPath[slot] == "" || fileFound[slot] == false) && !toSave[slot]) {
			fileLoaded[slot] = false;
		}
		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadSample(std::string fromPath, int slot) {
		std::string path = fromPath;
		z1 = 0; z2 = 0; z1r = 0; z2r = 0;

		// metamodule change
		//tempBuffer[0].clear();
		//tempBuffer[1].clear();
		vector<float> tempBuffer[2];

		double tempSampleRate;

		unsigned int c;
		unsigned int sr;
		drwav_uint64 tsc;
		float* pSampleData;
		pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &tsc);

		if (pSampleData != NULL && tsc > minSamplesToLoad * c) {

			fileFound[slot] = true;
			fileChannels[slot] = c;
			tempSampleRate = sr * 2;
			fileSampleRate[slot] = sr;

			for (int c=0; c < 16; c++)
				samplePos[slot][c] = 0;
			
			playBuffer[slot][LEFT][0].clear();
			playBuffer[slot][LEFT][1].clear();
			playBuffer[slot][RIGHT][0].clear();
			playBuffer[slot][RIGHT][1].clear();
			tempBuffer[LEFT].clear();
			tempBuffer[RIGHT].clear();
			displayBuff[slot].clear();

			// metamodule change
			vector<float>().swap(playBuffer[slot][LEFT][0]);
			vector<float>().swap(playBuffer[slot][RIGHT][0]);
			vector<float>().swap(playBuffer[slot][LEFT][1]);
			vector<float>().swap(playBuffer[slot][RIGHT][1]);

			if (!unlimitedRecording) {
				if (tsc > recordingLimit / 2)
					tsc = recordingLimit / 2;	// set memory allocation limit to 200Mb for samples (~18mins at 48.000khz MONO)
			}

			if (sr == APP->engine->getSampleRate()) {			//  **************************   NO RESAMPLE   ************************
				for (unsigned int i=0; i < tsc; i = i + c) {
					playBuffer[slot][LEFT][0].push_back(pSampleData[i] * 5);
					playBuffer[slot][LEFT][0].push_back(0);
					if (fileChannels[slot] == 2) {
						playBuffer[slot][RIGHT][0].push_back(pSampleData[i+1] * 5);
						playBuffer[slot][RIGHT][0].push_back(0);
					}
				}
				totalSampleC[slot] = playBuffer[slot][LEFT][0].size();
				totalSamples[slot] = totalSampleC[slot]-1;					// *****   DA VERIFICARE se è -2 ********************************************
				drwav_free(pSampleData);

				for (unsigned int i = 1; i < totalSamples[slot]; i = i+2) { // **************** tempSamples  o tempSampleC ????
					playBuffer[slot][LEFT][0][i] = playBuffer[slot][LEFT][0][i-1] * .5f + playBuffer[slot][LEFT][0][i+1] * .5f;
					if (fileChannels[slot] == 2)
						playBuffer[slot][RIGHT][0][i] = playBuffer[slot][RIGHT][0][i-1] * .5f + playBuffer[slot][RIGHT][0][i+1] * .5f;
				}

				playBuffer[slot][LEFT][0][totalSamples[slot]] = playBuffer[slot][LEFT][0][totalSamples[slot]-1] * .5f; // halve the last sample
				if (fileChannels[slot] == 2)
					playBuffer[slot][RIGHT][0][totalSamples[slot]] = playBuffer[slot][RIGHT][0][totalSamples[slot]-1] * .5f;

				resampled[slot] = false;

			} else if (sr == APP->engine->getSampleRate() * 2) {	// ***** LOAD DIRECTLY OVERSAMPLED, NO RESAMPLE *****
				for (unsigned int i=0; i < tsc; i = i + c) {
					playBuffer[slot][LEFT][0].push_back(pSampleData[i] * 5);
					if (fileChannels[slot] == 2) {
						playBuffer[slot][RIGHT][0].push_back(pSampleData[i+1] * 5);
					}
				}
				totalSampleC[slot] = playBuffer[slot][LEFT][0].size();
				totalSamples[slot] = totalSampleC[slot]-1;					// *****   DA VERIFICARE se è -2 ********************************************
				drwav_free(pSampleData);

				resampled[slot] = false;

				//sampleRate = APP->engine->getSampleRate() * 2;

			} else {											// ***************** RESAMPLE ****************************************
				for (unsigned int i=0; i < tsc; i = i + c) {
					tempBuffer[LEFT].push_back(pSampleData[i] * 5);
					tempBuffer[LEFT].push_back(0);
					if (fileChannels[slot] == 2) {
						tempBuffer[RIGHT].push_back(pSampleData[i+1] * 5);
						tempBuffer[RIGHT].push_back(0);
					}
				}

				drwav_free(pSampleData);

				drwav_uint64 tempSampleC = tempBuffer[LEFT].size();
				drwav_uint64 tempSamples = tempSampleC-1;					// *****   DA VERIFICARE se è -2 ********************************************
				

				for (unsigned int i = 1; i < tempSamples; i = i+2) { // **************** tempSamples  o tempSampleC ????
					tempBuffer[LEFT][i] = tempBuffer[LEFT][i-1] * .5f + tempBuffer[LEFT][i+1] * .5f;
					if (fileChannels[slot] == 2)
						tempBuffer[RIGHT][i] = tempBuffer[RIGHT][i-1] * .5f + tempBuffer[RIGHT][i+1] * .5f;
				}

				tempBuffer[LEFT][tempSamples] = tempBuffer[LEFT][tempSamples-1] * .5f; // halve the last sample
				if (fileChannels[slot] == 2)
					tempBuffer[RIGHT][tempSamples] = tempBuffer[RIGHT][tempSamples-1] * .5f;

				// ***************************************************************************

				double resampleCoeff = tempSampleRate * .5 / (APP->engine->getSampleRate());
				double currResamplePos = 0;
				int floorCurrResamplePos = 0;

				playBuffer[slot][LEFT][0].push_back(tempBuffer[LEFT][0]);
				if (fileChannels[slot] == 2)
					playBuffer[slot][RIGHT][0].push_back(tempBuffer[RIGHT][0]);

				currResamplePos += resampleCoeff;
				floorCurrResamplePos = floor(currResamplePos);
				int temp;

				while ( floorCurrResamplePos < 1 ) {
					temp = tempBuffer[LEFT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[LEFT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					playBuffer[slot][LEFT][0].push_back(temp);
					if (fileChannels[slot] == 2) {
						temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
						playBuffer[slot][RIGHT][0].push_back(temp);
					}
					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				} 

				while ( currResamplePos < tempSamples - 1) {
					playBuffer[slot][LEFT][0].push_back(	hermiteInterpol(tempBuffer[LEFT][floorCurrResamplePos - 1],
																	tempBuffer[LEFT][floorCurrResamplePos],
																	tempBuffer[LEFT][floorCurrResamplePos + 1],
																	tempBuffer[LEFT][floorCurrResamplePos + 2],
																	currResamplePos - floorCurrResamplePos)
												);
					if (fileChannels[slot] == 2)
						playBuffer[slot][RIGHT][0].push_back(	hermiteInterpol(tempBuffer[RIGHT][floorCurrResamplePos - 1],
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
					playBuffer[slot][LEFT][0].push_back(temp);
					if (fileChannels[slot] == 2) {
						temp = tempBuffer[RIGHT][floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[RIGHT][floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
						playBuffer[slot][RIGHT][0].push_back(temp);
					}
					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				}

				// metamodule change
				//tempBuffer[LEFT].clear();
				//tempBuffer[RIGHT].clear();

				// ***************************************************************************

				totalSampleC[slot] = playBuffer[slot][LEFT][0].size();
				totalSamples[slot] = totalSampleC[slot]-1;					// *****   DA VERIFICARE se è -2 ********************************************

				//sampleRate = APP->engine->getSampleRate() * 2;

				resampled[slot] = true;
			}

			calcBiquadLpf(20000.0, sampleRate, 1);
			for (unsigned int i = 0; i < totalSampleC[slot]; i++) {
				playBuffer[slot][LEFT][1].push_back(biquadLpf(playBuffer[slot][LEFT][0][i]));
				if (fileChannels[slot] == 2)
					playBuffer[slot][RIGHT][1].push_back(biquadLpf2(playBuffer[slot][RIGHT][0][i]));
			}

			vector<double>().swap(displayBuff[slot]);
			for (int i = 0; i < floor(totalSampleC[slot]); i = i + floor(totalSampleC[slot]/wavDisp))
				displayBuff[slot].push_back(playBuffer[slot][0][0][i]);

			seconds = totalSampleC[slot] * 0.5 / (APP->engine->getSampleRate());
			minutes = (seconds / 60) % 60;
			seconds = seconds % 60;

			timeDisplay[slot] = std::to_string(minutes) + ":";
			if (seconds < 10)
				timeDisplay[slot] += "0";
			timeDisplay[slot] += std::to_string(seconds);

			if (loadFromPatch[slot]) {
				if (storedPath[slot] != "")
					path = storedPath[slot];
				else
					//path = "_unknown_.wav";
					path = "_slot"+to_string(slot+1)+".waw";
			}

			char* pathDup = strdup(path.c_str());
			fileDescription[slot] = basename(pathDup);

			if (loadFromPatch[slot]) {
				if (storedPath[slot] != "")
					fileDescription[slot] = "(!)"+fileDescription[slot];
			}

			// *** CHARs CHECK according to font
			std::string tempFileDisplay = fileDescription[slot].substr(0, fileDescription[slot].size()-4);
			/*
			char tempFileChar;
			fileDisplay[slot] = "";
			for (int i = 0; i < int(tempFileDisplay.length()); i++) {
				tempFileChar = tempFileDisplay.at(i);
				if ( (int(tempFileChar) > 47 && int(tempFileChar) < 58)
					|| (int(tempFileChar) > 64 && int(tempFileChar) < 123)
					|| int(tempFileChar) == 45 || int(tempFileChar) == 46 || int(tempFileChar) == 95 )
					fileDisplay[slot] += tempFileChar;
			}
			fileDisplay[slot] = fileDisplay[slot].substr(0, maxTitleChars);
			*/
			fileDisplay[slot] = tempFileDisplay.substr(0, maxTitleChars);

			samplerateDisplay[slot] = std::to_string(fileSampleRate[slot]);
			channelsDisplay[slot] = std::to_string(fileChannels[slot]) + "ch";
			free(pathDup);
			storedPath[slot] = path;

			if (!loadFromPatch[slot]) {
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

				if (!firstLoad[slot] || toSave[slot]) {
					prevKnobCueStartPos[slot] = -1.f;
					prevKnobCueEndPos[slot] = 2.f;
					prevKnobLoopStartPos[slot] = -1.f;
					prevKnobLoopEndPos[slot] = 2.f;

					if (resetCursorsOnLoad) {
						params[CUESTART_PARAM].setValue(0.f);
						params[CUEEND_PARAM].setValue(1.f);
						params[LOOPSTART_PARAM].setValue(0.f);
						params[LOOPEND_PARAM].setValue(1.f);
						knobCueStartPos[slot] = 0.f;
						knobCueEndPos[slot] = 1.f;
						knobLoopStartPos[slot] = 0.f;
						knobLoopEndPos[slot] = 1.f;
					}

				}

				cueStartPos[slot] = floor(totalSamples[slot] * knobCueStartPos[slot]);
				cueEndPos[slot] = ceil(totalSamples[slot] * knobCueEndPos[slot]);
				loopStartPos[slot] = floor(totalSamples[slot] * knobLoopStartPos[slot]);
				loopEndPos[slot] = ceil(totalSamples[slot] * knobLoopEndPos[slot]);

				scanCueStartSample[slot] = cueStartPos[slot];
				scanCueEndSample[slot] = cueEndPos[slot];
				scanLoopStartSample[slot] = loopStartPos[slot];
				scanLoopEndSample[slot] = loopEndPos[slot];


				toSave[slot] = false;
				infoToSave[slot] = "";
			
			} else {

				// Questi commenti sotto evitano di sovrascrivere i valori dei knob sui diversi slot
				// da testare quando vengono caricati slot NON IN PATCH

				//knobCueStartPos[slot] = params[CUESTART_PARAM].getValue();
				//knobCueEndPos[slot] = params[CUEEND_PARAM].getValue();
				prevKnobCueStartPos[slot] = knobCueStartPos[slot];
				prevKnobCueEndPos[slot] = knobCueEndPos[slot];
				//knobLoopStartPos[slot] = params[LOOPSTART_PARAM].getValue();
				//knobLoopEndPos[slot] = params[LOOPEND_PARAM].getValue();
				prevKnobLoopStartPos[slot] = knobLoopStartPos[slot];
				prevKnobLoopEndPos[slot] = knobLoopEndPos[slot];

				cueStartPos[slot] = floor(totalSamples[slot] * knobCueStartPos[slot]);
				cueEndPos[slot] = ceil(totalSamples[slot] * knobCueEndPos[slot]);
				loopStartPos[slot] = floor(totalSamples[slot] * knobLoopStartPos[slot]);
				loopEndPos[slot] = ceil(totalSamples[slot] * knobLoopEndPos[slot]);

				scanCueStartSample[slot] = cueStartPos[slot];
				scanCueEndSample[slot] = cueEndPos[slot];
				scanLoopStartSample[slot] = loopStartPos[slot];
				scanLoopEndSample[slot] = loopEndPos[slot];

				toSave[slot] = true;
				infoToSave[slot] = "s";
			}

			recSeconds = 0;
			recMinutes = 0;
			
			firstLoad[slot] = false;

			fileLoaded[slot] = true;
			channels[slot] = fileChannels[slot];

		} else {
			fileFound[slot] = false;
			fileLoaded[slot] = false;
			//storedPath = path;
			if (loadFromPatch[slot])
				path = storedPath[slot];
			fileDescription[slot] = "(!)"+path;
			fileDisplay[slot] = "";
			timeDisplay[slot] = "";
			recSamples = 0;
			recTimeDisplay[slot] = "";
			channelsDisplay[slot] = "";
		}
	};
	
	void clearSlot(int slot) {
		fileLoaded[slot] = false;
		fileFound[slot] = false;
		loadFromPatch[slot] = false;
		restoreLoadFromPatch[slot] = false;
		storedPath[slot] = "";
		fileDescription[slot] = "--none--";
		fileDisplay[slot] = "";
		timeDisplay[slot] = "";
		recSamples = 0;
		recTimeDisplay[slot] = "";
		channelsDisplay[slot] = "";
		playBuffer[slot][LEFT][0].clear();
		playBuffer[slot][RIGHT][0].clear();
		playBuffer[slot][LEFT][1].clear();
		playBuffer[slot][RIGHT][1].clear();
		displayBuff[slot].clear();

		// metamodule change
		vector<float>().swap(playBuffer[slot][LEFT][0]);
		vector<float>().swap(playBuffer[slot][RIGHT][0]);
		vector<float>().swap(playBuffer[slot][LEFT][1]);
		vector<float>().swap(playBuffer[slot][RIGHT][1]);

		totalSampleC[slot] = 0;
		totalSamples[slot] = 0;
		resampled[slot] = false;
		toSave[slot] = false;
		infoToSave[slot] = "";
		for (int slot = 0; slot < 8; slot++)
			for (int c=0; c < 16; c++)
				fadingValue[slot][c] = 1;
	}

	void resetCursors() {
		params[CUESTART_PARAM].setValue(0);
		params[CUEEND_PARAM].setValue(1);
		params[LOOPSTART_PARAM].setValue(0);
		params[LOOPEND_PARAM].setValue(1);
	}

	void copyCueToLoop() {
		loopStartPos[currSlot] = cueStartPos[currSlot];
		loopEndPos[currSlot] = cueEndPos[currSlot];
		params[LOOPSTART_PARAM].setValue(params[CUESTART_PARAM].getValue());
		params[LOOPEND_PARAM].setValue(params[CUEEND_PARAM].getValue());
		prevKnobLoopStartPos[currSlot] = params[CUESTART_PARAM].getValue();
		prevKnobLoopEndPos[currSlot] = params[CUEEND_PARAM].getValue();
		knobLoopStartPos[currSlot] = params[CUESTART_PARAM].getValue();
		knobLoopEndPos[currSlot] = params[CUEEND_PARAM].getValue();
	}

	void copyLoopToCue() {
		cueStartPos[currSlot] = loopStartPos[currSlot];
		cueEndPos[currSlot] = loopEndPos[currSlot];
		params[CUESTART_PARAM].setValue(params[LOOPSTART_PARAM].getValue());
		params[CUEEND_PARAM].setValue(params[LOOPEND_PARAM].getValue());
		prevKnobCueStartPos[currSlot] = params[LOOPSTART_PARAM].getValue();
		prevKnobCueEndPos[currSlot] = params[LOOPEND_PARAM].getValue();
		knobCueStartPos[currSlot] = params[LOOPSTART_PARAM].getValue();
		knobCueEndPos[currSlot] = params[LOOPEND_PARAM].getValue();
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
				prevKnobCueStartPos[currSlot] = -1.f;
				prevKnobCueEndPos[currSlot] = 2.f;
				prevKnobLoopStartPos[currSlot] = -1.f;
				prevKnobLoopEndPos[currSlot] = 2.f;
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

				prevKnobCueStartPos[currSlot] = -1.f;
				prevKnobCueEndPos[currSlot] = 2.f;
				prevKnobLoopStartPos[currSlot] = -1.f;
				prevKnobLoopEndPos[currSlot] = 2.f;
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
				prevKnobCueStartPos[currSlot] = -1.f;
				prevKnobCueEndPos[currSlot] = 2.f;
				prevKnobLoopStartPos[currSlot] = -1.f;
				prevKnobLoopEndPos[currSlot] = 2.f;
			break;
		}
	}

	/*
	bool isPolyOuts() {
		return polyOuts;
	}

	void setPolyOuts(bool poly) {
		if (poly) 
			polyOuts = 1;
		else {
			polyOuts = 0;
			//outputs[OUT_OUTPUT].setChannels(1);
			//outputs[OUT_OUTPUT+1].setChannels(1);
			//outputs[EOC_OUTPUT].setChannels(1);
			//outputs[EOR_OUTPUT].setChannels(1);
		}
	}
	*/

	float shapeResponse(float value, int slot) {
		
		if (shape[slot] < 0.5f)
			return (expTable[0][int(expTableCoeff * value)] * (1 - (shape[slot] * 2.f))) + (value * (shape[slot] * 2.f));
		else
			return (value * (1 - ((shape[slot] - 0.5f) * 2.f))) + (expTable[1][int(expTableCoeff * value)] * ((shape[slot] - 0.5f) * 2.f));

	}

	float shapeResponse2(float value, int slot) {
		
		if (shape[slot] < 0.5f)
			return (expTable[1][int(expTableCoeff * value)] * (1 - (shape[slot] * 2.f))) + (value * (shape[slot] * 2.f));
		else
			return (value * (1 - ((shape[slot] - 0.5f) * 2.f))) + (expTable[0][int(expTableCoeff * value)] * ((shape[slot] - 0.5f) * 2.f));

	}


//																				██████╗░██████╗░░█████╗░░█████╗░███████╗░██████╗░██████╗
//																				██╔══██╗██╔══██╗██╔══██╗██╔══██╗██╔════╝██╔════╝██╔════╝
//																				██████╔╝██████╔╝██║░░██║██║░░╚═╝█████╗░░╚█████╗░╚█████╗░
//																				██╔═══╝░██╔══██╗██║░░██║██║░░██╗██╔══╝░░░╚═══██╗░╚═══██╗
//																				██║░░░░░██║░░██║╚█████╔╝╚█████╔╝███████╗██████╔╝██████╔╝
//																				╚═╝░░░░░╚═╝░░╚═╝░╚════╝░░╚════╝░╚══════╝╚═════╝░╚═════╝░

	void process(const ProcessArgs &args) override {

		currSlot = int(params[SLOT_SELECTOR_PARAM].getValue())-1;
		if (currSlot != prevSlot) {
			params[CUESTART_PARAM].setValue(knobCueStartPos[currSlot]);
			params[CUEEND_PARAM].setValue(knobCueEndPos[currSlot]);
			params[LOOPSTART_PARAM].setValue(knobLoopStartPos[currSlot]);
			params[LOOPEND_PARAM].setValue(knobLoopEndPos[currSlot]);
			params[REV_PARAM].setValue(reverseStart[currSlot]);
			params[XFADE_PARAM].setValue(xFade[currSlot]);
			params[LOOP_PARAM].setValue(loop[currSlot]);
			params[PINGPONG_PARAM].setValue(pingpong[currSlot]);
			params[TRIGGATEMODE_SWITCH].setValue(trigMode[currSlot]);
			params[TRIGMODE_SWITCH].setValue(trigType[currSlot]);
			params[PB_SWITCH].setValue(allowPB[currSlot]);
			params[PHASESCAN_SWITCH].setValue(phaseScan[currSlot]);
			params[ATTACK_PARAM].setValue(attackKnob[currSlot]);
			params[DECAY_PARAM].setValue(decayKnob[currSlot]);
			params[SUSTAIN_PARAM].setValue(sustainValue[currSlot]);
			params[RELEASE_PARAM].setValue(releaseKnob[currSlot]);
			params[CURVE_PARAM].setValue(shape[currSlot]);
			params[STRETCH_PARAM].setValue(stretchKnob[currSlot]);
			params[STR_SIZE_PARAM].setValue(cycleKnob[currSlot]);
			params[VOL_PARAM].setValue(volumeValue[currSlot]);
			params[OUT_SELECTOR_PARAM].setValue(outRoute[currSlot]+1);
			params[RANGE_PARAM].setValue(refKey[currSlot][selectedRange]);

			prevSlot = currSlot;
			//DEBUG("current slot %d", currSlot);
		}

		// *************** CURRENT SLOT PARAMS


		reverseStart[currSlot] = params[REV_PARAM].getValue();
		lights[REV_LIGHT].setBrightness(reverseStart[currSlot]);

		xFade[currSlot] = params[XFADE_PARAM].getValue();

		loop[currSlot] = params[LOOP_PARAM].getValue();
		lights[LOOP_LIGHT].setBrightness(loop[currSlot]);

		pingpong[currSlot] = params[PINGPONG_PARAM].getValue();
		lights[PINGPONG_LIGHT].setBrightness(pingpong[currSlot]);

		trigMode[currSlot] = params[TRIGGATEMODE_SWITCH].getValue();
		trigType[currSlot] = params[TRIGMODE_SWITCH].getValue();
		lights[TRIGMODE_LIGHT].setBrightness(trigType[currSlot]);

		allowPB[currSlot] = params[PB_SWITCH].getValue();
		lights[PB_LIGHT].setBrightness(allowPB[currSlot]);

		phaseScan[currSlot] = params[PHASESCAN_SWITCH].getValue();
		lights[PHASESCAN_LIGHT].setBrightness(phaseScan[currSlot]);

		attackKnob[currSlot] = params[ATTACK_PARAM].getValue();
		if (attackKnob[currSlot] != prevAttackKnob[currSlot])
			calcAttack(currSlot);

		decayKnob[currSlot] = params[DECAY_PARAM].getValue();
		if (decayKnob[currSlot] != prevDecayKnob[currSlot])
			calcDecay(currSlot);

		sustainValue[currSlot] = params[SUSTAIN_PARAM].getValue();

		releaseKnob[currSlot] = params[RELEASE_PARAM].getValue();
		if (releaseKnob[currSlot] != prevReleaseKnob[currSlot])
			calcRelease(currSlot);

		shape[currSlot] = params[CURVE_PARAM].getValue();

		stretchKnob[currSlot] = params[STRETCH_PARAM].getValue();
		cycleKnob[currSlot] = params[STR_SIZE_PARAM].getValue();
		stretchMaxPos[currSlot] = oneMsSamples * cycleKnob[currSlot];

		volumeValue[currSlot] = params[VOL_PARAM].getValue();

		outRoute[currSlot] = int(params[OUT_SELECTOR_PARAM].getValue())-1;

		refKey[currSlot][selectedRange] = params[RANGE_PARAM].getValue();

		trigRange = params[RANGE_SELECTOR_PARAM].getValue();
		if (!learn && !storedLearn) {
			learnButLightValue = trigRange;
		} else {
			if (learningButLight) {
				if (learnButLightValue > 1 || learnButLightValue < 0) {
					learnButLightDelta *= -1;
				}
				learnButLightValue += learnButLightDelta;
			}
		}

		if (trigRange && !prevTrigRange) {
			if (!waitingLearnEnter) {
				waitingLearnEnter = true;
				waitingLearnEnterTime = learnSamples;
				if (learn) {
					learn = false;
					learnButLightValue = 0.f;
					learningButLight = false;
					selectedRange--;
					if (selectedRange < LOKEY)
						selectedRange = REFKEY;
					params[RANGE_PARAM].setValue(refKey[currSlot][selectedRange]);
				}
			}
		} else if (!trigRange && prevTrigRange) {	// controlla il rilascio del tasto learn
			if (!learn) {
				selectedRange++;
				if (selectedRange > REFKEY)
					selectedRange = LOKEY;
				params[RANGE_PARAM].setValue(refKey[currSlot][selectedRange]);
			} 
			waitingLearnEnter = false;
		}
		prevTrigRange = trigRange;
		
		if (waitingLearnEnter) {
			waitingLearnEnterTime--;
			if (waitingLearnEnterTime < 0) {
				waitingLearnEnter = false;
				learn = true;
				learningButLight = true;
				learnButLightValue = 0.f;
				prevLearnKey = -11.f;
			}
		}

		chan = std::max(1, inputs[GATE_INPUT].getChannels());

		if (learn) {
			for (int c = 0; c < chan; c++) {
				learnKey = inputs[GATE_INPUT].getVoltage(c);
				if (learnKey >= 1.f && prevLearnKey < 1.f) {
					keyToStore = inputs[VO_INPUT].getVoltage(c);
				} else if (learnKey < 1.f && prevLearnKey >= 1.f) {
					learn = false;

					learningButLight = false;
					learnButLightValue = 1.f;

					storedLearn = true; 
					learnPulseTime = lightPulseTime;
					learnExitButStatus = true;
					storedLearnTime = maxStoredLearnTime;

					refKey[currSlot][selectedRange] = int((keyToStore + 6.041666666666667f) * 12);
					
					if (refKey[currSlot][selectedRange] < 0)
						refKey[currSlot][selectedRange] = 0;
					else if (refKey[currSlot][selectedRange] > 127)
						refKey[currSlot][selectedRange] = 127;

					params[RANGE_PARAM].setValue(refKey[currSlot][selectedRange]);

				}
				prevLearnKey = learnKey;
			}

		}

		if (storedLearn) {
			storedLearnTime--;
			if (storedLearnTime < 0) {
				lights[RANGE_SELECTOR_LIGHT].setBrightness(0);
				storedLearn = false;
			} else {
				learnPulseTime--;
				if (learnPulseTime < 0) {
					learnPulseTime = lightPulseTime;
					learnExitButStatus = !learnExitButStatus;
					learnButLightValue = learnExitButStatus;
				}
			}
		}

		lights[RANGE_SELECTOR_LIGHT].setBrightness(learnButLightValue);

		if (selectedRange == LOKEY && refKey[currSlot][LOKEY] > refKey[currSlot][HIKEY]) {
			refKey[currSlot][LOKEY] = refKey[currSlot][HIKEY];
			params[RANGE_PARAM].setValue(refKey[currSlot][HIKEY]);
		} else if (selectedRange == HIKEY && refKey[currSlot][HIKEY] < refKey[currSlot][LOKEY]) {
			refKey[currSlot][HIKEY] = refKey[currSlot][LOKEY];
			params[RANGE_PARAM].setValue(refKey[currSlot][LOKEY]);
		}

		// ************* NAV BUTTONS *******************

		if (!disableNav && !loadFromPatch[currSlot]) {
			nextSample = params[NEXTSAMPLE_PARAM].getValue();
			if (fileLoaded[currSlot] && fileFound[currSlot] && recordingState == 0 && nextSample && !prevNextSample) {
				for (int i = 0; i < 16; i++)
					play[currSlot][i] = false;
				currentFile++;
				if (currentFile >= int(currentFolderV.size()))
					currentFile = 0;
				loadSample(currentFolderV[currentFile], currSlot);
			}
			prevNextSample = nextSample;

			prevSample = params[PREVSAMPLE_PARAM].getValue();
			if (fileLoaded[currSlot] && fileFound[currSlot] && recordingState == 0 && prevSample && !prevPrevSample) {
				for (int i = 0; i < 16; i++)
					play[currSlot][i] = false;
				currentFile--;
				if (currentFile < 0)
					currentFile = currentFolderV.size()-1;
				loadSample(currentFolderV[currentFile], currSlot);
			}
			prevPrevSample = prevSample;
		}


		// ************************		

		monitorSwitch = int(params[MONITOR_SWITCH].getValue());
		lights[MONITOR_LIGHT].setBrightness(monitorSwitch);

	
		velConnected = inputs[VEL_INPUT].isConnected();

		pbConnected = inputs[PB_INPUT].isConnected();
		if (pbConnected) {
			pitchBend = inputs[PB_INPUT].getVoltage();
			//pbValue = pitchBend / 5 / 6;
			//pbValue = pitchBend / 30;
			pbValue = pitchBend / pbRangeTable[pbRange];
		} else {
			pbValue = 0.f;
		}
		
		//trigButValue = params[TESTBUT_PARAM].getValue();
		//lights[TESTBUT_LIGHT].setBrightness(trigButValue);


		// *********************************************************************************************** PLAY SECTION *******************************		

		sumOutput[0] = 0;
		sumOutputR[0] = 0;
		sumOutput[1] = 0;
		sumOutputR[1] = 0;
		sumOutput[2] = 0;
		sumOutputR[2] = 0;
		sumOutput[3] = 0;
		sumOutputR[3] = 0;
		sumOutput[4] = 0;
		sumOutputR[4] = 0;
		sumOutput[5] = 0;
		sumOutputR[5] = 0;
		sumOutput[6] = 0;
		sumOutputR[6] = 0;
		sumOutput[7] = 0;
		sumOutputR[7] = 0;
		currentOutput = 0;
		currentOutputR = 0;

		if (!fileLoaded[currSlot]) {

			for (int c = 0; c < chan; c++) {
				play[currSlot][c] = false;
				fadingType[currSlot][c] = NO_FADE;
				stage[currSlot][c] = STOP_STAGE;
				stageLevel[currSlot][c] = 0;
				env[currSlot][c] = 0;
				voct[currSlot][c] = 0.f;
				prevVoctDiffFloat[currSlot][c] = 11.f;
				outputs[LEFT_OUTPUT+outRoute[currSlot]].setVoltage(0, c);
				outputs[RIGHT_OUTPUT+outRoute[currSlot]].setVoltage(0, c);
			}
			
		} else {
			knobCueEndPos[currSlot] = params[CUEEND_PARAM].getValue();
			if (knobCueEndPos[currSlot] != prevKnobCueEndPos[currSlot]) {
				if (knobCueEndPos[currSlot] < prevKnobCueEndPos[currSlot])
					cueEndPos[currSlot] = floor(totalSamples[currSlot] * knobCueEndPos[currSlot]);
				else
					cueEndPos[currSlot] = ceil(totalSamples[currSlot] * knobCueEndPos[currSlot]);
				prevKnobCueEndPos[currSlot] = knobCueEndPos[currSlot];
				searchingCueEndPhase[currSlot] = true;
				scanCueEndSample[currSlot] = cueEndPos[currSlot];
				if (cueEndPos[currSlot] < cueStartPos[currSlot])
					cueEndPos[currSlot] = cueStartPos[currSlot];
			}
			knobCueStartPos[currSlot] = params[CUESTART_PARAM].getValue();
			if (knobCueStartPos[currSlot] != prevKnobCueStartPos[currSlot]) {
				if (knobCueStartPos[currSlot] < prevKnobCueStartPos[currSlot])
					cueStartPos[currSlot] = floor(totalSamples[currSlot] * knobCueStartPos[currSlot]);
				else
					cueStartPos[currSlot] = ceil(totalSamples[currSlot] * knobCueStartPos[currSlot]);
				prevKnobCueStartPos[currSlot] = knobCueStartPos[currSlot];
				searchingCueStartPhase[currSlot] = true;
				scanCueStartSample[currSlot] = cueStartPos[currSlot];
				if (cueStartPos[currSlot] > cueEndPos[currSlot])
					cueStartPos[currSlot] = cueEndPos[currSlot];
			}
			knobLoopEndPos[currSlot] = params[LOOPEND_PARAM].getValue();
			if (knobLoopEndPos[currSlot] != prevKnobLoopEndPos[currSlot]) {
				if (knobLoopEndPos[currSlot] < prevKnobLoopEndPos[currSlot])
					loopEndPos[currSlot] = floor(totalSamples[currSlot] * knobLoopEndPos[currSlot]);
				else
					loopEndPos[currSlot] = ceil(totalSamples[currSlot] * knobLoopEndPos[currSlot]);
				prevKnobLoopEndPos[currSlot] = knobLoopEndPos[currSlot];
				searchingLoopEndPhase[currSlot] = true;
				scanLoopEndSample[currSlot] = loopEndPos[currSlot];
				if (loopEndPos[currSlot] < loopStartPos[currSlot])
					loopEndPos[currSlot] = loopStartPos[currSlot];
			}
			knobLoopStartPos[currSlot] = params[LOOPSTART_PARAM].getValue();
			if (knobLoopStartPos[currSlot] != prevKnobLoopStartPos[currSlot]) {
				if (knobLoopStartPos[currSlot] < prevKnobLoopStartPos[currSlot])
					loopStartPos[currSlot] = floor(totalSamples[currSlot] * knobLoopStartPos[currSlot]);
				else
					loopStartPos[currSlot] = ceil(totalSamples[currSlot] * knobLoopStartPos[currSlot]);
				prevKnobLoopStartPos[currSlot] = knobLoopStartPos[currSlot];
				searchingLoopStartPhase[currSlot] = true;
				scanLoopStartSample[currSlot] = loopStartPos[currSlot];
				if (loopStartPos[currSlot] > loopEndPos[currSlot])
					loopStartPos[currSlot] = loopEndPos[currSlot];
			}

			
			if (phaseScan[currSlot] && !prevPhaseScan[currSlot]) {
				prevPhaseScan[currSlot] = true;
				searchingCueEndPhase[currSlot] = true;
				searchingCueStartPhase[currSlot] = true;
				searchingLoopEndPhase[currSlot] = true;
				searchingLoopStartPhase[currSlot] = true;
			}
			
			if (phaseScan[currSlot]) {
				float tempKnob;
				if (searchingCueEndPhase[currSlot]) {
					if (playBuffer[currSlot][LEFT][antiAlias][scanCueEndSample[currSlot]-1] <= 0 && playBuffer[currSlot][LEFT][antiAlias][scanCueEndSample[currSlot]] >= 0) {
						cueEndPos[currSlot] = scanCueEndSample[currSlot];
						searchingCueEndPhase[currSlot] = false;
						tempKnob = cueEndPos[currSlot]/totalSamples[currSlot];
						params[CUEEND_PARAM].setValue(tempKnob);
						knobCueEndPos[currSlot] = tempKnob;
					} else {
						scanCueEndSample[currSlot]--;
						if (scanCueEndSample[currSlot] < cueStartPos[currSlot]) {
							cueEndPos[currSlot] = cueStartPos[currSlot];
							searchingCueEndPhase[currSlot] = false;
							tempKnob = cueEndPos[currSlot]/totalSamples[currSlot];
							params[CUEEND_PARAM].setValue(tempKnob);
							knobCueEndPos[currSlot] = tempKnob;

						}
					}

				}
				if (searchingCueStartPhase[currSlot]) {
					if (playBuffer[currSlot][LEFT][antiAlias][scanCueStartSample[currSlot]+1] >= 0 && playBuffer[currSlot][LEFT][antiAlias][scanCueStartSample[currSlot]] <= 0) {
						cueStartPos[currSlot] = scanCueStartSample[currSlot];
						searchingCueStartPhase[currSlot] = false;
						tempKnob = cueStartPos[currSlot]/totalSamples[currSlot];
						params[CUESTART_PARAM].setValue(tempKnob);
						prevKnobCueStartPos[currSlot] = tempKnob;
					} else {
						scanCueStartSample[currSlot]++;
						if (scanCueStartSample[currSlot] > cueEndPos[currSlot]) {
							cueStartPos[currSlot] = cueEndPos[currSlot];
							searchingCueStartPhase[currSlot] = false;
							tempKnob = cueStartPos[currSlot]/totalSamples[currSlot];
							params[CUESTART_PARAM].setValue(tempKnob);
							prevKnobCueStartPos[currSlot] = tempKnob;
						}
					}
				}
				if (searchingLoopEndPhase[currSlot]) {
					if (playBuffer[currSlot][LEFT][antiAlias][scanLoopEndSample[currSlot]-1] <= 0 && playBuffer[currSlot][LEFT][antiAlias][scanLoopEndSample[currSlot]] >= 0) {
						loopEndPos[currSlot] = scanLoopEndSample[currSlot];
						searchingLoopEndPhase[currSlot] = false;
						tempKnob = loopEndPos[currSlot]/totalSamples[currSlot];
						params[LOOPEND_PARAM].setValue(tempKnob);
						prevKnobLoopEndPos[currSlot] = tempKnob;
					} else {
						scanLoopEndSample[currSlot]--;
						if (scanLoopEndSample[currSlot] < loopStartPos[currSlot]) {
							loopEndPos[currSlot] = loopStartPos[currSlot];
							searchingLoopEndPhase[currSlot] = false;
							tempKnob = loopEndPos[currSlot]/totalSamples[currSlot];
							params[LOOPEND_PARAM].setValue(tempKnob);
							prevKnobLoopEndPos[currSlot] = tempKnob;
						}
					}
				}
				if (searchingLoopStartPhase[currSlot]) {
					if (playBuffer[currSlot][LEFT][antiAlias][scanLoopStartSample[currSlot]+1] >= 0 && playBuffer[currSlot][LEFT][antiAlias][scanLoopStartSample[currSlot]] <= 0) {
						loopStartPos[currSlot] = scanLoopStartSample[currSlot];
						searchingLoopStartPhase[currSlot] = false;
						tempKnob = loopStartPos[currSlot]/totalSamples[currSlot];
						params[LOOPSTART_PARAM].setValue(tempKnob);
						prevKnobLoopStartPos[currSlot] = tempKnob;
					} else {
						scanLoopStartSample[currSlot]++;
						if (scanLoopStartSample[currSlot] > loopEndPos[currSlot]) {
							loopStartPos[currSlot] = loopEndPos[currSlot];
							searchingLoopStartPhase[currSlot] = false;
							tempKnob = loopStartPos[currSlot]/totalSamples[currSlot];
							params[LOOPSTART_PARAM].setValue(tempKnob);
							prevKnobLoopStartPos[currSlot] = tempKnob;
						}
					}
				}
				
			} else {
				prevPhaseScan[currSlot] = false;
			}
			

			//trigMode[currSlot] = params[TRIGGATEMODE_SWITCH].getValue();
			//trigType[currSlot] = params[TRIGMODE_SWITCH].getValue();

			//sustainValue[currSlot] = params[SUSTAIN_PARAM].getValue();

			if (xFade[currSlot] != prevXfade[currSlot]) {
				if (xFade[currSlot] == 0)
					fadeSamples[currSlot] = 0;
				else
					fadeSamples[currSlot] = floor(convertCVToSeconds(xFade[currSlot]) * vcvSampleRate); // number of samples before starting fade

				prevXfade[currSlot] = xFade[currSlot];
			}

		}

			// ********************************************************************* START CHANNEL MANAGEMENT

	if (!learn) {

		voctDisplay = 100.f;

		for (int c = 0; c < chan; c++) {

			vOctKey = inputs[VO_INPUT].getVoltage(c);

			if (vOctKey < -6.039)
				vOctNote = 0;
			else if (vOctKey > 4.624)
				vOctNote = 127;

			vOctNote = floor((vOctKey + 6.041666666666667f) * 12);

			velKey = inputs[VEL_INPUT].getVoltage(c);

			for (int slot = 0; slot < 8; slot++) {

				if (!fileLoaded[slot]) {

						play[slot][c] = false;
						fadingType[slot][c] = NO_FADE;
						stage[slot][c] = STOP_STAGE;
						stageLevel[slot][c] = 0;
						env[slot][c] = 0;
						voct[slot][c] = 0.f;
						prevVoctDiffFloat[slot][c] = 11.f;
					
				} else {

					if (vOctKey >= noteTable[refKey[slot][LOKEY]][1] && vOctKey < noteTable[refKey[slot][HIKEY]][2]) {
					//if (vOctNote >= refKey[slot][LOKEY] && vOctNote <= refKey[slot][HIKEY]) {
						
						masterLevel[slot][c] = volumeValue[slot];
						if (velConnected)
							masterLevel[slot][c] *= velKey * 0.1f;

						//if (c == 0 && slot == currSlot && trigButValue)
						//	trigValue[slot][c] = 1;
						//else
							trigValue[slot][c] = inputs[GATE_INPUT].getVoltage(c);
						
						switch (trigMode[slot]) {
							case GATE_MODE:												// ***** GATE MODE *****
								if (trigValue[slot][c] >= 1) {
									if (!play[slot][c]) {
										play[slot][c] = true;

										grainSampleCount[slot][c] = 0;
										grainCount[slot][c] = 1;
										grainFade[slot][c] = false;

										if (reverseStart[slot]) {
											reversePlaying[slot][c] = REVERSE;
											samplePos[slot][c] = floor(cueEndPos[slot]-1);
											currSampleWeight[slot][c] = sampleCoeff;
										} else {
											reversePlaying[slot][c] = FORWARD;
											samplePos[slot][c] = floor(cueStartPos[slot]+1);
											currSampleWeight[slot][c] = sampleCoeff;
										}
										stage[slot][c] = ATTACK_STAGE;
										refValue[slot][c] = 0;
										deltaValue[slot][c] = 1.f;
										slopeCorr[slot][c] = 1.f;
										stageCoeff[slot][c] = envSrCoeff / attackValue[slot];
									} else {
										if (stage[slot][c] == RELEASE_STAGE) {
											stage[slot][c] = ATTACK_STAGE;
											stageLevel[slot][c] = 0;
											refValue[slot][c] = env[slot][c];
											deltaValue[slot][c] = 1-env[slot][c];
											slopeCorr[slot][c] = 1-env[slot][c];
											stageCoeff[slot][c] = envSrCoeff / attackValue[slot];
										}
									}
								} else {
									if (play[slot][c]) {

										if (stage[slot][c] != RELEASE_STAGE) {
											stage[slot][c] = RELEASE_STAGE;
											//releaseNew[slot][c] = true;

											/*
											if (stageLevel[slot][c] != 0)
												stageCoeff[slot][c] = stageLevel[slot][c] / (vcvSampleRate * releaseValue[slot]);
											else
												stageCoeff[slot][c] = 1;
											*/
											
											stageLevel[slot][c] = 1;
											refValue[slot][c] = 0;
											deltaValue[slot][c] = env[slot][c];
											stageCoeff[slot][c] = envSrCoeff / releaseValue[slot];
										}
									}
								}
							break;

							case TRIG_MODE:												// ***** TRIG MODE *****

								if ((trigValue[slot][c] >= 1 && prevTrigValue[slot][c] < 1)) {
								
									switch (trigType[slot]) {
										case START_STOP:									// trig type: Start/Stop

											if (play[slot][c]) {

												if (stage[slot][c] != RELEASE_STAGE) {

													stage[slot][c] = RELEASE_STAGE;
													//releaseNew[slot][c] = true;

													/*
													if (stageLevel[slot][c] != 0)
														stageCoeff[slot][c] = stageLevel[slot][c] / (vcvSampleRate * releaseValue[slot]);
													else
														stageCoeff[slot][c] = 1;
													*/
													stageLevel[slot][c] = 1;
													refValue[slot][c] = 0;
													deltaValue[slot][c] = env[slot][c];
													stageCoeff[slot][c] = envSrCoeff / releaseValue[slot];

												} else {
													stage[slot][c] = ATTACK_STAGE;
													//stageCoeff[slot][c] = (1-stageLevel[slot][c]) / (vcvSampleRate * attackValue[slot]);
													stageLevel[slot][c] = 0;
													refValue[slot][c] = env[slot][c];
													deltaValue[slot][c] = 1-env[slot][c];
													slopeCorr[slot][c] = 1-env[slot][c];
													stageCoeff[slot][c] = envSrCoeff / attackValue[slot];
												}
												
											} else {
												play[slot][c] = true;

												grainSampleCount[slot][c] = 0;
												grainCount[slot][c] = 1;
												grainFade[slot][c] = false;
												
												if (reverseStart[slot]) {
													reversePlaying[slot][c] = REVERSE;
													samplePos[slot][c] = floor(cueEndPos[slot]-1);
													currSampleWeight[slot][c] = sampleCoeff;
												} else {
													reversePlaying[slot][c] = FORWARD;
													samplePos[slot][c] = floor(cueStartPos[slot]+1);
													currSampleWeight[slot][c] = sampleCoeff;
												}
												stage[slot][c] = ATTACK_STAGE;
												//stageCoeff[slot][c] = (1-stageLevel[slot][c]) / (vcvSampleRate * attackValue[slot]);
												stageLevel[slot][c] = 0;
												refValue[slot][c] = 0;
												deltaValue[slot][c] = 1.f;
												slopeCorr[slot][c] = 1.f;
												stageCoeff[slot][c] = envSrCoeff / attackValue[slot];

											}
										break;

										case START_RESTART:									// trig type: START RESTART
											if (!play[slot][c]) {		// ******* START PLAYBACK
												
												play[slot][c] = true;

												grainSampleCount[slot][c] = 0;
												grainCount[slot][c] = 1;
												grainFade[slot][c] = false;
												
												if (reverseStart[slot]) {
													reversePlaying[slot][c] = REVERSE;
													samplePos[slot][c] = floor(cueEndPos[slot]-1);
													currSampleWeight[slot][c] = sampleCoeff;
												} else {
													reversePlaying[slot][c] = FORWARD;
													samplePos[slot][c] = floor(cueStartPos[slot]+1);
													currSampleWeight[slot][c] = sampleCoeff;
												}
												stage[slot][c] = ATTACK_STAGE;
												//stageCoeff[slot][c] = (1-stageLevel[slot][c]) / (vcvSampleRate * attackValue[slot]);
												stageLevel[slot][c] = 0;
												refValue[slot][c] = 0;
												deltaValue[slot][c] = 1.f;
												slopeCorr[slot][c] = 1.f;
												stageCoeff[slot][c] = envSrCoeff / attackValue[slot];

											} else {	// ******* RESTART PLAYBACK

												grainSampleCount[slot][c] = 0;
												grainCount[slot][c] = 1;
												grainFade[slot][c] = false;

												fadingValue[slot][c] = 1.f;
												fadedPosition[slot][c] = samplePos[slot][c];
												if (fadeSamples[slot])
													fadeCoeff[slot] = 1 / convertCVToSeconds(xFade[slot]) / vcvSampleRate;
												else
													fadeCoeff[slot] = 1;
												fadingType[slot][c] = CROSS_FADE;
												if (reverseStart[slot]) {
													reversePlaying[slot][c] = REVERSE;
													samplePos[slot][c] = floor(cueEndPos[slot]-1);
													currSampleWeight[slot][c] = sampleCoeff;
												} else {
													reversePlaying[slot][c] = FORWARD;
													samplePos[slot][c] = floor(cueStartPos[slot]+1);
													currSampleWeight[slot][c] = sampleCoeff;
												}

												// *********** ATTENZIONE IN CASO DI RESTART VERRA' FATTO UN SEMPLICE CROSSFADE LINEARE PER LA DURATA DI XFADE KNOB
												//stage[slot][c] = ATTACK_STAGE;
												//stageCoeff[slot][c] = (1-stageLevel[slot][c]) / (vcvSampleRate * attackValue[slot]);

												/*
												stageLevel[slot][c] = 0;
												refValue[slot][c] = 0;
												deltaValue[slot][c] = 1.f;
												slopeCorr[slot][c] = 1.f;
												stageCoeff[slot][c] = envSrCoeff / attackValue[slot];
												*/
											}
										break;
										
									}
								}
								prevTrigValue[slot][c] = trigValue[slot][c];

							break;
						}

						currentOutput = 0.f;
						currentOutputR = 0.f;

						if (play[slot][c]) {


							voct[slot][c] = vOctKey;

							if (quantize) {

								vOctDiff = vOctNote - refKey[slot][REFKEY];
								
								if (allowPB[slot] && pbValue != 0) {
									speedVoct[slot][c] = pow(2,noteTable[72+vOctDiff][0] + pbValue);
								} else if (vOctDiff >= 0) {
									speedVoct[slot][c] = semiTonesPos[vOctDiff];
								} else {
									speedVoct[slot][c] = semiTonesNeg[refKey[slot][REFKEY] - vOctNote];
								}

							} else {

								vOctDiffFloat = vOctKey - noteTable[refKey[slot][REFKEY]][0];

								if (allowPB[slot] && pbValue != 0) {
									speedVoct[slot][c] = pow(2,vOctDiffFloat + pbValue);
								} else if (vOctDiffFloat != prevVoctDiffFloat[slot][c]) {
									speedVoct[slot][c] = pow(2,vOctDiffFloat);
									prevVoctDiffFloat[slot][c] = vOctDiffFloat;
								}

							}

							distancePos[slot][c] = speedVoct[slot][c] * sampleCoeff;


							//if (slot == currSlot && play[slot][c] && voct[slot][c] <= voctDisplay) {
							if (slot == currSlot && voct[slot][c] <= voctDisplay) {
								displaySlot = slot;
								currentDisplay = c;
								voctDisplay = voct[slot][c];
							}

							switch (reversePlaying[slot][c]) {
								case FORWARD:		// ********************************************************************************************  FORWARD PLAYBACK   ***********

									if (loop[slot] && samplePos[slot][c] > floor(loopEndPos[slot] - (fadeSamples[slot] * distancePos[slot][c]))) {		// *** REACHED END OF LOOP ***

										grainSampleCount[slot][c] = 0;
										grainCount[slot][c] = 1;
										grainFade[slot][c] = false;

										fadingValue[slot][c] = 1.f;
										fadedPosition[slot][c] = samplePos[slot][c];
										if (fadeSamples[slot])
											fadeCoeff[slot] = 1 / convertCVToSeconds(xFade[slot]) / vcvSampleRate;
										else
											fadeCoeff[slot] = 1;

										if (pingpong[slot]) {
											fadingType[slot][c] = PINGPONG_FADE;
											reversePlaying[slot][c] = REVERSE;
											samplePos[slot][c] = floor(loopEndPos[slot])-1;
											currSampleWeight[slot][c] = sampleCoeff;
										} else {
											fadingType[slot][c] = CROSS_FADE;
											samplePos[slot][c] = floor(loopStartPos[slot])+1;
											currSampleWeight[slot][c] = sampleCoeff;
										}

									} else if (!fadingType[slot][c] && floor(samplePos[slot][c]) > (totalSamples[slot] - (fadeSamples[slot] * distancePos[slot][c]))) {

										fadingType[slot][c] = FADE_OUT;
										fadingValue[slot][c] = 1.f;
										fadedPosition[slot][c] = samplePos[slot][c];
										if (fadeSamples[slot])
											fadeCoeff[slot] = 1 / convertCVToSeconds(xFade[slot]) / vcvSampleRate;
										else
											fadeCoeff[slot] = 1;

									} else if (floor(samplePos[slot][c]) > totalSamples[slot]) {	// *** REACHED END OF SAMPLE ***

										play[slot][c] = false;

									} else if (samplePos[slot][c] > cueEndPos[slot]) {				// *** REACHED CUE END ***
										
										if (stage[slot][c] != RELEASE_STAGE) {
											stage[slot][c] = RELEASE_STAGE;
											//releaseNew[slot][c] = true;

											/*
											if (stageLevel[slot][c] != 0)
												stageCoeff[slot][c] = stageLevel[slot][c] / (vcvSampleRate * releaseValue[slot]);
											else
												stageCoeff[slot][c] = 1;
											*/

										}
										if (trigMode[slot] == GATE_MODE) {
											if (pingpong[slot]) {
												grainSampleCount[slot][c] = 0;
												grainCount[slot][c] = 1;
												grainFade[slot][c] = false;

												reversePlaying[slot][c] = REVERSE;
												samplePos[slot][c] = floor(cueEndPos[slot])-1;
												currSampleWeight[slot][c] = sampleCoeff;
											} else {
												grainSampleCount[slot][c] = 0;
												grainCount[slot][c] = 1;
												grainFade[slot][c] = false;

												fadingType[slot][c] = CROSS_FADE;
												fadingValue[slot][c] = 1.f;
												fadedPosition[slot][c] = samplePos[slot][c];
												samplePos[slot][c] = floor(cueStartPos[slot])+1;
												currSampleWeight[slot][c] = sampleCoeff;
											}
										}
										
									} 
								break;

								case REVERSE:		// ********************************************************************************************  REVERSE PLAYBACK   ***********

									if (loop[slot] && samplePos[slot][c] < floor(loopStartPos[slot] + (fadeSamples[slot] * distancePos[slot][c]))) {	// *** REACHED BEGIN OF LOOP ***

										fadingValue[slot][c] = 1.f;
										fadedPosition[slot][c] = samplePos[slot][c];
										if (fadeSamples[slot])
											fadeCoeff[slot] = 1 / convertCVToSeconds(xFade[slot]) / vcvSampleRate;
										else
											fadeCoeff[slot] = 1;

										if (pingpong[slot]) {
											fadingType[slot][c] = PINGPONG_FADE;
											reversePlaying[slot][c] = FORWARD;
											
											grainSampleCount[slot][c] = 0;
											grainCount[slot][c] = 1;
											grainFade[slot][c] = false;

											samplePos[slot][c] = floor(loopStartPos[slot])+1;
											currSampleWeight[slot][c] = sampleCoeff;
										} else {
											fadingType[slot][c] = CROSS_FADE;

											grainSampleCount[slot][c] = 0;
											grainCount[slot][c] = 1;
											grainFade[slot][c] = false;

											samplePos[slot][c] = floor(loopEndPos[slot])-1;
											currSampleWeight[slot][c] = sampleCoeff;
										}

									} else if (!fadingType[slot][c] && floor(samplePos[slot][c]) < (fadeSamples[slot] * distancePos[slot][c])) {

										fadingType[slot][c] = FADE_OUT;
										fadingValue[slot][c] = 1.f;
										fadedPosition[slot][c] = samplePos[slot][c];
										if (fadeSamples[slot])
											fadeCoeff[slot] = 1 / convertCVToSeconds(xFade[slot]) / vcvSampleRate;
										else
											fadeCoeff[slot] = 1;

									} else if (floor(samplePos[slot][c]) < 0) {				// *** REACHED START OF SAMPLE ***

										play[slot][c] = false;

									} else if (samplePos[slot][c] < cueStartPos[slot]) {			// *** REACHED CUE START ***
										
										if (stage[slot][c] != RELEASE_STAGE) {
											stage[slot][c] = RELEASE_STAGE;
											//releaseNew[slot][c] = true;

											/*
											if (stageLevel[slot][c] != 0)
												stageCoeff[slot][c] = stageLevel[slot][c] / (vcvSampleRate * releaseValue[slot]);
											else
												stageCoeff[slot][c] = 1;
											*/

										}
										if (trigMode[slot] == GATE_MODE) {
											if (pingpong[slot]) {
												reversePlaying[slot][c] = FORWARD;

												grainSampleCount[slot][c] = 0;
												grainCount[slot][c] = 1;
												grainFade[slot][c] = false;

												samplePos[slot][c] = floor(cueStartPos[slot])+1;
												currSampleWeight[slot][c] = sampleCoeff;
											} else {
												fadingType[slot][c] = CROSS_FADE;
												fadingValue[slot][c] = 1.f;
												fadedPosition[slot][c] = samplePos[slot][c];

												grainSampleCount[slot][c] = 0;
												grainCount[slot][c] = 1;
												grainFade[slot][c] = false;

												samplePos[slot][c] = floor(cueEndPos[slot])-1;
												currSampleWeight[slot][c] = sampleCoeff;
											}
										}
									}
								
								break;
							}

							if (play[slot][c]) {									// it's false only if end of sample has reached, see above

								// *** SICKOSAMPLER USES HERMITE INTERPOLATION ONLY ***
								if (currSampleWeight[slot][c] == 0) {	// if no distance between samples, it means that speed is 1 and samplerates match -> no interpolation
									
									currentOutput = playBuffer[slot][LEFT][antiAlias][floor(samplePos[slot][c])];
									if (channels[slot] == 2)
										currentOutputR = playBuffer[slot][RIGHT][antiAlias][floor(samplePos[slot][c])];
									
								} else {
									if (floor(samplePos[slot][c]) > 0 && floor(samplePos[slot][c]) < totalSamples[slot] - 1) {

										// below is translation of the above function
										double a1 = .5F * (playBuffer[slot][LEFT][antiAlias][floor(samplePos[slot][c])+1] - playBuffer[slot][LEFT][antiAlias][floor(samplePos[slot][c])-1]);
										double a2 = playBuffer[slot][LEFT][antiAlias][floor(samplePos[slot][c])-1] - (2.5F * playBuffer[slot][LEFT][antiAlias][floor(samplePos[slot][c])]) + (2 * playBuffer[slot][LEFT][antiAlias][floor(samplePos[slot][c])+1]) - (.5F * playBuffer[slot][LEFT][antiAlias][floor(samplePos[slot][c])+2]);
										double a3 = (.5F * (playBuffer[slot][LEFT][antiAlias][floor(samplePos[slot][c])+2] - playBuffer[slot][LEFT][antiAlias][floor(samplePos[slot][c])-1])) + (1.5F * (playBuffer[slot][LEFT][antiAlias][floor(samplePos[slot][c])] - playBuffer[slot][LEFT][antiAlias][floor(samplePos[slot][c])+1]));
										currentOutput = (((((a3 * currSampleWeight[slot][c]) + a2) * currSampleWeight[slot][c]) + a1) * currSampleWeight[slot][c]) + playBuffer[slot][LEFT][antiAlias][floor(samplePos[slot][c])];
										if (channels[slot] == 2) {
											a1 = .5F * (playBuffer[slot][RIGHT][antiAlias][floor(samplePos[slot][c])+1] - playBuffer[slot][RIGHT][antiAlias][floor(samplePos[slot][c])-1]);
											a2 = playBuffer[slot][RIGHT][antiAlias][floor(samplePos[slot][c])-1] - (2.5F * playBuffer[slot][RIGHT][antiAlias][floor(samplePos[slot][c])]) + (2 * playBuffer[slot][RIGHT][antiAlias][floor(samplePos[slot][c])+1]) - (.5F * playBuffer[slot][RIGHT][antiAlias][floor(samplePos[slot][c])+2]);
											a3 = (.5F * (playBuffer[slot][RIGHT][antiAlias][floor(samplePos[slot][c])+2] - playBuffer[slot][RIGHT][antiAlias][floor(samplePos[slot][c])-1])) + (1.5F * (playBuffer[slot][RIGHT][antiAlias][floor(samplePos[slot][c])] - playBuffer[slot][RIGHT][antiAlias][floor(samplePos[slot][c])+1]));
											currentOutputR = (((((a3 * currSampleWeight[slot][c]) + a2) * currSampleWeight[slot][c]) + a1) * currSampleWeight[slot][c]) + playBuffer[slot][RIGHT][antiAlias][floor(samplePos[slot][c])];
										}

									} else { // if playing sample is the first or one of the last 3 -> no interpolation

										if (floor(samplePos[slot][c]) >= 0 && floor(samplePos[slot][c]) < totalSampleC[slot]) {
											currentOutput = playBuffer[slot][LEFT][antiAlias][floor(samplePos[slot][c])];
											if (channels[slot] == 2)
												currentOutputR = playBuffer[slot][RIGHT][antiAlias][floor(samplePos[slot][c])];	
										}
									}
								}

								
								// ******************************************************** G R A I N    F A D E ***********************************************
								
								if (grainFade[slot][c]) {
									grainFadeValue[slot][c] -= grainFadeCoeff[slot][c];
									if (grainFadeValue[slot][c] < 0) {
										grainFade[slot][c] = false;
									} else {
										float tempGrainFadeOutput;

										if (floor(grainPos[slot][c]) > 0 && floor(grainPos[slot][c]) < totalSamples[slot] - 1) {

											tempGrainFadeOutput = hermiteInterpol(playBuffer[slot][LEFT][antiAlias][floor(grainPos[slot][c])-1],
																		playBuffer[slot][LEFT][antiAlias][floor(grainPos[slot][c])],
																		playBuffer[slot][LEFT][antiAlias][floor(grainPos[slot][c])+1],
																		playBuffer[slot][LEFT][antiAlias][floor(grainPos[slot][c])+2],
																		currSampleWeight[slot][c]);
											currentOutput = (currentOutput * (1-grainFadeValue[slot][c])) + (tempGrainFadeOutput * grainFadeValue[slot][c]);
											if (channels[slot] == 2) {
												tempGrainFadeOutput = hermiteInterpol(playBuffer[slot][RIGHT][antiAlias][floor(grainPos[slot][c])-1],
																		playBuffer[slot][RIGHT][antiAlias][floor(grainPos[slot][c])],
																		playBuffer[slot][RIGHT][antiAlias][floor(grainPos[slot][c])+1],
																		playBuffer[slot][RIGHT][antiAlias][floor(grainPos[slot][c])+2],
																		currSampleWeight[slot][c]);
												currentOutputR = (currentOutputR * (1-grainFadeValue[slot][c])) + (tempGrainFadeOutput * grainFadeValue[slot][c]);
											}

										} else { // if playing sample is the first or one of the last 3 -> no interpolation

											if (floor(grainPos[slot][c]) >= 0 && floor(grainPos[slot][c]) < totalSampleC[slot]) {
												tempGrainFadeOutput = playBuffer[slot][LEFT][antiAlias][floor(grainPos[slot][c])];
												currentOutput = (currentOutput * (1-grainFadeValue[slot][c])) + (tempGrainFadeOutput * grainFadeValue[slot][c]);
												if (channels[slot] == 2) {
													tempGrainFadeOutput = playBuffer[slot][RIGHT][antiAlias][floor(grainPos[slot][c])];
													currentOutputR = (currentOutputR * (1-grainFadeValue[slot][c])) + (tempGrainFadeOutput * grainFadeValue[slot][c]);
												}
											}
										}

										if (!reversePlaying[slot][c])
											grainPos[slot][c] += distancePos[slot][c];
										else
											grainPos[slot][c] -= distancePos[slot][c];
									}
								}
								
								// --------------------- A D S R

								switch (stage[slot][c]) {
									case ATTACK_STAGE:
										stageLevel[slot][c] += stageCoeff[slot][c] / slopeCorr[slot][c];

										if (stageLevel[slot][c] >= 1) {
											stageLevel[slot][c] = 1;
											refValue[slot][c] = sustainValue[slot];
											deltaValue[slot][c] = 1.f - sustainValue[slot];

											stage[slot][c] = DECAY_STAGE;
											//stageCoeff[slot][c] = (1-sustainValue[slot]) / (vcvSampleRate * decayValue[slot]);
											//stageCoeff[slot][c] = 1 / (vcvSampleRate * decayValue[slot]);
											stageCoeff[slot][c] = envSrCoeff / decayValue[slot];

										}
										env[slot][c] = refValue[slot][c] + (shapeResponse(stageLevel[slot][c], slot) * deltaValue[slot][c]);
									break;

									case DECAY_STAGE:
										stageLevel[slot][c] -= stageCoeff[slot][c];
										
										//if (stageLevel[slot][c] <= sustainValue[slot]) {
										if (stageLevel[slot][c] <= 0) {
											//stageLevel[slot][c] = sustainValue[slot];
											stageLevel[slot][c] = 1;
											refValue[slot][c] = sustainValue[slot];
											deltaValue[slot][c] = 0;
											stage[slot][c] = SUSTAIN_STAGE;
										}
										env[slot][c] = refValue[slot][c] + (shapeResponse2(stageLevel[slot][c], slot) * deltaValue[slot][c]);
									break;

									case SUSTAIN_STAGE:
										//stageLevel[slot][c] = sustainValue[slot];
										refValue[slot][c] = sustainValue[slot];
										env[slot][c] = sustainValue[slot];
									break;

									case RELEASE_STAGE:
										stageLevel[slot][c] -= stageCoeff[slot][c];
										//env[slot][c] = shapeResponse2(stageLevel[slot][c], slot);
										
										if (stageLevel[slot][c] <= 0) {	// if release has ended
											stageLevel[slot][c] = 0;
											refValue[slot][c] = 0;
											deltaValue[slot][c] = 1;

											stage[slot][c] = STOP_STAGE;
											play[slot][c] = false;
											
											//if (releaseNew[slot][c])
											//	releaseNew[slot][c] = false;										

										}
										env[slot][c] = refValue[slot][c] + (shapeResponse2(stageLevel[slot][c], slot) * deltaValue[slot][c]);
									break;
								}

								if (reversePlaying[slot][c]) {
									//currentOutput *= stageLevel[slot][c] * masterLevel[slot][c] * -1;
									currentOutput *= env[slot][c] * masterLevel[slot][c] * -1;
									if (channels[slot] == 2)
										//currentOutputR *= stageLevel[slot][c] * masterLevel[slot][c] * -1;
										currentOutputR *= env[slot][c] * masterLevel[slot][c] * -1;
								} else {
									//currentOutput *= stageLevel[slot][c] * masterLevel[slot][c];
									currentOutput *= env[slot][c] * masterLevel[slot][c];
									if (channels[slot] == 2)
										//currentOutputR *= stageLevel[slot][c] * masterLevel[slot][c];
										currentOutputR *= env[slot][c] * masterLevel[slot][c];
								}


		//																													███████╗░█████╗░██████╗░███████╗
		//																													██╔════╝██╔══██╗██╔══██╗██╔════╝
		//																													█████╗░░███████║██║░░██║█████╗░░
		//																													██╔══╝░░██╔══██║██║░░██║██╔══╝░░
		//																													██║░░░░░██║░░██║██████╔╝███████╗
		//																													╚═╝░░░░░╚═╝░░╚═╝╚═════╝░╚══════╝

								
								switch (fadingType[slot][c]) {
									case NO_FADE:
									break;

									case CROSS_FADE:
										if (fadingValue[slot][c] > 0) {
											fadingValue[slot][c] -= fadeCoeff[slot];
											switch (reversePlaying[slot][c]) {
												case FORWARD:
													
													if (floor(fadedPosition[slot][c]) < totalSampleC[slot]) {
														currentOutput *= 1 - fadingValue[slot][c];
														//currentOutput += (playBuffer[slot][LEFT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * stageLevel[slot][c]);
														//currentOutput += (playBuffer[slot][LEFT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c]);
														currentOutput += (playBuffer[slot][LEFT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * env[slot][c]);
														if (channels[slot] == 2) {
															currentOutputR *= 1 - fadingValue[slot][c];
															//currentOutputR += (playBuffer[slot][RIGHT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * stageLevel[slot][c]);
															currentOutputR += (playBuffer[slot][RIGHT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * env[slot][c]);
														}
													} else {
														fadingType[slot][c] = NO_FADE;
													}
													
													fadedPosition[slot][c] += distancePos[slot][c];
												break;
												case REVERSE:
													
													if (floor(fadedPosition[slot][c]) >= 0) {
														currentOutput *= 1 - fadingValue[slot][c];
														//currentOutput += (playBuffer[slot][LEFT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * stageLevel[slot][c] * -1);
														currentOutput += (playBuffer[slot][LEFT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * env[slot][c] * -1);
														if (channels[slot] == 2) {
															currentOutputR *= 1 - fadingValue[slot][c];
															//currentOutputR += (playBuffer[slot][RIGHT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * stageLevel[slot][c] * -1);
															currentOutputR += (playBuffer[slot][RIGHT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * env[slot][c] * -1);
														}
													} else {
														fadingType[slot][c] = NO_FADE;
													}
													
													fadedPosition[slot][c] -= distancePos[slot][c];
													
												break;
											}
										} else {
											fadingType[slot][c] = NO_FADE;
										}
									break;
									
									case FADE_OUT:
										if (fadingValue[slot][c] > 0) {
											fadingValue[slot][c] -= fadeCoeff[slot];
											currentOutput *= fadingValue[slot][c];
											if (channels[slot] == 2)
												currentOutputR *= 1 - fadingValue[slot][c];
										} else
											fadingType[slot][c] = NO_FADE;
									break;

									case PINGPONG_FADE:
										if (fadingValue[slot][c] > 0) {
											fadingValue[slot][c] -= fadeCoeff[slot];
											switch (reversePlaying[slot][c]) {
												case FORWARD:
													
													if (fadedPosition[slot][c] >= 0) {
														currentOutput *= 1 - fadingValue[slot][c];
														//currentOutput += (playBuffer[slot][LEFT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * stageLevel[slot][c] * -1);
														currentOutput += (playBuffer[slot][LEFT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * env[slot][c] * -1);
														if (channels[slot] == 2) {
															currentOutputR *= 1 - fadingValue[slot][c];
															//currentOutputR += (playBuffer[slot][RIGHT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * stageLevel[slot][c] * -1);
															currentOutputR += (playBuffer[slot][RIGHT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * env[slot][c] * -1);
														}
													} else {
														fadingType[slot][c] = NO_FADE;
													}
													
													fadedPosition[slot][c] -= distancePos[slot][c];
												break;
												case REVERSE:
													
													if (fadedPosition[slot][c] < totalSampleC[slot]) {
														currentOutput *= 1 - fadingValue[slot][c];
														//currentOutput += (playBuffer[slot][LEFT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * stageLevel[slot][c]);
														currentOutput += (playBuffer[slot][LEFT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * env[slot][c]);
														if (channels[slot] == 2) {
															currentOutputR *= 1 - fadingValue[slot][c];
															//currentOutputR += (playBuffer[slot][RIGHT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * stageLevel[slot][c]);
															currentOutputR += (playBuffer[slot][RIGHT][antiAlias][floor(fadedPosition[slot][c])] * fadingValue[slot][c] * masterLevel[slot][c] * env[slot][c]);
														}
													} else {
														fadingType[slot][c] = NO_FADE;
													}
													
													fadedPosition[slot][c] += distancePos[slot][c];
												break;
											}
										} else
											fadingType[slot][c] = NO_FADE;
									break;
								}

								// ------------------------------------------------------- U P D A T E    S A M P L E   P O S I T I O N -----------------
								
								//if (params[STRETCH_PARAM].getValue() != 1) {
								if (stretchKnob[slot] != 1) {
									grainSampleCount[slot][c]++;
									//if (params[STRETCH_PARAM].getValue() > 1) {
									if (stretchKnob[slot] > 1) {

										if (grainSampleCount[slot][c] > stretchMaxPos[slot] ) {		//
											grainCount[slot][c]++;
											grainPos[slot][c] = samplePos[slot][c];
											grainFadeValue[slot][c] = 1;

											//grainFadeCoeff[c] = 10 / (params[STR_SIZE_PARAM].getValue() * oneMsSamples);
											grainFadeCoeff[slot][c] = 10 / (cycleKnob[slot] * oneMsSamples);
											grainFade[slot][c] = true;

											//if (grainCount[c] > (params[STRETCH_PARAM].getValue())) {
											if (grainCount[slot][c] > stretchKnob[slot]) {
												//double tempStretch1 = params[STRETCH_PARAM].getValue()-floor(params[STRETCH_PARAM].getValue());
												double tempStretch1 = stretchKnob[slot]-floor(stretchKnob[slot]);
												if (!reversePlaying[slot][c])
													samplePos[slot][c] -= stretchMaxPos[slot] * tempStretch1 * distancePos[slot][c];
												else
													samplePos[slot][c] += stretchMaxPos[slot] * tempStretch1 * distancePos[slot][c];
												grainSampleCount[slot][c] = -stretchMaxPos[slot] * tempStretch1;
												grainCount[slot][c] = 1;
											} else {
												if (!reversePlaying[slot][c])
													samplePos[slot][c] -= stretchMaxPos[slot] * distancePos[slot][c];
												else
													samplePos[slot][c] += stretchMaxPos[slot] * distancePos[slot][c];
												grainSampleCount[slot][c] = grainSampleCount[slot][c] - stretchMaxPos[slot];
											}
										}

									} else {

										if (grainSampleCount[slot][c] > stretchMaxPos[slot]) {
											
											grainPos[slot][c] = samplePos[slot][c];
											grainFadeValue[slot][c] = 1;
											
											//grainFadeCoeff[c] = 10 / (params[STR_SIZE_PARAM].getValue() * oneMsSamples);
											grainFadeCoeff[slot][c] = 10 / (cycleKnob[slot] * oneMsSamples);
											grainFade[slot][c] = true;
											
											if (!reversePlaying[slot][c])
												//samplePos[c] += ((stretchMaxPos / params[STRETCH_PARAM].getValue()) - stretchMaxPos) * distancePos[c];
												samplePos[slot][c] += ((stretchMaxPos[slot] / stretchKnob[slot]) - stretchMaxPos[slot]) * distancePos[slot][c];
											else
												//samplePos[c] -= ((stretchMaxPos / params[STRETCH_PARAM].getValue()) - stretchMaxPos) * distancePos[c];
												samplePos[slot][c] -= ((stretchMaxPos[slot] / stretchKnob[slot]) - stretchMaxPos[slot]) * distancePos[slot][c];

											grainSampleCount[slot][c] = grainSampleCount[slot][c] - stretchMaxPos[slot];
										}
									}
								}
								
								if (reversePlaying[slot][c])
									samplePos[slot][c] -= distancePos[slot][c];
								else
									samplePos[slot][c] += distancePos[slot][c];

								currSampleWeight[slot][c] = samplePos[slot][c] - floor(samplePos[slot][c]);

								// ------------------------------------------------------------ END POSITION UPDATE

							}

						} else {
							play[slot][c] = false;
							fadingType[slot][c] = NO_FADE;
						}

						sumOutput[outRoute[slot]] += currentOutput;
						if (channels[slot] == 2)
							sumOutputR[outRoute[slot]] += currentOutputR;
					
					}	// END OF IF  IN RANGE

				}	// END OF IF fileLoaded

			}	// END OF SLOT CYCLE

		} // END OF CHANNEL MANAGEMENT


//
//																							██████╗░███████╗░█████╗░░█████╗░██████╗░██████╗░
//																							██╔══██╗██╔════╝██╔══██╗██╔══██╗██╔══██╗██╔══██╗
//																							██████╔╝█████╗░░██║░░╚═╝██║░░██║██████╔╝██║░░██║
//																							██╔══██╗██╔══╝░░██║░░██╗██║░░██║██╔══██╗██║░░██║
//																							██║░░██║███████╗╚█████╔╝╚█████╔╝██║░░██║██████╔╝
//																							╚═╝░░╚═╝╚══════╝░╚════╝░░╚════╝░╚═╝░░╚═╝╚═════╝░


		recTrig = params[REC_PARAM].getValue();

		if (recTrig && !prevRecTrig) {
			if (!fileLoaded[currSlot]) {
				if (recordingState) {
					recButton = 0;
				} else {
					recButton = 1;
					recSlot = currSlot;
				}
				lights[REC_LIGHT].setBrightness(recButton);
			} else {
				lights[REC_LIGHT].setBrightness(0.f);
			}

		}
		prevRecTrig = recTrig;

		//if (!fileLoaded[currSlot]) {
		if (!fileLoaded[recSlot]) {
			// ********* NEW RECORDING **** START STOP COMMANDS

			//masterLevel[currSlot][recOutChan] = params[VOL_PARAM].getValue() + (inputs[VEL_INPUT].getVoltage(recOutChan) * 0.1);
			masterLevel[recSlot][recOutChan] = volumeValue[recSlot];

			/*
			if (masterLevel[currSlot][recOutChan] > 1)
				masterLevel[currSlot][recOutChan] = 1;
			else if (masterLevel[currSlot][recOutChan] < 0)
				masterLevel[currSlot][recOutChan] = 0;
			*/
			
			if (recButton && recordingState == 0) {

				if (inputs[IN_INPUT].isConnected()){
					z1 = 0; z2 = 0; z1r = 0; z2r = 0;
					recordingState = 1;
					recSamples = 0;
					if (inputs[IN_INPUT+1].isConnected())
						recChannels = 2;
					else
						recChannels = 1;

					currentRecordingLimit = recordingLimit / recChannels;

					fileChannels[recSlot] = recChannels;

					sampleRate = vcvSampleRate * 2;
					calcBiquadLpf(20000.0, sampleRate, 1);

					playBuffer[recSlot][LEFT][0].clear();
					playBuffer[recSlot][LEFT][1].clear();
					if (recChannels == 2) {
						playBuffer[recSlot][RIGHT][0].clear();
						playBuffer[recSlot][RIGHT][1].clear();
					}

					// metamodule change
					vector<float>().swap(playBuffer[recSlot][LEFT][0]);
					vector<float>().swap(playBuffer[recSlot][RIGHT][0]);
					vector<float>().swap(playBuffer[recSlot][LEFT][1]);
					vector<float>().swap(playBuffer[recSlot][RIGHT][1]);

					currRecPosition = 1;

					// set position knob to 0 and 1
					knobCueStartPos[recSlot] = 0;
					knobLoopStartPos[recSlot] = 0;
					knobCueEndPos[recSlot] = 1;
					knobLoopEndPos[recSlot] = 1;

					params[CUESTART_PARAM].setValue(0);
					params[CUEEND_PARAM].setValue(1);
					params[LOOPSTART_PARAM].setValue(0);
					params[LOOPEND_PARAM].setValue(1);

					if (xFade[recSlot] > 0) {
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

					if (xFade[recSlot] <= 0)
						recFadeValue = 0;
				}

				if (recFadeValue <= 0) {
					recordingState = 0;

					channels[recSlot] = recChannels;
					//fileDescription[recSlot] = "_unknown_.wav";
					//fileDisplay[recSlot] = "_unknown_";
					fileDescription[recSlot] = "_slot"+to_string(recSlot+1)+".waw";
					fileDisplay[recSlot] = "_slot"+to_string(recSlot+1);
					storedPath[recSlot] = "_slot"+to_string(recSlot+1)+".waw";;
					samplerateDisplay[recSlot] = std::to_string(int(vcvSampleRate));
					channelsDisplay[recSlot] = std::to_string(recChannels) + "ch";

					cueStartPos[recSlot] = 0;
					cueEndPos[recSlot] = totalSamples[recSlot];
					loopStartPos[recSlot] = 0;
					loopEndPos[recSlot] = totalSamples[recSlot];
					prevKnobCueStartPos[recSlot] = -1.f;
					prevKnobCueEndPos[recSlot] = 2.f;
					prevKnobLoopStartPos[recSlot] = -1.f;
					prevKnobLoopEndPos[recSlot] = 2.f;

					samplePos[recSlot][recOutChan] = 0;

					totalSampleC[recSlot] = playBuffer[recSlot][0][0].size();
					totalSamples[recSlot] = totalSampleC[recSlot]-1;

					vector<double>().swap(displayBuff[recSlot]);

					for (int i = 0; i < floor(totalSampleC[recSlot]); i = i + floor(totalSampleC[recSlot]/wavDisp))
						displayBuff[recSlot].push_back(playBuffer[recSlot][0][0][i]);

					seconds = totalSampleC[recSlot] * 0.5 / (APP->engine->getSampleRate());
					minutes = (seconds / 60) % 60;
					seconds = seconds % 60;

					timeDisplay[recSlot] = std::to_string(minutes) + ":";
					if (seconds < 10)
						timeDisplay[recSlot] += "0";
					timeDisplay[recSlot] += std::to_string(seconds);

					toSave[recSlot] = true;
					infoToSave[recSlot] ="s";
					fileLoaded[recSlot] = true;

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
					recFadeValue += 1 / (convertCVToSeconds(xFade[recSlot]) * vcvSampleRate);
				if (recFadeValue >= 1) {
					recFadeValue = 1;
					recFadeIn = false;
				}

			} else if (recFadeOut) {

				if (recFadeValue > 0)
					recFadeValue -= 1 / (convertCVToSeconds(xFade[recSlot]) * vcvSampleRate);
				if (recFadeValue <= 0) {
					recFadeValue = 0;
					recFadeOut = false;
				}
			}

	
			playBuffer[recSlot][LEFT][0].push_back(0);
			playBuffer[recSlot][LEFT][1].push_back(0);
			playBuffer[recSlot][LEFT][0].push_back(0);
			playBuffer[recSlot][LEFT][1].push_back(0);
			if (recChannels == 2) {
				playBuffer[recSlot][RIGHT][0].push_back(0);
				playBuffer[recSlot][RIGHT][1].push_back(0);
				playBuffer[recSlot][RIGHT][0].push_back(0);
				playBuffer[recSlot][RIGHT][1].push_back(0);
			}

			int rc = 0;

			prevRecValue[rc] = currRecValue[rc];
			currRecValue[rc] = inputs[IN_INPUT+rc].getVoltage() * params[GAIN_PARAM].getValue() * recFadeValue;

			playBuffer[recSlot][rc][0][currRecPosition] = currRecValue[rc];
			playBuffer[recSlot][rc][0][currRecPosition-1] = (currRecValue[rc] + prevRecValue[rc]) / 2;

			playBuffer[recSlot][rc][1][currRecPosition-1] = biquadLpf(playBuffer[recSlot][rc][0][currRecPosition-1]); // filtered vector
			playBuffer[recSlot][rc][1][currRecPosition] = biquadLpf(playBuffer[recSlot][rc][0][currRecPosition]); // filtered vector

			if (recChannels == 2) {
				rc = 1;

				prevRecValue[rc] = currRecValue[rc];
				currRecValue[rc] = inputs[IN_INPUT+rc].getVoltage() * params[GAIN_PARAM].getValue() * recFadeValue;

				playBuffer[recSlot][rc][0][currRecPosition] = currRecValue[rc];
				playBuffer[recSlot][rc][0][currRecPosition-1] = (currRecValue[rc] + prevRecValue[rc]) / 2;

				playBuffer[recSlot][rc][1][currRecPosition-1] = biquadLpf2(playBuffer[recSlot][rc][0][currRecPosition-1]); // filtered vector
				playBuffer[recSlot][rc][1][currRecPosition] = biquadLpf2(playBuffer[recSlot][rc][0][currRecPosition]); // filtered vector
			}

			recSamples++;
			currRecPosition = currRecPosition + 2;

			if (!unlimitedRecording && currRecPosition > currentRecordingLimit) {
				params[REC_PARAM].setValue(0);
				lights[REC_LIGHT].setBrightness(0);
				recButton = 0;
			}

		}	// end if RECORDING STATE = true

	}
//																						███╗░░░███╗░█████╗░███╗░░██╗██╗████████╗░█████╗░██████╗░
//																						████╗░████║██╔══██╗████╗░██║██║╚══██╔══╝██╔══██╗██╔══██╗
//																						██╔████╔██║██║░░██║██╔██╗██║██║░░░██║░░░██║░░██║██████╔╝
//																						██║╚██╔╝██║██║░░██║██║╚████║██║░░░██║░░░██║░░██║██╔══██╗
//																						██║░╚═╝░██║╚█████╔╝██║░╚███║██║░░░██║░░░╚█████╔╝██║░░██║
//																						╚═╝░░░░░╚═╝░╚════╝░╚═╝░░╚══╝╚═╝░░░╚═╝░░░░╚════╝░╚═╝░░╚═╝

		
		if (!fileLoaded[recSlot] && !recordingState) {
			if (inputs[IN_INPUT+1].isConnected())
				channels[currSlot] = 2;
			else
				channels[currSlot] = 1;
		}

		//monitorSwitch = int(params[MONITOR_SWITCH].getValue());
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

		if (monitorSwitch) {
			sumOutput[outRoute[currSlot]] += inputs[IN_INPUT].getVoltageSum() * params[GAIN_PARAM].getValue() * monitorFadeValue;
			//if (channels[currSlot] == 2) // superfluo, eliminato per velocizzare
				sumOutputR[outRoute[currSlot]] += inputs[IN_INPUT+1].getVoltageSum() * params[GAIN_PARAM].getValue() * monitorFadeValue;
				// n.b. : se lo slot è già registrato mono, verrà monitorato solo il canale sinistro
		} else if (monitorFade) {
			sumOutput[outRoute[currSlot]] += inputs[IN_INPUT].getVoltageSum() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue);
			//if (channels[currSlot] == 2)	// superfluo, eliminato per velocizzare
				sumOutputR[outRoute[currSlot]] += inputs[IN_INPUT+1].getVoltageSum() * params[GAIN_PARAM].getValue() * (1-monitorFadeValue);
				// n.b. : se lo slot è già registrato mono, verrà monitorato solo il canale sinistro
		}

		prevMonitorSwitch = monitorSwitch;


		for (int slot = 0; slot < 8; slot++) {

			// *** HARD CLIP ***

			if (sumOutput[slot] > 10)
				sumOutput[slot] = 10;
			else if (sumOutput[slot] < -10)
				sumOutput[slot] = -10;
			if (channels[slot] == 2) {
				if (sumOutputR[slot] > 10)
					sumOutputR[slot] = 10;
				else if (sumOutputR[slot] < -10)
					sumOutputR[slot] = -10;
			}

			//if (outputs[LEFT_OUTPUT+slot].isConnected()) { // condizione superflua ?
				outputs[LEFT_OUTPUT+slot].setVoltage(sumOutput[slot]);
				//outputs[LEFT_OUTPUT+outRoute[slot]].setChannels(1);		// facoltativo ?
			//}

			if (outputs[RIGHT_OUTPUT+slot].isConnected()) {
				if (channels[slot] == 2)
					outputs[RIGHT_OUTPUT+slot].setVoltage(sumOutputR[slot]);
				else
					outputs[RIGHT_OUTPUT+slot].setVoltage(sumOutput[slot]);
				//outputs[RIGHT_OUTPUT+slot].setChannels(1);	// facoltativo?
			}
		}

		// ******************************************************************************************************
		// *****************************  R E C O R D I N G        E N D  ***************************************
		// ******************************************************************************************************

		/*
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
		*/
	}
};




//															███████╗███╗░░██╗██████╗░  ██████╗░██████╗░░█████╗░░█████╗░███████╗░██████╗░██████╗
//															██╔════╝████╗░██║██╔══██╗  ██╔══██╗██╔══██╗██╔══██╗██╔══██╗██╔════╝██╔════╝██╔════╝
//															█████╗░░██╔██╗██║██║░░██║  ██████╔╝██████╔╝██║░░██║██║░░╚═╝█████╗░░╚█████╗░╚█████╗░
//															██╔══╝░░██║╚████║██║░░██║  ██╔═══╝░██╔══██╗██║░░██║██║░░██╗██╔══╝░░░╚═══██╗░╚═══██╗
//															███████╗██║░╚███║██████╔╝  ██║░░░░░██║░░██║╚█████╔╝╚█████╔╝███████╗██████╔╝██████╔╝
//															╚══════╝╚═╝░░╚══╝╚═════╝░  ╚═╝░░░░░╚═╝░░╚═╝░╚════╝░░╚════╝░╚══════╝╚═════╝░╚═════╝░

/*
struct KeySamplerDebugDisplay : TransparentWidget {
	KeySampler *module;
	int frame = 0;
	KeySamplerDebugDisplay() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-bold.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff)); 

				//module->debugDisplay[0] = to_string(module->fadingType[0][0]) + " " + to_string(module->stretchMaxPos[0]);
				//module->debugDisplay[1] = to_string(module->fadingType[1][1]) + " " + to_string(module->stretchMaxPos[1]);
				module->debugDisplay[0] = "cueStartPos " + to_string(module->cueStartPos[2]) ;
				module->debugDisplay[1] = "cueEndPos " + to_string(module->cueEndPos[2]) ;
				module->debugDisplay[2] = "KN cueStartPos " + to_string(module->knobCueStartPos[2]) ;
				module->debugDisplay[3] = "KN cueEndPos " + to_string(module->knobCueEndPos[2]) ;

				for (int i = 0; i < 16; i++)
					nvgTextBox(args.vg, 0, i*11,120, module->debugDisplay[i].c_str(), NULL);

			}
		}
		Widget::drawLayer(args, layer);
	}
};
*/

struct KeySamplerDisplay : TransparentWidget {
	KeySampler *module;
	int frame = 0;
	KeySamplerDisplay() {
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

				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/vgatrue.ttf"));
				nvgFontSize(args.vg, 12);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
				nvgTextBox(args.vg, xTitle, yInfo,247, module->fileDisplay[module->currSlot].c_str(), NULL);

				if (module->fileLoaded[module->currSlot]) {
					nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff));
					nvgTextBox(args.vg, xTime, yInfo,97, module->timeDisplay[module->currSlot].c_str(), NULL);

				} else if (module->recordingState) {
					module->recSeconds = module->recSamples / (APP->engine->getSampleRate());
					module->recMinutes = (module->recSeconds / 60) % 60;
					module->recSeconds = module->recSeconds % 60;
					module->recTimeDisplay[module->currSlot] = std::to_string(module->recMinutes) + ":";
					if (module->recSeconds < 10)
						module->recTimeDisplay[module->currSlot] += "0";
					module->recTimeDisplay[module->currSlot] += std::to_string(module->recSeconds);
					module->timeDisplay[module->currSlot] = module->recTimeDisplay[module->currSlot];
					nvgTextBox(args.vg, xTime, yInfo,97, module->timeDisplay[module->currSlot].c_str(), NULL);
				}

				std::string loVal = "LO:" + module->noteNames[module->refKey[module->currSlot][LOKEY]];
				std::string hiVal = "HI:" + module->noteNames[module->refKey[module->currSlot][HIKEY]];
				std::string rfVal = "RF:" + module->noteNames[module->refKey[module->currSlot][REFKEY]];

				switch (module->selectedRange) {
					case LOKEY:
						if (module->learn)
							loVal = "LO:---";
						nvgBeginPath(args.vg);
						nvgRect(args.vg, xKeyL, yKey, keyW, keyH);
						nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED));
						nvgFill(args.vg);

						nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_BLACK));
						nvgTextBox(args.vg, xLoVal, yInfo2, 80, loVal.c_str(), NULL);
						nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_GREEN));
						nvgTextBox(args.vg, xHiVal, yInfo2, 80, hiVal.c_str(), NULL);
						nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_BLUE));
						nvgTextBox(args.vg, xRfVal, yInfo2, 80, rfVal.c_str(), NULL);
					break;

					case HIKEY:
						if (module->learn)
							hiVal = "HI:---";
						nvgBeginPath(args.vg);
						nvgRect(args.vg, xKeyH, yKey, keyW, keyH);
						nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_GREEN));
						nvgFill(args.vg);

						nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED));
						nvgTextBox(args.vg, xLoVal, yInfo2, 80, loVal.c_str(), NULL);
						nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_BLACK));
						nvgTextBox(args.vg, xHiVal, yInfo2, 80, hiVal.c_str(), NULL);
						nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_BLUE));
						nvgTextBox(args.vg, xRfVal, yInfo2, 80, rfVal.c_str(), NULL);
					break;

					case REFKEY:
						if (module->learn)
							rfVal = "RF:---";
						nvgBeginPath(args.vg);
						nvgRect(args.vg, xKeyR, yKey, keyW, keyH);
						nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_BLUE));
						nvgFill(args.vg);

						nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED));
						nvgTextBox(args.vg, xLoVal, yInfo2, 80, loVal.c_str(), NULL);
						nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_GREEN));
						nvgTextBox(args.vg, xHiVal, yInfo2, 80, hiVal.c_str(), NULL);
						nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_BLACK));
						nvgTextBox(args.vg, xRfVal, yInfo2, 80, rfVal.c_str(), NULL);
					break;
				}

				nvgFillColor(args.vg, nvgRGBA(0x88, 0xaa, 0xff, 0xff));
				nvgTextBox(args.vg, xMagg, yInfo,20, std::string(">").c_str(), NULL);
				nvgTextBox(args.vg, xCh, yInfo2,97, module->channelsDisplay[module->currSlot].c_str(), NULL);

				nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0x33, 0xff));
				nvgTextBox(args.vg, xSlot, yInfo,20, to_string(module->currSlot+1).c_str(), NULL);

				nvgFillColor(args.vg, nvgRGBA(0xcc, 0xcc, 0x22, 0xff));
				nvgTextBox(args.vg, xRoute, yInfo,20, to_string(module->outRoute[module->currSlot]+1).c_str(), NULL);

				nvgFillColor(args.vg, nvgRGBA(0xee, 0xee, 0x22, 0xff));
				nvgTextBox(args.vg, xToSave, yInfo2,20, module->infoToSave[module->currSlot].c_str(), NULL);

	
				// Zero line
				nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x40));
				{
					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, xWav, yZeroLine);
					nvgLineTo(args.vg, 143, yZeroLine);
					nvgClosePath(args.vg);
				}
				nvgStroke(args.vg);
		
				if (module->fileLoaded[module->currSlot]) {
					int xLine;

					// Playback line
					if (module->recordingState)
						nvgStrokeColor(args.vg, nvgRGBA(0xf5, 0x00, 0xf5, 0xff));
					else
						nvgStrokeColor(args.vg, nvgRGBA(0xf5, 0xf5, 0xf5, 0xff));
					nvgStrokeWidth(args.vg, 0.8);

					if (module->currSlot == module->displaySlot) {
						nvgBeginPath(args.vg);
						xLine = xWav + floor(module->samplePos[module->currSlot][module->currentDisplay] * wavDisp / module->totalSampleC[module->currSlot]);
						nvgMoveTo(args.vg, xLine, yLineUp);
						nvgLineTo(args.vg, xLine, yLineDn);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Cue Start line
					nvgStrokeColor(args.vg, nvgRGBA(0x00, 0xf0, 0x00, 0xf0));
					nvgStrokeWidth(args.vg, 1);
					{
						nvgBeginPath(args.vg);
						xLine = xWav + floor(module->cueStartPos[module->currSlot] * wavDisp / module->totalSampleC[module->currSlot]);
						nvgMoveTo(args.vg, xLine , yLineUp);
						nvgLineTo(args.vg, xLine , yLineDn);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Cue End line
					nvgStrokeColor(args.vg, nvgRGBA(0xf0, 0x00, 0x00, 0xf0));
					nvgStrokeWidth(args.vg, 1);
					{
						nvgBeginPath(args.vg);
						xLine = xWav + floor(module->cueEndPos[module->currSlot] * wavDisp / module->totalSampleC[module->currSlot]);
						nvgMoveTo(args.vg, xLine , yLineUp);
						nvgLineTo(args.vg, xLine , yLineDn);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Loop Start line
					nvgStrokeColor(args.vg, nvgRGBA(0xf8, 0xec, 0x2e, 0xf0));
					nvgStrokeWidth(args.vg, 1);
					{
						nvgBeginPath(args.vg);
						xLine = xWav + floor(module->loopStartPos[module->currSlot] * wavDisp / module->totalSampleC[module->currSlot]);
						nvgMoveTo(args.vg, xLine , yLineUp);
						nvgLineTo(args.vg, xLine , yLineDn);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Loop End line
					nvgStrokeColor(args.vg, nvgRGBA(0xea, 0x79, 0x26, 0xf0));
					nvgStrokeWidth(args.vg, 1);
					{
						nvgBeginPath(args.vg);
						xLine = xWav + floor(module->loopEndPos[module->currSlot] * wavDisp / module->totalSampleC[module->currSlot]);
						nvgMoveTo(args.vg, xLine , yLineUp);
						nvgLineTo(args.vg, xLine , yLineDn);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);

					// Waveform
					nvgStrokeColor(args.vg, nvgRGBA(0x22, 0x44, 0xc9, 0xc0));
					nvgSave(args.vg);
					Rect b = Rect(Vec(xWav, 13), Vec(wavDisp, 28));
					nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
					nvgBeginPath(args.vg);
					for (unsigned int i = 0; i < module->displayBuff[module->currSlot].size(); i++) {
						float x, y;
						x = (float)i / (module->displayBuff[module->currSlot].size() - 1);
						y = module->displayBuff[module->currSlot][i] / 10.0 + 0.5;
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
		KeySampler *module = dynamic_cast<KeySampler*>(this->module);
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
					menu->addChild(createMenuItem(module->folderTreeDisplay[tempIndex][i], "", [=]() {module->loadFromPatch[module->currSlot] = false;module->loadSample(module->folderTreeData[tempIndex][i], module->currSlot);}));
				}
			}
		}
	}

	void createContextMenu() {
		KeySampler *module = dynamic_cast<KeySampler *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			menu->addChild(createMenuItem("Load Sample", "", [=]() {
				//module->menuLoadSample();
				bool temploadFromPatch = module->loadFromPatch[module->currSlot];
				module->loadFromPatch[module->currSlot] = false;
				module->menuLoadSample(module->currSlot);
				if (module->restoreLoadFromPatch[module->currSlot])
					module->loadFromPatch[module->currSlot] = temploadFromPatch;
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
							menu->addChild(createMenuItem(module->folderTreeDisplay[0][i], "", [=]() {module->loadFromPatch[module->currSlot] = false;module->loadSample(module->folderTreeData[0][i], module->currSlot);}));
						}
					}
				}));
			}

			if (module->fileLoaded[module->currSlot]) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription[module->currSlot]));
				menu->addChild(createMenuLabel(" " + module->samplerateDisplay[module->currSlot] + " - " + std::to_string(module->channels[module->currSlot]) + "ch"));
				std::string tempDisplay = " ";
				if (module->resampled[module->currSlot]) {
					tempDisplay += "resampled to " + std::to_string(int(APP->engine->getSampleRate()));
					if (module->toSave[module->currSlot])
						tempDisplay += " - ";
				}
				if (module->toSave[module->currSlot])
					tempDisplay += "to SAVE";
				if (tempDisplay != " ")
					menu->addChild(createMenuLabel(tempDisplay));

				menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot(module->currSlot);}));

				menu->addChild(createMenuItem("Save FULL Sample", "", [=]() {module->menuSaveSample(SAVE_FULL, module->currSlot);}));
				menu->addChild(createMenuItem("Save CUE Region", "", [=]() {module->menuSaveSample(SAVE_CUE, module->currSlot);}));
				menu->addChild(createMenuItem("Save LOOP Region", "", [=]() {module->menuSaveSample(SAVE_LOOP, module->currSlot);}));

				/*
				menu->addChild(createBoolPtrMenuItem("Trim Sample after Save", "", &module->trimOnSave));
				menu->addChild(createBoolPtrMenuItem("Save Oversampled", "", &module->saveOversampled));
				*/
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

struct KeySamplerWidget : ModuleWidget {
	KeySamplerWidget(KeySampler *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/KeySampler.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		{
			KeySamplerDisplay *display = new KeySamplerDisplay();
			display->box.pos = mm2px(Vec(0.6, 23.5));
			display->box.size = mm2px(Vec(49.5, 17.9));
			display->module = module;
			addChild(display);
		}

		/*
		{
			KeySamplerDebugDisplay *display = new KeySamplerDebugDisplay();
			display->box.pos = Vec(209, 30);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		} 
		*/		

		const float xInL = 6.4f;
		const float xInR = 15.4f;
		const float xGain = 25.9f;
		const float yGain = 17.1f;
		const float xRecBut = 36.4f;
		const float xMon = 45.4f;
		const float yMon = 17.5f;
		const float yInputs = 16.9f;

		const float xSampSelect = 63.1f;
		const float ySampSelect = 14.7f;
		//const float xTrigBut = 66.f;
		//const float yTrigBut = 18.4f;

		const float xRngBt = 56.f;
		const float yRngBt = 25.7f;

		const float xRngKn = 65.2f;
		const float yRngKn = 27.5f;

		const float xOutKn = 56.7f;
		const float yOutKn = 35.f;

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

		const float xTrigGate = 9.68f;
		const float yTrigGate = 77.6f;

		const float xTrigMode = 25.2f;
		const float yTrigMode = 77.9f;

		const float xPb = 34.1f;
		const float yPb = 77.9f;

		const float xScan = 45.4f;
		const float yScan = 77.9f;

		const float xEnv1 = 7.2f;
		const float xEnv1Add = 9.f;
		const float yEnv1 = 89.7f;

		const float xCurve = 44.2f;
		
		const float xStretch = 9.7f;
		const float yStretch = 103.f;

		const float xStretchSiz = 23.f;
		const float yStretchSiz = 103.2f;

		const float xVol = 43.1f;
		const float yVol = 101.8f;

		const float xGateIn = 7.9f;
		const float xVoct = 19.9f;
		const float xInVol = 31.9f;
		const float xInPb = 43.9f;

		const float yIns = 117.8f;

		const float xOut1 = 56.1f;
		const float xOut2 = 65.1f;
		const float yOut = 50.9f;
		const float yOutAdd = 9.5f;

		addParam(createParamCentered<VCVButton>(mm2px(Vec(11, 4)), module, KeySampler::PREVSAMPLE_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(60, 4)), module, KeySampler::NEXTSAMPLE_PARAM));

		//----------------------------------------------------------------------------------------------------------------------------

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInL, yInputs)), module, KeySampler::IN_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInR, yInputs)), module, KeySampler::IN_INPUT+1));
		
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xGain, yGain)), module, KeySampler::GAIN_PARAM));

		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xRecBut, yInputs)), module, KeySampler::REC_PARAM, KeySampler::REC_LIGHT));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xMon, yMon)), module, KeySampler::MONITOR_SWITCH, KeySampler::MONITOR_LIGHT));

		//------------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xSampSelect, ySampSelect)), module, KeySampler::SLOT_SELECTOR_PARAM));
		//addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<BlueLight>>>(mm2px(Vec(xTrigBut, yTrigBut)), module, KeySampler::TESTBUT_PARAM, KeySampler::TESTBUT_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<YellowLight>>>(mm2px(Vec(xRngBt, yRngBt)), module, KeySampler::RANGE_SELECTOR_PARAM, KeySampler::RANGE_SELECTOR_LIGHT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xRngKn, yRngKn)), module, KeySampler::RANGE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xOutKn, yOutKn)), module, KeySampler::OUT_SELECTOR_PARAM));

		//------------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<SickoSwitch_CKSS_Horiz>(mm2px(Vec(xTrigGate, yTrigGate)), module, KeySampler::TRIGGATEMODE_SWITCH));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xTrigMode, yTrigMode)), module, KeySampler::TRIGMODE_SWITCH, KeySampler::TRIGMODE_LIGHT));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xPb, yPb)), module, KeySampler::PB_SWITCH, KeySampler::PB_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xScan, yScan)), module, KeySampler::PHASESCAN_SWITCH, KeySampler::PHASESCAN_LIGHT));

		//----------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStart, yCue)), module, KeySampler::CUESTART_PARAM));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xEnd, yCue)), module, KeySampler::CUEEND_PARAM));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStart, yLoop)), module, KeySampler::LOOPSTART_PARAM));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xEnd, yLoop)), module, KeySampler::LOOPEND_PARAM));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xRev, yRev)), module, KeySampler::REV_PARAM, KeySampler::REV_LIGHT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xXfade, yXfade)), module, KeySampler::XFADE_PARAM));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xLp, yLp)), module, KeySampler::LOOP_PARAM, KeySampler::LOOP_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xPng, yPng)), module, KeySampler::PINGPONG_PARAM, KeySampler::PINGPONG_LIGHT));
		
		//----------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv1, yEnv1)), module, KeySampler::ATTACK_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv1+xEnv1Add, yEnv1)), module, KeySampler::DECAY_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv1+xEnv1Add*2, yEnv1)), module, KeySampler::SUSTAIN_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xEnv1+xEnv1Add*3, yEnv1)), module, KeySampler::RELEASE_PARAM));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCurve, yEnv1)), module, KeySampler::CURVE_PARAM));

		//----------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xStretch, yStretch)), module, KeySampler::STRETCH_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xStretchSiz, yStretchSiz)), module, KeySampler::STR_SIZE_PARAM));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xVol, yVol)), module, KeySampler::VOL_PARAM));

		//----------------------------------------------------------------------------------------------------------------------------

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xGateIn, yIns)), module, KeySampler::GATE_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xVoct, yIns)), module, KeySampler::VO_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInVol, yIns)), module, KeySampler::VEL_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xInPb, yIns)), module, KeySampler::PB_INPUT));

		//----------------------------------------------------------------------------------------------------------------------------
		
		for (int i = 0; i < 8; i++) {
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut1, yOut+(yOutAdd*i))), module, KeySampler::LEFT_OUTPUT+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut2, yOut+(yOutAdd*i))), module, KeySampler::RIGHT_OUTPUT+i));
		}

	}

	/*
	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		KeySampler *module = dynamic_cast<KeySampler*>(this->module);
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
					menu->addChild(createMenuItem(module->folderTreeDisplay[tempIndex][i], "", [=]() {module->loadFromPatch[module->currSlot] = false;module->loadSample(module->folderTreeData[tempIndex][i], module->currSlot);}));
				}
			}
		}
	}
	*/

	void appendContextMenu(Menu *menu) override {
	   	KeySampler *module = dynamic_cast<KeySampler*>(this->module);
			assert(module);

		/*
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Load Sample", "", [=]() {
			//module->menuLoadSample();
			bool temploadFromPatch = module->loadFromPatch[module->currSlot];
			module->loadFromPatch[module->currSlot] = false;
			module->menuLoadSample(module->currSlot);
			if (module->restoreLoadFromPatch[module->currSlot])
				module->loadFromPatch[module->currSlot] = temploadFromPatch;
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
						menu->addChild(createMenuItem(module->folderTreeDisplay[0][i], "", [=]() {module->loadFromPatch[module->currSlot] = false;module->loadSample(module->folderTreeData[0][i], module->currSlot);}));
					}
				}
			}));
		}

		if (module->fileLoaded[module->currSlot]) {
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("Current Sample:"));
			menu->addChild(createMenuLabel(module->fileDescription[module->currSlot]));
			menu->addChild(createMenuLabel(" " + module->samplerateDisplay[module->currSlot] + " - " + std::to_string(module->channels[module->currSlot]) + "ch"));
			std::string tempDisplay = " ";
			if (module->resampled[module->currSlot]) {
				tempDisplay += "resampled to " + std::to_string(int(APP->engine->getSampleRate()));
				if (module->toSave[module->currSlot])
					tempDisplay += " - ";
			}
			if (module->toSave[module->currSlot])
				tempDisplay += "to SAVE";
			if (tempDisplay != " ") {
				menu->addChild(createMenuLabel(tempDisplay));
			}

			menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot(module->currSlot);}));

			menu->addChild(new MenuSeparator());
			
			menu->addChild(createMenuItem("Save FULL Sample", "", [=]() {module->menuSaveSample(SAVE_FULL, module->currSlot);}));
			menu->addChild(createMenuItem("Save CUE Region", "", [=]() {module->menuSaveSample(SAVE_CUE, module->currSlot);}));
			menu->addChild(createMenuItem("Save LOOP Region", "", [=]() {module->menuSaveSample(SAVE_LOOP, module->currSlot);}));

			menu->addChild(createBoolPtrMenuItem("Trim Sample after Save", "", &module->trimOnSave));
			menu->addChild(createBoolPtrMenuItem("Save Oversampled", "", &module->saveOversampled));
		
		} else if (module->storedPath[module->currSlot] != "" && module->fileFound[module->currSlot] == false) {
			menu->addChild(createMenuLabel("Sample ERROR:"));
			menu->addChild(createMenuLabel(module->fileDescription[module->currSlot]));
			menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot(module->currSlot);}));
		}
		*/
		
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Select Samples Root", "", [=]() {module->selectRootFolder();}));

		if (module->userFolder != "") {
			menu->addChild(createMenuLabel(module->userFolder));
			//menu->addChild(createMenuItem("", "Refresh", [=]() {module->refreshRootFolder();}));
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Trim Sample after Save", "", &module->trimOnSave));
		menu->addChild(createBoolPtrMenuItem("Save Oversampled", "", &module->saveOversampled));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Quantize incoming v/Oct", "", &module->quantize));
		menu->addChild(new MenuSeparator());

		struct PbItem : MenuItem {
			KeySampler* module;
			int pbRange;
			void onAction(const event::Action& e) override {
				module->pbRange = pbRange;
			}
		};

		std::string pbNames[15] = {"1 semitone", "2 semitones", "3 semitones", "4 semitones", "5 semitones", "6 semitones", "7 semitones", "8 semitones", "9 semitones", "10 semitones", "11 semitones", "1 octave", "2 octaves", "3 octaves", "4 octaves"};
		menu->addChild(createSubmenuItem("Pitch Bend Range", pbNames[module->pbRange], [=](Menu* menu) {
		for (int i = 0; i < 15; i++) {
			PbItem* pbItem = createMenuItem<PbItem>(pbNames[i]);
			pbItem->rightText = CHECKMARK(module->pbRange == i);
			pbItem->module = module;
			pbItem->pbRange = i;
			menu->addChild(pbItem);
		} }));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Anti-aliasing filter", "", &module->antiAlias));

		menu->addChild(new MenuSeparator());
		/*
		menu->addChild(createBoolMenuItem("Polyphonic OUTs", "", [=]() {
				return module->isPolyOuts();
			}, [=](bool poly) {
				module->setPolyOuts(poly);
		}));
		menu->addChild(createBoolPtrMenuItem("Polyphonic Master IN", "", &module->polyMaster));
		*/

		menu->addChild(createBoolPtrMenuItem("Auto Monitor Off", "", &module->autoMonOff));

		/*
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
		*/
		menu->addChild(createBoolPtrMenuItem("Disable NAV Buttons", "", &module->disableNav));		
		//menu->addChild(createMenuItem("Reset Cursors", "", [=]() {module->resetCursors();}));
		menu->addChild(createBoolPtrMenuItem("Reset cursors on Load", "", &module->resetCursorsOnLoad));
		/*
		menu->addChild(createSubmenuItem("Presets", "", [=](Menu * menu) {
			menu->addChild(createMenuItem("Wavetable", "", [=]() {module->setPreset(0);}));
			menu->addChild(createMenuItem("Triggered Sample with Envelope", "", [=]() {module->setPreset(1);}));
			menu->addChild(createMenuItem("Drum Player", "", [=]() {module->setPreset(2);}));
		}));
		*/

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Store Sample in Patch", "", &module->sampleInPatch));
#if !defined(METAMODULE)
		menu->addChild(createBoolPtrMenuItem("Unlimited REC (risky)", "", &module->unlimitedRecording));
#endif
	}
};

Model *modelKeySampler = createModel<KeySampler, KeySamplerWidget>("KeySampler");
