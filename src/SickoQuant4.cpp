#define COLOR_LCD_RED 0xdd, 0x33, 0x33, 0xff
#define COLOR_LCD_GREEN 0x33, 0xdd, 0x33, 0xff

#define DISPLAYSCALE 0
#define DISPLAYPROG 1
#define CUSTOMSCALE 2

#define SUM_OFF 0
#define SUM_12 1
#define SUM_123 2
#define SUM_1234 3

#define CV_TYPE 0

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

struct tpScaleSQ4 : ParamQuantity {
	std::string getDisplayValueString() override {
		int value = abs(getValue());
		std::string outValue;

		if (value > 11)
			outValue = "B"; 
		else if (value > 10)
			outValue = "A#";
		else if (value > 9)
			outValue = "A";
		else if (value > 8)
			outValue = "G#";
		else if (value > 7)
			outValue = "G";
		else if (value > 6)
			outValue = "F#";
		else if (value > 5)
			outValue = "F";
		else if (value > 4)
			outValue = "E";
		else if (value > 3)
			outValue = "D#";
		else if (value > 2)
			outValue = "D";
		else if (value > 1)
			outValue = "C#";
		else if (value > 0)
			outValue = "C";
		else if (value == 0)
			outValue = "chrom.";
		else
			outValue = std::to_string(value);

		if (getValue() < 0)
			outValue += "m";

		return outValue;
	}
};

