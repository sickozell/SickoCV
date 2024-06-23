#define UNMUTED 0
#define SOLOED 1
#define MUTED_SOLOED 2
#define MUTED 3

#define COLOR_LCD_RED 0xdd, 0x33, 0x33, 0xff

#include "plugin.hpp"


using namespace std;

// ----------------------------------------------------------------------------

template <class BASE>
struct LightEmittingWidget : BASE {
    virtual bool isLit() = 0;

    void drawLayer(const typename BASE::DrawArgs& args, int layer) override {
        if (layer == 1 && isLit()) {
            drawLit(args);
        }
        BASE::drawLayer(args, layer);
    }

    virtual void drawLit(const typename BASE::DrawArgs& args) {}
};

// ----------------------------------------------------------------------------

struct PM8SoloMuteButton : LightEmittingWidget<ParamWidget> {
    std::vector<std::shared_ptr<Svg>> _frames;
    SvgWidget* _svgWidget;
    CircularShadow* shadow = NULL;

    PM8SoloMuteButton();
    void onButton(const event::Button& e) override;
    void onChange(const event::Change& e) override;
    bool isLit() override;
    void draw(const DrawArgs& args) override;
    void drawLit(const DrawArgs& args) override;
};


// ----------------------------------------------------------------------------


PM8SoloMuteButton::PM8SoloMuteButton() {
    shadow = new CircularShadow();
    addChild(shadow);

    _svgWidget = new SvgWidget();
    addChild(_svgWidget);

    auto svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoMuteButton_0.svg"));
    _frames.push_back(svg);
    _frames.push_back(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoMuteButton_1_green.svg")));
    _frames.push_back(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoMuteButton_3_red_green.svg")));
    //_frames.push_back(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoMuteButton_2_red.svg")));
    //_frames.push_back(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoMuteButton_1_green.svg")));
    _frames.push_back(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoMuteButton_2_red.svg")));
    _svgWidget->setSvg(svg);
    box.size = _svgWidget->box.size;
    shadow->box.size = _svgWidget->box.size;
    shadow->blurRadius = 1.0;
    shadow->box.pos = Vec(0.0, 1.0);
}

void PM8SoloMuteButton::onButton(const event::Button& e) {
    if (!getParamQuantity() || !(e.action == GLFW_PRESS && (e.mods & RACK_MOD_MASK) == 0)) {
        ParamWidget::onButton(e);
        return;
    }


    float value = getParamQuantity()->getValue();

    // 0 unmuted
    // 1 soloed
    // 2 soloed muted
    // 3 muted

    
    if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (value == 0.f)
            getParamQuantity()->setValue(1.f);
        else if (value == 1.f)
            getParamQuantity()->setValue(0.f);
        else if (value == 2.f)
            getParamQuantity()->setValue(3.f);
        else
            getParamQuantity()->setValue(2.f);
    }
    else if (value == 0) {
        getParamQuantity()->setValue(3.f);
    }
    else {
        getParamQuantity()->setValue(0.0f);
    }
    

    /*
    if (value >= 2.0f) {
    //if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
        getParamQuantity()->setValue(value + 2.0f);
    }
    else if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
    //else if (value >= 2.0f) {
        //getParamQuantity()->setValue(value - 2.0f);
        getParamQuantity()->setValue(value - 2.0f);
    }
    else {
        getParamQuantity()->setValue(value > 0.5f ? 0.0f : 1.0f);
    }
    */

    if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
        e.consume(this);
    } else {
        ParamWidget::onButton(e);
    }
}

void PM8SoloMuteButton::onChange(const event::Change& e) {
    assert(_frames.size() == 4);
    if (getParamQuantity()) {
        float value = getParamQuantity()->getValue();
        assert(value >= 0.0f && value <= 3.0f);
        _svgWidget->setSvg(_frames[(int)value]);
    }
    ParamWidget::onChange(e);
}

bool PM8SoloMuteButton::isLit() {
    return module && !module->isBypassed() && getParamQuantity() && getParamQuantity()->getValue() > 0.0f;
}

void PM8SoloMuteButton::draw(const DrawArgs& args) {
    if (!isLit() || !getParamQuantity() || getParamQuantity()->getValue() < 1.0f) {
        ParamWidget::draw(args);
    }
}

void PM8SoloMuteButton::drawLit(const DrawArgs& args) {
    if (getParamQuantity() && getParamQuantity()->getValue() >= 1.0f) {
        ParamWidget::draw(args);
    }
}
// ---------------------- 

struct PolyMuter8Plus : Module {
	enum ParamId {
		FADE_PARAM,
		ENUMS(MUTE_PARAM, 8),
		PARAMS_LEN
	};
	enum InputId {
		IN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		//ENUMS(MUTE_LIGHT, 8),
		LIGHTS_LEN
	};


	//**************************************************************
	//  DEBUG 

	/*
	std::string debugDisplay = "X";
	std::string debugDisplay2 = "X";
	std::string debugDisplay3 = "X";
	std::string debugDisplay4 = "X";
	std::string debugDisplay5 = "X";
	std::string debugDisplay6 = "X";
	std::string debugDisplay7 = "X";
	std::string debugDisplay8 = "X";
	std::string debugDisplay9 = "X";
	int debugInt = 0;
	bool debugBool = false;
	*/
	
	//std::string db[4] = {"unm", "sol", "m-s", "mut"};
	
	bool initStart = false;

	int inChans = 0;
	int outChans = 0;
	//int chan;
	int buttonValue[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int prevButtonValue[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int status[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int prevStatus[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	float ampValue[8] = {1, 1, 1, 1, 1, 1, 1, 1};
	float ampDelta[8] = {1, 1, 1, 1, 1, 1, 1, 1};
	bool fading[8] = {false, false, false, false, false, false, false, false};

	float fadeKnob = 0.f;
	float prevFadeKnob = 1.f;

	float fadeValue = 0;

	int soloChans = 0;
	bool globalSolo = false;
	bool prevGlobalSolo = false;

	long sampleRate = APP->engine->getSampleRate();

	const float noEnvTime = 0.00101;

	PolyMuter8Plus() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(FADE_PARAM, 0.f, 1.f, 0.25f, "Fade", "ms", 10000.f, 1.f);
		configInput(IN_INPUT, "Poly");

		configSwitch(MUTE_PARAM, 0.0f, 3.0f, 0.0f, "Mute #1", {"Unmuted", "Solo", "Solo", "Muted"});
		configSwitch(MUTE_PARAM+1, 0.0f, 3.0f, 0.0f, "Mute #2", {"Unmuted", "Solo", "Solo/Mute", "Muted"});
		configSwitch(MUTE_PARAM+2, 0.0f, 3.0f, 0.0f, "Mute #3", {"Unmuted", "Solo", "Solo/Mute", "Muted"});
		configSwitch(MUTE_PARAM+3, 0.0f, 3.0f, 0.0f, "Mute #4", {"Unmuted", "Solo", "Solo/Mute", "Muted"});
		configSwitch(MUTE_PARAM+4, 0.0f, 3.0f, 0.0f, "Mute #5", {"Unmuted", "Solo", "Solo/Mute", "Muted"});
		configSwitch(MUTE_PARAM+5, 0.0f, 3.0f, 0.0f, "Mute #6", {"Unmuted", "Solo", "Solo/Mute", "Muted"});
		configSwitch(MUTE_PARAM+6, 0.0f, 3.0f, 0.0f, "Mute #7", {"Unmuted", "Solo", "Solo/Mute", "Muted"});
		configSwitch(MUTE_PARAM+7, 0.0f, 3.0f, 0.0f, "Mute #8", {"Unmuted", "Solo", "Solo/Mute", "Muted"});

		/*
		configSwitch(MUTE_PARAM+9, 0.f, 1.f, 0.f, "Mute #9", {"Off", "On"});
		configSwitch(MUTE_PARAM+10, 0.f, 1.f, 0.f, "Mute #10", {"Off", "On"});
		configSwitch(MUTE_PARAM+11, 0.f, 1.f, 0.f, "Mute #11", {"Off", "On"});
		configSwitch(MUTE_PARAM+12, 0.f, 1.f, 0.f, "Mute #12", {"Off", "On"});
		configSwitch(MUTE_PARAM+13, 0.f, 1.f, 0.f, "Mute #13", {"Off", "On"});
		configSwitch(MUTE_PARAM+14, 0.f, 1.f, 0.f, "Mute #14", {"Off", "On"});
		configSwitch(MUTE_PARAM+15, 0.f, 1.f, 0.f, "Mute #15", {"Off", "On"});
		*/
		
		configOutput(OUT_OUTPUT, "Poly");
	}

	void onReset(const ResetEvent &e) override {
		initStart = false;

		fadeKnob = 0.f;
		prevFadeKnob = 1.f;
		fadeValue = 0;

		soloChans = 0;
		globalSolo = false;
		prevGlobalSolo = false;

		for (int i = 0; i < 8; i++) {
			buttonValue[i] = UNMUTED;
			prevButtonValue[i] = UNMUTED;
			status[i] = UNMUTED;
			prevStatus[i] = UNMUTED;
			ampValue[i] = 1;
			ampDelta[i] = 1;
			fading[i] = false;
		}

		Module::onReset(e);
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "initStart", json_boolean(initStart));
		json_object_set_new(rootJ, "status1", json_integer(params[MUTE_PARAM].getValue()));
		json_object_set_new(rootJ, "status2", json_integer(params[MUTE_PARAM+1].getValue()));
		json_object_set_new(rootJ, "status3", json_integer(params[MUTE_PARAM+2].getValue()));
		json_object_set_new(rootJ, "status4", json_integer(params[MUTE_PARAM+3].getValue()));
		json_object_set_new(rootJ, "status5", json_integer(params[MUTE_PARAM+4].getValue()));
		json_object_set_new(rootJ, "status6", json_integer(params[MUTE_PARAM+5].getValue()));
		json_object_set_new(rootJ, "status7", json_integer(params[MUTE_PARAM+6].getValue()));
		json_object_set_new(rootJ, "status8", json_integer(params[MUTE_PARAM+7].getValue()));
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* initStartJ = json_object_get(rootJ, "initStart");
		if (initStartJ)
			initStart = json_boolean_value(initStartJ);

		if (initStart) {
			for (int i = 0; i < 8; i++) {
				params[MUTE_PARAM+i].setValue(0.f);
			}
		} else {
			json_t* status1J = json_object_get(rootJ, "status1");
			if (status1J){
				buttonValue[0] = json_integer_value(status1J);
				firstStatusCheck(0);
			}

			json_t* status2J = json_object_get(rootJ, "status2");
			if (status2J){
				buttonValue[1] = json_integer_value(status2J);
				firstStatusCheck(1);
			}

			json_t* status3J = json_object_get(rootJ, "status3");
			if (status3J){
				buttonValue[2] = json_integer_value(status3J);
				firstStatusCheck(2);
			}

			json_t* status4J = json_object_get(rootJ, "status4");
			if (status4J){
				buttonValue[3] = json_integer_value(status4J);
				firstStatusCheck(3);
			}

			json_t* status5J = json_object_get(rootJ, "status5");
			if (status5J){
				buttonValue[4] = json_integer_value(status5J);
				firstStatusCheck(4);
			}

			json_t* status6J = json_object_get(rootJ, "status6");
			if (status6J){
				buttonValue[5] = json_integer_value(status6J);
				firstStatusCheck(5);
			}

			json_t* status7J = json_object_get(rootJ, "status7");
			if (status7J){
				buttonValue[6] = json_integer_value(status7J);
				firstStatusCheck(6);
				
			}

			json_t* status8J = json_object_get(rootJ, "status8");
			if (status8J){
				buttonValue[7] = json_integer_value(status8J);
				firstStatusCheck(7);
			}
		}
	}

	void inline firstStatusCheck(int c) {
		if (buttonValue[c] >= 0 && buttonValue[c] < 4) {
			switch (buttonValue[c]) {
				case MUTED:
					prevButtonValue[c] = MUTED;
					prevStatus[c] = MUTED;
					status[c] = MUTED;
					ampValue[c] = 0;
				break;

				case SOLOED:
					prevGlobalSolo = false;
					globalSolo = true;
					prevStatus[c] = UNMUTED;
					status[c] = SOLOED;
					ampValue[c] = 1;
				break;

				case MUTED_SOLOED:
					prevGlobalSolo = false;
					globalSolo = true;
					prevStatus[c] = MUTED;
					status[c] = MUTED_SOLOED;
					ampValue[c] = 1;
				break;
			}
		} else {
			buttonValue[c] = 0;
		}

	}
	
	void inline fadeIn(int c) {
		if (fadeValue > noEnvTime) {
			fading[c] = true;
			ampDelta[c] = 1 / fadeValue / sampleRate;
		} else {
			ampValue[c] = 1.f;
		}
	}

	void inline fadeOut(int c) {
		if (fadeValue > noEnvTime) {
			fading[c] = true;
			ampDelta[c] = -1 / fadeValue / sampleRate;
		} else {
			ampValue[c] = 0.f;
		}
	}

	void process(const ProcessArgs& args) override {

		fadeKnob = params[FADE_PARAM].getValue();

		if (fadeKnob != prevFadeKnob) {
			fadeValue = std::pow(10000.f, fadeKnob) / 1000;
			prevFadeKnob = fadeKnob;
		}

		if (globalSolo != prevGlobalSolo) {
			if (globalSolo) {	// solo on
				for (int c = 0; c < 8; c++) {
					if (status[c] == UNMUTED) {
						fadeOut(c);
						prevStatus[c] = status[c];

					} else if (status[c] == MUTED) {
						fadeOut(c);
						prevStatus[c] = status[c];
					}
				}
			} else {	// solo off
				for (int c = 0; c < 8; c++) {
					if (prevStatus[c] == UNMUTED) {
						fadeIn(c);
						
						status[c] = UNMUTED;

					} else if (prevStatus[c] == MUTED) {
						fadeOut(c);

						status[c] = MUTED;						
					}

				}
			}
			prevGlobalSolo = globalSolo;
		}

		for (int c = 0; c < 8; c++) {

			// 0 unmuted
			// 1 soloed
			// 2 soloed muted
			// 3 muted

			buttonValue[c] = int(params[MUTE_PARAM+c].getValue());

			if (buttonValue[c] != prevButtonValue[c]) {
				switch (prevButtonValue[c]) {
					case UNMUTED:
						switch (buttonValue[c]) {
							case MUTED:
								prevStatus[c] = MUTED;
								fadeOut(c);
								status[c] = MUTED;
							break;

							case SOLOED:
								soloChans++;
								fadeIn(c);
								status[c] = SOLOED;
								if (!globalSolo)
									prevStatus[c] = UNMUTED;
							break;

							case MUTED_SOLOED:
								soloChans++;
								fadeIn(c);
								status[c] = MUTED_SOLOED;
							break;

						}

					break;

					// -------------------------------------

					case MUTED:
						switch (buttonValue[c]) {
							case UNMUTED:
								if (!globalSolo) 
									fadeIn(c);
								prevStatus[c] = UNMUTED;
								
									
								status[c] = UNMUTED;

							break;

							case SOLOED:
								soloChans++;
								fadeIn(c);
								status[c] = SOLOED;
							break;

							case MUTED_SOLOED:
								soloChans++;
								fadeIn(c);
								status[c] = MUTED_SOLOED;
								if (!globalSolo)
									prevStatus[c] = MUTED;
							break;
						}

					break;

					// -------------------------------------

					case MUTED_SOLOED:
						switch (buttonValue[c]) {
							case UNMUTED:
								soloChans--;
								if (globalSolo) {
									if (prevStatus[c] == MUTED) {
										fadeOut(c);
									} else
										fadeIn(c);
									prevStatus[c] = UNMUTED;
								} else {
									fadeIn(c);
								}
								status[c] = UNMUTED;
							break;

							case MUTED:
								soloChans--;
								fadeOut(c);
								status[c] = MUTED;
							break;
						}

					break;

					// -------------------------------------

					case SOLOED:
						switch (buttonValue[c]) {

							case UNMUTED:
								if (!globalSolo) {
									fadeIn(c);
								} else {
									fadeOut(c);
								}
								soloChans--;

								status[c] = UNMUTED;
							break;

							case MUTED:
								fadeOut(c);
								soloChans--;
								status[c] = MUTED;
							break;

						}

					break;

				}
			}
			prevButtonValue[c] = buttonValue[c];
		}

		if (soloChans == 0) {
			globalSolo = false;
		} else {
			globalSolo = true;
		}

		/*
		debugDisplay = db[prevButtonValue[0]];
		debugDisplay2 = db[buttonValue[0]];
		debugDisplay3 = db[prevStatus[0]];
		debugDisplay4 = db[status[0]];
		debugDisplay5 = db[prevButtonValue[1]];
		debugDisplay6 = db[buttonValue[1]];
		debugDisplay7 = db[prevStatus[1]];
		debugDisplay8 = db[status[1]];
		debugDisplay9 = to_string(soloChans);
		*/

		inChans = std::max(1, inputs[IN_INPUT].getChannels());
		
		if (inChans < 9)
			outChans = inChans;
		else
			outChans = 8;
		

		for (int c = 0; c < outChans; c++) {

			if (fading[c]) {
				ampValue[c] += ampDelta[c];
				if (ampValue[c] > 1.f) {
					fading[c] = false;
					ampValue[c] = 1.f;
				} else if (ampValue[c] < 0.f) {
					fading[c] = false;
					ampValue[c] = 0.f;
				}
			}

			outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage(c) * ampValue[c], c);
		}

		outputs[OUT_OUTPUT].setChannels(outChans);

	}
};

struct PolyMuter8PlusDisplayChan : TransparentWidget {
	PolyMuter8Plus *module;
	int frame = 0;
	PolyMuter8PlusDisplayChan() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED));					
				if (module->inChans > 9)
					nvgTextBox(args.vg, 1.5, 17, 60, to_string(module->inChans).c_str(), NULL);
				else
					nvgTextBox(args.vg, 9.8, 17, 60, to_string(module->inChans).c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}
};

