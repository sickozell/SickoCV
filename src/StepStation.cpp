#include "StepStation_def.hpp"

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

struct SickoStepStation : Module {

	bool alive;

	virtual int getInputBase() const { return -1; }
	virtual int getOutputBase() const { return -1; }
	virtual int getParamBase() const { return -1; }

	//const std::string userInNames[KNOB_SHIFT] = {"Reverse", "Mode", "Length", "Reset Step#", "Run", "Retrig", "Out scale", "Swing"};
	//const std::string userKnobNames[KNOB_NR] = {"Reset Step#", "Mode", "Retrig Prob.", "Out scale", "Swing", "Attenuator", "Attenuverter"};
	const std::string userInNames[KNOB_SHIFT] = {"Change", "Change Prob.", "Length", "Mode", "Out scale", "Reset Step#", "Retrig", "Reverse", "Run", "Swing"};
	const std::string userKnobNames[KNOB_NR] = {"Change prob.", "Mode", "Out scale", "Reset Step#", "Retrig Prob.", "Swing", "Attenuator", "Attenuverter"};

	const std::string sampleDelayNames[7] = {"No Delay", "1 sample", "2 samples", "3 samples", "4 samples", "5 samples", "Default"};
	int sampleDelay[ALLTRACKS] = {6, 6, 6, 6, 6, 6, 6, 6, 0};

	const std::string rangeNames[11] = {"0/1v", "0/2v", "0/3v", "0/5v", "0/10v", "-1/+1v", "-2/+2v", "-3/+3v", "-5/+5v", "-10/+10v", "Default"};
	
	bool cvClockIn = false;
	bool cvClockOut = false;

	int userTable[MAXTRACKS][4] = {
							{IN_RUN, KNOB_CHANGEPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_CHANGEPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_CHANGEPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_CHANGEPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_CHANGEPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_CHANGEPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_CHANGEPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_CHANGEPROB, IN_REV, KNOB_MODE}
						};

	int userInputs[MAXTRACKS][MAXUSER][2] = {
//								 IN_CHANG IN_CHNGPR IN_LENG  IN_MODE  IN_OUTSC IN_RSTST IN_RETRG IN_REV   IN_RUN   IN_SWING KN_CHPR KN_MODE  KN_OUTSC KN_RSTST KN_RETRG KN_SWING ATTTEN  ATTENVERT 
								{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {1, 0}, {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0} },
								{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {1, 0}, {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0} },
								{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {1, 0}, {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0} },
								{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {1, 0}, {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0} },
								{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {1, 0}, {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0} },
								{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {1, 0}, {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0} },
								{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {1, 0}, {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0} },
								{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {1, 0}, {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0} }
							};

	void appendInputMenu(Menu *menu, engine::Port::Type type, int portId){

		menu->addChild(new MenuSeparator());

		int inputBase = getInputBase();

		if (inputBase >= 0) {
			
			int inputIndex = portId - inputBase;

			int t = inputIndex;
			if (t >= MAXTRACKS)
				t -= MAXTRACKS; 

			int column;

			if (inputIndex < MAXTRACKS) {
				column = 0;
			} else {
				column = 2;
			}

			SickoStepStation* module = this;

			struct UserInputItem : MenuItem {
				SickoStepStation* module;
				int userPortType;
				int tr;
				int inputIndex;
				int column;

				UserInputItem(SickoStepStation* m, int knobType, int track, int index, int col) {
					module = m;
					userPortType = knobType;
					tr = track;
					inputIndex = index;
					text = module->userInNames[knobType];
					column = col;
				}

				void onAction(const event::Action& e) override {

					if (inputIndex < MAXTRACKS) {
						module->userInputs[tr][module->userTable[tr][0]][0] = 0;
						module->userInputs[tr][userPortType][0] = 1;
						module->userInputs[tr][userPortType][1] = 0;
						module->userTable[tr][0] = userPortType;	
					} else {
						module->userInputs[tr][module->userTable[tr][2]][0] = 0;
						module->userInputs[tr][userPortType][0] = 1;
						module->userInputs[tr][userPortType][1] = 8;
						module->userTable[tr][2] = userPortType;	
					}

				}
			};
			

			int otherCol = 0;
			if (column == 0)
				otherCol = 2;

			for (int i = 0; i < KNOB_SHIFT; i++) {
				auto userPortTypeItem = new UserInputItem(module, i, t, inputIndex, column);

				userPortTypeItem->rightText = CHECKMARK(module->userTable[t][column] == i);

				if (i != module->userTable[t][otherCol])
					menu->addChild(userPortTypeItem);
				else
					menu->addChild(createMenuLabel(module->userInNames[i]));
			}

		} else {
			menu->addChild(createMenuLabel("Unknown input"));
		}
	}

	void appendClockInMenu(Menu *menu, engine::Port::Type type, int portId){

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("CV clock IN", "", &cvClockIn));
			
	}

	void appendClockOutMenu(Menu *menu, engine::Port::Type type, int portId){

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("CV clock OUT", "", &cvClockOut));
		
	}

	void appendOutMenu(Menu *menu, engine::Port::Type type, int portId){

		menu->addChild(new MenuSeparator());

		int outputBase = getOutputBase();

		if (outputBase >= 0) {
			
			int t = portId - outputBase;

			SickoStepStation* module = this;

			struct SampleDelayItem : MenuItem {
				SickoStepStation* module;
				int menuValue;
				int t;

				SampleDelayItem(SickoStepStation* m, int value, int track) {
					module = m;
					menuValue = value;
					t = track;
					text = module->sampleDelayNames[value];
				}

				void onAction(const event::Action& e) override {
					module->sampleDelay[t] = menuValue;
				}
			};

			//menu->addChild(createMenuLabel("Out DELAY:"));
			menu->addChild(createSubmenuItem("Out DELAY:", (module->sampleDelayNames[module->sampleDelay[t]]), [=](Menu* menu) {
				for (int i = 0; i < 7; i++) {
					auto sampleDelayItem = new SampleDelayItem(module, i, t);
					
					if (i == 6)
						sampleDelayItem->rightText = module->sampleDelayNames[module->sampleDelay[MC]] + " " + CHECKMARK(module->sampleDelay[t] == i);
					else
						sampleDelayItem->rightText = CHECKMARK(module->sampleDelay[t] == i);
					menu->addChild(sampleDelayItem);
				}
			}));
			
		}  else {
			menu->addChild(createMenuLabel("Unknown output"));
		}
	}

	void appendParamMenu(Menu* menu, int paramId) {

		menu->addChild(new MenuSeparator());
		
		int paramBase = getParamBase();
		

		if (paramBase >= 0) {
			
			int knobIndex = paramId - paramBase;

			int t = knobIndex;
			if (t >= MAXTRACKS)
				t -= MAXTRACKS; 

			int column;

			if (knobIndex < MAXTRACKS) {
				column = 1;
			} else {
				column = 3;
			}
/*
			std::string label = "PARAM " + std::to_string(knobIndex);
			std::string label2 = "TRACK " + std::to_string(t);
			std::string label3 = "COL " + std::to_string(column);

			menu->addChild(createMenuLabel(label));
			menu->addChild(createMenuLabel(label2));
			menu->addChild(createMenuLabel(label3));
*/
			SickoStepStation* module = this;

			struct UserKnobItem : MenuItem {
				SickoStepStation* module;
				int userKnobType;
				int tr;
				int knobIndex;
				int column;

				UserKnobItem(SickoStepStation* m, int knobType, int track, int index, int col) {
					module = m;
					userKnobType = knobType;
					tr = track;
					knobIndex = index;
					text = module->userKnobNames[knobType];
					column = col;
				}

				void onAction(const event::Action& e) override {

					if (knobIndex < MAXTRACKS) {
						module->userInputs[tr][module->userTable[tr][1]][0] = 0;
						module->userInputs[tr][userKnobType+KNOB_SHIFT][0] = 1;
						module->userInputs[tr][userKnobType+KNOB_SHIFT][1] = 0;
						module->userTable[tr][1] = userKnobType+KNOB_SHIFT;	
					} else {
						module->userInputs[tr][module->userTable[tr][3]][0] = 0;
						module->userInputs[tr][userKnobType+KNOB_SHIFT][0] = 1;
						module->userInputs[tr][userKnobType+KNOB_SHIFT][1] = 8;
						module->userTable[tr][3] = userKnobType+KNOB_SHIFT;	
					}

				}
			};
			

			int otherCol = 1;
			if (column == 1)
				otherCol = 3;

			for (int i = 0; i < KNOB_NR; i++) {
				auto userKnobTypeItem = new UserKnobItem(module, i, t, knobIndex, column);

				userKnobTypeItem->rightText = CHECKMARK(module->userTable[t][column]-KNOB_SHIFT == i);

				if (i != module->userTable[t][otherCol]-KNOB_SHIFT || i > FIRST_ATN)
					menu->addChild(userKnobTypeItem);
				else
					menu->addChild(createMenuLabel(module->userKnobNames[i]));
			}

		} else {
			menu->addChild(createMenuLabel("Unknown param"));
		}

	}

};

template <class TWidget>
TWidget* createStepStationParamCentered(math::Vec pos, engine::Module* module, int paramId){
	//SickoStepStation* mod = dynamic_cast<SickoStepStation*>(module);
	//mod->alive = true;
  return createParamCentered<TWidget>(pos, module, paramId);
}

template <class TWidget>
TWidget* createStepStationInputCentered(math::Vec pos, engine::Module* module, int paramId){
	//SickoStepStation* mod = dynamic_cast<SickoStepStation*>(module);
	//mod->alive = true;
  return createInputCentered<TWidget>(pos, module, paramId);
}

template <class TWidget>
TWidget* createStepStationClockInCentered(math::Vec pos, engine::Module* module, int paramId){
	//SickoStepStation* mod = dynamic_cast<SickoStepStation*>(module);
	//mod->alive = true;
  return createInputCentered<TWidget>(pos, module, paramId);
}

template <class TWidget>
TWidget* createStepStationClockOutCentered(math::Vec pos, engine::Module* module, int paramId){
	//SickoStepStation* mod = dynamic_cast<SickoStepStation*>(module);
	//mod->alive = true;
  return createOutputCentered<TWidget>(pos, module, paramId);
}


struct SickoKnobStepStation : SickoTrimpot {
	void appendContextMenu(Menu* menu) override {
	    if (module) {
	      dynamic_cast<SickoStepStation*>(this->module)->appendParamMenu(menu, this->paramId);
	    }
	  }
};

struct SickoInputStepStation : SickoInPort {
	void appendContextMenu(Menu* menu) override {
		if (this->module)
			dynamic_cast<SickoStepStation*>(this->module)->appendInputMenu(menu, this->type, this->portId);
	}
};

struct SickoClockInStepStation : SickoInPort {
	void appendContextMenu(Menu* menu) override {
		if (this->module)
			dynamic_cast<SickoStepStation*>(this->module)->appendClockInMenu(menu, this->type, this->portId);
	}
};

struct SickoClockOutStepStation : SickoOutPort {
	void appendContextMenu(Menu* menu) override {
		if (this->module)
			dynamic_cast<SickoStepStation*>(this->module)->appendClockOutMenu(menu, this->type, this->portId);
	}
};

struct SickoOutStation : SickoOutPort {
	void appendContextMenu(Menu* menu) override {
		if (this->module)
			dynamic_cast<SickoStepStation*>(this->module)->appendOutMenu(menu, this->type, this->portId);
	}
};

struct tpDivMult : ParamQuantity {
	std::string getDisplayValueString() override {
		const std::string valueDisplay[45] = {"/256", "/128", "/64", "/48", "/32", "/24", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "x1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x24", "x32", "x48", "x64", "x128", "x256"};
		return valueDisplay[int(getValue())];
	}
};

//struct StepStation : Module {
struct StepStation : SickoStepStation {
	enum ParamId {
		BPM_KNOB_PARAM,
		INTCLOCKBUT_PARAM,
		SEQRUN_PARAM,
		RSTALLBUT_PARAM,
		PROG_PARAM,
		SET_PARAM,
		AUTO_PARAM,
		RECALL_PARAM,
		STORE_PARAM,
		ENUMS(DIVMULT_KNOB_PARAM, 8),
		ENUMS(LENGTH_PARAM, 8),
		ENUMS(USER_PARAM, 16),
		ENUMS(STEP_PARAM, 128),