struct SickoQuant4 : Module {
	enum ParamId {
		ENUMS(NOTE_PARAM, 12),
		ENUMS(ATN_PARAM,4),
		ENUMS(OCT_PARAM,4),
		MANUALSET_PARAM,
		SCALE_PARAM,
		RESETSCALE_PARAM,
		AUTO_PARAM,
		PROG_PARAM,
		STORE_PARAM,
		RECALL_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SCALE_INPUT,
		PROG_INPUT,
		ENUMS(TRIG_INPUT, 4),
		ENUMS(IN_INPUT, 4),
		ENUMS(OFFS_INPUT, 4),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT, 4),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(NOTE_LIGHT, 12),
		MANUALSET_LIGHT,
		RESETSCALE_LIGHT,
		AUTO_LIGHT,
		RECALL_LIGHT,
		STORE_LIGHT,
		LIGHTS_LEN
	};

	const float noteVoltage = 1/12;
	const float noteVtable[12] = {0, 0.0833333f, 0.1666667f, 0.25f, 0.3333333f, 0.4166667, 0.5f, 0.5833333f, 0.6666667f, 0.75f, 0.8333333f, 0.9166667f};
	const std::string noteName[13] = {"chr","C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	const std::string scaleDisplay[2] = {"m", ""};
	const int scalesNotes[2][13][12] =	{	
												{
													{1,1,1,1,1,1,1,1,1,1,1,1},
													{1,0,1,1,0,1,0,1,1,0,1,0},	// C minor
													{0,1,0,1,1,0,1,0,1,1,0,1},	// C# minor
													{1,0,1,0,1,1,0,1,0,1,1,0},	// D minor
													{0,1,0,1,0,1,1,0,1,0,1,1},	// D# minor
													{1,0,1,0,1,0,1,1,0,1,0,1},	// D# minor
													{1,1,0,1,0,1,0,1,1,0,1,0},	// E minor
													{0,1,1,0,1,0,1,0,1,1,0,1},	// F minor
													{1,0,1,1,0,1,0,1,0,1,1,0},	// F# minor
													{0,1,0,1,1,0,1,0,1,0,1,1},	// G# minor
													{1,0,1,0,1,1,0,1,0,1,0,1},	// A minor
													{1,1,0,1,0,1,1,0,1,0,1,0},	// A# minor
													{0,1,1,0,1,0,1,1,0,1,0,1}	// B minor
												},
												{
													{1,1,1,1,1,1,1,1,1,1,1,1},
													{1,0,1,0,1,1,0,1,0,1,0,1},	// C major
													{1,1,0,1,0,1,1,0,1,0,1,0},	// C# major
													{0,1,1,0,1,0,1,1,0,1,0,1},	// D Major
													{1,0,1,1,0,1,0,1,1,0,1,0},	// D# Major
													{0,1,0,1,1,0,1,0,1,1,0,1},	// E Major
													{1,0,1,0,1,1,0,1,0,1,1,0},	// F Major
													{0,1,0,1,0,1,1,0,1,0,1,1},	// F# Major
													{1,0,1,0,1,0,1,1,0,1,0,1},	// G Major
													{1,1,0,1,0,1,0,1,1,0,1,0},	// G# Major
													{0,1,1,0,1,0,1,0,1,1,0,1},	// A Major
													{1,0,1,1,0,1,0,1,0,1,1,0},	// A# Major
													{0,1,0,1,1,0,1,0,1,0,1,1}	// B Major
												}
											};

	int progNotes[32][12] = {
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},

								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},

								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0}
							};

	int note[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	int nextNote[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	int changeNote[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	float voltageScale[12] = {0, 0.08333f, 0.16667f, 0.25f, 0.33333f, 0.41667, 0.5f, 0.58333f, 0.66667f, 0.75f, 0.83333f, 0.91667f};
	int notesInVoltageScale = 0;
	int lastNoteInVoltageScale = -1;
	float noteDelta = 0.f;
	float noteDeltaMin = 5.f;
	int chosenNote = 0;

	// --------------scale
	int scaleKnob = 0;
	int prevScaleKnob = 0;
	int savedScaleKnob = 0;

	int selectedScale = 0;
	int selectedMinMaj = 0;
	bool scaleChanged = false;

	// --------------prog
	int progKnob = 0;
	int prevProgKnob = 0;
	int savedProgKnob = 0;

	float progTrig = 0;
	float prevProgTrig = 0;

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
	int workingScale = 0;
	int workingMinMaj = 0;
	int workingProg = 0;
	
	bool instantScaleChange = false;

	bool butSetScale = false;
	float scaleSetBut = 0;
	float prevScaleSetBut = 0;

	float resetScale = 0;
	float prevResetScale = 0;

	//bool pendingUpdate = false;

	int displayWorking = CUSTOMSCALE;
	int displayLastSelected = CUSTOMSCALE;

	int chan = 0;
	float atten = 0;
	float octPostValue = 0;

	float inSignal[4][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}};
	float atnSignal[4][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}};
	float outSignal[4][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}};
	float octSignal[4][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}};
	float noteSignal[4][16] = {{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}};

	float quantTrig[4] = {0.f, 0.f, 0.f, 0.f};
	float prevQuantTrig[4] = {0.f, 0.f, 0.f, 0.f};

	int outSumMode = 0;
	int outSumModePlus1 = 1;
	int chanSum = 1;
	float outSum = 0;

	// ------- set button light

	bool setButLight = false;
	float setButLightDelta = 2 / APP->engine->getSampleRate();
	float setButLightValue = 0.f;

	int progInType = CV_TYPE;
	int lastProg = 0;

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

	SickoQuant4() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configSwitch(NOTE_PARAM+0, 0.f, 1.f, 0.f, "C", {"OFF", "ON"});
		configSwitch(NOTE_PARAM+1, 0.f, 1.f, 0.f, "C#", {"OFF", "ON"});
		configSwitch(NOTE_PARAM+2, 0.f, 1.f, 0.f, "D", {"OFF", "ON"});
		configSwitch(NOTE_PARAM+3, 0.f, 1.f, 0.f, "D#", {"OFF", "ON"});
		configSwitch(NOTE_PARAM+4, 0.f, 1.f, 0.f, "E", {"OFF", "ON"});
		configSwitch(NOTE_PARAM+5, 0.f, 1.f, 0.f, "F", {"OFF", "ON"});
		configSwitch(NOTE_PARAM+6, 0.f, 1.f, 0.f, "F#", {"OFF", "ON"});
		configSwitch(NOTE_PARAM+7, 0.f, 1.f, 0.f, "G", {"OFF", "ON"});
		configSwitch(NOTE_PARAM+8, 0.f, 1.f, 0.f, "G#", {"OFF", "ON"});
		configSwitch(NOTE_PARAM+9, 0.f, 1.f, 0.f, "A", {"OFF", "ON"});
		configSwitch(NOTE_PARAM+10, 0.f, 1.f, 0.f, "A#", {"OFF", "ON"});
		configSwitch(NOTE_PARAM+11, 0.f, 1.f, 0.f, "B", {"OFF", "ON"});

		configSwitch(MANUALSET_PARAM, 0, 1.f, 0.f, "Set", {"OFF", "ON"});

		configInput(SCALE_INPUT, "Scale");
		configParam<tpScaleSQ4>(SCALE_PARAM, -12, 12.f, 0.f, "Scale");
		paramQuantities[SCALE_PARAM]->snapEnabled = true;
		configSwitch(RESETSCALE_PARAM, 0, 1.f, 0.f, "Set Scale", {"OFF", "ON"});

		configSwitch(AUTO_PARAM, 0, 1.f, 0.f, "Auto", {"OFF", "ON"});

		configInput(PROG_INPUT, "Prog");
		configParam(PROG_PARAM, 0.f, 31.f, 0.f, "Prog");
		paramQuantities[PROG_PARAM]->snapEnabled = true;
		configSwitch(RECALL_PARAM, 0, 1.f, 0.f, "Recall", {"OFF", "ON"});
		configSwitch(STORE_PARAM, 0, 1.f, 0.f, "Store", {"OFF", "ON"});

		configInput(TRIG_INPUT, "Trig #1");
		configInput(TRIG_INPUT+1, "Trig #2");
		configInput(TRIG_INPUT+2, "Trig #3");
		configInput(TRIG_INPUT+3, "Trig #4");
		
		configInput(IN_INPUT, "Signal #1");
		configInput(IN_INPUT+1, "Signal #2");
		configInput(IN_INPUT+2, "Signal #3");
		configInput(IN_INPUT+3, "Signal #4");

		configParam(ATN_PARAM, 0.f, 1.f, 1.f, "Att. #1", "%", 0, 100);
		configParam(ATN_PARAM+1, 0.f, 1.f, 1.f, "Att. #2", "%", 0, 100);
		configParam(ATN_PARAM+2, 0.f, 1.f, 1.f, "Att. #3", "%", 0, 100);
		configParam(ATN_PARAM+3, 0.f, 1.f, 1.f, "Att. #4", "%", 0, 100);

		configInput(OFFS_INPUT, "Offset #1");
		configInput(OFFS_INPUT+1, "Offset #2");
		configInput(OFFS_INPUT+2, "Offset #3");
		configInput(OFFS_INPUT+3, "Offset #4");

		configParam(OCT_PARAM, -3.f, 3.f, 0.f, "Octave #1");
		paramQuantities[OCT_PARAM]->snapEnabled = true;
		configParam(OCT_PARAM+1, -3.f, 3.f, 0.f, "Octave #2");
		paramQuantities[OCT_PARAM+1]->snapEnabled = true;
		configParam(OCT_PARAM+2, -3.f, 3.f, 0.f, "Octave #3");
		paramQuantities[OCT_PARAM+2]->snapEnabled = true;
		configParam(OCT_PARAM+3, -3.f, 3.f, 0.f, "Octave #4");
		paramQuantities[OCT_PARAM+3]->snapEnabled = true;

		configOutput(OUT_OUTPUT, "Signal #1");
		configOutput(OUT_OUTPUT+1, "Signal #2");
		configOutput(OUT_OUTPUT+2, "Signal #3");
		configOutput(OUT_OUTPUT+3, "Signal #4");
		
	}

	void processBypass(const ProcessArgs &e) override {
		for (int track = 0; track < 4; track++) {
			if (inputs[IN_INPUT+track].isConnected())
				chan = inputs[IN_INPUT+track].getChannels();
			else if (track == 0)
				chan = 1;

			for (int c = 0; c < chan; c++)
				outputs[OUT_OUTPUT+track].setVoltage(inputs[IN_INPUT+track].getVoltage(c), c);
			outputs[OUT_OUTPUT+track].setChannels(chan);
		}
		Module::processBypass(e);
	}

	void onReset(const ResetEvent &e) override {
		for (int i = 0; i < 12; i++) {
			note[i] = 0;
			//pendingNote[i] = 0;
			nextNote[i] = 0;
			changeNote[i] = 0;
			voltageScale[i] = noteVtable[i];
		}

		notesInVoltageScale = 0;
		lastNoteInVoltageScale = -1;

		scaleKnob = 0;
		prevScaleKnob = 0;
		savedScaleKnob = 0;

		selectedScale = 0;
		selectedMinMaj = 0;
		scaleChanged = false;

		progKnob = 0;
		prevProgKnob = 0;
		savedProgKnob = 0;

		selectedProg = 0;
		progChanged = false;

		workingScale = 0;
		workingMinMaj = 0;
		workingProg = 0;

		displayWorking = CUSTOMSCALE;
		displayLastSelected = CUSTOMSCALE;

		outSumMode = 0;
		outSumModePlus1 = 1;

		setButLight = false;
		setButLightDelta = 2 / APP->engine->getSampleRate();
		setButLightValue = 0.f;
		Module::onReset(e);
	}

	void onSampleRateChange() override {

		storeSamples = APP->engine->getSampleRate() / 1.5f;
		maxStoredProgramTime = APP->engine->getSampleRate() * 1.5;
		setButLightDelta = 2 / APP->engine->getSampleRate();

	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_object_set_new(rootJ, "progInType", json_boolean(progInType));
		json_object_set_new(rootJ, "lastProg", json_integer(lastProg));

		json_object_set_new(rootJ, "outSumMode", json_integer(outSumMode));

		json_object_set_new(rootJ, "displayWorking", json_integer(displayWorking));
		
		json_object_set_new(rootJ, "savedScaleKnob", json_integer(savedScaleKnob));
		json_object_set_new(rootJ, "savedProgKnob", json_integer(savedProgKnob));

		json_t *note_json_array = json_array();
		for (int notes = 0; notes < 12; notes++) {
			json_array_append_new(note_json_array, json_integer(note[notes]));
		}
		json_object_set_new(rootJ, "note", note_json_array);	
		
		for (int prog = 0; prog < 32; prog++) {
			json_t *prog_json_array = json_array();
			for (int notes = 0; notes < 12; notes++) {
				json_array_append_new(prog_json_array, json_integer(progNotes[prog][notes]));
			}
			json_object_set_new(rootJ, ("prog"+to_string(prog)).c_str(), prog_json_array);	
		}
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {

		json_t* progInTypeJ = json_object_get(rootJ, "progInType");
		if (progInTypeJ) {
			progInType = json_boolean_value(progInTypeJ);
		}

		json_t* lastProgJ = json_object_get(rootJ, "lastProg");
		if (lastProgJ) {
			lastProg = json_integer_value(lastProgJ);
			if (lastProg < 0 || lastProg > 31)
				lastProg = 0;
		}

		json_t* outSumModeJ = json_object_get(rootJ, "outSumMode");
		if (outSumModeJ) {
			outSumMode = json_integer_value(outSumModeJ);
			if (outSumMode < 0 || outSumMode > 3)
				outSumMode = SUM_OFF;
			outSumModePlus1 = outSumMode + 1;
		}

		for (int prog = 0; prog < 32; prog++) {
			json_t *prog_json_array = json_object_get(rootJ, ("prog"+to_string(prog)).c_str());
			size_t notes;
			json_t *json_value;
			if (prog_json_array) {
				json_array_foreach(prog_json_array, notes, json_value) {
					progNotes[prog][notes] = json_integer_value(json_value);
				}
			}
		}

		json_t* displayWorkingJ = json_object_get(rootJ, "displayWorking");
		if (displayWorkingJ) {
			displayWorking = json_integer_value(displayWorkingJ);
			if (displayWorking < 0 || displayWorking > 2)
				displayWorking = DISPLAYSCALE;
		} else {
			displayWorking = DISPLAYSCALE;
		}

		json_t* savedScaleKnobJ = json_object_get(rootJ, "savedScaleKnob");
		if (savedScaleKnobJ) {
			savedScaleKnob = json_integer_value(savedScaleKnobJ);
			if (savedScaleKnob < 0)
				workingMinMaj = 0;
			else
				workingMinMaj = 1;
			selectedMinMaj = workingMinMaj;
			prevScaleKnob = savedScaleKnob;
			workingScale = abs(savedScaleKnob);
			selectedScale = workingScale;

		} else {
			workingScale = 0;
			workingMinMaj = 0;
			selectedMinMaj = 0;
			prevScaleKnob = 0;
		}

		params[SCALE_PARAM].setValue(prevScaleKnob);
		
		json_t* savedProgKnobJ = json_object_get(rootJ, "savedProgKnob");
		if (savedProgKnobJ) {
			savedProgKnob = json_integer_value(savedProgKnobJ);
			if (savedProgKnob < 0 || savedProgKnob > 31)
				savedProgKnob = 0;
			
		} else {
			savedProgKnob = 0;
		}

		workingProg = savedProgKnob;
		prevProgKnob = workingProg;
		selectedProg = workingProg;
		params[PROG_PARAM].setValue(workingProg);
	
		// --------------------------------------------
		
		if (displayWorking == DISPLAYSCALE) {
			notesInVoltageScale = 0;
			for (int i = 0; i < 12; i++) {
				note[i] = scalesNotes[selectedMinMaj][selectedScale][i];
				nextNote[i] = note[i];
				changeNote[i] = note[i];
				if (note[i]) {
					voltageScale[notesInVoltageScale] = noteVtable[i];
					notesInVoltageScale++;
				}
				params[NOTE_PARAM+i].setValue(note[i]);
				lights[NOTE_LIGHT+i].setBrightness(note[i]);
			}
			lastNoteInVoltageScale = notesInVoltageScale - 1;

			
		} else if (displayWorking == DISPLAYPROG) {
			notesInVoltageScale = 0;
			for (int i = 0; i < 12; i++) {
				note[i] = progNotes[selectedProg][i];
				nextNote[i] = note[i];
				changeNote[i] = note[i];
				if (note[i]) {
					voltageScale[notesInVoltageScale] = noteVtable[i];
					notesInVoltageScale++;
				}
				params[NOTE_PARAM+i].setValue(note[i]);
				lights[NOTE_LIGHT+i].setBrightness(note[i]);
			}
			lastNoteInVoltageScale = notesInVoltageScale - 1;

		} else {

			json_t *note_json_array = json_object_get(rootJ, "note");
			size_t notes;
			json_t *json_value;
			if (note_json_array) {
				json_array_foreach(note_json_array, notes, json_value) {
					note[notes] = json_integer_value(json_value);
				}
			}

			notesInVoltageScale = 0;
			for (int i = 0; i < 12; i++) {
				nextNote[i] = note[i];
				changeNote[i] = note[i];
				if (note[i]) {
					voltageScale[notesInVoltageScale] = noteVtable[i];
					notesInVoltageScale++;
				}
				params[NOTE_PARAM+i].setValue(note[i]);
				lights[NOTE_LIGHT+i].setBrightness(note[i]);
			}
			lastNoteInVoltageScale = notesInVoltageScale - 1;
		}
	}

	json_t *presetToJson() {
		json_t *rootJ = json_object();
		for (int prog = 0; prog < 32; prog++) {
			json_t *prog_json_array = json_array();
			for (int notes = 0; notes < 12; notes++) {
				json_array_append_new(prog_json_array, json_integer(progNotes[prog][notes]));
			}
			json_object_set_new(rootJ, ("prog"+to_string(prog)).c_str(), prog_json_array);	
		}
		return rootJ;
	}

	void presetFromJson(json_t *rootJ) {
		for (int prog = 0; prog < 32; prog++) {
			json_t *prog_json_array = json_object_get(rootJ, ("prog"+to_string(prog)).c_str());
			size_t notes;
			json_t *json_value;
			if (prog_json_array) {
				json_array_foreach(prog_json_array, notes, json_value) {
					progNotes[prog][notes] = json_integer_value(json_value);
				}
			}
		}
	}

	void menuLoadPreset() {
		static const char FILE_FILTERS[] = "sickoQuant preset (.sqn):sqn,SQN";
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

		static const char FILE_FILTERS[] = "sickoQuant preset (.sqn):sqn,SQN";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".sqn" and strPath.substr(strPath.size() - 4) != ".SQN")
				strPath += ".sqn";
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

	void eraseProgs() {
		for (int p = 0; p < 32; p++)
			for (int n = 0; n < 12; n++)
				progNotes[p][n] = 0;

		lastProg = 0;
	}

	void scanLastProg() {
		lastProg = 31;
		bool exitFunc = false;

		for (int p = 31; p >= 0; p--) {
			for (int n = 0; n < 12; n++) {
				if (progNotes[p][n] != 0) {
					n = 12;
					exitFunc = true;
				}
			}

			lastProg = p;
			
			if (exitFunc)
				p = 0;
		}
	}

	void process(const ProcessArgs& args) override {

		// ----------- AUTO SWITCH

		instantScaleChange = int(params[AUTO_PARAM].getValue());
		lights[AUTO_LIGHT].setBrightness(instantScaleChange);

		// ----------- PROGRAM MANAGEMENT

		if (progInType == CV_TYPE) {

			progKnob = int(params[PROG_PARAM].getValue() + (inputs[PROG_INPUT].getVoltage() * 3.2));
			if (progKnob < 0)
				progKnob = 0;
			else if (progKnob > 31)
				progKnob = 31;

		} else {

			progKnob = params[PROG_PARAM].getValue();
			if (progKnob < 0)
				progKnob = 0;
			else if (progKnob > 31)
				progKnob = 31;

			progTrig = inputs[PROG_INPUT].getVoltage();
			if (progTrig >= 1.f && prevProgTrig < 1.f) {
				progKnob++;
				if (progKnob > lastProg)
					progKnob = 0;

				params[PROG_PARAM].setValue(progKnob);
			}
			prevProgTrig = progTrig;

		}

		if (progKnob != prevProgKnob) {
			progChanged = true;
			displayLastSelected = DISPLAYPROG;
			if (scaleChanged)
				scaleChanged = false;
			selectedProg = progKnob;
			prevProgKnob = progKnob;

			for (int i = 0; i < 12; i++) {
				nextNote[i] = progNotes[selectedProg][i];
				changeNote[i] = nextNote[i];
				params[NOTE_PARAM+i].setValue(nextNote[i]);
			}

			setButLight = true;
			setButLightValue = 0.f;
		}
		
		// ----------- SCALE MANAGEMENT

		scaleKnob = int(params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * 1.2));
		
		if (scaleKnob < -12)
			scaleKnob = -12;
		else if (scaleKnob > 12)
			scaleKnob = 12;

		if (scaleKnob != prevScaleKnob) {
			selectedScale = abs(scaleKnob);
			if (scaleKnob < 0)
				selectedMinMaj = 0;
			else
				selectedMinMaj = 1;
			prevScaleKnob = scaleKnob;

			for (int i = 0; i < 12; i++) {
				nextNote[i] = scalesNotes[selectedMinMaj][selectedScale][i];
				changeNote[i] = nextNote[i];
				params[NOTE_PARAM+i].setValue(nextNote[i]);
			}

			scaleChanged = true;
			if (progChanged)
				progChanged = false;

			displayLastSelected = DISPLAYSCALE;

			setButLight = true;
			setButLightValue = 0.f;
		}

		// -------- CURRENT SCALE UPDATE

		butSetScale = false;

		scaleSetBut = params[MANUALSET_PARAM].getValue();

		if (scaleSetBut >= 1.f && prevScaleSetBut < 1.f)
			butSetScale = true;

		prevScaleSetBut = scaleSetBut;

		if (!progChanged && !scaleChanged) {

			notesInVoltageScale = 0;
			for (int i = 0; i < 12; i++) {
				note[i] = params[NOTE_PARAM+i].getValue();
				lights[NOTE_LIGHT+i].setBrightness(note[i]);

				if (note[i]) {
					voltageScale[notesInVoltageScale] = noteVtable[i];
					notesInVoltageScale++;
				}

				nextNote[i] = note[i];

				if (note[i] != changeNote[i]) {		/// CHANGE NOTE
					changeNote[i] = note[i];
					displayWorking = CUSTOMSCALE;
					displayLastSelected = CUSTOMSCALE;
				}
			}
			lastNoteInVoltageScale = notesInVoltageScale - 1;

		} else {
			
			if (instantScaleChange || butSetScale) {
				notesInVoltageScale = 0;
				for (int i = 0; i < 12; i++) {
					note[i] = nextNote[i];
					changeNote[i] = note[i];	/// CHANGE NOTE
					if (nextNote[i]) {
						voltageScale[notesInVoltageScale] = noteVtable[i];
						notesInVoltageScale++;
					}
				}
				lastNoteInVoltageScale = notesInVoltageScale - 1;

				displayWorking = displayLastSelected;
				if (scaleChanged) {
					workingScale = selectedScale;
					workingMinMaj = selectedMinMaj;
					if (progInType == CV_TYPE)
						savedScaleKnob = scaleKnob - (inputs[SCALE_INPUT].getVoltage() * 1.2);
					else
						savedScaleKnob = scaleKnob;
				}
				if (progChanged) {
					workingProg = selectedProg;
					if (progInType == CV_TYPE)
						savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
					else
						savedProgKnob = progKnob;
				}
				scaleChanged = false;
				progChanged = false;

				setButLight = false;
				setButLightValue = 0.f;

			} else {	// IF SET IS PENDING -> GET NEW SETTINGS

				for (int i = 0; i < 12; i++) {
					nextNote[i] = params[NOTE_PARAM+i].getValue();
					lights[NOTE_LIGHT+i].setBrightness(nextNote[i]);

					if (nextNote[i] != changeNote[i]) {
						displayWorking = CUSTOMSCALE;
						displayLastSelected = CUSTOMSCALE;
					}
				}

			}
		}

		// --------------------------------------
		// -------------------------- RESET SCALE
		// --------------------------------------

		resetScale = params[RESETSCALE_PARAM].getValue();
		lights[RESETSCALE_LIGHT].setBrightness(resetScale);

		if (resetScale >= 1.f && prevResetScale < 1.f) {
			notesInVoltageScale = 0;
			for (int i = 0; i < 12; i++) {
				note[i] = scalesNotes[selectedMinMaj][selectedScale][i];
				nextNote[i] = note[i];
				changeNote[i] = note[i];
				if (note[i]) {
					voltageScale[notesInVoltageScale] = noteVtable[i];
					notesInVoltageScale++;
				}
				params[NOTE_PARAM+i].setValue(note[i]);
				lights[NOTE_LIGHT+i].setBrightness(note[i]);
			}
			lastNoteInVoltageScale = notesInVoltageScale - 1;
			workingScale = selectedScale;
			workingMinMaj = selectedMinMaj;
			displayWorking = DISPLAYSCALE;
			if (progInType == CV_TYPE)
				savedScaleKnob = scaleKnob - (inputs[SCALE_INPUT].getVoltage() * 1.2);
			else
				savedScaleKnob = scaleKnob;

			progChanged = false;
			scaleChanged = false;

			setButLight = false;
			setButLightValue = 0.f;
		}
		prevResetScale = resetScale;

		// --------------------------------------
		// -------------------------- RECALL PROG
		// --------------------------------------

		recallBut = params[RECALL_PARAM].getValue();
		lights[RECALL_LIGHT].setBrightness(recallBut);

		if (recallBut >= 1.f && prevRecallBut < 1.f) {

			//if (!progChanged) {

			//} else {
				notesInVoltageScale = 0;
				for (int i = 0; i < 12; i++) {
					note[i] = progNotes[selectedProg][i];
					nextNote[i] = note[i];
					changeNote[i] = note[i];
					if (note[i]) {
						voltageScale[notesInVoltageScale] = noteVtable[i];
						notesInVoltageScale++;
					}
					params[NOTE_PARAM+i].setValue(note[i]);
					lights[NOTE_LIGHT+i].setBrightness(note[i]);
				}
				lastNoteInVoltageScale = notesInVoltageScale - 1;

				workingProg = selectedProg;
				displayWorking = DISPLAYPROG;
				if (progInType == CV_TYPE)
					savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
				else
					savedProgKnob = progKnob;
			//}

			progChanged = false;
			scaleChanged = false;

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
				for (int i = 0; i < 12; i++)
					//progNotes[progKnob][i] = note[i];
					progNotes[progKnob][i] = nextNote[i];

				if (progKnob > lastProg)
					lastProg = progKnob;

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

		lights[MANUALSET_LIGHT].setBrightness(setButLightValue);

		// -------------------------------------------------------
		// ------------------- SIGNAL QUANTIZATION ---------------
		// -------------------------------------------------------

		if (notesInVoltageScale) {

			if (outSumMode == SUM_OFF) {

				chan = 1;

				for (int track = 0; track < 4; track++) {

					if (inputs[IN_INPUT+track].isConnected()) {
						chan = inputs[IN_INPUT+track].getChannels();
						for (int c = 0; c < chan; c++)
							inSignal[track][c] = inputs[IN_INPUT+track].getVoltage(c);

					} else if (track > 0) {

						for (int c = 0; c < chan; c++)
							inSignal[track][c] = inSignal[track-1][c];
					} else {

						inSignal[track][0] = 0;
					}

					if (outputs[OUT_OUTPUT+track].isConnected()) {

						atten = params[ATN_PARAM+track].getValue();
						octPostValue = params[OCT_PARAM+track].getValue();

						if (!inputs[TRIG_INPUT+track].isConnected()) {
							for (int c = 0; c < chan; c++)
								atnSignal[track][c] = 10 + (inSignal[track][c] * atten) + inputs[OFFS_INPUT+track].getVoltage();

						} else {
							quantTrig[track] = inputs[TRIG_INPUT+track].getVoltage();
							if (quantTrig[track] >= 1.f && prevQuantTrig[track] < 1.f) {
								for (int c = 0; c < chan; c++)	
									atnSignal[track][c] = 10 + (inSignal[track][c] * atten) + inputs[OFFS_INPUT+track].getVoltage();
							}
							prevQuantTrig[track] = quantTrig[track];
						}

						// single channels quantization

						for (int c = 0; c < chan; c++) {
							octSignal[track][c] = int(atnSignal[track][c]);
							noteSignal[track][c] = atnSignal[track][c] - octSignal[track][c];

							if (noteSignal[track][c] <= voltageScale[0]) {
								noteDeltaMin = voltageScale[0] - noteSignal[track][c];
								chosenNote = 0;
								if ((1 - voltageScale[lastNoteInVoltageScale] + noteSignal[track][c]) < noteDeltaMin) {
									chosenNote = lastNoteInVoltageScale;
									octSignal[track][c] -= 1;
								}

							} else if (noteSignal[track][c] >= voltageScale[lastNoteInVoltageScale]) {
								noteDeltaMin = noteSignal[track][c] - voltageScale[lastNoteInVoltageScale];
								chosenNote = lastNoteInVoltageScale;
								if ((voltageScale[0] + 1 - noteSignal[track][c]) < noteDeltaMin) {
									chosenNote = 0;
									octSignal[track][c] += 1;
								}

							} else {
								chosenNote = 0;
								noteDeltaMin = 10;
								for (int i = 0; i < notesInVoltageScale; i++) {
									noteDelta = fabs(noteSignal[track][c] - voltageScale[i]);
									if (noteDelta < noteDeltaMin) {
										noteDeltaMin = noteDelta;
										chosenNote = i;
									}
								}
							}

							outSignal[track][c] = octSignal[track][c] + voltageScale[chosenNote] - 10 + octPostValue;
							if (outSignal[track][c] > 5)
								outSignal[track][c] = 4 + (outSignal[track][c] - int(outSignal[track][c]));
							else if (outSignal[track][c] < -4)
								outSignal[track][c] = -4 - (outSignal[track][c] - int(outSignal[track][c]));

							outputs[OUT_OUTPUT+track].setVoltage(outSignal[track][c], c);	
						}

					} else {
						outputs[OUT_OUTPUT+track].setVoltage(0, 0);	
					}
		
					outputs[OUT_OUTPUT+track].setChannels(chan);

				}

			} else {

				// ************************************************************  SUMMED OUTS
				// ************************************************************  SUMMED OUTS
				// ************************************************************  SUMMED OUTS
				// ************************************************************  SUMMED OUTS

				// ---------- track ZERO  : get all channels but quantize only first

				chan = 1;
				if (inputs[IN_INPUT].isConnected()) {
					chan = inputs[IN_INPUT].getChannels();
					for (int c = 0; c < chan; c++)
						inSignal[0][c] = inputs[IN_INPUT].getVoltage(c);
				} else {
					inSignal[0][0] = 0;
				}

				if (outputs[OUT_OUTPUT].isConnected()) {

					atten = params[ATN_PARAM].getValue();
					octPostValue = params[OCT_PARAM].getValue();

					if (!inputs[TRIG_INPUT].isConnected()) {
						
						atnSignal[0][0] = 10 + (inSignal[0][0] * atten) + inputs[OFFS_INPUT].getVoltage();

					} else {
						quantTrig[0] = inputs[TRIG_INPUT].getVoltage();
						if (quantTrig[0] >= 1.f && prevQuantTrig[0] < 1.f)
							atnSignal[0][0] = 10 + (inSignal[0][0] * atten) + inputs[OFFS_INPUT].getVoltage();

						prevQuantTrig[0] = quantTrig[0];
					}

					// only FIRST channel quantization

					octSignal[0][0] = int(atnSignal[0][0]);
					noteSignal[0][0] = atnSignal[0][0] - octSignal[0][0];

					if (noteSignal[0][0] <= voltageScale[0]) {
						noteDeltaMin = voltageScale[0] - noteSignal[0][0];
						chosenNote = 0;
						if ((1 - voltageScale[lastNoteInVoltageScale] + noteSignal[0][0]) < noteDeltaMin) {
							chosenNote = lastNoteInVoltageScale;
							octSignal[0][0] -= 1;
						}

					} else if (noteSignal[0][0] >= voltageScale[lastNoteInVoltageScale]) {
						noteDeltaMin = noteSignal[0][0] - voltageScale[lastNoteInVoltageScale];
						chosenNote = lastNoteInVoltageScale;
						if ((voltageScale[0] + 1 - noteSignal[0][0]) < noteDeltaMin) {
							chosenNote = 0;
							octSignal[0][0] += 1;
						}

					} else {
						chosenNote = 0;
						noteDeltaMin = 10;
						for (int i = 0; i < notesInVoltageScale; i++) {
							noteDelta = fabs(noteSignal[0][0] - voltageScale[i]);
							if (noteDelta < noteDeltaMin) {
								noteDeltaMin = noteDelta;
								chosenNote = i;
							}
						}
					}

					outSignal[0][0] = octSignal[0][0] + voltageScale[chosenNote] - 10 + octPostValue;
					if (outSignal[0][0] > 5)
						outSignal[0][0] = 4 + (outSignal[0][0] - int(outSignal[0][0]));
					else if (outSignal[0][0] < -4)
						outSignal[0][0] = -4 - (outSignal[0][0] - int(outSignal[0][0]));

				} else {
					outSignal[0][0] = 0;
				}

				outputs[OUT_OUTPUT].setVoltage(outSignal[0][0], 0);
				
				// ------------------------------------------------------- OTHER SUMMED TRACKS

				for (int track = 1; track < outSumModePlus1; track++) {

					if (inputs[IN_INPUT+track].isConnected()) {
						chan = inputs[IN_INPUT+track].getChannels();
						for (int c = 0; c < chan; c++)
							inSignal[track][c] = inputs[IN_INPUT+track].getVoltage(c);
					} else {
						for (int c = 0; c < chan; c++)
							inSignal[track][c] = inSignal[track-1][c];
					}


					atten = params[ATN_PARAM+track].getValue();
					octPostValue = params[OCT_PARAM+track].getValue();

					if (!inputs[TRIG_INPUT+track].isConnected()) {
						for (int c = 0; c < chan; c++)
							atnSignal[track][c] = 10 + (inSignal[track][c] * atten) + inputs[OFFS_INPUT+track].getVoltage();
					} else {
						quantTrig[track] = inputs[TRIG_INPUT+track].getVoltage();
						if (quantTrig[track] >= 1.f && prevQuantTrig[track] < 1.f) {
							for (int c = 0; c < chan; c++)
								atnSignal[track][c] = 10 + (inSignal[track][c] * atten) + inputs[OFFS_INPUT+track].getVoltage();
						}
						prevQuantTrig[track] = quantTrig[track];
					}

					// single channels quantization

					for (int c = 0; c < chan; c++) {
						octSignal[track][c] = int(atnSignal[track][c]);
						noteSignal[track][c] = atnSignal[track][c] - octSignal[track][c];

						if (noteSignal[track][c] <= voltageScale[0]) {
							noteDeltaMin = voltageScale[0] - noteSignal[track][c];
							chosenNote = 0;
							if ((1 - voltageScale[lastNoteInVoltageScale] + noteSignal[track][c]) < noteDeltaMin) {
								chosenNote = lastNoteInVoltageScale;
								octSignal[track][c] -= 1;
							}
						} else if (noteSignal[track][c] >= voltageScale[lastNoteInVoltageScale]) {
							noteDeltaMin = noteSignal[track][c] - voltageScale[lastNoteInVoltageScale];
							chosenNote = lastNoteInVoltageScale;
							if ((voltageScale[0] + 1 - noteSignal[track][c]) < noteDeltaMin) {
								chosenNote = 0;
								octSignal[track][c] += 1;
							}
						} else {
							chosenNote = 0;
							noteDeltaMin = 10;
							for (int i = 0; i < notesInVoltageScale; i++) {
								noteDelta = fabs(noteSignal[track][c] - voltageScale[i]);
								if (noteDelta < noteDeltaMin) {
									noteDeltaMin = noteDelta;
									chosenNote = i;
								}
							}
						}

						outSignal[track][c] = octSignal[track][c] + voltageScale[chosenNote] - 10 + octPostValue;
						if (outSignal[track][c] > 5)
							outSignal[track][c] = 4 + (outSignal[track][c] - int(outSignal[track][c]));
						else if (outSignal[track][c] < -4)
							outSignal[track][c] = -4 - (outSignal[track][c] - int(outSignal[track][c]));

						outputs[OUT_OUTPUT+track].setVoltage(outSignal[track][c], c);	
					}
							
					outputs[OUT_OUTPUT+track].setChannels(chan);

					outputs[OUT_OUTPUT].setVoltage(outSignal[track][0], track);

				}

				outputs[OUT_OUTPUT].setChannels(outSumModePlus1);

				// ******************************************************************************
				// ******************************************************************************
				// ****************** RESIDUAL CHANNELS QUANTIZATION

				for (int track = outSumModePlus1; track < 4; track++) {
					
					if (inputs[IN_INPUT+track].isConnected()) {
						chan = inputs[IN_INPUT+track].getChannels();
						for (int c = 0; c < chan; c++)
							inSignal[track][c] = inputs[IN_INPUT+track].getVoltage(c);
					} else {
						for (int c = 0; c < chan; c++)
							inSignal[track][c] = inSignal[track-1][c];
					}

					if (outputs[OUT_OUTPUT+track].isConnected()) {
						atten = params[ATN_PARAM+track].getValue();
						octPostValue = params[OCT_PARAM+track].getValue();

						if (!inputs[TRIG_INPUT+track].isConnected()) {
							for (int c = 0; c < chan; c++)
								atnSignal[track][c] = 10 + (inSignal[track][c] * atten) + inputs[OFFS_INPUT+track].getVoltage();
						} else {
							quantTrig[track] = inputs[TRIG_INPUT+track].getVoltage();
							if (quantTrig[track] >= 1.f && prevQuantTrig[track] < 1.f) {
								for (int c = 0; c < chan; c++)
									atnSignal[track][c] = 10 + (inSignal[track][c] * atten) + inputs[OFFS_INPUT+track].getVoltage();
							}
							prevQuantTrig[track] = quantTrig[track];
						}

						// single channels quantization

						for (int c = 0; c < chan; c++) {
							octSignal[track][c] = int(atnSignal[track][c]);
							noteSignal[track][c] = atnSignal[track][c] - octSignal[track][c];
							if (noteSignal[track][c] <= voltageScale[0]) {
								noteDeltaMin = voltageScale[0] - noteSignal[track][c];
								chosenNote = 0;
								if ((1 - voltageScale[lastNoteInVoltageScale] + noteSignal[track][c]) < noteDeltaMin) {
									chosenNote = lastNoteInVoltageScale;
									octSignal[track][c] -= 1;
								}
							} else if (noteSignal[track][c] >= voltageScale[lastNoteInVoltageScale]) {
								noteDeltaMin = noteSignal[track][c] - voltageScale[lastNoteInVoltageScale];
								chosenNote = lastNoteInVoltageScale;
								if ((voltageScale[0] + 1 - noteSignal[track][c]) < noteDeltaMin) {
									chosenNote = 0;
									octSignal[track][c] += 1;
								}
							} else {
								chosenNote = 0;
								noteDeltaMin = 10;
								for (int i = 0; i < notesInVoltageScale; i++) {
									noteDelta = fabs(noteSignal[track][c] - voltageScale[i]);
									if (noteDelta < noteDeltaMin) {
										noteDeltaMin = noteDelta;
										chosenNote = i;
									}
								}
							}
							outSignal[track][c] = octSignal[track][c] + voltageScale[chosenNote] - 10 + octPostValue;
							if (outSignal[track][c] > 5)
								outSignal[track][c] = 4 + (outSignal[track][c] - int(outSignal[track][c]));
							else if (outSignal[track][c] < -4)
								outSignal[track][c] = -4 - (outSignal[track][c] - int(outSignal[track][c]));

							outputs[OUT_OUTPUT+track].setVoltage(outSignal[track][c], c);	
						}
					} else {
						chan = 1;
						outputs[OUT_OUTPUT+track].setVoltage(0, 0);	
					}
					outputs[OUT_OUTPUT+track].setChannels(chan);
				}
			}

		} else {	//  **** IF THERE ARE NO NOTES TO QUANTIZE

			if (outSumMode == SUM_OFF) {
				for (int track = 0; track < 4; track++) {
					if (inputs[IN_INPUT+track].isConnected())
						chan = inputs[IN_INPUT+track].getChannels();
					else if (track == 0)
						chan = 1;

					for (int c = 0; c < chan; c++)
						outputs[OUT_OUTPUT+track].setVoltage(0.f, c);
					outputs[OUT_OUTPUT+track].setChannels(chan);
				}
			} else {
				
				if (inputs[IN_INPUT].isConnected())
					chan = inputs[IN_INPUT].getChannels();
				else
					chan = 1;
				
				outputs[OUT_OUTPUT].setVoltage(0.f, 0);

				for (int track = 1; track < outSumModePlus1; track++) {

					if (inputs[IN_INPUT+track].isConnected()) {
						chan = inputs[IN_INPUT+track].getChannels();
						for (int c = 0; c < chan; c++)
							outputs[OUT_OUTPUT+track].setVoltage(0.f, c);
					} else {
						for (int c = 0; c < chan; c++)
							outputs[OUT_OUTPUT+track].setVoltage(0.f, c);
					}
					outputs[OUT_OUTPUT+track].setChannels(chan);
					outputs[OUT_OUTPUT].setVoltage(0.f, track);
				}
				outputs[OUT_OUTPUT].setChannels(outSumModePlus1);

				for (int track = outSumModePlus1; track < 4; track++) {
					if (inputs[IN_INPUT+track].isConnected()) {
						chan = inputs[IN_INPUT+track].getChannels();
						for (int c = 0; c < chan; c++)
							outputs[OUT_OUTPUT+track].setVoltage(0.f, c);
					} else {
						for (int c = 0; c < chan; c++)
							outputs[OUT_OUTPUT+track].setVoltage(0.f, c);
					}
					outputs[OUT_OUTPUT+track].setChannels(chan);
				}

			}

		}
	}
};

