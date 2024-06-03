#define RAMPUP 0
#define TRIANGLE 1
#define RAMPDOWN 2

#include "plugin.hpp"

//using namespace std;

struct Modulator : Module {
	enum ParamId {
		RATE_PARAM,
		RATE_ATTENUV_PARAM,
		SYNCSW_PARAM,
		PPC_PARAM,
		XRATE_PARAM,
		WAVEFORM_PARAM,
		BIPOLAR_PARAM,
		PHASERST_PARAM,
		SCALE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RATE_INPUT,
		SYNC_INPUT,
		SYNCSW_INPUT,
		SCALE_INPUT,
		RST_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		SYNCSW_LIGHT,
		BIPOLAR_LIGHT,
		LIGHTS_LEN
	};

	float rst = 0.f;
	float prevRst = 0.f;
	float phaseResetValue = 0.f;

	bool syncEnabled = false;

	float trigSyncSwitch = 0.f;
	float prevTrigSyncSwitch = 0.f;

	float syncTrig = 0.f;
	float prevSyncTrig = 0.f;

	double sampleRate = APP->engine->getSampleRate();
	double sampleCount = sampleRate;

	double clockSampleCount = 0;
	double clockMaxSample = sampleRate;

	/*
	float sampleRateCoeff = sampleCount / 2.f;
	float rateProvv = 1.f;
	float rate = 1.f;
	float rateKnob = 0.5f;
	float prevRateKnob = 1.f;

	float xRate = 1;
	float xRateKnob = 1;
	float prevXrateKnob = 0;

	float waveCoeff = rate / sampleRateCoeff;
	float syncWaveCoeff = waveCoeff;
	float waveValue = 0;
	*/

	// double is more precise
	double sampleRateCoeff = sampleCount / 2.f; 
	double rateProvv = 1.f;
	double rate = 1.f;
	double rateKnob = 0.5f;
	double prevRateKnob = 1.f;

	double xRate = 1;
	double xRateKnob = 1;
	double prevXrateKnob = 0;

	double waveCoeff = rate / sampleRateCoeff;
	double syncWaveCoeff = 0;
	double waveValue = 0;
	
	bool firstRun = true;
	bool wait2ndClock = false;

	int waitingClock = -1;
	int waitingClockCount = 1;

	int waveSlope = 1;

	int waveForm = TRIANGLE;
	bool bipolar = true;

	float scaleValue = 0;
	float out = 0.f;

	static constexpr float minRate = 0.01f;  // in milliseconds
	static constexpr float maxRate = 100.f;  // in milliseconds

	static constexpr float minXrate = 0.047619f;  // in milliseconds
	static constexpr float maxXrate = 21.f;  // in milliseconds

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

