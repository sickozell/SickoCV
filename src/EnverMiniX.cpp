#include "plugin.hpp"
#include "EnverMiniExp.hpp"

//using namespace std;

struct EnverMiniX : Module {

	#include "shapes.hpp"

	enum ParamIds {
		NUM_PARAMS 
	};
	enum InputIds {
		SIGNAL_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIGNAL_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	int chanVca = 1;
	int chanTrig = 1;

	float env[16] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	float outSignal = 0.f;

	float volLevel;

	//**************************************************************
	// EXPANDER variables

	bool connectedToMaster = false;

	EnvMiniExpMsg expInputMessage[2][1];
	EnvMiniExpMsg expOutputMessage[2][1];

	EnverMiniX() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configInput(SIGNAL_INPUT,"Signal");
		configOutput(SIGNAL_OUTPUT,"Signal");

		leftExpander.producerMessage = &expInputMessage[0];
		leftExpander.consumerMessage = &expInputMessage[1];
		rightExpander.producerMessage = &expOutputMessage[0];
		rightExpander.consumerMessage = &expOutputMessage[1];
	}

	void addExpander( Model* model, ModuleWidget* parentModWidget, bool left = false ) {
		Module* module = model->createModule();
		APP->engine->addModule(module);
		ModuleWidget* modWidget = model->createModuleWidget(module);
		APP->scene->rack->setModulePosForce( modWidget, Vec( parentModWidget->box.pos.x + (left ? -modWidget->box.size.x : parentModWidget->box.size.x), parentModWidget->box.pos.y));
		APP->scene->rack->addModule(modWidget);
		history::ModuleAdd* h = new history::ModuleAdd;
		h->name = "create "+model->name;
		h->setModule(modWidget);
		APP->history->push(h);
	}

	void process(const ProcessArgs &args) override {

		// RECEIVING DATA FROM LEFT
		if (leftExpander.module && (leftExpander.module->model == modelEnverMini ||
									leftExpander.module->model == modelEnverMiniX ||
									leftExpander.module->model == modelAdMini)) {
			
			EnvMiniExpMsg *msgFromModule = (EnvMiniExpMsg *)(leftExpander.consumerMessage);

			if (msgFromModule->connectedToMaster == true)
				connectedToMaster = true;
			else
				connectedToMaster = false;

			chanTrig = msgFromModule->chanTrig;
			volLevel = msgFromModule->volLevel;
			for (int c = 0; c < chanTrig; c++)
				env[c] = msgFromModule->env[c];

		} else {
			connectedToMaster = false;
			chanTrig = 1;
			volLevel = 0.f;
			env[0] = 0.f;
		}

		if (connectedToMaster) {

			if (outputs[SIGNAL_OUTPUT].isConnected()) {

				if (chanTrig == 1) {

					if (outputs[SIGNAL_OUTPUT].isConnected()) {
						chanVca = inputs[SIGNAL_INPUT].getChannels();

						for (int c = 0; c < chanVca; c++) {
							outSignal = inputs[SIGNAL_INPUT].getVoltage(c) * env[0] * volLevel;
							if (outSignal > 10.f)
								outSignal = 10;
							else if (outSignal < -10.f)
								outSignal = -10.f;
							outputs[SIGNAL_OUTPUT].setVoltage(outSignal, c);
						}
						outputs[SIGNAL_OUTPUT].setChannels(chanVca);
					}

				} else {	// trigger polyPhony

					for (int c = 0; c < chanTrig; c++) {

						outSignal = inputs[SIGNAL_INPUT].getVoltage(c) * env[c] * volLevel;
						if (outSignal > 10.f)
							outSignal = 10;
						else if (outSignal < -10.f)
							outSignal = -10.f;
						outputs[SIGNAL_OUTPUT].setVoltage(outSignal, c);

					}
					outputs[SIGNAL_OUTPUT].setChannels(chanTrig);

				}
				
			}
			
		} else {
			outputs[SIGNAL_OUTPUT].setVoltage(0.f);
			outputs[SIGNAL_OUTPUT].setChannels(1);
		}

		// ------------------------------------------------------------------  E X P A N D E R

		if (rightExpander.module && rightExpander.module->model == modelEnverMiniX) {
			EnvMiniExpMsg *msgToModule = (EnvMiniExpMsg *)(rightExpander.module->leftExpander.producerMessage);

			msgToModule->connectedToMaster = connectedToMaster;

			msgToModule->chanTrig = chanTrig;
			msgToModule->volLevel = volLevel;
			for (int c = 0; c < chanTrig; c++)
				msgToModule->env[c] = env[c];

			rightExpander.module->leftExpander.messageFlipRequested = true;
		}

	}
	
};


struct EnverMiniXWidget : ModuleWidget {
	EnverMiniXWidget(EnverMiniX *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/EnverMiniX.svg")));

		const float xCenter = 5.08f;

		const float yIn1 = 90.5;
		const float yOut1 = 104.5;

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yIn1)), module, EnverMiniX::SIGNAL_INPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xCenter, yOut1)), module, EnverMiniX::SIGNAL_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		EnverMiniX* module = dynamic_cast<EnverMiniX*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Add Expander", "", [=]() {module->addExpander(modelEnverMiniX, this);}));
	}
};

Model *modelEnverMiniX = createModel<EnverMiniX, EnverMiniXWidget>("EnverMiniX");
