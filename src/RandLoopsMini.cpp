#define STD2x_PROGRESSION 0
#define GOLDEN_PROGRESSION 1 // 1,6180339887x
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

struct RandLoopsMini : Module {
	enum ParamId {
		CTRL_PARAM,
		LENGTH_PARAM,
		SCALE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CTRL_INPUT,
		CLK_INPUT,
		RST_INPUT,
		CLEAR_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		TRIG_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	bool initStart = false;

	float clock = 0;
	float prevClock = 0;

	float rstValue = 0;
	float prevRstValue = 0;

	float volt = 0;
	float cvOut = 0;
	float trigOut = 0;

	float controlValue = 0;

	bool dontAdvance = false;
	bool dontAdvanceSetting = true;

	int progression = STD2x_PROGRESSION;

	const float tableVolts[4][2][16] = {
		{
			{0.03922, 0.07844, 0.15688, 0.31376, 0.62752, 1.25504, 2.51008, 5.02016, 0, 0, 0, 0, 0, 0, 0, 0},
			{0.00015259, 0.000305181, 0.000610361, 0.001220722, 0.002441445, 0.00488289, 0.009765779, 0.019531558, 0.039063117, 0.078126234, 0.156252467, 0.312504934, 0.625009869, 1.250019738, 2.500039475, 5.00007895}
		},
		{
			{0.13441742, 0.217491954, 0.351909374, 0.569401328, 0.921310703, 1.490712031, 2.412022733, 3.902734764, 0, 0, 0, 0, 0, 0, 0, 0},
			{0.0028017, 0.004533246, 0.007334946, 0.011868192, 0.019203137, 0.031071329, 0.050274467, 0.081345796, 0.131620262, 0.212966058, 0.34458632, 0.557552378, 0.902138699, 1.459691077, 2.361829775, 3.821520852}
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

	bool shiftRegister[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	bool saveRegister[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	bool tempRegister[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool tempSaveRegister[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	float probCtrl = 0;
	float probCtrlRnd = 0;
	float probRegister = 0;

	bool incomingRegister = false;

	int startingStep = 0;

	int bitResolution = BIT_8;
	int bitRes[2] = {8, 16};

	float clrValue = 0;
	float prevClrValue = 0;
	
	bool pulse = false;
	float pulseTime = 0;

	int tableLength[8] = {1, 2, 3, 4, 5, 7, 11, 15};

	int outType = OUT_TRIG;

	float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	bool stepPulse = false;
	float stepPulseTime = 0;
	bool outGate = false;

	RandLoopsMini() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(CTRL_PARAM, -1, 1.f, 0.f, "Control");
		configInput(CTRL_INPUT, "Ctrl CV");

		configParam(LENGTH_PARAM, 1.f, 16.f, 8.f, "Length");
		paramQuantities[LENGTH_PARAM]->snapEnabled = true;

		configParam(SCALE_PARAM, 0.f, 1.f, 1.f, "Scale", "%", 0, 100);

		configInput(CLK_INPUT, "Clock");
		configInput(CLEAR_INPUT, "Clear");

		configInput(RST_INPUT, "Reset");
		configOutput(OUT_OUTPUT, "Out");
		configOutput(TRIG_OUTPUT, "Trig");

		calcVoltage();

	}

	void onReset(const ResetEvent &e) override {

		for (int i=0; i < 16; i++)
			shiftRegister[i] = false;

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
		json_object_set_new(rootJ, "outType", json_integer(outType));
		
		sequence_to_saveRegister();

		json_t *track_json_array = json_array();
		for (int tempStep = 0; tempStep < 16; tempStep++) {
			json_array_append_new(track_json_array, json_boolean(saveRegister[tempStep]));
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
			if (progression < 0 && progression > 3)
				progression = STD2x_PROGRESSION;
		}

		json_t* dontAdvanceSettingJ = json_object_get(rootJ, "dontAdvanceSetting");
		if (dontAdvanceSettingJ) {
			dontAdvanceSetting = json_boolean_value(dontAdvanceSettingJ);
		}

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
						shiftRegister[tempSeq] = json_boolean_value(json_value);
					}
				}
		}
	}

/*
	void inline saveSequence() {
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
*/

	// ------------------------ LOAD / SAVE   SINGLE SEQUENCE

	json_t *sequenceToJson() {

		sequence_to_saveRegister();

		json_t *rootJ = json_object();
		
		json_t *wSeq_json_array = json_array();
		for (int st = 0; st < 16; st++) {
			json_array_append_new(wSeq_json_array, json_integer(saveRegister[st]));
		}
		json_object_set_new(rootJ, "sr", wSeq_json_array);	
	
		json_object_set_new(rootJ, "length", json_integer(int(params[LENGTH_PARAM].getValue())));
		json_object_set_new(rootJ, "reset", json_real(params[SCALE_PARAM].getValue()));
		json_object_set_new(rootJ, "ctrl", json_real(params[CTRL_PARAM].getValue()));
		json_object_set_new(rootJ, "offset", json_real(0));

		return rootJ;
	}

	void sequenceFromJson(json_t *rootJ) {

		json_t *wSeq_json_array = json_object_get(rootJ, "sr");
		size_t st;
		json_t *wSeq_json_value;
		if (wSeq_json_array) {
			json_array_foreach(wSeq_json_array, st, wSeq_json_value) {
				shiftRegister[st] = json_integer_value(wSeq_json_value);
			}
		}
		startingStep = 0;

		json_t* lengthJ = json_object_get(rootJ, "length");
		if (lengthJ) {
			int wSteps;
			if (json_integer_value(lengthJ) < 1 || json_integer_value(lengthJ) > 16)
				wSteps = 16;				
			else
				wSteps = json_integer_value(lengthJ);

			params[LENGTH_PARAM].setValue(wSteps);
		}

		json_t* scaleJ = json_object_get(rootJ, "reset");
		if (scaleJ) {
			float wScale;
			if (json_real_value(scaleJ) < 0 || json_real_value(scaleJ) > 1)
				wScale = 1;				
			else
				wScale = json_real_value(scaleJ);

			params[SCALE_PARAM].setValue(wScale);
		}

		json_t* ctrlJ = json_object_get(rootJ, "ctrl");
		if (ctrlJ) {
			float wCtrl;
			if (json_real_value(ctrlJ) < -1 || json_real_value(ctrlJ) > 1)
				wCtrl = 0;				
			else
				wCtrl = json_real_value(ctrlJ);

			params[CTRL_PARAM].setValue(wCtrl);
		}

	}

	void menuLoadSequence() {
		static const char FILE_FILTERS[] = "trigSeq sequence (.tss):tss,TSS";
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


	void inline sequence_to_saveRegister() {

		// this rebuilds the sequence, shifting startingStep to position 0,
		// and filling exceeding length steps with the same sequence as it would run for some time.

		int cursor = startingStep;
		int wSteps = int(params[LENGTH_PARAM].getValue());
		//for (int i = 0; i <= wSteps; i++) {
		for (int i = 0; i < wSteps; i++) {
			tempSaveRegister[i] = shiftRegister[cursor];
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
			saveRegister[i] = tempSaveRegister[i];

	}

// ----------------------------------------------
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

			startingStep = 0;

			if (dontAdvanceSetting)
				dontAdvance = true;

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

	void inline clearAll() {
		for (int step = 0; step < 16; step++)
			shiftRegister[step] = false;

		calcVoltage();
	}
	
	void process(const ProcessArgs& args) override {

		// -------------------------------- clear sequence

		clrValue = inputs[CLEAR_INPUT].getVoltage();
		if (clrValue >= 1.f && prevClrValue < 1.f)
			clearAll();

		prevClrValue = clrValue;

		// -------------------------------- reset check

		rstValue = inputs[RST_INPUT].getVoltage();
				
		resetCheck();

		// -------------------------------- clock trigger

		clock = inputs[CLK_INPUT].getVoltage();
		if (clock >= 1.f && prevClock < 1.f) {

			if (!dontAdvance) {

				startingStep++;

				if (startingStep >= int(params[LENGTH_PARAM].getValue()))
					startingStep = 0;

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
						incomingRegister = true;
					else
						incomingRegister = false;

				} else {

					incomingRegister = shiftRegister[int(params[LENGTH_PARAM].getValue())-1];
					if (controlValue < 0)
						incomingRegister = !incomingRegister;
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

				calcVoltage();

				if (incomingRegister) {
					stepPulse = true;
					stepPulseTime = oneMsTime;
					if (outType == OUT_GATE)
						outGate = true;
				} else {
					if (outType == OUT_GATE) {
						outGate = false;
						trigOut = 0.f;
					}
				}

			} else {
				dontAdvance = false;
			}

		}
		prevClock = clock;

		if (stepPulse) {

			if (outType == OUT_TRIG) {
				stepPulseTime--;
				if (stepPulseTime < 0) {
					stepPulse = false;
					trigOut = 0.f;
				} else {
					trigOut = 10.f;
				}
			} else if (outType == OUT_CLOCK) {
				trigOut = clock;
				if (trigOut < 1.f) {
					trigOut = 0.f;
					stepPulse = false;
				}
			} else if (outType == OUT_GATE) {
				if (outGate)
					trigOut = 10.f;
				else
					trigOut = 0.f;
			}
		}

		cvOut = volt * params[SCALE_PARAM].getValue();

		if (cvOut > 10.f)
			cvOut = 10.f;
		else if (cvOut < -10.f)
			cvOut = -10.f;

		outputs[OUT_OUTPUT].setVoltage(cvOut);

		outputs[TRIG_OUTPUT].setVoltage(trigOut);

	}
};

struct RandLoopsMiniWidget : ModuleWidget {
	RandLoopsMiniWidget(RandLoopsMini* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RandLoopsMini.svg")));

		//addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		//addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float xCenter = 5.08f;

		const float yCtrl = 17.5;
		const float yCtrlCV = 26;
		const float yLength = 39;
		const float yScale = 52;

		const float yClr = 65;
		const float yRst = 77;
		const float yClk = 89.5;
		const float yOut = 104.1;
		const float yOutTrg = 117.5;

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, yCtrl)), module, RandLoopsMini::CTRL_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yCtrlCV)), module, RandLoopsMini::CTRL_INPUT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, yLength)), module, RandLoopsMini::LENGTH_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, yScale)), module, RandLoopsMini::SCALE_PARAM));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yClk)), module, RandLoopsMini::CLK_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yClr)), module, RandLoopsMini::CLEAR_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yRst)), module, RandLoopsMini::RST_INPUT));
		
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xCenter, yOutTrg)), module, RandLoopsMini::TRIG_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xCenter, yOut)), module, RandLoopsMini::OUT_OUTPUT));

	}

	
	void appendContextMenu(Menu* menu) override {
		RandLoopsMini* module = dynamic_cast<RandLoopsMini*>(this->module);

		menu->addChild(new MenuSeparator());

		struct BitResTypeItem : MenuItem {
			RandLoopsMini* module;
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
			RandLoopsMini* module;
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
			RandLoopsMini* module;
			int outType;
			void onAction(const event::Action& e) override {
				module->outType = outType;
			}
		};
		std::string OutTypeNames[3] = {"Trig", "Gate", "Clock Width"};

		menu->addChild(createSubmenuItem("Trig Output type", (OutTypeNames[module->outType]), [=](Menu * menu) {
			for (int i = 0; i < 3; i++) {
				OutTypeItem* outTypeItem = createMenuItem<OutTypeItem>(OutTypeNames[i]);
				outTypeItem->rightText = CHECKMARK(module->outType == i);
				outTypeItem->module = module;
				outTypeItem->outType = i;
				menu->addChild(outTypeItem);
			}
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("1st clock after reset:"));
		menu->addChild(createBoolPtrMenuItem("Don't advance", "", &module->dontAdvanceSetting));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("DISK operations", "", [=](Menu * menu) {
			menu->addChild(createMenuItem("Import trigSeq seq.", "", [=]() {module->menuLoadSequence();}));
			menu->addChild(createMenuItem("Export trigSeq seq.", "", [=]() {module->menuSaveSequence();}));
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}
	
};

Model* modelRandLoopsMini = createModel<RandLoopsMini, RandLoopsMiniWidget>("RandLoopsMini");