struct SickoQuant4DisplayScale : TransparentWidget {
	SickoQuant4 *module;
	int frame = 0;
	SickoQuant4DisplayScale() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/vgatrue.ttf"));
				
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				std::string currentDisplay;

				if (module->displayWorking == DISPLAYSCALE) {
					currentDisplay = module->noteName[module->workingScale];
					if (module->workingScale != 0)
						currentDisplay +=  module->scaleDisplay[module->workingMinMaj];
				} else if (module->displayWorking == DISPLAYPROG) {
					currentDisplay = "P" + to_string(module->workingProg);
				} else if (module->notesInVoltageScale == 0) {
					currentDisplay = "N.Q";
				} else {
					currentDisplay = "CST";
				}

				if (!module->progChanged && !module->scaleChanged) {
					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_GREEN));
					nvgFontSize(args.vg, 32);
					if (currentDisplay.size() == 3)
						nvgTextBox(args.vg, 7, 30, 80, currentDisplay.c_str(), NULL);
					else if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 13, 30, 80, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 20, 30, 80, currentDisplay.c_str(), NULL);

				} else {
					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_GREEN));
					nvgFontSize(args.vg, 26);
					if (currentDisplay.size() == 3)
						nvgTextBox(args.vg, 5, 21, 80, currentDisplay.c_str(), NULL);	
					else if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 11, 21, 80, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 17, 21, 80, currentDisplay.c_str(), NULL);
					
					if (module->displayLastSelected == DISPLAYSCALE) {
						currentDisplay = module->noteName[module->selectedScale];
						if (module->selectedScale != 0)
							currentDisplay +=  module->scaleDisplay[module->selectedMinMaj];
					} else if (module->displayLastSelected == DISPLAYPROG) {
						currentDisplay = "P" + to_string(module->selectedProg);
					} else {
						currentDisplay = "CST";
					}
					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED));
					nvgFontSize(args.vg, 20);
					if (currentDisplay.size() == 3)
						nvgTextBox(args.vg, 23, 36, 60, currentDisplay.c_str(), NULL);
					else if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 28, 36, 60, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 33, 36, 60, currentDisplay.c_str(), NULL);
				}

				
			}
		}
		Widget::drawLayer(args, layer);
	}
};

