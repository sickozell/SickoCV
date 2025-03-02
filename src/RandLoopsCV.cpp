#define STD2x_PROGRESSION 0
#define GOLDEN_PROGRESSION 1 // 1,6180339887x
#define P_1_3_PROGRESSION 2 // 1.3x
#define FIBONACCI_PROGRESSION 3

#define BIT_8 0
#define BIT_16 1

#include "plugin.hpp"

using namespace std;

struct RandLoopsCV : Module {
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
		ENUMS(OUT_OUTPUT, 8),
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
	float out[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	//int recStep[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	float controlValue = 0;

	//float outSum = 0;

	int polyChans = 0;

	bool dontAdvance[8] = {false, false, false, false, false, false, false, false};
	bool dontAdvanceSetting = true;

	/*float tableVolts[2][16] = {
		{0.039 , 0.078, 0.157, 0.314, 0.627, 1.255, 2.51, 5.02, 0, 0, 0, 0, 0, 0, 0, 0},
		{0.00015 , 0.00031, 0.00061, 0.00122, 0.00244, 0.00488, 0.00977, 0.01953, 0.03906, 0.07813, 0.15625, 0.3125, 0.062501, 1.25002, 2.50004, 5.00008}
	};*/

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

	bool shiftRegister[8][16] = {{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}
							};

	bool saveRegister[8][16] = {{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
								{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}
							};

	bool tempRegister[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool tempSaveRegister[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	float probCtrl = 0;
	float probCtrlRnd = 0;
	float probRegister = 0;

	bool incomingRegister = false;

	int startingStep[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
	int bitResolution = BIT_8;
	int bitRes[2] = {8, 16};

	std::string resolutionName[2] = {"8 bit", "16 bit"};
	std::string progressionName[4] = {"2x (std)", "Golden Ratio", "1.3x", "Fibonacci"};

	RandLoopsCV() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		for (int i = 0; i < 8; i++) {
			configInput(CLK_INPUT+i, "Clock #" + to_string(i+1));
			configInput(RST_INPUT+i, "Reset #" + to_string(i+1));
			//configParam(CTRL_PARAM+i, 0.f, 1.f, 1.f, "Control #" + to_string(i+1));
			configParam(CTRL_PARAM+i, -1.f, 1.f, 0.f, "Control #" + to_string(i+1));
			configInput(CTRL_INPUT+i, "CV Control #" + to_string(i+1));
			configParam(LENGTH_PARAM+i, 1.f, 16.f, 8.f, "Length #" + to_string(i+1));
			paramQuantities[LENGTH_PARAM+i]->snapEnabled = true;
			configParam(SCALE_PARAM+i, 0.f, 1.f, 1.f, "Scale #" + to_string(i+1), "%", 0, 100);
			configParam(OFFSET_PARAM+i, -10.f, 10.f, 0.f, "Offset #" + to_string(i+1));
			configOutput(OUT_OUTPUT+i, "OUT #" + to_string(i+1));
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

		for (int t = 0; t < 8; t++)
			saveSequence(t);

		for (int t = 0; t < 8; t++) {
			json_t *track_json_array = json_array();
			for (int tempStep = 0; tempStep < 16; tempStep++) {
				json_array_append_new(track_json_array, json_boolean(saveRegister[t][tempStep]));
			}
			json_object_set_new(rootJ, ("sr"+to_string(t)).c_str(), track_json_array);	
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
			if (progression < 0 && progression > 3)
				progression = STD2x_PROGRESSION;
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
				json_t *track_json_array = json_object_get(rootJ, ("sr"+to_string(t)).c_str());
				size_t tempSeq;
				json_t *json_value;
				if (track_json_array) {
					json_array_foreach(track_json_array, tempSeq, json_value) {
						shiftRegister[t][tempSeq] = json_boolean_value(json_value);
					}
				}
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

	void inline debugResettt(int t) {
		if (t == 0) {
			DEBUG ("%i%i%i%i%i%i%i%i%i%i%i%i%i%i%i%i RESET %i", shiftRegister[t][0], shiftRegister[t][1], shiftRegister[t][2], shiftRegister[t][3],
					shiftRegister[t][4], shiftRegister[t][5], shiftRegister[t][6], shiftRegister[t][7], shiftRegister[t][8], shiftRegister[t][9],
					shiftRegister[t][10], shiftRegister[t][11], shiftRegister[t][12], shiftRegister[t][13], shiftRegister[t][14], shiftRegister[t][15],
					startingStep[t]);
		}
	}
	*/

	void inline saveSequence(int t) {
		int cursor = 0;
		for (int i = startingStep[t]; i < int(params[LENGTH_PARAM+t].getValue()); i++) {
			tempSaveRegister[cursor] = shiftRegister[t][i];
			cursor++;
		}

		for (int i = 0; i < startingStep[t]; i++) {
			tempSaveRegister[cursor] = shiftRegister[t][i];
			cursor++;
		}

		int fillCursor = 0;
		for (int i = cursor; i < 16; i++) {
			tempSaveRegister[i] = tempSaveRegister[fillCursor];
			fillCursor++;
			if (fillCursor >= int(params[LENGTH_PARAM+t].getValue()))
				fillCursor = 0;
		}

		for (int i = 0; i < 16; i++)
			saveRegister[t][i] = tempSaveRegister[i];

	}

	void inline resetSequence(int t) {
		int cursor = 0;
		for (int i = startingStep[t]; i < int(params[LENGTH_PARAM+t].getValue()); i++) {
			tempRegister[cursor] = shiftRegister[t][i];
			cursor++;
		}

		for (int i = 0; i < startingStep[t]; i++) {
			tempRegister[cursor] = shiftRegister[t][i];
			cursor++;
		}

		int fillCursor = 0;
		for (int i = cursor; i < 16; i++) {
			tempRegister[i] = tempRegister[fillCursor];
			fillCursor++;
			if (fillCursor >= int(params[LENGTH_PARAM+t].getValue()))
				fillCursor = 0;
		}

		for (int i = 0; i < 16; i++)
			shiftRegister[t][i] = tempRegister[i];
	}

	void inline resetCheck(int t) {
		if (rstValue[t] >= 1.f && prevRstValue[t] < 1.f) {
			
			resetSequence(t);

			//debugResettt(t);

			startingStep[t] = 0;

			if (dontAdvanceSetting)
				dontAdvance[t] = true;
		
			calcVoltage(t);

		}
		prevRstValue[t] = rstValue[t];
	}

	void inline calcVoltage(int t) {
		volt[t] = 0;
		for (int i=0; i < bitRes[bitResolution]; i++) {
			if (shiftRegister[t][i])
				volt[t] += tableVolts[progression][bitResolution][i];
		}
	}

	void clearAll() {
		for (int t = 0; t < 8; t++) {
			for (int step = 0; step < 16; step++)
				shiftRegister[t][step] = false;
			calcVoltage(t);
		}
	}
	
	void process(const ProcessArgs& args) override {
		
		//outSum = 0.f;

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
							incomingRegister = true;
						else
							incomingRegister = false;
						//DEBUG ("BIT CHANGED");

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

				} else {
					dontAdvance[t] = false;
				}

			}
			prevClock[t] = clock[t];

			out[t] = (volt[t] * params[SCALE_PARAM+t].getValue()) + params[OFFSET_PARAM+t].getValue();

			if (out[t] > 10.f)
				out[t] = 10.f;
			else if (out[t] < -10.f)
				out[t] = -10.f;

			outputs[OUT_OUTPUT+t].setVoltage(out[t]);

		}
		
		if (polyChans == 0) {
			outputs[OUT_OUTPUT+7].setChannels(1);
		} else {
			for (int c = 0; c < polyChans + 1; c++)
				outputs[OUT_OUTPUT+7].setVoltage(out[c], c);
			outputs[OUT_OUTPUT+7].setChannels(polyChans+1);
		}

	}
};

struct RandLoopsCVWidget : ModuleWidget {
	RandLoopsCVWidget(RandLoopsCV* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RandLoopsCV.svg")));

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

		const float xOut = 74.5f;


		for (int track = 0; track < 8; track++) {

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xClk, yStart+(track*yDelta))), module, RandLoopsCV::CLK_INPUT+track));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRst, yStart+(track*yDelta))), module, RandLoopsCV::RST_INPUT+track));

			addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xCtrlKn, yStart+(track*yDelta))), module, RandLoopsCV::CTRL_PARAM+track));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCtrlIn, yStart+(track*yDelta))), module, RandLoopsCV::CTRL_INPUT+track));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xLength, yStart+(track*yDelta))), module, RandLoopsCV::LENGTH_PARAM+track));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xScale, yStart+(track*yDelta))), module, RandLoopsCV::SCALE_PARAM+track));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xOfs, yStart+(track*yDelta))), module, RandLoopsCV::OFFSET_PARAM+track));

			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yStart+(track*yDelta))), module, RandLoopsCV::OUT_OUTPUT+track));


		}
	}

	void appendContextMenu(Menu* menu) override {
		RandLoopsCV* module = dynamic_cast<RandLoopsCV*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Out Reference", (module->resolutionName[module->bitResolution]), [=](Menu * menu) {
			menu->addChild(createMenuItem("8 bit", "", [=]() {module->bitResolution = BIT_8;}));
			menu->addChild(createMenuItem("16 bit", "", [=]() {module->bitResolution = BIT_16;}));
		}));

		menu->addChild(createSubmenuItem("Voltage progression", (module->progressionName[module->progression]), [=](Menu * menu) {
			menu->addChild(createMenuItem("2x (standard)", "", [=]() {module->progression = STD2x_PROGRESSION;}));
			menu->addChild(createMenuItem("Golden Ratio (1.618033x)", "", [=]() {module->progression = GOLDEN_PROGRESSION;}));
			menu->addChild(createMenuItem("1.3x", "", [=]() {module->progression = P_1_3_PROGRESSION;}));
			menu->addChild(createMenuItem("Fibonacci", "", [=]() {module->progression = FIBONACCI_PROGRESSION;}));
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
		menu->addChild(createMenuItem("Clear ALL Sequences", "", [=]() {module->clearAll();}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}
};

Model* modelRandLoopsCV = createModel<RandLoopsCV, RandLoopsCVWidget>("RandLoopsCV");