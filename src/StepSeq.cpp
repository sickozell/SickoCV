#define FORWARD 0
#define REVERSE 1
#define CLOCK_MODE 1
#define CV_MODE 0
#define POSITIVE_V 0
#define NEGATIVE_V 1
#define RUN_GATE 0
#define RUN_TRIG 1
//#define OUT_TRIG 0
//#define OUT_GATE 1
//#define OUT_CLOCK 2

#include "plugin.hpp"

//using namespace std;

struct StepSeq : Module {
	enum ParamId {
		ENUMS(STEPKNOB_PARAM, 16),
		LENGTH_PARAM,
		MODE_SWITCH,
		RST_PARAM,
		RUNBUT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CLK_INPUT,
		REV_INPUT,
		RUN_INPUT,
		RST_INPUT,
		LENGTH_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		//ENUMS(STEPBUT_LIGHT, 16),
		ENUMS(STEP_LIGHT, 16),
		RUNBUT_LIGHT,
		LIGHTS_LEN
	};

	float clkValue = 0;
	float prevClkValue = 0;

	float rstValue = 0;
	float prevRstValue = 0;

	bool direction = FORWARD;

	float out = 0;

	int step = 0;

	bool runSetting = true;
	bool prevRunSetting = false;

	float runButton = 0;
	float runTrig = 0.f;
	float prevRunTrig = 0.f;

	int range = 9;

	bool initStart = false;
	int recStep = 0;

	int revType = POSITIVE_V;
	int runType = RUN_GATE;

	//float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	//bool stepPulse = false;
	//float stepPulseTime = 0;

	int maxSteps = 16;
	int mode = 0;
	int prevMode = 1;

	int currAddr = 0;
	int prevAddr = 0;

	//int outType = OUT_TRIG;
	bool rstOnRun = true;
	bool dontAdvance = false;
	bool dontAdvanceSetting = true;

	//bool outGate = false;

	StepSeq() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(MODE_SWITCH, 0.f, 1.f, 1.f, "Mode", {"Cv", "Clock"});
		configInput(CLK_INPUT, "Clock");
		configInput(REV_INPUT, "Reverse");

		configSwitch(RUNBUT_PARAM, 0.f, 1.f, 1.f, "Run", {"OFF", "ON"});
		configInput(RUN_INPUT, "Run");
		
		configParam(RST_PARAM, 1.f,16.f, 1.f, "Reset Input");
		paramQuantities[RST_PARAM]->snapEnabled = true;
		configInput(RST_INPUT, "Reset");

		configOutput(OUT_OUTPUT, "Output");

		configParam(LENGTH_PARAM, 1.f,16.f, 16.f, "Length");
		paramQuantities[LENGTH_PARAM]->snapEnabled = true;

		configInput(LENGTH_INPUT, "Length");