	Modulator() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RATE_PARAM, 0.f, 1.f, 0.5f, "Rate", "Hz", maxRate / minRate, minRate);
		configInput(RATE_INPUT, "Rate");
		configParam(RATE_ATTENUV_PARAM, -1.f, 1.f, 0.f, "Rate CV", "%", 0, 100);
		configSwitch(SYNCSW_PARAM, 0.f, 1.f, 0.f, "Sync", {"Off", "On"});
		configInput(SYNCSW_INPUT, "Sync Toggle");
		configInput(SYNC_INPUT, "Sync");
		configParam(PPC_PARAM, 1, 24.f, 1.f, "Pulses per Cycle");
		paramQuantities[PPC_PARAM]->snapEnabled = true;

		configParam(XRATE_PARAM, 0.f, 1.f, 0.5f, "xRate", "x", maxXrate / minXrate, minXrate);

		configSwitch(WAVEFORM_PARAM, 0.f, 2.f, 1.f, "Type", {"Ramp Up", "Triangle", "Ramp Down"});
		
		configSwitch(BIPOLAR_PARAM, 0.f, 1.f, 0.f, "Bipolar", {"Off", "On"});

		configParam(SCALE_PARAM, 0.f, 1.f, 1.f, "Scale", "%", 0, 100);
		configInput(SCALE_INPUT, "Scale CV");

		configParam(PHASERST_PARAM, 0.f, 1.f, 0.f, "Phase Rst", "°", 0, 360);
		configInput(RST_INPUT, "Reset");
		
		configOutput(OUT_OUTPUT, "OUT");

	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "wait2ndClock", json_boolean(wait2ndClock));

		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {

		json_t* wait2ndClockJ = json_object_get(rootJ, "wait2ndClock");
		if (wait2ndClockJ)
			wait2ndClock = json_boolean_value(wait2ndClockJ);
		
	}

	void onReset(const ResetEvent &e) override {
		sampleCount = 0;
		clockSampleCount = 0;

		rateKnob = 0.5;
		prevRateKnob = 1;

		xRateKnob = 1;
		prevXrateKnob = 0;

		waveSlope = 1;
		waveValue = 0;

		rst = 0;
		prevRst = 0;

		firstRun = true;
		wait2ndClock = false;

		waitingClock = -1;
		waitingClockCount = 1;

	    Module::onReset(e);
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
		sampleCount = sampleRate;
		sampleRateCoeff = sampleCount / 2;
		waveCoeff = rate / sampleRateCoeff;
	}


	static float convertXrateToSeconds(float cv) {		
		return minXrate * std::pow(maxXrate / minXrate, cv);
	}

	static float convertRateToSeconds(float cv) {		
		return minRate * std::pow(maxRate / minRate, cv);
	}

	void process(const ProcessArgs& args) override {

		waveForm = int(params[WAVEFORM_PARAM].getValue());

		bipolar = params[BIPOLAR_PARAM].getValue();
		lights[BIPOLAR_LIGHT].setBrightness(bipolar);

		rst = inputs[RST_INPUT].getVoltage();
		if (rst >= 1 && prevRst < 1) {

			if (wait2ndClock) {

				waitingClock = -1;
				syncWaveCoeff = 0;

			} else {

				if (!firstRun) {
					waitingClock = 1;
					//sampleCount = prevSampleCount;
				} else {
					syncWaveCoeff = 0;
				}

			}

			firstRun = false;

			phaseResetValue = params[PHASERST_PARAM].getValue();

			switch (waveForm) {

				case TRIANGLE:
					if (phaseResetValue < 0.5) {
						waveSlope = 1;
						waveValue = phaseResetValue * 2;
						if (bipolar)
							waveValue += 0.5;

					} else {
						waveSlope = -1;
						waveValue = 1 - ((phaseResetValue - 0.5) * 2);
						if (bipolar)
							waveValue -= 0.5;
					}
				break;

				case RAMPUP:
					waveSlope = 1;
					waveValue = phaseResetValue;
					if (bipolar)
						waveValue += 0.5;
				break;

				case RAMPDOWN:
					waveSlope = -1;
					waveValue = 1 - phaseResetValue;
					if (bipolar)
						waveValue -= 0.5;
				break;
			}

		}
		prevRst = rst;

		trigSyncSwitch = inputs[SYNCSW_INPUT].getVoltage();

		if (trigSyncSwitch >= 1.f && prevTrigSyncSwitch < 1) {
			if (syncEnabled) {
				syncEnabled = false;
				params[SYNCSW_PARAM].setValue(0.f);
				lights[SYNCSW_LIGHT].setBrightness(0.f);
			} else {
				syncEnabled = true;
				params[SYNCSW_PARAM].setValue(1.f);
				lights[SYNCSW_LIGHT].setBrightness(1.f);
			}
		}
		prevTrigSyncSwitch = trigSyncSwitch;

		syncEnabled = params[SYNCSW_PARAM].getValue();
		lights[SYNCSW_LIGHT].setBrightness(syncEnabled);


		if (inputs[SYNC_INPUT].isConnected()) {

			xRateKnob = params[XRATE_PARAM].getValue();

			if (xRateKnob != prevXrateKnob) {
				xRate = convertXrateToSeconds(xRateKnob);
				prevXrateKnob = xRateKnob;
			}

			syncTrig = inputs[SYNC_INPUT].getVoltage();

			if (syncTrig >= 1 && prevSyncTrig < 1) {

				if (waitingClock == 0) {

					syncWaveCoeff = 2 / sampleCount / params[PPC_PARAM].getValue();

					waitingClockCount++;

					if (syncEnabled && waitingClockCount > params[PPC_PARAM].getValue())
						waitingClockCount = 1;

				} else if (waitingClock < 0) {

					waitingClock = 1;

					waitingClockCount = 1;

					syncWaveCoeff = 0;

				} else {	// waitingClock = 1

					waitingClockCount++;

					if (waitingClockCount > params[PPC_PARAM].getValue()) {

						waitingClock = 0;

						waitingClockCount = 1;

						syncWaveCoeff = 2 / sampleCount / params[PPC_PARAM].getValue();

					}

				}
				
				sampleCount = 0;
			}
			
			prevSyncTrig = syncTrig;

			sampleCount++;
		
		} else {

			waitingClock = -1;

			syncWaveCoeff = 0;
		}


		if (!syncEnabled) {

			rateKnob = params[RATE_PARAM].getValue();
			
			if (rateKnob != prevRateKnob) {
				rateProvv = convertRateToSeconds(rateKnob);
				prevRateKnob = rateKnob;
			}

			rate = rateProvv + (inputs[RATE_INPUT].getVoltage() * params[RATE_ATTENUV_PARAM].getValue() * 10);

			if (rate > 100)
				rate = 100;
			else if (rate < 0.01)
				rate = 0.01;

			waveCoeff = rate / sampleRateCoeff;

		} else {

			waveCoeff = syncWaveCoeff * xRate;

		}
		
		switch (waveForm) {
			case TRIANGLE:

				waveValue += waveCoeff * waveSlope;
				
				if (waveValue > 1) {
					waveSlope = -1;
					waveValue = 2 - waveValue;
				} else if (waveValue < 0) {
					waveSlope = 1;
					waveValue = -waveValue;
				}

			break;

			case RAMPUP:

				waveValue += waveCoeff * waveSlope / 2;

				if (waveValue > 1) {
					waveSlope = 1;
					waveValue = waveValue - 1;
				} else if (waveSlope < 0) {
					waveSlope = 1;
				}

			break;

			case RAMPDOWN:

				waveValue += waveCoeff * waveSlope / 2;

				if (waveValue < 0) {
					waveSlope = -1;
					waveValue = 1 + waveValue;
				} else if (waveSlope > 0) {
					waveSlope = -1;
				}

			break;
		}

		scaleValue = params[SCALE_PARAM].getValue() + (inputs[SCALE_INPUT].getVoltage() * .1f);

		if (scaleValue > 1)
			scaleValue = 1;
		else if (scaleValue < 0)
			scaleValue = 0;

		if (bipolar)
			out = ((waveValue * 10.f) - 5.f) * scaleValue;
		else
			out = waveValue * 10.f * scaleValue;
			
		outputs[OUT_OUTPUT].setVoltage(out);

		//debugDisplay = to_string(waveCoeff);
	}
};

