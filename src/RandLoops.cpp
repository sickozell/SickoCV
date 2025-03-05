#define STD2x_PROGRESSION 0
#define P_1_3_PROGRESSION 2 // 1.3x
#define FIBONACCI_PROGRESSION 3

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

struct RandLoops : Module {
	enum ParamId {
		CTRL_PARAM,
		LENGTH_PARAM,
		SCALE_PARAM,
		DEL_BUTTON,
		ADD_BUTTON,
		RND_BUTTON,
		PARAMS_LEN
	};
	enum InputId {
		CTRL_INPUT,
		DEL_INPUT,
		ADD_INPUT,
		CLK_INPUT,
		RST_INPUT,
		CLEAR_INPUT,
		RND_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		DEL_LIGHT,
		ADD_LIGHT,
		RND_LIGHT,
		ENUMS(REGISTER_LIGHT, 16),
		LIGHTS_LEN
	};

	bool initStart = false;

	float clock = 0;
	float prevClock = 0;

	float rstValue = 0;
	float prevRstValue = 0;

	float volt = 0;
	float out = 0;

	float controlValue = 0;

	bool dontAdvance = false;
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

	//bool shiftRegister[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	int shiftRegister[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	//bool saveRegister[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	int saveRegister[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	//bool tempRegister[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	int tempRegister[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	
	//bool tempSaveRegister[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	int tempSaveRegister[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	float probCtrl = 0;
	float probCtrlRnd = 0;
	float probRegister = 0;

	bool incomingRegister = false;

	int startingStep = 0;

	int bitResolution = BIT_8;
	int bitRes[2] = {8, 16};

	std::string resolutionName[2] = {"8 bit", "16 bit"};
	std::string progressionName[3] = {"2x (std)", "1.3x", "Fibonacci"};

	bool bufferedAddDel = true;
	bool bufferedRandom = true;

	float delTrig = 0;
	float prevDelTrig = 0;
	bool delWait = false;

	float addTrig = 0;
	float prevAddTrig = 0;
	bool addWait = false;

	float rndTrig = 0;
	float prevRndTrig = 0;
	bool rndWait = false;

	float clrValue = 0;
	float prevClrValue = 0;
	
	//bool pulse = false;
	//float pulseTime = 0;

	int tableLength[8] = {1, 2, 3, 4, 5, 7, 11, 15};

	bool trigMode = false;

	int outType = OUT_TRIG;

	float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	bool stepPulse = false;
	float stepPulseTime = 0;
	bool outGate = false;

	RandLoops() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(CTRL_PARAM, -1, 1.f, 0.f, "Control");
		configInput(CTRL_INPUT, "Ctrl CV");

		configParam(LENGTH_PARAM, 1.f, 16.f, 8.f, "Length");
		paramQuantities[LENGTH_PARAM]->snapEnabled = true;

		configParam(SCALE_PARAM, 0.f, 1.f, 1.f, "Scale", "%", 0, 100);

		configSwitch(DEL_BUTTON, 0.f, 1.f, 0.f, "Delete", {"OFF", "ON"});
		configSwitch(ADD_BUTTON, 0.f, 1.f, 0.f, "Add", {"OFF", "ON"});
		configSwitch(RND_BUTTON, 0.f, 1.f, 0.f, "Random", {"OFF", "ON"});

		configInput(DEL_INPUT, "Delete");
		configInput(ADD_INPUT, "Add");
		configInput(RND_INPUT, "Random");

		configInput(CLK_INPUT, "Clock");
		configInput(CLEAR_INPUT, "Clear");

		configInput(RST_INPUT, "Reset");
		configOutput(OUT_OUTPUT, "Output");

