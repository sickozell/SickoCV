#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
	p->addModel(modelBlender);
	p->addModel(modelBlender8);
	p->addModel(modelBtoggler);
	p->addModel(modelBtogglerCompact);
	p->addModel(modelBtoggler8);
	p->addModel(modelBtoggler8Plus);
	p->addModel(modelCalcs);
	p->addModel(modelToggler);
	p->addModel(modelTogglerCompact);
}
