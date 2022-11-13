#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
	p->addModel(modelBtoggler);
	p->addModel(modelBtogglerPlus);
	p->addModel(modelCalcs);
	p->addModel(modelToggler);
	p->addModel(modelTogglerCompact);	
	p->addModel(modelBtogglerSt);
	p->addModel(modelBtogglerStCompact);
	p->addModel(modelBlender);
	p->addModel(modelBlender8);
	p->addModel(modelSwitcher);
	p->addModel(modelSwitcherSt);
	p->addModel(modelDrummer);
	p->addModel(modelDrummer4);
	p->addModel(modelParking);
}