		configParam(STEPKNOB_PARAM+0, 0, 1.f, 0.5f, "Knob 1");
		configParam(STEPKNOB_PARAM+1, 0, 1.f, 0.5f, "Knob 2");
		configParam(STEPKNOB_PARAM+2, 0, 1.f, 0.5f, "Knob 3");
		configParam(STEPKNOB_PARAM+3, 0, 1.f, 0.5f, "Knob 4");
		configParam(STEPKNOB_PARAM+4, 0, 1.f, 0.5f, "Knob 5");
		configParam(STEPKNOB_PARAM+5, 0, 1.f, 0.5f, "Knob 6");
		configParam(STEPKNOB_PARAM+6, 0, 1.f, 0.5f, "Knob 7");
		configParam(STEPKNOB_PARAM+7, 0, 1.f, 0.5f, "Knob 8");
		configParam(STEPKNOB_PARAM+8, 0, 1.f, 0.5f, "Knob 9");
		configParam(STEPKNOB_PARAM+9, 0, 1.f, 0.5f, "Knob 10");
		configParam(STEPKNOB_PARAM+10, 0, 1.f, 0.5f, "Knob 11");
		configParam(STEPKNOB_PARAM+11, 0, 1.f, 0.5f, "Knob 12");
		configParam(STEPKNOB_PARAM+12, 0, 1.f, 0.5f, "Knob 13");
		configParam(STEPKNOB_PARAM+13, 0, 1.f, 0.5f, "Knob 14");
		configParam(STEPKNOB_PARAM+14, 0, 1.f, 0.5f, "Knob 15");
		configParam(STEPKNOB_PARAM+15, 0, 1.f, 0.5f, "Knob 16");

	}

	void onReset(const ResetEvent &e) override {

		step = 0;

		lights[STEP_LIGHT].setBrightness(1);
		lights[STEP_LIGHT+1].setBrightness(0);
		lights[STEP_LIGHT+2].setBrightness(0);
		lights[STEP_LIGHT+3].setBrightness(0);
		lights[STEP_LIGHT+4].setBrightness(0);
		lights[STEP_LIGHT+5].setBrightness(0);
		lights[STEP_LIGHT+6].setBrightness(0);
		lights[STEP_LIGHT+7].setBrightness(0);
		lights[STEP_LIGHT+8].setBrightness(0);
		lights[STEP_LIGHT+9].setBrightness(0);
		lights[STEP_LIGHT+10].setBrightness(0);
		lights[STEP_LIGHT+11].setBrightness(0);
		lights[STEP_LIGHT+12].setBrightness(0);
		lights[STEP_LIGHT+13].setBrightness(0);
		lights[STEP_LIGHT+14].setBrightness(0);
		lights[STEP_LIGHT+15].setBrightness(0);

		Module::onReset(e);
	}

	/*
	void onSampleRateChange() override {
		oneMsTime = (APP->engine->getSampleRate()) / 1000;
	}
	*/

	json_t* dataToJson() override {
		if (initStart)
			recStep = 0;
		else
			recStep = step;

		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "runType", json_integer(runType));
		json_object_set_new(rootJ, "revType", json_integer(revType));
		//json_object_set_new(rootJ, "outType", json_integer(outType));
		json_object_set_new(rootJ, "rstOnRun", json_boolean(rstOnRun));
		json_object_set_new(rootJ, "dontAdvanceSetting", json_boolean(dontAdvanceSetting));
		json_object_set_new(rootJ, "step", json_integer(recStep));
		json_object_set_new(rootJ, "initStart", json_boolean(initStart));

		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		
		json_t* runTypeJ = json_object_get(rootJ, "runType");
		if (runTypeJ) {
			runType = json_integer_value(runTypeJ);
			if (runType < 0 || runType > 1)
				runType = 0;
		}

		json_t* revTypeJ = json_object_get(rootJ, "revType");
		if (revTypeJ) {
			revType = json_integer_value(revTypeJ);
			if (revType < 0 || revType > 1)
				revType = 0;
		}
		/*
		json_t* outTypeJ = json_object_get(rootJ, "outType");
		if (outTypeJ) {
			outType = json_integer_value(outTypeJ);
			if (outType < 0 || outType > 2)
				outType = 0;
		}
		*/

		json_t* rstOnRunJ = json_object_get(rootJ, "rstOnRun");
		if (rstOnRunJ) {
			rstOnRun = json_boolean_value(rstOnRunJ);
		}

		json_t* dontAdvanceSettingJ = json_object_get(rootJ, "dontAdvanceSetting");
		if (dontAdvanceSettingJ) {
			dontAdvanceSetting = json_boolean_value(dontAdvanceSettingJ);
		}

		json_t* stepJ = json_object_get(rootJ, "step");
		if (stepJ) {
			step = json_integer_value(stepJ);
			if (step < 0 || step > 3)
				range = 0;
			lights[STEP_LIGHT+step].setBrightness(1);

		} 
		
		json_t* initStartJ = json_object_get(rootJ, "initStart");
		if (initStartJ) {
			initStart = json_boolean_value(initStartJ);
			if (initStart)
				step = 0;
		}

	}

	
	void inline resetStep() {
		lights[STEP_LIGHT+step].setBrightness(0);
		step = int(params[RST_PARAM].getValue() - 1);
		if (mode == CLOCK_MODE && dontAdvanceSetting)
			dontAdvance = true;
	}
	
	
	void process(const ProcessArgs& args) override {

		/*
		for (int i = 0; i < 16; i++)
			lights[STEPBUT_LIGHT+i].setBrightness(params[STEPKNOB_PARAM+i].getValue());
		*/

		out = 0.f;

		mode = params[MODE_SWITCH].getValue();

		rstValue = inputs[RST_INPUT].getVoltage();
		if (mode == CLOCK_MODE && rstValue >= 1.f && prevRstValue < 1.f)
			resetStep();

		prevRstValue = rstValue;

		if (inputs[RUN_INPUT].isConnected()) {

			runTrig = inputs[RUN_INPUT].getVoltage();

			if (runType == RUN_GATE) {
				
				if (runTrig > 1) {
					runSetting = 1;
					if (!prevRunSetting && mode == CLOCK_MODE && rstOnRun)
						resetStep();
				} else {
					runSetting = 0;
				}

			} else {	// runType == RUN_TRIG

				if (runTrig > 1 && prevRunTrig <=1) {
					if (runSetting) {
						runSetting = 0;
						params[RUNBUT_PARAM].setValue(0);
					} else {
						runSetting = 1;
						params[RUNBUT_PARAM].setValue(1);
						if (!prevRunSetting && mode == CLOCK_MODE && rstOnRun)
							resetStep();
					}
				}				
			}
			prevRunSetting = runSetting;
			prevRunTrig = runTrig;
		
		} else {
			
			runSetting = params[RUNBUT_PARAM].getValue();
			if (mode == CLOCK_MODE && rstOnRun && runSetting && !prevRunSetting)
				resetStep();
		}

		prevRunSetting = runSetting;

		lights[RUNBUT_LIGHT].setBrightness(runSetting);


		if (runSetting) {

			maxSteps = params[LENGTH_PARAM].getValue();

			if (inputs[LENGTH_INPUT].isConnected()) {
				float stepsIn = inputs[LENGTH_INPUT].getVoltage();
				if (stepsIn < 0.f)
					stepsIn = 0.f;
				else if (stepsIn > 10.f)
					stepsIn = 10.f;


				// 1+int(clkValue / 10 * (maxSteps));
				int addSteps = int(stepsIn / 10 * (16 - maxSteps));

				maxSteps += addSteps;
				if (maxSteps > 16)
					maxSteps = 16;
			}
			
			if (mode == CV_MODE && prevMode == CLOCK_MODE) {
				prevClkValue = 11.f;
				prevAddr = 11.f;
			}
			prevMode = mode;

			clkValue = inputs[CLK_INPUT].getVoltage();

			switch (mode) {
				case CLOCK_MODE:

					if (clkValue >= 1.f && prevClkValue < 1.f) {

						if (revType == POSITIVE_V) {
							if (inputs[REV_INPUT].getVoltage() < 1)
								direction = FORWARD;
							else
								direction = REVERSE;
						} else {
							if (inputs[REV_INPUT].getVoltage() < -1)
								direction = REVERSE;
							else
								direction = FORWARD;
						}

						lights[STEP_LIGHT + step].setBrightness(0);

						if (direction == FORWARD) {

							if (!dontAdvance)
								step++;
							else
								dontAdvance = false;
							
							//step++;

							if (step >= maxSteps)
								step = 0;
						} else {

							if (!dontAdvance)
								step--;
							else
								dontAdvance = false;

							step--;

							if (step < 0)
								step = maxSteps - 1;
						}

						/*
						if (params[STEPKNOB_PARAM+step].getValue()) {
							stepPulse = true;
							stepPulseTime = oneMsTime;
							if (outType == OUT_GATE)
								outGate = true;
						} else {
							if (outType == OUT_GATE) {
								outGate = false;
								out = 0.f;
							}
						}
						*/
					}
					prevClkValue = clkValue;
				break;

				case CV_MODE:
					if (clkValue > 10.f)
						clkValue = 10.f;
					else if (clkValue < 0.f)
						clkValue = 0.f;

					
					if (clkValue != prevClkValue) {
						
						currAddr = 1+int(clkValue / 10 * (maxSteps));
						if (currAddr >= maxSteps)
							currAddr = maxSteps;
						if (currAddr != prevAddr) {
							lights[STEP_LIGHT+step].setBrightness(0);
							step = currAddr-1;

							/*
							if (params[STEPKNOB_PARAM+step].getValue()) {
								stepPulse = true;
								stepPulseTime = oneMsTime;
								if (outType == OUT_GATE)
									outGate = true;
							} else {
								if (outType == OUT_GATE) {
									outGate = false;
									out = 0.f;
								}
							}
							*/
							prevAddr = currAddr;
							//step = currAddr-1;
						}
					}
					prevClkValue = clkValue;
					
				break;
			}
		}
			
		/*
		if (stepPulse) {

			if ( (mode == CLOCK_MODE && outType == OUT_TRIG) || (mode == CV_MODE && (outType == OUT_TRIG || outType == OUT_CLOCK) ) ) {
				stepPulseTime--;
				if (stepPulseTime < 0) {
					stepPulse = false;
					out = 0.f;
				} else {
					out = 10.f;
				}
			} else if (mode == CLOCK_MODE && outType == OUT_CLOCK) {
				out = inputs[CLK_INPUT].getVoltage();
				if (out < 1.f) {
					out = 0.f;
					stepPulse = false;
				}
			} else if (outType == OUT_GATE) {
				if (outGate)
					out = 10.f;
				else
					out = 0.f;
			}
		}
		outputs[OUT_OUTPUT].setVoltage(out);
		
		lights[STEP_LIGHT+step].setBrightness(1);
		*/
		out = params[STEPKNOB_PARAM+step].getValue();

		switch (range) {
			//case 0:
			//break;

			case 1:
				out *= 2;
			break;

			case 2:
				out *= 3;
			break;

			case 3:
				out *= 5;
			break;

			case 4:
				out *= 10;
			break;

			case 5:
				out = (out * 2) - 1;
			break;

			case 6:
				out = (out * 4) - 2;
			break;

			case 7:
				out = (out * 6) - 3;
			break;

			case 8:
				out = (out * 10) - 5;
			break;

			case 9:
				out = (out * 20) - 10;
			break;
		}

		outputs[OUT_OUTPUT].setVoltage(out);

		lights[STEP_LIGHT+step].setBrightness(1);

	}
};

