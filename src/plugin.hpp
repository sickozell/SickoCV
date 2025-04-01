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
extern Model* modelSwitcher;
extern Model* modelSwitcherSt;
extern Model* modelSwitcher8;
extern Model* modelToggler;
extern Model* modelTogglerCompact;
extern Model* modelTrigSeq;
extern Model* modelTrigSeqPlus;
extern Model* modelTrigSeq8x;
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