		PARAMS_LEN
	};
	enum InputId {
		EXTCLOCK_INPUT,
		RUN_INPUT,
		SEQRUN_INPUT,
		RSTALL_INPUT,
		PROG_INPUT,
		RECALL_INPUT,
		ENUMS(CLK_INPUT, 8),
		ENUMS(RST_INPUT, 8),
		ENUMS(USER_INPUT, 16),
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT, 8),
		RST_OUTPUT,
		CLOCK_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(STEP_LIGHT, 128),
		RUNBUT_LIGHT,
		SEQRUN_LIGHT,
		RSTALLBUT_LIGHT,
		RECALL_LIGHT,
		STORE_LIGHT,
		SET_LIGHT,
		AUTO_LIGHT,
		ENUMS(MODE_LIGHT, 8),
		LIGHTS_LEN
	};

	int getInputBase() const override { return USER_INPUT; }
	int getOutputBase() const override { return OUT_OUTPUT; }
	int getParamBase() const override { return USER_PARAM; }

	float clkValue[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};
	float prevClkValue[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};

	float rstValue[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevRstValue[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	bool direction[MAXTRACKS] = {FORWARD, FORWARD, FORWARD, FORWARD, FORWARD, FORWARD, FORWARD, FORWARD};
	float revVolt[MAXTRACKS] = {};
	float prevRevVolt[MAXTRACKS] = {};

	float out[MAXTRACKS] = {0,0,0,0,0,0,0,0};

	int step[MAXTRACKS] = {0,0,0,0,0,0,0,0};

	bool runSetting = false;
	bool prevRunSetting = false;

	int seqRunSetting = 1;
	int prevSeqRunSetting = 0;
	float seqRunTrig = 0.f;
	float prevSeqRunTrig = 0.f;
	float seqButtRunTrig = 0.f;
	float prevSeqButtRunTrig = 0.f;

	float runButton = 0;
	float runTrig = 0.f;
	float prevRunTrig = 0.f;

	bool internalClock = false;
	bool prevInternalClock = false;
	bool externalClock = false;
	bool prevExternalClock = true;

	float buttRunTrig = 0.f;
	float prevButtRunTrig = -1.f;

	//int range = 9;

	int recStep[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};

	int runType = RUN_GATE;

	int maxSteps[MAXTRACKS] = {0,0,0,0,0,0,0,0};

	int currAddr[MAXTRACKS] = {0,0,0,0,0,0,0,0};
	int prevAddr[MAXTRACKS] = {0,0,0,0,0,0,0,0};

	bool rstClkOnRst = true;
	bool rstSeqOnProgChange = true;

	bool dontAdvance[MAXTRACKS] = {false, false, false, false, false, false, false, false};

	// ************************************************************************************
	
	#include "StepStation_init.hpp"
	
	// ************************************************************************************

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

	int workingProg = 0;

	bool instantProgChange = false;

	bool progToSet = false;
	float progSetBut = 0;
	float prevProgSetBut = 0;

	// ------- set button light

	bool setButLight = false;
	float setButLightDelta = 2 / APP->engine->getSampleRate();
	float setButLightValue = 0.f;


	// -------------------------------------------------------------------------

	int progInType = CV_TYPE;
	int lastProg = 0;

	int currentMode[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};

 	//**************************************************************
	//   

	const std::string divMultDisplay[45] = {"/ 256", "/ 128", "/ 64", "/ 48", "/ 32", "/ 24", "/ 17", "/ 16", "/ 15", "/ 14", "/ 13", "/ 12", "/ 11", "/ 10", "/ 9", "/ 8", "/ 7", "/ 6", "/ 5", "/ 4", "/ 3", "/ 2",
	 					"X 1", "X 2", "X 3", "X 4", "X 5", "X 6", "X 7", "X 8", "X 9", "X 10", "X 11", "X 12", "X 13", "X 14", "X 15", "X 16", "X 17", "X 24", "X 32", "X 48", "X 64", "X 128", "X 256"};

	const float divMult[45] = {256, 128, 64, 48, 32, 24, 17, 16, 15, 14, 13, 12, 11, 10, 9 , 8, 7, 6, 5, 4, 3, 2, 1,
							2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 24, 32, 48, 64, 128, 256};

	//**************************************************************

	// ********* C L O C K *************

	double sampleRateCoeff = (double)APP->engine->getSampleRate() * 60;

	double bpm[ALLTRACKS] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
	double extBpm = 0.1;
	double prevBpm[ALLTRACKS] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
	double prevExtBpm = -1;

	float extTrigValue[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevExtTrigValue[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	float resetBut = 0;
	float resetValue[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevResetValue[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	bool resetStart = true;
	bool resetStartExt = true;
	//bool resetDiv = true;

	double clockSample[ALLTRACKS] = {1, 1, 1, 1, 1, 1, 1, 1, 999999999999};
	double clockMaxSample[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	double maxPulseSample[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	double extClockSample = 999999999999;
	double extClockMaxSample = 0;
	double extMaxPulseSample = 0;
	
	int ppqnTable[7] = {1, 2, 4, 8, 12, 16, 24}; // 1 2 4 8 12 16 24
	int ppqn = PPQN1;
	int tempPpqn = ppqn;
	bool ppqnChange = false;

	int ppqnValue = ppqnTable[ppqn];
	int ppqnComparison = ppqnValue - 1;

	int pulseNr = 0;

	float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	//float oneMsTime = (APP->engine->getSampleRate()) / 10;	// for testing purposes
	bool resetPulse[ALLTRACKS] = {false, false, false, false, false, false, false, false, false};
	float resetPulseTime[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	bool resetOnRun = true;
	bool resetPulseOnRun = false;
	bool resetOnStop = false;
	bool resetPulseOnStop = false;

	bool extSync[ALLTRACKS] = {false, false, false, false, false, false, false, false, false};
	bool extConn[ALLTRACKS] = {false, false, false, false, false, false, false, false, false};
	bool prevExtConn[ALLTRACKS] = {true, true, true, true, true, true, true, true, true};
	bool extBeat[ALLTRACKS] = {false, false, false, false, false, false, false, false, false};

	double divClockSample[MAXTRACKS] = {1, 1, 1, 1, 1, 1, 1, 1};
	double divMaxSample[MAXTRACKS][2] = {{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}};
	int divOddCounter[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};
	bool divPulse[MAXTRACKS] = {false, false, false, false, false, false, false, false};
	float divPulseTime[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};
	int divCount[MAXTRACKS] = {1, 1, 1, 1, 1, 1, 1, 1};

	bool divSwing[MAXTRACKS] = {false, false, false, false, false, false, false, false};

//	bool cvClockIn = false;
//	bool cvClockOut = false;
	float cvClockInValue[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevCvClockInValue[ALLTRACKS] = {11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f};
	float cvClockOutValue = 0.f;

	bool clockAdv[ALLTRACKS] = {false, false, false, false, false, false, false, false, false};
	bool prevClockAdv[ALLTRACKS] = {false, false, false, false, false, false, false, false, false};

	bool edge[ALLTRACKS] = {false, false, false, false, false, false, false, false, false};
	bool prevEdge[ALLTRACKS] = {false, false, false, false, false, false, false, false, false};

	bool stepAdv[MAXTRACKS] = {false, false, false, false, false, false, false, false};
	int delayReadPos;

	bool clkBuff[MAXTRACKS][5] = {{false, false, false, false, false}, {false, false, false, false, false}, {false, false, false, false, false}, {false, false, false, false, false}, {false, false, false, false, false}, {false, false, false, false, false}, {false, false, false, false, false}, {false, false, false, false, false}};
	int clkBuffPos = 0;

	bool divControls = true;
	bool modeControls = true;

	bool alreadyChanged[MAXTRACKS] = {true, true, true, true, true, true, true, true};

	StepStation() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		
		struct RangeQuantity : ParamQuantity {
			int t = -1;  // sarà assegnato dopo la creazione
			float getDisplayValue() override {
				StepStation* module = reinterpret_cast<StepStation*>(this->module);

				if (t >= 0) {

					int tempRange = module->range[MC];

					if (module->range[t] != 10)
						tempRange = module->range[t];

					switch (tempRange) {
						case 0:	// 0/1v
							displayMultiplier = 1.f;
							displayOffset = 0.f;
						break;
						case 1:	// 0/2v
							displayMultiplier = 2.f;
							displayOffset = 0.f;
						break;
						case 2:	// 0/3v
							displayMultiplier = 3.f;
							displayOffset = 0.f;
						break;
						case 3:	// 0/5v
							displayMultiplier = 5.f;
							displayOffset = 0.f;
						break;
						case 4:	// 0/10v
							displayMultiplier = 10.f;
							displayOffset = 0.f;
						break;
						case 5:	// -1/+1v
							displayMultiplier = 2.f;
							displayOffset = -1.f;
						break;
						case 6:	// -2/+2v
							displayMultiplier = 4.f;
							displayOffset = -2.f;
						break;
						case 7:	// -3/+3v
							displayMultiplier = 6.f;
							displayOffset = -3.f;
						break;
						case 8:	// -5/+5v
							displayMultiplier = 10.f;
							displayOffset = -5.f;
						break;
						case 9:	// -10/+10v
							displayMultiplier = 20.f;
							displayOffset = -10.f;
						break;
					}
				}
				return ParamQuantity::getDisplayValue();
			}
		};

		struct UserQuantity : ParamQuantity {
			int t = -1;  // sarà assegnato dopo la creazione
			int userColumn = 1;  // 1 per default (primo knob)

			float getDisplayValue() override {
				StepStation* module = reinterpret_cast<StepStation*>(this->module);

				if (t >= 0) {
					switch (module->userTable[t][userColumn]) {
				 		case KNOB_RSTSTEP:
				 			name = "Reset Step# #" + to_string(t+1);
							unit = "";
							displayMultiplier = 15.f;
							displayOffset = 1.f;
							return int(ParamQuantity::getDisplayValue());
				 		break;

				 		case KNOB_MODE:
				 			name = "Mode #" + to_string(t+1);
							unit = "";
							displayMultiplier = MAXMODES;
							displayOffset = 0.f;
							return int(ParamQuantity::getDisplayValue());
						break;

						case KNOB_OUTSCALE:
				 			name = "Out scale #" + to_string(t+1);
							unit = "%";
							displayMultiplier = 100.f;
							displayOffset = 0.f;
							return int(ParamQuantity::getDisplayValue());
						break;

				 		case KNOB_ATN:
				 			name = "Atten. #" + to_string(t+1);
							unit = "%";
							displayMultiplier = 100.f;
							displayOffset = 0.f;
						break;

						case KNOB_RETRIGPROB:
				 			name = "Retrig Prob. #" + to_string(t+1);
							unit = "%";
							displayMultiplier = 100.f;
							displayOffset = 0.f;
						break;

						case KNOB_CHANGEPROB:
				 			name = "Change Prob. #" + to_string(t+1);
							unit = "%";
							displayMultiplier = 100.f;
							displayOffset = 0.f;
						break;

						case KNOB_ATNV:
				 			name = "Attenuv. #" + to_string(t+1);
							unit = "%";
							displayMultiplier = 200.f;
							displayOffset = -100.f;
						break;

						case KNOB_SWING:
				 			name = "Swing #" + to_string(t+1);
							unit = "%";
							displayMultiplier = 100.f;
							displayOffset = 0.f;
						break;				
				 	}
				}
				return ParamQuantity::getDisplayValue();
			}
		};

		struct UserLabel : PortInfo {
			int t = -1;  // sarà assegnato dopo la creazione
			int userColumn = 1;  // 1 per default (primo knob)

			std::string getName() override {
				StepStation* module = reinterpret_cast<StepStation*>(this->module);

				if (t >= 0) {
					switch (module->userTable[t][userColumn]) {
				 		case IN_REV:		name = "Reverse #" + to_string(t+1); break;
				 		case IN_MODE:		name = "Mode #" + to_string(t+1); break;
				 		case IN_LENGTH:		name = "Length #" + to_string(t+1); break;
						case IN_RSTSTEP:	name = "Reset Step# #" + to_string(t+1); break;
						case IN_OUTSCALE:	name = "Out scale #" + to_string(t+1); break;
						case IN_RUN:		name = "Run #" + to_string(t+1); break;
						case IN_RETRIG:		name = "Retrig #" + to_string(t+1); break;
						case IN_SWING:		name = "Swing #" + to_string(t+1); break;
						case IN_CHANGE:		name = "Change #" + to_string(t+1); break;
						case IN_CHANGEPROB:	name = "Change Prob.#" + to_string(t+1); break;
				 	}
				}
				return PortInfo::getName();
			}
		};

		configParam(BPM_KNOB_PARAM, 300.f, 3000.f, 1200.f, "Int.Clk", " bpm", 0, 0.1);
		paramQuantities[BPM_KNOB_PARAM]->snapEnabled = true;

		configInput(EXTCLOCK_INPUT,"External Clock");

		configSwitch(INTCLOCKBUT_PARAM, 0.f, 1.f, 0.f, "Int.Clock Run", {"0", "1"});
		configInput(RUN_INPUT, "Int.Clock");
		
		configSwitch(SEQRUN_PARAM, 0.f, 1.f, 0.f, "Seq. run", {"0", "1"});
		//configSwitch(SEQRUN_PARAM, 0.f, 1.f, 0.f, "Seq. run");
		configInput(SEQRUN_INPUT, "Seq.run");

		configInput(RSTALL_INPUT, "Reset");
		configSwitch(RSTALLBUT_PARAM, 0.f, 1.f, 0.f, "Rst All", {"0", "1"});
		configOutput(RST_OUTPUT, "Reset");

		configParam(PROG_PARAM, 0.f, 31.f, 0.f, "Prog");
		configInput(PROG_INPUT, "Prog");
		paramQuantities[PROG_PARAM]->snapEnabled = true;
		configSwitch(SET_PARAM, 0, 1.f, 0.f, "Set", {"0", "1"});
		configSwitch(AUTO_PARAM, 0, 1.f, 0.f, "Auto", {"OFF", "ON"});
		configSwitch(RECALL_PARAM, 0, 1.f, 0.f, "Recall", {"0", "1"});
		configSwitch(STORE_PARAM, 0, 1.f, 0.f, "Store", {"0", "1"});

		configOutput(CLOCK_OUTPUT,"Clock");

		for (int t = 0; t < MAXTRACKS; t++) {
			
			configInput(CLK_INPUT+t, ("Clock#"+to_string(t+1)).c_str());

			configParam<tpDivMult>(DIVMULT_KNOB_PARAM+t, 0.f, 44.f, 22.f, ("Mult/Div #"+to_string(t+1)).c_str());
			paramQuantities[DIVMULT_KNOB_PARAM+t]->snapEnabled = true;

			configInput(RST_INPUT+t, ("Rst.#"+to_string(t+1)).c_str());
			
			configParam(LENGTH_PARAM+t, 1.f,16.f, 16.f, ("Len.#"+to_string(t+1)).c_str());
			paramQuantities[LENGTH_PARAM+t]->snapEnabled = true;

			configInput<UserLabel>(USER_INPUT+t, ("U1 #"+to_string(t+1)).c_str());
			if (auto ul1 = dynamic_cast<UserLabel*>(inputInfos[USER_INPUT + t])) {
				ul1->t = t;
				ul1->userColumn = 0;
			}

			configParam<UserQuantity>(USER_PARAM+t, 0.f, 1.f, 0.f, ("K1 #"+to_string(t+1)).c_str());
			// Setta `t` manualmente
			if (auto uq1 = dynamic_cast<UserQuantity*>(paramQuantities[USER_PARAM + t])) {
				uq1->t = t;
				uq1->userColumn = 1;
			}

			configInput<UserLabel>(USER_INPUT+t+8, ("U2 #"+to_string(t+1)).c_str());
			if (auto ul2 = dynamic_cast<UserLabel*>(inputInfos[USER_INPUT + t + 8])) {
				ul2->t = t;
				ul2->userColumn = 2;
			}
			
			configParam<UserQuantity>(USER_PARAM+t+8, 0.f, 1.f, 0.f, ("K2 #"+to_string(t+1)).c_str());

			// Setta `t` per il secondo user knob
			if (auto uq2 = dynamic_cast<UserQuantity*>(paramQuantities[USER_PARAM + t + 8])) {
				uq2->t = t;
				uq2->userColumn = 3;
			}

			configOutput(OUT_OUTPUT+t, ("Tr.#"+to_string(t+1)).c_str());

			for (int s = 0; s < 16; s++) {
				configParam<RangeQuantity>(STEP_PARAM+(t*16)+s, 0.f, 1.f, 0.5f, ("Tr.#"+to_string(t+1)+" Step#"+to_string(s+1)).c_str());
				if (auto rq = dynamic_cast<RangeQuantity*>(paramQuantities[STEP_PARAM+(t*16)+s])) {
					rq->t = t;
					//uq2->userColumn = 3;
				}
			}



			/*	// init non funziona qui
			for (int p = 0; t < 32; t++) {
				for (int t = 0; t < MAXTRACKS; t++) {
					for (int s = 1; s < 16; s++) {
						progSeq[p][t][s] = 0.5f;
					}
					progSteps[p][t] = 16;
				}
			}
			*/
		}

	}

	void onReset(const ResetEvent &e) override {

		for (int t = 0; t < MAXTRACKS; t++) {
			step[t] = 0;
			lights[STEP_LIGHT+(t*16)].setBrightness(1);
			for (int s = 1; s < 16; s++) {
				lights[STEP_LIGHT+(t*16+s)].setBrightness(0);
				wSeq[t][s] = 0.5f;
			}
		}
		
		setButLight = false;
		setButLightValue = 0.f;


		// ----- clock
		resetStart = true;
		resetStartExt = true;
		//resetDiv = true;

		runButton = 0;
		runTrig = 0.f;
		prevRunTrig = 0.f;

		//resetOnRun = true;
		//resetPulseOnRun = false;
		//resetOnStop = false;
		//resetPulseOnStop = false;

		bpm[MC] = 0;
		clockSample[MC] = 1.0;
		extSync[MC] = false;
		extConn[MC] = false;
		prevExtConn[MC] = true;
		extBeat[MC] = false;

		for (int t = 0; t < MAXTRACKS; t++) {
			wSteps[t] = 16;
			params[LENGTH_PARAM+t].setValue(wSteps[t]);
			bpm[t] = 0.1;
			clockSample[t] = 1.0;
			extSync[t] = false;
			extConn[t] = false;
			prevExtConn[t] = true;
			extBeat[t] = false;

			divClockSample[t] = 1.0;
			divMaxSample[t][0] = 0.0;
			divMaxSample[t][1] = 0.0;
			divPulse[t] = false;
			divPulseTime[t] = false;
			divCount[t] = 1;
			divSwing[t] = false;

			if (dontAdvanceSetting[t] == 2)
				dontAdvance[t] = dontAdvanceSetting[MC];
			else
				dontAdvance[t] = dontAdvanceSetting[t];

			edge[t] = false;
			prevEdge[t] = false;
			stepAdv[t] = false;
			clockAdv[t] = false;

			alreadyChanged[t] = false;
		}
		Module::onReset(e);
	}

	void onSampleRateChange() override {

		storeSamples = APP->engine->getSampleRate() / 1.5f;
		maxStoredProgramTime = APP->engine->getSampleRate() * 1.5;
		setButLightDelta = 2 / APP->engine->getSampleRate();

		sampleRateCoeff = (double)APP->engine->getSampleRate() * 60;
		oneMsTime = (APP->engine->getSampleRate()) / 1000;
		//oneMsTime = (APP->engine->getSampleRate()) / 10; // for testing purposes
	}

	
	#include "StepStation_json.hpp"
	

	void copyTrack(int t) {

		for (int s = 0; s < 16; s++)
			stepSeq_cbSeq[s] = wSeq[t][s];
		
		stepSeq_cbSteps = wSteps[t];

		if (userInputs[t][KNOB_RSTSTEP][0]) {
			stepSeq_cbRst = int(params[USER_PARAM+t+userInputs[t][KNOB_RSTSTEP][1]].getValue() * 15);
		} else {
			stepSeq_cbRst = 0;
		}

		stepSeq_clipboard = true;

	}

	void copyAllTracks() {
		
		for (int t = 0; t < MAXTRACKS; t++) {
			for (int s = 0; s < 16; s++)
				stepSeq8_cbSeq[t][s] = wSeq[t][s];

			stepSeq8_cbSteps = wSteps[0];
			stepStation_cbSteps[t] = wSteps[t];
			if (userInputs[t][KNOB_RSTSTEP][0]) {
				stepSeq8_cbRst = int(params[USER_PARAM+t+userInputs[t][KNOB_RSTSTEP][1]].getValue() * 15);
			} else {
				stepSeq8_cbRst = 0;
			}
			
		}

		stepSeq8_clipboard = true;

	}

	void copyUser(int t) {

		for (int i = 0; i < 4; i++)
			stepStation_cbUserTableTrack[i] = userTable[t][i];

		for (int i = 0; i < MAXUSER; i++)
			for (int j = 0; j < 2; j++)
				stepStation_cbUserInputsTrack[i][j] = userInputs[t][i][j];

		stepStation_cbUserValuesTrack[0] = params[USER_PARAM+t].getValue();
		stepStation_cbUserValuesTrack[1] = params[USER_PARAM+t+8].getValue();

		stepStation_cbCurrentModeTrack = currentMode[t];
		stepStation_cbDivMultTrack = params[DIVMULT_KNOB_PARAM+t].getValue();;
		
		stepStation_cbXcludeFromRunTrack = xcludeFromRun[t];
		stepStation_cbXcludeFromRstTrack = xcludeFromRst[t];

		//stepStation_cbRevTypeTrack = revType[t];
		stepStation_cbDontAdvanceSettingTrack = dontAdvanceSetting[t];
		stepStation_cbRstStepsWhenTrack = rstStepsWhen[t];

		stepStation_clipboardTrack = true;
	}

	void copyUserSettings() {
		for (int t = 0; t < MAXTRACKS; t++) {

			for (int i = 0; i < 4; i++)
				stepStation_cbUserTable[t][i] = userTable[t][i];

			for (int i = 0; i < MAXUSER; i++)
				for (int j = 0; j < 2; j++)
					stepStation_cbUserInputs[t][i][j] = userInputs[t][i][j];

			stepStation_cbUserValues[t][0] = params[USER_PARAM+t].getValue();
			stepStation_cbUserValues[t][1] = params[USER_PARAM+t+8].getValue();

			stepStation_cbCurrentMode[t] = currentMode[t];
			stepStation_cbDivMult[t] = params[DIVMULT_KNOB_PARAM+t].getValue();;
			
			stepStation_cbXcludeFromRun[t] = xcludeFromRun[t];
			stepStation_cbXcludeFromRst[t] = xcludeFromRst[t];

		}

		for (int t = 0; t < ALLTRACKS; t++) {
			//stepStation_cbRevType[t] = revType[t];
			stepStation_cbDontAdvanceSetting[t] = dontAdvanceSetting[t];
			stepStation_cbRstStepsWhen[t] = rstStepsWhen[t];
		}
		stepStation_cbSeqRunSetting = seqRunSetting;
		stepStation_cbInternalClock = internalClock;
		stepStation_cbBpmKnob = params[BPM_KNOB_PARAM].getValue();

		stepStation_userClipboard = true;
	}

	void copyPanel() {

		copyAllTracks();
		copyUserSettings();
		stepStation_clipboard = true;

	}

	void pasteToTrack(int t) {

		for (int s = 0; s < 16; s++) {
			wSeq[t][s] = stepSeq_cbSeq[s];
			params[STEP_PARAM+(t*16)+s].setValue(wSeq[t][s]);
		}
		wSteps[t] = stepSeq_cbSteps;
		params[LENGTH_PARAM+t].setValue(wSteps[t]);

		if (userInputs[t][KNOB_RSTSTEP][0])
			params[USER_PARAM+t+userInputs[t][KNOB_RSTSTEP][1]].setValue((float)stepSeq_cbRst / 15);

	}

	void pasteAllTracks() {

		for (int t = 0; t < MAXTRACKS; t++) {
			for (int s = 0; s < 16; s++)
				params[STEP_PARAM+(t*16)+s].setValue(stepSeq8_cbSeq[t][s]);
			params[LENGTH_PARAM+t].setValue(stepSeq8_cbSteps);

			if (stepStation_clipboard) {
				params[LENGTH_PARAM+t].setValue(stepStation_cbSteps[t]);
			}
			if (userInputs[t][KNOB_RSTSTEP][0]) {
				params[USER_PARAM+t+userInputs[t][KNOB_RSTSTEP][1]].setValue((float)stepSeq8_cbRst / 15);
			}
		}

	}

	void pasteUser(int t) {

		params[USER_PARAM+t].setValue(stepStation_cbUserValuesTrack[0]);
		params[USER_PARAM+t+8].setValue(stepStation_cbUserValuesTrack[1]);

		for (int i = 0; i < 4; i++)
			userTable[t][i] = stepStation_cbUserTableTrack[i];

		for (int i = 0; i < MAXUSER; i++)
			for (int j = 0; j < 2; j++)
				userInputs[t][i][j] = stepStation_cbUserInputsTrack[i][j];

		params[USER_PARAM+t].setValue(stepStation_cbUserValuesTrack[0]);
		params[USER_PARAM+t+8].setValue(stepStation_cbUserValuesTrack[1]);

		currentMode[t] = stepStation_cbCurrentModeTrack;
		params[DIVMULT_KNOB_PARAM+t].setValue(stepStation_cbDivMultTrack);
		
		xcludeFromRun[t] = stepStation_cbXcludeFromRunTrack;
		xcludeFromRst[t] = stepStation_cbXcludeFromRstTrack;

		//revType[t] = stepStation_cbRevTypeTrack;
		dontAdvanceSetting[t] = stepStation_cbDontAdvanceSettingTrack;
		rstStepsWhen[t] = stepStation_cbRstStepsWhenTrack;

	}

	void pasteUserSettings() {
		for (int t = 0; t < MAXTRACKS; t++) {
			params[USER_PARAM+t].setValue(stepStation_cbUserValues[t][0]);
			params[USER_PARAM+t+8].setValue(stepStation_cbUserValues[t][1]);

			for (int i = 0; i < 4; i++)
				userTable[t][i] = stepStation_cbUserTable[t][i];

			for (int i = 0; i < MAXUSER; i++)
				for (int j = 0; j < 2; j++)
					userInputs[t][i][j] = stepStation_cbUserInputs[t][i][j];

			params[USER_PARAM+t].setValue(stepStation_cbUserValues[t][0]);
			params[USER_PARAM+t+8].setValue(stepStation_cbUserValues[t][1]);

			currentMode[t] = stepStation_cbCurrentMode[t];
			params[DIVMULT_KNOB_PARAM+t].setValue(stepStation_cbDivMult[t]);
			
			xcludeFromRun[t] = stepStation_cbXcludeFromRun[t];
			xcludeFromRst[t] = stepStation_cbXcludeFromRst[t];

		}

		for (int t = 0; t < ALLTRACKS; t++) {
			//revType[t] = stepStation_cbRevType[t];
			dontAdvanceSetting[t] = stepStation_cbDontAdvanceSetting[t];
			rstStepsWhen[t] = stepStation_cbRstStepsWhen[t];
		}

		seqRunSetting = stepStation_cbSeqRunSetting;
		internalClock = stepStation_cbInternalClock;
		params[BPM_KNOB_PARAM].setValue(stepStation_cbBpmKnob);
	}

	void pastePanel() {

		pasteUserSettings();

		pasteAllTracks();

	}

	void eraseProgs() {

		for (int p = 0; p < 32; p++) {

			for (int t = 0; t < MAXTRACKS; t++) {
				for (int s = 0; s < 16; s++)
					progSeq[p][t][s] = 0.5f;
				progSteps[p][t] = 16;

				for (int u = 0; u < 4; u++)
					progUserTable[p][t][u] = defaultUserTable[t][u];

				for (int u = 0; u < MAXUSER; u++) {
					progUserInputs[p][t][u][0] = 0;
					progUserInputs[p][t][u][1] = 0;
				}

				progUserInputs[p][t][userTable[t][0]][0] = 1;
				progUserInputs[p][t][userTable[t][0]][1] = 0;
				progUserInputs[p][t][userTable[t][1]][0] = 1;
				progUserInputs[p][t][userTable[t][1]][1] = 0;
				progUserInputs[p][t][userTable[t][2]][0] = 1;
				progUserInputs[p][t][userTable[t][2]][1] = 8;
				progUserInputs[p][t][userTable[t][3]][0] = 1;
				progUserInputs[p][t][userTable[t][3]][1] = 8;

				progSteps[p][t] = 16;
				progCurrentMode[p][t] = 0;
				progDivMult[p][t] = 22;
				progXcludeFromRun[p][t] = false;
				progXcludeFromRst[p][t] = false;
				progUserValues[p][t][0] = 0.f;
				progUserValues[p][t][1] = 0.f;
				//progRevType[p][t] = 2;
				progDontAdvanceSetting[p][t] = 2;
				progRstStepsWhen[p][t] = 3;	
				progSampleDelay[p][t] = 6;
			}

			//progRevType[p][MC] = POSITIVE_V;
			progDontAdvanceSetting[p][MC] = 1;
			progRstStepsWhen[p][MC] = 1;
			progSampleDelay[p][MC] = 0;

			progSeqRunSetting[p] = 1;
			progInternalClock[p] = 1;
			progBpmKnob[p] = 1200.f;

		}
		lastProg = 0;
	}

	void resetUserSettings() {

		for (int t = 0; t < 8; t++) {
			for (int u = 0; u < 4; u++)
				userTable[t][u] = defaultUserTable[t][u];

			for (int u = 0; u < MAXUSER; u++)
				for (int i = 0; i < 2; i++)
				userInputs[t][u][i] = defaultUserInputs[t][u][i];

		}	
	}
	
	void inline resetAllSteps() {

		for (int t = 0; t < MAXTRACKS; t++) {

			if (!xcludeFromRst[t]) {
				resetTrackSteps(t);
			}
		}

		if (progInType != CV_TYPE)
			progKnob = 0;
	}

	void inline resetTrackSteps(int t) {

		lights[STEP_LIGHT+(t*16+step[t])].setBrightness(0);

		//alreadyChanged[t] = false;

		if (currentMode[t] != CVOLTAGE) {

			if (userInputs[t][IN_RSTSTEP][0]) {
				int tempUserIn = userInputs[t][IN_RSTSTEP][1];  // 0 oppure 8 a seconda che l'input sia U1 o U2
				float tempRst = inputs[USER_INPUT+t+tempUserIn].getVoltage() * 0.1;

				if (tempUserIn == 0) {
					if (userTable[t][1] == KNOB_ATN)
						tempRst *= params[USER_PARAM+t].getValue();
					else if (userTable[t][1] == KNOB_ATNV)
						tempRst *= params[USER_PARAM+t].getValue() * 2 - 1;
				} else { // altrimenti è 8
					if (userTable[t][3] == KNOB_ATN)
						tempRst *= params[USER_PARAM+t+8].getValue();
					else if (userTable[t][3] == KNOB_ATNV)
						tempRst *= params[USER_PARAM+t+8].getValue() * 2 - 1;
				}

				if (userInputs[t][KNOB_RSTSTEP][0])
					tempRst += params[USER_PARAM+t+userInputs[t][KNOB_RSTSTEP][1]].getValue();

				if (tempRst < 0)
					tempRst = 0;
				else if (tempRst > 1)
					tempRst = 1;

				step[t] = int(tempRst * 15);

			} else if (userInputs[t][KNOB_RSTSTEP][0]) {
				step[t] = int(params[USER_PARAM+t+userInputs[t][KNOB_RSTSTEP][1]].getValue() * 15);
			} else {
				step[t] = 0;
			}

			if (dontAdvanceSetting[t] == 2)
				dontAdvance[t] = dontAdvanceSetting[MC];
			else
				dontAdvance[t] = dontAdvanceSetting[t];
		}
		
	}

	void scanLastProg() {
		lastProg = 31;
		bool exitFunc = false;

		for (int p = 31; p >= 0; p--) {
			for (int t = 0; t < MAXTRACKS; t++) {
				for (int st = 0; st < 16; st++) {
					if (progSeq[p][t][st] != 0.f) {
						st = 16;
						t = MAXTRACKS;
						exitFunc = true;
					}
				}

				if (progSteps[p][t] != 16)
					exitFunc = true;
			}

			lastProg = p;

			if (exitFunc)
				p = 0;
		}
	}

	void randomizeSequence(int t) {
		for (int s = 0; s < 16; s++) {
			wSeq[t][s] = random::uniform();
			params[STEP_PARAM+(t*16)+s].setValue(wSeq[t][s]);
		}
	}

	void randomizeAll() {
		for (int t = 0; t < MAXTRACKS; t++)
			randomizeSequence(t);
	}

	void initializeSequence(int t) {
		
		float tempValue = 0.5;
		int tempRange = range[MC];
		if (range[t] != 10)
			tempRange = range[t];

		if (tempRange < 5)
			tempValue = 0.f;

		for (int s = 0; s < 16; s++) {
			wSeq[t][s] = tempValue;
			params[STEP_PARAM+(t*16)+s].setValue(tempValue);
		}
	}

	void initializeAll() {
		for (int t = 0; t < MAXTRACKS; t++)
			initializeSequence(t);
	}

	void changePpqnSetting() {
		ppqnChange = false;
		ppqn = tempPpqn;
		ppqnValue = ppqnTable[ppqn];
		ppqnComparison = ppqnValue - 1;

		pulseNr = 0;
		extSync[MC]	= false;
		if (extConn[MC])
			clockSample[MC] = 1.0;
	}

	float swingCalc(int t) {
		float swing;

		if (userInputs[t][IN_SWING][0]) {
			int tempUserIn = userInputs[t][IN_SWING][1];  // 0 oppure 8 a seconda che l'input sia U1 o U2
			float tempSwing = inputs[USER_INPUT+t+tempUserIn].getVoltage() * 0.1f;

			if (tempUserIn == 0) {
				if (userTable[t][1] == KNOB_ATN)
					tempSwing *= params[USER_PARAM+t].getValue();
				else if (userTable[t][1] == KNOB_ATNV)
					tempSwing *= params[USER_PARAM+t].getValue() * 2 - 1;
			} else { // altrimenti è 8
				if (userTable[t][3] == KNOB_ATN)
					tempSwing *= params[USER_PARAM+t+8].getValue();
				else if (userTable[t][3] == KNOB_ATNV)
					tempSwing *= params[USER_PARAM+t+8].getValue() * 2 - 1;
			}

			if (userInputs[t][KNOB_SWING][0])
				tempSwing += params[USER_PARAM+t+userInputs[t][KNOB_SWING][1]].getValue();

			if (tempSwing < 0)
				tempSwing = 0;
			else if (tempSwing > 1)
				tempSwing = 1;

			swing = tempSwing;

		} else if (userInputs[t][KNOB_SWING][0]) {
			swing = params[USER_PARAM+t+userInputs[t][KNOB_SWING][1]].getValue();
		} else {
			swing = 0;
		}
		
		return swing;
	}


	void inline stepForward(int t) {

		if (userInputs[t][IN_RETRIG][0]) {
			if (userInputs[t][KNOB_RETRIGPROB][0]) {
				if (inputs[USER_INPUT+t+userInputs[t][IN_RETRIG][1]].getVoltage() < 1.f)
					step[t]++;
				else if (random::uniform() >= params[USER_PARAM+t+userInputs[t][KNOB_RETRIGPROB][1]].getValue())
					step[t]++;
			} else {
				if (inputs[USER_INPUT+t+userInputs[t][IN_RETRIG][1]].getVoltage() < 1.f)
					step[t]++;
			}
		} else if (userInputs[t][KNOB_RETRIGPROB][0]) {
			if (random::uniform() >= params[USER_PARAM+t+userInputs[t][KNOB_RETRIGPROB][1]].getValue())
				step[t]++;
		} else {
			step[t]++;
		}

	}

	void inline stepBack(int t) {
		if (userInputs[t][IN_RETRIG][0]) {
			if (userInputs[t][KNOB_RETRIGPROB][0]) {
				if (inputs[USER_INPUT+t+userInputs[t][IN_RETRIG][1]].getVoltage() < 1.f)
					step[t]--;
				else if (random::uniform() >= params[USER_PARAM+t+userInputs[t][KNOB_RETRIGPROB][1]].getValue())
					step[t]--;
			} else {
				if (inputs[USER_INPUT+t+userInputs[t][IN_RETRIG][1]].getVoltage() < 1.f)
					step[t]--;
			}
		} else if (userInputs[t][KNOB_RETRIGPROB][0]) {
			if (random::uniform() >= params[USER_PARAM+t+userInputs[t][KNOB_RETRIGPROB][1]].getValue())
				step[t]--;
		} else {
			step[t]--;
		}
	}

	float inline outScale(float outVal, int t) {
		if (userInputs[t][IN_OUTSCALE][0]) {
			int tempUserIn = userInputs[t][IN_OUTSCALE][1];  // 0 oppure 8 a seconda che l'input sia U1 o U2
			float tempOutScale = inputs[USER_INPUT+t+tempUserIn].getVoltage() * 0.1;

			if (tempUserIn == 0) {
				if (userTable[t][1] == KNOB_ATN)
					tempOutScale *= params[USER_PARAM+t].getValue();
				else if (userTable[t][1] == KNOB_ATNV)
					tempOutScale *= params[USER_PARAM+t].getValue() * 2 - 1;
			} else { // altrimenti è 8
				if (userTable[t][3] == KNOB_ATN)
					tempOutScale *= params[USER_PARAM+t+8].getValue();
				else if (userTable[t][3] == KNOB_ATNV)
					tempOutScale *= params[USER_PARAM+t+8].getValue() * 2 - 1;
			}

			// outScale knob is added to input (or is to prefer that outScale knob scales the CV input?)

			if (userInputs[t][KNOB_OUTSCALE][0])
				tempOutScale += params[USER_PARAM+t+userInputs[t][KNOB_OUTSCALE][1]].getValue();
				//tempOutScale *= params[USER_PARAM+t+userInputs[t][KNOB_OUTSCALE][1]].getValue();

			if (tempOutScale < 0)
				tempOutScale = 0;
			else if (tempOutScale > 1)
				tempOutScale = 1;


			return outVal * tempOutScale;

		} else if (userInputs[t][KNOB_OUTSCALE][0]) {
			return outVal * params[USER_PARAM+t+userInputs[t][KNOB_OUTSCALE][1]].getValue();
		} else {
			return outVal;
		}

	}

	void inline calcChange(int t) {

		if (!alreadyChanged[t]) {

			bool change = false;

			if (userInputs[t][IN_CHANGE][0]) {
				if (userInputs[t][KNOB_CHANGEPROB][0]) {
					if (inputs[USER_INPUT+t+userInputs[t][IN_CHANGE][1]].isConnected() && 
						inputs[USER_INPUT+t+userInputs[t][IN_CHANGE][1]].getVoltage() >= 1.f) {

						if (random::uniform() <= params[USER_PARAM+t+userInputs[t][KNOB_CHANGEPROB][1]].getValue())
							change = true;
						
					}
				} else {
					if (inputs[USER_INPUT+t+userInputs[t][IN_CHANGE][1]].isConnected() && 
						inputs[USER_INPUT+t+userInputs[t][IN_CHANGE][1]].getVoltage() >= 1.f) {

						change = true;

					}
				}

			} else {

				if (userInputs[t][IN_CHANGEPROB][0]) {
					if (userInputs[t][KNOB_CHANGEPROB][0]) {

						if (inputs[USER_INPUT+t+userInputs[t][IN_CHANGEPROB][1]].isConnected()) {

							if(inputs[USER_INPUT+t+userInputs[t][IN_CHANGEPROB][1]].getVoltage() >= 1.f &&
								random::uniform() <= params[USER_PARAM+t+userInputs[t][KNOB_CHANGEPROB][1]].getValue())
									change = true;

						} else if (userInputs[t][KNOB_CHANGEPROB][0] && random::uniform() <= params[USER_PARAM+t+userInputs[t][KNOB_CHANGEPROB][1]].getValue()) {
							change = true;
						}

					} else if (inputs[USER_INPUT+t+userInputs[t][IN_CHANGEPROB][1]].isConnected() && inputs[USER_INPUT+t+userInputs[t][IN_CHANGEPROB][1]].getVoltage() >= 1.f) {
						change = true;
					}
				} else if (userInputs[t][KNOB_CHANGEPROB][0] && random::uniform() <= params[USER_PARAM+t+userInputs[t][KNOB_CHANGEPROB][1]].getValue()) {
					change = true;
				}

			}

			if (change) {
				wSeq[t][step[t]] = random::uniform();
				params[STEP_PARAM+(t*16)+step[t]].setValue(wSeq[t][step[t]]);
			}

			alreadyChanged[t] = true;
		}
	}

	void process(const ProcessArgs& args) override {


		// ----------- AUTO SWITCH

		instantProgChange = int(params[AUTO_PARAM].getValue());
		lights[AUTO_LIGHT].setBrightness(instantProgChange);

/*

																				██████╗░██████╗░░█████╗░░██████╗░██████╗░░█████╗░███╗░░░███╗░██████╗
																				██╔══██╗██╔══██╗██╔══██╗██╔════╝░██╔══██╗██╔══██╗████╗░████║██╔════╝
																				██████╔╝██████╔╝██║░░██║██║░░██╗░██████╔╝███████║██╔████╔██║╚█████╗░
																				██╔═══╝░██╔══██╗██║░░██║██║░░╚██╗██╔══██╗██╔══██║██║╚██╔╝██║░╚═══██╗
																				██║░░░░░██║░░██║╚█████╔╝╚██████╔╝██║░░██║██║░░██║██║░╚═╝░██║██████╔╝
																				╚═╝░░░░░╚═╝░░╚═╝░╚════╝░░╚═════╝░╚═╝░░╚═╝╚═╝░░╚═╝╚═╝░░░░░╚═╝╚═════╝░
*/
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
			selectedProg = progKnob;
			prevProgKnob = progKnob;

			for (int t = 0; t < MAXTRACKS; t++) {
				for (int i = 0; i < 16; i++) {
					nextSeq[t][i] = progSeq[selectedProg][t][i];
					params[STEP_PARAM+(t*16)+i].setValue(nextSeq[t][i]);
				}

				nextSteps[t] = progSteps[selectedProg][t];
				params[LENGTH_PARAM+t].setValue(nextSteps[t]);

				for (int i = 0; i < 4; i++)
					nextUserTable[t][i] = progUserTable[selectedProg][t][i];

				for (int i = 0; i < MAXUSER; i++)
					for (int j = 0; j < 2; j++)
						nextUserInputs[t][i][j] = progUserInputs[selectedProg][t][i][j];

				nextUserValues[t][0] = progUserValues[selectedProg][t][0];
				nextUserValues[t][1] = progUserValues[selectedProg][t][1];

				nextCurrentMode[t] = progCurrentMode[selectedProg][t];
				nextDivMult[t] = progDivMult[selectedProg][t];

				nextXcludeFromRun[t] = progXcludeFromRun[selectedProg][t];
				nextXcludeFromRst[t] = progXcludeFromRst[selectedProg][t];

			}

			for (int t = 0; t < ALLTRACKS; t++) {
				
				nextRange[t] = progRange[selectedProg][t];
				//nextRevType[t] = progRevType[selectedProg][t];
				nextDontAdvanceSetting[t] = progDontAdvanceSetting[selectedProg][t];
				nextRstStepsWhen[t] = progRstStepsWhen[selectedProg][t];
				nextSampleDelay[t] = progSampleDelay[selectedProg][t];
			}

			nextSeqRunSetting = progSeqRunSetting[selectedProg];
			nextInternalClock = progInternalClock[selectedProg];
			nextBpmKnob = progBpmKnob[selectedProg];

			setButLight = true;
			setButLightValue = 0.f;

		}

		// -------- CURRENT SEQ UPDATE

		progToSet = false;

		progSetBut = params[SET_PARAM].getValue();
		if (progSetBut >= 1.f && prevProgSetBut < 1.f)
			progToSet = true;

		prevProgSetBut = progSetBut;

		if (!progChanged) {
			for (int t = 0; t < 8; t++) {
				for (int i = 0; i < 16; i++)
					wSeq[t][i] = params[STEP_PARAM+(t*16)+i].getValue();
				wSteps[t] = params[LENGTH_PARAM+t].getValue();
			}

		} else {

			if (instantProgChange || progToSet) {

				for (int t = 0; t < MAXTRACKS; t++) {
					for (int i = 0; i < 16; i++)
						wSeq[t][i] = nextSeq[t][i];

					wSteps[t] = nextSteps[t];
					params[LENGTH_PARAM+t].setValue(wSteps[t]);

					for (int i = 0; i < 4; i++)
						userTable[t][i] = nextUserTable[t][i];

					for (int i = 0; i < MAXUSER; i++)
						for (int j = 0; j < 2; j++)
							userInputs[t][i][j] = nextUserInputs[t][i][j];

					params[USER_PARAM+t].setValue(nextUserValues[t][0]);
					params[USER_PARAM+t+8].setValue(nextUserValues[t][1]);

					currentMode[t] = nextCurrentMode[t];
					params[DIVMULT_KNOB_PARAM+t].setValue(nextDivMult[t]);
					
					xcludeFromRun[t] = nextXcludeFromRun[t];
					xcludeFromRst[t] = nextXcludeFromRst[t];
					

				}

				for (int t = 0; t < ALLTRACKS; t++) {
					range[t] = nextRange[t];
					//revType[t] = nextRevType[t];
					dontAdvanceSetting[t] = nextDontAdvanceSetting[t];
					rstStepsWhen[t] = nextRstStepsWhen[t];
					sampleDelay[t] = nextSampleDelay[t];
				}

				seqRunSetting = nextSeqRunSetting;
				internalClock = nextInternalClock;
				params[BPM_KNOB_PARAM].setValue(nextBpmKnob);

				workingProg = selectedProg;
				if (progInType == CV_TYPE)
					savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
				else
					savedProgKnob = progKnob;
				
				progChanged = false;
				progToSet = false;
				setButLight = false;
				setButLightValue = 0.f;

				if (rstSeqOnProgChange) {
					resetAllSteps();
				}

			} else { 	// IF SET IS PENDING -> GET NEW SETTINGS
				
				for (int t = 0; t < MAXTRACKS; t++) {
					for (int i = 0; i < 16; i++)
						nextSeq[t][i] = params[STEP_PARAM+(t*16)+i].getValue();

					nextSteps[t] = params[LENGTH_PARAM+t].getValue();
				}
			}
		}

		// -------------------------- RECALL PROG

		recallBut = params[RECALL_PARAM].getValue();
		lights[RECALL_LIGHT].setBrightness(recallBut);

		if (recallBut >= 1.f && prevRecallBut < 1.f) {

			if (!progChanged) {

				for (int t = 0; t < MAXTRACKS; t++) {
					for (int i = 0; i < 16; i++) {
						wSeq[t][i] = progSeq[selectedProg][t][i];
						params[STEP_PARAM+(t*16)+i].setValue(wSeq[t][i]);
					}

					wSteps[t] = progSteps[selectedProg][t];
					params[LENGTH_PARAM+t].setValue(wSteps[t]);

					for (int i = 0; i < 4; i++)
						userTable[t][i] = progUserTable[selectedProg][t][i];

					for (int i = 0; i < MAXUSER; i++)
						for (int j = 0; j < 2; j++)
							userInputs[t][i][j] = progUserInputs[selectedProg][t][i][j];

					params[USER_PARAM+t].setValue(progUserValues[selectedProg][t][0]);
					params[USER_PARAM+t+8].setValue(progUserValues[selectedProg][t][1]);

					currentMode[t] = progCurrentMode[selectedProg][t];
					params[DIVMULT_KNOB_PARAM+t].setValue(progDivMult[selectedProg][t]);
					
					xcludeFromRun[t] = progXcludeFromRun[selectedProg][t];
					xcludeFromRst[t] = progXcludeFromRst[selectedProg][t];
				}

				for (int t = 0; t < ALLTRACKS; t++) {
					range[t] = progRange[selectedProg][t];
					//revType[t] = progRevType[selectedProg][t];
					dontAdvanceSetting[t] = progDontAdvanceSetting[selectedProg][t];
					rstStepsWhen[t] = progRstStepsWhen[selectedProg][t];
					sampleDelay[t] = progSampleDelay[selectedProg][t];
				}

				seqRunSetting = progSeqRunSetting[selectedProg];
				internalClock = progInternalClock[selectedProg];
				params[BPM_KNOB_PARAM].setValue(progBpmKnob[selectedProg]);

				workingProg = selectedProg;
				if (progInType == CV_TYPE)
					savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
				else
					savedProgKnob = progKnob;

			} else {

				for (int t = 0; t < MAXTRACKS; t++) {
					for (int i = 0; i < 16; i++)
						params[STEP_PARAM+(t*16)+i].setValue(wSeq[t][i]);

					params[LENGTH_PARAM+t].setValue(wSteps[t]);
				}

				params[PROG_PARAM].setValue(workingProg);

				//savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
				savedProgKnob = workingProg;
				selectedProg = workingProg;
				prevProgKnob = workingProg;

			}
			progChanged = false;
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
				for (int t = 0; t < MAXTRACKS; t++) {
					for (int i = 0; i < 16; i++)
						progSeq[progKnob][t][i] = wSeq[t][i];

					progSteps[progKnob][t] = wSteps[t];

					for (int i = 0; i < 4; i++)
						progUserTable[progKnob][t][i] = userTable[t][i];

					for (int i = 0; i < MAXUSER; i++)
						for (int j = 0; j < 2; j++)
							progUserInputs[progKnob][t][i][j] = userInputs[t][i][j];

					progUserValues[progKnob][t][0] = params[USER_PARAM+t].getValue();
					progUserValues[progKnob][t][1] = params[USER_PARAM+t+8].getValue();

					progCurrentMode[progKnob][t] = currentMode[t];
					progDivMult[progKnob][t] = params[DIVMULT_KNOB_PARAM+t].getValue();
					
					progXcludeFromRun[progKnob][t] = xcludeFromRun[t];
					progXcludeFromRst[progKnob][t] = xcludeFromRst[t];
				}

				for (int t = 0; t < ALLTRACKS; t++) {
					//progRevType[progKnob][t] = revType[t];
					progDontAdvanceSetting[progKnob][t] = dontAdvanceSetting[t];
					progRstStepsWhen[progKnob][t] = rstStepsWhen[t];
					progSampleDelay[progKnob][t] = sampleDelay[t];
				}

				progSeqRunSetting[progKnob] = seqRunSetting;
				progInternalClock[progKnob] = internalClock;
				progBpmKnob[progKnob] = params[BPM_KNOB_PARAM].getValue();

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

		lights[SET_LIGHT].setBrightness(setButLightValue);

/*

																										░█████╗░██╗░░░░░░█████╗░░█████╗░██╗░░██╗
																										██╔══██╗██║░░░░░██╔══██╗██╔══██╗██║░██╔╝
																										██║░░╚═╝██║░░░░░██║░░██║██║░░╚═╝█████═╝░
																										██║░░██╗██║░░░░░██║░░██║██║░░██╗██╔═██╗░
																										╚█████╔╝███████╗╚█████╔╝╚█████╔╝██║░╚██╗
																										░╚════╝░╚══════╝░╚════╝░░╚════╝░╚═╝░░╚═╝
*/

		// ---------------------- initialize clockAdv & swing

		clockAdv[MC] = false;

		for (int t = 0; t < MAXTRACKS; t++) {
			clockAdv[t] = false;
			stepAdv[t] = false;

			if (userInputs[t][KNOB_SWING][0] || userInputs[t][IN_SWING][0])
				divSwing[t] = true;
			else
				divSwing[t] = false;

			// get external clock from each track // copiato da sotto per ottimizzare
			extConn[t] = inputs[CLK_INPUT+t].isConnected();
			if (extConn[t] && !prevExtConn[t]) {
				extSync[t] = false;
			}
			prevExtConn[t] = extConn[t];
		}

		// ---------------------- check PPQN

		if (ppqnChange)
			changePpqnSetting();

		// ********* EXTERNAL CONNECTION

		if (inputs[RUN_INPUT].isConnected()) {
			runTrig = inputs[RUN_INPUT].getVoltage();
			if (runType == RUN_GATE) {
				if (runTrig > 1.f) {
					internalClock = true;
				} else {
					internalClock = false;
				}
			} else {	// runType == RUN_TRIG
				buttRunTrig = params[INTCLOCKBUT_PARAM].getValue();
				if ((runTrig >= 1 && prevRunTrig < 1.f) || (buttRunTrig >= 1.f && prevButtRunTrig < 1.f)) {
					if (internalClock) {
						internalClock = false;
					} else {
						internalClock = true;
					}			
				}
				prevButtRunTrig = buttRunTrig;
			}
			prevRunTrig = runTrig;
		} else {
			buttRunTrig = params[INTCLOCKBUT_PARAM].getValue();
			if ((runTrig >= 1 && prevRunTrig < 1.f) || (buttRunTrig >= 1.f && prevButtRunTrig < 1.f)) {
				if (internalClock) {
					internalClock = false;
				} else {
					internalClock = true;
				}			
			}
			prevButtRunTrig = buttRunTrig;
		}

		if (inputs[EXTCLOCK_INPUT].isConnected()) {
			externalClock = true;
			if (externalClock && !prevExternalClock) {
				if (cvClockIn) {
					extSync[MC] = true;
					prevCvClockInValue[MC] = 11;
				} else {
					extSync[MC] = false;
					//bpm[MC] = 0.0;
					extBpm = 0.0;
					pulseNr = 0;
				}
			}
		} else {
			externalClock = false;
			extSync[MC] = false;
			//bpm[MC] = 0.0;
			extBpm = 0.0;
			pulseNr = 0;
		}
		prevExternalClock = externalClock;

		if (internalClock) {
			extConn[MC] = false;
			runSetting = 1;
		} else if (externalClock) {
			extConn[MC] = true;
			runSetting = 1;
		} else {
			extConn[MC] = false;
			runSetting = 0;
		}

		if (cvClockOut) {
			if (internalClock && !prevInternalClock) {
				bpm[MC] = (double)params[BPM_KNOB_PARAM].getValue()/10;
				cvClockOutValue = log2(bpm[MC] / 120.f);
				outputs[CLOCK_OUTPUT].setVoltage(cvClockOutValue);
			} else if (!internalClock && prevInternalClock) {
				if (extConn[MC]) {
					cvClockOutValue = log2(extBpm / 120.f);
					outputs[CLOCK_OUTPUT].setVoltage(cvClockOutValue);
				}
			}
		}

		prevInternalClock = internalClock;

		lights[RUNBUT_LIGHT].setBrightness(internalClock);

		if (!runSetting && prevRunSetting) {
			//runSetting = 0;
			if (resetOnStop) {
				resetStart = true;
				//if (!extConn[MC])
				if (internalClock)
					clockSample[MC] = 1.0;
				
				if (!cvClockOut)
					outputs[CLOCK_OUTPUT].setVoltage(0.f);
				
				for (int t = 0; t < MAXTRACKS; t++) {
					divPulse[t] = false;
					divClockSample[t] = 1.0;
					divMaxSample[t][0] = 0.0;
					divMaxSample[t][1] = 0.0;
					divCount[t] = 999999;
					//outputs[DIVMULT_OUTPUT+t].setVoltage(0.f);
					//outputs[OUT_OUTPUT+t].setVoltage(0.f);

				}
			}
			if (resetPulseOnStop) {
				resetPulse[MC] = true;
				resetPulseTime[MC] = oneMsTime;
			}
		} else if (runSetting && !prevRunSetting) {
			
			if (resetOnRun) {

				resetStart = true;
				if (!extConn[MC])	//             <-    DA RIVEDERE *************************
					clockSample[MC] = 1.0;
				else
					extSync[MC] = false;
				
				if (!cvClockOut)
					outputs[CLOCK_OUTPUT].setVoltage(0.f);
				
				for (int t = 0; t < MAXTRACKS; t++) {
					divPulse[t] = false;
					divClockSample[t] = 1.0;
					divMaxSample[t][0] = 0.0;
					divMaxSample[t][1] = 0.0;
					divCount[t] = 999999;
					//outputs[DIVMULT_OUTPUT+t].setVoltage(0.f);
					//outputs[OUT_OUTPUT+t].setVoltage(0.f);
					edge[t] = false;
				}
			}

			if (resetPulseOnRun) {
				resetPulse[MC] = true;
				resetPulseTime[MC] = oneMsTime;
			}
		}

		prevRunSetting = runSetting;

		// ---------------------------------------------------------------------------------

		// **********  RESET ALL

		resetBut = params[RSTALLBUT_PARAM].getValue();
		if (resetBut)
			resetValue[MC] = 1;
		else
			resetValue[MC] = inputs[RSTALL_INPUT].getVoltage();

		lights[RSTALLBUT_LIGHT].setBrightness(resetValue[MC]);

		if (resetValue[MC] >= 1 && prevResetValue[MC] < 1) {

			resetAllSteps();

			if (rstClkOnRst) {

				// ******** NEW ***********
				if (!extConn[MC])
					clockSample[MC] = 1.0;
				else {
					extClockSample = 1.0;
					//extClockMaxSample = 0;
					//extMaxPulseSample = 0;
				}

				if (!cvClockOut)
					outputs[CLOCK_OUTPUT].setVoltage(0.f);

				for (int t = 0; t < MAXTRACKS; t++) {
					if (!extConn[t]) {
						divPulse[t] = false;
						divClockSample[t] = 1.0;
						divMaxSample[t][0] = 0.0;
						divMaxSample[t][1] = 0.0;
						divCount[t] = 999999;
						//outputs[DIVMULT_OUTPUT+t].setVoltage(0.f);
						//outputs[OUT_OUTPUT+t].setVoltage(0.f);
						edge[t] = false;
					}
				}

				resetPulse[MC] = true;
				resetPulseTime[MC] = oneMsTime;

				//resetDiv = true;
			}
		}
		prevResetValue[MC] = resetValue[MC];

		//if (!extConn[MC]) {
		if (internalClock) {

			// ************************* INTERNAL MAIN CLOCK

			// *********** BPM CALC - INTERNAL MAIN CLOCK
			
			bpm[MC] = (double)params[BPM_KNOB_PARAM].getValue()/10;
			
			if (cvClockOut) {
				
				if (bpm[MC] != prevBpm[MC])
					cvClockOutValue = log2(bpm[MC] / 120.f);
				
				outputs[CLOCK_OUTPUT].setVoltage(cvClockOutValue);
				
			}
			prevBpm[MC] = bpm[MC];

			if (runSetting && resetPulseTime[MC] <= 0) {

				// ****** CLOCK PULSE WIDTH - INTERNAL MAIN CLOCK

				//maxPulseSample = clockMaxSample * (double)params[PW_KNOB_PARAM].getValue();
				maxPulseSample[MC] = clockMaxSample[MC] * 0.5;
				if (clockSample[MC] > maxPulseSample[MC])
					if (!cvClockOut)
						outputs[CLOCK_OUTPUT].setVoltage(0.f);

				// ************ DIV / MULT - INTERNAL MC TRACKS

				for (int t = 0; t < MAXTRACKS; t++) {

					if (!extConn[t]) {

						if (!divSwing[t]) {

							if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22 && divClockSample[t] > divMaxSample[t][0]) {
								// ***** CLOCK MULTIPLIER *****
								divClockSample[t] = 1.0;
								//outputs[DIVMULT_OUTPUT+t].setVoltage(10.f);
								//outputs[OUT_OUTPUT+t].setVoltage(10.f);
								divPulse[t] = true;
								edge[t] = true;
							}

							// ***** CLOCK MULTIPLIER/DIVIDER - PULSE WIDTH OFF
							//if (divPulse[t] && divClockSample[t] > divMaxSample[t][0] * params[DIVPW_KNOB_PARAM+t].getValue()) {
							if (divPulse[t] && divClockSample[t] > divMaxSample[t][0] * 0.5) {
								//outputs[DIVMULT_OUTPUT+t].setVoltage(0.f);
								//outputs[OUT_OUTPUT+t].setVoltage(0.f);
								divPulse[t] = false;
								edge[t] = false;
							}
							
						} else {

							if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22 && divClockSample[t] > divMaxSample[t][divOddCounter[t]]) {
								// ***** CLOCK MULTIPLIER *****
								divPulseTime[t] = oneMsTime;
								//outputs[DIVMULT_OUTPUT+t].setVoltage(10.f);
								//outputs[OUT_OUTPUT+t].setVoltage(10.f);
								divPulse[t] = true;
								edge[t] = true;

								if (divOddCounter[t] == 0) {
									divClockSample[t] = 1.0;
									divOddCounter[t] = 1;
								} else {
									divClockSample[t] = 1.0 + divMaxSample[t][1] - divMaxSample[t][0];
									divOddCounter[t] = 0;
								}
							}
						}
					}
				}

				// ---------------------------------	INTERNAL CLOCK - MAX SAMPLES CONTROL & TRACKS RE-INIT

				clockMaxSample[MC] = sampleRateCoeff / bpm[MC];
				
				if (clockSample[MC] > clockMaxSample[MC] || resetStart)  {

					clockAdv[MC] = true;

					if (resetStart) {
						clockSample[MC] = 1.0;
						resetStart = false;
					} else

						clockSample[MC] -= clockMaxSample[MC];
					
					for (int t = 0; t < MAXTRACKS; t++) {
						
						if (!extConn[t]) {

							if (!divSwing[t]) {

								if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22) {
									// ***** CLOCK MULTIPLIER *****
									divMaxSample[t][0] = clockMaxSample[MC] / (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
									divMaxSample[t][1] = divMaxSample[t][0];
									divClockSample[t] = 1.0;
									//outputs[DIVMULT_OUTPUT+t].setVoltage(10.f);
									//outputs[OUT_OUTPUT+t].setVoltage(10.f);
									divPulse[t] = true;
									edge[t] = true;
								} else {
									// ***** CLOCK DIVIDER *****
									divMaxSample[t][0] = clockMaxSample[MC] * (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
									divMaxSample[t][1] = divMaxSample[t][0];
									divCount[t]++;
									if (divCount[t] > divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]) {
										divClockSample[t] = 1.0;
										divCount[t] = 1;
										//outputs[DIVMULT_OUTPUT+t].setVoltage(10.f);
										//outputs[OUT_OUTPUT+t].setVoltage(10.f);
										divPulse[t] = true;
										edge[t] = true;
									}
								}
							} else {

								if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22) {
									// ***** CLOCK MULTIPLIER *****
									divMaxSample[t][0] = clockMaxSample[MC] / (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
									//divMaxSample[t][1] = divMaxSample[t][0] + (divMaxSample[][0] * params[DIVPW_KNOB_PARAM+t].getValue());
									//divMaxSample[t][1] = divMaxSample[t][0] + (divMaxSample[t][0] * 0.5);
									divMaxSample[t][1] = divMaxSample[t][0] + (divMaxSample[t][0] * swingCalc(t));
									divOddCounter[t] = 1;
									divClockSample[t] = 1.0;
									divPulseTime[t] = oneMsTime;
									//outputs[DIVMULT_OUTPUT+t].setVoltage(10.f);
									//outputs[OUT_OUTPUT+t].setVoltage(10.f);
									divPulse[t] = true;
									edge[t] = true;
								} else {
									// ***** CLOCK DIVIDER *****
									divMaxSample[t][0] = clockMaxSample[MC] * (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
									divMaxSample[t][1] = divMaxSample[t][0];
									divCount[t]++;
									if (divCount[t] > divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]) {
										divOddCounter[t] = 1;
										divClockSample[t] = 1.0;
										divCount[t] = 1;
										divPulseTime[t] = oneMsTime;
										//outputs[DIVMULT_OUTPUT+t].setVoltage(10.f);
										//outputs[OUT_OUTPUT+t].setVoltage(10.f);
										divPulse[t] = true;
										edge[t] = true;
									}
								}
							}
						}
					}

					if (!cvClockOut)
						outputs[CLOCK_OUTPUT].setVoltage(10.f);
					
				}

				clockSample[MC]++;
				for (int t = 0; t < 8; t++) {
					if (!extConn[t])
						divClockSample[t]++;
				}

			}

		}


/*

																						███████╗██╗░░██╗████████╗  ░█████╗░██╗░░░░░░█████╗░░█████╗░██╗░░██╗
																						██╔════╝╚██╗██╔╝╚══██╔══╝  ██╔══██╗██║░░░░░██╔══██╗██╔══██╗██║░██╔╝
																						█████╗░░░╚███╔╝░░░░██║░░░  ██║░░╚═╝██║░░░░░██║░░██║██║░░╚═╝█████═╝░
																						██╔══╝░░░██╔██╗░░░░██║░░░  ██║░░██╗██║░░░░░██║░░██║██║░░██╗██╔═██╗░
																						███████╗██╔╝╚██╗░░░██║░░░  ╚█████╔╝███████╗╚█████╔╝╚█████╔╝██║░╚██╗
																						╚══════╝╚═╝░░╚═╝░░░╚═╝░░░  ░╚════╝░╚══════╝░╚════╝░░╚════╝░╚═╝░░╚═╝

*/


		if (externalClock) {

			// ************************************************ EXTERNAL MAIN CLOCK

			// ********** EXTERNAL MAIN CLOCK SYNC

			// ************** EXTERNAL MAIN CLOCK CV CV CV

			if (cvClockIn) {
				cvClockInValue[MC] = inputs[EXTCLOCK_INPUT].getVoltage();
				if (cvClockInValue[MC] != prevCvClockInValue[MC]) {
					extBpm = 120 * pow(2, cvClockInValue[MC]);

					//DEBUG("PREV %f",extBpm);
					extBpm = round(extBpm * 10)/10;
					//DEBUG("POST %f",extBpm);
					
					if (extBpm > 999)
						extBpm = 999;

					if (!internalClock) {
						bpm[MC] = extBpm;
						if (cvClockOut) {
							cvClockOutValue = log2(extBpm / 120.f);
							outputs[CLOCK_OUTPUT].setVoltage(cvClockOutValue);
						}
					}
				}
				prevCvClockInValue[MC] = cvClockInValue[MC];
				
				//clockMaxSample[MC] = sampleRateCoeff / bpm[MC];
				extClockMaxSample = sampleRateCoeff / extBpm;
				
				if (extClockSample > extClockMaxSample || resetStartExt)  {

					if (resetStartExt) {
						extClockSample = 1.0;
						resetStartExt = false;		/// <---- cambiato con il nuovo extBpm, controllare
					} else
						extClockSample -= extClockMaxSample;

					//if (runSetting) {
						extBeat[MC] = true;
						//extClockSample = 1.0;
					//}

					// ----------------------------------------EXTERNAL MAIN CLOCK CV -> RE-INIT TRACKS 
					if (!internalClock) {
						for (int t = 0; t < MAXTRACKS; t++) {
							
							if (!extConn[t]) {
								if (!divSwing[t]) {

									if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22) {
										// ***** CLOCK MULTIPLIER *****
										//divMaxSample[t][0] = clockMaxSample[MC] / (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
										divMaxSample[t][0] = extClockMaxSample / (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
										divMaxSample[t][1] = divMaxSample[t][0];
										divClockSample[t] = 1.0;
										//outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
										divPulse[t] = true;
										edge[t] = true;
									} else {
										// ***** CLOCK DIVIDER *****
										//divMaxSample[t][0] = clockMaxSample[MC] * (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
										divMaxSample[t][0] = extClockMaxSample * (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
										divMaxSample[t][1] = divMaxSample[t][0];
										divCount[t]++;
										if (divCount[t] > divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]) {
											divClockSample[t] = 1.0;
											divCount[t] = 1;
											//outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
											divPulse[t] = true;
											edge[t] = true;
										}
									}
								} else {
									
									if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22) {
										// ***** CLOCK MULTIPLIER *****
										//divMaxSample[t][0] = clockMaxSample[MC] / (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
										divMaxSample[t][0] = extClockMaxSample / (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
										//divMaxSample[t][1] = divMaxSample[t][0] + (divMaxSample[t][0] * params[DIVPW_KNOB_PARAM+t].getValue());
										//divMaxSample[t][1] = divMaxSample[t][0] + (divMaxSample[t][0] * 0.5);
										divMaxSample[t][1] = divMaxSample[t][0] + (divMaxSample[t][0] * swingCalc(t));
										divOddCounter[t] = 1;
										divClockSample[t] = 1.0;
										divPulseTime[t] = oneMsTime;
										//outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
										divPulse[t] = true;
										edge[t] = true;
									} else {
										// ***** CLOCK DIVIDER *****
										//divMaxSample[t][0] = clockMaxSample[MC] * (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
										divMaxSample[t][0] = extClockMaxSample * (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
										divMaxSample[t][1] = divMaxSample[t][0];
										divCount[t]++;
										if (divCount[t] > divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]) {
											divOddCounter[t] = 1;
											divClockSample[t] = 1.0;
											divCount[t] = 1;
											divPulseTime[t] = oneMsTime;
											//outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
											divPulse[t] = true;
											edge[t] = true;
										}
									}
								}
							}
						}

						if (!cvClockOut)
							outputs[CLOCK_OUTPUT].setVoltage(10.f);
					}
				} else {

					extBeat[MC] = false;
				}

			} else { 	// ************** EXTERNAL MAIN CLOCK WITH PULSES
			
				extTrigValue[MC] = inputs[EXTCLOCK_INPUT].getVoltage();
					
				if (extTrigValue[MC] >= 1 && prevExtTrigValue[MC] < 1) {

					pulseNr++;
					if (extSync[MC]) {

						if (pulseNr > ppqnComparison) {
							pulseNr = 0;
							//clockMaxSample[MC] = clockSample[MC];
							extClockMaxSample = extClockSample;
							extClockSample = 0.0;
							
							//if (runSetting)
								extBeat[MC] = true;

							// calculate bpms
							extBpm = round(sampleRateCoeff / extClockMaxSample);
							if (extBpm > 999)
								extBpm = 999;

							if (!internalClock) {
								bpm[MC] = extBpm;
								if (cvClockOut) {
									cvClockOutValue = log2(extBpm / 120.f);
									outputs[CLOCK_OUTPUT].setVoltage(cvClockOutValue);
								}
							}
						}

					} else {

						//bpm[MC] = 0.0;
						extBpm = 0.0;
						extSync[MC] = true;
						//clockSample[MC] = 1.0;
						extClockSample = 1.0;
						pulseNr = 0;

						//if (runSetting)
							extBeat[MC] = true;
					}

				} else {
					extBeat[MC] = false;
				}
				prevExtTrigValue[MC] = extTrigValue[MC];
			
			}

			// **************   RUN PROCESS   ***************  EXTERNAL CLOCK CV & PULSES

			//if (runSetting && resetPulseTime[MC] <= 0) {
			if (resetPulseTime[MC] <= 0) {

				// ****** CLOCK PULSE WIDTH

				extMaxPulseSample = extClockMaxSample * 0.5;	// <<<<<<<<<<------------------ OTTIMIZZARE POSIZIONE
				
				if (!internalClock) {

					if (extClockSample > extMaxPulseSample)
						if (!cvClockOut)
							outputs[CLOCK_OUTPUT].setVoltage(0.f);

					// ************ DIV / MULT

					for (int t = 0; t < MAXTRACKS; t++) {

						if (!extConn[t]) {
							if(!divSwing[t]) {


								if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22 && divClockSample[t] > divMaxSample[t][0]) {
									// ***** CLOCK MULTIPLIER *****
									divClockSample[t] = 1.0;
									edge[t] = true;
									divPulse[t] = true;
								}

								// ***** CLOCK MULTIPLIER/DIVIDER   PULSE WIDTH OFF
								if (divPulse[t] && divClockSample[t] > divMaxSample[t][0] * 0.5) {
									divPulse[t] = false;
									edge[t] = false;
								}

							} else {

								if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22 && divClockSample[t] > divMaxSample[t][divOddCounter[t]]) {
									// ***** CLOCK MULTIPLIER *****
									divPulseTime[t] = oneMsTime;
									divPulse[t] = true;
									edge[t] = true;
			
									if (divOddCounter[t] == 0) {
										divClockSample[t] = 1.0;
										divOddCounter[t] = 1;
									}	else {
										divClockSample[t] = 1.0 + divMaxSample[t][1] - divMaxSample[t][0];
										divOddCounter[t] = 0;
									}
								}
							}
						}
					}

					// ************************ EXTERNAL CLOCK DETECTION ******************

					if (extBeat[MC]) {

						if (extSync[MC]) {

							// ********** SYNCED BEAT
							
							for (int t = 0; t < MAXTRACKS; t++) {

								if (!extConn[t]) {

									if (!divSwing[t]) {

										if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22) {
											// ***** CLOCK MULTIPLIER *****
											divMaxSample[t][0] = extClockMaxSample / (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
											divMaxSample[t][1] = divMaxSample[t][0];
											divClockSample[t] = 1.0;
											divPulse[t] = true;
											edge[t] = true;
										} else {
											// ***** CLOCK DIVIDER *****
											divMaxSample[t][0] = extClockMaxSample * (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
											divMaxSample[t][1] = divMaxSample[t][1];
											divCount[t]++;
											if (divCount[t] > divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]) {
												divClockSample[t] = 1.0;
												divCount[t] = 1;
												divPulse[t] = true;
												edge[t] = true;
											}
										}
									} else {

										if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22) {
											// ***** CLOCK MULTIPLIER *****
											divMaxSample[t][0] = extClockMaxSample / (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
											divMaxSample[t][1] = divMaxSample[t][0] + (divMaxSample[t][0] * swingCalc(t));
											divOddCounter[t] = 1;
											divClockSample[t] = 1.0;
											divPulseTime[t] = oneMsTime;
											divPulse[t] = true;
											edge[t] = true;
										} else {
											// ***** CLOCK DIVIDER *****
											divMaxSample[t][0] = extClockMaxSample * (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
											divMaxSample[t][1] = divMaxSample[t][0];
											divCount[t]++;
											if (divCount[t] > divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]) {
												divOddCounter[t] = 1;
												divClockSample[t] = 1.0;
												divCount[t] = 1;
												divPulseTime[t] = oneMsTime;
												divPulse[t] = true;
												edge[t] = true;
											}
										}
									}
								}
							}
							
							if (!cvClockOut)
								outputs[CLOCK_OUTPUT].setVoltage(10.f);
						
						}
					}
				}

				extClockSample++;

				if (!internalClock)
					for (int t = 0; t < MAXTRACKS; t++)
						if (!extConn[t])
							divClockSample[t]++;

			}
				
		}

		// *********************************************************************************************
		// *********************************************************************************************
		// *********************************************************************************************
		// *********************************************************************************************
		// *********************************************************************************************
		
		// ********************************************************** SINGLE TRACKS WITH EXTERNAL CLOCK

		for (int t = 0; t < MAXTRACKS; t++) {
			extConn[t] = inputs[CLK_INPUT+t].isConnected();
			if (extConn[t] && !prevExtConn[t]) {
				extSync[t] = false;
				//bpm[t] = 0.0;
				//pulseNr[t] = 0;
			}
			prevExtConn[t] = extConn[t];

			if (extConn[t]) {

				extTrigValue[t] = inputs[CLK_INPUT+t].getVoltage();
					
				if (extTrigValue[t] >= 1 && prevExtTrigValue[t] < 1) {

					if (extSync[t]) {

							clockMaxSample[t] = clockSample[t];
							clockSample[t] = 0.0;

							extBeat[t] = true;

					} else {

						extSync[t] = true;
						clockSample[t] = 1.0;
						extBeat[t] = true;
					}

				} else {
					extBeat[t] = false;
				}
				prevExtTrigValue[t] = extTrigValue[t];

				// **************   RUN PROCESS   ***************  ON SINGLE TRACKS WITH EXTERNAL CLOCK

				if (resetPulseTime[MC] <= 0) {

					// ****** CLOCK PULSE WIDTH
					maxPulseSample[t] = clockMaxSample[t] * 0.5;		// ---------------------------------- DA OTTIMIZZARE
				
					// ************ DIV / MULT OLD OLD OLD

					if(!divSwing[t]) {

						if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22 && divClockSample[t] > divMaxSample[t][0]) {
							// ***** CLOCK MULTIPLIER *****
							divClockSample[t] = 1.0;
							divPulse[t] = true;
							edge[t] = true;
						}

						// ***** CLOCK MULTIPLIER/DIVIDER   PULSE WIDTH OFF
						if (divPulse[t] && divClockSample[t] > divMaxSample[t][0] * 0.5) {
							divPulse[t] = false;
							edge[t] = false;
						}

					} else {

						if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22 && divClockSample[t] > divMaxSample[t][divOddCounter[t]]) {
							// ***** CLOCK MULTIPLIER *****
							divPulseTime[t] = oneMsTime;
							divPulse[t] = true;
							edge[t] = true;

							if (divOddCounter[t] == 0) {
								divClockSample[t] = 1.0;
								divOddCounter[t] = 1;
							}	else {
								divClockSample[t] = 1.0 + divMaxSample[t][1] - divMaxSample[t][0];
								divOddCounter[t] = 0;
							}
						}
					}
				}

				// ************************ EXTERNAL CLOCK ************   ON SINGLE TRACKS WITH EXTERNAL CLOCK

				if (extBeat[t]) {

					if (extSync[t]) {

						// ********** SYNCED BEAT

						if (!divSwing[t]) {

							if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22) {
								// ***** CLOCK MULTIPLIER *****
								divMaxSample[t][0] = clockMaxSample[t] / (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
								divMaxSample[t][1] = divMaxSample[t][0];
								divClockSample[t] = 1.0;
								divPulse[t] = true;
								edge[t] = true;
							} else {
								// ***** CLOCK DIVIDER *****
								divMaxSample[t][0] = clockMaxSample[t] * (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
								divMaxSample[t][1] = divMaxSample[t][1];
								divCount[t]++;
								if (divCount[t] > divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]) {
									divClockSample[t] = 1.0;
									divCount[t] = 1;
									divPulse[t] = true;
									edge[t] = true;
								}
							}
						} else {

							if (params[DIVMULT_KNOB_PARAM+t].getValue() > 22) {
								// ***** CLOCK MULTIPLIER *****
								divMaxSample[t][0] = clockMaxSample[t] / (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
								divMaxSample[t][1] = divMaxSample[t][0] + (divMaxSample[t][0] * swingCalc(t));
								divOddCounter[t] = 1;
								divClockSample[t] = 1.0;
								divPulseTime[t] = oneMsTime;
								divPulse[t] = true;
								edge[t] = true;
							} else {
								// ***** CLOCK DIVIDER *****
								divMaxSample[t][0] = clockMaxSample[t] * (divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]);
								divMaxSample[t][1] = divMaxSample[t][0];
								divCount[t]++;
								if (divCount[t] > divMult[int(params[DIVMULT_KNOB_PARAM+t].getValue())]) {
									divOddCounter[t] = 1;
									divClockSample[t] = 1.0;
									divCount[t] = 1;
									divPulseTime[t] = oneMsTime;
									divPulse[t] = true;
									edge[t] = true;
								}
							}
						}
					}
				}

				clockSample[t]++;
				divClockSample[t]++;
				
			}
		}

		// ***************************** COMMON CLOCK PROCESS

		//	********** RESET AND DIV PULSES

		if (resetPulse[MC]) {
			resetPulseTime[MC]--;
			if (resetPulseTime[MC] < 0) {
				resetPulse[MC] = false;
				outputs[RST_OUTPUT].setVoltage(0.f);
			} else
				outputs[RST_OUTPUT].setVoltage(10.f);
		}
/*
		// SWING SWITCH OFF ON SINGLE TRACKS  		<--------------------------------- spostato poco sotto
		for (int t = 0; t < MAXTRACKS; t++) {
			if (divSwing[t] && divPulse[t]) {
				divPulseTime[t]--;
				if (divPulseTime[t] < 0) {
					divPulse[t] = false;
					edge[t] = false;
				} 
			}
		}
*/		


/*

																		░██████╗███████╗░██████╗░██╗░░░██╗███████╗███╗░░██╗░█████╗░███████╗██████╗░
																		██╔════╝██╔════╝██╔═══██╗██║░░░██║██╔════╝████╗░██║██╔══██╗██╔════╝██╔══██╗
																		╚█████╗░█████╗░░██║██╗██║██║░░░██║█████╗░░██╔██╗██║██║░░╚═╝█████╗░░██████╔╝
																		░╚═══██╗██╔══╝░░╚██████╔╝██║░░░██║██╔══╝░░██║╚████║██║░░██╗██╔══╝░░██╔══██╗
																		██████╔╝███████╗░╚═██╔═╝░╚██████╔╝███████╗██║░╚███║╚█████╔╝███████╗██║░░██║
																		╚═════╝░╚══════╝░░░╚═╝░░░░╚═════╝░╚══════╝╚═╝░░╚══╝░╚════╝░╚══════╝╚═╝░░╚═╝
*/
		
		clkBuffPos++;	
		if (clkBuffPos > 4)
			clkBuffPos = 0;
/*
 // SPOSTATO IN BLOCCO PIU' SOTTO
		for (int t = 0; t < MAXTRACKS; t++) {

			// SWING SWITCH OFF ON SINGLE TRACKS  		<--------------------------------- spostato DA SOPRA PER OTTIMIZZARE
			if (divSwing[t] && divPulse[t]) {
				divPulseTime[t]--;
				if (divPulseTime[t] < 0) {
					divPulse[t] = false;
					edge[t] = false;
				} 
			}

			// -------------------------------- CLOCK EDGE DETECTION and x SAMPLE DELAY
			if (edge[t] && !prevEdge[t]) {
				clockAdv[t] = true;
			}
			prevEdge[t] = edge[t];

			int tempDelay;
			if (sampleDelay[t] == 6)
				tempDelay = sampleDelay[MC];
			else
				tempDelay = sampleDelay[t];

			clkBuff[t][clkBuffPos] = clockAdv[t];

			delayReadPos = clkBuffPos - tempDelay;

			if (delayReadPos < 0)
				delayReadPos = 5 + clkBuffPos - tempDelay;

			if (clkBuff[t][delayReadPos])
				stepAdv[t] = true;
				//clockAdv[t] = true;

			// --------------------------------MODE SELECTION

			if (userInputs[t][IN_MODE][0]) {
				int tempUserIn = userInputs[t][IN_MODE][1];  // 0 oppure 8 a seconda che l'input sia U1 o U2
				float tempMode = inputs[USER_INPUT+t+tempUserIn].getVoltage() * 0.1;

				if (tempUserIn == 0) {
					if (userTable[t][1] == KNOB_ATN)
						tempMode *= params[USER_PARAM+t].getValue();
					else if (userTable[t][1] == KNOB_ATNV)
						tempMode *= params[USER_PARAM+t].getValue() * 2 - 1;
				} else { // altrimenti è 8
					if (userTable[t][3] == KNOB_ATN)
						tempMode *= params[USER_PARAM+t+8].getValue();
					else if (userTable[t][3] == KNOB_ATNV)
						tempMode *= params[USER_PARAM+t+8].getValue() * 2 - 1;
				}

				tempMode *= MAXMODES;

				if (userInputs[t][KNOB_MODE][0])
					tempMode += params[USER_PARAM+t+userInputs[t][KNOB_MODE][1]].getValue() * MAXMODES;

				if (tempMode < 0)
					tempMode = 0;
				else if (tempMode > MAXMODES)
					tempMode = MAXMODES;

				currentMode[t] = tempMode;

			} else if (userInputs[t][KNOB_MODE][0]) {
				currentMode[t] = params[USER_PARAM+t+userInputs[t][KNOB_MODE][1]].getValue() * MAXMODES;
			}

			out[t] = 0.f;
		}
*/

	/*
		if (turingMode && !prevTuringMode)
			for (int t = 0; t < 8; t++)
				calcVoltage(t);

		prevTuringMode = turingMode;
	*/

		// *******************SETUP SEQUENCER RUN

		if (inputs[SEQRUN_INPUT].isConnected()) {
			seqRunTrig = inputs[SEQRUN_INPUT].getVoltage();
			if (runType == RUN_GATE) {
				if (seqRunTrig >= 1.f) {
					seqRunSetting = 1;
				} else {
					seqRunSetting = 0;
				}
			} else {	// runType == RUN_TRIG
				seqButtRunTrig = params[SEQRUN_PARAM].getValue();
				if ((seqRunTrig >= 1 && prevSeqRunTrig < 1.f) || (seqButtRunTrig >= 1.f && prevSeqButtRunTrig < 1.f)) {
					if (seqRunSetting) {
						seqRunSetting = 0;
					} else {
						seqRunSetting = 1;
					}			
				}
				prevSeqButtRunTrig = seqButtRunTrig;
			}
			prevSeqRunTrig = seqRunTrig;
		} else {
			seqButtRunTrig = params[SEQRUN_PARAM].getValue();
			if (seqButtRunTrig && !prevSeqButtRunTrig) {
				if (seqRunSetting) {
					seqRunSetting = 0;
				} else {
					seqRunSetting = 1;
				}			
			}
			prevSeqButtRunTrig = seqButtRunTrig;
		}

		if (seqRunSetting && !prevSeqRunSetting) {
			for (int t = 0; t < MAXTRACKS; t++)
				if (rstStepsWhen[t] == RST_ONRUN || (rstStepsWhen[t] == RST_DEFAULT && rstStepsWhen[MC] == RST_ONRUN))
					resetTrackSteps(t);
		} else if (!seqRunSetting && prevSeqRunSetting) {
			for (int t = 0; t < MAXTRACKS; t++)
				if (rstStepsWhen[t] == RST_ONSTOP || (rstStepsWhen[t] == RST_DEFAULT && rstStepsWhen[MC] == RST_ONSTOP))
					resetTrackSteps(t);
		}
		prevSeqRunSetting = seqRunSetting;

		lights[SEQRUN_LIGHT].setBrightness(seqRunSetting);

		for (int t = 0; t < MAXTRACKS; t++) {

// -------------- INIZIO INIT TRACK (SPOSTATO DA SOPRA)
			// SWING SWITCH OFF ON SINGLE TRACKS  		<--------------------------------- spostato DA SOPRA PER OTTIMIZZARE
			if (divSwing[t] && divPulse[t]) {
				divPulseTime[t]--;
				if (divPulseTime[t] < 0) {
					divPulse[t] = false;
					edge[t] = false;
				} 
			}

			// -------------------------------- CLOCK EDGE DETECTION and x SAMPLE DELAY
			if (edge[t] && !prevEdge[t]) {
				clockAdv[t] = true;
			}
			prevEdge[t] = edge[t];

			int tempDelay;
			if (sampleDelay[t] == 6)
				tempDelay = sampleDelay[MC];
			else
				tempDelay = sampleDelay[t];

			clkBuff[t][clkBuffPos] = clockAdv[t];

			delayReadPos = clkBuffPos - tempDelay;

			if (delayReadPos < 0)
				delayReadPos = 5 + clkBuffPos - tempDelay;

			if (clkBuff[t][delayReadPos])
				stepAdv[t] = true;

			// --------------------------------MODE SELECTION
 
			if (userInputs[t][IN_MODE][0] && inputs[USER_INPUT+t+userInputs[t][IN_MODE][1]].isConnected()) {
				int tempUserIn = userInputs[t][IN_MODE][1];  // 0 oppure 8 a seconda che l'input sia U1 o U2
				float tempMode = inputs[USER_INPUT+t+tempUserIn].getVoltage() * 0.1;

				if (tempUserIn == 0) {
					if (userTable[t][1] == KNOB_ATN)
						tempMode *= params[USER_PARAM+t].getValue();
					else if (userTable[t][1] == KNOB_ATNV)
						tempMode *= params[USER_PARAM+t].getValue() * 2 - 1;
				} else { // altrimenti è 8
					if (userTable[t][3] == KNOB_ATN)
						tempMode *= params[USER_PARAM+t+8].getValue();
					else if (userTable[t][3] == KNOB_ATNV)
						tempMode *= params[USER_PARAM+t+8].getValue() * 2 - 1;
				}

				tempMode *= MAXMODES;

				if (userInputs[t][KNOB_MODE][0])
					tempMode += params[USER_PARAM+t+userInputs[t][KNOB_MODE][1]].getValue() * MAXMODES;

				if (tempMode < 0)
					tempMode = 0;
				else if (tempMode > MAXMODES)
					tempMode = MAXMODES;

				currentMode[t] = tempMode;

			} else if (userInputs[t][KNOB_MODE][0]) {
				currentMode[t] = params[USER_PARAM+t+userInputs[t][KNOB_MODE][1]].getValue() * MAXMODES;
			}

			out[t] = 0.f;

			// -------------- FINE INIT TRACK (SPOSTATO DA SOPRA)

			// CHECK TRACK RESET

			rstValue[t] = inputs[RST_INPUT+t].getVoltage();
			if (rstValue[t] >= 1.f && prevRstValue[t] < 1.f && currentMode[t] != CVOLTAGE)
				resetTrackSteps(t);
			prevRstValue[t] = rstValue[t];

			// GET SEQUENCE LENGTH 

			maxSteps[t] = wSteps[t];

			if (userInputs[t][IN_LENGTH][0]) {

				if (inputs[USER_INPUT+t+userInputs[t][IN_LENGTH][1]].isConnected()) {

					float stepsIn = inputs[USER_INPUT+t+userInputs[t][IN_LENGTH][1]].getVoltage();

					int tempUser = userInputs[t][IN_LENGTH][1];  // 0 oppure 8 a seconda che l'input sia U1 o U2

					if (tempUser == 0) {
						if (userTable[t][1] == KNOB_ATN)
							stepsIn *= params[USER_PARAM+t].getValue();
						else if (userTable[t][1] == KNOB_ATNV)
							stepsIn *= params[USER_PARAM+t].getValue() * 2 - 1;
					} else if (tempUser == 8) {
						if (userTable[t][3] == KNOB_ATN)
							stepsIn *= params[USER_PARAM+t+8].getValue();
						else if (userTable[t][3] == KNOB_ATNV)
							stepsIn *= params[USER_PARAM+t+8].getValue() * 2 - 1;
					}

					if (stepsIn < 0.f)
						stepsIn = 0.f;
					else if (stepsIn > 10.f)
						stepsIn = 10.f;

					int addSteps = int(stepsIn / 10 * (16 - maxSteps[t]));

					maxSteps[t] += addSteps;
					if (maxSteps[t] > 16)
						maxSteps[t] = 16;
				}
			}

			// SETUP DIRECTION

			if (currentMode[t] != CVOLTAGE) {

				if (stepAdv[t] && (seqRunSetting == 1 || xcludeFromRun[t])) {
					if (!userInputs[t][IN_RUN][0] ||
							(userInputs[t][IN_RUN][0] &&
								(!inputs[USER_INPUT+t+userInputs[t][IN_RUN][1]].isConnected() || (inputs[USER_INPUT+t+userInputs[t][IN_RUN][1]].isConnected() && inputs[USER_INPUT+t+userInputs[t][IN_RUN][1]].getVoltage() >= 1.f))
							)
						) {

						// ********************************************************************** TO DO: CHECK USER RUN INPUT

						lights[STEP_LIGHT + (t*16+step[t])].setBrightness(0);
										
						switch (currentMode[t]) {

							case FORWARD:

								if (userInputs[t][IN_REV][0]) {
									/*
									if (revType[t] == 2) {
										if (revType[MC] == POSITIVE_V) {
											if (inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage() < 1)
												direction[t] = FORWARD;
											else
												direction[t] = REVERSE;
										} else {
											if (inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage() < -1)
												direction[t] = REVERSE;
											else
												direction[t] = FORWARD;
										}
									} else if (revType[t] == POSITIVE_V) {
										if (inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage() < 1)
											direction[t] = FORWARD;
										else
											direction[t] = REVERSE;
									} else {
										if (inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage() < -1)
											direction[t] = REVERSE;
										else
											direction[t] = FORWARD;
									}
									*/
									if (inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage() < 1)
										direction[t] = FORWARD;
									else
										direction[t] = REVERSE;

								} else {
									direction[t] = FORWARD;
								}

								if (direction[t] == FORWARD) {

									if (!dontAdvance[t])
										stepForward(t);
									else
										dontAdvance[t] = false;

									if (step[t] >= maxSteps[t])
										step[t] = 0;

								} else {

									if (!dontAdvance[t])
										stepBack(t);
									else
										dontAdvance[t] = false;

									if (step[t] < 0)
										step[t] = maxSteps[t] - 1;

								}
			/*
								if (!turingMode) {
									for (int t = 0; t < MAXTRACKS; t++) {
										if (wSeq[t][step]) {
											stepPulse[t] = true;
											stepPulseTime[t] = oneMsTime;
											if (outType == OUT_GATE)
												outGate[t] = true;
										} else {
											if (outType == OUT_GATE) {
												outGate[t] = false;
												out[t] = 0.f;
											}
										}
									}
								} else {
									for (int t = 0; t < MAXTRACKS; t++)
										calcVoltage(t);
								}
			*/
								
							break;

							case REVERSE:
							
								if (userInputs[t][IN_REV][0]) {
									/*
									if (revType[t] == 2) {
										if (revType[MC] == POSITIVE_V) {
											if (inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage() < 1)
												direction[t] = REVERSE;
											else
												direction[t] = FORWARD;
										} else {
											if (inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage() < -1)
												direction[t] = FORWARD;
											else
												direction[t] = REVERSE;
										}
									} else if (revType[t] == POSITIVE_V) {
										if (inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage() < 1)
											direction[t] = REVERSE;
										else
											direction[t] = FORWARD;
									} else {
										if (inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage() < -1)
											direction[t] = FORWARD;
										else
											direction[t] = REVERSE;
									}
									*/
									if (inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage() < 1)
										direction[t] = REVERSE;
									else
										direction[t] = FORWARD;
								} else {
									direction[t] = REVERSE;
								}

								if (direction[t] == FORWARD) {

									if (!dontAdvance[t])
										stepForward(t);
									else
										dontAdvance[t] = false;

									if (step[t] >= maxSteps[t])
										step[t] = 0;

								} else {

									if (!dontAdvance[t])
										stepBack(t);
									else
										dontAdvance[t] = false;

									if (step[t] < 0)
										step[t] = maxSteps[t] - 1;

								}

							
							break;

							case PINGPONG:

								if (userInputs[t][IN_REV][0]) {
									revVolt[t] = inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage();
									/*
									if (revType[t] == 2) {
										if (revType[MC] == POSITIVE_V) {
											if ((revVolt[t] >= 1 && prevRevVolt[t] < 1) || (revVolt[t] < 1 && prevRevVolt[t] >= 1))
												direction[t] = !direction[t];
										} else {
											if ((revVolt[t] <= -1 && prevRevVolt[t] > -1) || (revVolt[t] > -1 && prevRevVolt[t] <= -1))
												direction[t] = !direction[t];
										}
									} else if (revType[t] == POSITIVE_V) {
										if ((revVolt[t] >= 1 && prevRevVolt[t] < 1) || (revVolt[t] < 1 && prevRevVolt[t] >= 1))
											direction[t] = !direction[t];
									} else {
										if ((revVolt[t] <= -1 && prevRevVolt[t] > -1) || (revVolt[t] > -1 && prevRevVolt[t] <= -1))
											direction[t] = !direction[t];
									}
									*/
									if ((revVolt[t] >= 1 && prevRevVolt[t] < 1) || (revVolt[t] < 1 && prevRevVolt[t] >= 1))
												direction[t] = !direction[t];

									prevRevVolt[t] = revVolt[t];
								}

								if (direction[t] == FORWARD) {

									if (!dontAdvance[t])
										stepForward(t);
									else
										dontAdvance[t] = false;

									if (step[t] >= maxSteps[t]) {
										direction[t] = REVERSE;
										step[t] = step[t] - 2;
										if (step[t] < 0) {
											step[t] = 0;
											direction[t] = FORWARD;
										}
									}

								} else {

									if (!dontAdvance[t])
										stepBack(t);
									else
										dontAdvance[t] = false;

									if (step[t] < 0) {
										direction[t] = FORWARD;
										step[t] = 1;
										if (step[t] >= maxSteps[t]) {
											step[t] = 0;
											direction[t] = REVERSE;
										}

									}

								}
							
							break;

							case PINGPONGEXT:
							
								if (userInputs[t][IN_REV][0]) {
									revVolt[t] = inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage();
									/*
									if (revType[t] == 2) {
										if (revType[MC] == POSITIVE_V) {
											if ((revVolt[t] >= 1 && prevRevVolt[t] < 1) || (revVolt[t] < 1 && prevRevVolt[t] >= 1))
												direction[t] = !direction[t];
										} else {
											if ((revVolt[t] <= -1 && prevRevVolt[t] > -1) || (revVolt[t] > -1 && prevRevVolt[t] <= -1))
												direction[t] = !direction[t];
										}
									} else if (revType[t] == POSITIVE_V) {
										if ((revVolt[t] >= 1 && prevRevVolt[t] < 1) || (revVolt[t] < 1 && prevRevVolt[t] >= 1))
											direction[t] = !direction[t];
									} else {
										if ((revVolt[t] <= -1 && prevRevVolt[t] > -1) || (revVolt[t] > -1 && prevRevVolt[t] <= -1))
											direction[t] = !direction[t];
									}
									*/
									if ((revVolt[t] >= 1 && prevRevVolt[t] < 1) || (revVolt[t] < 1 && prevRevVolt[t] >= 1))
										direction[t] = !direction[t];
									prevRevVolt[t] = revVolt[t];
								}

								if (direction[t] == FORWARD) {

									if (!dontAdvance[t])
										stepForward(t);
									else
										dontAdvance[t] = false;

									if (step[t] >= maxSteps[t]) {
										direction[t] = REVERSE;
										step[t]--;
										//stepBack(t);
										if (step[t] < 0)
											step[t] = 0;
									}

								} else {

									if (!dontAdvance[t])
										stepBack(t);
									else
										dontAdvance[t] = false;

									if (step[t] < 0) {
										direction[t] = FORWARD;
										step[t]++;
										//stepForward(t);
										if (step[t] >= maxSteps[t])
											step[t] = maxSteps[t] -1;
									}

								}

							break;

							case RANDOM1:
								if (userInputs[t][IN_REV][0]) {
									revVolt[t] = inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage();
									/*
									if (revType[t] == 2) {
										if (revType[MC] == POSITIVE_V) {
											if (revVolt[t] < 1) {
												int tempStep = step[t];
												step[t] = maxSteps[t] * random::uniform();
												if (step[t] == tempStep) {
													step[t] = maxSteps[t] * random::uniform();
													if (step[t] == tempStep)
														step[t] = maxSteps[t] * random::uniform();
												}
												if (step[t] >= maxSteps[t])
													maxSteps[t] = maxSteps[t] - 1;
											} else {
												if (!dontAdvance[t])
													stepForward(t);
												else
													dontAdvance[t] = false;
												if (step[t] >= maxSteps[t])
													step[t] = 0;
											}
										} else {
											if (revVolt[t] > -1) {
												int tempStep = step[t];
												step[t] = maxSteps[t] * random::uniform();
												if (step[t] == tempStep) {
													step[t] = maxSteps[t] * random::uniform();
													if (step[t] == tempStep)
														step[t] = maxSteps[t] * random::uniform();
												}
												if (step[t] >= maxSteps[t])
													maxSteps[t] = maxSteps[t] - 1;
											} else {
												if (!dontAdvance[t])
													stepForward(t);
												else
													dontAdvance[t] = false;
												if (step[t] >= maxSteps[t])
													step[t] = 0;
											}
										}
									} else if (revType[t] == POSITIVE_V) {
										if (revVolt[t] < 1) {
											int tempStep = step[t];
											step[t] = maxSteps[t] * random::uniform();
											if (step[t] == tempStep) {
												step[t] = maxSteps[t] * random::uniform();
												if (step[t] == tempStep)
													step[t] = maxSteps[t] * random::uniform();
											}
											if (step[t] >= maxSteps[t])
												maxSteps[t] = maxSteps[t] - 1;
										} else {
											if (!dontAdvance[t])
												stepForward(t);
											else
												dontAdvance[t] = false;
											if (step[t] >= maxSteps[t])
												step[t] = 0;
										}
									} else {
										if (revVolt[t] > -1) {
											int tempStep = step[t];
											step[t] = maxSteps[t] * random::uniform();
											if (step[t] == tempStep) {
												step[t] = maxSteps[t] * random::uniform();
												if (step[t] == tempStep)
													step[t] = maxSteps[t] * random::uniform();
											}
											if (step[t] >= maxSteps[t])
												maxSteps[t] = maxSteps[t] - 1;
										} else {
											if (!dontAdvance[t])
												stepForward(t);
											else
												dontAdvance[t] = false;
											if (step[t] >= maxSteps[t])
												step[t] = 0;
										}
									}
									*/
									if (revVolt[t] < 1) {
										int tempStep = step[t];
										step[t] = maxSteps[t] * random::uniform();
										if (step[t] == tempStep) {
											step[t] = maxSteps[t] * random::uniform();
											if (step[t] == tempStep)
												step[t] = maxSteps[t] * random::uniform();
										}
										if (step[t] >= maxSteps[t])
											maxSteps[t] = maxSteps[t] - 1;
									} else {
										if (!dontAdvance[t])
											stepForward(t);
										else
											dontAdvance[t] = false;
										if (step[t] >= maxSteps[t])
											step[t] = 0;
									}

								} else {
									int tempStep = step[t];
									step[t] = maxSteps[t] * random::uniform();
									if (step[t] == tempStep) {
										step[t] = maxSteps[t] * random::uniform();
										if (step[t] == tempStep)
											step[t] = maxSteps[t] * random::uniform();
									}
									if (step[t] >= maxSteps[t])
										maxSteps[t] = maxSteps[t] - 1;
								}
							break;

							case RANDOM2:
								if (userInputs[t][IN_REV][0]) {
									revVolt[t] = inputs[USER_INPUT+t+userInputs[t][IN_REV][1]].getVoltage();
									/*
									if (revType[t] == 2) {
										if (revType[MC] == POSITIVE_V) {
											if (revVolt[t] < 1) {
												if (!dontAdvance[t])
													stepForward(t);
												else
													dontAdvance[t] = false;
												if (step[t] >= maxSteps[t])
													step[t] = 0;
											} else {
												int tempStep = step[t];
												step[t] = maxSteps[t] * random::uniform();
												if (step[t] == tempStep) {
													step[t] = maxSteps[t] * random::uniform();
													if (step[t] == tempStep)
														step[t] = maxSteps[t] * random::uniform();
												}
												if (step[t] >= maxSteps[t])
													maxSteps[t] = maxSteps[t] - 1;
											}
										} else {
											if (revVolt[t] > -1) {
												if (!dontAdvance[t])
													stepForward(t);
												else
													dontAdvance[t] = false;
												if (step[t] >= maxSteps[t])
													step[t] = 0;
											} else {
												int tempStep = step[t];
												step[t] = maxSteps[t] * random::uniform();
												if (step[t] == tempStep) {
													step[t] = maxSteps[t] * random::uniform();
													if (step[t] == tempStep)
														step[t] = maxSteps[t] * random::uniform();
												}
												if (step[t] >= maxSteps[t])
													maxSteps[t] = maxSteps[t] - 1;
											}
										}
									} else if (revType[t] == POSITIVE_V) {
										if (revVolt[t] < 1) {
											if (!dontAdvance[t])
												stepForward(t);
											else
												dontAdvance[t] = false;
											if (step[t] >= maxSteps[t])
												step[t] = 0;
										} else {
											int tempStep = step[t];
											step[t] = maxSteps[t] * random::uniform();
											if (step[t] == tempStep) {
												step[t] = maxSteps[t] * random::uniform();
												if (step[t] == tempStep)
													step[t] = maxSteps[t] * random::uniform();
											}
											if (step[t] >= maxSteps[t])
												maxSteps[t] = maxSteps[t] - 1;
										}
									} else {
										if (revVolt[t] > -1) {
											if (!dontAdvance[t])
												stepForward(t);
											else
												dontAdvance[t] = false;
											if (step[t] >= maxSteps[t])
												step[t] = 0;
										} else {
											int tempStep = step[t];
											step[t] = maxSteps[t] * random::uniform();
											if (step[t] == tempStep) {
												step[t] = maxSteps[t] * random::uniform();
												if (step[t] == tempStep)
													step[t] = maxSteps[t] * random::uniform();
											}
											if (step[t] >= maxSteps[t])
												maxSteps[t] = maxSteps[t] - 1;
										}
									}
									*/
									if (revVolt[t] < 1) {
										if (!dontAdvance[t])
											stepForward(t);
										else
											dontAdvance[t] = false;
										if (step[t] >= maxSteps[t])
											step[t] = 0;
									} else {
										int tempStep = step[t];
										step[t] = maxSteps[t] * random::uniform();
										if (step[t] == tempStep) {
											step[t] = maxSteps[t] * random::uniform();
											if (step[t] == tempStep)
												step[t] = maxSteps[t] * random::uniform();
										}
										if (step[t] >= maxSteps[t])
											maxSteps[t] = maxSteps[t] - 1;
									}
								} else {
									if (!dontAdvance[t])
										stepForward(t);
									else
										dontAdvance[t] = false;
									if (step[t] >= maxSteps[t])
										step[t] = 0;
								}
							break;
						}

						alreadyChanged[t] = false;

					}
				}
			} else {	// CONTROL VOLTAGE ADVANCE

				clkValue[t] = inputs[CLK_INPUT+t].getVoltage();

				if (clkValue[t] > 10.f)
					clkValue[t] = 10.f;
				else if (clkValue[t] < 0.f)
					clkValue[t] = 0.f;

				if (clkValue[t] != prevClkValue[t]) {
					
					currAddr[t] = 1+int(clkValue[t] / 10 * (maxSteps)[t]);
					if (currAddr[t] >= maxSteps[t])
						currAddr[t] = maxSteps[t];
					if (currAddr[t] != prevAddr[t]) {
						/*
						//debug
						if (step[t] >= 16 || step[t] < 0)
							DEBUG("step[t] out of bounds: step=%i, t=%i", step[t], t);
						*/
						lights[STEP_LIGHT+(t*16+step[t])].setBrightness(0);
						step[t] = currAddr[t]-1;

						alreadyChanged[t] = false;
/*
						if (!turingMode) {
							for (int t = 0; t < MAXTRACKS; t++) {
								if (wSeq[t][step]) {
									stepPulse[t] = true;
									stepPulseTime[t] = oneMsTime;
									if (outType == OUT_GATE)
										outGate[t] = true;
								} else {
									if (outType == OUT_GATE) {
										outGate[t] = false;
										out[t] = 0.f;
									}
								}
							}
						} else {
							for (int t = 0; t < MAXTRACKS; t++) 
								calcVoltage(t);
						}
*/
						prevAddr[t] = currAddr[t];

					}
				}
				prevClkValue[t] = clkValue[t];

			}

			calcChange(t);

			out[t] = wSeq[t][step[t]];

			int tempRange = range[MC];

			if (range[t] != 10)
				tempRange = range[t];

			switch (tempRange) {
				case 0:	break;
				case 1:	out[t] *= 2;	break;
				case 2:	out[t] *= 3;	break;
				case 3:	out[t] *= 5;	break;
				case 4:	out[t] *= 10;	break;
				case 5:	out[t] = (out[t] * 2) - 1;	break;
				case 6:	out[t] = (out[t] * 4) - 2;	break;
				case 7:	out[t] = (out[t] * 6) - 3;	break;
				case 8:	out[t] = (out[t] * 10) - 5;	break;
				case 9:	out[t] = (out[t] * 20) - 10;	break;
			}

			out[t] = outScale(out[t], t);

			outputs[OUT_OUTPUT+t].setVoltage(out[t]);

			lights[STEP_LIGHT+(t*16+step[t])].setBrightness(1);

		}
	
	}
};

/*

																													███████╗███╗░░██╗██████╗░
																													██╔════╝████╗░██║██╔══██╗
																													█████╗░░██╔██╗██║██║░░██║
																													██╔══╝░░██║╚████║██║░░██║
																													███████╗██║░╚███║██████╔╝
																													╚══════╝╚═╝░░╚══╝╚═════╝░
*/
struct StepStationDisplay : TransparentWidget {
	StepStation *module;
	int frame = 0;
	StepStationDisplay() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/vgatrue.ttf"));
				
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				std::string currentDisplay;

				currentDisplay = to_string(module->workingProg);

				if (!module->progChanged) {
					nvgFillColor(args.vg, nvgRGB(COLOR_LCD_GREEN));
					nvgFontSize(args.vg, 32);
					if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 8, 31, 80, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 15, 31, 80, currentDisplay.c_str(), NULL);

				} else {
					nvgFillColor(args.vg, nvgRGB(COLOR_LCD_GREEN));
					nvgFontSize(args.vg, 26);
					if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 6, 21, 80, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 12, 21, 80, currentDisplay.c_str(), NULL);
					
					currentDisplay = to_string(module->selectedProg);

					nvgFillColor(args.vg, nvgRGB(COLOR_LCD_RED));
					nvgFontSize(args.vg, 20);
					if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 20, 36, 60, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 25, 36, 60, currentDisplay.c_str(), NULL);
				}
				
			}
		}
		Widget::drawLayer(args, layer);
	}
};