struct StepSeqWidget : ModuleWidget {
	StepSeqWidget(StepSeq* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/StepSeq.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float xLeft = 7;

		const float yCvSw = 19;
		const float yTrig = 33;

		const float yRunBut = 48.5;
		const float yRunIn = 57;

		const float yRev = 74;

		const float yRstKn = 90.9;
		const float yRst = 100;

		const float yOut = 117.5;

		const float xLength = 20.5;
		const float yLength = 19.6;

		const float xStepsIn = 29.5;
		const float yStepsIn = 24;

		const float xInL = 19.3;
		const float xInR = xInL+9;
		const float xLightL = xInL+3;
		const float xLightR = xInR+3;

		const float ys = 34;
		const float yShift = 11;

		const float yShiftBlock = 3;
		const float yShiftBlock2 = 3.5;

		const float yLightShift = 4.7;

		addParam(createParamCentered<CKSS>(mm2px(Vec(xLeft, yCvSw)), module, StepSeq::MODE_SWITCH));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yTrig)), module, StepSeq::CLK_INPUT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xLeft, yRstKn)), module, StepSeq::RST_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yRst)), module, StepSeq::RST_INPUT));

		addParam(createLightParamCentered<VCVLightBezelLatch<BlueLight>>(mm2px(Vec(xLeft, yRunBut)), module, StepSeq::RUNBUT_PARAM, StepSeq::RUNBUT_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yRunIn)), module, StepSeq::RUN_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yRev)), module, StepSeq::REV_INPUT));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xLeft, yOut)), module, StepSeq::OUT_OUTPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xStepsIn, yStepsIn)), module, StepSeq::LENGTH_INPUT));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xLength, yLength)), module, StepSeq::LENGTH_PARAM));

		for (int i = 0; i < 4; i++) {

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xInL, ys+(i*yShift))), module, StepSeq::STEPKNOB_PARAM+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xInR, ys+(i*yShift)+yShiftBlock2)), module, StepSeq::STEPKNOB_PARAM+i+8));

			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xLightL, ys+(i*yShift)-yLightShift)), module, StepSeq::STEP_LIGHT+i));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xLightR, ys+(i*yShift)+yShiftBlock2-yLightShift)), module, StepSeq::STEP_LIGHT+i+8));
		
		}

		for (int i = 4; i < 8; i++) {

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xInL, ys+(i*yShift)+yShiftBlock)), module, StepSeq::STEPKNOB_PARAM+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xInR, ys+(i*yShift)+yShiftBlock+yShiftBlock2)), module, StepSeq::STEPKNOB_PARAM+i+8));

			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xLightL, ys+(i*yShift)+yShiftBlock-yLightShift)), module, StepSeq::STEP_LIGHT+i));
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xLightR, ys+(i*yShift)+yShiftBlock+yShiftBlock2-yLightShift)), module, StepSeq::STEP_LIGHT+i+8));
		}
		
	}

	void appendContextMenu(Menu* menu) override {
		StepSeq* module = dynamic_cast<StepSeq*>(this->module);

		
		struct RangeItem : MenuItem {
			StepSeq* module;
			int range;
			void onAction(const event::Action& e) override {
				module->range = range;
			}
		};

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Knobs Range"));
		std::string rangeNames[10] = {"0/1v", "0/2v", "0/3v", "0/5v", "0/10v", "-1/+1v", "-2/+2v", "-3/+3v", "-5/+5v", "-10/+10v"};
		for (int i = 0; i < 10; i++) {
			RangeItem* rangeItem = createMenuItem<RangeItem>(rangeNames[i]);
			rangeItem->rightText = CHECKMARK(module->range == i);
			rangeItem->module = module;
			rangeItem->range = i;
			menu->addChild(rangeItem);
		}
		
		struct RunTypeItem : MenuItem {
			StepSeq* module;
			int runType;
			void onAction(const event::Action& e) override {
				module->runType = runType;
			}
		};

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Run Input"));
		std::string RunTypeNames[2] = {"Gate", "Trig"};
		for (int i = 0; i < 2; i++) {
			RunTypeItem* runTypeItem = createMenuItem<RunTypeItem>(RunTypeNames[i]);
			runTypeItem->rightText = CHECKMARK(module->runType == i);
			runTypeItem->module = module;
			runTypeItem->runType = i;
			menu->addChild(runTypeItem);
		}

		struct RevTypeItem : MenuItem {
			StepSeq* module;
			int revType;
			void onAction(const event::Action& e) override {
				module->revType = revType;
			}
		};

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Reverse Input Voltage"));
		std::string RevTypeNames[2] = {"Positive", "Negative"};
		for (int i = 0; i < 2; i++) {
			RevTypeItem* revTypeItem = createMenuItem<RevTypeItem>(RevTypeNames[i]);
			revTypeItem->rightText = CHECKMARK(module->revType == i);
			revTypeItem->module = module;
			revTypeItem->revType = i;
			menu->addChild(revTypeItem);
		}

		/*
		struct OutTypeItem : MenuItem {
			StepSeq* module;
			int outType;
			void onAction(const event::Action& e) override {
				module->outType = outType;
			}
		};

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Output type"));
		std::string OutTypeNames[3] = {"Trig", "Gate", "Clock Width"};
		for (int i = 0; i < 3; i++) {
			OutTypeItem* outTypeItem = createMenuItem<OutTypeItem>(OutTypeNames[i]);
			outTypeItem->rightText = CHECKMARK(module->outType == i);
			outTypeItem->module = module;
			outTypeItem->outType = i;
			menu->addChild(outTypeItem);
		}
		*/

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Reset on Run", "", &module->rstOnRun));
		
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("1st clock after reset:"));
		menu->addChild(createBoolPtrMenuItem("Don't advance", "", &module->dontAdvanceSetting));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}

};

Model* modelStepSeq = createModel<StepSeq, StepSeqWidget>("StepSeq");