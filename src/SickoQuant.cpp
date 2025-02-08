#define COLOR_LCD_RED 0xdd, 0x33, 0x33, 0xff
#define COLOR_LCD_GREEN 0x33, 0xdd, 0x33, 0xff

#define DISPLAYSCALE 0
#define DISPLAYPROG 1
#define CUSTOMSCALE 2

#define SUM_OFF 0
#define SUM_12 1
#define SUM_123 2
#define SUM_1234 3

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

struct tpScaleSQ : ParamQuantity {
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

struct SickoQuant : Module {
	enum ParamId {
		ENUMS(NOTE_PARAM, 12),
		ATN_PARAM,
		OCT_PARAM,
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
		TRIG_INPUT,
		IN_INPUT,
		OFFS_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
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
	int pendingNote[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	int nextNote[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	float voltageScale[12] = {0, 0.08333f, 0.16667f, 0.25f, 0.33333f, 0.41667, 0.5f, 0.58333f, 0.66667f, 0.75f, 0.83333f, 0.91667f};
	int notesInVoltageScale = 0;
	int lastNoteInVoltageScale = -1;
	float noteDelta = 0.f;
	float noteDeltaMin = 5.f;
	int chosenNote = 0;
	bool notesChanged = false;

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

	bool pendingUpdate = false;

	int displayWorking = CUSTOMSCALE;
	int displayLastSelected = CUSTOMSCALE;

	int chan = 0;
	float atten = 0;
	float octPostValue = 0;

	float inSignal[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float atnSignal[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float outSignal[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float octSignal[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float noteSignal[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

	float quantTrig = 0.f;
	float prevQuantTrig = 0.f;

	int outSumMode = 0;
	int chanSum = 1;
	float outSum = 0;

	// ------- set button light

	bool setButLight = false;
	float setButLightDelta = 2 / APP->engine->getSampleRate();
	float setButLightValue = 0.f;

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

	SickoQuant() {
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
		configParam<tpScaleSQ>(SCALE_PARAM, -12, 12.f, 0.f, "Scale");
		paramQuantities[SCALE_PARAM]->snapEnabled = true;
		configSwitch(RESETSCALE_PARAM, 0, 1.f, 0.f, "Set Scale", {"OFF", "ON"});

		configSwitch(AUTO_PARAM, 0, 1.f, 0.f, "Auto", {"OFF", "ON"});

		configInput(PROG_INPUT, "Prog");
		configParam(PROG_PARAM, 0.f, 31.f, 0.f, "Prog");
		paramQuantities[PROG_PARAM]->snapEnabled = true;
		configSwitch(RECALL_PARAM, 0, 1.f, 0.f, "Recall", {"OFF", "ON"});
		configSwitch(STORE_PARAM, 0, 1.f, 0.f, "Store", {"OFF", "ON"});

		configInput(TRIG_INPUT, "Trig");
		
		configInput(IN_INPUT, "Signal");

		configParam(ATN_PARAM, 0.f, 1.f, 1.f, "Att.", "%", 0, 100);

		configInput(OFFS_INPUT, "Offset");

		configParam(OCT_PARAM, -3.f, 3.f, 0.f, "Octave");
		paramQuantities[OCT_PARAM]->snapEnabled = true;

		configOutput(OUT_OUTPUT, "Signal");
		
	}

	void processBypass(const ProcessArgs &e) override {
		if (inputs[IN_INPUT].isConnected())
			chan = inputs[IN_INPUT].getChannels();
		else
			chan = 1;

		for (int c = 0; c < chan; c++)
			outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage(c), c);
		outputs[OUT_OUTPUT].setChannels(chan);

		Module::processBypass(e);
	}

	void onReset(const ResetEvent &e) override {
		for (int i = 0; i < 12; i++) {
			note[i] = 0;
			pendingNote[i] = 0;
			nextNote[i] = 0;
			voltageScale[i] = noteVtable[i];
		}

		notesInVoltageScale = 0;
		lastNoteInVoltageScale = -1;
		notesChanged = false;

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

		pendingUpdate = false;

		displayWorking = CUSTOMSCALE;
		displayLastSelected = CUSTOMSCALE;

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
		for (int i = 0; i < 12; i++)
			for (int j = 0; j < 32; j++)
				progNotes[j][i] = 0;
	}

	void process(const ProcessArgs& args) override {

		// ----------- AUTO SWITCH

		instantScaleChange = int(params[AUTO_PARAM].getValue());
		lights[AUTO_LIGHT].setBrightness(instantScaleChange);

		// --------------------------------------
		// ----------- PROGRAM MANAGEMENT
		// --------------------------------------

		progKnob = int(params[PROG_PARAM].getValue() + (inputs[PROG_INPUT].getVoltage() * 3.2));
		if (progKnob < 0)
			progKnob = 0;
		else if (progKnob > 31)
			progKnob = 31;

		if (progKnob != prevProgKnob) {
			pendingUpdate = true;
			progChanged = true;
			displayLastSelected = DISPLAYPROG;
			if (scaleChanged)
				scaleChanged = false;
			selectedProg = progKnob;
			prevProgKnob = progKnob;

			for (int i = 0; i < 12; i++) {
				nextNote[i] = progNotes[selectedProg][i];
				pendingNote[i] = nextNote[i];
				params[NOTE_PARAM+i].setValue(nextNote[i]);
			}
			notesChanged = true;
			
			setButLight = true;
			setButLightValue = 0.f;

		}
		
		// --------------------------------------
		// ----------- SCALE MANAGEMENT
		// --------------------------------------

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
				pendingNote[i] = nextNote[i];
				params[NOTE_PARAM+i].setValue(nextNote[i]);
			}
			notesChanged = true;

			pendingUpdate = true;
			scaleChanged = true;
			if (progChanged)
				progChanged = false;

			displayLastSelected = DISPLAYSCALE;

			setButLight = true;
			setButLightValue = 0.f;

		}
		
		// -------- populate next notes array and show them

		if (pendingUpdate) {
			for (int i = 0; i < 12; i++) {
				nextNote[i] = int(params[NOTE_PARAM+i].getValue());
				if (nextNote[i] != pendingNote[i]) {
					displayLastSelected = CUSTOMSCALE;
					notesChanged = true;
				}
				lights[NOTE_LIGHT+i].setBrightness(nextNote[i]);
			}

		} else {
			for (int i = 0; i < 12; i++) {
				nextNote[i] = int(params[NOTE_PARAM+i].getValue());
				if (nextNote[i] != note[i]) {
					notesChanged = true;
				}
				lights[NOTE_LIGHT+i].setBrightness(nextNote[i]);
			}
		}

		// --------------------------------------
		// -------- CURRENT SCALE UPDATE
		// --------------------------------------

		butSetScale = false;

		scaleSetBut = params[MANUALSET_PARAM].getValue();
		//lights[MANUALSET_LIGHT].setBrightness(scaleSetBut);
		if (scaleSetBut >= 1.f && prevScaleSetBut < 1.f) {
			butSetScale = true;
		}
		prevScaleSetBut = scaleSetBut;

		if (notesChanged) {
			if (pendingUpdate) {
				if (instantScaleChange) {
					notesInVoltageScale = 0;
					for (int i = 0; i < 12; i++) {
						note[i] = nextNote[i];
						if (nextNote[i]) {
							voltageScale[notesInVoltageScale] = noteVtable[i];
							notesInVoltageScale++;
						}
					}
					lastNoteInVoltageScale = notesInVoltageScale - 1;
					pendingUpdate = false;
					displayWorking = displayLastSelected;
					if (scaleChanged) {
						workingScale = selectedScale;
						workingMinMaj = selectedMinMaj;
						savedScaleKnob = scaleKnob - (inputs[SCALE_INPUT].getVoltage() * 1.2);
					}
					if (progChanged) {
						workingProg = selectedProg;
						savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
					}
					notesChanged = false;

					setButLight = false;
					setButLightValue = 0.f;

				} else {
					
					if (butSetScale) {
						butSetScale = false;
						notesInVoltageScale = 0;
						for (int i = 0; i < 12; i++) {
							note[i] = nextNote[i];
							if (nextNote[i]) {
								voltageScale[notesInVoltageScale] = noteVtable[i];
								notesInVoltageScale++;
							}
						}
						lastNoteInVoltageScale = notesInVoltageScale - 1;
						pendingUpdate = false;
						displayWorking = displayLastSelected;
						if (scaleChanged) {
							workingScale = selectedScale;
							workingMinMaj = selectedMinMaj;
							savedScaleKnob = scaleKnob - (inputs[SCALE_INPUT].getVoltage() * 1.2);
						}
						if (progChanged) {
							workingProg = selectedProg;
							savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
						}
						notesChanged = false;

						setButLight = false;
						setButLightValue = 0.f;

					}

				}
		
			} else {	// if there are NO pending scale or prog updates (only manual notes are changed)
				notesInVoltageScale = 0;
				for (int i = 0; i < 12; i++) {
					note[i] = nextNote[i];
					if (nextNote[i]) {
						voltageScale[notesInVoltageScale] = noteVtable[i];
						notesInVoltageScale++;
					}
				}
				lastNoteInVoltageScale = notesInVoltageScale - 1;
				displayWorking = CUSTOMSCALE;
				displayLastSelected = CUSTOMSCALE;
				notesChanged = false;
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
			savedScaleKnob = scaleKnob - (inputs[SCALE_INPUT].getVoltage() * 1.2);
			notesChanged = false;
			pendingUpdate = false;
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

			notesInVoltageScale = 0;
			for (int i = 0; i < 12; i++) {
				note[i] = progNotes[selectedProg][i];
				nextNote[i] = note[i];
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
			savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
			notesChanged = false;
			pendingUpdate = false;
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
					progNotes[progKnob][i] = nextNote[i];
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

		// ---------------------------------------
		// ------------------- SIGNAL QUANTIZATION
		// ---------------------------------------

		if (notesInVoltageScale) {

			if (outputs[OUT_OUTPUT].isConnected() && inputs[IN_INPUT].isConnected()) {

				chan = inputs[IN_INPUT].getChannels();

				octPostValue = params[OCT_PARAM].getValue();

				if (!inputs[TRIG_INPUT].isConnected()) {

					for (int c = 0; c < chan; c++)
						atnSignal[c] = 10 + (inputs[IN_INPUT].getVoltage(c) * params[ATN_PARAM].getValue()) + inputs[OFFS_INPUT].getVoltage();
					
				} else {
					quantTrig = inputs[TRIG_INPUT].getVoltage();
					if (quantTrig >= 1.f && prevQuantTrig < 1.f) {
						for (int c = 0; c < chan; c++) 
							atnSignal[c] = 10 + (inputs[IN_INPUT].getVoltage(c) * params[ATN_PARAM].getValue()) + inputs[OFFS_INPUT].getVoltage();
					}
					prevQuantTrig = quantTrig;
				}

				// single channels quantization

				for (int c = 0; c < chan; c++) {
					octSignal[c] = int(atnSignal[c]);
					noteSignal[c] = atnSignal[c] - octSignal[c];

					if (noteSignal[c] <= voltageScale[0]) {
						noteDeltaMin = voltageScale[0] - noteSignal[c];
						chosenNote = 0;
						if ((1 - voltageScale[lastNoteInVoltageScale] + noteSignal[c]) < noteDeltaMin) {
							chosenNote = lastNoteInVoltageScale;
							octSignal[c] -= 1;
						}

					} else if (noteSignal[c] >= voltageScale[lastNoteInVoltageScale]) {
						noteDeltaMin = noteSignal[c] - voltageScale[lastNoteInVoltageScale];
						chosenNote = lastNoteInVoltageScale;
						if ((voltageScale[0] + 1 - noteSignal[c]) < noteDeltaMin) {
							chosenNote = 0;
							octSignal[c] += 1;
						}

					} else {
						chosenNote = 0;
						noteDeltaMin = 10;
						for (int i = 0; i < notesInVoltageScale; i++) {
							noteDelta = fabs(noteSignal[c] - voltageScale[i]);
							if (noteDelta < noteDeltaMin) {
								noteDeltaMin = noteDelta;
								chosenNote = i;
							}
						}
					}

					outSignal[c] = octSignal[c] + voltageScale[chosenNote] - 10 + octPostValue;
					if (outSignal[c] > 5)
						outSignal[c] = 4 + (outSignal[c] - int(outSignal[c]));
					else if (outSignal[c] < -4)
						outSignal[c] = -4 - (outSignal[c] - int(outSignal[c]));

					outputs[OUT_OUTPUT].setVoltage(outSignal[c], c);	
				}

			} else {

				chan = 1;
				outputs[OUT_OUTPUT].setVoltage(0, 0);

			}

			outputs[OUT_OUTPUT].setChannels(chan);

		} else {	// IF THERE ARE NO NOTES TO QUANTIZE -> DO NOT QUANTIZE

			chan = inputs[IN_INPUT].getChannels();
			for (int c = 0; c < chan; c++)
				outputs[OUT_OUTPUT].setVoltage(0.f, c);
			outputs[OUT_OUTPUT].setChannels(chan);
			
		}
	}
};

struct SickoQuantDisplayScale : TransparentWidget {
	SickoQuant *module;
	int frame = 0;
	SickoQuantDisplayScale() {
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

				if (!module->pendingUpdate) {
					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_GREEN));
					nvgFontSize(args.vg, 30);
					if (currentDisplay.size() == 3)
						nvgTextBox(args.vg, 5.5, 25, 80, currentDisplay.c_str(), NULL);
					else if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 12, 25, 80, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 18, 25, 80, currentDisplay.c_str(), NULL);

				} else {
					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_GREEN));
					nvgFontSize(args.vg, 19);
					if (currentDisplay.size() == 3)
						nvgTextBox(args.vg, 5.5, 15.5, 80, currentDisplay.c_str(), NULL);	
					else if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 12, 15.5, 80, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 18, 15.5, 80, currentDisplay.c_str(), NULL);
					
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
					nvgFontSize(args.vg, 17);
					if (currentDisplay.size() == 3)
						nvgTextBox(args.vg, 21, 28, 60, currentDisplay.c_str(), NULL);
					else if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 26, 28, 60, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 31, 28, 60, currentDisplay.c_str(), NULL);
				}
			}
		}
		Widget::drawLayer(args, layer);
	}
};