struct StepStationDisplayTempo : TransparentWidget {
	StepStation *module;
	int frame = 0;
	StepStationDisplayTempo() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 13);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				
				int tempBpmInteger;
				std::string tempBpm;
				std::string tempBpmInt;
				std::string tempBpmDec;

				if (module->internalClock || module->externalClock) {
					if (!module->extConn[MC]) {
						tempBpmInteger = int(module->bpm[MC] * 10 + .5);

						tempBpm = to_string(tempBpmInteger);
						tempBpmInt = tempBpm.substr(0, tempBpm.size()-1);
						tempBpmDec = tempBpm.substr(tempBpm.size() - 1);
						tempBpm = tempBpmInt+"."+tempBpmDec;

						nvgFillColor(args.vg, nvgRGB(BPM_YELLOW)); 
						if (tempBpmInteger < 1000)
							nvgTextBox(args.vg, 14.5, 16.3, 60, tempBpm.c_str(), NULL);
						else
							nvgTextBox(args.vg, 4, 16.3, 60, tempBpm.c_str(), NULL);
					} else {
						if (!module->cvClockIn) {
							tempBpmInteger = int(module->extBpm);
							
							tempBpm = to_string(tempBpmInteger)+".X";

							nvgFillColor(args.vg, nvgRGB(BPM_BLUE)); 
							if (tempBpmInteger < 100)
								nvgTextBox(args.vg, 14.5, 16.3, 60, tempBpm.c_str(), NULL);
							else
								nvgTextBox(args.vg, 4, 16.3, 60, tempBpm.c_str(), NULL);
						} else {
							tempBpmInteger = int(module->extBpm * 10 + .5);

							tempBpm = to_string(tempBpmInteger);
							tempBpmInt = tempBpm.substr(0, tempBpm.size()-1);
							tempBpmDec = tempBpm.substr(tempBpm.size() - 1);
							tempBpm = tempBpmInt+"."+tempBpmDec;

							nvgFillColor(args.vg, nvgRGB(BPM_GREEN)); 
							if (tempBpmInteger < 1000)
								nvgTextBox(args.vg, 14.5, 16.3, 60, tempBpm.c_str(), NULL);
							else
								nvgTextBox(args.vg, 4, 16.3, 60, tempBpm.c_str(), NULL);
						}
					}
				} else {
					tempBpm = "---.-";
					nvgFillColor(args.vg, nvgRGB(BPM_YELLOW)); 
					nvgTextBox(args.vg, 4, 16.3, 60, tempBpm.c_str(), NULL);
				}
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct StepStationDisplayDiv : TransparentWidget {
	StepStation *module;
	int frame = 0;
	int t;

	StepStationDisplayDiv(int tIndex) : t(tIndex) {
	}

	void onButton(const event::Button &e) override {

		if (module->divControls) {

			if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
				int tempValue = int(module->params[module->DIVMULT_KNOB_PARAM + t].getValue());
				if (tempValue + 1 <= 44)
					tempValue++;
				module->params[module->DIVMULT_KNOB_PARAM + t].setValue(tempValue);
				e.consume(this);
			}
			if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
				int tempValue = int(module->params[module->DIVMULT_KNOB_PARAM + t].getValue());
				if (tempValue - 1 >= 0)
					tempValue--;
				module->params[module->DIVMULT_KNOB_PARAM + t].setValue(tempValue);
				e.consume(this);
			}

		} else {

			if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
				createContextMenu();
				e.consume(this);
			}
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module && layer == 1) {
			shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/vgatrue.ttf"));
			nvgFontSize(args.vg, 16);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);

			int tempValue = int(module->params[module->DIVMULT_KNOB_PARAM + t].getValue());
			float tempXpos = (tempValue > 1 && tempValue < 43) ? 10 : 6;

			nvgFillColor(args.vg, (tempValue < 22) ?
				nvgRGB(COLOR_LCD_RED) :
				nvgRGB(COLOR_LCD_GREEN));

			nvgTextBox(args.vg, tempXpos, 15.1, 60, module->divMultDisplay[tempValue].c_str(), NULL);
		}
		Widget::drawLayer(args, layer);
	}
	
	void createContextMenu() {
		StepStation *module = dynamic_cast<StepStation *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			static const std::string menuNames[45] = {
				"/256", "/128", "/64", "/48", "/32", "/24", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "x1",
				"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x24", "x32", "x48", "x64", "x128", "x256"
			};

			struct ThisItem : MenuItem {
				StepStation* module;
				int valueNr;
				int t;
				void onAction(const event::Action& e) override {
					module->params[module->DIVMULT_KNOB_PARAM + t].setValue(float(valueNr));
				}
			};

			for (int i = 0; i < 45; i++) {
				ThisItem* thisItem = createMenuItem<ThisItem>(menuNames[i]);
				thisItem->rightText = CHECKMARK(int(module->params[module->DIVMULT_KNOB_PARAM + t].getValue()) == i);
				thisItem->module = module;
				thisItem->valueNr = i;
				thisItem->t = t;
				menu->addChild(thisItem);
			}
		}
	}
	
};