		if (!trigMode)
			calcVoltage();

	}

	void onReset(const ResetEvent &e) override {

		for (int i=0; i < 16; i++) {
			shiftRegister[i] = false;
			lights[REGISTER_LIGHT+i].setBrightness(0.f);
		}

		rndWait = false;
		addWait = false;
		delWait = false;

		initStart = false;
		clearAll();

		Module::onReset(e);
	}

	void onSampleRateChange() override {
		oneMsTime = (APP->engine->getSampleRate()) / 1000;
	}
	
	json_t* dataToJson() override {

		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "initStart", json_boolean(initStart));
		json_object_set_new(rootJ, "dontAdvanceSetting", json_boolean(dontAdvanceSetting));
		json_object_set_new(rootJ, "bitResolution", json_integer(bitResolution));
		json_object_set_new(rootJ, "progression", json_integer(progression));
		json_object_set_new(rootJ, "bufferedAddDel", json_boolean(bufferedAddDel));
		json_object_set_new(rootJ, "bufferedRandom", json_boolean(bufferedRandom));
		json_object_set_new(rootJ, "trigMode", json_boolean(trigMode));
		json_object_set_new(rootJ, "outType", json_integer(outType));
		
		storeSequence();

		json_t *track_json_array = json_array();
		for (int tempStep = 0; tempStep < 16; tempStep++) {
			json_array_append_new(track_json_array, json_integer(saveRegister[tempStep]));
		}
		json_object_set_new(rootJ, "sr", track_json_array);	

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

		json_t* dontAdvanceSettingJ = json_object_get(rootJ, "dontAdvanceSetting");
		if (dontAdvanceSettingJ) {
			dontAdvanceSetting = json_boolean_value(dontAdvanceSettingJ);
		}
		
		json_t* bufferedAddDelJ = json_object_get(rootJ, "bufferedAddDel");
		if (bufferedAddDelJ)
			bufferedAddDel = json_boolean_value(bufferedAddDelJ);

		json_t* bufferedRandomJ = json_object_get(rootJ, "bufferedRandom");
		if (bufferedRandomJ)
			bufferedRandom = json_boolean_value(bufferedRandomJ);

		json_t* trigModeJ = json_object_get(rootJ, "trigMode");
		if (trigModeJ)
			trigMode = json_boolean_value(trigModeJ);

		json_t* outTypeJ = json_object_get(rootJ, "outType");
		if (outTypeJ) {
			outType = json_integer_value(outTypeJ);
			if (outType < 0 || outType > 2)
				outType = 0;
		}

		if (!initStart) {

			json_t *track_json_array = json_object_get(rootJ, "sr");
			size_t tempSeq;
			json_t *json_value;
			if (track_json_array) {
				json_array_foreach(track_json_array, tempSeq, json_value) {
					shiftRegister[tempSeq] = json_integer_value(json_value);
				}
			}
		}
	}

	json_t *sequenceToJson() {
		storeSequence();

		json_t *rootJ = json_object();
		
		json_t *prog_json_array = json_array();
		for (int tempStep = 0; tempStep < 16; tempStep++) {
			json_array_append_new(prog_json_array, json_integer(saveRegister[tempStep]));
		}
		json_object_set_new(rootJ, "sr", prog_json_array);	
	
		json_object_set_new(rootJ, "length", json_integer((int)params[LENGTH_PARAM].getValue()));
		return rootJ;
	}

	void sequenceFromJson(json_t *rootJ) {

		json_t *prog_json_array = json_object_get(rootJ, "sr");
		size_t tempSeq;
		json_t *json_value;
		if (prog_json_array) {
			json_array_foreach(prog_json_array, tempSeq, json_value) {
				shiftRegister[tempSeq] = json_integer_value(json_value);
			}
		}
		startingStep = 0;

		json_t* lengthJ = json_object_get(rootJ, "length");
		if (lengthJ) {
			if (json_integer_value(lengthJ) < 1 || json_integer_value(lengthJ) > 16)
				params[LENGTH_PARAM].setValue(16);
			else
				params[LENGTH_PARAM].setValue(int(json_integer_value(lengthJ)));
		}

	}

	void menuLoadSequence() {
		static const char FILE_FILTERS[] = "trigSeq preset (.tss):tss,TSS";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		if (path)
			loadSequence(path);

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadSequence(std::string path) {

		FILE *file = fopen(path.c_str(), "r");
		json_error_t error;
		json_t *rootJ = json_loadf(file, 0, &error);
		if (rootJ == NULL) {
			WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		}

		fclose(file);

		if (rootJ) {

			sequenceFromJson(rootJ);

		} else {
			WARN("problem loading preset json file");
			//return;
		}
		
	}

	void menuSaveSequence() {

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
			saveSequence(path, sequenceToJson());
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


	void inline storeSequence() {
		int cursor = 0;
		for (int i = startingStep; i < int(params[LENGTH_PARAM].getValue()); i++) {
			tempSaveRegister[cursor] = shiftRegister[i];
			cursor++;
		}

		for (int i = 0; i < startingStep; i++) {
			tempSaveRegister[cursor] = shiftRegister[i];
			cursor++;
		}

		int fillCursor = 0;
		for (int i = cursor; i < 16; i++) {
			tempSaveRegister[i] = tempSaveRegister[fillCursor];
			fillCursor++;
			if (fillCursor >= int(params[LENGTH_PARAM].getValue()))
				fillCursor = 0;
		}

		for (int i = 0; i < 16; i++)
			saveRegister[i] = tempSaveRegister[i];

	}

	void inline resetSequence() {
		int cursor = 0;
		for (int i = startingStep; i < int(params[LENGTH_PARAM].getValue()); i++) {
			tempRegister[cursor] = shiftRegister[i];
			cursor++;
		}

		for (int i = 0; i < startingStep; i++) {
			tempRegister[cursor] = shiftRegister[i];
			cursor++;
		}

		int fillCursor = 0;
		for (int i = cursor; i < 16; i++) {
			tempRegister[i] = tempRegister[fillCursor];
			fillCursor++;
			if (fillCursor >= int(params[LENGTH_PARAM].getValue()))
				fillCursor = 0;
		}

		for (int i = 0; i < 16; i++)
			shiftRegister[i] = tempRegister[i];

	}

	void inline resetCheck() {
		if (rstValue >= 1.f && prevRstValue < 1.f) {
			
			resetSequence();

			//debugResettt(t);

			startingStep = 0;

			if (dontAdvanceSetting)
				dontAdvance = true;
		
			if (!trigMode)
				calcVoltage();

		}
		prevRstValue = rstValue;
	}

	void inline calcVoltage() {
		volt = 0;
		for (int i=0; i < bitRes[bitResolution]; i++) {
			if (shiftRegister[i])
				volt += tableVolts[progression][bitResolution][i];
		}
	}

	void clearAll() {
		for (int step = 0; step < 16; step++)
			shiftRegister[step] = 0;

		if (!trigMode)
			calcVoltage();
	}

	void inline calcRandom() {
		//int rndLength = 1 + tableLength[int(params[LENGTH_PARAM].getValue())];

		int cursor = 0;
		for (int i = 0; i < int(params[LENGTH_PARAM].getValue()); i++) {
			probRegister = random::uniform();
			if (probRegister > 0.5)
				shiftRegister[i] = 1;
			else
				shiftRegister[i] = 0;
			cursor++;
		}


		int fillCursor = 0;
		for (int i = cursor; i < 16; i++) {
			shiftRegister[i] = shiftRegister[fillCursor];
			fillCursor++;
			if (fillCursor >= int(params[LENGTH_PARAM].getValue()))
				fillCursor = 0;
		}

		for (int i=0; i<16; i++)
			lights[REGISTER_LIGHT+i].setBrightness(shiftRegister[i]);
	}
	
	void process(const ProcessArgs& args) override {

		// -------------------------------- clear sequence

		clrValue = inputs[CLEAR_INPUT].getVoltage();
		if (clrValue >= 1.f && prevClrValue < 1.f) {
			/*for (int i = 0; i < 8; i++) {
				shiftRegister[i] = false;
				lights[REGISTER_LIGHT+i].setBrightness(0.f);
			}

			for (int i = 8; i < 16; i++)
				shiftRegister[i] = false;
			*/
			for (int i = 0; i < 16; i++) {
				shiftRegister[i] = false;
				lights[REGISTER_LIGHT+i].setBrightness(0.f);
			}

			rndWait = false;
			addWait = false;
			delWait = false;

			lights[RND_LIGHT].setBrightness(0.f);
			lights[ADD_LIGHT].setBrightness(0.f);
			lights[DEL_LIGHT].setBrightness(0.f);

		}
		prevClrValue = clrValue;

		// -------------------------------- random trigger

		rndTrig = inputs[RND_INPUT].getVoltage() + params[RND_BUTTON].getValue();
		if (rndTrig >= 1.f && prevRndTrig < 1.f) {
			rndTrig = 1;
			if (bufferedRandom) {
				rndWait = true;
				lights[RND_LIGHT].setBrightness(1.f);
			} else {
				calcRandom();
				rndWait = false;
			}
		}
		prevRndTrig = rndTrig;

		if (!bufferedRandom)
			lights[RND_LIGHT].setBrightness(rndTrig);

		// -------------------------------- del trigger

		delTrig = inputs[DEL_INPUT].getVoltage() + params[DEL_BUTTON].getValue();
		
		if (bufferedAddDel) {
			if (delTrig >= 1.f && prevDelTrig < 1.f) {
				delTrig = 1;
				delWait = true;
				lights[DEL_LIGHT].setBrightness(1.f);
			}
		} else {
			if (delTrig >= 1.f) {
				delTrig = 1;
				delWait = true;
			}
			lights[DEL_LIGHT].setBrightness(delTrig);
		}
		
		prevDelTrig = delTrig;
		
		// -------------------------------- add trigger

		addTrig = inputs[ADD_INPUT].getVoltage() + params[ADD_BUTTON].getValue();
		
		if (bufferedAddDel) {
			if (addTrig >= 1.f && prevAddTrig < 1.f) {
				addTrig = 1;
				addWait = true;
				lights[ADD_LIGHT].setBrightness(1.f);
			}
		} else {
			if (addTrig >= 1.f) {
				addTrig = 1;
				addWait = true;
			}
			lights[ADD_LIGHT].setBrightness(addTrig);
		}

		prevAddTrig = addTrig;

		// -------------------------------- reset check

		rstValue = inputs[RST_INPUT].getVoltage();
				
		resetCheck();

		// -------------------------------- clock trigger

		clock = inputs[CLK_INPUT].getVoltage();
		if (clock >= 1.f && prevClock < 1.f) {

			if (!dontAdvance) {

				if (rndWait && bufferedRandom) {
					calcRandom();
					rndWait = false;
					lights[RND_LIGHT].setBrightness(0.f);
				}

				startingStep++;

				if (startingStep >= int(params[LENGTH_PARAM].getValue()))
					startingStep = 0;


				if (addWait) {

					incomingRegister = 1;
					addWait = false;
					if (bufferedAddDel)
						lights[ADD_LIGHT].setBrightness(0.f);

				} else if (delWait) {

					incomingRegister = 0;
					delWait = false;
					if (bufferedAddDel)
						lights[DEL_LIGHT].setBrightness(0.f);

				} else {

					controlValue = params[CTRL_PARAM].getValue() + (inputs[CTRL_INPUT].getVoltage() / 10.f);
					if (controlValue < -1.f)
						controlValue = -1.f;
					else if (controlValue > 1.f)
						controlValue = 1.f;

					probCtrl = int(abs(controlValue * 10));
					probCtrlRnd = int(random::uniform() * 10);

					if (probCtrlRnd > probCtrl) {
						probRegister = random::uniform();
						if (probRegister > 0.5)
							incomingRegister = 1;
						else
							incomingRegister = 0;

					} else {

						//incomingRegister = shiftRegister[tableLength[int(params[LENGTH_PARAM].getValue())]];
						incomingRegister = shiftRegister[int(params[LENGTH_PARAM].getValue())-1];
						if (controlValue < 0)
							incomingRegister = !incomingRegister;
					}
				}

				shiftRegister[15] = shiftRegister[14];
				shiftRegister[14] = shiftRegister[13];
				shiftRegister[13] = shiftRegister[12];
				shiftRegister[12] = shiftRegister[11];
				shiftRegister[11] = shiftRegister[10];
				shiftRegister[10] = shiftRegister[9];
				shiftRegister[9] = shiftRegister[8];
				shiftRegister[8] = shiftRegister[7];
				shiftRegister[7] = shiftRegister[6];
				shiftRegister[6] = shiftRegister[5];
				shiftRegister[5] = shiftRegister[4];
				shiftRegister[4] = shiftRegister[3];
				shiftRegister[3] = shiftRegister[2];
				shiftRegister[2] = shiftRegister[1];
				shiftRegister[1] = shiftRegister[0];
				shiftRegister[0] = incomingRegister;

				if (!trigMode)
					calcVoltage();
				else {
					if (incomingRegister) {
						stepPulse = true;
						stepPulseTime = oneMsTime;
						if (outType == OUT_GATE)
							outGate = true;
					} else {
						if (outType == OUT_GATE) {
							outGate = false;
							volt = 0.f;
						}
					}
				}

			} else {
				dontAdvance = false;
			}

		}
		prevClock = clock;

		if (trigMode) {

			if (stepPulse) {

				if (outType == OUT_TRIG) {
					stepPulseTime--;
					if (stepPulseTime < 0) {
						stepPulse = false;
						volt = 0.f;
					} else {
						volt = 10.f;
					}
				} else if (outType == OUT_CLOCK) {
					volt = clock;
					if (volt < 1.f) {
						volt = 0.f;
						stepPulse = false;
					}
				} else if (outType == OUT_GATE) {
					if (outGate)
						volt = 10.f;
					else
						volt = 0.f;
				}
			}
		}

		out = volt * params[SCALE_PARAM].getValue();

		if (out > 10.f)
			out = 10.f;
		else if (out < -10.f)
			out = -10.f;

		outputs[OUT_OUTPUT].setVoltage(out);

		for (int i = 0; i < 16; i++)
			lights[REGISTER_LIGHT+i].setBrightness(shiftRegister[i]);

		if (!bufferedAddDel) {
			delWait = false;
			addWait = false;
		}

		if (!bufferedRandom) {
			rndWait = false;
		}

	}
};

struct RandLoopsWidget : ModuleWidget {
	RandLoopsWidget(RandLoops* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RandLoops.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float xCtrl = 26;
		const float yCtrl = 20.5;

		const float xCtrlCV = 10;
		const float yCtrlCV = 24;

		const float xLength = 11.7;
		const float yLength = 46;

		const float xScale = 30.9;
		const float yScale = 51.4;

		const float xDelBut = 8.5;
		const float yDelBut = 69.5;

		const float xAddBut = 20.5;
		const float yAddBut = 69.5;

		const float xRndBut = 32.5;
		const float yRndBut = 69.5;

		const float xDelIn = 8.5;
		const float yDelIn = 79.5;

		const float xAddIn = 20.5;
		const float yAddIn = 79.5;

		const float xRndIn = 32.5;
		const float yRndIn = 79.5;

		const float xClk = 11.5;
		const float yClk = 98.5;

		//const float xClr = 29.5;
		//const float yClr = 98.5;

		const float xClr = 11;
		const float yClr = 116;

		

		const float xRst = 29.5;
		const float yRst = 98.5;

		const float xOut = 30.8;
		const float yOut = 116;

		const float xLgStart = 24.5;
		const float xLgShift = 4;
		const float yLgStart1 = 31;
		const float yLgStart2 = 34;
		const float yLgStart3 = 37;
		const float yLgStart4 = 40;
		
		addParam(createParamCentered<SickoBigKnob>(mm2px(Vec(xCtrl, yCtrl)), module, RandLoops::CTRL_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCtrlCV, yCtrlCV)), module, RandLoops::CTRL_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xLength, yLength)), module, RandLoops::LENGTH_PARAM));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xScale, yScale)), module, RandLoops::SCALE_PARAM));

		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xDelBut, yDelBut)), module, RandLoops::DEL_BUTTON, RandLoops::DEL_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<GreenLight>>(mm2px(Vec(xAddBut, yAddBut)), module, RandLoops::ADD_BUTTON, RandLoops::ADD_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xRndBut, yRndBut)), module, RandLoops::RND_BUTTON, RandLoops::RND_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xDelIn, yDelIn)), module, RandLoops::DEL_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xAddIn, yAddIn)), module, RandLoops::ADD_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRndIn, yRndIn)), module, RandLoops::RND_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xClk, yClk)), module, RandLoops::CLK_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xClr, yClr)), module, RandLoops::CLEAR_INPUT));
		
		
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart, yLgStart1)), module, RandLoops::REGISTER_LIGHT+0));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + xLgShift, yLgStart1)), module, RandLoops::REGISTER_LIGHT+1));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart1)), module, RandLoops::REGISTER_LIGHT+2));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart1)), module, RandLoops::REGISTER_LIGHT+3));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart, yLgStart2)), module, RandLoops::REGISTER_LIGHT+4));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + xLgShift, yLgStart2)), module, RandLoops::REGISTER_LIGHT+5));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart2)), module, RandLoops::REGISTER_LIGHT+6));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart2)), module, RandLoops::REGISTER_LIGHT+7));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart, yLgStart3)), module, RandLoops::REGISTER_LIGHT+8));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + xLgShift, yLgStart3)), module, RandLoops::REGISTER_LIGHT+9));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart3)), module, RandLoops::REGISTER_LIGHT+10));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart3)), module, RandLoops::REGISTER_LIGHT+11));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart, yLgStart4)), module, RandLoops::REGISTER_LIGHT+12));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + xLgShift, yLgStart4)), module, RandLoops::REGISTER_LIGHT+13));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart4)), module, RandLoops::REGISTER_LIGHT+14));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart4)), module, RandLoops::REGISTER_LIGHT+15));
		
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRst, yRst)), module, RandLoops::RST_INPUT));
		
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yOut)), module, RandLoops::OUT_OUTPUT));

	}

	
	void appendContextMenu(Menu* menu) override {
		RandLoops* module = dynamic_cast<RandLoops*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Buffered Del/Add", "", &module->bufferedAddDel));
		menu->addChild(createBoolPtrMenuItem("Buffered Random", "", &module->bufferedRandom));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Out Reference", (module->resolutionName[module->bitResolution]), [=](Menu * menu) {
			menu->addChild(createMenuItem("8 bit", "", [=]() {module->bitResolution = BIT_8;}));
			menu->addChild(createMenuItem("16 bit", "", [=]() {module->bitResolution = BIT_16;}));
		}));

		menu->addChild(createSubmenuItem("Voltage progression", (module->progressionName[module->progression]), [=](Menu * menu) {
			menu->addChild(createMenuItem("2x (standard)", "", [=]() {module->progression = STD2x_PROGRESSION;}));
			menu->addChild(createMenuItem("1.3x", "", [=]() {module->progression = P_1_3_PROGRESSION;}));
			menu->addChild(createMenuItem("Fibonacci", "", [=]() {module->progression = FIBONACCI_PROGRESSION;}));
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("TRIG MODE", "", &module->trigMode));

		menu->addChild(createMenuLabel("Output type"));
		if (module->trigMode) {
			struct OutTypeItem : MenuItem {
				RandLoops* module;
				int outType;
				void onAction(const event::Action& e) override {
					module->outType = outType;
				}
			};

			std::string OutTypeNames[3] = {"Trig", "Gate", "Clock Width"};
			for (int i = 0; i < 3; i++) {
				OutTypeItem* outTypeItem = createMenuItem<OutTypeItem>(OutTypeNames[i]);
				outTypeItem->rightText = CHECKMARK(module->outType == i);
				outTypeItem->module = module;
				outTypeItem->outType = i;
				menu->addChild(outTypeItem);
			}
		} else {
			menu->addChild(createMenuLabel("Trig"));
			menu->addChild(createMenuLabel("Gate"));
			menu->addChild(createMenuLabel("Clock Width"));
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("1st clock after reset:"));
		menu->addChild(createBoolPtrMenuItem("Don't advance", "", &module->dontAdvanceSetting));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Load Sequence", "", [=]() {module->menuLoadSequence();}));
		menu->addChild(createMenuItem("Save Sequence", "", [=]() {module->menuSaveSequence();}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Hints", "", [=](Menu * menu) {
			menu->addChild(createMenuLabel("Store Programs with double-click"));
		}));
	}
	

};

Model* modelRandLoops = createModel<RandLoops, RandLoopsWidget>("RandLoops");