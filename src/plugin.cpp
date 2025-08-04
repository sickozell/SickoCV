#include "plugin.hpp"

//Plugin* pluginInstance;
//extern Plugin *pluginInstance;

#if defined(METAMODULE_BUILTIN)
extern Plugin *pluginInstance;
#else
Plugin *pluginInstance;
#endif

//void init(Plugin* p) {
//void init_SickoCV(rack::Plugin *p) {

#if defined(METAMODULE_BUILTIN)
void init_SickoCV(rack::Plugin *p) {
#else 
void init(rack::Plugin *p) {
#endif
	pluginInstance = p;

	p->addModel(modelAdMini);
	p->addModel(modelAdder8);
	p->addModel(modelBgates);
	p->addModel(modelBlender);
	p->addModel(modelBlender8);
	p->addModel(modelBtogglerSt);
	p->addModel(modelBtogglerStCompact);
	p->addModel(modelBtoggler);
	p->addModel(modelBtogglerPlus);
	p->addModel(modelCalcs);
	p->addModel(modelClocker);
	p->addModel(modelClocker2);
	p->addModel(modelCVmeter);
	p->addModel(modelCVrouter);
	p->addModel(modelCVswitcher);
	p->addModel(modelDrummer);
	p->addModel(modelDrummer4);
	p->addModel(modelDrummer4Plus);
	p->addModel(modelDrumPlayer);
	p->addModel(modelDrumPlayerMk2);
	p->addModel(modelDrumPlayerPlus);
	p->addModel(modelDrumPlayerXtra);
	p->addModel(modelDrumPlayerMini);
	p->addModel(modelEnver);
	p->addModel(modelEnverMini);
	p->addModel(modelEnverMiniX);
	p->addModel(modelHolder);
	p->addModel(modelHolderCompact);
	p->addModel(modelHolder8);
	p->addModel(modelKeySampler);
	p->addModel(modelModulator);
	p->addModel(modelModulator7);
	p->addModel(modelModulator7Compact);
	p->addModel(modelMultiSwitcher);
	p->addModel(modelMultiRouter);
	p->addModel(modelParking);
	p->addModel(modelPolyMuter8);
	p->addModel(modelPolyMuter8Plus);
	p->addModel(modelPolyMuter16);
	p->addModel(modelPolyMuter16Plus);
	p->addModel(modelRandLoops);
	p->addModel(modelRandLoopsMini);
	p->addModel(modelRandLoops8);
	p->addModel(modelRandMod7);
	p->addModel(modelRandMod7Compact);
	p->addModel(modelSampleDelay);
	p->addModel(modelShifter);
	p->addModel(modelSickoAmp);
	p->addModel(modelSickoCrosser);
	p->addModel(modelSickoCrosser4);
	p->addModel(modelSickoLooper1);
	p->addModel(modelSickoLooper1Exp);
	p->addModel(modelSickoLooper3);
	p->addModel(modelSickoLooper5);
	p->addModel(modelSickoPlayer);
	p->addModel(modelSickoQuant);
	p->addModel(modelSickoQuant4);
	p->addModel(modelSickoSampler);
	p->addModel(modelSickoSampler2);
	p->addModel(modelSimpleSeq4);
	p->addModel(modelSlewer);
	p->addModel(modelSlewerMini);
	p->addModel(modelStepSeq);
	p->addModel(modelStepSeqPlus);
	p->addModel(modelStepSeq8x);
	p->addModel(modelStepStation);
	p->addModel(modelSwitcher);
	p->addModel(modelSwitcherSt);
	p->addModel(modelSwitcher8);
	p->addModel(modelToggler);
	p->addModel(modelTogglerCompact);
	p->addModel(modelTrigSeq);
	p->addModel(modelTrigSeqPlus);
	p->addModel(modelTrigSeq8x);
	p->addModel(modelTrigStation);
	p->addModel(modelWavetabler);

}

// -----------------------------------

bool randLoops_clipboard = false;
int randLoops_cbSeq[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int randLoops_cbSteps = 16;
float randLoops_cbCtrl = 0.f;
float randLoops_cbScale = 1.f;
float randLoops_cbOffset = 0.f;

bool randLoops8_clipboard = false;
int randLoops8_cbSeq[8][16] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
								{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
int randLoops8_cbSteps[8] = {16,16,16,16,16,16,16,16};
float randLoops8_cbCtrl[8] = {0,0,0,0,0,0,0,0};
float randLoops8_cbScale[8] = {0,0,0,0,0,0,0,0};
float randLoops8_cbOffset[8] = {0,0,0,0,0,0,0,0};

bool stepSeq_clipboard = false;
float stepSeq_cbSeq[16] = {0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f};
int stepSeq_cbSteps = 16;
int stepSeq_cbRst = 1;

bool stepSeq8_clipboard = false;
float stepSeq8_cbSeq[8][16] = {{0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5},
								{0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5},
								{0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5},
								{0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5},
								{0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5},
								{0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5},
								{0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5},
								{0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5}};
int stepSeq8_cbSteps = 16;
int stepSeq8_cbRst = 1;


bool stepStation_clipboard = false;
bool stepStation_userClipboard = false;
int stepStation_cbSteps[8] = {16, 16, 16, 16, 16, 16, 16, 16};
int stepStation_cbCurrentMode[8] = {0, 0, 0, 0, 0, 0, 0, 0};
//float stepStation_cbDivMult[8] = {22, 22, 22, 22, 22, 22, 22, 22};
int stepStation_cbRange[9] = {10, 10, 10, 10, 10, 10, 10, 10, 9};
//int stepStation_cbRevType[9] = {2, 2, 2, 2, 2, 2, 2, 2, 0};	// 0 means POSITIVE_V
int stepStation_cbDontAdvanceSetting[9] = {2, 2, 2, 2, 2, 2, 2, 2, 1};
int stepStation_cbRstStepsWhen[9] = {3, 3, 3, 3, 3, 3, 3, 3, 1};
bool stepStation_cbXcludeFromRun[8] = {false, false, false, false, false, false, false, false};
bool stepStation_cbXcludeFromRst[8] = {false, false, false, false, false, false, false, false};
bool stepStation_cbSeqRunSetting =  1; //means seqRunSetting;
//bool stepStation_cbInternalClock = 0; //means internalClock;
//float stepStation_cbBpmKnob = 1200.f;
float stepStation_cbUserValues[8][2] = {};
float stepStation_cbUserInputs[8][16][2] = {};	// 16 is MAXUSER
float stepStation_cbUserTable[8][4] = {};

bool stepStation_clipboardTrack = false;
int stepStation_cbStepsTrack = 16;
int stepStation_cbCurrentModeTrack = 0;
//float stepStation_cbDivMultTrack = 22;
//int stepStation_cbRevTypeTrack = 2;	// 0 means POSITIVE_V
int stepStation_cbDontAdvanceSettingTrack = 2;
int stepStation_cbRstStepsWhenTrack = 3;
bool stepStation_cbXcludeFromRunTrack = false;
bool stepStation_cbXcludeFromRstTrack = false;
float stepStation_cbUserValuesTrack[2] = {};
float stepStation_cbUserInputsTrack[16][2] = {};	// 16 is MAXUSER
float stepStation_cbUserTableTrack[4] = {};
int stepStation_cbRangeTrack = 10;


bool trigStation_clipboard = false;
bool trigStation_userClipboard = false;
int trigStation_cbSteps[8] = {16, 16, 16, 16, 16, 16, 16, 16};
int trigStation_cbCurrentMode[8] = {0, 0, 0, 0, 0, 0, 0, 0};
//float trigStation_cbDivMult[8] = {22, 22, 22, 22, 22, 22, 22, 22};
int trigStation_cbTuringMode[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int trigStation_cbOutTypeSetting[9] = {3, 3, 3, 3, 3, 3, 3, 3, 0};
//int trigStation_cbRevType[9] = {2, 2, 2, 2, 2, 2, 2, 2, 0};	// 0 means POSITIVE_V
int trigStation_cbDontAdvanceSetting[9] = {2, 2, 2, 2, 2, 2, 2, 2, 1};
int trigStation_cbRstStepsWhen[9] = {3, 3, 3, 3, 3, 3, 3, 3, 1};
bool trigStation_cbXcludeFromRun[8] = {false, false, false, false, false, false, false, false};
bool trigStation_cbXcludeFromRst[8] = {false, false, false, false, false, false, false, false};
bool trigStation_cbSeqRunSetting =  1; //means seqRunSetting;
//bool trigStation_cbInternalClock = 0; //means internalClock;
//float trigStation_cbBpmKnob = 1200.f;
float trigStation_cbUserValues[8][2] = {};
float trigStation_cbUserInputs[8][19][2] = {};	// 19 is MAXUSER
float trigStation_cbUserTable[8][4] = {};

bool trigStation_clipboardTrack = false;
int trigStation_cbStepsTrack = 16;
int trigStation_cbCurrentModeTrack = 0;
//float trigStation_cbDivMultTrack = 22;
int trigStation_cbTuringModeTrack = 0;
int trigStation_cbOutTypeSettingTrack = 0;
//int trigStation_cbRevTypeTrack = 2;	// 0 means POSITIVE_V
int trigStation_cbDontAdvanceSettingTrack = 2;
int trigStation_cbRstStepsWhenTrack = 3;
bool trigStation_cbXcludeFromRunTrack = false;
bool trigStation_cbXcludeFromRstTrack = false;
float trigStation_cbUserValuesTrack[2] = {};
float trigStation_cbUserInputsTrack[19][2] = {};	// 19 is MAXUSER
float trigStation_cbUserTableTrack[4] = {};