struct StepStationDisplayMode : TransparentWidget {
	StepStation *module;
	int frame = 0;
	int t;

	StepStationDisplayMode(int tIndex) : t(tIndex) {
	}

	void onButton(const event::Button &e) override {

		if (module->modeControls) {

			if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {

				if (
					(module->userInputs[t][IN_MODE][0] && module->inputs[module->USER_INPUT+t+module->userInputs[t][IN_MODE][1]].isConnected() ) ||
					(module->userInputs[t][IN_MODE][0] && module->inputs[module->USER_INPUT+t+module->userInputs[t][IN_MODE][1]].isConnected() && !module->userInputs[t][KNOB_MODE][0] ) ) {
				} else {

					if (module->userInputs[t][KNOB_MODE][0]) {
						module->currentMode[t]--;
						if (module->currentMode[t] < 0)
							module->currentMode[t] = MAXMODES;
						module->params[module->USER_PARAM+t+module->userInputs[t][KNOB_MODE][1]].setValue((float)module->currentMode[t] / MAXMODES);
					} else {
						module->currentMode[t]--;
						if (module->currentMode[t] < 0)
							module->currentMode[t] = MAXMODES;
					}
				}

				e.consume(this);
			}

			if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {

				if (
					(module->userInputs[t][IN_MODE][0] && module->inputs[module->USER_INPUT+t+module->userInputs[t][IN_MODE][1]].isConnected() ) ||
					(module->userInputs[t][IN_MODE][0] && module->inputs[module->USER_INPUT+t+module->userInputs[t][IN_MODE][1]].isConnected() && !module->userInputs[t][KNOB_MODE][0] ) ) {
				} else {

					if (module->userInputs[t][KNOB_MODE][0]) {
						module->currentMode[t]++;
						if (module->currentMode[t] > MAXMODES)
							module->currentMode[t] = 0;
						module->params[module->USER_PARAM+t+module->userInputs[t][KNOB_MODE][1]].setValue((float)module->currentMode[t] / MAXMODES);
					} else {
						module->currentMode[t]++;
						if (module->currentMode[t] > MAXMODES)
							module->currentMode[t] = 0;
					}
				}

				e.consume(this);
			}
		} else {
			/*
			if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {

				if (
					(module->userInputs[t][IN_MODE][0] && module->inputs[module->USER_INPUT+t+module->userInputs[t][IN_MODE][1]].isConnected() ) ||
					(module->userInputs[t][IN_MODE][0] && module->inputs[module->USER_INPUT+t+module->userInputs[t][IN_MODE][1]].isConnected() && !module->userInputs[t][KNOB_MODE][0] ) ) {
				} else {

					if (module->userInputs[t][KNOB_MODE][0]) {
						module->currentMode[t]++;
						if (module->currentMode[t] > MAXMODES)
							module->currentMode[t] = 0;
						module->params[module->USER_PARAM+t+module->userInputs[t][KNOB_MODE][1]].setValue((float)module->currentMode[t] / MAXMODES);
					} else {
						module->currentMode[t]++;
						if (module->currentMode[t] > MAXMODES)
							module->currentMode[t] = 0;
					}
				}

				e.consume(this);
			}
			*/

			if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
					createContextMenu();
					e.consume(this);
			}
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module && layer == 1) {
			shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/vgatrue.ttf"));
			nvgFontSize(args.vg, 16);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);

