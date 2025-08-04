#pragma once
#include <rack.hpp>

using namespace rack;

extern Plugin* pluginInstance;

#include "Controls.hpp"

extern Model* modelAdMini;
extern Model* modelAdder8;
extern Model* modelBgates;
extern Model* modelBlender;
extern Model* modelBlender8;
extern Model* modelBtogglerSt;
extern Model* modelBtogglerStCompact;
extern Model* modelBtoggler;
extern Model* modelBtogglerPlus;
extern Model* modelCalcs;
extern Model* modelClocker;
extern Model* modelClocker2;
extern Model* modelCVmeter;
extern Model* modelCVrouter;
extern Model* modelCVswitcher;
extern Model* modelDrummer;
extern Model* modelDrummer4;
extern Model* modelDrummer4Plus;
extern Model* modelDrumPlayer;
extern Model* modelDrumPlayerMk2;
extern Model* modelDrumPlayerPlus;
extern Model* modelDrumPlayerXtra;
extern Model* modelDrumPlayerMini;
extern Model* modelEnver;
extern Model* modelEnverMini;
extern Model* modelEnverMiniX;
extern Model* modelHolder;
extern Model* modelHolderCompact;
extern Model* modelHolder8;
extern Model* modelKeySampler;
extern Model* modelModulator;
extern Model* modelModulator7;
extern Model* modelModulator7Compact;
extern Model* modelMultiSwitcher;
extern Model* modelMultiRouter;
extern Model* modelParking;
extern Model* modelPolyMuter8;
extern Model* modelPolyMuter8Plus;
extern Model* modelPolyMuter16;
extern Model* modelPolyMuter16Plus;
extern Model* modelRandLoops;
extern Model* modelRandLoopsMini;
extern Model* modelRandLoops8;
extern Model* modelRandMod7;
extern Model* modelRandMod7Compact;
extern Model* modelSampleDelay;
extern Model* modelShifter;
extern Model* modelSickoAmp;
extern Model* modelSickoCrosser;
extern Model* modelSickoCrosser4;
extern Model* modelSickoLooper1;
extern Model* modelSickoLooper1Exp;
extern Model* modelSickoLooper3;
extern Model* modelSickoLooper5;
extern Model* modelSickoPlayer;
extern Model* modelSickoQuant;
extern Model* modelSickoQuant4;
extern Model* modelSickoSampler;
extern Model* modelSickoSampler2;
extern Model* modelSimpleSeq4;
extern Model* modelSlewer;
extern Model* modelSlewerMini;
extern Model* modelStepSeq;
extern Model* modelStepSeqPlus;
extern Model* modelStepSeq8x;
extern Model* modelStepStation;
extern Model* modelSwitcher;
extern Model* modelSwitcherSt;
extern Model* modelSwitcher8;
extern Model* modelToggler;
extern Model* modelTogglerCompact;
extern Model* modelTrigSeq;
extern Model* modelTrigSeqPlus;
extern Model* modelTrigSeq8x;
extern Model* modelTrigStation;
extern Model* modelWavetabler;

// ----------------------------------

extern bool randLoops_clipboard;
extern int randLoops_cbSeq[16];
extern int randLoops_cbSteps;
extern float randLoops_cbCtrl;
extern float randLoops_cbScale;
extern float randLoops_cbOffset;

extern bool randLoops8_clipboard;
extern int randLoops8_cbSeq[8][16];
extern int randLoops8_cbSteps[8];
extern float randLoops8_cbCtrl[8];
extern float randLoops8_cbScale[8];
extern float randLoops8_cbOffset[8];

extern bool stepSeq_clipboard;
extern float stepSeq_cbSeq[16];
extern int stepSeq_cbSteps;
extern int stepSeq_cbRst;

extern bool stepSeq8_clipboard;
extern float stepSeq8_cbSeq[8][16];
extern int stepSeq8_cbSteps;
extern int stepSeq8_cbRst;


extern bool stepStation_clipboard;
extern bool stepStation_userClipboard;
extern int stepStation_cbSteps[8];
extern int stepStation_cbCurrentMode[8];
//extern float stepStation_cbDivMult[8];
extern int stepStation_cbRange[9];
//extern int stepStation_cbRevType[9];
extern int stepStation_cbDontAdvanceSetting[9];
extern int stepStation_cbRstStepsWhen[9];
extern bool stepStation_cbXcludeFromRun[8];
extern bool stepStation_cbXcludeFromRst[8];
extern bool stepStation_cbSeqRunSetting;
//extern bool stepStation_cbInternalClock;
//extern float stepStation_cbBpmKnob;
extern float stepStation_cbUserValues[8][2];
extern float stepStation_cbUserInputs[8][16][2];	// 16 is MAXUSER
extern float stepStation_cbUserTable[8][4];

extern bool stepStation_clipboardTrack;
extern int stepStation_cbStepsTrack;
extern int stepStation_cbCurrentModeTrack;
//extern float stepStation_cbDivMultTrack;
extern int stepStation_cbRangeTrack;
//extern int stepStation_cbRevTypeTrack;	// 0 means POSITIVE_V
extern int stepStation_cbDontAdvanceSettingTrack;
extern int stepStation_cbRstStepsWhenTrack;
extern bool stepStation_cbXcludeFromRunTrack;
extern bool stepStation_cbXcludeFromRstTrack;
extern float stepStation_cbUserValuesTrack[2];
extern float stepStation_cbUserInputsTrack[16][2];	// 16 is MAXUSER
extern float stepStation_cbUserTableTrack[4];


extern bool trigStation_clipboard;
extern bool trigStation_userClipboard;
extern int trigStation_cbSteps[8];
extern int trigStation_cbCurrentMode[8];
//extern float trigStation_cbDivMult[8];
extern int trigStation_cbTuringMode[8];
extern int trigStation_cbOutTypeSetting[9];
extern int trigStation_cbRevType[9];
extern int trigStation_cbDontAdvanceSetting[9];
extern int trigStation_cbRstStepsWhen[9];
extern bool trigStation_cbXcludeFromRun[8];
extern bool trigStation_cbXcludeFromRst[8];
extern bool trigStation_cbSeqRunSetting;
//extern bool trigStation_cbInternalClock;
//extern float trigStation_cbBpmKnob;
extern float trigStation_cbUserValues[8][2];
extern float trigStation_cbUserInputs[8][19][2];	// 19 is MAXUSER
extern float trigStation_cbUserTable[8][4];

extern bool trigStation_clipboardTrack;
extern int trigStation_cbStepsTrack;
extern int trigStation_cbCurrentModeTrack;
//extern float trigStation_cbDivMultTrack;
extern int trigStation_cbTuringModeTrack;
extern int trigStation_cbOutTypeSettingTrack;
//extern int trigStation_cbRevTypeTrack;	// 0 means POSITIVE_V
extern int trigStation_cbDontAdvanceSettingTrack;
extern int trigStation_cbRstStepsWhenTrack;
extern bool trigStation_cbXcludeFromRunTrack;
extern bool trigStation_cbXcludeFromRstTrack;
extern float trigStation_cbUserValuesTrack[2];
extern float trigStation_cbUserInputsTrack[19][2];	// 19 is MAXUSER
extern float trigStation_cbUserTableTrack[4];