/*
struct SickoQuant4DebugDisplay : TransparentWidget {
	SickoQuant4 *module;
	int frame = 0;
	SickoQuant4DebugDisplay() {
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
				//nvgTextBox(args.vg, 9, 16,120, module->debugDisplay2.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 6,120, module->debugDisplay3.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 16,120, module->debugDisplay4.c_str(), NULL);

			}
		}
		Widget::drawLayer(args, layer);
	}
	
};
*/

struct SickoQuant4Widget : ModuleWidget {
	SickoQuant4Widget(SickoQuant4* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SickoQuant4.svg")));

		/*
		{
			SickoQuant4DebugDisplay *display = new SickoQuant4DebugDisplay();
			display->box.pos = Vec(15, 15);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/

		{
			SickoQuant4DisplayScale *display = new SickoQuant4DisplayScale();
			display->box.pos = mm2px(Vec(42, 34.3));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}
		
		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		constexpr float xKbdStart = 6;
		constexpr float xKbdDelta = 4.1;
		constexpr float yKbdBlck = 12.3;
		constexpr float yKbdWht = 18.8;
		
		const float xSclIn = 8.5;
		const float xSclKnob = 21.9;
		const float xRstScl = 34.5;
		const float yRstScl = 36.8;
		const float yScl = 37.8;

		const float xSet = 53.5;
		const float ySet = 28.8;

		const float xAuto = 6.8;
		const float yAuto = 48.4;

		const float xProgIn = 9;
		const float xProgKnob = 22.4;
		const float xRecall = 37.7;
		const float xStore = 51.5;

		const float yProg = 60.5;

		// CHAN

		constexpr float yChanStart = 80.5;
		constexpr float yChanDelta = 12;
		const float xTrig = 6.7;
		const float xIn = 16.2;
		const float xAtn = 25.7;
		const float xOfs = 35.2;
		const float xOct = 44.8;
		const float xOut = 54.5;
	
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdStart, yKbdWht)), module, SickoQuant4::NOTE_PARAM+0, SickoQuant4::NOTE_LIGHT+0));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdStart+xKbdDelta, yKbdBlck)), module, SickoQuant4::NOTE_PARAM+1, SickoQuant4::NOTE_LIGHT+1));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdStart+(2*xKbdDelta), yKbdWht)), module, SickoQuant4::NOTE_PARAM+2, SickoQuant4::NOTE_LIGHT+2));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdStart+(3*xKbdDelta), yKbdBlck)), module, SickoQuant4::NOTE_PARAM+3, SickoQuant4::NOTE_LIGHT+3));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdStart+(4*xKbdDelta), yKbdWht)), module, SickoQuant4::NOTE_PARAM+4, SickoQuant4::NOTE_LIGHT+4));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdStart+(6*xKbdDelta), yKbdWht)), module, SickoQuant4::NOTE_PARAM+5, SickoQuant4::NOTE_LIGHT+5));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdStart+(7*xKbdDelta), yKbdBlck)), module, SickoQuant4::NOTE_PARAM+6, SickoQuant4::NOTE_LIGHT+6));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdStart+(8*xKbdDelta), yKbdWht)), module, SickoQuant4::NOTE_PARAM+7, SickoQuant4::NOTE_LIGHT+7));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdStart+(9*xKbdDelta), yKbdBlck)), module, SickoQuant4::NOTE_PARAM+8, SickoQuant4::NOTE_LIGHT+8));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdStart+(10*xKbdDelta), yKbdWht)), module, SickoQuant4::NOTE_PARAM+9, SickoQuant4::NOTE_LIGHT+9));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdStart+(11*xKbdDelta), yKbdBlck)), module, SickoQuant4::NOTE_PARAM+10, SickoQuant4::NOTE_LIGHT+10));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdStart+(12*xKbdDelta), yKbdWht)), module, SickoQuant4::NOTE_PARAM+11, SickoQuant4::NOTE_LIGHT+11));
		
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xSet, ySet)), module, SickoQuant4::MANUALSET_PARAM, SickoQuant4::MANUALSET_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xSclIn, yScl)), module, SickoQuant4::SCALE_INPUT));
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xSclKnob, yScl)), module, SickoQuant4::SCALE_PARAM));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xRstScl, yRstScl)), module, SickoQuant4::RESETSCALE_PARAM, SickoQuant4::RESETSCALE_LIGHT));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xAuto, yAuto)), module, SickoQuant4::AUTO_PARAM, SickoQuant4::AUTO_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xProgIn, yProg)), module, SickoQuant4::PROG_INPUT));
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xProgKnob, yProg)), module, SickoQuant4::PROG_PARAM));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xRecall, yProg)), module, SickoQuant4::RECALL_PARAM, SickoQuant4::RECALL_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xStore, yProg)), module, SickoQuant4::STORE_PARAM, SickoQuant4::STORE_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xTrig, yChanStart)), module, SickoQuant4::TRIG_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xTrig, yChanStart+yChanDelta)), module, SickoQuant4::TRIG_INPUT+1));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xTrig, yChanStart+(2*yChanDelta))), module, SickoQuant4::TRIG_INPUT+2));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xTrig, yChanStart+(3*yChanDelta))), module, SickoQuant4::TRIG_INPUT+3));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xIn, yChanStart)), module, SickoQuant4::IN_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xIn, yChanStart+yChanDelta)), module, SickoQuant4::IN_INPUT+1));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xIn, yChanStart+(2*yChanDelta))), module, SickoQuant4::IN_INPUT+2));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xIn, yChanStart+(3*yChanDelta))), module, SickoQuant4::IN_INPUT+3));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xAtn, yChanStart)), module, SickoQuant4::ATN_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xAtn, yChanStart+yChanDelta)), module, SickoQuant4::ATN_PARAM+1));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xAtn, yChanStart+(2*yChanDelta))), module, SickoQuant4::ATN_PARAM+2));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xAtn, yChanStart+(3*yChanDelta))), module, SickoQuant4::ATN_PARAM+3));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xOfs, yChanStart)), module, SickoQuant4::OFFS_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xOfs, yChanStart+yChanDelta)), module, SickoQuant4::OFFS_INPUT+1));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xOfs, yChanStart+(2*yChanDelta))), module, SickoQuant4::OFFS_INPUT+2));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xOfs, yChanStart+(3*yChanDelta))), module, SickoQuant4::OFFS_INPUT+3));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xOct, yChanStart)), module, SickoQuant4::OCT_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xOct, yChanStart+yChanDelta)), module, SickoQuant4::OCT_PARAM+1));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xOct, yChanStart+(2*yChanDelta))), module, SickoQuant4::OCT_PARAM+2));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xOct, yChanStart+(3*yChanDelta))), module, SickoQuant4::OCT_PARAM+3));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yChanStart)), module, SickoQuant4::OUT_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yChanStart+yChanDelta)), module, SickoQuant4::OUT_OUTPUT+1));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yChanStart+(2*yChanDelta))), module, SickoQuant4::OUT_OUTPUT+2));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yChanStart+(3*yChanDelta))), module, SickoQuant4::OUT_OUTPUT+3));

	}

	void appendContextMenu(Menu* menu) override {
		SickoQuant4* module = dynamic_cast<SickoQuant4*>(this->module);

		menu->addChild(new MenuSeparator());
		struct ProgInTypeItem : MenuItem {
			SickoQuant4* module;
			int progInType;
			void onAction(const event::Action& e) override {
				module->progInType = progInType;
			}
		};

		std::string ProgInTypeNames[2] = {"CV", "Trig"};
		menu->addChild(createSubmenuItem("Prog Input type", (ProgInTypeNames[module->progInType]), [=](Menu * menu) {
			for (int i = 0; i < 2; i++) {
				ProgInTypeItem* progInTypeItem = createMenuItem<ProgInTypeItem>(ProgInTypeNames[i]);
				progInTypeItem->rightText = CHECKMARK(module->progInType == i);
				progInTypeItem->module = module;
				progInTypeItem->progInType = i;
				menu->addChild(progInTypeItem);
			}
		}));
		menu->addChild(createMenuItem("Scan Last Prog", "current: " + to_string(module->lastProg), [=]() {module->scanLastProg();}));

		menu->addChild(new MenuSeparator());

		struct ModeItem : MenuItem {
			SickoQuant4* module;
			int outSumMode;
			void onAction(const event::Action& e) override {
				module->outSumMode = outSumMode;
				module->outSumModePlus1 = outSumMode + 1;
			}
		};

		menu->addChild(createMenuLabel("SUM chans to poly out #1:"));
		std::string modeNames[4] = {"OFF", "1+2", "1+2+3", "1+2+3+4"};
		for (int i = 0; i < 4; i++) {
			ModeItem* modeItem = createMenuItem<ModeItem>(modeNames[i]);
			modeItem->rightText = CHECKMARK(module->outSumMode == i);
			modeItem->module = module;
			modeItem->outSumMode = i;
			menu->addChild(modeItem);
		}

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
		menu->addChild(createSubmenuItem("Tips", "", [=](Menu * menu) {
			menu->addChild(createMenuLabel("Store Programs with"));
			menu->addChild(createMenuLabel("double-click on STOR button"));
		}));

	}
	
};

Model* modelSickoQuant4 = createModel<SickoQuant4, SickoQuant4Widget>("SickoQuant4");