			switch (module->currentMode[t]) {
				case 0: nvgFillColor(args.vg, nvgRGB(COLOR_LCD_GREEN)); break;
				case 1: nvgFillColor(args.vg, nvgRGB(COLOR_LCD_YELLOW)); break;
				case 2: nvgFillColor(args.vg, nvgRGB(COLOR_LCD_RED)); break;
				case 3: nvgFillColor(args.vg, nvgRGB(COLOR_LCD_VIOLET)); break;
				case 4: nvgFillColor(args.vg, nvgRGB(COLOR_LCD_LBLUE)); break;
				case 5: nvgFillColor(args.vg, nvgRGB(COLOR_LCD_ROSE)); break;
				case 6: nvgFillColor(args.vg, nvgRGB(COLOR_LCD_GREEN)); break;
				default: nvgFillColor(args.vg, nvgRGB(COLOR_LCD_GREEN)); break;
			}

			nvgTextBox(args.vg, 3.5, 15.3f, 60, module->modeNameDisplay[module->currentMode[t]].c_str(), NULL);

		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		StepStation *module = dynamic_cast<StepStation *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ModeTypeItem : MenuItem {
				StepStation* module;
				int menuValue;
				int t;

				ModeTypeItem(StepStation* m, int value, int track) {
					module = m;
					menuValue = value;
					t = track;
					text = module->modeNames[value];
				}

				void onAction(const event::Action& e) override {
					
					if (module->userInputs[t][KNOB_MODE][0])
							module->params[module->USER_PARAM+t+module->userInputs[t][KNOB_MODE][1]].setValue((float)menuValue/MAXMODES);
					module->currentMode[t] = menuValue;
				}
			};

