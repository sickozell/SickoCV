#define STD2x_PROGRESSION 0
#define P_1_3_PROGRESSION 1 // 1.3x
#define FIBONACCI_PROGRESSION 2

#define BIT_8 0
#define BIT_16 1

#define OUT_TRIG 0
#define OUT_GATE 1
#define OUT_CLOCK 2

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

struct RandLoops8 : Module {
	enum ParamId {
		ENUMS(CTRL_PARAM, 8),
		ENUMS(LENGTH_PARAM, 8),
		ENUMS(SCALE_PARAM, 8),
		ENUMS(OFFSET_PARAM, 8),
		PARAMS_LEN
	};
	enum InputId {
		ENUMS(CLK_INPUT, 8),
		ENUMS(RST_INPUT, 8),
		ENUMS(CTRL_INPUT, 8),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(TRG_OUTPUT, 8),
		ENUMS(CV_OUTPUT, 8),
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	bool initStart = false;

	float clock[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	float prevClock[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	float rstValue[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	float prevRstValue[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	float volt[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	float cvOut[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	float trgOut[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	float controlValue = 0;

	int polyChans = 0;

	bool dontAdvance[8] = {false, false, false, false, false, false, false, false};
	bool dontAdvanceSetting = true;

	int progression = STD2x_PROGRESSION;

	const float tableVolts[3][2][16] = {
		{
			{0.03922, 0.07844, 0.15688, 0.31376, 0.62752, 1.25504, 2.51008, 5.02016, 0, 0, 0, 0, 0, 0, 0, 0},
			{0.00015259, 0.000305181, 0.000610361, 0.001220722, 0.002441445, 0.00488289, 0.009765779, 0.019531558, 0.039063117, 0.078126234, 0.156252467, 0.312504934, 0.625009869, 1.250019738, 2.500039475, 5.00007895}
		},
		{
			{0.4191521, 0.54489773, 0.708367049, 0.920877164, 1.197140313, 1.556282407, 2.023167129, 2.630117267, 0, 0, 0, 0, 0, 0, 0, 0},
			{0.04577242, 0.059504146, 0.07735539, 0.100562007, 0.130730609, 0.169949791, 0.220934729, 0.287215147, 0.373379692, 0.485393599, 0.631011679, 0.820315183, 1.066409737, 1.386332659, 1.802232456, 2.342902193}
		},
		{
			{0.114942529, 0.229885058, 0.344827586, 0.574712644, 0.91954023, 1.494252874, 2.413793105, 3.908045979, 0, 0, 0, 0, 0, 0, 0, 0},
			{0.002392917, 0.004785834, 0.007178751, 0.011964585, 0.019143336, 0.031107921, 0.050251256, 0.081359177, 0.131610433, 0.21296961, 0.344580044, 0.557549654, 0.902129698, 1.459679352, 2.361809049, 3.821488401}
		}
	};

	int shiftRegister[8][16] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
							};

	int saveRegister[8][16] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
							};

	int tempRegister[16] =     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int tempSaveRegister[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	float probCtrl = 0;
	float probCtrlRnd = 0;
	float probRegister = 0;

	int incomingRegister = 0;

	int startingStep[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
	const int bitResTable[2] = {8, 16};
	int bitResolution = BIT_8;

	int outType = OUT_TRIG;

	float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	bool stepPulse[8] = {false, false, false, false, false, false, false, false};
	float stepPulseTime[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	bool outGate[8] = {false, false, false, false, false, false, false, false};
	
	RandLoops8() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		for (int i = 0; i < 8; i++) {
			configInput(CLK_INPUT+i, "Clock #" + to_string(i+1));
			configInput(RST_INPUT+i, "Reset #" + to_string(i+1));
			configParam(CTRL_PARAM+i, -1.f, 1.f, 0.f, "Control #" + to_string(i+1));
			configInput(CTRL_INPUT+i, "CV Control #" + to_string(i+1));
			configParam(LENGTH_PARAM+i, 1.f, 16.f, 16.f, "Length #" + to_string(i+1));
			paramQuantities[LENGTH_PARAM+i]->snapEnabled = true;
			configParam(SCALE_PARAM+i, 0.f, 1.f, 1.f, "Scale #" + to_string(i+1), "%", 0, 100);
			configParam(OFFSET_PARAM+i, -10.f, 10.f, 0.f, "Offset #" + to_string(i+1));
			configOutput(TRG_OUTPUT+i, "Trig #" + to_string(i+1));
			configOutput(CV_OUTPUT+i, "CV #" + to_string(i+1));
		}
	}

	void onReset(const ResetEvent &e) override {
		initStart = false;
		clearAll();

	    Module::onReset(e);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "InitStart", json_boolean(initStart));
		json_object_set_new(rootJ, "dontAdvanceSetting", json_boolean(dontAdvanceSetting));
		json_object_set_new(rootJ, "polyChans", json_integer(polyChans));
		json_object_set_new(rootJ, "bitResolution", json_integer(bitResolution));
		json_object_set_new(rootJ, "progression", json_integer(progression));
		json_object_set_new(rootJ, "outType", json_integer(outType));

		for (int t = 0; t < 8; t++)
			sequence_to_saveRegister(t);

		//------------------

		for (int t = 0; t < 8; t++) {
			json_t *wSeq_json_array = json_array();
			for (int st = 0; st < 16; st++) {
				json_array_append_new(wSeq_json_array, json_integer(saveRegister[t][st]));
			}
			json_object_set_new(rootJ, ("sr"+to_string(t)).c_str(), wSeq_json_array);	
		}

		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* initStartJ = json_object_get(rootJ, "InitStart");
		if (initStartJ)
			initStart = json_boolean_value(initStartJ);

		json_t* bitResolutionJ = json_object_get(rootJ, "bitResolution");
		if (bitResolutionJ) {
			bitResolution = json_integer_value(bitResolutionJ);
			if (bitResolution < 0 && bitResolution > 1)
				bitResolution = BIT_8;
		}

		json_t* progressionJ = json_object_get(rootJ, "progression");
		if (progressionJ) {
			progression = json_integer_value(progressionJ);
			if (progression < 0 && progression > 2)
				progression = STD2x_PROGRESSION;
		}

		json_t* outTypeJ = json_object_get(rootJ, "outType");
		if (outTypeJ) {
			outType = json_integer_value(outTypeJ);
			if (outType < 0 || outType > 2)
				outType = 0;
		}

		json_t* dontAdvanceSettingJ = json_object_get(rootJ, "dontAdvanceSetting");
		if (dontAdvanceSettingJ) {
			dontAdvanceSetting = json_boolean_value(dontAdvanceSettingJ);
		}

		json_t* polyChansJ = json_object_get(rootJ, "polyChans");
		if (polyChansJ)
			polyChans = json_integer_value(polyChansJ);

		if (!initStart) {
			for (int t = 0; t < 8; t++) {
				json_t *wSeq_json_array = json_object_get(rootJ, ("sr"+to_string(t)).c_str());
				size_t st;
				json_t *wSeq_json_value;
				if (wSeq_json_array) {
					json_array_foreach(wSeq_json_array, st, wSeq_json_value) {
						shiftRegister[t][st] = json_integer_value(wSeq_json_value);
					}
				}
			}

		}

	}

	// ------------------------ LOAD / SAVE   RANDLOOPS8 SINGLE PROGRAM

	json_t *singleToJson() {

		json_t *rootJ = json_object();

		//json_object_set_new(rootJ, "runType", json_integer(runType));
		//json_object_set_new(rootJ, "revType", json_integer(revType));
		json_object_set_new(rootJ, "outType", json_integer(outType));
		//json_object_set_new(rootJ, "rstOnRun", json_boolean(rstOnRun));
		json_object_set_new(rootJ, "dontAdvanceSetting", json_boolean(dontAdvanceSetting));

		//json_object_set_new(rootJ, "turingMode", json_boolean(turingMode));
		json_object_set_new(rootJ, "bitResolution", json_integer(bitResolution));
		json_object_set_new(rootJ, "progression", json_integer(progression));


		for (int t = 0; t < 8; t++) {

			sequence_to_saveRegister(t);

			json_t *wSeq_json_array = json_array();
			for (int st = 0; st < 16; st++) {
				json_array_append_new(wSeq_json_array, json_integer(shiftRegister[t][st]));
			}
			json_object_set_new(rootJ, ("t"+to_string(t)).c_str(), wSeq_json_array);
		}

		for (int t = 0; t < 8; t++) {
			json_t *wSteps_json_array = json_array();
			json_array_append_new(wSteps_json_array, json_integer(params[LENGTH_PARAM+t].getValue()));
			json_object_set_new(rootJ, ("wSteps"+to_string(t)).c_str(), wSteps_json_array);
		}

		for (int t = 0; t < 8; t++) {
			json_t *wRst_json_array = json_array();
			json_array_append_new(wRst_json_array, json_real(params[SCALE_PARAM+t].getValue()));
			json_object_set_new(rootJ, ("wRst"+to_string(t)).c_str(), wRst_json_array);
		}

		for (int t = 0; t < 8; t++) {
			json_t *wOfs_json_array = json_array();
			json_array_append_new(wOfs_json_array, json_real(params[OFFSET_PARAM+t].getValue()));
			json_object_set_new(rootJ, ("wOfs"+to_string(t)).c_str(), wOfs_json_array);
		}

		return rootJ;

	}

	void singleFromJson(json_t *rootJ) {

		/*json_t* runTypeJ = json_object_get(rootJ, "runType");
		if (runTypeJ) {
			runType = json_integer_value(runTypeJ);
			if (runType < 0 || runType > 1)
				runType = 0;
		}*/

		/*json_t* revTypeJ = json_object_get(rootJ, "revType");
		if (revTypeJ) {
			revType = json_integer_value(revTypeJ);
			if (revType < 0 || revType > 1)
				revType = 0;
		}*/

		json_t* outTypeJ = json_object_get(rootJ, "outType");
		if (outTypeJ) {
			outType = json_integer_value(outTypeJ);
			if (outType < 0 || outType > 2)
				outType = 0;
		}

		/*json_t* rstOnRunJ = json_object_get(rootJ, "rstOnRun");
		if (rstOnRunJ) {
			rstOnRun = json_boolean_value(rstOnRunJ);
		}*/

		json_t* dontAdvanceSettingJ = json_object_get(rootJ, "dontAdvanceSetting");
		if (dontAdvanceSettingJ) {
			dontAdvanceSetting = json_boolean_value(dontAdvanceSettingJ);
		}

		/*json_t* turingModeJ = json_object_get(rootJ, "turingMode");
		if (turingModeJ) {
			turingMode = json_boolean_value(turingModeJ);
		}*/

		json_t* bitResolutionJ = json_object_get(rootJ, "bitResolution");
		if (bitResolutionJ) {
			bitResolution = json_integer_value(bitResolutionJ);
			if (bitResolution < 0 && bitResolution > 1)
				bitResolution = BIT_8;
		}

		json_t* progressionJ = json_object_get(rootJ, "progression");
		if (progressionJ) {
			progression = json_integer_value(progressionJ);
			if (progression < 0 && progression > 2)
				progression = STD2x_PROGRESSION;
		}


		for (int t = 0; t < 8; t++) {
			json_t *wSeq_json_array = json_object_get(rootJ, ("t"+to_string(t)).c_str());
			size_t st;
			json_t *wSeq_json_value;
			if (wSeq_json_array) {
				json_array_foreach(wSeq_json_array, st, wSeq_json_value) {
					shiftRegister[t][st] = json_integer_value(wSeq_json_value);
				}
			}
		}


		for (int t = 0; t < 8; t++) {
			json_t *wSteps_json_array = json_object_get(rootJ, ("wSteps"+to_string(t)).c_str());
			size_t jSteps;
			json_t *wSteps_json_value;
			if (wSteps_json_array) {
				json_array_foreach(wSteps_json_array, jSteps, wSteps_json_value) {
					params[LENGTH_PARAM+t].setValue(json_integer_value(wSteps_json_value));
				}
			}
		}

		for (int t = 0; t < 8; t++) {
			json_t *wRst_json_array = json_object_get(rootJ, ("wRst"+to_string(t)).c_str());
			size_t jRst;
			json_t *wRst_json_value;
			if (wRst_json_array) {
				json_array_foreach(wRst_json_array, jRst, wRst_json_value) {
					params[SCALE_PARAM+t].setValue(json_real_value(wRst_json_value));
				}
			}
		}

		for (int t = 0; t < 8; t++) {
			json_t *wOfs_json_array = json_object_get(rootJ, ("wOfs"+to_string(t)).c_str());
			size_t jOfs;
			json_t *wOfs_json_value;
			if (wOfs_json_array) {
				json_array_foreach(wOfs_json_array, jOfs, wOfs_json_value) {
					params[OFFSET_PARAM+t].setValue(json_real_value(wOfs_json_value));
				}
			}
		}

	}

	void menuLoadSingle() {
		static const char FILE_FILTERS[] = "randLoops8 preset (.r8p):r8sp,R8P";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		if (path)
			loadSingle(path);

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadSingle(std::string path) {

		FILE *file = fopen(path.c_str(), "r");
		json_error_t error;
		json_t *rootJ = json_loadf(file, 0, &error);
		if (rootJ == NULL) {
			WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		}

		fclose(file);

		if (rootJ) {

			singleFromJson(rootJ);

		} else {
			WARN("problem loading preset json file");
			//return;
		}
		
	}

	void menuSaveSingle() {

		static const char FILE_FILTERS[] = "randLoops8 preset (.r8p):r8p,R8P";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".r8p" and strPath.substr(strPath.size() - 4) != ".R8P")
				strPath += ".r8p";
			path = strcpy(new char[strPath.length() + 1], strPath.c_str());
			saveSingle(path, singleToJson());
		}

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void saveSingle(std::string path, json_t *rootJ) {

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

	// ------------------------ LOAD / SAVE   SINGLE SEQUENCE

	json_t *sequenceToJson(int t) {

		sequence_to_saveRegister(t);

		json_t *rootJ = json_object();

		json_t *wSeq_json_array = json_array();
		for (int st = 0; st < 16; st++) {
			json_array_append_new(wSeq_json_array, json_integer(saveRegister[t][st]));
		}
		json_object_set_new(rootJ, "sr", wSeq_json_array);	
		json_object_set_new(rootJ, "length", json_integer((int)params[LENGTH_PARAM+t].getValue()));
		json_object_set_new(rootJ, "reset", json_real(params[SCALE_PARAM+t].getValue()));
		json_object_set_new(rootJ, "offset", json_real(params[OFFSET_PARAM+t].getValue()));

		return rootJ;

	}

	void sequenceFromJson(int t, json_t *rootJ) {

		json_t *wSeq_json_array = json_object_get(rootJ, "sr");
		size_t st;
		json_t *wSeq_json_value;
		if (wSeq_json_array) {
			json_array_foreach(wSeq_json_array, st, wSeq_json_value) {
				shiftRegister[t][st] = json_integer_value(wSeq_json_value);
			}
		}
		startingStep[t] = 0;

		json_t* lengthJ = json_object_get(rootJ, "length");
		if (lengthJ) {
			if (json_integer_value(lengthJ) < 1 || json_integer_value(lengthJ) > 16)
				params[LENGTH_PARAM+t].setValue(16);
			else
				params[LENGTH_PARAM+t].setValue(int(json_integer_value(lengthJ)));
		}

		json_t* rstJ = json_object_get(rootJ, "reset");
		if (rstJ) {
			if (json_real_value(rstJ) < 0 || json_real_value(rstJ) > 1)
				params[SCALE_PARAM+t].setValue(0);
			else
				params[SCALE_PARAM+t].setValue(json_real_value(rstJ));
		}

		json_t* ofsJ = json_object_get(rootJ, "offset");
		if (ofsJ) {
			if (json_real_value(ofsJ) < -10 || json_real_value(ofsJ) > 10)
				params[OFFSET_PARAM+t].setValue(0);
			else
				params[OFFSET_PARAM+t].setValue(json_real_value(ofsJ));
		}
	}

	void menuLoadSequence(int track) {
		static const char FILE_FILTERS[] = "trigSeq sequence (.tss):tss,TSS";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		if (path)
			loadSequence(track, path);

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadSequence(int track, std::string path) {

		FILE *file = fopen(path.c_str(), "r");
		json_error_t error;
		json_t *rootJ = json_loadf(file, 0, &error);
		if (rootJ == NULL) {
			WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		}

		fclose(file);

		if (rootJ) {

			sequenceFromJson(track, rootJ);

		} else {
			WARN("problem loading preset json file");
			//return;
		}
		
	}

	void menuSaveSequence(int track) {

		static const char FILE_FILTERS[] = "trigSeq sequence (.tss):tss,TSS";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".tss" and strPath.substr(strPath.size() - 4) != ".TSS")
				strPath += ".tss";
			path = strcpy(new char[strPath.length() + 1], strPath.c_str());
			saveSequence(path, sequenceToJson(track));
		}

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void saveSequence(std::string path, json_t *rootJ) {

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
	
	/*
	void inline debugCurrent(int t) {
		if (t == 0) {
			DEBUG ("%i%i%i%i%i%i%i%i%i%i%i%i%i%i%i%i", shiftRegister[t][0], shiftRegister[t][1], shiftRegister[t][2], shiftRegister[t][3],
					shiftRegister[t][4], shiftRegister[t][5], shiftRegister[t][6], shiftRegister[t][7], shiftRegister[t][8], shiftRegister[t][9],
					shiftRegister[t][10], shiftRegister[t][11], shiftRegister[t][12], shiftRegister[t][13], shiftRegister[t][14], shiftRegister[t][15]);
		}
	}
	*/
	/*
	void inline debugResettt(int t) {
		if (t == 0) {
			DEBUG ("%i%i%i%i%i%i%i%i%i%i%i%i%i%i%i%i RESET %i", shiftRegister[t][0], shiftRegister[t][1], shiftRegister[t][2], shiftRegister[t][3],
					shiftRegister[t][4], shiftRegister[t][5], shiftRegister[t][6], shiftRegister[t][7], shiftRegister[t][8], shiftRegister[t][9],
					shiftRegister[t][10], shiftRegister[t][11], shiftRegister[t][12], shiftRegister[t][13], shiftRegister[t][14], shiftRegister[t][15],
					startingStep[t]);
		}
	}
	*/

	
	void inline sequence_to_saveRegister(int t) {

		int cursor = startingStep[t];
		int wSteps = int(params[LENGTH_PARAM+t].getValue());
		//for (int i = 0; i <= wSteps; i++) {
		for (int i = 0; i < wSteps; i++) {
			tempSaveRegister[i] = shiftRegister[t][cursor];
			cursor++;
			if (cursor > 15)
				cursor = 0;
		}
		int fillCursor = 0;
		for (int i = wSteps; i < 16; i++) {
			tempSaveRegister[i] = tempSaveRegister[fillCursor];
			fillCursor++;
			if (fillCursor >= wSteps)
				fillCursor = 0;
		}
		for (int i = 0; i < 16; i++)
			saveRegister[t][i] = tempSaveRegister[i];
	}

	void inline resetCheck(int t) {
		if (rstValue[t] >= 1.f && prevRstValue[t] < 1.f) {
			
			int cursor = startingStep[t];
			int wSteps = params[LENGTH_PARAM+t].getValue();

			//for (int i = 0; i <= wSteps; i++) {
			for (int i = 0; i < wSteps; i++) {
				tempRegister[i] = shiftRegister[t][cursor];
				cursor++;
				if (cursor > 15)
					cursor = 0;
			}

			int fillCursor = 0;
			for (int i = wSteps; i < 16; i++) {
				tempRegister[i] = tempRegister[fillCursor];
				fillCursor++;
				if (fillCursor >= wSteps)
					fillCursor = 0;
			}

			// -------------------------------

			for (int i = 0; i < 16; i++)
				shiftRegister[t][i] = tempRegister[i];

			startingStep[t] = 0;

			if (dontAdvanceSetting)
				dontAdvance[t] = true;
		
			calcVoltage(t);

		}
		prevRstValue[t] = rstValue[t];
	}

	void inline calcVoltage(int t) {
		volt[t] = 0;
		for (int i=0; i < bitResTable[bitResolution]; i++) {
			if (shiftRegister[t][i])
				volt[t] += tableVolts[progression][bitResolution][i];
		}
	}

	void clearAll() {
		for (int t = 0; t < 8; t++) {
			for (int step = 0; step < 16; step++)
				shiftRegister[t][step] = 0;
			calcVoltage(t);
		}
	}

	void copyAllTracks() {
		for (int t = 0; t < 8; t++) {
			sequence_to_saveRegister(t);

			for (int st = 0; st < 16; st++)
				randLoops8_cbSeq[t][st] = saveRegister[t][st];

			randLoops8_cbSteps[t] = params[LENGTH_PARAM+t].getValue();
			randLoops8_cbCtrl[t] = params[CTRL_PARAM+t].getValue();
			randLoops8_cbScale[t] = params[SCALE_PARAM+t].getValue();
			randLoops8_cbOffset[t] = params[OFFSET_PARAM+t].getValue();
		}
		
		randLoops8_clipboard = true;
	}

	void pasteAllTracks() {
		for (int t = 0; t < 8; t++) {
			for (int st = 0; st < 16; st++)
				shiftRegister[t][st] = randLoops8_cbSeq[t][st];

			params[LENGTH_PARAM+t].setValue(randLoops8_cbSteps[t]);
			params[CTRL_PARAM+t].setValue(randLoops8_cbCtrl[t]);
			params[SCALE_PARAM+t].setValue(randLoops8_cbScale[t]);
			params[OFFSET_PARAM+t].setValue(randLoops8_cbOffset[t]);
		}
	}

	void copyTrack(int t) {

		sequence_to_saveRegister(t);

		for (int i = 0; i < 16; i++)
			randLoops_cbSeq[i] = saveRegister[t][i];
		
		randLoops_cbSteps = params[LENGTH_PARAM+t].getValue();
		randLoops_cbCtrl = params[CTRL_PARAM+t].getValue();
		randLoops_cbScale = params[SCALE_PARAM+t].getValue();
		randLoops_cbOffset = params[OFFSET_PARAM+t].getValue();
		randLoops_clipboard = true;
	}

	void pasteToTrack(int t) {
		for (int i = 0; i < 16; i++)
			shiftRegister[t][i] = randLoops_cbSeq[i];
		
		startingStep[t] = 0;
		params[LENGTH_PARAM+t].setValue(randLoops_cbSteps);
		params[CTRL_PARAM+t].setValue(randLoops_cbCtrl);
		params[SCALE_PARAM+t].setValue(randLoops_cbScale);
		params[OFFSET_PARAM+t].setValue(randLoops_cbOffset);
	}


	
	void process(const ProcessArgs& args) override {
		
		for (int t = 0; t < 8; t++) {

			if (!inputs[CLK_INPUT+t].isConnected() && t != 0) {
				
				if (inputs[RST_INPUT+t].isConnected())
					rstValue[t] = inputs[RST_INPUT+t].getVoltage();
				else
					rstValue[t] = rstValue[t-1];

				resetCheck(t);

				clock[t] = clock[t-1];

			} else {

				rstValue[t] = inputs[RST_INPUT+t].getVoltage();
				
				resetCheck(t);

				clock[t] = inputs[CLK_INPUT+t].getVoltage();

			}

			if (clock[t] >= 1.f && prevClock[t] < 1.f) {

				if (!dontAdvance[t]) {

					startingStep[t]++;

					if (startingStep[t] >= int(params[LENGTH_PARAM+t].getValue()))
						startingStep[t] = 0;

					controlValue = params[CTRL_PARAM+t].getValue() + (inputs[CTRL_INPUT+t].getVoltage() / 10.f);
					if (controlValue < -1.f)
						controlValue = -1.f;
					else if (controlValue > 1.f)
						controlValue = 1.f;

					probCtrl = int(abs(controlValue * 10));
					probCtrlRnd = int(random::uniform() * 10);

					if (probCtrlRnd > probCtrl) {

						probRegister = random::uniform();
						if (probRegister > 0.5f)
							incomingRegister = 1;
						else
							incomingRegister = 0;

					} else {

						incomingRegister = shiftRegister[t][int(params[LENGTH_PARAM+t].getValue())-1];
						if (controlValue < 0)
							incomingRegister = !incomingRegister;
					}

					shiftRegister[t][15] = shiftRegister[t][14];
					shiftRegister[t][14] = shiftRegister[t][13];
					shiftRegister[t][13] = shiftRegister[t][12];
					shiftRegister[t][12] = shiftRegister[t][11];
					shiftRegister[t][11] = shiftRegister[t][10];
					shiftRegister[t][10] = shiftRegister[t][9];
					shiftRegister[t][9] = shiftRegister[t][8];
					shiftRegister[t][8] = shiftRegister[t][7];
					shiftRegister[t][7] = shiftRegister[t][6];
					shiftRegister[t][6] = shiftRegister[t][5];
					shiftRegister[t][5] = shiftRegister[t][4];
					shiftRegister[t][4] = shiftRegister[t][3];
					shiftRegister[t][3] = shiftRegister[t][2];
					shiftRegister[t][2] = shiftRegister[t][1];
					shiftRegister[t][1] = shiftRegister[t][0];
					shiftRegister[t][0] = incomingRegister;

					calcVoltage(t);

					//debugCurrent(t);

					if (incomingRegister) {
						stepPulse[t] = true;
						stepPulseTime[t] = oneMsTime;
						if (outType == OUT_GATE)
							outGate[t] = true;
					} else {
						if (outType == OUT_GATE) {
							outGate[t] = false;
							trgOut[t] = 0.f;
						}
					}

				} else {
					dontAdvance[t] = false;
				}

			}
			prevClock[t] = clock[t];

			cvOut[t] = (volt[t] * params[SCALE_PARAM+t].getValue()) + params[OFFSET_PARAM+t].getValue();

			if (cvOut[t] > 10.f)
				cvOut[t] = 10.f;
			else if (cvOut[t] < -10.f)
				cvOut[t] = -10.f;

			outputs[CV_OUTPUT+t].setVoltage(cvOut[t]);

			if (stepPulse[t]) {

				if (outType == OUT_TRIG) {
					stepPulseTime[t]--;
					if (stepPulseTime[t] < 0) {
						stepPulse[t] = false;
						trgOut[t] = 0.f;
					} else {
						trgOut[t] = 10.f;
					}
				} else if (outType == OUT_CLOCK) {
					trgOut[t] = clock[t];
					if (trgOut[t] < 1.f) {
						trgOut[t] = 0.f;
						stepPulse[t] = false;
					}
				} else if (outType == OUT_GATE) {
					if (outGate[t])
						trgOut[t] = 10.f;
					else
						trgOut[t] = 0.f;
				}
			}

			outputs[TRG_OUTPUT+t].setVoltage(trgOut[t]);

		}
		
		if (polyChans == 0) {
			outputs[TRG_OUTPUT+7].setChannels(1);
			outputs[CV_OUTPUT+7].setChannels(1);
		} else {
			for (int c = 0; c < polyChans + 1; c++) {
				outputs[TRG_OUTPUT+7].setVoltage(trgOut[c], c);
				outputs[CV_OUTPUT+7].setVoltage(cvOut[c], c);
			}
			outputs[TRG_OUTPUT+7].setChannels(polyChans+1);
			outputs[CV_OUTPUT+7].setChannels(polyChans+1);
		}

	}
};

struct RandLoops8Widget : ModuleWidget {
	RandLoops8Widget(RandLoops8* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RandLoops8.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		constexpr float yStart = 18.9f;
		constexpr float yDelta = 14.f;

		const float xClk = 6.6f;

		const float xRst = 16.3f;

		const float xCtrlKn = 26.4f;
		const float xCtrlIn = 36.3f;

		const float xLength = 45.9f;
		
		const float xScale = 55.7f;

		const float xOfs = 64.6f;

		const float xTrg = 74.9f;

		const float xOut = 84.5f;

		for (int track = 0; track < 8; track++) {

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xClk, yStart+(track*yDelta))), module, RandLoops8::CLK_INPUT+track));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRst, yStart+(track*yDelta))), module, RandLoops8::RST_INPUT+track));

			addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xCtrlKn, yStart+(track*yDelta))), module, RandLoops8::CTRL_PARAM+track));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCtrlIn, yStart+(track*yDelta))), module, RandLoops8::CTRL_INPUT+track));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xLength, yStart+(track*yDelta))), module, RandLoops8::LENGTH_PARAM+track));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xScale, yStart+(track*yDelta))), module, RandLoops8::SCALE_PARAM+track));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xOfs, yStart+(track*yDelta))), module, RandLoops8::OFFSET_PARAM+track));

			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xTrg, yStart+(track*yDelta))), module, RandLoops8::TRG_OUTPUT+track));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yStart+(track*yDelta))), module, RandLoops8::CV_OUTPUT+track));

		}
	}

	void appendContextMenu(Menu* menu) override {
		RandLoops8* module = dynamic_cast<RandLoops8*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Math Settings"));

		struct BitResTypeItem : MenuItem {
			RandLoops8* module;
			int bitResType;
			void onAction(const event::Action& e) override {
				module->bitResolution = bitResType;
			}
		};
		std::string BitResTypeNames[2] = {"8 bit", "16 bit"};
		menu->addChild(createSubmenuItem("Bit Resolution", (BitResTypeNames[module->bitResolution]), [=](Menu * menu) {
			for (int i = 0; i < 2; i++) {
				BitResTypeItem* bitResTypeItem = createMenuItem<BitResTypeItem>(BitResTypeNames[i]);
				bitResTypeItem->rightText = CHECKMARK(module->bitResolution == i);
				bitResTypeItem->module = module;
				bitResTypeItem->bitResType = i;
				menu->addChild(bitResTypeItem);
			}
		}));

		struct ProgressionTypeItem : MenuItem {
			RandLoops8* module;
			int progressionType;
			void onAction(const event::Action& e) override {
				module->progression = progressionType;
			}
		};
		std::string ProgressionTypeNames[3] = {"2x (std.)", "1.3x", "Fibonacci"};
		menu->addChild(createSubmenuItem("Voltage progression", (ProgressionTypeNames[module->progression]), [=](Menu * menu) {
			for (int i = 0; i < 3; i++) {
				ProgressionTypeItem* progressionTypeItem = createMenuItem<ProgressionTypeItem>(ProgressionTypeNames[i]);
				progressionTypeItem->rightText = CHECKMARK(module->progression == i);
				progressionTypeItem->module = module;
				progressionTypeItem->progressionType = i;
				menu->addChild(progressionTypeItem);
			}
		}));

		menu->addChild(new MenuSeparator());

		struct OutTypeItem : MenuItem {
			RandLoops8* module;
			int outType;
			void onAction(const event::Action& e) override {
				module->outType = outType;
			}
		};
		std::string OutTypeNames[3] = {"Trig", "Gate", "Clock Width"};
		menu->addChild(createSubmenuItem("Output TRG type", (OutTypeNames[module->outType]), [=](Menu * menu) {
			for (int i = 0; i < 3; i++) {
				OutTypeItem* outTypeItem = createMenuItem<OutTypeItem>(OutTypeNames[i]);
				outTypeItem->rightText = CHECKMARK(module->outType == i);
				outTypeItem->module = module;
				outTypeItem->outType = i;
				menu->addChild(outTypeItem);
			}
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Polyphony on 8th out", std::to_string(module->polyChans + 1), [=](Menu * menu) {
			menu->addChild(createMenuItem("Monophonic", "", [=]() {module->polyChans = 0;}));
			menu->addChild(createMenuItem("2", "", [=]() {module->polyChans = 1;}));
			menu->addChild(createMenuItem("3", "", [=]() {module->polyChans = 2;}));
			menu->addChild(createMenuItem("4", "", [=]() {module->polyChans = 3;}));
			menu->addChild(createMenuItem("5", "", [=]() {module->polyChans = 4;}));
			menu->addChild(createMenuItem("6", "", [=]() {module->polyChans = 5;}));
			menu->addChild(createMenuItem("7", "", [=]() {module->polyChans = 6;}));
			menu->addChild(createMenuItem("8", "", [=]() {module->polyChans = 7;}));
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("1st clock after reset:"));
		menu->addChild(createBoolPtrMenuItem("Don't advance", "", &module->dontAdvanceSetting));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Copy All Tracks", "", [=]() {module->copyAllTracks();}));
		if (randLoops8_clipboard) {
			menu->addChild(createMenuItem("Paste All Tracks", "", [=]() {module->pasteAllTracks();}));
		} else {
			menu->addChild(createMenuLabel("Paste All Tracks"));
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Copy track", "", [=](Menu * menu) {
			menu->addChild(createMenuItem("Copy Track 1", "", [=]() {module->copyTrack(0);}));
			menu->addChild(createMenuItem("Copy Track 2", "", [=]() {module->copyTrack(1);}));
			menu->addChild(createMenuItem("Copy Track 3", "", [=]() {module->copyTrack(2);}));
			menu->addChild(createMenuItem("Copy Track 4", "", [=]() {module->copyTrack(3);}));
			menu->addChild(createMenuItem("Copy Track 5", "", [=]() {module->copyTrack(4);}));
			menu->addChild(createMenuItem("Copy Track 6", "", [=]() {module->copyTrack(5);}));
			menu->addChild(createMenuItem("Copy Track 7", "", [=]() {module->copyTrack(6);}));
			menu->addChild(createMenuItem("Copy Track 8", "", [=]() {module->copyTrack(7);}));
		}));

		if (randLoops_clipboard) {
			menu->addChild(createSubmenuItem("Paste to track", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("Paste Track 1", "", [=]() {module->pasteToTrack(0);}));
				menu->addChild(createMenuItem("Paste Track 2", "", [=]() {module->pasteToTrack(1);}));
				menu->addChild(createMenuItem("Paste Track 3", "", [=]() {module->pasteToTrack(2);}));
				menu->addChild(createMenuItem("Paste Track 4", "", [=]() {module->pasteToTrack(3);}));
				menu->addChild(createMenuItem("Paste Track 5", "", [=]() {module->pasteToTrack(4);}));
				menu->addChild(createMenuItem("Paste Track 6", "", [=]() {module->pasteToTrack(5);}));
				menu->addChild(createMenuItem("Paste Track 7", "", [=]() {module->pasteToTrack(6);}));
				menu->addChild(createMenuItem("Paste Track 8", "", [=]() {module->pasteToTrack(7);}));
			}));
		} else {
			menu->addChild(createMenuLabel("Paste to track"));
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("DISK operations", "", [=](Menu * menu) {
			
			menu->addChild(createMenuItem("Load Preset", "", [=]() {module->menuLoadSingle();}));
			menu->addChild(createMenuItem("Save Preset", "", [=]() {module->menuSaveSingle();}));

			menu->addChild(new MenuSeparator());

			menu->addChild(createSubmenuItem("Import trigSeq seq. to:", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("Track 1", "", [=]() {module->menuLoadSequence(0);}));
				menu->addChild(createMenuItem("Track 2", "", [=]() {module->menuLoadSequence(1);}));
				menu->addChild(createMenuItem("Track 3", "", [=]() {module->menuLoadSequence(2);}));
				menu->addChild(createMenuItem("Track 4", "", [=]() {module->menuLoadSequence(3);}));
				menu->addChild(createMenuItem("Track 5", "", [=]() {module->menuLoadSequence(4);}));
				menu->addChild(createMenuItem("Track 6", "", [=]() {module->menuLoadSequence(5);}));
				menu->addChild(createMenuItem("Track 7", "", [=]() {module->menuLoadSequence(6);}));
				menu->addChild(createMenuItem("Track 8", "", [=]() {module->menuLoadSequence(7);}));
			}));
			menu->addChild(createSubmenuItem("Export trigSeq seq. from:", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("Track 1", "", [=]() {module->menuSaveSequence(0);}));
				menu->addChild(createMenuItem("Track 2", "", [=]() {module->menuSaveSequence(1);}));
				menu->addChild(createMenuItem("Track 3", "", [=]() {module->menuSaveSequence(2);}));
				menu->addChild(createMenuItem("Track 4", "", [=]() {module->menuSaveSequence(3);}));
				menu->addChild(createMenuItem("Track 5", "", [=]() {module->menuSaveSequence(4);}));
				menu->addChild(createMenuItem("Track 6", "", [=]() {module->menuSaveSequence(5);}));
				menu->addChild(createMenuItem("Track 7", "", [=]() {module->menuSaveSequence(6);}));
				menu->addChild(createMenuItem("Track 8", "", [=]() {module->menuSaveSequence(7);}));
			}));
		}));
		menu->addChild(createMenuItem("Clear ALL Sequences", "", [=]() {module->clearAll();}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}
};

Model* modelRandLoops8 = createModel<RandLoops8, RandLoops8Widget>("RandLoops8");