/*
struct SickoQuantDebugDisplay : TransparentWidget {
	SickoQuant *module;
	int frame = 0;
	SickoQuantDebugDisplay() {
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

struct SickoQuantWidget : ModuleWidget {
	SickoQuantWidget(SickoQuant* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SickoQuant.svg")));

		/*
		{
			SickoQuantDebugDisplay *display = new SickoQuantDebugDisplay();
			display->box.pos = Vec(23, 30);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/

		{
			SickoQuantDisplayScale *display = new SickoQuantDisplayScale();
			display->box.pos = mm2px(Vec(18.4, 7.8));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}
		

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		constexpr float xKbdBlck = 5.9;
		constexpr float xKbdWht = 12.9;
		constexpr float yKbdStart = 11.6;
		constexpr float yKbdDelta = 3.6;

		const float xRight = 26.5;		

		const float yScaleKnob = 30.5;
		const float ySclIn = 41.5;
		const float yScaleBut = 54.5;

		const float xAuto = 28.5;
		const float yAuto = 63.1;

		const float xSet = 6.5;
		const float ySet = 64;
		
		const float xProgKnob = 8.5;
		const float yProgKnob = 79.5;
		const float xProgIn = 15.5;
		const float yProgIn = 87;

		const float xRecall = 26;
		const float yRecall = 74.8;

		const float xStore = 26;
		
		const float xTrig = 7.5;
		const float xIn = 18;
		const float xAtn = 28.6;
		const float xOfs = 7.5;
		const float xOct = 18;
		const float xOut = 28.6;

		const float yOut1 = 103.5;
		const float yOut2 = 117.5;
	
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdWht, yKbdStart)), module, SickoQuant::NOTE_PARAM+11, SickoQuant::NOTE_LIGHT+11));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdBlck, yKbdStart+yKbdDelta)), module, SickoQuant::NOTE_PARAM+10, SickoQuant::NOTE_LIGHT+10));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdWht, yKbdStart+(2*yKbdDelta))), module, SickoQuant::NOTE_PARAM+9, SickoQuant::NOTE_LIGHT+9));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdBlck, yKbdStart+(3*yKbdDelta))), module, SickoQuant::NOTE_PARAM+8, SickoQuant::NOTE_LIGHT+8));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdWht, yKbdStart+(4*yKbdDelta))), module, SickoQuant::NOTE_PARAM+7, SickoQuant::NOTE_LIGHT+7));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdBlck, yKbdStart+(5*yKbdDelta))), module, SickoQuant::NOTE_PARAM+6, SickoQuant::NOTE_LIGHT+6));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdWht, yKbdStart+(6*yKbdDelta))), module, SickoQuant::NOTE_PARAM+5, SickoQuant::NOTE_LIGHT+5));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdWht, yKbdStart+(8*yKbdDelta))), module, SickoQuant::NOTE_PARAM+4, SickoQuant::NOTE_LIGHT+4));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdBlck, yKbdStart+(9*yKbdDelta))), module, SickoQuant::NOTE_PARAM+3, SickoQuant::NOTE_LIGHT+3));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdWht, yKbdStart+(10*yKbdDelta))), module, SickoQuant::NOTE_PARAM+2, SickoQuant::NOTE_LIGHT+2));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdBlck, yKbdStart+(11*yKbdDelta))), module, SickoQuant::NOTE_PARAM+1, SickoQuant::NOTE_LIGHT+1));
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xKbdWht, yKbdStart+(12*yKbdDelta))), module, SickoQuant::NOTE_PARAM+0, SickoQuant::NOTE_LIGHT+0));
		
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRight, ySclIn)), module, SickoQuant::SCALE_INPUT));
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xRight, yScaleKnob)), module, SickoQuant::SCALE_PARAM));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xRight, yScaleBut)), module, SickoQuant::RESETSCALE_PARAM, SickoQuant::RESETSCALE_LIGHT));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xAuto, yAuto)), module, SickoQuant::AUTO_PARAM, SickoQuant::AUTO_LIGHT));

		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xSet, ySet)), module, SickoQuant::MANUALSET_PARAM, SickoQuant::MANUALSET_LIGHT));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xProgKnob, yProgKnob)), module, SickoQuant::PROG_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xProgIn, yProgIn)), module, SickoQuant::PROG_INPUT));
		
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xRecall, yRecall)), module, SickoQuant::RECALL_PARAM, SickoQuant::RECALL_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xStore, yProgIn)), module, SickoQuant::STORE_PARAM, SickoQuant::STORE_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xTrig, yOut1)), module, SickoQuant::TRIG_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xIn, yOut1)), module, SickoQuant::IN_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xAtn, yOut1)), module, SickoQuant::ATN_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xOfs, yOut2)), module, SickoQuant::OFFS_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xOct, yOut2)), module, SickoQuant::OCT_PARAM));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yOut2)), module, SickoQuant::OUT_OUTPUT));
		
	}

	void appendContextMenu(Menu* menu) override {
		SickoQuant* module = dynamic_cast<SickoQuant*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Load PROG preset", "", [=]() {module->menuLoadPreset();}));
		menu->addChild(createMenuItem("Save PROG preset", "", [=]() {module->menuSavePreset();}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Store Programs"));
		menu->addChild(createMenuLabel("with double-click"));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Erase ALL progs", "", [=](Menu * menu) {
			menu->addChild(createSubmenuItem("Are you Sure?", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("ERASE!", "", [=]() {module->eraseProgs();}));
			}));
		}));
	}

};

Model* modelSickoQuant = createModel<SickoQuant, SickoQuantWidget>("SickoQuant");