			if (	(module->userInputs[t][IN_MODE][0] && module->inputs[module->USER_INPUT+t+module->userInputs[t][IN_MODE][1]].isConnected() ) ||
					(module->userInputs[t][IN_MODE][0] && module->inputs[module->USER_INPUT+t+module->userInputs[t][IN_MODE][1]].isConnected() && !module->userInputs[t][KNOB_MODE][0] ) ) {
				for (int i = 0; i < MAXMODES+1; i++)
					menu->addChild(createMenuLabel(module->modeNames[i]));
			} else {

				
				for (int i = 0; i < MAXMODES+1; i++) {
					auto modeTypeItem = new ModeTypeItem(module, i, t);
					modeTypeItem->rightText = CHECKMARK(module->currentMode[t] == i);
					menu->addChild(modeTypeItem);
				}
				
			}
		}
	}
};

struct StepStationDisplayTrackSett : TransparentWidget {
	StepStation *module;
	int frame = 0;
	int t;

	StepStationDisplayTrackSett(int tIndex) : t(tIndex) {
	}

	void onButton(const event::Button &e) override {

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
				createContextMenu();
				e.consume(this);
		}
		
	}

	void createContextMenu() {
		StepStation *module = dynamic_cast<StepStation *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			menu->addChild(createMenuLabel(("Track " + to_string(t+1)).c_str()));
/*
			menu->addChild(new MenuSeparator());

			struct ModeTypeItem : MenuItem {
				StepStation* module;
				int menuValue;
				int t;

				ModeTypeItem(StepStation* m, int value, int track) {
					module = m;
					menuValue = value;
					t = track;
					text = module->modeNames[value];
				}

				void onAction(const event::Action& e) override {
					
					if (module->userInputs[t][KNOB_MODE][0])
							module->params[module->USER_PARAM+t+module->userInputs[t][KNOB_MODE][1]].setValue((float)menuValue/MAXMODES);
					module->currentMode[t] = menuValue;
				}
			};

			if (	(module->userInputs[t][IN_MODE][0] && module->inputs[module->USER_INPUT+t+module->userInputs[t][IN_MODE][1]].isConnected() ) ||
					(module->userInputs[t][IN_MODE][0] && module->inputs[module->USER_INPUT+t+module->userInputs[t][IN_MODE][1]].isConnected() && !module->userInputs[t][KNOB_MODE][0] ) ) {
				for (int i = 0; i < MAXMODES+1; i++)
					menu->addChild(createMenuLabel(module->modeNames[i]));
			} else {

				
				for (int i = 0; i < MAXMODES+1; i++) {
					auto modeTypeItem = new ModeTypeItem(module, i, t);
					modeTypeItem->rightText = CHECKMARK(module->currentMode[t] == i);
					menu->addChild(modeTypeItem);
				}
				
			}
*/
			menu->addChild(new MenuSeparator());

			menu->addChild(createSubmenuItem("User U1 U2", "", [=](Menu* menu) {
				struct UserInItem : MenuItem {
					StepStation* module;
					int userInType;
					int t;
					int userColumn;

					UserInItem(StepStation* m, int inType, int track, int column) {
						module = m;
						userInType = inType;
						t = track;
						userColumn = column;
						text = module->userInNames[inType];
					}

					void onAction(const event::Action& e) override {
						module->userInputs[t][module->userTable[t][userColumn]][0] = 0;
						module->userInputs[t][userInType][0] = 1;
						if (userColumn == 0)
							module->userInputs[t][userInType][1] = 0;
						else
							module->userInputs[t][userInType][1] = 8;
						module->userTable[t][userColumn] = userInType;
					}
				};

				struct UserKnobItem : MenuItem {
					StepStation* module;
					int userKnobType;
					int t;
					int userColumn;

					UserKnobItem(StepStation* m, int knobType, int track, int column) {
						module = m;
						userKnobType = knobType;
						t = track;
						userColumn = column;
						text = module->userKnobNames[knobType];
					}

					void onAction(const event::Action& e) override {
						module->userInputs[t][module->userTable[t][userColumn]][0] = 0;
						module->userInputs[t][userKnobType+KNOB_SHIFT][0] = 1;
						if (userColumn == 1)
							module->userInputs[t][userKnobType+KNOB_SHIFT][1] = 0;
						else
							module->userInputs[t][userKnobType+KNOB_SHIFT][1] = 8;
						module->userTable[t][userColumn] = userKnobType+KNOB_SHIFT;
					}
				};

				menu->addChild(createSubmenuItem("Input U1", (module->userInNames[module->userTable[t][0]]), [=](Menu* menu) {
					for (int i = 0; i < KNOB_SHIFT; i++) {
						auto userInTypeItem = new UserInItem(module, i, t, 0);
						userInTypeItem->rightText = CHECKMARK(module->userTable[t][0] == i);
						if (i == module->userTable[t][2])
							menu->addChild(createMenuLabel(module->userInNames[i]));
						else
							menu->addChild(userInTypeItem);
					}
				}));
				menu->addChild(createSubmenuItem("Knob U1", (module->userKnobNames[module->userTable[t][1]-KNOB_SHIFT]), [=](Menu* menu) {
					for (int i = 0; i < KNOB_NR; i++) {
						auto userKnobTypeItem = new UserKnobItem(module, i, t, 1);
						userKnobTypeItem->rightText = CHECKMARK(module->userTable[t][1]-KNOB_SHIFT == i);
						if (i+KNOB_SHIFT != module->userTable[t][3] || i > 2)
							menu->addChild(userKnobTypeItem);
						else
							menu->addChild(createMenuLabel(module->userKnobNames[i]));
					}
				}));
				menu->addChild(createSubmenuItem("Input U2", (module->userInNames[module->userTable[t][2]]), [=](Menu* menu) {
					for (int i = 0; i < KNOB_SHIFT; i++) {
						auto userInTypeItem = new UserInItem(module, i, t, 2);
						userInTypeItem->rightText = CHECKMARK(module->userTable[t][2] == i);
						if (i == module->userTable[t][0])
							menu->addChild(createMenuLabel(module->userInNames[i]));
						else
							menu->addChild(userInTypeItem);
					}
				}));
				menu->addChild(createSubmenuItem("Knob U2", (module->userKnobNames[module->userTable[t][3]-KNOB_SHIFT]), [=](Menu* menu) {
					for (int i = 0; i < KNOB_NR; i++) {
						auto userKnobTypeItem = new UserKnobItem(module, i, t, 3);
						userKnobTypeItem->rightText = CHECKMARK(module->userTable[t][3]-KNOB_SHIFT == i);
						if (i+KNOB_SHIFT != module->userTable[t][1] || i > 2)
							menu->addChild(userKnobTypeItem);
						else
							menu->addChild(createMenuLabel(module->userKnobNames[i]));
					}
				}));
			}));


			menu->addChild(createSubmenuItem("TRACK Settings", "", [=](Menu* menu) {
/*
				struct RevTypeItem : MenuItem {
					StepStation* module;
					int menuValue;
					int t;

					RevTypeItem(StepStation* m, int value, int track) {
						module = m;
						menuValue = value;
						t = track;
						text = module->revTypeNames[value];
					}

					void onAction(const event::Action& e) override {
						module->revType[t] = menuValue;
					}
				};

				menu->addChild(createSubmenuItem("Reverse Input Voltage", (module->revTypeNames[module->revType[t]]), [=](Menu * menu) {
					for (int i = 0; i < 3; i++) {
						auto revTypeItem = new RevTypeItem(module, i, t);
						if (i == 2)
							revTypeItem->rightText = module->revTypeNames[module->revType[MC]] + " " + CHECKMARK(module->revType[t] == i);
						else
							revTypeItem->rightText = CHECKMARK(module->revType[t] == i);
						menu->addChild(revTypeItem);
					}
				}));
*/
				//----------------------------------------

				struct DontAdvanceItem : MenuItem {
					StepStation* module;
					int menuValue;
					int t;

					DontAdvanceItem(StepStation* m, int value, int track) {
						module = m;
						menuValue = value;
						t = track;
						text = module->dontAdvanceNames[value];
					}

					void onAction(const event::Action& e) override {
						module->dontAdvanceSetting[t] = menuValue;
					}
				};

				menu->addChild(createSubmenuItem("1st clk after reset:", (module->dontAdvanceNames[module->dontAdvanceSetting[t]]), [=](Menu * menu) {
					for (int i = 0; i < 3; i++) {
						auto dontAdvanceItem = new DontAdvanceItem(module, i, t);
						if (i == 2)
							dontAdvanceItem->rightText = module->dontAdvanceNames[module->dontAdvanceSetting[MC]] + " " + CHECKMARK(module->dontAdvanceSetting[t] == i);
						else
							dontAdvanceItem->rightText = CHECKMARK(module->dontAdvanceSetting[t] == i);
						menu->addChild(dontAdvanceItem);
					}
				}));

				//----------------------------------------

				struct RstStepsWhenItem : MenuItem {
					StepStation* module;
					int menuValue;
					int t;

					RstStepsWhenItem(StepStation* m, int value, int track) {
						module = m;
						menuValue = value;
						t = track;
						text = module->rstStepsWhenNames[value];
					}

					void onAction(const event::Action& e) override {
						module->rstStepsWhen[t] = menuValue;
					}
				};

				menu->addChild(createSubmenuItem("Sequence Reset:", (module->rstStepsWhenNames[module->rstStepsWhen[t]]), [=](Menu * menu) {
					for (int i = 0; i < 4; i++) {
						auto rstStepsWhenItem = new RstStepsWhenItem(module, i, t);
						if (i == 3)
							rstStepsWhenItem->rightText = module->rstStepsWhenNames[module->rstStepsWhen[MC]] + " " + CHECKMARK(module->rstStepsWhen[t] == i);
						else
							rstStepsWhenItem->rightText = CHECKMARK(module->rstStepsWhen[t] == i);
						menu->addChild(rstStepsWhenItem);
					}
				}));

				menu->addChild(new MenuSeparator());

				menu->addChild(createBoolPtrMenuItem("Exclude from MASTER RST", "", &module->xcludeFromRst[t]));
				menu->addChild(createBoolPtrMenuItem("Exclude from RUN", "", &module->xcludeFromRun[t]));
		
			}));

			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuItem("Copy Sequence + User", "", [=]() {module->copyUser(t);module->copyTrack(t);}));

			if (stepStation_clipboardTrack) {
				menu->addChild(createMenuItem("Paste Sequence", "", [=]() {module->pasteToTrack(t);}));
				menu->addChild(createMenuItem("Paste User", "", [=]() {module->pasteUser(t);}));
				menu->addChild(createMenuItem("Paste Sequence + User", "", [=]() {module->pasteUser(t);module->pasteToTrack(t);}));
			} else if (stepSeq_clipboard) {
				menu->addChild(createMenuItem("Paste Sequence", "", [=]() {module->pasteToTrack(t);}));
				menu->addChild(createMenuLabel("Paste User"));
				menu->addChild(createMenuLabel("Paste Sequence + User"));
			} else {
				menu->addChild(createMenuLabel("Paste Sequence"));
				menu->addChild(createMenuLabel("Paste User"));
				menu->addChild(createMenuLabel("Paste Sequence + User"));
			}

			menu->addChild(new MenuSeparator());

			struct SampleDelayItem : MenuItem {
				StepStation* module;
				int menuValue;
				int t;

				SampleDelayItem(StepStation* m, int value, int track) {
					module = m;
					menuValue = value;
					t = track;
					text = module->sampleDelayNames[value];
				}

				void onAction(const event::Action& e) override {
					module->sampleDelay[t] = menuValue;
				}
			};

			menu->addChild(createSubmenuItem("Out DELAY:", (module->sampleDelayNames[module->sampleDelay[t]]), [=](Menu * menu) {
				for (int i = 0; i < 7; i++) {
					auto sampleDelayItem = new SampleDelayItem(module, i, t);
					
					if (i == 6)
						sampleDelayItem->rightText = module->sampleDelayNames[module->sampleDelay[MC]] + " " + CHECKMARK(module->sampleDelay[t] == i);
					else
						sampleDelayItem->rightText = CHECKMARK(module->sampleDelay[t] == i);
					menu->addChild(sampleDelayItem);
				}
			}));
			
			struct RangeItem : MenuItem {
				StepStation* module;
				int menuValue;
				int t;

				RangeItem(StepStation* m, int value, int track) {
					module = m;
					menuValue = value;
					t = track;
					text = module->rangeNames[value];
				}

				void onAction(const event::Action& e) override {
					module->range[t] = menuValue;
				}
			};

			menu->addChild(createSubmenuItem("Knobs RANGE", (module->rangeNames[module->range[t]]), [=](Menu * menu) {
				for (int i = 0; i < 11; i++) {
					auto rangeItem = new RangeItem(module, i, t);
					if (i == 10)
						rangeItem->rightText = module->rangeNames[module->range[MC]] + " " + CHECKMARK(module->range[t] == i);
					else
						rangeItem->rightText = CHECKMARK(module->range[t] == i);
					menu->addChild(rangeItem);
				}
			}));

			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuItem("Randomize Steps", "", [=]() {module->randomizeSequence(t);}));
			menu->addChild(createMenuItem("Initialize Steps", "", [=]() {module->initializeSequence(t);}));

			menu->addChild(new MenuSeparator());
		}
	}
};

