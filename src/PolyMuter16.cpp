#define COLOR_LCD_RED 0xdd, 0x33, 0x33, 0xff

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
	
	bool initStart = false;

	int inChans = 0;
	//int outChans = 0;
	//int chan;
	float mute[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
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
		json_object_set_new(rootJ, "initStart", json_boolean(initStart));
		json_object_set_new(rootJ, "mute1", json_real(params[MUTE_PARAM].getValue()));
		json_object_set_new(rootJ, "mute2", json_real(params[MUTE_PARAM+1].getValue()));
		json_object_set_new(rootJ, "mute3", json_real(params[MUTE_PARAM+2].getValue()));
		json_object_set_new(rootJ, "mute4", json_real(params[MUTE_PARAM+3].getValue()));
		json_object_set_new(rootJ, "mute5", json_real(params[MUTE_PARAM+4].getValue()));
		json_object_set_new(rootJ, "mute6", json_real(params[MUTE_PARAM+5].getValue()));
		json_object_set_new(rootJ, "mute7", json_real(params[MUTE_PARAM+6].getValue()));
		json_object_set_new(rootJ, "mute8", json_real(params[MUTE_PARAM+7].getValue()));
		json_object_set_new(rootJ, "mute9", json_real(params[MUTE_PARAM+8].getValue()));
		json_object_set_new(rootJ, "mute10", json_real(params[MUTE_PARAM+9].getValue()));
		json_object_set_new(rootJ, "mute11", json_real(params[MUTE_PARAM+10].getValue()));
		json_object_set_new(rootJ, "mute12", json_real(params[MUTE_PARAM+11].getValue()));
		json_object_set_new(rootJ, "mute13", json_real(params[MUTE_PARAM+12].getValue()));
		json_object_set_new(rootJ, "mute14", json_real(params[MUTE_PARAM+13].getValue()));
		json_object_set_new(rootJ, "mute15", json_real(params[MUTE_PARAM+14].getValue()));
		json_object_set_new(rootJ, "mute16", json_real(params[MUTE_PARAM+15].getValue()));
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
			json_t* mute1J = json_object_get(rootJ, "mute1");
			if (mute1J){
				mute[0] = json_real_value(mute1J);
				if (mute[0] == 1.f) {
					prevMute[0] = 1;
					ampValue[0] = 0;
				}
			}

			json_t* mute2J = json_object_get(rootJ, "mute2");
			if (mute2J){
				mute[1] = json_real_value(mute2J);
				if (mute[1] == 1.f) {
					prevMute[1] = 1;
					ampValue[1] = 0;
				}
			}

			json_t* mute3J = json_object_get(rootJ, "mute3");
			if (mute3J){
				mute[2] = json_real_value(mute3J);
				if (mute[2] == 1.f) {
					prevMute[2] = 1;
					ampValue[2] = 0;
				}
			}

			json_t* mute4J = json_object_get(rootJ, "mute4");
			if (mute4J){
				mute[3] = json_real_value(mute4J);
				if (mute[3] == 1.f) {
					prevMute[3] = 1;
					ampValue[3] = 0;
				}
			}

			json_t* mute5J = json_object_get(rootJ, "mute5");
			if (mute5J){
				mute[4] = json_real_value(mute5J);
				if (mute[4] == 1.f) {
					prevMute[4] = 1;
					ampValue[4] = 0;
				}
			}

			json_t* mute6J = json_object_get(rootJ, "mute6");
			if (mute6J){
				mute[5] = json_real_value(mute6J);
				if (mute[5] == 1.f) {
					prevMute[5] = 1;
					ampValue[5] = 0;
				}
			}

			json_t* mute7J = json_object_get(rootJ, "mute7");
			if (mute7J){
				mute[6] = json_real_value(mute7J);
				if (mute[6] == 1.f) {
					prevMute[6] = 1;
					ampValue[6] = 0;
				}
			}

			json_t* mute8J = json_object_get(rootJ, "mute8");
			if (mute8J){
				mute[7] = json_real_value(mute8J);
				if (mute[7] == 1.f) {
					prevMute[7] = 1;
					ampValue[7] = 0;
				}
			}

			json_t* mute9J = json_object_get(rootJ, "mute9");
			if (mute9J){
				mute[8] = json_real_value(mute9J);
				if (mute[8] == 1.f) {
					prevMute[8] = 1;
					ampValue[8] = 0;
				}
			}

			json_t* mute10J = json_object_get(rootJ, "mute10");
			if (mute10J){
				mute[9] = json_real_value(mute9J);
				if (mute[9] == 1.f) {
					prevMute[9] = 1;
					ampValue[9] = 0;
				}
			}

			json_t* mute11J = json_object_get(rootJ, "mute11");
			if (mute11J){
				mute[10] = json_real_value(mute11J);
				if (mute[10] == 1.f) {
					prevMute[10] = 1;
					ampValue[10] = 0;
				}
			}

			json_t* mute12J = json_object_get(rootJ, "mute12");
			if (mute12J){
				mute[11] = json_real_value(mute12J);
				if (mute[11] == 1.f) {
					prevMute[11] = 1;
					ampValue[11] = 0;
				}
			}

			json_t* mute13J = json_object_get(rootJ, "mute13");
			if (mute13J){
				mute[12] = json_real_value(mute13J);
				if (mute[12] == 1.f) {
					prevMute[12] = 1;
					ampValue[12] = 0;
				}
			}

			json_t* mute14J = json_object_get(rootJ, "mute14");
			if (mute14J){
				mute[13] = json_real_value(mute14J);
				if (mute[13] == 1.f) {
					prevMute[13] = 1;
					ampValue[13] = 0;
				}
			}

			json_t* mute15J = json_object_get(rootJ, "mute15");
			if (mute15J){
				mute[14] = json_real_value(mute15J);
				if (mute[14] == 1.f) {
					prevMute[14] = 1;
					ampValue[14] = 0;
				}
			}

			json_t* mute16J = json_object_get(rootJ, "mute16");
			if (mute16J){
				mute[15] = json_real_value(mute16J);
				if (mute[15] == 1.f) {
					prevMute[15] = 1;
					ampValue[15] = 0;
				}
			}
		}
	}
	
	void process(const ProcessArgs& args) override {

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

		outputs[OUT_OUTPUT].setChannels(inChans);

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

				nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED));					
				if (module->inChans > 9)
					nvgTextBox(args.vg, 4, 20.8, 60, to_string(module->inChans).c_str(), NULL);
				else
					nvgTextBox(args.vg, 17, 20.8, 60, to_string(module->inChans).c_str(), NULL);
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

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

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

		for (int i = 0; i < 8; i++) {
			addParam(createLightParamCentered<VCVLightBezelLatch<RedLight>>(mm2px(Vec(xLeft, yStart+(i*y))), module, PolyMuter16::MUTE_PARAM+i, PolyMuter16::MUTE_LIGHT+i));
		}

		for (int i = 0; i < 8; i++) {
			addParam(createLightParamCentered<VCVLightBezelLatch<RedLight>>(mm2px(Vec(xRight, yStart2+(i*y))), module, PolyMuter16::MUTE_PARAM+8+i, PolyMuter16::MUTE_LIGHT+8+i));
		}

	}

	void appendContextMenu(Menu* menu) override {
		PolyMuter16* module = dynamic_cast<PolyMuter16*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}
};

Model* modelPolyMuter16 = createModel<PolyMuter16, PolyMuter16Widget>("PolyMuter16");