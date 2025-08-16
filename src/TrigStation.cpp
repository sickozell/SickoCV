#include "TrigStation_def.hpp"

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

struct SickoTrigStation : Module {

	//bool alive;

	virtual int getInputBase() const { return -1; }
	virtual int getOutputBase() const { return -1; }
	virtual int getParamBase() const { return -1; }

	const std::string userInNames[KNOB_SHIFT] = {"Change", "Change Prob.", "Length", "Mode", "Out scale", "Reset Step#", "Retrig", "Reverse", "Run", "Skip", "Skip Prob."};
	const std::string userKnobNames[KNOB_NR] = {"Change Prob.", "Mode", "Out scale", "Reset Step#", "Retrig Prob.", "Skip Prob.", "Attenuator", "Attenuverter"};

	const std::string sampleDelayNames[7] = {"No Delay", "1 sample", "2 samples", "3 samples", "4 samples", "5 samples", "Default"};

	int sampleDelay[ALLTRACKS] = {6, 6, 6, 6, 6, 6, 6, 6, 0};

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
//						    IN_CHANGE IN_FLPPRB IN_LENGH IN_MODE IN_OUTSC IN_RSTST IN_RETRIG IN_REV   IN_RUN   IN_SKIP IN_SKIPRB KN_FLIP  KN_MODE KN_OUTSC KN_RSTST  KN_PROB KN_SKIP KN_ATN  KN_ATNV
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0} }
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
/*
			std::string label = "PARAM " + std::to_string(inputIndex);
			std::string label2 = "TRACK " + std::to_string(t);
			std::string label3 = "COL " + std::to_string(column);

			menu->addChild(createMenuLabel(label));
			menu->addChild(createMenuLabel(label2));
			menu->addChild(createMenuLabel(label3));
*/
			SickoTrigStation* module = this;

			struct UserInputItem : MenuItem {
				SickoTrigStation* module;
				int userPortType;
				int tr;
				int inputIndex;
				int column;

				UserInputItem(SickoTrigStation* m, int knobType, int track, int index, int col) {
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
/*
	void appendClockInMenu(Menu *menu, engine::Port::Type type, int portId){

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("CV clock IN", "", &cvClockIn));
			
	}

	void appendClockOutMenu(Menu *menu, engine::Port::Type type, int portId){


		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("CV clock OUT", "", &cvClockOut));
		
	}
*/
	void appendOutMenu(Menu *menu, engine::Port::Type type, int portId){

		menu->addChild(new MenuSeparator());

		int outputBase = getOutputBase();

		if (outputBase >= 0) {
			
			int t = portId - outputBase;

			SickoTrigStation* module = this;

			struct SampleDelayItem : MenuItem {
				SickoTrigStation* module;
				int menuValue;
				int t;

				SampleDelayItem(SickoTrigStation* m, int value, int track) {
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
			menu->addChild(createSubmenuItem("Track DELAY:", (module->sampleDelayNames[module->sampleDelay[t]]), [=](Menu* menu) {
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
			SickoTrigStation* module = this;

			struct UserKnobItem : MenuItem {
				SickoTrigStation* module;
				int userKnobType;
				int tr;
				int knobIndex;
				int column;

				UserKnobItem(SickoTrigStation* m, int knobType, int track, int index, int col) {
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
TWidget* createStationParamCentered(math::Vec pos, engine::Module* module, int paramId){
  return createParamCentered<TWidget>(pos, module, paramId);
}

template <class TWidget>
TWidget* createStationInputCentered(math::Vec pos, engine::Module* module, int paramId){
  return createInputCentered<TWidget>(pos, module, paramId);
}
/*
template <class TWidget>
TWidget* createStationClockInCentered(math::Vec pos, engine::Module* module, int paramId){
	//SickoTrigStation* mod = dynamic_cast<SickoTrigStation*>(module);
	//mod->alive = true;
  return createInputCentered<TWidget>(pos, module, paramId);
}
*/
template <class TWidget>
TWidget* createStationClockOutCentered(math::Vec pos, engine::Module* module, int paramId){
	//SickoTrigStation* mod = dynamic_cast<SickoTrigStation*>(module);
	//mod->alive = true;
  return createOutputCentered<TWidget>(pos, module, paramId);
}


struct SickoKnobStation : SickoTrimpot {
	void appendContextMenu(Menu* menu) override {
	    if (module) {
	      dynamic_cast<SickoTrigStation*>(this->module)->appendParamMenu(menu, this->paramId);
	    }
	  }
};

struct SickoInputStation : SickoInPort {
	void appendContextMenu(Menu* menu) override {
		if (this->module)
			dynamic_cast<SickoTrigStation*>(this->module)->appendInputMenu(menu, this->type, this->portId);
	}
};
/*
struct SickoClockInStation : SickoInPort {
	void appendContextMenu(Menu* menu) override {
		if (this->module)
			dynamic_cast<SickoTrigStation*>(this->module)->appendClockInMenu(menu, this->type, this->portId);
	}
};

struct SickoClockOutStation : SickoOutPort {
	void appendContextMenu(Menu* menu) override {
		if (this->module)
			dynamic_cast<SickoTrigStation*>(this->module)->appendClockOutMenu(menu, this->type, this->portId);
	}
};
*/
struct SickoTrigOutStation : SickoOutPort {
	void appendContextMenu(Menu* menu) override {
		if (this->module)
			dynamic_cast<SickoTrigStation*>(this->module)->appendOutMenu(menu, this->type, this->portId);
	}
};


/*
struct tpDivMult : ParamQuantity {
	std::string getDisplayValueString() override {
		const std::string valueDisplay[45] = {"/256", "/128", "/64", "/48", "/32", "/24", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "x1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x24", "x32", "x48", "x64", "x128", "x256"};
		return valueDisplay[int(getValue())];
	}
};
*/

//struct TrigStation : Module {
struct TrigStation : SickoTrigStation {
	enum ParamId {
		//BPM_KNOB_PARAM,
		//INTCLOCKBUT_PARAM,
		SEQRUN_PARAM,
		RSTALLBUT_PARAM,
		PROG_PARAM,
		SET_PARAM,
		AUTO_PARAM,
		RECALL_PARAM,
		STORE_PARAM,
		//ENUMS(DIVMULT_KNOB_PARAM, 8),
		ENUMS(LENGTH_PARAM, 8),
		ENUMS(USER_PARAM, 16),
		ENUMS(STEP_PARAM, 128),

		PARAMS_LEN
	};
	enum InputId {
		EXTCLOCK_INPUT,
		//RUN_INPUT,
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
		//CLOCK_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(STEPBUT_LIGHT, 128),
		ENUMS(STEP_LIGHT, 128),
		//RUNBUT_LIGHT,
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

	//bool waitClock2RstSeq[MAXTRACKS] = {false, false, false, false, false, false, false, false};

	float clkValue[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};
	float prevClkValue[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};

	float rstValue[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevRstValue[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	bool direction[MAXTRACKS] = {FORWARD, FORWARD, FORWARD, FORWARD, FORWARD, FORWARD, FORWARD, FORWARD};
	float revVolt[MAXTRACKS] = {};
	float prevRevVolt[MAXTRACKS] = {};

	float out[MAXTRACKS] = {0,0,0,0,0,0,0,0};

	float volt[MAXTRACKS] = {0,0,0,0,0,0,0,0};

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

/*
	bool internalClock = false;
	bool prevInternalClock = false;
	bool externalClock = false;
	bool prevExternalClock = false;

	float buttRunTrig = 0.f;
	float prevButtRunTrig = -1.f;
*/
	int recStep[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};

	int runType = RUN_GATE;

	int maxSteps[MAXTRACKS] = {0,0,0,0,0,0,0,0};

	int currAddr[MAXTRACKS] = {0,0,0,0,0,0,0,0};
	int prevAddr[MAXTRACKS] = {0,0,0,0,0,0,0,0};

//	bool rstClkOnRst = true;
	bool rstSeqOnProgChange = true;

	bool dontAdvance[MAXTRACKS] = {false, false, false, false, false, false, false, false};

	

	// ************************************************************************************
	
	#include "TrigStation_init.hpp"
	
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
/*
	const std::string divMultDisplay[45] = {"/ 256", "/ 128", "/ 64", "/ 48", "/ 32", "/ 24", "/ 17", "/ 16", "/ 15", "/ 14", "/ 13", "/ 12", "/ 11", "/ 10", "/ 9", "/ 8", "/ 7", "/ 6", "/ 5", "/ 4", "/ 3", "/ 2",
	 					"X 1", "X 2", "X 3", "X 4", "X 5", "X 6", "X 7", "X 8", "X 9", "X 10", "X 11", "X 12", "X 13", "X 14", "X 15", "X 16", "X 17", "X 24", "X 32", "X 48", "X 64", "X 128", "X 256"};

	const float divMult[45] = {256, 128, 64, 48, 32, 24, 17, 16, 15, 14, 13, 12, 11, 10, 9 , 8, 7, 6, 5, 4, 3, 2, 1,
							2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 24, 32, 48, 64, 128, 256};
*/
	//**************************************************************

	// ********* C L O C K *************

	double sampleRateCoeff = (double)APP->engine->getSampleRate() * 60;

/*
	double bpm[ALLTRACKS] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
	double extBpm = 0.1;
	double prevBpm[ALLTRACKS] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
	double prevExtBpm = -1;
*/
	float extTrigValue[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevExtTrigValue[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	float resetBut = 0;
	float resetValue[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevResetValue[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	bool resetStart = true;
	bool resetStartExt = true;
	//bool resetDiv = true;
/*
	double clockSample[ALLTRACKS] = {1, 1, 1, 1, 1, 1, 1, 1, 999999999999};
	double clockMaxSample[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	double maxPulseSample[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	double extClockSample[ALLTRACKS] = {999999999999, 999999999999,  999999999999,  999999999999,  999999999999,  999999999999,  999999999999,  999999999999,  999999999999};
	double extClockMaxSample[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	double extMaxPulseSample[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	
	int ppqnTable[7] = {1, 2, 4, 8, 12, 16, 24}; // 1 2 4 8 12 16 24
	int ppqn = PPQN1;
	int tempPpqn = ppqn;
	bool ppqnChange = false;

	int ppqnValue = ppqnTable[ppqn];
	int ppqnComparison = ppqnValue - 1;

	int pulseNr = 0;
*/
	float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	//float oneMsTime = (APP->engine->getSampleRate()) / 10;	// for testing purposes
	bool resetPulse[ALLTRACKS] = {false, false, false, false, false, false, false, false, false};
	float resetPulseTime[ALLTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	bool resetOnRun = true;
	bool resetPulseOnRun = false;
	bool resetOnStop = false;
	bool resetPulseOnStop = false;
/*
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
*/
	bool extConn[MAXTRACKS] = {false, false, false, false, false, false, false, false};
	bool clockAdv[ALLTRACKS] = {false, false, false, false, false, false, false, false, false};
	bool prevClockAdv[ALLTRACKS] = {false, false, false, false, false, false, false, false, false};
/*
	bool edge[ALLTRACKS] = {false, false, false, false, false, false, false, false, false};
	bool prevEdge[ALLTRACKS] = {false, false, false, false, false, false, false, false, false};
*/
	bool stepAdv[MAXTRACKS] = {false, false, false, false, false, false, false, false};
	int delayReadPos;

	bool clkBuff[MAXTRACKS][5] = {{false, false, false, false, false}, {false, false, false, false, false}, {false, false, false, false, false}, {false, false, false, false, false}, {false, false, false, false, false}, {false, false, false, false, false}, {false, false, false, false, false}, {false, false, false, false, false}};
	int clkBuffPos = 0;


	// ------------- TURING MODE

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
	
	const int bitResTable[2] = {8, 16};
	int bitResolution = BIT_8;

	bool stepPulse[MAXTRACKS] = {false, false, false, false, false, false, false, false};
	float stepPulseTime[MAXTRACKS] = {0,0,0,0,0,0,0,0};
	bool outGate[MAXTRACKS] = {false, false, false, false, false, false, false, false};

	int outType[MAXTRACKS] = {0,0,0,0,0,0,0,0};

//	bool divControls = true;
	bool modeControls = true;

	float rstLightValue = 0.f;
	float lightDecayPerSample = 1.f / (APP->engine->getSampleRate() * 0.2f);

	TrigStation() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		struct UserQuantity : ParamQuantity {
			int t = -1;  // sarà assegnato dopo la creazione
			int userColumn = 1;  // 1 per default (primo knob)

			float getDisplayValue() override {
				TrigStation* module = reinterpret_cast<TrigStation*>(this->module);

				if (t >= 0) {
					switch (module->userTable[t][userColumn]) {
/*
						case KNOB_PW:
				 			name = "Pulse Width #" + to_string(t+1);
							unit = "%";
							displayMultiplier = 99.f;
							displayOffset = 0.f;
						break;
*/
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

						case KNOB_SKIPPROB:
				 			name = "Skip Prob. #" + to_string(t+1);
							unit = "%";
							displayMultiplier = 100.f;
							displayOffset = 0.f;
						break;	
/*
						case KNOB_SWING:
				 			name = "Swing #" + to_string(t+1);
							unit = "%";
							displayMultiplier = 100.f;
							displayOffset = 0.f;
						break;	
*/
						case KNOB_ATN:
				 			name = "Atten. #" + to_string(t+1);
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

									
		
				 	}
				}
				return ParamQuantity::getDisplayValue();
			}
		};

		struct UserLabel : PortInfo {
			int t = -1;  // sarà assegnato dopo la creazione
			int userColumn = 1;  // 1 per default (primo knob)

			std::string getName() override {
				TrigStation* module = reinterpret_cast<TrigStation*>(this->module);

				if (t >= 0) {
					switch (module->userTable[t][userColumn]) {

//						case IN_PW:			name = "Pulse Width #" + to_string(t+1); break;
				 		case IN_REV:		name = "Reverse #" + to_string(t+1); break;
				 		case IN_MODE:		name = "Mode #" + to_string(t+1); break;
						case IN_LENGTH:		name = "Length #" + to_string(t+1);	break;
						case IN_RSTSTEP:	name = "Reset Step# #" + to_string(t+1); break;
						case IN_OUTSCALE:	name = "Out scale #" + to_string(t+1); break;
						case IN_RUN:		name = "Run #" + to_string(t+1); break;
						case IN_RETRIG:		name = "Retrig #" + to_string(t+1);	break;
						case IN_SKIP:		name = "Skip #" + to_string(t+1); break;
						case IN_SKIPPROB:	name = "Skip Prob.#" + to_string(t+1); break;
//						case IN_SWING:		name = "Swing #" + to_string(t+1); break;
						case IN_CHANGE:		name = "Change #" + to_string(t+1); break;
						case IN_CHANGEPROB:	name = "Change Prob.#" + to_string(t+1); break;
				 	}
				}
				return PortInfo::getName();
			}
		};

		//configParam(BPM_KNOB_PARAM, 300.f, 3000.f, 1200.f, "Int.Clk", " bpm", 0, 0.1);
		//paramQuantities[BPM_KNOB_PARAM]->snapEnabled = true;

		configInput(EXTCLOCK_INPUT,"External Clock");

		//configSwitch(INTCLOCKBUT_PARAM, 0.f, 1.f, 0.f, "Int.Clock Run", {"0", "1"});
		//configInput(RUN_INPUT, "Int.Clock");
		
		configSwitch(SEQRUN_PARAM, 0.f, 1.f, 0.f, "Seq. run", {"0", "1"});
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

//		configOutput(CLOCK_OUTPUT,"Clock");

		for (int t = 0; t < MAXTRACKS; t++) {
			
			configInput(CLK_INPUT+t, ("Clock#"+to_string(t+1)).c_str());

			//configParam<tpDivMult>(DIVMULT_KNOB_PARAM+t, 0.f, 44.f, 22.f, ("Mult/Div #"+to_string(t+1)).c_str());
			//paramQuantities[DIVMULT_KNOB_PARAM+t]->snapEnabled = true;

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

			for (int s = 0; s < 16; s++)
				configSwitch(STEP_PARAM+(t*16)+s, 0.f, 1.f, 0.f, ("Tr.#"+to_string(t+1)+" Step#"+to_string(s+1)).c_str(), {"OFF", "ON"});
			

			/*	// init non funziona qui. ma da riprovare
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
			lights[STEPBUT_LIGHT+(t*16)].setBrightness(0);
			wSeq[t][0] = 0.f;
			for (int s = 1; s < 16; s++) {
				lights[STEP_LIGHT+(t*16+s)].setBrightness(0);
				lights[STEPBUT_LIGHT+(t*16)+s].setBrightness(0);
				wSeq[t][s] = 0.f;
			}
		}
		
		setButLight = false;
		setButLightValue = 0.f;


		// ----- clock
		resetStart = true;
		resetStartExt = true;
		//resetDiv = true;
		
		//clock_runSetting = 1;
		//clock_prevRunSetting = 0;

		runButton = 0;
		runTrig = 0.f;
		prevRunTrig = 0.f;

		//resetOnRun = true;
		//resetPulseOnRun = false;
		//resetOnStop = false;
		//resetPulseOnStop = false;
/*
		bpm[MC] = 0;
		clockSample[MC] = 1.0;
		extSync[MC] = false;
		extConn[MC] = false;
		prevExtConn[MC] = true;
		extBeat[MC] = false;
*/
		progKnob = 0;
		prevProgKnob = 0;
		savedProgKnob = 0;
		selectedProg = 0;
		progChanged = false;
		workingProg = 0;


		for (int t = 0; t < MAXTRACKS; t++) {
			wSteps[t] = 16;
			params[LENGTH_PARAM+t].setValue(wSteps[t]);
/*			
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
*/
			if (dontAdvanceSetting[t] == 2)
				dontAdvance[t] = dontAdvanceSetting[MC];
			else
				dontAdvance[t] = dontAdvanceSetting[t];
/*
			edge[t] = false;
			prevEdge[t] = false;
*/
			stepAdv[t] = false;
			clockAdv[t] = false;
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

		lightDecayPerSample = 1.f / (APP->engine->getSampleRate() * 0.2f);
	}

	
	#include "TrigStation_json.hpp"
	

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
			trigStation_cbSteps[t] = wSteps[t];
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
			trigStation_cbUserTableTrack[i] = userTable[t][i];

		for (int i = 0; i < MAXUSER; i++)
			for (int j = 0; j < 2; j++)
				trigStation_cbUserInputsTrack[i][j] = userInputs[t][i][j];

		trigStation_cbUserValuesTrack[0] = params[USER_PARAM+t].getValue();
		trigStation_cbUserValuesTrack[1] = params[USER_PARAM+t+8].getValue();

		trigStation_cbCurrentModeTrack = currentMode[t];
		//trigStation_cbDivMultTrack = params[DIVMULT_KNOB_PARAM+t].getValue();
		
		trigStation_cbXcludeFromRunTrack = xcludeFromRun[t];
		trigStation_cbXcludeFromRstTrack = xcludeFromRst[t];

		trigStation_cbTuringModeTrack = turingMode[t];
		trigStation_cbOutTypeSettingTrack = outTypeSetting[t];
		//trigStation_cbRevTypeTrack = revType[t];
		trigStation_cbDontAdvanceSettingTrack = dontAdvanceSetting[t];
		trigStation_cbRstStepsWhenTrack = rstStepsWhen[t];

		trigStation_clipboardTrack = true;
	}

	void copyUserSettings() {
		for (int t = 0; t < MAXTRACKS; t++) {

			for (int i = 0; i < 4; i++)
				trigStation_cbUserTable[t][i] = userTable[t][i];

			for (int i = 0; i < MAXUSER; i++)
				for (int j = 0; j < 2; j++)
					trigStation_cbUserInputs[t][i][j] = userInputs[t][i][j];

			trigStation_cbUserValues[t][0] = params[USER_PARAM+t].getValue();
			trigStation_cbUserValues[t][1] = params[USER_PARAM+t+8].getValue();

			trigStation_cbCurrentMode[t] = currentMode[t];
			//trigStation_cbDivMult[t] = params[DIVMULT_KNOB_PARAM+t].getValue();;

			trigStation_cbTuringMode[t] = turingMode[t];
			trigStation_cbOutTypeSetting[t] = outTypeSetting[t];
			
			trigStation_cbXcludeFromRun[t] = xcludeFromRun[t];
			trigStation_cbXcludeFromRst[t] = xcludeFromRst[t];

		}

		for (int t = 0; t < ALLTRACKS; t++) {
			//trigStation_cbRevType[t] = revType[t];
			trigStation_cbDontAdvanceSetting[t] = dontAdvanceSetting[t];
			trigStation_cbRstStepsWhen[t] = rstStepsWhen[t];
		}
		trigStation_cbSeqRunSetting = seqRunSetting;
//		trigStation_cbInternalClock = internalClock;
//		trigStation_cbBpmKnob = params[BPM_KNOB_PARAM].getValue();

		trigStation_userClipboard = true;
	}

	void copyPanel() {

		copyAllTracks();
		copyUserSettings();
		trigStation_clipboard = true;

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

	void pasteToTrackRandLoops(int t) {
/*
		for (int s = 0; s < 16; s++) {
			wSeq[t][s] = stepSeq_cbSeq[s];
			params[STEP_PARAM+(t*16)+s].setValue(wSeq[t][s]);
		}
		wSteps[t] = stepSeq_cbSteps;
		params[LENGTH_PARAM+t].setValue(wSteps[t]);

		if (userInputs[t][KNOB_RSTSTEP][0])
			params[USER_PARAM+t+userInputs[t][KNOB_RSTSTEP][1]].setValue((float)stepSeq_cbRst / 15);
*/
		for (int st = 0; st < 16; st++) {
			wSeq[t][st] = randLoops_cbSeq[st];
			params[STEP_PARAM+(t*16)+st].setValue(wSeq[t][st]);
		}
		
		wSteps[t] = randLoops_cbSteps;
		params[LENGTH_PARAM].setValue(wSteps[t]);

	}

	void pasteAllTracks() {

		for (int t = 0; t < MAXTRACKS; t++) {
			for (int s = 0; s < 16; s++)
				params[STEP_PARAM+(t*16)+s].setValue(stepSeq8_cbSeq[t][s]);
			params[LENGTH_PARAM+t].setValue(stepSeq8_cbSteps);

			if (trigStation_clipboard) {
				params[LENGTH_PARAM+t].setValue(trigStation_cbSteps[t]);
			}
			if (userInputs[t][KNOB_RSTSTEP][0]) {
				params[USER_PARAM+t+userInputs[t][KNOB_RSTSTEP][1]].setValue((float)stepSeq8_cbRst / 15);
			}
		}

	}

	void pasteUser(int t) {

		params[USER_PARAM+t].setValue(trigStation_cbUserValuesTrack[0]);
		params[USER_PARAM+t+8].setValue(trigStation_cbUserValuesTrack[1]);

		for (int i = 0; i < 4; i++)
			userTable[t][i] = trigStation_cbUserTableTrack[i];

		for (int i = 0; i < MAXUSER; i++)
			for (int j = 0; j < 2; j++)
				userInputs[t][i][j] = trigStation_cbUserInputsTrack[i][j];

		params[USER_PARAM+t].setValue(trigStation_cbUserValuesTrack[0]);
		params[USER_PARAM+t+8].setValue(trigStation_cbUserValuesTrack[1]);

		currentMode[t] = trigStation_cbCurrentModeTrack;
		//params[DIVMULT_KNOB_PARAM+t].setValue(trigStation_cbDivMultTrack);
		
		xcludeFromRun[t] = trigStation_cbXcludeFromRunTrack;
		xcludeFromRst[t] = trigStation_cbXcludeFromRstTrack;

		turingMode[t] = trigStation_cbTuringModeTrack;
		outTypeSetting[t] = trigStation_cbOutTypeSettingTrack;

		//revType[t] = trigStation_cbRevTypeTrack;
		dontAdvanceSetting[t] = trigStation_cbDontAdvanceSettingTrack;
		rstStepsWhen[t] = trigStation_cbRstStepsWhenTrack;

	}

	void pasteUserSettings() {
		for (int t = 0; t < MAXTRACKS; t++) {
			params[USER_PARAM+t].setValue(trigStation_cbUserValues[t][0]);
			params[USER_PARAM+t+8].setValue(trigStation_cbUserValues[t][1]);

			for (int i = 0; i < 4; i++)
				userTable[t][i] = trigStation_cbUserTable[t][i];

			for (int i = 0; i < MAXUSER; i++)
				for (int j = 0; j < 2; j++)
					userInputs[t][i][j] = trigStation_cbUserInputs[t][i][j];

			params[USER_PARAM+t].setValue(trigStation_cbUserValues[t][0]);
			params[USER_PARAM+t+8].setValue(trigStation_cbUserValues[t][1]);

			turingMode[t] = trigStation_cbTuringMode[t];

			currentMode[t] = trigStation_cbCurrentMode[t];
			//params[DIVMULT_KNOB_PARAM+t].setValue(trigStation_cbDivMult[t]);
			
			xcludeFromRun[t] = trigStation_cbXcludeFromRun[t];
			xcludeFromRst[t] = trigStation_cbXcludeFromRst[t];

		}

		for (int t = 0; t < ALLTRACKS; t++) {
			outTypeSetting[t] = trigStation_cbOutTypeSetting[t];
			//revType[t] = trigStation_cbRevType[t];
			dontAdvanceSetting[t] = trigStation_cbDontAdvanceSetting[t];
			rstStepsWhen[t] = trigStation_cbRstStepsWhen[t];
		}

		seqRunSetting = trigStation_cbSeqRunSetting;
//		internalClock = trigStation_cbInternalClock;
//		params[BPM_KNOB_PARAM].setValue(trigStation_cbBpmKnob);
	}

	void pastePanel() {

		pasteUserSettings();

		pasteAllTracks();

	}

	void eraseProgs() {

		for (int p = 0; p < 32; p++) {

			for (int t = 0; t < MAXTRACKS; t++) {
				for (int s = 0; s < 16; s++)
					progSeq[p][t][s] = 0.f;
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
				//progDivMult[p][t] = 22;
				progXcludeFromRun[p][t] = false;
				progXcludeFromRst[p][t] = false;
				progUserValues[p][t][0] = 0.f;
				progUserValues[p][t][1] = 0.f;
				progTuringMode[p][t] = 2;
				progOutTypeSetting[p][t] = 3;
				//progRevType[p][t] = 2;
				progDontAdvanceSetting[p][t] = 2;
				progRstStepsWhen[p][t] = 3;	
				progSampleDelay[p][t] = 6;
			}

			progOutTypeSetting[p][MC] = 0;
			//progRevType[p][MC] = POSITIVE_V;
			progDontAdvanceSetting[p][MC] = 1;
			progRstStepsWhen[p][MC] = 1;
			progSampleDelay[p][MC] = 0;

			progSeqRunSetting[p] = 1;
//			progInternalClock[p] = 1;
//			progBpmKnob[p] = 1200.f;

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

		rstLightValue = 1.f;
	}

	void inline resetTrackSteps(int t) {

		lights[STEP_LIGHT+(t*16+step[t])].setBrightness(0);

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

	void inline calcVoltage(int t) {

		int startCursor = maxSteps[t] - step[t];
		int lengthCursor = 0;

		if (startCursor >= maxSteps[t])
			startCursor = 0;

		int tempCursor = startCursor;

		volt[t] = 0.f;

		for (int i=0; i < 16; i++) {
			if (lengthCursor >= maxSteps[t]) {
				lengthCursor = 0;
				tempCursor = startCursor;
			}
			
			if (tempCursor >= maxSteps[t])
				tempCursor = 0;

			if (wSeq[t][tempCursor])
				volt[t] += tableVolts[progression][bitResolution][i];

			tempCursor++;
			lengthCursor++;
		}
		
		if (volt[t] > 10.f)
			volt[t] = 10.f;

	}

	void scanLastProg() {
		lastProg = 31;
		bool exitFunc = false;

		for (int p = 31; p >= 0; p--) {
			for (int t = 0; t < MAXTRACKS; t++) {
				for (int st = 0; st < 16; st++) {
					if (progSeq[p][t][st] != 0.5f) {
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

	void initializeSequence(int t) {
		for (int s = 0; s < 16; s++) {
			wSeq[t][s] = 0.f;
			params[STEP_PARAM+(t*16)+s].setValue(wSeq[t][s]);
		}
	}

	void initializeAll() {
		for (int t = 0; t < MAXTRACKS; t++)
			initializeSequence(t);
	}

	void randomizeSequence(int t) {

		for (int s = 0; s < 16; s++) {
			if (random::uniform() > .5f)
				params[STEP_PARAM+(t*16)+s].setValue(1.f);
			else
				params[STEP_PARAM+(t*16)+s].setValue(0.f);
		}
	}

	void randomizeAll() {
		for (int t = 0; t < MAXTRACKS; t++)
			randomizeSequence(t);
	}
/*
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
			float tempSwing = inputs[USER_INPUT+t+tempUserIn].getVoltage() * 0.1;

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

*/
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
			
			float tempOutScale = 1.f;

			if (inputs[USER_INPUT+t+tempUserIn].isConnected()) {

				tempOutScale = inputs[USER_INPUT+t+tempUserIn].getVoltage() * 0.1f;

				if (tempUserIn == 0) {
					if (userTable[t][1] == KNOB_ATN)
						tempOutScale *= params[USER_PARAM+t].getValue();
					else if (userTable[t][1] == KNOB_ATNV)
						tempOutScale *= params[USER_PARAM+t].getValue() * 2.f - 1.f;
				} else { // altrimenti è 8
					if (userTable[t][3] == KNOB_ATN)
						tempOutScale *= params[USER_PARAM+t+8].getValue();
					else if (userTable[t][3] == KNOB_ATNV)
						tempOutScale *= params[USER_PARAM+t+8].getValue() * 2.f - 1.f;
				}

				// outScale knob is added to input (or is to prefer that outScale knob scales the CV input?)

				if (userInputs[t][KNOB_OUTSCALE][0])
					tempOutScale += params[USER_PARAM+t+userInputs[t][KNOB_OUTSCALE][1]].getValue();
					//tempOutScale *= params[USER_PARAM+t+userInputs[t][KNOB_OUTSCALE][1]].getValue();

				if (tempOutScale < 0.f)
					tempOutScale = 0.f;
				else if (tempOutScale > 1.f)
					tempOutScale = 1.f;
			}

			return outVal * tempOutScale;

		} else if (userInputs[t][KNOB_OUTSCALE][0]) {
			return outVal * params[USER_PARAM+t+userInputs[t][KNOB_OUTSCALE][1]].getValue();
		} else {
			return outVal;
		}

	}

	bool inline calcSkip(int t) {
		bool skip = false;

		if (userInputs[t][IN_SKIP][0]) {
			if (userInputs[t][KNOB_SKIPPROB][0]) {
				if (inputs[USER_INPUT+t+userInputs[t][IN_SKIP][1]].isConnected() && 
					inputs[USER_INPUT+t+userInputs[t][IN_SKIP][1]].getVoltage() >= 1.f) {

					if (random::uniform() <= params[USER_PARAM+t+userInputs[t][KNOB_SKIPPROB][1]].getValue())
						skip = true;
					
				}
			} else {
				if (inputs[USER_INPUT+t+userInputs[t][IN_SKIP][1]].isConnected() && 
					inputs[USER_INPUT+t+userInputs[t][IN_SKIP][1]].getVoltage() >= 1.f) {

					skip = true;

				}
			}

		} else {

			if (userInputs[t][IN_SKIPPROB][0]) {
				if (userInputs[t][KNOB_SKIPPROB][0]) {

					if (inputs[USER_INPUT+t+userInputs[t][IN_SKIPPROB][1]].isConnected()) {

						float tempValue = inputs[USER_INPUT+t+userInputs[t][IN_SKIPPROB][1]].getVoltage() * 0.1f + 
							params[USER_PARAM+t+userInputs[t][KNOB_SKIPPROB][1]].getValue();

						int tempUserIn = userInputs[t][IN_SKIPPROB][1];  // 0 oppure 8 a seconda che l'input sia U1 o U2

						if (tempValue < 0.f)
							tempValue = 0.f;
						else if (tempValue > 1.f)
							tempValue = 1.f;

						if (tempUserIn == 0) {
							if (userTable[t][1] == KNOB_ATN)
								tempValue *= params[USER_PARAM+t].getValue();
							else if (userTable[t][1] == KNOB_ATNV)
								tempValue *= params[USER_PARAM+t].getValue() * 2 - 1;
						} else { // altrimenti è 8
							if (userTable[t][3] == KNOB_ATN)
								tempValue *= params[USER_PARAM+t+8].getValue();
							else if (userTable[t][3] == KNOB_ATNV)
								tempValue *= params[USER_PARAM+t+8].getValue() * 2 - 1;
						}

						if (random::uniform() <= tempValue)
							skip = true;

					} else if (userInputs[t][KNOB_SKIPPROB][0] && random::uniform() <= params[USER_PARAM+t+userInputs[t][KNOB_SKIPPROB][1]].getValue()) {
						skip = true;
					}

				} else if (inputs[USER_INPUT+t+userInputs[t][IN_SKIPPROB][1]].isConnected()) {

					float tempValue = inputs[USER_INPUT+t+userInputs[t][IN_SKIPPROB][1]].getVoltage() * 0.1f;
					int tempUserIn = userInputs[t][IN_SKIPPROB][1];  // 0 oppure 8 a seconda che l'input sia U1 o U2

					if (tempValue < 0.f)
						tempValue = 0.f;
					else if (tempValue > 1.f)
						tempValue = 1.f;

					if (tempUserIn == 0) {
						if (userTable[t][1] == KNOB_ATN)
							tempValue *= params[USER_PARAM+t].getValue();
						else if (userTable[t][1] == KNOB_ATNV)
							tempValue *= params[USER_PARAM+t].getValue() * 2 - 1;
					} else { // altrimenti è 8
						if (userTable[t][3] == KNOB_ATN)
							tempValue *= params[USER_PARAM+t+8].getValue();
						else if (userTable[t][3] == KNOB_ATNV)
							tempValue *= params[USER_PARAM+t+8].getValue() * 2 - 1;
					}

					if (random::uniform() <= tempValue)
						skip = true;
				}
			} else if (userInputs[t][KNOB_SKIPPROB][0] && random::uniform() <= params[USER_PARAM+t+userInputs[t][KNOB_SKIPPROB][1]].getValue()) {
				skip = true;
			}
		}
		/*
		if (skip && random::uniform() > .5f)
			skip = false;
		*/
		return skip;
	}

	void inline calcChange(int t) {

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
					
		//if (change && random::uniform() > .5f) {
		if (change) {
			
			if (wSeq[t][step[t]] == 0)
				wSeq[t][step[t]] = 1;
			else
				wSeq[t][step[t]] = 0;
		
			params[STEP_PARAM+(t*16)+step[t]].setValue(wSeq[t][step[t]]);
			lights[STEPBUT_LIGHT+(t*16)+step[t]].setBrightness(wSeq[t][step[t]]);
		
		}

	}
/*
	float inline calcPw(int t) {

		float val = 0.f;

		if (userInputs[t][IN_PW][0]) {

			int tempUserIn = userInputs[t][IN_PW][1];  // 0 oppure 8 a seconda che l'input sia U1 o U2
			
			if (inputs[USER_INPUT+t+tempUserIn].isConnected()) {

				val = inputs[USER_INPUT+t+tempUserIn].getVoltage() * 0.099f;

				if (tempUserIn == 0) {
					if (userTable[t][1] == KNOB_ATN)
						val *= params[USER_PARAM+t].getValue();
					else if (userTable[t][1] == KNOB_ATNV)
						val *= params[USER_PARAM+t].getValue() * 2 - 1;
				} else { // altrimenti è 8
					if (userTable[t][3] == KNOB_ATN)
						val *= params[USER_PARAM+t+8].getValue();
					else if (userTable[t][3] == KNOB_ATNV)
						val *= params[USER_PARAM+t+8].getValue() * 2 - 1;
				}

				if (userInputs[t][KNOB_PW][0])
					val += params[USER_PARAM+t+userInputs[t][KNOB_PW][1]].getValue() * 0.99f;

				if (val < 0)
					val = 0.f;
				else if (val > 0.99f)
					val = 0.99f;

			} else if (userInputs[t][KNOB_PW][0]) {

				val = params[USER_PARAM+t+userInputs[t][KNOB_PW][1]].getValue() * 0.99f;

			} else {

				val = 0.5f;

			}

		} else if (userInputs[t][KNOB_PW][0]) {

			val = params[USER_PARAM+t+userInputs[t][KNOB_PW][1]].getValue() * 0.99f;

		} else {

			val = 0.5f;

		}

		return val;
	}
*/
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
					lights[STEPBUT_LIGHT+(t*16)+i].setBrightness(nextSeq[t][i]);
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
				//nextDivMult[t] = progDivMult[selectedProg][t];

				nextTuringMode[t] = progTuringMode[selectedProg][t];

				nextXcludeFromRun[t] = progXcludeFromRun[selectedProg][t];
				nextXcludeFromRst[t] = progXcludeFromRst[selectedProg][t];

			}

			for (int t = 0; t < ALLTRACKS; t++) {
				
				nextOutTypeSetting[t] = progOutTypeSetting[selectedProg][t];
				//nextRevType[t] = progRevType[selectedProg][t];
				nextDontAdvanceSetting[t] = progDontAdvanceSetting[selectedProg][t];
				nextRstStepsWhen[t] = progRstStepsWhen[selectedProg][t];
				nextSampleDelay[t] = progSampleDelay[selectedProg][t];
			}

			nextSeqRunSetting = progSeqRunSetting[selectedProg];
			//nextInternalClock = progInternalClock[selectedProg];
			//nextBpmKnob = progBpmKnob[selectedProg];

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
				for (int i = 0; i < 16; i++) {
					wSeq[t][i] = params[STEP_PARAM+(t*16)+i].getValue();
					lights[STEPBUT_LIGHT+(t*16)+i].setBrightness(wSeq[t][i]);
				}
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

					turingMode[t] = nextTuringMode[t];
					
					currentMode[t] = nextCurrentMode[t];
					//params[DIVMULT_KNOB_PARAM+t].setValue(nextDivMult[t]);
					
					xcludeFromRun[t] = nextXcludeFromRun[t];
					xcludeFromRst[t] = nextXcludeFromRst[t];
					

				}

				for (int t = 0; t < ALLTRACKS; t++) {
					outTypeSetting[t] = nextOutTypeSetting[t];
					//revType[t] = nextRevType[t];
					dontAdvanceSetting[t] = nextDontAdvanceSetting[t];
					rstStepsWhen[t] = nextRstStepsWhen[t];
					sampleDelay[t] = nextSampleDelay[t];
				}

				seqRunSetting = nextSeqRunSetting;
				//internalClock = nextInternalClock;
				//params[BPM_KNOB_PARAM].setValue(nextBpmKnob);

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
					for (int i = 0; i < 16; i++) {
						nextSeq[t][i] = params[STEP_PARAM+(t*16)+i].getValue();
						lights[STEPBUT_LIGHT+(t*16)+i].setBrightness(nextSeq[t][i]);
					}

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
						lights[STEPBUT_LIGHT+(t*16)+i].setBrightness(wSeq[t][i]);
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

					turingMode[t] = progTuringMode[selectedProg][t];
					currentMode[t] = progCurrentMode[selectedProg][t];
					//params[DIVMULT_KNOB_PARAM+t].setValue(progDivMult[selectedProg][t]);
					
					xcludeFromRun[t] = progXcludeFromRun[selectedProg][t];
					xcludeFromRst[t] = progXcludeFromRst[selectedProg][t];
				}

				for (int t = 0; t < ALLTRACKS; t++) {
					outTypeSetting[t] = progOutTypeSetting[selectedProg][t];
					//revType[t] = progRevType[selectedProg][t];
					dontAdvanceSetting[t] = progDontAdvanceSetting[selectedProg][t];
					rstStepsWhen[t] = progRstStepsWhen[selectedProg][t];
					sampleDelay[t] = progSampleDelay[selectedProg][t];
				}

				seqRunSetting = progSeqRunSetting[selectedProg];
				//internalClock = progInternalClock[selectedProg];
				//params[BPM_KNOB_PARAM].setValue(progBpmKnob[selectedProg]);

				workingProg = selectedProg;
				if (progInType == CV_TYPE)
					savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
				else
					savedProgKnob = progKnob;

			} else {

				for (int t = 0; t < MAXTRACKS; t++) {
					for (int i = 0; i < 16; i++) {
						params[STEP_PARAM+(t*16)+i].setValue(wSeq[t][i]);
						lights[STEPBUT_LIGHT+(t*16)+i].setBrightness(wSeq[t][i]);
					}
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

					progTuringMode[progKnob][t] = turingMode[t];
					progCurrentMode[progKnob][t] = currentMode[t];
					//progDivMult[progKnob][t] = params[DIVMULT_KNOB_PARAM+t].getValue();
					
					progXcludeFromRun[progKnob][t] = xcludeFromRun[t];
					progXcludeFromRst[progKnob][t] = xcludeFromRst[t];
				}

				for (int t = 0; t < ALLTRACKS; t++) {
					progOutTypeSetting[progKnob][t] = outTypeSetting[t];
					//progRevType[progKnob][t] = revType[t];
					progDontAdvanceSetting[progKnob][t] = dontAdvanceSetting[t];
					progRstStepsWhen[progKnob][t] = rstStepsWhen[t];
					progSampleDelay[progKnob][t] = sampleDelay[t];
				}

				progSeqRunSetting[progKnob] = seqRunSetting;
				//progInternalClock[progKnob] = internalClock;
				//progBpmKnob[progKnob] = params[BPM_KNOB_PARAM].getValue();

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

																		░██████╗███████╗░██████╗░██╗░░░██╗███████╗███╗░░██╗░█████╗░███████╗██████╗░
																		██╔════╝██╔════╝██╔═══██╗██║░░░██║██╔════╝████╗░██║██╔══██╗██╔════╝██╔══██╗
																		╚█████╗░█████╗░░██║██╗██║██║░░░██║█████╗░░██╔██╗██║██║░░╚═╝█████╗░░██████╔╝
																		░╚═══██╗██╔══╝░░╚██████╔╝██║░░░██║██╔══╝░░██║╚████║██║░░██╗██╔══╝░░██╔══██╗
																		██████╔╝███████╗░╚═██╔═╝░╚██████╔╝███████╗██║░╚███║╚█████╔╝███████╗██║░░██║
																		╚═════╝░╚══════╝░░░╚═╝░░░░╚═════╝░╚══════╝╚═╝░░╚══╝░╚════╝░╚══════╝╚═╝░░╚═╝
*/
		

		// **********  RESET ALL
		// **********  RESET ALL
		// **********  RESET ALL
		// **********  RESET ALL

		resetBut = params[RSTALLBUT_PARAM].getValue();
		if (resetBut)
			resetValue[MC] = 1;
		else
			resetValue[MC] = inputs[RSTALL_INPUT].getVoltage();

		//lights[RSTALLBUT_LIGHT].setBrightness(resetValue[MC]);



		if (resetValue[MC] >= 1 && prevResetValue[MC] < 1) {
			resetAllSteps();
		}

		prevResetValue[MC] = resetValue[MC];

		if (rstLightValue > 0.f) {
			rstLightValue -= lightDecayPerSample;
			if (rstLightValue < 0.f)
				rstLightValue = 0.f;
		}

		lights[RSTALLBUT_LIGHT].setBrightness(rstLightValue);

		// initialize clocks

		clockAdv[MC] = false;

		extTrigValue[MC] = inputs[EXTCLOCK_INPUT].getVoltage();
					
		if (extTrigValue[MC] >= 1.f && prevExtTrigValue[MC] < 1.f) {
			clockAdv[MC] = true;
		}
		prevExtTrigValue[MC] = extTrigValue[MC];

		clkBuffPos++;	
		if (clkBuffPos > 4)
			clkBuffPos = 0;

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
			for (int t = 0; t < MAXTRACKS; t++) {
				if (rstStepsWhen[t] == RST_ONRUN || (rstStepsWhen[t] == RST_DEFAULT && rstStepsWhen[MC] == RST_ONRUN)) {
					//if (!wait2RstSetting)
						resetTrackSteps(t);
					//else
					//	waitClock2RstSeq[t] = true;	// NUOVO CAMBIO
				}
			}

		} else if (!seqRunSetting && prevSeqRunSetting) {
			for (int t = 0; t < MAXTRACKS; t++) {
				if (rstStepsWhen[t] == RST_ONSTOP || (rstStepsWhen[t] == RST_DEFAULT && rstStepsWhen[MC] == RST_ONSTOP)) {
					resetTrackSteps(t);
				}
			}
		}
		prevSeqRunSetting = seqRunSetting;

		lights[SEQRUN_LIGHT].setBrightness(seqRunSetting);

/*

																								████████╗██████╗░░█████╗░░█████╗░██╗░░██╗░██████╗
																								╚══██╔══╝██╔══██╗██╔══██╗██╔══██╗██║░██╔╝██╔════╝
																								░░░██║░░░██████╔╝███████║██║░░╚═╝█████═╝░╚█████╗░
																								░░░██║░░░██╔══██╗██╔══██║██║░░██╗██╔═██╗░░╚═══██╗
																								░░░██║░░░██║░░██║██║░░██║╚█████╔╝██║░╚██╗██████╔╝
																								░░░╚═╝░░░╚═╝░░╚═╝╚═╝░░╚═╝░╚════╝░╚═╝░░╚═╝╚═════╝░
*/

		for (int t = 0; t < MAXTRACKS; t++) {

			if (turingMode[t] && !prevTuringMode[t])
				for (int t = 0; t < 8; t++)
					calcVoltage(t);
			prevTuringMode[t] = turingMode[t];


			if (outTypeSetting[t] == 3) 
				outType[t] = outTypeSetting[MC];
			else
				outType[t] = outTypeSetting[t];

			clockAdv[t] = false;
			stepAdv[t] = false;

			if (inputs[CLK_INPUT+t].isConnected()) {
				extConn[t] = true;
				extTrigValue[t] = inputs[CLK_INPUT+t].getVoltage();
				if (extTrigValue[t] >= 1.f && prevExtTrigValue[t] < 1.f) {
					clockAdv[t] = true;
				}
				prevExtTrigValue[t] = extTrigValue[t];

			} else {
				extConn[t] = false;
				clockAdv[t] = clockAdv[MC];
			}

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

			//if (currentMode[t] != CVOLTAGE && !resetPulse[MC]) {
			if (currentMode[t] != CVOLTAGE) {
			

				if (resetPulseTime[MC] <= 0) {

					if (stepAdv[t] && (seqRunSetting == 1 || xcludeFromRun[t])) {


						if (!userInputs[t][IN_RUN][0] || (userInputs[t][IN_RUN][0] && (!inputs[USER_INPUT+t+userInputs[t][IN_RUN][1]].isConnected() || (inputs[USER_INPUT+t+userInputs[t][IN_RUN][1]].isConnected() && inputs[USER_INPUT+t+userInputs[t][IN_RUN][1]].getVoltage() >= 1.f)))) {

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

						}

						// ------------------ CHANGE OPTION

						calcChange(t);
		

						// **************************************** 

						if (!turingMode[t]) {

							if (!calcSkip(t))  {
								if (wSeq[t][step[t]]) {
									stepPulse[t] = true;
									stepPulseTime[t] = oneMsTime;
									if (outType[t] == OUT_GATE)
										outGate[t] = true;
								} else {
									if (outType[t] == OUT_GATE) {
										outGate[t] = false;
										out[t] = 0.f;
									}
								}
							} else {
								if (outType[t] == OUT_GATE) {
									outGate[t] = false;
									out[t] = 0.f;
								}
							}

						} else {
							calcVoltage(t);
						}
	

					}
				}

			} else {	// CONTROL VOLTAGE ADVANCE

				if (resetPulseTime[MC] <= 0) {

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

							lights[STEP_LIGHT+(t*16+step[t])].setBrightness(0);
							step[t] = currAddr[t]-1;

							if (!turingMode[t]) {

								if (wSeq[t][step[t]]) {
									stepPulse[t] = true;
									stepPulseTime[t] = oneMsTime;
									if (outType[t] == OUT_GATE)
										outGate[t] = true;
								} else {
									if (outType[t] == OUT_GATE) {
										outGate[t] = false;
										out[t] = 0.f;
									}
								}
							} else {
								calcVoltage(t);
							}
							prevAddr[t] = currAddr[t];

						}
					}
					prevClkValue[t] = clkValue[t];

				}

			}


			if (!turingMode[t]) {

				if (stepPulse[t]) {
/*
					if (userInputs[t][KNOB_PW][0] && currentMode[t] != CVOLTAGE) { 
						if (divPulse[t] < 1) {
							stepPulse[t] = false;
							out[t] = 0.f;
						} else {
							out[t] = 10.f;
						}
					} else {*/
						//if ( (currentMode[t] != CVOLTAGE && outType[t] == OUT_TRIG) || (currentMode[t] == CVOLTAGE && (outType[t] == OUT_TRIG || outType[t] == OUT_CLOCK) ) ) {
						if (outType[t] == OUT_TRIG) {
							stepPulseTime[t]--;
							if (stepPulseTime[t] < 0) {
								stepPulse[t] = false;
								out[t] = 0.f;
							} else {
								out[t] = 10.f;
							}
						} else if (outType[t] == OUT_CLOCK) {
							if (!extConn[t])
								out[t] = inputs[EXTCLOCK_INPUT].getVoltage();
							else
								out[t] = inputs[CLK_INPUT+t].getVoltage();

							if (out[t] < 1.f) {
								out[t] = 0.f;
								stepPulse[t] = false;
							}

						} else {
							if (outGate[t]) 
								out[t] = 10.f;
							else
								out[t] = 0.f;
						}
					//}

				}

			} else {
				out[t] = outScale(volt[t], t);
			}

			outputs[OUT_OUTPUT+t].setVoltage(out[t]);

			lights[STEP_LIGHT+(t*16+step[t])].setBrightness(1);

		}
		
		if (resetPulse[MC]) {
			resetPulseTime[MC]--;
			if (resetPulseTime[MC] < 0) {
				resetPulse[MC] = false;
				outputs[RST_OUTPUT].setVoltage(0.f);
			} else
				outputs[RST_OUTPUT].setVoltage(10.f);
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
struct TrigStationDisplay : TransparentWidget {
	TrigStation *module;
	int frame = 0;
	TrigStationDisplay() {
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

struct TrigStationDisplayMode : TransparentWidget {
	TrigStation *module;
	int frame = 0;
	int t;

	TrigStationDisplayMode(int tIndex) : t(tIndex) {
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

			font = APP->window->loadFont(asset::plugin(pluginInstance, "res/c64.ttf"));
			nvgFontFaceId(args.vg, font->handle);
			nvgFontSize(args.vg, 10);

			if (!module->turingMode[t])
				nvgFillColor(args.vg, nvgRGB(COLOR_EGA_WHITE));				
			else
				nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_RED));				

			nvgTextBox(args.vg, 40.f, 15.f, 10, to_string(t+1).c_str(), NULL);
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		TrigStation *module = dynamic_cast<TrigStation *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ModeTypeItem : MenuItem {
				TrigStation* module;
				int menuValue;
				int t;

				ModeTypeItem(TrigStation* m, int value, int track) {
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

struct TrigStationDisplayTrackSett : TransparentWidget {
	TrigStation *module;
	int frame = 0;
	int t;

	TrigStationDisplayTrackSett(int tIndex) : t(tIndex) {
	}

	void onButton(const event::Button &e) override {

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
				createContextMenu();
				e.consume(this);
		}
		
	}

	void createContextMenu() {
		TrigStation *module = dynamic_cast<TrigStation *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			menu->addChild(createMenuLabel(("Track " + to_string(t+1)).c_str()));
/*
			menu->addChild(new MenuSeparator());

			struct ModeTypeItem : MenuItem {
				TrigStation* module;
				int menuValue;
				int t;

				ModeTypeItem(TrigStation* m, int value, int track) {
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
					TrigStation* module;
					int userInType;
					int t;
					int userColumn;

					UserInItem(TrigStation* m, int inType, int track, int column) {
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
					TrigStation* module;
					int userKnobType;
					int t;
					int userColumn;

					UserKnobItem(TrigStation* m, int knobType, int track, int column) {
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
						if (i+KNOB_SHIFT != module->userTable[t][3] || i > FIRST_ATN)
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
						if (i+KNOB_SHIFT != module->userTable[t][1] || i > FIRST_ATN)
							menu->addChild(userKnobTypeItem);
						else
							menu->addChild(createMenuLabel(module->userKnobNames[i]));
					}
				}));
			}));

			menu->addChild(new MenuSeparator());

			menu->addChild(createSubmenuItem("TRACK Settings", "", [=](Menu* menu) {

				struct OutTypeItem : MenuItem {
					TrigStation* module;
					int menuValue;
					int t;

					OutTypeItem(TrigStation* m, int value, int track) {
						module = m;
						menuValue = value;
						t = track;
						text = module->outTypeNames[value];
					}

					void onAction(const event::Action& e) override {
						module->outTypeSetting[t] = menuValue;
					}
				};

				menu->addChild(createSubmenuItem("Out Type", (module->outTypeNames[module->outTypeSetting[t]]), [=](Menu * menu) {
					for (int i = 0; i < 4; i++) {
						auto outTypeItem = new OutTypeItem(module, i, t);
						if (i == 3)
							outTypeItem->rightText = module->outTypeNames[module->outTypeSetting[MC]] + " " + CHECKMARK(module->outTypeSetting[t] == i);
						else
							outTypeItem->rightText = CHECKMARK(module->outTypeSetting[t] == i);
						menu->addChild(outTypeItem);
					}
				}));

				//----------------------------------------
/*
				struct RevTypeItem : MenuItem {
					TrigStation* module;
					int menuValue;
					int t;

					RevTypeItem(TrigStation* m, int value, int track) {
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
					TrigStation* module;
					int menuValue;
					int t;

					DontAdvanceItem(TrigStation* m, int value, int track) {
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
					TrigStation* module;
					int menuValue;
					int t;

					RstStepsWhenItem(TrigStation* m, int value, int track) {
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

			if (trigStation_clipboardTrack) {
				menu->addChild(createMenuItem("Paste Sequence", "", [=]() {module->pasteToTrack(t);}));
				menu->addChild(createMenuItem("Paste User", "", [=]() {module->pasteUser(t);}));
				menu->addChild(createMenuItem("Paste Sequence + User", "", [=]() {module->pasteUser(t);module->pasteToTrack(t);}));
			} else if (randLoops_clipboard) {
				menu->addChild(createMenuItem("Paste Sequence", "", [=]() {module->pasteToTrackRandLoops(t);}));
				menu->addChild(createMenuLabel("Paste User"));
				menu->addChild(createMenuLabel("Paste Sequence + User"));
			} else {
				menu->addChild(createMenuLabel("Paste Sequence"));
				menu->addChild(createMenuLabel("Paste User"));
				menu->addChild(createMenuLabel("Paste Sequence + User"));
			}

			menu->addChild(new MenuSeparator());

			struct SampleDelayItem : MenuItem {
				TrigStation* module;
				int menuValue;
				int t;

				SampleDelayItem(TrigStation* m, int value, int track) {
					module = m;
					menuValue = value;
					t = track;
					text = module->sampleDelayNames[value];
				}

				void onAction(const event::Action& e) override {
					module->sampleDelay[t] = menuValue;
				}
			};

			menu->addChild(createSubmenuItem("Track DELAY:", (module->sampleDelayNames[module->sampleDelay[t]]), [=](Menu * menu) {
				for (int i = 0; i < 7; i++) {
					auto sampleDelayItem = new SampleDelayItem(module, i, t);
					
					if (i == 6)
						sampleDelayItem->rightText = module->sampleDelayNames[module->sampleDelay[MC]] + " " + CHECKMARK(module->sampleDelay[t] == i);
					else
						sampleDelayItem->rightText = CHECKMARK(module->sampleDelay[t] == i);
					menu->addChild(sampleDelayItem);
				}
			}));

			menu->addChild(createBoolPtrMenuItem("TURING mode", "", &module->turingMode[t]));

			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuItem("Randomize Steps", "", [=]() {module->randomizeSequence(t);}));
			menu->addChild(createMenuItem("Initialize Steps", "", [=]() {module->initializeSequence(t);}));

		}
	}
};

struct TrigStationDisplayU1 : TransparentWidget {
	TrigStation *module;
	int frame = 0;
	int t;

	TrigStationDisplayU1(int tIndex) : t(tIndex) {
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
/*
				case IN_PW:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_MAGENTA));
					tempText = "PW";
					if (module->userInputs[t][KNOB_SWING][0] || module->userInputs[t][IN_SWING][0])
						tempText += "*";
				break;
*/
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

				case IN_SKIP:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_RED));
					tempText = "SK";
				break;

				case IN_SKIPPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_RED));
					tempText = "SP";
					if (module->userInputs[t][IN_SKIP][0])
						tempText += "*";
				break;
/*
				case IN_SWING:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_PURPLE));
					tempText = "SW";
					if (module->params[module->DIVMULT_KNOB_PARAM+t].getValue() < 23)
						tempText += "*";
				break;
*/				
			}
			nvgTextBox(args.vg, 0, 0, 15, tempText.c_str(), NULL);
		}
		Widget::drawLayer(args, layer);
	}
};

struct TrigStationDisplayU2 : TransparentWidget {
	TrigStation *module;
	int frame = 0;
	int t;

	TrigStationDisplayU2(int tIndex) : t(tIndex) {
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
/*
				case IN_PW:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_MAGENTA));
					tempText = "PW";
					if (module->userInputs[t][KNOB_SWING][0] || module->userInputs[t][IN_SWING][0])
						tempText += "*";
				break;
*/
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

				case IN_SKIP:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_RED));
					tempText = "SK";
				break;

				case IN_SKIPPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_RED));
					tempText = "SP";
					if (module->userInputs[t][IN_SKIP][0])
						tempText += "*";
				break;
/*
				case IN_SWING:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_PURPLE));
					tempText = "SW";
					if (module->params[module->DIVMULT_KNOB_PARAM+t].getValue() < 23)
						tempText += "*";
				break;
*/
			}
			nvgTextBox(args.vg, 0, 0, 15, tempText.c_str(), NULL);
		}
		Widget::drawLayer(args, layer);
	}
};

struct TrigStationDisplayK1 : TransparentWidget {
	TrigStation *module;
	int frame = 0;
	int t;

	TrigStationDisplayK1(int tIndex) : t(tIndex) {
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
/*
				case KNOB_PW:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_MAGENTA));
					tempText = "PW";
					if (module->userInputs[t][KNOB_SWING][0] || module->userInputs[t][IN_SWING][0])
						tempText += "*";
				break;
*/
				case KNOB_RSTSTEP:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_BROWN));
					tempText = "RS";
				break;

				case KNOB_RETRIGPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_GREY));
					tempText = "RP";
				break;

				case KNOB_SKIPPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_RED));
					tempText = "SP";
				break;
/*
				case KNOB_SWING:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_PURPLE));
					tempText = "SW";
					if (module->params[module->DIVMULT_KNOB_PARAM+t].getValue() < 23)
						tempText += "*";
				break;
*/
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

			}
			nvgTextBox(args.vg, 0, 0, 15, tempText.c_str(), NULL);
		}
		Widget::drawLayer(args, layer);
	}
};

struct TrigStationDisplayK2 : TransparentWidget {
	TrigStation *module;
	int frame = 0;
	int t;

	TrigStationDisplayK2(int tIndex) : t(tIndex) {
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
/*
				case KNOB_PW:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_MAGENTA));
					tempText = "PW";
					if (module->userInputs[t][KNOB_SWING][0] || module->userInputs[t][IN_SWING][0])
						tempText += "*";
				break;
*/
				case KNOB_RSTSTEP:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_BROWN));
					tempText = "RS";
				break;

				case KNOB_RETRIGPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_GREY));
					tempText = "RP";
				break;

				case KNOB_SKIPPROB:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_RED));
					tempText = "SP";
				break;