struct StepStationDisplayU1 : TransparentWidget {
	StepStation *module;
	int frame = 0;
	int t;

	StepStationDisplayU1(int tIndex) : t(tIndex) {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module && layer == 1) {
			shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/vgatrue.ttf"));
			nvgFontSize(args.vg, 10);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);

			std::string tempText;
			switch (module->userTable[t][0]) {

				case IN_CHANGE:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_GREEN));
					tempText = "CH";
				break;

				case IN_CHANGEPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_GREEN));
					tempText = "CP";
					if (module->userInputs[t][IN_CHANGE][0])
						tempText += "*";
				break;

				case IN_LENGTH:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_GREEN));
					tempText = "LN";
				break;
				
				case IN_MODE:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_CYAN));
					tempText = "MD";
				break;

				case IN_OUTSCALE:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_YELLOW));
					tempText = "SC";
				break;

				case IN_RSTSTEP:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_BROWN));
					tempText = "RS";
				break;

				case IN_RETRIG:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_GREY));
					tempText = "RT";
				break;

				case IN_REV:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_YELLOW));
					tempText = "RV";
				break;

				case IN_RUN:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_BLUE));
					tempText = "RN";
				break;

				case IN_SWING:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_PURPLE));
					tempText = "SW";
					if (module->params[module->DIVMULT_KNOB_PARAM+t].getValue() < 23)
						tempText += "*";
				break;
				/*
				// ***********

				case IN_CHANGE:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_LIGHT_RED));
					tempText = "CH";
				break;

				case IN_CHANGEPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_LIGHT_RED));
					tempText = "CP";
					if (module->userInputs[t][IN_CHANGE][0])
						tempText += "*";
				break;

				case IN_LENGTH:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_GREEN));
					tempText = "LN";
				break;

				case IN_MODE:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_LIGHT_GREEN));
					tempText = "MD";
				break;

				case IN_OUTSCALE:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_YELLOW));
					tempText = "SC";
				break;

				case IN_RSTSTEP:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_LIGHT_MAGENTA));
					tempText = "RS";
				break;

				case IN_RETRIG:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_LIGHT_CYAN));
					tempText = "RT";
				break;

				case IN_REV:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_YELLOW));
					tempText = "RV";
				break;

				case IN_RUN:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_LIGHT_BLUE));
					tempText = "RN";
				break;

				case IN_SWING:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_RED));
					tempText = "SW";
				break;
				if (module->params[module->DIVMULT_KNOB_PARAM+t].getValue() < 23)
						tempText += "*";
				*/
			}
			nvgTextBox(args.vg, 0, 0, 10, tempText.c_str(), NULL);
		}
		Widget::drawLayer(args, layer);
	}
};

struct StepStationDisplayU2 : TransparentWidget {
	StepStation *module;
	int frame = 0;
	int t;

	StepStationDisplayU2(int tIndex) : t(tIndex) {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module && layer == 1) {
			shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/vgatrue.ttf"));
			nvgFontSize(args.vg, 10);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);

			std::string tempText;
			switch (module->userTable[t][2]) {

				case IN_CHANGE:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_GREEN));
					tempText = "CH";
				break;

				case IN_CHANGEPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_GREEN));
					tempText = "CP";
					if (module->userInputs[t][IN_CHANGE][0])
						tempText += "*";
				break;

				case IN_LENGTH:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_GREEN));
					tempText = "LN";
				break;
				
				case IN_MODE:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_CYAN));
					tempText = "MD";
				break;

				case IN_OUTSCALE:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_YELLOW));
					tempText = "SC";
				break;

				case IN_RSTSTEP:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_BROWN));
					tempText = "RS";
				break;

				case IN_RETRIG:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_GREY));
					tempText = "RT";
				break;

				case IN_REV:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_YELLOW));
					tempText = "RV";
				break;

				case IN_RUN:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_BLUE));
					tempText = "RN";
				break;

				case IN_SWING:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_PURPLE));
					tempText = "SW";
					if (module->params[module->DIVMULT_KNOB_PARAM+t].getValue() < 23)
						tempText += "*";
				break;
			}
			nvgTextBox(args.vg, 0, 0, 10, tempText.c_str(), NULL);
		}
		Widget::drawLayer(args, layer);
	}
};

struct StepStationDisplayK1 : TransparentWidget {
	StepStation *module;
	int frame = 0;
	int t;

	StepStationDisplayK1(int tIndex) : t(tIndex) {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module && layer == 1) {
			shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/vgatrue.ttf"));
			nvgFontSize(args.vg, 10);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);

			std::string tempText;
			switch (module->userTable[t][1]) {


				case KNOB_CHANGEPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_GREEN));
					tempText = "CH";
				break;

				case KNOB_MODE:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_BLUE));
					tempText = "MD";
				break;

				case KNOB_OUTSCALE:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_YELLOW));
					tempText = "SC";
				break;

				case KNOB_RSTSTEP:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_BROWN));
					tempText = "RS";
				break;

				case KNOB_RETRIGPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_GREY));
					tempText = "RP";
				break;

				case KNOB_SWING:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_PURPLE));
					tempText = "SW";
					if (module->params[module->DIVMULT_KNOB_PARAM+t].getValue() < 23)
						tempText += "*";
				break;

				case KNOB_ATN:
					nvgFontSize(args.vg, 14);
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_GREEN));
					tempText = "+";
				break;

				case KNOB_ATNV:
					nvgFontSize(args.vg, 14);
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_RED));
					tempText = "±";
				break;
/*
				case KNOB_CHANGEPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_LIGHT_RED));
					tempText = "CP";
				break;

				case KNOB_MODE:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_LIGHT_GREEN));
					tempText = "MD";
				break;

				case KNOB_OUTSCALE:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_YELLOW));
					tempText = "SC";
				break;

				case KNOB_RSTSTEP:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_LIGHT_MAGENTA));
					tempText = "RS";
				break;
			
				case KNOB_RETRIGPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_LIGHT_CYAN));
					tempText = "RP";
				break;

				case KNOB_SWING:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_RED));
					tempText = "SW";
					if (module->params[module->DIVMULT_KNOB_PARAM+t].getValue() < 23)
						tempText += "*";
				break;

				case KNOB_ATN:
					nvgFontSize(args.vg, 14);
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_GREEN));
					tempText = "+";
				break;

				case KNOB_ATNV:
					nvgFontSize(args.vg, 14);
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_RED));
					tempText = "±";
				break;
*/
			}
			nvgTextBox(args.vg, 0, 0, 15, tempText.c_str(), NULL);
		}
		Widget::drawLayer(args, layer);
	}
};

struct StepStationDisplayK2 : TransparentWidget {
	StepStation *module;
	int frame = 0;
	int t;

	StepStationDisplayK2(int tIndex) : t(tIndex) {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module && layer == 1) {
			shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/vgatrue.ttf"));
			nvgFontSize(args.vg, 10);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);

			std::string tempText;
			switch (module->userTable[t][3]) {

				case KNOB_CHANGEPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_GREEN));
					tempText = "CH";
				break;

				case KNOB_MODE:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_BLUE));
					tempText = "MD";
				break;

				case KNOB_OUTSCALE:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_YELLOW));
					tempText = "SC";
				break;

				case KNOB_RSTSTEP:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_BROWN));
					tempText = "RS";
				break;

				case KNOB_RETRIGPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_GREY));
					tempText = "RP";
				break;

				case KNOB_SWING:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_PURPLE));
					tempText = "SW";
					if (module->params[module->DIVMULT_KNOB_PARAM+t].getValue() < 23)
						tempText += "*";
				break;

				case KNOB_ATN:
					nvgFontSize(args.vg, 14);
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_GREEN));
					tempText = "+";
				break;

				case KNOB_ATNV:
					nvgFontSize(args.vg, 14);
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_RED));
					tempText = "±";
				break;

				/*
				case KNOB_MODE:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_LIGHT_GREEN));
					tempText = "MD";
				break;

				case KNOB_OUTSCALE:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_YELLOW));
					tempText = "SC";
				break;

				case KNOB_RSTSTEP:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_LIGHT_MAGENTA));
					tempText = "RS";
				break;
			
				case KNOB_RETRIGPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_LIGHT_CYAN));
					tempText = "RP";
				break;

				case KNOB_SWING:
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_RED));
					tempText = "SW";
					if (module->params[module->DIVMULT_KNOB_PARAM+t].getValue() < 23)
						tempText += "*";
				break;

				case KNOB_ATN:
					nvgFontSize(args.vg, 14);
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_GREEN));
					tempText = "+";
				break;

				case KNOB_ATNV:
					nvgFontSize(args.vg, 14);
					nvgFillColor(args.vg, nvgRGB(COLOR_EGA_RED));
					tempText = "±";
				break;
				*/
			}
			nvgTextBox(args.vg, 0, 0, 15, tempText.c_str(), NULL);
		}
		Widget::drawLayer(args, layer);
	}
};

struct StepStationWidget : ModuleWidget {
	StepStationWidget(StepStation* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/StepStation.svg")));

		{
			StepStationDisplayTempo *display = new StepStationDisplayTempo();
			display->box.pos = mm2px(Vec(22.2 - 5 - 3, 14.8));
			display->box.size = mm2px(Vec(16.8, 6.5));
			display->module = module;
			addChild(display);
		}

		{
			StepStationDisplay *display = new StepStationDisplay();
			display->box.pos = mm2px(Vec(126 + 5.7, 9.1));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}

		const float yLine = 33;
		const float xTemp = 0.3;

		for (int t = 0; t < MAXTRACKS; t++) {

			{
				StepStationDisplayDiv *display = new StepStationDisplayDiv(t);
				display->box.pos = mm2px(Vec(22.5, 33+11.5*t));
				display->box.size = mm2px(Vec(15, 6.3));
				display->module = module;
				addChild(display);
			}
			{
				StepStationDisplayMode *display = new StepStationDisplayMode(t);
				display->box.pos = mm2px(Vec(98.8, 33+11.5*t));
				display->box.size = mm2px(Vec(11, 6.3));
				display->module = module;
				addChild(display);
			}
			{
				StepStationDisplayTrackSett *display = new StepStationDisplayTrackSett(t);
				display->box.pos = mm2px(Vec(108.8, 33+11.5*t));
				display->box.size = mm2px(Vec(8, 6.3));
				display->module = module;
				addChild(display);
			}
			{
				StepStationDisplayU1 *display = new StepStationDisplayU1(t);
				display->box.pos = mm2px(Vec(61-xTemp, yLine+11.5*t));
				display->box.size = mm2px(Vec(4, 5));
				display->module = module;
				addChild(display);
			}
			{
				StepStationDisplayK1 *display = new StepStationDisplayK1(t);
				display->box.pos = mm2px(Vec(70-xTemp, 33+11.5*t));
				display->box.size = mm2px(Vec(4, 5));
				display->module = module;
				addChild(display);
			}
			{
				StepStationDisplayU2 *display = new StepStationDisplayU2(t);
				display->box.pos = mm2px(Vec(80-xTemp, yLine+11.5*t));
				display->box.size = mm2px(Vec(4, 5));
				display->module = module;
				addChild(display);
			}
			{
				StepStationDisplayK2 *display = new StepStationDisplayK2(t);
				display->box.pos = mm2px(Vec(89-xTemp, yLine+11.5*t));
				display->box.size = mm2px(Vec(4, 5));
				display->module = module;
				addChild(display);
			}
		}

		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		const float yProgLine = 18.2f;

		const float xExtClock = 11.5f - 1.5f - 2.f - 1.f;

		const float xRun1 = 61.5f;
		const float yRun1 = yProgLine + 1.3f;
		const float xRun2 = 69.f;
		const float yRun2 = yProgLine - 5.2f;

		
		const float xSeqRunBut = 56.5f + 20 + 6;
		const float xSeqRunIn = xSeqRunBut + 10.f;

		const float xBpmKnob = 85.5f - 27.f - 5.f - 0.5f - 3.f;
		const float yBpmKnob =  16.3f;

		const float xRst = 100.f + 5.7;
		const float xRstB = xRst + 9.8f;
		const float xRstOut = xRstB + 9.8f;
		
		const float xProgIn = 149.f + 5; 

		const float xProgKnob = 163.5f +5; 
		const float yProgKnob = 16.3f;
		
		const float xSet = 181.f + 4;
		const float xAuto = 193.2f + 3;

		const float xRecallBut = 209.f + 2;
		const float xRecallIn = 219.f + 2;

		const float xStore = 231.4f + 1;

		const float xClkOut = 247.f;

		const float yStart = 36.5f;
		const float yDelta = 11.5f;

		const float xStart = 120.5f;
		const float xDelta = 7.45f;
		const float xBlockShift = 1.5f;

		const float yLight = -5.f;
		const float xLight = 2.2f;

		const float xOut = 247.f;

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xBpmKnob, yBpmKnob)), module, StepStation::BPM_KNOB_PARAM));

		addInput(createStepStationClockInCentered<SickoClockInStepStation>(mm2px(Vec(xExtClock, yProgLine)), module, StepStation::EXTCLOCK_INPUT));

		addParam(createLightParamCentered<VCVLightBezel<YellowLight>>(mm2px(Vec(xRun1, yRun1)), module, StepStation::INTCLOCKBUT_PARAM, StepStation::RUNBUT_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRun2, yRun2)), module, StepStation::RUN_INPUT));

		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xSeqRunBut, yProgLine)), module, StepStation::SEQRUN_PARAM, StepStation::SEQRUN_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xSeqRunIn, yProgLine)), module, StepStation::SEQRUN_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRst, yProgLine)), module, StepStation::RSTALL_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<YellowLight>>(mm2px(Vec(xRstB, yProgLine)), module, StepStation::RSTALLBUT_PARAM, StepStation::RSTALLBUT_LIGHT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xRstOut, yProgLine)), module, StepStation::RST_OUTPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xProgIn, yProgLine)), module, StepStation::PROG_INPUT));
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xProgKnob, yProgKnob)), module, StepStation::PROG_PARAM));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xSet, yProgLine)), module, StepStation::SET_PARAM, StepStation::SET_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xAuto, yProgLine)), module, StepStation::AUTO_PARAM, StepStation::AUTO_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xRecallBut, yProgLine)), module, StepStation::RECALL_PARAM, StepStation::RECALL_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRecallIn, yProgLine)), module, StepStation::RECALL_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xStore, yProgLine)), module, StepStation::STORE_PARAM, StepStation::STORE_LIGHT));
		addOutput(createStepStationClockOutCentered<SickoClockOutStepStation>(mm2px(Vec(xClkOut, yProgLine)), module, StepStation::CLOCK_OUTPUT));
		
		int shiftCont = 0;
		int shiftGroup = 0;
		float xShift = 0;
		
		const float xClock = 7.5f;
		const float xDivMul = xClock + 9.5f;
		
		const float xRstIn = xDivMul + 27.f;
		const float xLen = xRstIn + 10.f;

		const float xU1 = xLen + 11.f;
		const float xK1 = xU1 + 9.f;
		const float xU2 = xK1 + 10.f;
		const float xK2 = xU2 + 9.f;

		for (int t = 0; t < MAXTRACKS; t++) {
			
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xClock, yStart+(t*yDelta))), module, StepStation::CLK_INPUT+t));
			
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xDivMul, yStart+(t*yDelta))), module, StepStation::DIVMULT_KNOB_PARAM+t));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRstIn, yStart+(t*yDelta))), module, StepStation::RST_INPUT+t));

			addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xLen, yStart+(t*yDelta))), module, StepStation::LENGTH_PARAM+t));

			addInput(createStepStationInputCentered<SickoInputStepStation>(mm2px(Vec(xU1, yStart+(t*yDelta))), module, StepStation::USER_INPUT+t));
			addParam(createStepStationParamCentered<SickoKnobStepStation>(mm2px(Vec(xK1, yStart+(t*yDelta))), module, StepStation::USER_PARAM+t));
			addInput(createStepStationInputCentered<SickoInputStepStation>(mm2px(Vec(xU2, yStart+(t*yDelta))), module, StepStation::USER_INPUT+t+8));
			addParam(createStepStationParamCentered<SickoKnobStepStation>(mm2px(Vec(xK2, yStart+(t*yDelta))), module, StepStation::USER_PARAM+t+8));

			shiftCont = 0;
			shiftGroup = 0;
			xShift = 0;

			for (int st = 0; st < 16; st++) {

				addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, StepStation::STEP_PARAM+(t*16)+st));
				addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(xStart + (xDelta*st) + xShift + xLight, yStart+(t*yDelta)+yLight)), module, StepStation::STEP_LIGHT+(t*16)+st));
				shiftCont++;
				if (shiftCont > 3) {
					shiftGroup++;
					xShift = xBlockShift * shiftGroup;
					shiftCont = 0;
				}
				
			}

			addOutput(createStepStationClockOutCentered<SickoOutStation>(mm2px(Vec(xOut, yStart+(t*yDelta))), module, StepStation::OUT_OUTPUT+t));
		}
		
	}

	void appendContextMenu(Menu* menu) override {
		StepStation* module = dynamic_cast<StepStation*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createSubmenuItem("Initialize Steps", "", [=](Menu * menu) {
			menu->addChild(createMenuItem("All tracks", "", [=]() {module->initializeAll();}));
			menu->addChild(createMenuItem("Track 1", "", [=]() {module->initializeSequence(0);}));
			menu->addChild(createMenuItem("Track 2", "", [=]() {module->initializeSequence(1);}));
			menu->addChild(createMenuItem("Track 3", "", [=]() {module->initializeSequence(2);}));
			menu->addChild(createMenuItem("Track 4", "", [=]() {module->initializeSequence(3);}));
			menu->addChild(createMenuItem("Track 5", "", [=]() {module->initializeSequence(4);}));
			menu->addChild(createMenuItem("Track 6", "", [=]() {module->initializeSequence(5);}));
			menu->addChild(createMenuItem("Track 7", "", [=]() {module->initializeSequence(6);}));
			menu->addChild(createMenuItem("Track 8", "", [=]() {module->initializeSequence(7);}));
		}));

		menu->addChild(createSubmenuItem("Randomize Steps", "", [=](Menu * menu) {
			menu->addChild(createMenuItem("All tracks", "", [=]() {module->randomizeAll();}));
			menu->addChild(createMenuItem("Track 1", "", [=]() {module->randomizeSequence(0);}));
			menu->addChild(createMenuItem("Track 2", "", [=]() {module->randomizeSequence(1);}));
			menu->addChild(createMenuItem("Track 3", "", [=]() {module->randomizeSequence(2);}));
			menu->addChild(createMenuItem("Track 4", "", [=]() {module->randomizeSequence(3);}));
			menu->addChild(createMenuItem("Track 5", "", [=]() {module->randomizeSequence(4);}));
			menu->addChild(createMenuItem("Track 6", "", [=]() {module->randomizeSequence(5);}));
			menu->addChild(createMenuItem("Track 7", "", [=]() {module->randomizeSequence(6);}));
			menu->addChild(createMenuItem("Track 8", "", [=]() {module->randomizeSequence(7);}));
		}));



		struct RangeItem : MenuItem {
			StepStation* module;
			int range;
			void onAction(const event::Action& e) override {
				module->range[MC] = range;
			}
		};

		menu->addChild(new MenuSeparator());

		std::string rangeNames[10] = {"0/1v", "0/2v", "0/3v", "0/5v", "0/10v", "-1/+1v", "-2/+2v", "-3/+3v", "-5/+5v", "-10/+10v"};
		menu->addChild(createSubmenuItem("Default Knob RANGE", rangeNames[module->range[MC]], [=](Menu * menu) {
			for (int i = 0; i < 10; i++) {
				RangeItem* rangeItem = createMenuItem<RangeItem>(rangeNames[i]);
				rangeItem->rightText = CHECKMARK(module->range[MC] == i);
				rangeItem->module = module;
				rangeItem->range = i;
				menu->addChild(rangeItem);
			}
		}));

		struct SampleDelayItem : MenuItem {
			StepStation* module;
			int sampleDelay;
			void onAction(const event::Action& e) override {
				module->sampleDelay[MC] = sampleDelay;
			}
		};

		menu->addChild(createSubmenuItem("Default OUT DELAY", (module->sampleDelayNames[module->sampleDelay[MC]]), [=](Menu * menu) {
			for (int i = 0; i < 6; i++) {
				SampleDelayItem* sampleDelayItem = createMenuItem<SampleDelayItem>(module->sampleDelayNames[i]);
				sampleDelayItem->rightText = CHECKMARK(module->sampleDelay[MC] == i);
				sampleDelayItem->module = module;
				sampleDelayItem->sampleDelay = i;
				menu->addChild(sampleDelayItem);
			}
		}));

		// *****************************************************************************************
		// *****************************************************************************************
		// *****************************************************************************************

		menu->addChild(new MenuSeparator());

		menu->addChild(createSubmenuItem("GLOBAL settings", "", [=](Menu * menu) {


			menu->addChild(createMenuLabel("Track Defaults"));
/*			
			struct RevTypeItem : MenuItem {
				StepStation* module;
				int revType;
				void onAction(const event::Action& e) override {
					module->revType[MC] = revType;
				}
			};


			std::string RevTypeNames[2] = {"Positive", "Negative"};
			menu->addChild(createSubmenuItem("Reverse Input Voltage", (RevTypeNames[module->revType[MC]]), [=](Menu * menu) {
				for (int i = 0; i < 2; i++) {
					RevTypeItem* revTypeItem = createMenuItem<RevTypeItem>(RevTypeNames[i]);
					revTypeItem->rightText = CHECKMARK(module->revType[MC] == i);
					revTypeItem->module = module;
					revTypeItem->revType = i;
					menu->addChild(revTypeItem);
				}
			}));
*/
			struct DontAdvanceItem : MenuItem {
				StepStation* module;
				int dontAdvanceSetting;
				void onAction(const event::Action& e) override {
					module->dontAdvanceSetting[MC] = dontAdvanceSetting;
				}
			};

			std::string FirstClkNames[2] = {"Advance", "Don't Advance"};
			menu->addChild(createSubmenuItem("1st clk after reset:", (FirstClkNames[module->dontAdvanceSetting[MC]]), [=](Menu * menu) {
				for (int i = 0; i < 2; i++) {
					DontAdvanceItem* dontAdvanceItem = createMenuItem<DontAdvanceItem>(FirstClkNames[i]);
					dontAdvanceItem->rightText = CHECKMARK(module->dontAdvanceSetting[MC] == i);
					dontAdvanceItem->module = module;
					dontAdvanceItem->dontAdvanceSetting = i;
					menu->addChild(dontAdvanceItem);
				}
			}));

			struct RstStepsWhenItem : MenuItem {
				StepStation* module;
				int rstStepsWhen;
				void onAction(const event::Action& e) override {
					module->rstStepsWhen[MC] = rstStepsWhen;
				}
			};

			std::string RstStepsWhenNames[3] = {"No Reset", "On RUN", "On STOP"};
			menu->addChild(createSubmenuItem("Sequence Reset:", (RstStepsWhenNames[module->rstStepsWhen[MC]]), [=](Menu * menu) {
				for (int i = 0; i < 3; i++) {
					RstStepsWhenItem* rstStepsWhenItem = createMenuItem<RstStepsWhenItem>(RstStepsWhenNames[i]);
					rstStepsWhenItem->rightText = CHECKMARK(module->rstStepsWhen[MC] == i);
					rstStepsWhenItem->module = module;
					rstStepsWhenItem->rstStepsWhen = i;
					menu->addChild(rstStepsWhenItem);
				}
			}));

			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuLabel("Module Settings"));

			struct RunTypeItem : MenuItem {
				StepStation* module;
				int runType;
				void onAction(const event::Action& e) override {
					module->runType = runType;
				}
			};

			std::string RunTypeNames[2] = {"Gate", "Trig"};
			menu->addChild(createSubmenuItem("RUN Input", (RunTypeNames[module->runType]), [=](Menu * menu) {
				for (int i = 0; i < 2; i++) {
					RunTypeItem* runTypeItem = createMenuItem<RunTypeItem>(RunTypeNames[i]);
					runTypeItem->rightText = CHECKMARK(module->runType == i);
					runTypeItem->module = module;
					runTypeItem->runType = i;
					menu->addChild(runTypeItem);
				}
			}));

			menu->addChild(createBoolPtrMenuItem("Reset internal clock on RST", "", &module->rstClkOnRst));
			menu->addChild(createBoolPtrMenuItem("Reset Seq on PROG change", "", &module->rstSeqOnProgChange));

			struct PpqnItem : MenuItem {
				StepStation* module;
				int ppqn;
				void onAction(const event::Action& e) override {
					module->tempPpqn = ppqn;
					module->ppqnChange = true;
				}
			};

			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuLabel("External Main Clock"));
			std::string ppqnNames[7] = {"1 PPQN", "2 PPQN", "4 PPQN", "8 PPQN", "12 PPQN", "16 PPQN", "24 PPQN"};
			menu->addChild(createSubmenuItem("Resolution", ppqnNames[module->ppqn], [=](Menu * menu) {
				for (int i = 0; i < 7; i++) {
					PpqnItem* ppqnItem = createMenuItem<PpqnItem>(ppqnNames[i]);
					ppqnItem->rightText = CHECKMARK(module->ppqn == i);
					ppqnItem->module = module;
					ppqnItem->ppqn = i;
					menu->addChild(ppqnItem);
				}
			}));

			menu->addChild(createBoolPtrMenuItem("CV clock IN", "", &module->cvClockIn));
			menu->addChild(createBoolPtrMenuItem("CV clock OUT", "", &module->cvClockOut));

			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("User Interface"));
			menu->addChild(createBoolPtrMenuItem("DIV/MULT mouse controls", "", &module->divControls));
			menu->addChild(createBoolPtrMenuItem("MODE mouse controls", "", &module->modeControls));

		}));