/*
struct PolyMuter8PlusDebugDisplay : TransparentWidget {
	PolyMuter8Plus *module;
	int frame = 0;
	PolyMuter8PlusDebugDisplay() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-bold.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff)); 
				
				nvgTextBox(args.vg, 30, 0,120, module->debugDisplay.c_str(), NULL);
				nvgTextBox(args.vg, 30, 10,120, module->debugDisplay2.c_str(), NULL);
				nvgTextBox(args.vg, 30, 20,120, module->debugDisplay3.c_str(), NULL);
				nvgTextBox(args.vg, 30, 30,120, module->debugDisplay4.c_str(), NULL);

				nvgTextBox(args.vg, 60, 0,120, module->debugDisplay5.c_str(), NULL);
				nvgTextBox(args.vg, 60, 10,120, module->debugDisplay6.c_str(), NULL);
				nvgTextBox(args.vg, 60, 20,120, module->debugDisplay7.c_str(), NULL);
				nvgTextBox(args.vg, 60, 30,120, module->debugDisplay8.c_str(), NULL);
				nvgTextBox(args.vg, 60, 40,120, module->debugDisplay9.c_str(), NULL);

			}
		}
		Widget::drawLayer(args, layer);
	}
};
*/


struct PolyMuter8PlusWidget : ModuleWidget {
	PolyMuter8PlusWidget(PolyMuter8Plus* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/PolyMuter8Plus.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		{
			PolyMuter8PlusDisplayChan *display = new PolyMuter8PlusDisplayChan();
			display->box.pos = mm2px(Vec(10.7, 13));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}
		
		/*
		{
			PolyMuter8PlusDebugDisplay *display = new PolyMuter8PlusDebugDisplay();
			display->box.pos = Vec(3, 25);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/

		const float xCenter = 10.1f;
		const float xLeft = 6.5f;
		const float xRight = 14.f;

		const float yIn = 17;
		const float yFade = 30;
		const float yOut = 116;

		constexpr float yStart = 42.5;
		constexpr float yStart2 = 50.5;
		constexpr float y = 8.f;

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yIn)), module, PolyMuter8Plus::IN_INPUT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, yFade)), module, PolyMuter8Plus::FADE_PARAM));

		for (int i = 0; i < 8; i=i+2) {
			addParam(createParamCentered<PM8SoloMuteButton>(mm2px(Vec(xLeft, yStart+(i*y))), module, PolyMuter8Plus::MUTE_PARAM+i));
		}

		for (int i = 1; i < 8; i=i+2) {
			addParam(createParamCentered<PM8SoloMuteButton>(mm2px(Vec(xRight, yStart2+((i-1)*y))), module, PolyMuter8Plus::MUTE_PARAM+i));
		}

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xCenter, yOut)), module, PolyMuter8Plus::OUT_OUTPUT));

	}

	void appendContextMenu(Menu* menu) override {
		PolyMuter8Plus* module = dynamic_cast<PolyMuter8Plus*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Right-click on buttons"));
		menu->addChild(createMenuLabel("to SOLO channel"));
		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}
};

Model* modelPolyMuter8Plus = createModel<PolyMuter8Plus, PolyMuter8PlusWidget>("PolyMuter8Plus");