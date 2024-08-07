#define COLOR_LCD_RED 0xdd, 0x33, 0x33, 0xff
#define COLOR_LCD_GREEN 0x33, 0xdd, 0x33, 0xff

#include "plugin.hpp"

using namespace std;

struct PolyMuter8 : Module {
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
		ENUMS(MUTE_LIGHT, 8),
		LIGHTS_LEN
	};


	//**************************************************************
	//  DEBUG 

	/*	
	std::string debugDisplay = "X";
	std::string debugDisplay2 = "X";
	std::string debugDisplay3 = "X";
	std::string debugDisplay4 = "X";
	int debugInt = 0;
	bool debugBool = false;
	*/
	
	bool shrink = false;
	bool prevShrink = false;
	bool shrink10v = false;
	int progChan;

	bool showOut = false;

	bool initStart = false;

	int inChans = 0;
	float inValue = 0.f;
	int outChans = 0;

	int mute[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	float prevMute[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	float ampValue[8] = {1, 1, 1, 1, 1, 1, 1, 1};
	float ampDelta[8] = {1, 1, 1, 1, 1, 1, 1, 1};
	bool fading[8] = {false, false, false, false, false, false, false, false};

	float fadeKnob = 0.f;
	float prevFadeKnob = 1.f;

	float fadeValue = 0;

	const float noEnvTime = 0.00101;

	PolyMuter8() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(FADE_PARAM, 0.f, 1.f, 0.25f, "Fade", "ms", 10000.f, 1.f);
		configInput(IN_INPUT, "Poly");

		configSwitch(MUTE_PARAM, 0.f, 1.f, 0.f, "Mute #1", {"Off", "On"});
		configSwitch(MUTE_PARAM+1, 0.f, 1.f, 0.f, "Mute #1", {"Off", "On"});
		configSwitch(MUTE_PARAM+2, 0.f, 1.f, 0.f, "Mute #2", {"Off", "On"});
		configSwitch(MUTE_PARAM+3, 0.f, 1.f, 0.f, "Mute #3", {"Off", "On"});
		configSwitch(MUTE_PARAM+4, 0.f, 1.f, 0.f, "Mute #4", {"Off", "On"});
		configSwitch(MUTE_PARAM+5, 0.f, 1.f, 0.f, "Mute #5", {"Off", "On"});
		configSwitch(MUTE_PARAM+6, 0.f, 1.f, 0.f, "Mute #6", {"Off", "On"});
		configSwitch(MUTE_PARAM+7, 0.f, 1.f, 0.f, "Mute #7", {"Off", "On"});
		
		configOutput(OUT_OUTPUT, "Poly");
	}

	void onReset(const ResetEvent &e) override {
		showOut = false;
		shrink = false;
		prevShrink = false;
		shrink10v = false;
		initStart = false;

		fadeKnob = 0.f;
		prevFadeKnob = 1.f;
		fadeValue = 0;

		for (int i = 0; i < 8; i++) {
			mute[i] = 0;
			prevMute[i] = 0;
			ampValue[i] = 1;
			ampDelta[i] = 1;
			fading[i] = false;
		}

		Module::onReset(e);
	}

	
	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "showOut", json_boolean(showOut));
		json_object_set_new(rootJ, "shrink", json_boolean(shrink));
		json_object_set_new(rootJ, "shrink10v", json_boolean(shrink10v));
		json_object_set_new(rootJ, "initStart", json_boolean(initStart));
		for (int i=0; i < 8; i++)
			json_object_set_new(rootJ, ("mute"+to_string(i)).c_str(), json_integer(params[MUTE_PARAM+i].getValue()));

		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* showOutJ = json_object_get(rootJ, "showOut");
		if (showOutJ)
			showOut = json_boolean_value(showOutJ);

		json_t* shrinkJ = json_object_get(rootJ, "shrink");
		if (shrinkJ)
			shrink = json_boolean_value(shrinkJ);

		json_t* shrink10vJ = json_object_get(rootJ, "shrink10v");
		if (shrink10vJ)
			shrink10v = json_boolean_value(shrink10vJ);

		json_t* initStartJ = json_object_get(rootJ, "initStart");
		if (initStartJ)
			initStart = json_boolean_value(initStartJ);

		if (initStart) {
			for (int i = 0; i < 8; i++)
				params[MUTE_PARAM+i].setValue(0.f);
		} else {
			for (int i = 0; i < 8; i++){
				json_t* muteJ = json_object_get(rootJ, ("mute"+to_string(i)).c_str());
				if (muteJ){
					mute[i] = json_integer_value(muteJ);
					if (mute[i] == 1) {
						prevMute[i] = 1;
						ampValue[i] = 0;
					}
				}
			}
		}
	}
	


	void process(const ProcessArgs& args) override {

		if (!shrink && prevShrink) {
			for (int c = 0; c < 8; c++) {
				if (mute[c]) {
					prevMute[c] = 1;
					ampValue[c] = 0;
				} else {
					prevMute[c] = 0;
					ampValue[c] = 1;
				}
			}
		}

		if (!shrink) {

			// ********************************* STANDARD MUTER *****************************

			fadeKnob = params[FADE_PARAM].getValue();

			if (fadeKnob != prevFadeKnob) {
				fadeValue = std::pow(10000.f, fadeKnob) / 1000;
				prevFadeKnob = fadeKnob;
			}

			inChans = std::max(1, inputs[IN_INPUT].getChannels());

			if (inChans < 9)
				outChans = inChans;
			else
				outChans = 8;
			
			for (int c = 0; c < 8; c++) {
				mute[c] = params[MUTE_PARAM+c].getValue();
				lights[MUTE_LIGHT+c].setBrightness(mute[c]);

				if (mute[c] && !prevMute[c]) {	// mute chan
					if (fadeValue > noEnvTime) {
						fading[c] = true;
						ampDelta[c] = -1 / fadeValue / args.sampleRate;
					} else {
						ampValue[c] = 0.f;
					}
					
				} else if (!mute[c] && prevMute[c]) { // unmute chan
					if (fadeValue > noEnvTime) {
						fading[c] = true;
						ampDelta[c] = 1 / fadeValue / args.sampleRate;
					} else {
						ampValue[c] = 1.f;
					}
				}
				prevMute[c] = mute[c];

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

		} else {

			// ***************************** SHRINK CHANNELS *******************

			inChans = std::max(1, inputs[IN_INPUT].getChannels());
			if (inChans > 8)
				inChans = 8;

			progChan = 0;
			
			for (int c = 0; c < inChans; c++) {
				mute[c] = params[MUTE_PARAM+c].getValue();
				lights[MUTE_LIGHT+c].setBrightness(mute[c]);

				inValue = inputs[IN_INPUT].getVoltage(c);

				if (!mute[c]) {
					if (shrink10v) {
						if (inValue > -10.f) {
							outputs[OUT_OUTPUT].setVoltage(inValue, progChan);
							progChan++;
						}
					} else {
						outputs[OUT_OUTPUT].setVoltage(inValue, progChan);
						progChan++;
					}
				}
			}

			outChans = progChan;

			prevShrink = true;

		}

		outputs[OUT_OUTPUT].setChannels(outChans);

	}
};