/*
struct ModulatorDebugDisplay : TransparentWidget {
	Modulator *module;
	int frame = 0;
	ModulatorDebugDisplay() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-bold.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff)); 
				
				nvgTextBox(args.vg, 9, 6,120, module->debugDisplay.c_str(), NULL);
				//nvgTextBox(args.vg, 9, 16,120, module->debugDisplay2.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 6,120, module->debugDisplay3.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 16,120, module->debugDisplay4.c_str(), NULL);

			}
		}
		Widget::drawLayer(args, layer);
	}
};
*/

struct ModulatorWidget : ModuleWidget {
	ModulatorWidget(Modulator* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Modulator.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH))); 

		/*
		{
			ModulatorDebugDisplay *display = new ModulatorDebugDisplay();
			display->box.pos = Vec(23, 3);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/


		const float xRtKnob = 8;
		const float yRtKnob = 15.9;

		const float xRateAtnv = 19;
		const float yRateAtnv = 21.9;

		const float xRateIn = 10;
		const float yRateIn = 27;

		const float xSyncSw = 7.3;
		const float ySyncSw = 43.5;

		const float xSyncSwIn = 18.5;
		const float ySyncSwIn = 43.5;

		const float xSync = 7.3;
		const float ySync = 56.2;

		const float xPpc = 18.2;
		const float yPpc = 56.2;

		const float xRt = 12.7;
		const float yRt = 65.7;

		const float xType = 8.5;
		const float yType = 79.5;

		const float xBi = 19.5;
		const float yBi = 79.5;

		const float xScl = 7.3;
		const float yScl = 93.5;

		const float xSclIn = 18.5;
		const float ySclIn = 93.5;

		const float xRst = 7.3; 
		const float yRst = 105.8; 

		const float xPhR = 18.5; 
		const float yPhR = 105.8; 

		const float xOut = 18.5;
		const float yOut = 117.4;

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xRtKnob, yRtKnob)), module, Modulator::RATE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xRateAtnv, yRateAtnv)), module, Modulator::RATE_ATTENUV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRateIn, yRateIn)), module, Modulator::RATE_INPUT));

		addParam(createLightParamCentered<VCVLightBezelLatch<GreenLight>>(mm2px(Vec(xSyncSw, ySyncSw)), module, Modulator::SYNCSW_PARAM, Modulator::SYNCSW_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xSyncSwIn, ySyncSwIn)), module, Modulator::SYNCSW_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xSync, ySync)), module, Modulator::SYNC_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPpc, yPpc)), module, Modulator::PPC_PARAM));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xRt, yRt)), module, Modulator::XRATE_PARAM));
	
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(xType, yType)), module, Modulator::WAVEFORM_PARAM));		

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(xBi, yBi)), module, Modulator::BIPOLAR_PARAM, Modulator::BIPOLAR_LIGHT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xScl, yScl)), module, Modulator::SCALE_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xSclIn, ySclIn)), module, Modulator::SCALE_INPUT));
		
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRst, yRst)), module, Modulator::RST_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPhR, yPhR)), module, Modulator::PHASERST_PARAM));

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yOut)), module, Modulator::OUT_OUTPUT));

	}

	void appendContextMenu(Menu* menu) override {
		Modulator* module = dynamic_cast<Modulator*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Sync settings"));
		menu->addChild(createBoolPtrMenuItem("Wait full clock after reset", "", &module->wait2ndClock));

	}

};

Model* modelModulator = createModel<Modulator, ModulatorWidget>("Modulator");