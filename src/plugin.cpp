#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
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
	p->addModel(modelCVrouter);
	p->addModel(modelCVswitcher);
	p->addModel(modelDrummer);
	p->addModel(modelDrummer4);
	p->addModel(modelDrummer4Plus);
	p->addModel(modelDrumPlayer);
	p->addModel(modelDrumPlayerPlus);
	p->addModel(modelDrumPlayerXtra);
	p->addModel(modelParking);
	p->addModel(modelShifter);
	p->addModel(modelSickoAmp);
	p->addModel(modelSickoLooper5);
	p->addModel(modelSickoPlayer);
	p->addModel(modelSickoSampler);
	p->addModel(modelSickoSampler2);
	p->addModel(modelSwitcher);
	p->addModel(modelSwitcherSt);
	p->addModel(modelToggler);
	p->addModel(modelTogglerCompact);
	p->addModel(modelWavetabler);
}