/*
				case KNOB_SWING:
					nvgFillColor(args.vg, nvgRGB(COLOR_USER_LIGHT_PURPLE));
					tempText = "SW";
					if (module->params[module->DIVMULT_KNOB_PARAM+t].getValue() < 23)
						tempText += "*";
				break;
*/
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
			}
			nvgTextBox(args.vg, 0, 0, 15, tempText.c_str(), NULL);
		}
		Widget::drawLayer(args, layer);
	}
};

struct TrigStationWidget : ModuleWidget {
	TrigStationWidget(TrigStation* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/TrigStation.svg")));
/*
		{
			TrigStationDisplayTempo *display = new TrigStationDisplayTempo();
			display->box.pos = mm2px(Vec(22.2 - 5 - 3, 14.8));
			display->box.size = mm2px(Vec(16.8, 6.5));
			display->module = module;
			addChild(display);
		}
*/
		{
			TrigStationDisplay *display = new TrigStationDisplay();
			display->box.pos = mm2px(Vec(126 - 14 + 5.7 + 1.7 - 38 + 4, 9.1));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}

		const float yLine = 33;
		const float xTemp = 0.3;

		for (int t = 0; t < MAXTRACKS; t++) {
/*
			{
				TrigStationDisplayDiv *display = new TrigStationDisplayDiv(t);
				display->box.pos = mm2px(Vec(22.5, 33+11.5*t));
				display->box.size = mm2px(Vec(15, 6.3));
				display->module = module;
				addChild(display);
			}
*/
			{
				TrigStationDisplayMode *display = new TrigStationDisplayMode(t);
				display->box.pos = mm2px(Vec(98.8 - 27, 33+11.5*t));
				display->box.size = mm2px(Vec(11, 6.3));
				display->module = module;
				addChild(display);
			}
			{
				TrigStationDisplayTrackSett *display = new TrigStationDisplayTrackSett(t);
				display->box.pos = mm2px(Vec(108.8 - 27, 33+11.5*t));
				display->box.size = mm2px(Vec(8, 6.3));
				display->module = module;
				addChild(display);
			}
			{
				TrigStationDisplayU1 *display = new TrigStationDisplayU1(t);
				display->box.pos = mm2px(Vec(61-27-xTemp, yLine+11.5*t));
				display->box.size = mm2px(Vec(4, 5));
				display->module = module;
				addChild(display);
			}
			{
				TrigStationDisplayK1 *display = new TrigStationDisplayK1(t);
				display->box.pos = mm2px(Vec(70-27-xTemp, 33+11.5*t));
				display->box.size = mm2px(Vec(4, 5));
				display->module = module;
				addChild(display);
			}
			{
				TrigStationDisplayU2 *display = new TrigStationDisplayU2(t);
				display->box.pos = mm2px(Vec(80-27-xTemp, yLine+11.5*t));
				display->box.size = mm2px(Vec(4, 5));
				display->module = module;
				addChild(display);
			}
			{
				TrigStationDisplayK2 *display = new TrigStationDisplayK2(t);
				display->box.pos = mm2px(Vec(89-27-xTemp, yLine+11.5*t));
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

		const float xSeqRunBut = 56.5f + 20 + 6 - 14 + 1.7 - 49.5 + 1 + 1;
		const float xSeqRunIn = xSeqRunBut + 10.f + 2 + 1;

		const float xRst = 100.f + 5.7 - 14 + 1.7 - 44.9 + 1 + 3;
		const float xRstB = xRst + 9.8f + 2;
		const float xRstOut = xRstB + 9.8f + 2;
		
		const float xProgIn = 149.f + 5 - 14 + 1.7 - 37.2 + 3 + 5; 

		const float xProgKnob = 163.5f + 5 -14 +1.7 -37.2 + 6 + 5; 
		const float yProgKnob = 16.3f;
		
		const float xSet = 181.f + 4 - 14 + 1.7 - 27.2 + 2 + 6;
		const float xAuto = 193.2f + 3 - 14 + 1.7 - 27.2 + 4 + 6;

		const float xRecallBut = 209.f + 2 - 14 + 1.7 - 15 + 4;
		const float xRecallIn = 219.f + 2 - 14 + 1.7 - 15 + 7;

		const float xStore = 231.4f + 1 - 14 + 1.7 - 15 + 12;

		const float yStart = 36.5f;
		const float yDelta = 11.5f;

		const float xStart = 120.5f - 27.f;
		const float xDelta = 7.45f;
		const float xBlockShift = 1.5f;

		const float yLight = -5.f;
		const float xLight = 2.2f;

		const float xOut = 247.f - 27 + 1.7;


//		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xBpmKnob, yBpmKnob)), module, TrigStation::BPM_KNOB_PARAM));

		//addInput(createStationClockInCentered<SickoClockInStation>(mm2px(Vec(xExtClock, yProgLine)), module, TrigStation::EXTCLOCK_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xExtClock, yProgLine)), module, TrigStation::EXTCLOCK_INPUT));

//		addParam(createLightParamCentered<VCVLightBezel<YellowLight>>(mm2px(Vec(xRun1, yRun1)), module, TrigStation::INTCLOCKBUT_PARAM, TrigStation::RUNBUT_LIGHT));
//		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRun2, yRun2)), module, TrigStation::RUN_INPUT));

		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xSeqRunBut, yProgLine)), module, TrigStation::SEQRUN_PARAM, TrigStation::SEQRUN_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xSeqRunIn, yProgLine)), module, TrigStation::SEQRUN_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRst, yProgLine)), module, TrigStation::RSTALL_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<YellowLight>>(mm2px(Vec(xRstB, yProgLine)), module, TrigStation::RSTALLBUT_PARAM, TrigStation::RSTALLBUT_LIGHT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xRstOut, yProgLine)), module, TrigStation::RST_OUTPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xProgIn, yProgLine)), module, TrigStation::PROG_INPUT));
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xProgKnob, yProgKnob)), module, TrigStation::PROG_PARAM));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xSet, yProgLine)), module, TrigStation::SET_PARAM, TrigStation::SET_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xAuto, yProgLine)), module, TrigStation::AUTO_PARAM, TrigStation::AUTO_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xRecallBut, yProgLine)), module, TrigStation::RECALL_PARAM, TrigStation::RECALL_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRecallIn, yProgLine)), module, TrigStation::RECALL_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xStore, yProgLine)), module, TrigStation::STORE_PARAM, TrigStation::STORE_LIGHT));
//		addOutput(createStationClockOutCentered<SickoClockOutStation>(mm2px(Vec(xClkOut, yProgLine)), module, TrigStation::CLOCK_OUTPUT));
		
		int shiftCont = 0;
		int shiftGroup = 0;
		float xShift = 0;
		
		const float xClock = 7.5f;
		//const float xDivMul = xClock + 9.5f;
		
		//const float xRstIn = xDivMul + 27.f;
		const float xRstIn = xClock + 9.3f;
		const float xLen = xRstIn + 10.2f;

		const float xU1 = xLen + 11.f;
		const float xK1 = xU1 + 9.f;
		const float xU2 = xK1 + 10.f;
		const float xK2 = xU2 + 9.f;

		for (int t = 0; t < MAXTRACKS; t++) {
			
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xClock, yStart+(t*yDelta))), module, TrigStation::CLK_INPUT+t));
			
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRstIn, yStart+(t*yDelta))), module, TrigStation::RST_INPUT+t));

			addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xLen, yStart+(t*yDelta))), module, TrigStation::LENGTH_PARAM+t));

			addInput(createStationInputCentered<SickoInputStation>(mm2px(Vec(xU1, yStart+(t*yDelta))), module, TrigStation::USER_INPUT+t));
			addParam(createStationParamCentered<SickoKnobStation>(mm2px(Vec(xK1, yStart+(t*yDelta))), module, TrigStation::USER_PARAM+t));
			addInput(createStationInputCentered<SickoInputStation>(mm2px(Vec(xU2, yStart+(t*yDelta))), module, TrigStation::USER_INPUT+t+8));
			addParam(createStationParamCentered<SickoKnobStation>(mm2px(Vec(xK2, yStart+(t*yDelta))), module, TrigStation::USER_PARAM+t+8));


			shiftCont = 0;
			shiftGroup = 0;
			xShift = 0;
			for (int st = 0; st < 16; st++) {

				addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(xStart + (xDelta*st) + xShift + xLight, yStart+(t*yDelta)+yLight)), module, TrigStation::STEP_LIGHT+(t*16)+st));

				switch (t) {
					case 0:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigStation::STEP_PARAM+(t*16)+st, TrigStation::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 1:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigStation::STEP_PARAM+(t*16)+st, TrigStation::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 2:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigStation::STEP_PARAM+(t*16)+st, TrigStation::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 3:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigStation::STEP_PARAM+(t*16)+st, TrigStation::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 4:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigStation::STEP_PARAM+(t*16)+st, TrigStation::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 5:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigStation::STEP_PARAM+(t*16)+st, TrigStation::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 6:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigStation::STEP_PARAM+(t*16)+st, TrigStation::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 7:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigStation::STEP_PARAM+(t*16)+st, TrigStation::STEPBUT_LIGHT+(t*16)+st));
					break;
				}

				shiftCont++;
				if (shiftCont > 3) {
					shiftGroup++;
					xShift = xBlockShift * shiftGroup;
					shiftCont = 0;
				}
				
				
			}

			addOutput(createStationClockOutCentered<SickoTrigOutStation>(mm2px(Vec(xOut, yStart+(t*yDelta))), module, TrigStation::OUT_OUTPUT+t));
			//addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yStart+(t*yDelta))), module, TrigStation::OUT_OUTPUT+t));

		}
		
	}

	void appendContextMenu(Menu* menu) override {
		TrigStation* module = dynamic_cast<TrigStation*>(this->module);

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

		menu->addChild(new MenuSeparator());

		struct SampleDelayItem : MenuItem {
			TrigStation* module;
			int sampleDelay;
			void onAction(const event::Action& e) override {
				module->sampleDelay[MC] = sampleDelay;
			}
		};

		menu->addChild(createSubmenuItem("Global DELAY:", (module->sampleDelayNames[module->sampleDelay[MC]]), [=](Menu * menu) {
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

			struct OutTypeItem : MenuItem {
				TrigStation* module;
				int outType;
				void onAction(const event::Action& e) override {
					module->outTypeSetting[MC] = outType;
				}
			};
			std::string OutTypeNames[3] = {"Trig", "Gate", "Clock Width"};
			menu->addChild(createSubmenuItem("Out Type", (OutTypeNames[module->outTypeSetting[MC]]), [=](Menu * menu) {
				for (int i = 0; i < 3; i++) {
					OutTypeItem* outTypeItem = createMenuItem<OutTypeItem>(OutTypeNames[i]);
					outTypeItem->rightText = CHECKMARK(module->outTypeSetting[MC] == i);
					outTypeItem->module = module;
					outTypeItem->outType = i;
					menu->addChild(outTypeItem);
				}
			}));

			/*
			struct RevTypeItem : MenuItem {
				TrigStation* module;
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
				TrigStation* module;
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
				TrigStation* module;
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
/*
			struct WaitClockItem : MenuItem {
				TrigStation* module;
				int wait2RstSetting;
				void onAction(const event::Action& e) override {
					module->wait2RstSetting = wait2RstSetting;
				}
			};

			std::string WaitClockNames[3] = {"No", "Yes"};
			menu->addChild(createSubmenuItem("Wait 1st clock to reset seq:", (WaitClockNames[module->wait2RstSetting]), [=](Menu * menu) {
				for (int i = 0; i < 2; i++) {
					WaitClockItem* waitClockItem = createMenuItem<WaitClockItem>(WaitClockNames[i]);
					waitClockItem->rightText = CHECKMARK(module->wait2RstSetting == i);
					waitClockItem->module = module;
					waitClockItem->wait2RstSetting = i;
					menu->addChild(waitClockItem);
				}
			}));
*/
			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuLabel("Module Settings"));

			struct RunTypeItem : MenuItem {
				TrigStation* module;
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

//			menu->addChild(createBoolPtrMenuItem("Reset internal clock on RST", "", &module->rstClkOnRst));
			menu->addChild(createBoolPtrMenuItem("Reset Seq on PROG change", "", &module->rstSeqOnProgChange));

			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuLabel("Turing settings"));

			struct BitResTypeItem : MenuItem {
				TrigStation* module;
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
				TrigStation* module;
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
/*
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
*/
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("User Interface"));
//			menu->addChild(createBoolPtrMenuItem("DIV/MULT mouse controls", "", &module->divControls));
			menu->addChild(createBoolPtrMenuItem("MODE mouse controls", "", &module->modeControls));

			
		}));

		menu->addChild(createSubmenuItem("TRACK settings", "", [=](Menu * menu) {


			struct UserInItem : MenuItem {
				TrigStation* module;
				int userInType;
				int t;
				int userColumn;

				UserInItem(TrigStation* m, int inType, int track, int column) {
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
				TrigStation* module;
				int userKnobType;
				int t;
				int userColumn;

				UserKnobItem(TrigStation* m, int knobType, int track, int column) {
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
							if (i+KNOB_SHIFT != module->userTable[t][3] || i > FIRST_ATN)
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
							if (i+KNOB_SHIFT != module->userTable[t][1] || i > FIRST_ATN)
								menu->addChild(userKnobTypeItem);
							else
								menu->addChild(createMenuLabel(module->userKnobNames[i]));
						}
					}));

					menu->addChild(new MenuSeparator());

					// **********************************************************************

					struct OutTypeItem : MenuItem {
						TrigStation* module;
						int menuValue;
						int t;

						OutTypeItem(TrigStation* m, int value, int track) {
							module = m;
							menuValue = value;
							t = track;
							text = module->outTypeNames[value];
						}

						void onAction(const event::Action& e) override {
							module->outTypeSetting[t] = menuValue;
						}
					};

					menu->addChild(createSubmenuItem("Out Type", (module->outTypeNames[module->outTypeSetting[t]]), [=](Menu * menu) {
						for (int i = 0; i < 4; i++) {
							auto outTypeItem = new OutTypeItem(module, i, t);
							if (i == 3)
								outTypeItem->rightText = module->outTypeNames[module->outTypeSetting[MC]] + " " + CHECKMARK(module->outTypeSetting[t] == i);
							else
								outTypeItem->rightText = CHECKMARK(module->outTypeSetting[t] == i);
							menu->addChild(outTypeItem);
						}
					}));

					//----------------------------------------
/*
					struct RevTypeItem : MenuItem {
						TrigStation* module;
						int menuValue;
						int t;

						RevTypeItem(TrigStation* m, int value, int track) {
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
						TrigStation* module;
						int menuValue;
						int t;

						DontAdvanceItem(TrigStation* m, int value, int track) {
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
						TrigStation* module;
						int menuValue;
						int t;

						RstStepsWhenItem(TrigStation* m, int value, int track) {
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

					if (trigStation_clipboardTrack) {
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
						TrigStation* module;
						int menuValue;
						int t;

						SampleDelayItem(TrigStation* m, int value, int track) {
							module = m;
							menuValue = value;
							t = track;
							text = module->sampleDelayNames[value];
						}

						void onAction(const event::Action& e) override {
							module->sampleDelay[t] = menuValue;
						}
					};

					menu->addChild(createSubmenuItem("Track DELAY:", (module->sampleDelayNames[module->sampleDelay[t]]), [=](Menu * menu) {
						for (int i = 0; i < 7; i++) {
							auto sampleDelayItem = new SampleDelayItem(module, i, t);
							if (i == 6)
								sampleDelayItem->rightText = module->sampleDelayNames[module->sampleDelay[MC]] + " " + CHECKMARK(module->sampleDelay[t] == i);
							else
								sampleDelayItem->rightText = CHECKMARK(module->sampleDelay[t] == i);
							menu->addChild(sampleDelayItem);
						}
					}));

					menu->addChild(createBoolPtrMenuItem("TURING mode", "", &module->turingMode[t]));

					menu->addChild(new MenuSeparator());

					menu->addChild(createMenuItem("Randomize Steps", "", [=]() {module->randomizeSequence(t);}));
					menu->addChild(createMenuItem("Initialize Steps", "", [=]() {module->initializeSequence(t);}));

				}));
			}
		}));

		menu->addChild(new MenuSeparator());
		
		struct ProgInTypeItem : MenuItem {
			TrigStation* module;
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
			if (trigStation_clipboard) {
				menu->addChild(createMenuItem("Paste Panel", "", [=]() {module->pastePanel();}));
			} else {
				menu->addChild(createMenuLabel("Paste Panel"));
			}

			menu->addChild(new MenuSeparator());
			
			menu->addChild(createMenuItem("Copy User Settings", "", [=]() {module->copyUserSettings();}));
			if (trigStation_userClipboard) {
				menu->addChild(createMenuItem("Paste User Settings", "", [=]() {module->pasteUserSettings();}));
			} else {
				menu->addChild(createMenuLabel("Paste User Settings"));
			}
			
			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuItem("Copy All Sequences", "", [=]() {module->copyAllTracks();}));
			if (stepSeq8_clipboard || trigStation_clipboard) {
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
			if (stepSeq_clipboard || trigStation_clipboard) {
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
			
			menu->addChild(createMenuItem("Load preset", "", [=]() {module->menuLoadTrigStationPreset();}));
			menu->addChild(createMenuItem("Save preset", "", [=]() {module->menuSaveTrigStationPreset();}));

/*
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Import stepSeq+ preset", "", [=]() {module->menuLoadSingle();}));
			menu->addChild(createMenuItem("Export stepSeq+ preset", "", [=]() {module->menuSaveSingle();}));
*/

			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Import All Sequences", "", [=]() {module->menuLoadAllSequences();}));
			menu->addChild(createMenuItem("Export All Sequences", "", [=]() {module->menuSaveAllSequences();}));
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
			menu->addChild(createMenuLabel("Adjust Step Advance timing with 'Track DELAY' options"));
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("Store Programs with double-click on STOR"));
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("Remember to store programs when"));
			menu->addChild(createMenuLabel("changing / importing / pasting sequences"));
		}));

	}

};

Model* modelTrigStation = createModel<TrigStation, TrigStationWidget>("TrigStation");