struct PolyMuter8DisplayChan : TransparentWidget {
	PolyMuter8 *module;
	int frame = 0;
	PolyMuter8DisplayChan() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				if (!module->showOut) {
					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED));
					if (module->inChans > 9)
						nvgTextBox(args.vg, 1.5, 17, 60, to_string(module->inChans).c_str(), NULL);
					else
						nvgTextBox(args.vg, 9.8, 17, 60, to_string(module->inChans).c_str(), NULL);
				} else {
					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_GREEN));
					nvgTextBox(args.vg, 9.8, 17, 60, to_string(module->outChans).c_str(), NULL);
				}
			}
		}
		Widget::drawLayer(args, layer);
	}
};
/*
struct PolyMuter8DebugDisplay : TransparentWidget {
	PolyMuter8 *module;
	int frame = 0;
	PolyMuter8DebugDisplay() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-bold.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff)); 
				
				nvgTextBox(args.vg, 0, 0,120, module->debugDisplay.c_str(), NULL);
				//nvgTextBox(args.vg, 9, 16,120, module->debugDisplay2.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 6,120, module->debugDisplay3.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 16,120, module->debugDisplay4.c_str(), NULL);

			}
		}
		Widget::drawLayer(args, layer);
	}
};
*/

struct PolyMuter8Widget : ModuleWidget {
	PolyMuter8Widget(PolyMuter8* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/PolyMuter8.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		{
			PolyMuter8DisplayChan *display = new PolyMuter8DisplayChan();
			display->box.pos = mm2px(Vec(10.7, 13));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}
		/*
		{
			PolyMuter8DebugDisplay *display = new PolyMuter8DebugDisplay();
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

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yIn)), module, PolyMuter8::IN_INPUT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, yFade)), module, PolyMuter8::FADE_PARAM));

		for (int i = 0; i < 8; i=i+2) {
			addParam(createLightParamCentered<VCVLightBezelLatch<RedLight>>(mm2px(Vec(xLeft, yStart+(i*y))), module, PolyMuter8::MUTE_PARAM+i, PolyMuter8::MUTE_LIGHT+i));
		}

		for (int i = 1; i < 8; i=i+2) {
			addParam(createLightParamCentered<VCVLightBezelLatch<RedLight>>(mm2px(Vec(xRight, yStart2+((i-1)*y))), module, PolyMuter8::MUTE_PARAM+i, PolyMuter8::MUTE_LIGHT+i));
		}

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xCenter, yOut)), module, PolyMuter8::OUT_OUTPUT));

	}

	void appendContextMenu(Menu* menu) override {
		PolyMuter8* module = dynamic_cast<PolyMuter8*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Show OUT channels", "", &module->showOut));
		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Shrink channels", "", &module->shrink));
		if (module->shrink)
			menu->addChild(createBoolPtrMenuItem("exclude -10v chans too", "", &module->shrink10v));
		else
			menu->addChild(createMenuLabel("exclude -10v chans too"));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}
};

Model* modelPolyMuter8 = createModel<PolyMuter8, PolyMuter8Widget>("PolyMuter8");