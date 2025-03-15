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

	// Add modules here
	// p->addModel(modelMyModule);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
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
	p->addModel(modelCVrouter);
	p->addModel(modelCVswitcher);
	p->addModel(modelDrummer);
	p->addModel(modelDrummer4);
	p->addModel(modelDrummer4Plus);
	p->addModel(modelDrumPlayer);
	p->addModel(modelDrumPlayerPlus);
	p->addModel(modelDrumPlayerXtra);
	p->addModel(modelEnver);
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
	p->addModel(modelRandLoops8);
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
	p->addModel(modelStepSeq);
	p->addModel(modelStepSeqPlus);
	p->addModel(modelSwitcher);
	p->addModel(modelSwitcherSt);
	p->addModel(modelSwitcher8);
	p->addModel(modelToggler);
	p->addModel(modelTogglerCompact);
	p->addModel(modelTrigSeq);
	p->addModel(modelTrigSeqPlus);
	p->addModel(modelWavetabler);

	
}

bool randLoops_clipboard = false;
int randLoops_cbSeq[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int randLoops_cbSteps = 16;
float randLoops_cbCtrl = 0.f;
float randLoops_cbScale = 1.f;
float randLoops_cbOffset = 0.f;

bool stepSeq_clipboard = false;
float stepSeq_cbSeq[16] = {0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f,0.5f};
int stepSeq_cbSteps = 16;
int stepSeq_cbRst = 1;