/*
		menu->addChild(createSubmenuItem("GLOBAL settings", "", [=](Menu * menu) {
		
			


		}));
*/
		menu->addChild(createSubmenuItem("TRACK settings", "", [=](Menu * menu) {

			struct UserInItem : MenuItem {
				StepStation* module;
				int userInType;
				int t;
				int userColumn;

				UserInItem(StepStation* m, int inType, int track, int column) {
					module = m;
					userInType = inType;
					t = track;
					userColumn = column;
					text = module->userInNames[inType];
				}

				void onAction(const event::Action& e) override {
					module->userInputs[t][module->userTable[t][userColumn]][0] = 0;
					module->userInputs[t][userInType][0] = 1;
					module->userInputs[t][userInType][1] = 0;
					module->userTable[t][userColumn] = userInType;
				}
			};

			struct UserKnobItem : MenuItem {
				StepStation* module;
				int userKnobType;
				int t;
				int userColumn;

				UserKnobItem(StepStation* m, int knobType, int track, int column) {
					module = m;
					userKnobType = knobType;
					t = track;
					userColumn = column;
					text = module->userKnobNames[knobType];
				}

				void onAction(const event::Action& e) override {
					module->userInputs[t][module->userTable[t][userColumn]][0] = 0;
					module->userInputs[t][userKnobType+KNOB_SHIFT][0] = 1;
					module->userInputs[t][userKnobType+KNOB_SHIFT][1] = 0;
					module->userTable[t][userColumn] = userKnobType+KNOB_SHIFT;
				}
			};

			for (int t = 0; t < MAXTRACKS; t++) {
				menu->addChild(createSubmenuItem(("Track " + to_string(t + 1)).c_str(), "", [=](Menu* menu) {

					menu->addChild(createSubmenuItem("Input U1", (module->userInNames[module->userTable[t][0]]), [=](Menu* menu) {
						for (int i = 0; i < KNOB_SHIFT; i++) {
							auto userInTypeItem = new UserInItem(module, i, t, 0);
							userInTypeItem->rightText = CHECKMARK(module->userTable[t][0] == i);
							if (i == module->userTable[t][2])
								menu->addChild(createMenuLabel(module->userInNames[i]));
							else
								menu->addChild(userInTypeItem);
						}
					}));
					menu->addChild(createSubmenuItem("Knob U1", (module->userKnobNames[module->userTable[t][1]-KNOB_SHIFT]), [=](Menu* menu) {
						for (int i = 0; i < KNOB_NR; i++) {
							auto userKnobTypeItem = new UserKnobItem(module, i, t, 1);
							userKnobTypeItem->rightText = CHECKMARK(module->userTable[t][1]-KNOB_SHIFT == i);
							if (i+KNOB_SHIFT != module->userTable[t][3] || i > 2)
								menu->addChild(userKnobTypeItem);
							else
								menu->addChild(createMenuLabel(module->userKnobNames[i]));
						}
					}));
					menu->addChild(createSubmenuItem("Input U2", (module->userInNames[module->userTable[t][2]]), [=](Menu* menu) {
						for (int i = 0; i < KNOB_SHIFT; i++) {
							auto userInTypeItem = new UserInItem(module, i, t, 2);
							userInTypeItem->rightText = CHECKMARK(module->userTable[t][2] == i);
							if (i == module->userTable[t][0])
								menu->addChild(createMenuLabel(module->userInNames[i]));
							else
								menu->addChild(userInTypeItem);
						}
					}));
					menu->addChild(createSubmenuItem("Knob U2", (module->userKnobNames[module->userTable[t][3]-KNOB_SHIFT]), [=](Menu* menu) {
						for (int i = 0; i < KNOB_NR; i++) {
							auto userKnobTypeItem = new UserKnobItem(module, i, t, 3);
							userKnobTypeItem->rightText = CHECKMARK(module->userTable[t][3]-KNOB_SHIFT == i);
							if (i+KNOB_SHIFT != module->userTable[t][1] || i > 2)
								menu->addChild(userKnobTypeItem);
							else
								menu->addChild(createMenuLabel(module->userKnobNames[i]));
						}
					}));

					menu->addChild(new MenuSeparator());


					//menu->addChild(createMenuLabel("Tweaks"));

					// **********************************************************************
/*
					struct RevTypeItem : MenuItem {
						StepStation* module;
						int menuValue;
						int t;

						RevTypeItem(StepStation* m, int value, int track) {
							module = m;
							menuValue = value;
							t = track;
							text = module->revTypeNames[value];
						}

						void onAction(const event::Action& e) override {
							module->revType[t] = menuValue;
						}
					};

					menu->addChild(createSubmenuItem("Reverse Input Voltage", (module->revTypeNames[module->revType[t]]), [=](Menu * menu) {
						for (int i = 0; i < 3; i++) {
							auto revTypeItem = new RevTypeItem(module, i, t);
							if (i == 2)
								revTypeItem->rightText = module->revTypeNames[module->revType[MC]] + " " + CHECKMARK(module->revType[t] == i);
							else
								revTypeItem->rightText = CHECKMARK(module->revType[t] == i);
							menu->addChild(revTypeItem);
						}
					}));
*/
					//----------------------------------------

					struct DontAdvanceItem : MenuItem {
						StepStation* module;
						int menuValue;
						int t;

						DontAdvanceItem(StepStation* m, int value, int track) {
							module = m;
							menuValue = value;
							t = track;
							text = module->dontAdvanceNames[value];
						}

						void onAction(const event::Action& e) override {
							module->dontAdvanceSetting[t] = menuValue;
						}
					};

					menu->addChild(createSubmenuItem("1st clk after reset:", (module->dontAdvanceNames[module->dontAdvanceSetting[t]]), [=](Menu * menu) {
						for (int i = 0; i < 3; i++) {
							auto dontAdvanceItem = new DontAdvanceItem(module, i, t);
							if (i == 2)
								dontAdvanceItem->rightText = module->dontAdvanceNames[module->dontAdvanceSetting[MC]] + " " + CHECKMARK(module->dontAdvanceSetting[t] == i);
							else
								dontAdvanceItem->rightText = CHECKMARK(module->dontAdvanceSetting[t] == i);
							menu->addChild(dontAdvanceItem);
						}
					}));

					//----------------------------------------

					struct RstStepsWhenItem : MenuItem {
						StepStation* module;
						int menuValue;
						int t;

						RstStepsWhenItem(StepStation* m, int value, int track) {
							module = m;
							menuValue = value;
							t = track;
							text = module->rstStepsWhenNames[value];
						}

						void onAction(const event::Action& e) override {
							module->rstStepsWhen[t] = menuValue;
						}
					};

					menu->addChild(createSubmenuItem("Sequence Reset:", (module->rstStepsWhenNames[module->rstStepsWhen[t]]), [=](Menu * menu) {
						for (int i = 0; i < 4; i++) {
							auto rstStepsWhenItem = new RstStepsWhenItem(module, i, t);
							if (i == 3)
								rstStepsWhenItem->rightText = module->rstStepsWhenNames[module->rstStepsWhen[MC]] + " " + CHECKMARK(module->rstStepsWhen[t] == i);
							else
								rstStepsWhenItem->rightText = CHECKMARK(module->rstStepsWhen[t] == i);
							menu->addChild(rstStepsWhenItem);
						}
					}));

					menu->addChild(new MenuSeparator());

					menu->addChild(createBoolPtrMenuItem("Exclude from MASTER RST", "", &module->xcludeFromRst[t]));
					menu->addChild(createBoolPtrMenuItem("Exclude from RUN", "", &module->xcludeFromRun[t]));
			
					menu->addChild(new MenuSeparator());

					menu->addChild(createMenuItem("Copy Sequence + User", "", [=]() {module->copyUser(t);module->copyTrack(t);}));

					if (stepStation_clipboardTrack) {
						menu->addChild(createMenuItem("Paste Sequence", "", [=]() {module->pasteToTrack(t);}));
						menu->addChild(createMenuItem("Paste User", "", [=]() {module->pasteUser(t);}));
						menu->addChild(createMenuItem("Paste Sequence + User", "", [=]() {module->pasteUser(t);module->pasteToTrack(t);}));
					} else if (stepSeq_clipboard) {
						menu->addChild(createMenuItem("Paste Sequence", "", [=]() {module->pasteToTrack(t);}));
						menu->addChild(createMenuLabel("Paste User"));
						menu->addChild(createMenuLabel("Paste Sequence + User"));
					} else {
						menu->addChild(createMenuLabel("Paste Sequence"));
						menu->addChild(createMenuLabel("Paste User"));
						menu->addChild(createMenuLabel("Paste Sequence + User"));
					}

					menu->addChild(new MenuSeparator());

					struct SampleDelayItem : MenuItem {
						StepStation* module;
						int menuValue;
						int t;

						SampleDelayItem(StepStation* m, int value, int track) {
							module = m;
							menuValue = value;
							t = track;
							text = module->sampleDelayNames[value];
						}

						void onAction(const event::Action& e) override {
							module->sampleDelay[t] = menuValue;
						}
					};

					menu->addChild(createSubmenuItem("Out DELAY:", (module->sampleDelayNames[module->sampleDelay[t]]), [=](Menu * menu) {
						for (int i = 0; i < 7; i++) {
							auto sampleDelayItem = new SampleDelayItem(module, i, t);
							if (i == 6)
								sampleDelayItem->rightText = module->sampleDelayNames[module->sampleDelay[MC]] + " " + CHECKMARK(module->sampleDelay[t] == i);
							else
								sampleDelayItem->rightText = CHECKMARK(module->sampleDelay[t] == i);
							menu->addChild(sampleDelayItem);
						}
					}));

					struct RangeItem : MenuItem {
						StepStation* module;
						int menuValue;
						int t;

						RangeItem(StepStation* m, int value, int track) {
							module = m;
							menuValue = value;
							t = track;
							text = module->rangeNames[value];
						}

						void onAction(const event::Action& e) override {
							module->range[t] = menuValue;
						}
					};
					menu->addChild(createSubmenuItem("Knobs RANGE", (module->rangeNames[module->range[t]]), [=](Menu * menu) {
						for (int i = 0; i < 11; i++) {
							auto rangeItem = new RangeItem(module, i, t);
							if (i == 10)
								rangeItem->rightText = module->rangeNames[module->range[MC]] + " " + CHECKMARK(module->range[t] == i);
							else
								rangeItem->rightText = CHECKMARK(module->range[t] == i);
							menu->addChild(rangeItem);
						}
					}));

					menu->addChild(new MenuSeparator());

					menu->addChild(createMenuItem("Randomize Steps", "", [=]() {module->randomizeSequence(t);}));
					menu->addChild(createMenuItem("Initialize Steps", "", [=]() {module->initializeSequence(t);}));
					
				}));
			}
		}));

		menu->addChild(new MenuSeparator());
		
		struct ProgInTypeItem : MenuItem {
			StepStation* module;
			int progInType;
			void onAction(const event::Action& e) override {
				module->progInType = progInType;
			}
		};

		std::string ProgInTypeNames[2] = {"CV", "Trig"};
		menu->addChild(createSubmenuItem("PROG Input type", (ProgInTypeNames[module->progInType]), [=](Menu * menu) {
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
		menu->addChild(createSubmenuItem("copy/paste", "", [=](Menu * menu) {
			menu->addChild(createMenuItem("Copy Panel", "", [=]() {module->copyPanel();}));
			if (stepStation_clipboard) {
				menu->addChild(createMenuItem("Paste Panel", "", [=]() {module->pastePanel();}));
			} else {
				menu->addChild(createMenuLabel("Paste Panel"));
			}

			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuItem("Copy User Settings", "", [=]() {module->copyUserSettings();}));
			if (stepStation_userClipboard) {
				menu->addChild(createMenuItem("Paste User Settings", "", [=]() {module->pasteUserSettings();}));
			} else {
				menu->addChild(createMenuLabel("Paste User Settings"));
			}
		
			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuItem("Copy All Sequences", "", [=]() {module->copyAllTracks();}));
			if (stepSeq8_clipboard || stepStation_clipboard) {
				menu->addChild(createMenuItem("Paste All Sequences", "", [=]() {module->pasteAllTracks();}));
			} else {
				menu->addChild(createMenuLabel("Paste All Sequences"));
			}

			menu->addChild(new MenuSeparator());

			menu->addChild(createSubmenuItem("Copy single sequence", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("Copy Sequence 1", "", [=]() {module->copyTrack(0);}));
				menu->addChild(createMenuItem("Copy Sequence 2", "", [=]() {module->copyTrack(1);}));
				menu->addChild(createMenuItem("Copy Sequence 3", "", [=]() {module->copyTrack(2);}));
				menu->addChild(createMenuItem("Copy Sequence 4", "", [=]() {module->copyTrack(3);}));
				menu->addChild(createMenuItem("Copy Sequence 5", "", [=]() {module->copyTrack(4);}));
				menu->addChild(createMenuItem("Copy Sequence 6", "", [=]() {module->copyTrack(5);}));
				menu->addChild(createMenuItem("Copy Sequence 7", "", [=]() {module->copyTrack(6);}));
				menu->addChild(createMenuItem("Copy Sequence 8", "", [=]() {module->copyTrack(7);}));
			}));
			if (stepSeq_clipboard || stepStation_clipboard) {
				menu->addChild(createSubmenuItem("Paste single Sequence to", "", [=](Menu * menu) {
					menu->addChild(createMenuItem("Paste Sequence 1", "", [=]() {module->pasteToTrack(0);}));
					menu->addChild(createMenuItem("Paste Sequence 2", "", [=]() {module->pasteToTrack(1);}));
					menu->addChild(createMenuItem("Paste Sequence 3", "", [=]() {module->pasteToTrack(2);}));
					menu->addChild(createMenuItem("Paste Sequence 4", "", [=]() {module->pasteToTrack(3);}));
					menu->addChild(createMenuItem("Paste Sequence 5", "", [=]() {module->pasteToTrack(4);}));
					menu->addChild(createMenuItem("Paste Sequence 6", "", [=]() {module->pasteToTrack(5);}));
					menu->addChild(createMenuItem("Paste Sequence 7", "", [=]() {module->pasteToTrack(6);}));
					menu->addChild(createMenuItem("Paste Sequence 8", "", [=]() {module->pasteToTrack(7);}));
				}));
			} else {
				menu->addChild(createMenuLabel("Paste single Sequence"));
			}
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("DISK operations", "", [=](Menu * menu) {
			
			menu->addChild(createMenuItem("Load preset", "", [=]() {module->menuLoadStepStationPreset();}));
			menu->addChild(createMenuItem("Save preset", "", [=]() {module->menuSaveStepStationPreset();}));

/*
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Import stepSeq+ preset", "", [=]() {module->menuLoadSingle();}));
			menu->addChild(createMenuItem("Export stepSeq+ preset", "", [=]() {module->menuSaveSingle();}));
*/

			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Import All Sequences", "", [=]() {module->menuLoadAllSequences();}));
			menu->addChild(createMenuItem("Export All Sequences", "", [=]() {module->menuSaveAllSequences();}));
			menu->addChild(new MenuSeparator());
			menu->addChild(createSubmenuItem("Import stepSeq seq. to:", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("Track 1", "", [=]() {module->menuLoadSequence(0);}));
				menu->addChild(createMenuItem("Track 2", "", [=]() {module->menuLoadSequence(1);}));
				menu->addChild(createMenuItem("Track 3", "", [=]() {module->menuLoadSequence(2);}));
				menu->addChild(createMenuItem("Track 4", "", [=]() {module->menuLoadSequence(3);}));
				menu->addChild(createMenuItem("Track 5", "", [=]() {module->menuLoadSequence(4);}));
				menu->addChild(createMenuItem("Track 6", "", [=]() {module->menuLoadSequence(5);}));
				menu->addChild(createMenuItem("Track 7", "", [=]() {module->menuLoadSequence(6);}));
				menu->addChild(createMenuItem("Track 8", "", [=]() {module->menuLoadSequence(7);}));
			}));
			
			menu->addChild(createSubmenuItem("Export stepSeq seq. from:", "", [=](Menu * menu) {
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

		menu->addChild(createSubmenuItem("Reset default USER settings", "", [=](Menu * menu) {
			menu->addChild(createSubmenuItem("Are you Sure?", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("RESET!", "", [=]() {module->resetUserSettings();}));
			}));
		}));

		menu->addChild(createSubmenuItem("Erase ALL PROGS", "", [=](Menu * menu) {
			menu->addChild(createSubmenuItem("Are you Sure?", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("ERASE!", "", [=]() {module->eraseProgs();}));
			}));
		}));
		
		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Tips", "", [=](Menu * menu) {
			menu->addChild(createMenuLabel("Rclick on user controls (U1/U2) or Track# to config"));
			menu->addChild(createMenuLabel("Click or Rclick on DIV/MULT or MODE displays"));
			menu->addChild(createMenuLabel("Adjust OUTs timing with 'Out DELAY' options"));

			menu->addChild(createMenuLabel("User SW + PW* = SWing only"));
			menu->addChild(createMenuLabel("User SK + PW* = SWing only"));
			
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("Store Programs with double-click on STOR"));
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("Remember to store programs when"));
			menu->addChild(createMenuLabel("changing / importing / pasting sequences"));
		}));

	}

};

Model* modelStepStation = createModel<StepStation, StepStationWidget>("StepStation");