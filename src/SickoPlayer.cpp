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
#define START_ONLY 1
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
//#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <vector>
#include "cmath"
#include <dirent.h>
#include <libgen.h>

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
		NUM_PARAMS 
	};
	enum InputIds {
		TRIG_INPUT,
		STOP_INPUT,
		VOL_INPUT,
		TUNE_INPUT,
		VO_INPUT,
		LOOPSTART_INPUT,
		LOOPEND_INPUT,
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
		NUM_LIGHTS
	};
  
	unsigned int channels;
	unsigned int sampleRate;
	drwav_uint64 totalSampleC;
	drwav_uint64 totalSamples;

	vector<float> playBuffer[2][2];
	vector<double> displayBuff;
	int currentDisplay = 0;
	float voctDisplay = 100.f;

	bool fileLoaded = false;

	bool play[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool inPause[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

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

	float knobTune = 0.f;
	float prevKnobTune = 9.f;
	float tune;

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

	//bool searchingRestartPhase = false; // used only for trig start/restart mode

	double prevSampleWeight[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double currSampleWeight[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double prevSamplePos[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	std::string storedPath = "";
	std::string fileDescription = "--none--";
	std::string fileDisplay = "";
	std::string channelsDisplay = "";
	std::string timeDisplay = "";
	std::string samplerateDisplay = "";

	std::string userFolderName = "";
	vector <std::string> browserFileName;
	vector <std::string> browserFileDisplay;

	int seconds;
	int minutes;

	float trigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevTrigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float stopValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevStopValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	bool loop = false;

	int interpolationMode = HERMITE_INTERP;
	int antiAlias = 1;
	int polyOuts = POLYPHONIC;
	bool phaseScan = true;
	
	//float fadeCoeff[7] = {APP->engine->getSampleRate(), 2000.f, 1000.f, 200.f, 100.f, 50.f, 20.f};	// None, 0.5ms, 1ms, 5ms, 10ms, 20ms, 50ms fading
	float fadeCoeff = 0.f;

	int fadingType[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float fadingValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float fadeDecrement = 0;
	double fadedPosition[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double fadeSamples = 0;

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
	//float fadeStageLevel[16];
	float lastStageLevel[16];
	float currentStageSample[16];
	float maxStageSample[16];
	
	bool eoc = false;
	bool eor = false;
	float eocTime;
	float eorTime;
	float eocEorTime = (APP->engine->getSampleRate()) / 2000;

	int chan;

	float attackValue;
	float decayValue;
	float sustainValue;
	float releaseValue;
	float masterLevel;
	int limiter;

	bool reverseStart = false;
	int reversePlaying[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int pingpong;

	float clippingValue = 0;
	//float clippingCoeff = 2 / (APP->engine->getSampleRate());
	float clippingCoeff = 5 / (APP->engine->getSampleRate());	// nr of samples of 200 ms (1/0.2)
	bool clipping = false;

	SickoPlayer() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configSwitch(TRIGGATEMODE_SWITCH, 0.f, 1.f, 1.f, "Mode", {"Gate", "Trig"});
		configSwitch(TRIGMODE_SWITCH, 0.f, 2.f, 0.f, "Trig mode", {"Start/Stop", "Start Only", "Play/Pause"});

		configInput(TRIG_INPUT,"Trig/Gate");
		configInput(STOP_INPUT,"Stop");

		//******************************************************************************

		configParam(CUESTART_PARAM, 0.f, 1.0f, 0.f, "Cue Start", "%", 0, 100);
		configParam(CUEEND_PARAM, 0.f, 1.0f, 1.f, "Cue End", "%", 0, 100);

		configParam(LOOPSTART_PARAM, 0.f, 1.0f, 0.f, "Loop Start", "%", 0, 100);
		configParam(LOOPEND_PARAM, 0.f, 1.0f, 1.0f, "Loop End", "%", 0, 100);

		configSwitch(REV_PARAM, 0.f, 1.f, 0.f, "Playback Start", {"Forward", "Reverse"});
		configParam(XFADE_PARAM, 0.f, 1.f, 0.f, "Crossfade", "ms", 0, 1000);
		
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});
		configSwitch(PINGPONG_PARAM, 0.f, 1.f, 0.f, "PingPong", {"Off", "On"});

		//******************************************************************************

		configParam(ATTACK_PARAM, 0.0001f, 10.f, 0.0001f, "Attack", "ms", 0, 1000);
		configInput(ATTACK_INPUT,"Attack CV");
		configParam(ATTACKATNV_PARAM, -1.0f, 1.0f, 0.0f, "Attack CV Attenuv.");

		configParam(DECAY_PARAM, 0.0001f, 10.f, 0.0001f, "Decay", "ms", 0, 1000);
		configInput(DECAY_INPUT,"Decay CV");
		configParam(DECAYATNV_PARAM, -1.0f, 1.0f, 0.0f, "Decay CV Attenuv.");

		configParam(SUSTAIN_PARAM, 0.f, 1.0f, 1.0f, "Sustain","%", 0, 100);
		configInput(SUSTAIN_INPUT,"Sustain CV");
		configParam(SUSTAINATNV_PARAM, -1.0f, 1.0f, 0.0f, "Sustain CV Attenuv.");

		configParam(RELEASE_PARAM, 0.0001f, 10.f, 0.0001f, "Release", "ms", 0, 1000);
		configInput(RELEASE_INPUT,"Release CV");
		configParam(RELEASEATNV_PARAM, -1.0f, 1.0f, 0.0f, "Release CV Attenuv.");

		//******************************************************************************
		
		configInput(VO_INPUT,"V/Oct");

		configParam(TUNE_PARAM, -2.f, 2.f, 0.f, "Tune", " semitones", 0, 12);
		configInput(TUNE_INPUT,"Tune CV");
		configParam(TUNEATNV_PARAM, -1.0f, 1.0f, 0.0f, "Tune CV Attenuv.");

		configParam(VOL_PARAM, 0.f, 2.0f, 1.0f, "Master Volume", "%", 0, 100);
		configInput(VOL_INPUT,"Master Volume CV");
		configParam(VOLATNV_PARAM, -1.f, 1.0f, 0.f, "Master Volume CV Attenuv.", "%", 0, 100);

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

	void onReset() override {
		interpolationMode = HERMITE_INTERP;
		antiAlias = 1;
		polyOuts = POLYPHONIC;
		phaseScan = true;
		clearSlot();
		for (int i = 0; i < 16; i++) {
			play[i] = false;
			inPause[i] = false;
			fadingType[i] = NO_FADE;
			stage[i] = STOP_STAGE;
			stageLevel[i] = 0;
			voct[i] = 0.f;
			prevVoct[i] = 11.f;
			reversePlaying[i] = FORWARD;
		}
		prevKnobCueStartPos = -1.f;
		prevKnobCueEndPos = 2.f;
		prevKnobLoopStartPos = -1.f;
		prevKnobLoopEndPos = 2.f;
		prevKnobTune = -1.f;
		reverseStart = false;
		totalSampleC = 0;
		totalSamples = 0;
	}

	void onSampleRateChange() override {
		eocEorTime = (APP->engine->getSampleRate())/2000;			// number of samples for 1 ms used for output triggers
		clippingCoeff = 5 / (APP->engine->getSampleRate());	// decrement for 200 ms clipping light (1/0.2)
		if (fileLoaded)
			sampleCoeff = sampleRate / (APP->engine->getSampleRate());			// the % distance between samples at speed 1x
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Interpolation", json_integer(interpolationMode));
		json_object_set_new(rootJ, "AntiAlias", json_integer(antiAlias));
		json_object_set_new(rootJ, "PolyOuts", json_integer(polyOuts));
		json_object_set_new(rootJ, "PhaseScan", json_boolean(phaseScan));
		json_object_set_new(rootJ, "Slot", json_string(storedPath.c_str()));
		json_object_set_new(rootJ, "UserFolder", json_string(userFolderName.c_str()));
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

		json_t* phaseScanJ = json_object_get(rootJ, "PhaseScan");
		if (phaseScanJ)
			phaseScan = json_boolean_value(phaseScanJ);

		json_t *slotJ = json_object_get(rootJ, "Slot");
		if (slotJ) {
			storedPath = json_string_value(slotJ);
			loadSample(storedPath);
		}
		json_t *userFolderNameJ = json_object_get(rootJ, "UserFolder");
		if (userFolderNameJ) {
			userFolderName = json_string_value(userFolderNameJ);
			folderSelection(userFolderName);
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

	void folderSelection(std::string path) {
		DIR* rep = NULL;
		struct dirent* dirp = NULL;
			char* pathDup = strdup(path.c_str());
			std::string dir = pathDup;
		rep = opendir(dir.c_str());
		browserFileName.clear();
		browserFileDisplay.clear();
		while ((dirp = readdir(rep)) != NULL) {
			std::string name = dirp->d_name;
			std::size_t found = name.find(".wav",name.length()-5);
			if (found==std::string::npos)
				found = name.find(".WAV",name.length()-5);

			if (found!=std::string::npos) {
				browserFileName.push_back(name);
				browserFileDisplay.push_back(name.substr(0, name.length()-4));
			}
		}
		sort(browserFileName.begin(), browserFileName.end());
		sort(browserFileDisplay.begin(), browserFileDisplay.end());
		closedir(rep);
	};

	void loadSample(std::string path) {
		z1 = 0; z2 = 0; z1r = 0; z2r = 0;
		unsigned int c;
		unsigned int sr;
		drwav_uint64 tsc;
		float* pSampleData;
		pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &tsc);

		if (pSampleData != NULL) {
			channels = c;
			sampleRate = sr * 2;
			calcBiquadLpf(20000.0, sampleRate, 1);
			playBuffer[LEFT][0].clear();
			playBuffer[LEFT][1].clear();
			playBuffer[RIGHT][0].clear();
			playBuffer[RIGHT][1].clear();
			displayBuff.clear();

			if (tsc > 52428800)
				tsc = 52428800;	// set memory allocation limit to 200Mb for samples (~18mins at 48.000khz MONO)

			for (unsigned int i=0; i < tsc; i = i + c) {
				//playBuffer[LEFT][0].push_back(pSampleData[i]);
				playBuffer[LEFT][0].push_back(pSampleData[i] * 5);
				playBuffer[LEFT][0].push_back(0);
				if (channels == 2) {
					//playBuffer[RIGHT][0].push_back(pSampleData[i+1]);
					playBuffer[RIGHT][0].push_back(pSampleData[i+1] * 5);
					playBuffer[RIGHT][0].push_back(0);
				}
			}
			totalSampleC = playBuffer[LEFT][0].size();
			totalSamples = totalSampleC-1;
			drwav_free(pSampleData);

			for (unsigned int i = 1; i < totalSamples; i = i+2) {
				playBuffer[LEFT][0][i] = playBuffer[LEFT][0][i-1] * .5f + playBuffer[LEFT][0][i+1] * .5f;
				if (channels == 2)
					playBuffer[RIGHT][0][i] = playBuffer[RIGHT][0][i-1] * .5f + playBuffer[RIGHT][0][i+1] * .5f;
			}
			
			playBuffer[LEFT][0][totalSamples] = playBuffer[LEFT][0][totalSamples-1] * .5f; // halve the last sample
			if (channels == 2)
				playBuffer[RIGHT][0][totalSamples] = playBuffer[RIGHT][0][totalSamples-1] * .5f;

			for (unsigned int i = 0; i < totalSampleC; i++) {
				playBuffer[LEFT][1].push_back(biquadLpf(playBuffer[LEFT][0][i]));
				if (channels == 2)
					playBuffer[RIGHT][1].push_back(biquadLpf2(playBuffer[RIGHT][0][i]));
			}

			sampleCoeff = sampleRate / (APP->engine->getSampleRate());			// the % distance between samples at speed 1x

			prevKnobCueStartPos = -1.f;
			prevKnobCueEndPos = 2.f;
			prevKnobLoopStartPos = -1.f;
			prevKnobLoopEndPos = 2.f;

			vector<double>().swap(displayBuff);
			for (int i = 0; i < floor(totalSampleC); i = i + floor(totalSampleC/240))
				displayBuff.push_back(playBuffer[0][0][i]);

			seconds = totalSampleC / sampleRate;
			minutes = (seconds / 60) % 60;
			seconds = seconds % 60;

			timeDisplay = std::to_string(minutes) + ":";
			if (seconds < 10)
				timeDisplay += "0";
			timeDisplay += std::to_string(seconds);

			char* pathDup = strdup(path.c_str());
			fileDescription = basename(pathDup);

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

			fileLoaded = true;
		} else {
			fileLoaded = false;
			storedPath = "";
			fileDescription = "--none--";
			fileDisplay = "";
			timeDisplay = "";
			channelsDisplay = "";
		}
	};
	
	void clearSlot() {
		storedPath = "";
		fileDescription = "--none--";
		fileDisplay = "";
		timeDisplay = "";
		channelsDisplay = "";
		fileLoaded = false;
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

	void setPreset(int presetNo) {
		switch (presetNo) {
			case 0:
				phaseScan = false;
				params[TRIGGATEMODE_SWITCH].setValue(0);
				params[TRIGMODE_SWITCH].setValue(0);
				params[CUESTART_PARAM].setValue(0);
				params[CUEEND_PARAM].setValue(1);
				params[LOOPSTART_PARAM].setValue(0);
				params[LOOPEND_PARAM].setValue(1);
				params[XFADE_PARAM].setValue(0);
				params[LOOP_PARAM].setValue(1);
				params[PINGPONG_PARAM].setValue(0);
				reverseStart = false;
				prevKnobCueStartPos = -1;
				prevKnobCueEndPos = 2;
				prevKnobLoopStartPos = -1;
				prevKnobLoopEndPos = 2;
			break;
			case 1:
				phaseScan = true;
				params[TRIGGATEMODE_SWITCH].setValue(1);
				params[TRIGMODE_SWITCH].setValue(0);
				params[XFADE_PARAM].setValue(0.3f);
				params[LOOP_PARAM].setValue(0);
				params[PINGPONG_PARAM].setValue(0);
				params[ATTACK_PARAM].setValue(0.2f);
				params[DECAY_PARAM].setValue(0);
				params[SUSTAIN_PARAM].setValue(1);
				params[RELEASE_PARAM].setValue(0.2f);
				prevKnobCueStartPos = -1;
				prevKnobCueEndPos = 2;
				prevKnobLoopStartPos = -1;
				prevKnobLoopEndPos = 2;
			break;
		}
	}
	
	void process(const ProcessArgs &args) override {

		reverseStart = params[REV_PARAM].getValue();
		lights[REV_LIGHT].setBrightness(reverseStart);

		pingpong = params[PINGPONG_PARAM].getValue();
		lights[PINGPONG_LIGHT].setBrightness(pingpong);

		loop = params[LOOP_PARAM].getValue();
		lights[LOOP_LIGHT].setBrightness(loop);
		chan = std::max(1, inputs[VO_INPUT].getChannels());
		
		if (!fileLoaded) {

			for (int c = 0; c < chan; c++) {
				play[c] = false;
				inPause[c] = false;
				fadingType[c] = NO_FADE;
				stage[c] = STOP_STAGE;
				stageLevel[c] = 0;
				voct[c] = 0.f;
				prevVoct[c] = 11.f;
				outputs[OUT_OUTPUT].setVoltage(0, c);
				outputs[OUT_OUTPUT+1].setVoltage(0, c);
			}
			
		} else {

			knobCueStartPos = params[CUESTART_PARAM].getValue();
			if (knobCueStartPos != prevKnobCueStartPos) {
				prevKnobCueStartPos = knobCueStartPos;
				cueStartPos = floor(totalSamples * knobCueStartPos);
				searchingCueStartPhase = true;
				scanCueStartSample = cueStartPos;
				if (cueStartPos > cueEndPos)
					cueStartPos = cueEndPos;
			}

			knobCueEndPos = params[CUEEND_PARAM].getValue();
			if (knobCueEndPos != prevKnobCueEndPos) {
				prevKnobCueEndPos = knobCueEndPos;
				cueEndPos = floor(totalSamples * knobCueEndPos);
				searchingCueEndPhase = true;
				scanCueEndSample = cueEndPos;
				if (cueEndPos < cueStartPos)
					cueEndPos = cueStartPos;
			}
			
			knobLoopStartPos = params[LOOPSTART_PARAM].getValue();
			if (knobLoopStartPos != prevKnobLoopStartPos) {
				prevKnobLoopStartPos = knobLoopStartPos;
				loopStartPos = floor(totalSamples * knobLoopStartPos);
				searchingLoopStartPhase = true;
				scanLoopStartSample = loopStartPos;
				if (loopStartPos > loopEndPos)
					loopStartPos = loopEndPos;
			} 

			knobLoopEndPos = params[LOOPEND_PARAM].getValue();
			if (knobLoopEndPos != prevKnobLoopEndPos) {
				prevKnobLoopEndPos = knobLoopEndPos;
				loopEndPos = floor(totalSamples * knobLoopEndPos);
				searchingLoopEndPhase = true;
				scanLoopEndSample = loopEndPos;
				if (loopEndPos < loopStartPos)
					loopEndPos = loopStartPos;
			}

			if (phaseScan) {
				float tempKnob;
				if (searchingCueStartPhase) {
					if (playBuffer[LEFT][antiAlias][scanCueStartSample+1] >= 0 && playBuffer[LEFT][antiAlias][scanCueStartSample] < 0) {
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

				if (searchingCueEndPhase) {
					if (playBuffer[LEFT][antiAlias][scanCueEndSample-1] <= 0 && playBuffer[LEFT][antiAlias][scanCueEndSample] > 0) {
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
				if (searchingLoopStartPhase) {
					if (playBuffer[LEFT][antiAlias][scanLoopStartSample+1] >= 0 && playBuffer[LEFT][antiAlias][scanLoopStartSample] < 0) {
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

				if (searchingLoopEndPhase) {
					if (playBuffer[LEFT][antiAlias][scanLoopEndSample-1] <= 0 && playBuffer[LEFT][antiAlias][scanLoopEndSample] > 0) {
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
			} 

			/*
			if (searchingRestartPhase) {  		// *** *** on PLAY/RESTART trig mode this continues playing until phase is reached (attack value must be set to 0)
				if (playBuffer[LEFT][antiAlias][floor(samplePos)-1] <= 0 && playBuffer[LEFT][antiAlias][floor(samplePos)] > 0) {
					searchingRestartPhase = false;
					fading = true;
					fadingValue = 1.f;
					fadedPosition = samplePos;

					samplePos = floor(cueStartPos+1);
					currSampleWeight = sampleCoeff;
					prevSamplePos = floor(cueStartPos);
					prevSampleWeight = 0;
					stage = ATTACK_STAGE;
					currentStageSample = 0;
					lastStageLevel = 0;
					fadeStageLevel = stageLevel;
				} 
			}
			*/

			trigMode = params[TRIGGATEMODE_SWITCH].getValue();
			trigType = params[TRIGMODE_SWITCH].getValue();

			attackValue = params[ATTACK_PARAM].getValue() + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());

			decayValue = params[DECAY_PARAM].getValue() + (inputs[DECAY_INPUT].getVoltage() * params[DECAYATNV_PARAM].getValue());
			
			sustainValue = params[SUSTAIN_PARAM].getValue() + (inputs[SUSTAIN_INPUT].getVoltage() * params[SUSTAINATNV_PARAM].getValue() * 0.1);
			if (sustainValue > 1)
				sustainValue = 1;
			else if (sustainValue < 0)
				sustainValue = 0;

			releaseValue = params[RELEASE_PARAM].getValue() + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
			
			masterLevel = params[VOL_PARAM].getValue() + (inputs[VOL_INPUT].getVoltage() * params[VOLATNV_PARAM].getValue() * 0.1);
			if (masterLevel > 2)
				masterLevel = 2;
			else if (masterLevel < 0)
				masterLevel = 0;
			
			limiter = params[LIMIT_SWITCH].getValue();

			knobTune = params[TUNE_PARAM].getValue();
			if (knobTune != prevKnobTune) {
				tune = powf(2,knobTune);
				knobTune = prevKnobTune;
			}

			if (inputs[TUNE_INPUT].isConnected()) {
				currentSpeed = double(tune + (inputs[TUNE_INPUT].getVoltage() * params[TUNEATNV_PARAM].getValue() * 0.1));
				if (currentSpeed > 4)
					currentSpeed = 4;
				else if (currentSpeed < 0.25)
						currentSpeed = 0.25;
			} else {
				currentSpeed = double(tune);
			}

			sumOutput = 0;
			sumOutputR = 0;

			fadeSamples = floor(params[XFADE_PARAM].getValue() * args.sampleRate); // number of samples before starting fade

			// START CHANNEL MANAGEMENT

			voctDisplay = 100.f;

			for (int c = 0; c < chan; c++) {
				
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
								currentStageSample[c] = 0;
								lastStageLevel[c] = 0;
							} else {
								if (stage[c] == RELEASE_STAGE) {
									stage[c] = ATTACK_STAGE;
									currentStageSample[c] = 0;
									lastStageLevel[c] = stageLevel[c];
								}
							}
						} else {
							if (play[c]) {
								if (stage[c] != RELEASE_STAGE) {
									stage[c]=RELEASE_STAGE;
									currentStageSample[c] = 0;
									lastStageLevel[c] = 1-stageLevel[c];
								}
							}
						}
					break;

					case TRIG_MODE:												// ***** TRIG MODE *****
						if (trigValue[c] >= 1 && prevTrigValue[c] < 1){
							switch (trigType) {
								case START_STOP:									// trig type: Start/Stop
									if (play[c]) {
										if (stage[c] != RELEASE_STAGE) {
											stage[c]=RELEASE_STAGE;
											currentStageSample[c] = 0;
											lastStageLevel[c] = 1-stageLevel[c];
										} else {
											stage[c] = ATTACK_STAGE;
											currentStageSample[c] = 0;
											lastStageLevel[c] = stageLevel[c];
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
										currentStageSample[c] = 0;
										lastStageLevel[c] = 0;
									}
								break;

								case START_ONLY:									// trig type: START ONLY
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
										currentStageSample[c] = 0;
										lastStageLevel[c] = 0;
									} else if (stage[c] == RELEASE_STAGE) {
										play[c] = true;
										stage[c] = ATTACK_STAGE;
										currentStageSample[c] = 0;
										lastStageLevel[c] = stageLevel[c];
									}
								break;
								
								/*
								case START_RESTART:									// trig type: Start/Restart
									if (play[c]) {
										if (!searchingRestartPhase)
											searchingRestartPhase = true;
										//restartSample = samplePos;
										//fading[c] = true;
										//fadingValue[c] = 1.f;
										//fadedPosition[c] = samplePos;
									} else {
										play[c] = true;
										samplePos[c] = floor(cueStartPos+1);
										currSampleWeight[c] = sampleCoeff;
										prevSamplePos[c] = floor(cueStartPos);
										prevSampleWeight[c] = 0;
										stage[c] = ATTACK_STAGE;
										currentStageSample[c] = 0;
										lastStageLevel[c] = 0;
										//fadeStageLevel[c] = stageLevel[c];
									}
								break;
								*/
								case PLAY_PAUSE:									// trig type: Play/Pause
									if (play[c]) {
										if (stage[c] != RELEASE_STAGE) {
											inPause[c] = true;
											stage[c] = RELEASE_STAGE;
											currentStageSample[c] = 0;
											lastStageLevel[c] = 1-stageLevel[c];
										} else {
											stage[c] = ATTACK_STAGE;
											currentStageSample[c] = 0;
											lastStageLevel[c] = stageLevel[c];
										}
									} else {
										if (inPause[c]) {
											play[c] = true;
											inPause[c] = false;
											stage[c] = ATTACK_STAGE;
											currentStageSample[c] = 0;
											lastStageLevel[c] = 0;
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
											currentStageSample[c] = 0;
											lastStageLevel[c] = 0;
										}
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
						distancePos[c] = currentSpeed * sampleCoeff * speedVoct[c];
					} else
						distancePos[c] = currentSpeed * sampleCoeff;

					if (play[c] && voct[c] < voctDisplay) {
						currentDisplay = c;
						voctDisplay = voct[c];
					}

					switch (reversePlaying[c]) {
						case FORWARD:		// FORWARD PLAYBACK
							if (loop && samplePos[c] > floor(loopEndPos - (fadeSamples * distancePos[c]))) {		// *** REACHED END OF LOOP ***
								fadingValue[c] = 1.f;
								fadedPosition[c] = samplePos[c];
								if (fadeSamples)
									fadeCoeff =  1 / params[XFADE_PARAM].getValue() / args.sampleRate;
								else
									fadeCoeff = 1;

								if (pingpong) {
									fadingType[c] = PINGPONG_FADE;
									reversePlaying[c] = REVERSE;
									params[REV_PARAM].setValue(1);
									samplePos[c] = floor(loopEndPos)-1;
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = floor(loopEndPos);
									prevSampleWeight[c] = 0;
								} else {
									fadingType[c] = CROSS_FADE;
									samplePos[c] = floor(loopStartPos)+1;
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = floor(loopStartPos);
									prevSampleWeight[c] = 0;
								}
								if (c == currentDisplay) {
									eoc = true;
									eocTime = eocEorTime;
								}
							} else if (!fadingType[c] && floor(samplePos[c]) > (totalSamples - (fadeSamples * distancePos[c]))) {
								fadingType[c] = FADE_OUT;
								fadingValue[c] = 1.f;
								fadedPosition[c] = samplePos[c];
								if (fadeSamples)
									fadeCoeff =  1 / params[XFADE_PARAM].getValue() / args.sampleRate;
								else
									fadeCoeff = 1;
							} else if (floor(samplePos[c]) > totalSamples) {	// *** REACHED END OF SAMPLE ***
								play[c] = false;
								inPause[c] = false;
								if (c == currentDisplay) {
									eoc = true;
									eocTime = eocEorTime;
								}
							} else if (samplePos[c] > cueEndPos) {				// *** REACHED CUE END ***
								if (stage[c] != RELEASE_STAGE) {
									stage[c] = RELEASE_STAGE;
									currentStageSample[c] = 0;
									lastStageLevel[c] = 1-stageLevel[c];
								}
								if (trigMode == GATE_MODE) {
									if (pingpong) {
										reversePlaying[c] = REVERSE;
										params[REV_PARAM].setValue(1);
										samplePos[c] = floor(cueEndPos)-1;
										currSampleWeight[c] = sampleCoeff;
										prevSamplePos[c] = floor(cueEndPos);
										prevSampleWeight[c] = 0;
									} else {
										fadingType[c] = CROSS_FADE;
										fadingValue[c] = 1.f;
										samplePos[c] = floor(cueStartPos)+1;
										currSampleWeight[c] = sampleCoeff;
										prevSamplePos[c] = floor(cueStartPos);
										prevSampleWeight[c] = 0;
									}
								}
								if (c == currentDisplay) {
									eoc = true;
									eocTime = eocEorTime;
								}
							} 
						break;

						case REVERSE:		// REVERSE PLAYBACK
							//if (loop && samplePos[c] < floor(loopStartPos)) {	// *** REACHED BEGIN OF LOOP ***
							if (loop && samplePos[c] < floor(loopStartPos + (fadeSamples * distancePos[c]))) {	// *** REACHED BEGIN OF LOOP ***
								fadingValue[c] = 1.f;
								fadedPosition[c] = samplePos[c];
								if (fadeSamples)
									fadeCoeff =  1 / params[XFADE_PARAM].getValue() / args.sampleRate;
								else
									fadeCoeff = 1;

								if (pingpong) {
									fadingType[c] = PINGPONG_FADE;
									reversePlaying[c] = FORWARD;
									params[REV_PARAM].setValue(0);
									samplePos[c] = floor(loopStartPos)+1;
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = floor(loopStartPos);
									prevSampleWeight[c] = 0;
								} else {
									fadingType[c] = CROSS_FADE;
									samplePos[c] = floor(loopEndPos)-1;
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = floor(loopEndPos);
									prevSampleWeight[c] = 0;
								}
								if (c == currentDisplay) {
									eoc = true;
									eocTime = eocEorTime;
								}
							} else if (!fadingType[c] && floor(samplePos[c]) < (fadeSamples * distancePos[c])) {
								fadingType[c] = FADE_OUT;
								fadingValue[c] = 1.f;
								fadedPosition[c] = samplePos[c];
								if (fadeSamples)
									fadeCoeff =  1 / params[XFADE_PARAM].getValue() / args.sampleRate;
								else
									fadeCoeff = 1;
							} else if (floor(samplePos[c]) < 0) {				// *** REACHED START OF SAMPLE ***
								play[c] = false;
								inPause[c] = false;
								if (c == currentDisplay) {
									eoc = true;
									eocTime = eocEorTime;
								}
							} else if (samplePos[c] < cueStartPos) {			// *** REACHED CUE START ***
								if (stage[c] != RELEASE_STAGE) {
									stage[c] = RELEASE_STAGE;
									currentStageSample[c] = 0;
									lastStageLevel[c] = 1-stageLevel[c];
								}
								if (trigMode == GATE_MODE) {
									if (pingpong) {
										reversePlaying[c] = FORWARD;
										params[REV_PARAM].setValue(0);
										samplePos[c] = floor(cueStartPos)+1;
										currSampleWeight[c] = sampleCoeff;
										prevSamplePos[c] = floor(cueStartPos);
										prevSampleWeight[c] = 0;										
									} else {
										fadingType[c] = CROSS_FADE;
										fadingValue[c] = 1.f;
										fadedPosition[c] = samplePos[c];
										samplePos[c] = floor(cueEndPos)-1;
										currSampleWeight[c] = sampleCoeff;
										prevSamplePos[c] = floor(cueEndPos);
										prevSampleWeight[c] = 0;
									}
								}
								if (c == currentDisplay) {
									eoc = true;
									eocTime = eocEorTime;
								}
							}
						break;
					}

					if (play[c]) {									// it's false only if end of sample has reached, see above
						stopValue[c] = inputs[STOP_INPUT].getVoltage(c);
						if (stopValue[c] >= 1 && prevStopValue[c] < 1 && trigMode) {
							if (stage[c] != RELEASE_STAGE) {
								stage[c] = RELEASE_STAGE;
								currentStageSample[c] = 0;
								lastStageLevel[c] = 1-stageLevel[c];
							}
							if (c == currentDisplay) {
								eoc = true;
								eocTime = eocEorTime;
							}
						}
						prevStopValue[c] = stopValue[c];

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
								if (attackValue > 10) {
									attackValue = 10;
								} else 	if (attackValue < 0.0001f) {
									attackValue = 0.0001f;
								}
								maxStageSample[c] = args.sampleRate * attackValue;
								stageLevel[c] = (currentStageSample[c] / maxStageSample[c]) + lastStageLevel[c];
								if (stageLevel[c] > 1) {
									stageLevel[c] = 1;
									stage[c] = DECAY_STAGE;
									currentStageSample[c] = 0;
									lastStageLevel[c] = 0;
								}
								currentStageSample[c]++;
							break;

							case DECAY_STAGE:
								if (decayValue > 10) {
									decayValue = 10;
								} else 	if (decayValue < 0.0001f) {
									decayValue = 0.0001f;
								}
								maxStageSample[c] = args.sampleRate * decayValue / (1-sustainValue);
								stageLevel[c] = 1-(currentStageSample[c] / maxStageSample[c]) + lastStageLevel[c];
								if (stageLevel[c] < sustainValue) {
									stageLevel[c] = sustainValue;
									stage[c] = SUSTAIN_STAGE;
									currentStageSample[c] = 0;
									lastStageLevel[c] = 0;
								}
								currentStageSample[c]++;
							break;

							case SUSTAIN_STAGE:
								if (sustainValue > 1) {
									sustainValue = 1;
								} else 	if (sustainValue < 0) {
									sustainValue = 0;
								}
								stageLevel[c] = sustainValue;
							break;

							case RELEASE_STAGE:
								if (releaseValue > 10) {
									releaseValue = 10;
								} else 	if (releaseValue < 0.0001f) {
									releaseValue = 0.0001f;
								}
								maxStageSample[c] = args.sampleRate * releaseValue;
								stageLevel[c] = 1-(currentStageSample[c] / maxStageSample[c]) - lastStageLevel[c];
								if (stageLevel[c] < 0) {
									stageLevel[c] = 0;
									stage[c] = STOP_STAGE;
									play[c] = false;
									lastStageLevel[c] = 0;
									if (c == currentDisplay) {
										eor = true;
										eorTime = eocEorTime;
									}
									if (trigType == PLAY_PAUSE) {
										if (samplePos[c] > cueEndPos) {
											inPause[c] = false;
										}
									}
								}
								currentStageSample[c]++;
							break;
						}

						if (reversePlaying[c]) {
							//currentOutput *= stageLevel[c] * masterLevel * -5;
							currentOutput *= stageLevel[c] * masterLevel * -1;
							if (channels == 2)
								//currentOutputR *= stageLevel[c] * masterLevel * -5;
								currentOutputR *= stageLevel[c] * masterLevel * -1;
						} else {
							//currentOutput *= stageLevel[c] * masterLevel * 5;
							currentOutput *= stageLevel[c] * masterLevel;
							if (channels == 2)
								//currentOutputR *= stageLevel[c] * masterLevel * 5;
								currentOutputR *= stageLevel[c] * masterLevel;
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
											//currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * 5 * masterLevel * stageLevel[c]);
											currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel * stageLevel[c]);
											if (channels == 2) {
												currentOutputR *= 1 - fadingValue[c];
												//currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * 5 * masterLevel * stageLevel[c]);
												currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel * stageLevel[c]);
											}
											fadedPosition[c] += distancePos[c];
											if (fadedPosition[c] > totalSamples)
												fadingType[c] = NO_FADE;
										break;
										case REVERSE:
											currentOutput *= 1 - fadingValue[c];
											//currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * 5 * masterLevel * stageLevel[c] * -1);
											currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel * stageLevel[c] * -1);
											if (channels == 2) {
												currentOutputR *= 1 - fadingValue[c];
												//currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * 5 * masterLevel * stageLevel[c] * -1);
												currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel * stageLevel[c] * -1);
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
											//currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * 5 * masterLevel * stageLevel[c] * -1);
											currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel * stageLevel[c] * -1);
											if (channels == 2) {
												currentOutputR *= 1 - fadingValue[c];
												//currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * 5 * masterLevel * stageLevel[c] * -1);
												currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel * stageLevel[c] * -1);
											}
											fadedPosition[c] -= distancePos[c];
											if (fadedPosition[c] < 0)
												fadingType[c] = NO_FADE;
										break;
										case REVERSE:
											currentOutput *= 1 - fadingValue[c];
											//currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * 5 * masterLevel * stageLevel[c]);
											currentOutput += (playBuffer[LEFT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel * stageLevel[c]);
											if (channels == 2) {
												currentOutputR *= 1 - fadingValue[c];
												//currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * 5 * masterLevel * stageLevel[c]);
												currentOutputR += (playBuffer[RIGHT][antiAlias][floor(fadedPosition[c])] * fadingValue[c] * masterLevel * stageLevel[c]);
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

						if (outputs[OUT_OUTPUT].isConnected()) {
							outputs[OUT_OUTPUT].setVoltage(currentOutput, c);
							//outputs[OUT_OUTPUT+1].setVoltage(currentOutputR);           // ********** FOR FADE DEBUG
						}

						if (outputs[OUT_OUTPUT+1].isConnected()) {
							if (channels == 2)
								outputs[OUT_OUTPUT+1].setVoltage(currentOutputR, c);
							else
								outputs[OUT_OUTPUT+1].setVoltage(currentOutput, c);
						}
					break;
				}

				if (!inputs[TRIG_INPUT].isConnected()) {
					play[c] = false;
					inPause[c] = false;
					fadingType[c] = NO_FADE;
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

			if (eoc) {
				eocTime--;
				if (eocTime < 0) {
					eoc = false;
					outputs[EOC_OUTPUT].setVoltage(0.f);
				} else
					outputs[EOC_OUTPUT].setVoltage(10.f);
			}

			if (eor) {
				eorTime--;
				if (eorTime < 0) {
					eor = false;
					outputs[EOR_OUTPUT].setVoltage(0.f);
				} else
					outputs[EOR_OUTPUT].setVoltage(10.f);
			}
		}

	}
};

struct SickoPlayerOpenFolder : MenuItem {
	SickoPlayer *rm ;
	void onAction(const event::Action &e) override {
		const char* prevFolder = rm->userFolderName.c_str();
		char *path = osdialog_file(OSDIALOG_OPEN_DIR, prevFolder, NULL, NULL);
		if (path) {
			rm->userFolderName = std::string(path);
			DIR* rep = NULL;
			struct dirent* dirp = NULL;
				char* pathDup = path;
				std::string dir = pathDup;

			rep = opendir(dir.c_str());
			rm->browserFileName.clear();
			rm->browserFileDisplay.clear();
			while ((dirp = readdir(rep)) != NULL) {
				std::string name = dirp->d_name;

				std::size_t found = name.find(".wav",name.length()-5);
				if (found==std::string::npos)
					found = name.find(".WAV",name.length()-5);

				if (found!=std::string::npos) {
					rm->browserFileName.push_back(name);
					rm->browserFileDisplay.push_back(name.substr(0, name.length()-4));
				}
			}
			sort(rm->browserFileName.begin(), rm->browserFileName.end());
			sort(rm->browserFileDisplay.begin(), rm->browserFileDisplay.end());
			closedir(rep);
		}
		free(path);
	}
};

struct SickoPlayerItem : MenuItem {
	SickoPlayer *rm ;
	void onAction(const event::Action &e) override {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV;All files (*.*):*.*";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
		rm->fileLoaded = false;
		if (path) {
			rm->loadSample(path);
			rm->storedPath = std::string(path);
		} else {
			rm->fileLoaded = true;
		}
		if (rm->storedPath == "") {
			rm->fileLoaded = false;
		}
		free(path);
	}
};

struct SickoPlayerDisplay : TransparentWidget {
	SickoPlayer *module;
	int frame = 0;
	SickoPlayerDisplay() {
	}

	struct ClearSlotItem : MenuItem {
		SickoPlayer *module;
		void onAction(const event::Action &e) override {
			module->clearSlot();
		}
	};

	struct ResetCursorsItem : MenuItem {
		SickoPlayer *module;
		void onAction(const event::Action &e) override {
			module->resetCursors();
		}
	};

	struct SetPreset0 : MenuItem {
		SickoPlayer *module;
		void onAction(const event::Action &e) override {
			module->setPreset(0);
		}
	};

	struct SetPreset1 : MenuItem {
		SickoPlayer *module;
		void onAction(const event::Action &e) override {
			module->setPreset(1);
		}
	};

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);

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
					nvgMoveTo(args.vg, 7, 59.5);
					nvgLineTo(args.vg, 242, 59.5);
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
						//y = module->displayBuff[i] / 2.0 + 0.5;
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

	void createContextMenu() {
		SickoPlayer *module = dynamic_cast<SickoPlayer *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			SickoPlayerItem *rootDirItem = new SickoPlayerItem;
				rootDirItem->text = "Load Sample";
				rootDirItem->rm = module;
				menu->addChild(rootDirItem);

				if (module->browserFileName.size() > 0) {
					menu->addChild(createSubmenuItem("Folder Browser", "",
						[=](Menu* menu) {
							for (unsigned int i = 0; i < module->browserFileName.size(); i++) {
								//menu->addChild(createMenuItem(module->browserFileDisplay[i], "", [=]() {module->loadSample(module->userFolderName+"\\"+ module->browserFileName[i]);}));
								menu->addChild(createMenuItem(module->browserFileDisplay[i], "", [=]() {module->loadSample(module->userFolderName+"/"+ module->browserFileName[i]);}));
							}
						}
					));
				}

				if (module->fileLoaded) {
					menu->addChild(new MenuSeparator());
					menu->addChild(createMenuLabel("Current Sample:"));
					menu->addChild(createMenuLabel(module->fileDescription));
					menu->addChild(createMenuLabel(" " + module->samplerateDisplay + " - " + std::to_string(module->channels) + "ch"));
					menu->addChild(construct<ClearSlotItem>(&MenuItem::rightText, "Clear", &ClearSlotItem::module, module));
				}



			menu->addChild(new MenuSeparator());
			menu->addChild(construct<ResetCursorsItem>(&MenuItem::text, "Reset Cursors", &ResetCursorsItem::module, module));

			menu->addChild(createSubmenuItem("Presets", "",
				[ = ](Menu * menu) {
					menu->addChild(construct<SetPreset0>(&MenuItem::text, "Wavetable", &SetPreset0::module, module));
					menu->addChild(construct<SetPreset1>(&MenuItem::text, "Triggered Sample with Envelope", &SetPreset1::module, module));
				}));
		}
	}
};

struct SickoPlayerWidget : ModuleWidget {
	SickoPlayerWidget(SickoPlayer *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SickoPlayer.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		{
			SickoPlayerDisplay *display = new SickoPlayerDisplay();
			display->box.pos = Vec(3, 24);
			display->box.size = Vec(247, 100);
			display->module = module;
			addChild(display);
		}

		float xTrig1 = 11;
		float xTrig2 = 29;
		float yTrig1 = 50;
		float yTrig2 = 64; 
		float xStart1 = 50;
		float xStart2 = 62;
		float yStart1 = 53.5;
		float yStart2 = 63.5;

		float xEnv1 = 11.5f;
		float xEnv1Add = 21.f;
		float xEnv2 = 6.5f;
		float xEnv2Add = 10.f;
		float xEnv2Skip = 21.f;
		float yEnv1 = 81.f;
		float yEnv2 = 90.f;
		
		float yTunVol = 108;
		float yTunVol2 = 117.5;

		addParam(createParamCentered<CKSS>(mm2px(Vec(xTrig1, yTrig1)), module, SickoPlayer::TRIGGATEMODE_SWITCH));
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(xTrig2, yTrig1+1)), module, SickoPlayer::TRIGMODE_SWITCH));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xTrig1, yTrig2)), module, SickoPlayer::TRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xTrig2, yTrig2)), module, SickoPlayer::STOP_INPUT));
		//----------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xStart1, yStart1)), module, SickoPlayer::CUESTART_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xStart2, yStart1)), module, SickoPlayer::CUEEND_PARAM));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xStart1, yStart2)), module, SickoPlayer::LOOPSTART_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xStart2, yStart2)), module, SickoPlayer::LOOPEND_PARAM));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xStart2+9.8, yStart1-1.5)), module, SickoPlayer::REV_PARAM, SickoPlayer::REV_LIGHT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(xStart2 + 18, yStart1-1)), module, SickoPlayer::XFADE_PARAM));;

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xStart2+10, yStart2-2)), module, SickoPlayer::LOOP_PARAM, SickoPlayer::LOOP_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xStart2+18, yStart2+0.5)), module, SickoPlayer::PINGPONG_PARAM, SickoPlayer::PINGPONG_LIGHT));
		//----------------------------------------------------------------------------------------------------------------------------

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xEnv1, yEnv1)), module, SickoPlayer::ATTACK_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xEnv2, yEnv2)), module, SickoPlayer::ATTACK_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(xEnv2+xEnv2Add, yEnv2)), module, SickoPlayer::ATTACKATNV_PARAM));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xEnv1+xEnv1Add, yEnv1)), module, SickoPlayer::DECAY_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xEnv2+xEnv2Skip, yEnv2)), module, SickoPlayer::DECAY_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(xEnv2+xEnv2Add+xEnv2Skip, yEnv2)), module, SickoPlayer::DECAYATNV_PARAM));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xEnv1+xEnv1Add*2, yEnv1)), module, SickoPlayer::SUSTAIN_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xEnv2+xEnv2Skip*2, yEnv2)), module, SickoPlayer::SUSTAIN_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(xEnv2+xEnv2Add+xEnv2Skip*2, yEnv2)), module, SickoPlayer::SUSTAINATNV_PARAM));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xEnv1+xEnv1Add*3, yEnv1)), module, SickoPlayer::RELEASE_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xEnv2+xEnv2Skip*3, yEnv2)), module, SickoPlayer::RELEASE_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(xEnv2+xEnv2Add+xEnv2Skip*3, yEnv2)), module, SickoPlayer::RELEASEATNV_PARAM));
		//----------------------------------------------------------------------------------------------------------------------------

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9, yTunVol+2.5)), module, SickoPlayer::VO_INPUT));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(23, yTunVol)), module, SickoPlayer::TUNE_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18, yTunVol2)), module, SickoPlayer::TUNE_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(28, yTunVol2)), module, SickoPlayer::TUNEATNV_PARAM));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.5, yTunVol)), module, SickoPlayer::VOL_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(40.5, yTunVol2)), module, SickoPlayer::VOL_INPUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(50.5, yTunVol2)), module, SickoPlayer::VOLATNV_PARAM));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(53, yTunVol-4.5)), module, SickoPlayer::CLIPPING_LIGHT));

		addParam(createParamCentered<CKSS>(mm2px(Vec(59.3, 113)), module, SickoPlayer::LIMIT_SWITCH));
		//----------------------------------------------------------------------------------------------------------------------------

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(70.2, 105.3)), module, SickoPlayer::OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(80.2, 105.3)), module, SickoPlayer::OUT_OUTPUT+1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(70.2, 117.5)), module, SickoPlayer::EOC_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(80.2, 117.5)), module, SickoPlayer::EOR_OUTPUT));
	}

	struct ClearSlotItem : MenuItem {
		SickoPlayer *module;
		void onAction(const event::Action &e) override {
			module->clearSlot();
		}
	};

	struct ResetCursorsItem : MenuItem {
		SickoPlayer *module;
		void onAction(const event::Action &e) override {
			module->resetCursors();
		}
	};

	struct SetPreset0 : MenuItem {
		SickoPlayer *module;
		void onAction(const event::Action &e) override {
			module->setPreset(0);
		}
	};

	struct SetPreset1 : MenuItem {
		SickoPlayer *module;
		void onAction(const event::Action &e) override {
			module->setPreset(1);
		}
	};

	struct RefreshUserFolderItem : MenuItem {
		SickoPlayer *module;
		void onAction(const event::Action &e) override {
			module->folderSelection(module->userFolderName);
		}
	};

	void appendContextMenu(Menu *menu) override {
	   	SickoPlayer *module = dynamic_cast<SickoPlayer*>(this->module);
			assert(module);
			menu->addChild(new MenuSeparator());

		SickoPlayerItem *rootDirItem = new SickoPlayerItem;
			rootDirItem->text = "Load Sample";
			rootDirItem->rm = module;
			menu->addChild(rootDirItem);

			if (module->browserFileName.size() > 0) {
				menu->addChild(createSubmenuItem("Folder Browser", "",
					[=](Menu* menu) {
						for (unsigned int i = 0; i < module->browserFileName.size(); i++) {
							//menu->addChild(createMenuItem(module->browserFileDisplay[i], "", [=]() {module->loadSample(module->userFolderName+"\\"+ module->browserFileName[i]);}));
							menu->addChild(createMenuItem(module->browserFileDisplay[i], "", [=]() {module->loadSample(module->userFolderName+"/"+ module->browserFileName[i]);}));
						}
					}
				));
			}

			if (module->fileLoaded) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription));
				menu->addChild(createMenuLabel(" " + module->samplerateDisplay + " - " + std::to_string(module->channels) + "ch"));
				menu->addChild(construct<ClearSlotItem>(&MenuItem::rightText, "Clear", &ClearSlotItem::module, module));
			}

		menu->addChild(new MenuSeparator());
		SickoPlayerOpenFolder *rootFolder = new SickoPlayerOpenFolder;
			rootFolder->text = "Select Samples Folder";
			rootFolder->rm = module;
			menu->addChild(rootFolder);
		if (module->userFolderName != "") {
			menu->addChild(createMenuLabel(module->userFolderName));
			menu->addChild(construct<RefreshUserFolderItem>(&MenuItem::rightText, "Refresh", &RefreshUserFolderItem::module, module));
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Interpolation"));
		struct ModeItem : MenuItem {
			SickoPlayer* module;
			int interpolationMode;
			void onAction(const event::Action& e) override {
				module->interpolationMode = interpolationMode;
			}
		};

		std::string modeNames[4] = {"None", "Linear 1", "Linear 2", "Hermite"};
		for (int i = 0; i < 4; i++) {
			ModeItem* modeItem = createMenuItem<ModeItem>(modeNames[i]);
			modeItem->rightText = CHECKMARK(module->interpolationMode == i);
			modeItem->module = module;
			modeItem->interpolationMode = i;
			menu->addChild(modeItem);
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Anti-aliasing filter", "", &module->antiAlias));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Polyphonic outs", "", &module->polyOuts));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Phase scan", "", &module->phaseScan));

		menu->addChild(new MenuSeparator());
		menu->addChild(construct<ResetCursorsItem>(&MenuItem::text, "Reset Cursors", &ResetCursorsItem::module, module));
		
		menu->addChild(createSubmenuItem("Presets", "",
			[ = ](Menu * menu) {
				menu->addChild(construct<SetPreset0>(&MenuItem::text, "Wavetable", &SetPreset0::module, module));
				menu->addChild(construct<SetPreset1>(&MenuItem::text, "Triggered Sample with Envelope", &SetPreset1::module, module));
			}));
	}
};

Model *modelSickoPlayer = createModel<SickoPlayer, SickoPlayerWidget>("SickoPlayer");
