#define COLOR_LCD_RED 0xdd, 0x33, 0x33, 0xff
#define COLOR_LCD_GREEN 0x33, 0xdd, 0x33, 0xff

#include "plugin.hpp"

using namespace std;

struct PolyMuter16 : Module {
	enum ParamId {
		FADE_PARAM,
		ENUMS(MUTE_PARAM, 16),
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
		ENUMS(MUTE_LIGHT, 16),
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

	int mute[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevMute[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float ampValue[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	float ampDelta[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	bool fading[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	float fadeKnob = 0.f;
	float prevFadeKnob = 1.f;

	float fadeValue = 0;

	const float noEnvTime = 0.00101;

	PolyMuter16() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configInput(IN_INPUT, "Poly");
		configParam(FADE_PARAM, 0.f, 1.f, 0.25f, "Fade", "ms", 10000.f, 1.f);
		configOutput(OUT_OUTPUT, "Poly");

		configSwitch(MUTE_PARAM, 0.f, 1.f, 0.f, "Mute #1", {"Off", "On"});
		configSwitch(MUTE_PARAM+1, 0.f, 1.f, 0.f, "Mute #2", {"Off", "On"});
		configSwitch(MUTE_PARAM+2, 0.f, 1.f, 0.f, "Mute #3", {"Off", "On"});
		configSwitch(MUTE_PARAM+3, 0.f, 1.f, 0.f, "Mute #4", {"Off", "On"});
		configSwitch(MUTE_PARAM+4, 0.f, 1.f, 0.f, "Mute #5", {"Off", "On"});
		configSwitch(MUTE_PARAM+5, 0.f, 1.f, 0.f, "Mute #+", {"Off", "On"});
		configSwitch(MUTE_PARAM+6, 0.f, 1.f, 0.f, "Mute #7", {"Off", "On"});
		configSwitch(MUTE_PARAM+7, 0.f, 1.f, 0.f, "Mute #8", {"Off", "On"});
		configSwitch(MUTE_PARAM+8, 0.f, 1.f, 0.f, "Mute #9", {"Off", "On"});
		configSwitch(MUTE_PARAM+9, 0.f, 1.f, 0.f, "Mute #10", {"Off", "On"});
		configSwitch(MUTE_PARAM+10, 0.f, 1.f, 0.f, "Mute #11", {"Off", "On"});
		configSwitch(MUTE_PARAM+11, 0.f, 1.f, 0.f, "Mute #12", {"Off", "On"});
		configSwitch(MUTE_PARAM+12, 0.f, 1.f, 0.f, "Mute #13", {"Off", "On"});
		configSwitch(MUTE_PARAM+13, 0.f, 1.f, 0.f, "Mute #14", {"Off", "On"});
		configSwitch(MUTE_PARAM+14, 0.f, 1.f, 0.f, "Mute #15", {"Off", "On"});
		configSwitch(MUTE_PARAM+15, 0.f, 1.f, 0.f, "Mute #16", {"Off", "On"});
		
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

		for (int i = 0; i < 16; i++) {
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
		for (int i=0; i < 16; i++)
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
			for (int i = 0; i < 16; i++) {
				params[MUTE_PARAM+i].setValue(0.f);
			}
		} else {
			for (int i = 0; i < 16; i++){
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
			for (int c = 0; c < 16; c++) {
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
			
			for (int c = 0; c < 16; c++) {
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

			outChans = inChans;

			prevShrink = false;

		} else {

			// ***************************** SHRINK CHANNELS *******************

			inChans = std::max(1, inputs[IN_INPUT].getChannels());
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

struct PolyMuter16DisplayChan : TransparentWidget {
	PolyMuter16 *module;
	int frame = 0;
	PolyMuter16DisplayChan() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 16);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				if (!module->showOut) {	
					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED));		
					if (module->inChans > 9)
						nvgTextBox(args.vg, 4, 21.2, 60, to_string(module->inChans).c_str(), NULL);
					else
						nvgTextBox(args.vg, 17, 21.2, 60, to_string(module->inChans).c_str(), NULL);
				} else {
					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_GREEN));
					if (module->outChans > 9)
						nvgTextBox(args.vg, 4, 21.2, 60, to_string(module->outChans).c_str(), NULL);
					else
						nvgTextBox(args.vg, 17, 21.2, 60, to_string(module->outChans).c_str(), NULL);
				}
			}
		}
		Widget::drawLayer(args, layer);
	}
};
/*
struct PolyMuter16DebugDisplay : TransparentWidget {
	PolyMuter16 *module;
	int frame = 0;
	PolyMuter16DebugDisplay() {
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

struct PolyMuter16Widget : ModuleWidget {
	PolyMuter16Widget(PolyMuter16* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/PolyMuter16.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		{
			PolyMuter16DisplayChan *display = new PolyMuter16DisplayChan();
			display->box.pos = mm2px(Vec(20.9, 8.2));
			display->box.size = mm2px(Vec(13.2, 8.6));
			display->module = module;
			addChild(display);
		}
		/*
		{
			PolyMuter16DebugDisplay *display = new PolyMuter16DebugDisplay();
			display->box.pos = Vec(3, 25);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/

		const float xCenter = 10.1;
		const float xLeft = 12;
		const float xRight = 21;

		const float yIn = 17;
		const float yFade = 30;
		const float xOut = 27.5;
		const float yOut = 28;

		constexpr float yStart = 41.5;
		constexpr float yStart2 = 46.5;
		constexpr float y = 10.f;


		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yIn)), module, PolyMuter16::IN_INPUT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, yFade)), module, PolyMuter16::FADE_PARAM));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yOut)), module, PolyMuter16::OUT_OUTPUT));

		for (int i = 0; i < 8; i++)
			addParam(createLightParamCentered<VCVLightBezelLatch<RedLight>>(mm2px(Vec(xLeft, yStart+(i*y))), module, PolyMuter16::MUTE_PARAM+i, PolyMuter16::MUTE_LIGHT+i));

		for (int i = 0; i < 8; i++)
			addParam(createLightParamCentered<VCVLightBezelLatch<RedLight>>(mm2px(Vec(xRight, yStart2+(i*y))), module, PolyMuter16::MUTE_PARAM+8+i, PolyMuter16::MUTE_LIGHT+8+i));

	}

	void appendContextMenu(Menu* menu) override {
		PolyMuter16* module = dynamic_cast<PolyMuter16*>(this->module);

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

Model* modelPolyMuter16 = createModel<PolyMuter16, PolyMuter16Widget>("PolyMuter16");