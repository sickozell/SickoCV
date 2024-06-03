#define RAMPUP 0
#define TRIANGLE 1
#define RAMPDOWN 2

#include "plugin.hpp"

//using namespace std;	// this is for debug

struct Modulator7 : Module {
	enum ParamId {
		RATE_PARAM,
		RATE_ATTENUV_PARAM,
		SYNCSW_PARAM,
		PPC_PARAM,
		POLY_PARAM,
		RST_PARAM,
		ENUMS(PHASERST_PARAM, 7),
		ENUMS(WAVEFORM_PARAM, 7),
		ENUMS(XRATE_PARAM, 7),
		ENUMS(BIPOLAR_PARAM, 7),
		ENUMS(SCALE_PARAM, 7),
		PARAMS_LEN
	};
	enum InputId {
		RATE_INPUT,
		SYNC_INPUT,
		SYNCSW_INPUT,
		RST_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT, 7),
		RST_OUTPUT,
		POLY_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		SYNCSW_LIGHT,
		RST_LIGHT,
		ENUMS(BIPOLAR_LIGHT, 7),
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
	double sampleCount = 0;
	//double prevSampleCount = 0;
	
	float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	bool clockPulse = false;
	float clockPulseTime = 0.f;

	bool resetPulse = false;
	float resetPulseTime = 0.f;

	int polyKnob = 1;
	double clockSampleCount = 0;
	double clockMaxSample = sampleRate;

	double sampleRateCoeff = sampleRate / 2;
	double rateProvv = 1.f;
	double rate = 1.f;
	double rateKnob = 0.5f;
	double prevRateKnob = 1.f;
	double xRate[7] = {1, 1, 1, 1, 1, 1, 1};
	double xRateKnob[7] = {1, 1, 1, 1, 1, 1, 1};
	double prevXrateKnob[7] = {0, 0, 0, 0, 0, 0, 0};
	double waveCoeff = 0;
	double syncWaveCoeff = 0;
	double waveValue[7] = {0, 0, 0, 0, 0, 0, 0};

	int waveSlope[7] = {1, 1, 1, 1, 1, 1, 1};
	int waveForm[7] = {TRIANGLE, TRIANGLE, TRIANGLE, TRIANGLE, TRIANGLE, TRIANGLE, TRIANGLE};
	bool bipolar[7] = {true, true, true, true, true, true, true};
	float out = 0.f;

	bool firstRun = true;
	int waitingClock = -1;
	int waitingClockCount = 1;

	bool wait2ndClock = false;

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

	Modulator7() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RATE_PARAM, 0.f, 1.f, 0.5f, "Rate", "Hz", maxRate / minRate, minRate);
		configInput(RATE_INPUT, "Rate");
		configParam(RATE_ATTENUV_PARAM, -1.f, 1.f, 0.f, "Rate CV", "%", 0, 100);
		configSwitch(SYNCSW_PARAM, 0.f, 1.f, 0.f, "Sync", {"Off", "On"});
		configInput(SYNCSW_INPUT, "Sync Toggle");
		configInput(SYNC_INPUT, "Sync");
		configParam(PPC_PARAM, 1, 24.f, 1.f, "Pulses per Cycle");
		paramQuantities[PPC_PARAM]->snapEnabled = true;
		configSwitch(RST_PARAM, 0.f, 1.f, 0.f, "Reset", {"Off", "On"});
		configInput(RST_INPUT, "Reset");
		configOutput(RST_OUTPUT, "Reset");
		configParam(POLY_PARAM, 1.f, 7.f, 7.f, "Poly Chans");
		paramQuantities[POLY_PARAM]->snapEnabled = true;
		configOutput(POLY_OUTPUT, "Poly");
		for (int i = 0; i < 7; i++) {
			configParam(PHASERST_PARAM+i, 0.f, 1.f, 0.f, "Phase Rst", "°", 0, 360);
			configSwitch(WAVEFORM_PARAM+i, 0.f, 2.f, 1.f, "Type", {"Ramp Up", "Triangle", "Ramp Down"});
			configParam(XRATE_PARAM+i, 0.f, 1.f, 0.5f, "xRate", "x", maxXrate / minXrate, minXrate);
			configSwitch(BIPOLAR_PARAM+i, 0.f, 1.f, 0.f, "Bipolar", {"Off", "On"});
			configParam(SCALE_PARAM+i, 0.f, 1.f, 1.f, "Scale", "%", 0, 100);
			configOutput(OUT_OUTPUT+i, "");
		}
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
		rateKnob = 0.5;
		prevRateKnob = 1;
		for (int i = 0; i < 7; i++) {
			waveSlope[i] = 1;
			waveValue[i] = 0;
			xRateKnob[i] = 1;
			prevXrateKnob[i] = 0;
		}
		clockSampleCount = 0;
		clockMaxSample = sampleRate;
		clockPulse = false;
		clockPulseTime = 0;
		rst = 0;
		prevRst = 0;

		waveCoeff = 0;
		syncWaveCoeff = 0;
		firstRun = true;
		waitingClock = -1;
		waitingClockCount = 1;

		sampleCount = 0;
		//prevSampleCount = 0;

		wait2ndClock = false;

	    Module::onReset(e);
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
		//sampleCount = sampleRate;
		sampleRateCoeff = sampleRate / 2;
		//waveCoeff = rate / sampleRateCoeff;
		//
		oneMsTime = (APP->engine->getSampleRate()) / 1000;

	}


	void setPreset(int preset) {
		switch (preset) {

			case 0: // integer *
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.613835);	// 2x
				params[XRATE_PARAM+2].setValue(0.680424);	// 3x
				params[XRATE_PARAM+3].setValue(0.727670);	// 4x
				params[XRATE_PARAM+4].setValue(0.764317);	// 5x
				params[XRATE_PARAM+5].setValue(0.794259);	// 6x
				params[XRATE_PARAM+6].setValue(0.819576);	// 7x
			break;

			case 1: // integer /
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.386165);	// 2x
				params[XRATE_PARAM+2].setValue(0.319576);	// 3x
				params[XRATE_PARAM+3].setValue(0.272330);	// 4x
				params[XRATE_PARAM+4].setValue(0.235683);	// 5x
				params[XRATE_PARAM+5].setValue(0.205741);	// 6x
				params[XRATE_PARAM+6].setValue(0.180424);	// 7x
			break;

			case 2: // even *
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.613835);	// 2x
				params[XRATE_PARAM+2].setValue(0.727670);	// 4x
				params[XRATE_PARAM+3].setValue(0.794259);	// 6x
				params[XRATE_PARAM+4].setValue(0.841505);	// 8x
				params[XRATE_PARAM+5].setValue(0.878152);	// 10x
				params[XRATE_PARAM+6].setValue(0.908095);	// 12x
			break;

			case 3: // even /
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.386165);	// 2x
				params[XRATE_PARAM+2].setValue(0.272330);	// 4x
				params[XRATE_PARAM+3].setValue(0.205741);	// 6x
				params[XRATE_PARAM+4].setValue(0.158495);	// 8x
				params[XRATE_PARAM+5].setValue(0.121848);	// 10x
				params[XRATE_PARAM+6].setValue(0.091905);	// 12x
			break;

			case 4:	// odd +
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.680424);	// 3x
				params[XRATE_PARAM+2].setValue(0.764317);	// 5x
				params[XRATE_PARAM+3].setValue(0.819576);	// 7x
				params[XRATE_PARAM+4].setValue(0.860849);	// 9x
				params[XRATE_PARAM+5].setValue(0.893805);	// 11x
				params[XRATE_PARAM+6].setValue(0.921240);	// 13x
			break;

			case 5:	// odd -
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.319576);	// 3x
				params[XRATE_PARAM+2].setValue(0.235683);	// 5x
				params[XRATE_PARAM+3].setValue(0.180424);	// 7x
				params[XRATE_PARAM+4].setValue(0.139151);	// 9x
				params[XRATE_PARAM+5].setValue(0.106195);	// 11x
				params[XRATE_PARAM+6].setValue(0.078760);	// 13x
			break;

			case 6: // primes *
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.613835);	// 2x
				params[XRATE_PARAM+2].setValue(0.680424);	// 3x
				params[XRATE_PARAM+3].setValue(0.764317);	// 5x
				params[XRATE_PARAM+4].setValue(0.819576);	// 7x
				params[XRATE_PARAM+5].setValue(0.893805);	// 11x
				params[XRATE_PARAM+6].setValue(0.921240);	// 13x
			break;

			case 7:	// prime /
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.386165);	// 2x
				params[XRATE_PARAM+2].setValue(0.319576);	// 3x
				params[XRATE_PARAM+3].setValue(0.235683);	// 5x
				params[XRATE_PARAM+4].setValue(0.180424);	// 7x
				params[XRATE_PARAM+5].setValue(0.106195);	// 11x
				params[XRATE_PARAM+6].setValue(0.078760);	// 13x
			break;

			case 8: // fibonacci *
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.613835);	// 2x
				params[XRATE_PARAM+2].setValue(0.680424);	// 3x
				params[XRATE_PARAM+3].setValue(0.764317);	// 5x
				params[XRATE_PARAM+4].setValue(0.841505);	// 8x
				params[XRATE_PARAM+5].setValue(0.921240);	// 13x
				params[XRATE_PARAM+6].setValue(1);			// 21x
			break;

			case 9:	// fibonacci /
				params[XRATE_PARAM+0].setValue(0.5);		// 1x
				params[XRATE_PARAM+1].setValue(0.386165);	// 2x
				params[XRATE_PARAM+2].setValue(0.319576);	// 3x
				params[XRATE_PARAM+3].setValue(0.235683);	// 5x
				params[XRATE_PARAM+4].setValue(0.158495);	// 8x
				params[XRATE_PARAM+5].setValue(0.078760);	// 13x
				params[XRATE_PARAM+6].setValue(0);			// 21x
			break;
		}

	}

	static float convertXrateToSeconds(float cv) {		
		return minXrate * std::pow(maxXrate / minXrate, cv);
	}

	static float convertRateToSeconds(float cv) {		
		return minRate * std::pow(maxRate / minRate, cv);
	}

	void process(const ProcessArgs& args) override {

		rst = inputs[RST_INPUT].getVoltage() + params[RST_PARAM].getValue();
		if (!rst)
			lights[RST_LIGHT].setBrightness(0);
		else
			lights[RST_LIGHT].setBrightness(1);

		if ((rst >= 1 && prevRst < 1) || firstRun) {

			if (!firstRun) {
				resetPulse = true;
				resetPulseTime = oneMsTime;
			}

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

			for (int i = 0; i < 7; i++) {

				phaseResetValue = params[PHASERST_PARAM+i].getValue();

				switch (waveForm[i]) {
					
					case TRIANGLE:
						if (phaseResetValue < 0.5) {
							waveSlope[i] = 1;
							waveValue[i] = phaseResetValue * 2;
							if (params[BIPOLAR_PARAM+i].getValue())
								waveValue[i] += 0.5;

						} else {
							waveSlope[i] = -1;
							waveValue[i] = 1 - ((phaseResetValue - 0.5) * 2);
							if (params[BIPOLAR_PARAM+i].getValue())
								waveValue[i] -= 0.5;
						}
					break;

					case RAMPUP:
						waveSlope[i] = 1;
						waveValue[i] = phaseResetValue;
						if (params[BIPOLAR_PARAM+i].getValue())
							waveValue[i] += 0.5;
					break;

					case RAMPDOWN:
						waveSlope[i] = -1;
						waveValue[i] = 1 - phaseResetValue;
						if (params[BIPOLAR_PARAM+i].getValue())
							waveValue[i] -= 0.5;
					break;
				}
			}

			clockPulse = true;
			clockPulseTime = oneMsTime;
			clockSampleCount = 0;	

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
			
			syncTrig = inputs[SYNC_INPUT].getVoltage();

			if (syncTrig >= 1 && prevSyncTrig < 1) {

				if (waitingClock == 0) {

					syncWaveCoeff = 2 / sampleCount / params[PPC_PARAM].getValue();

					//prevSampleCount = sampleCount;

					waitingClockCount++;

					if (syncEnabled && waitingClockCount > params[PPC_PARAM].getValue()) {
						clockPulse = true;
						clockPulseTime = oneMsTime;
						clockSampleCount = 0;
						waitingClockCount = 1;
					}

					
				} else if (waitingClock < 0) {

					waitingClock = 1;
					waitingClockCount = 1;

					syncWaveCoeff = 0;

				} else {	// waitingClock = 1

					waitingClockCount++;

					if (waitingClockCount > params[PPC_PARAM].getValue()) {
						waitingClockCount = 1;

						waitingClock = 0;

						syncWaveCoeff = 2 / sampleCount / params[PPC_PARAM].getValue();

						//prevSampleCount = sampleCount;

						if (syncEnabled) {
							clockPulse = true;
							clockPulseTime = oneMsTime;
							clockSampleCount = 0;
						}

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

			clockMaxSample = int(sampleRate / rate);

			if (clockSampleCount >= clockMaxSample) {
				clockPulse = true;
				clockPulseTime = oneMsTime;
				clockSampleCount = 0;		
			}

		} else {

			waveCoeff = syncWaveCoeff;

		}

		clockSampleCount++;

		for (int i = 0; i < 7; i++) {

			waveForm[i] = int(params[WAVEFORM_PARAM+i].getValue());

			xRateKnob[i] = params[XRATE_PARAM+i].getValue();

			if (xRateKnob[i] != prevXrateKnob[i]) {
				xRate[i] = convertXrateToSeconds(xRateKnob[i]);
				prevXrateKnob[i] = xRateKnob[i];
			}

			switch (waveForm[i]) {
				case TRIANGLE:

					waveValue[i] += xRate[i] * waveCoeff * waveSlope[i];
					
					if (waveValue[i] > 1) {
						waveSlope[i] = -1;
						waveValue[i] = 2 - waveValue[i];
					} else if (waveValue[i] < 0) {
						waveSlope[i] = 1;
						waveValue[i] = -waveValue[i];
					}

				break;

				case RAMPUP:

					waveValue[i] += xRate[i] * waveCoeff * waveSlope[i] / 2;

					if (waveValue[i] > 1) {
						waveSlope[i] = 1;
						waveValue[i] = waveValue[i] - 1;
					} else if (waveSlope[i] < 0) {
						waveSlope[i] = 1;
					}

				break;

				case RAMPDOWN:

					waveValue[i] += xRate[i] * waveCoeff * waveSlope[i] / 2;

					if (waveValue[i] < 0) {
						waveSlope[i] = -1;
						waveValue[i] = 1 + waveValue[i];
					} else if (waveSlope[i] > 0) {
						waveSlope[i] = -1;
					}

				break;
			}

			bipolar[i] = params[BIPOLAR_PARAM+i].getValue();
			lights[BIPOLAR_LIGHT+i].setBrightness(bipolar[i]);

			if (bipolar[i])
				out = ((waveValue[i] * 10.f) - 5.f) * params[SCALE_PARAM+i].getValue();
			else
				out = waveValue[i] * 10.f * params[SCALE_PARAM+i].getValue();
				
			outputs[OUT_OUTPUT+i].setVoltage(out);
			outputs[POLY_OUTPUT].setVoltage(out, i);
		}

		polyKnob = int(params[POLY_PARAM].getValue());

		if (polyKnob != 1) {
			outputs[POLY_OUTPUT].setChannels(polyKnob);
		} else {

			if (clockPulse) {
				clockPulseTime--;
				if (clockPulseTime < 0) {
					clockPulse = false;
					outputs[POLY_OUTPUT].setVoltage(0.f);
				} else
					outputs[POLY_OUTPUT].setVoltage(10.f);
			} else {
				outputs[POLY_OUTPUT].setVoltage(0.f);	
			}
			
			outputs[POLY_OUTPUT].setChannels(1);
		}

		if (resetPulse) {
			resetPulseTime--;
			if (resetPulseTime < 0) {
				resetPulse = false;
				outputs[RST_OUTPUT].setVoltage(0.f);
			} else
				outputs[RST_OUTPUT].setVoltage(10.f);
		} else {
			outputs[RST_OUTPUT].setVoltage(0.f);	
		}
		
		
		//debugDisplay = to_string(params[XRATE_PARAM].getValue());
	}
};

/*
struct Modulator7DebugDisplay : TransparentWidget {
	Modulator7 *module;
	int frame = 0;
	Modulator7DebugDisplay() {
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
				nvgTextBox(args.vg, 9, 16,120, module->debugDisplay2.c_str(), NULL);
				nvgTextBox(args.vg, 129, 6,120, module->debugDisplay3.c_str(), NULL);
				//nvgTextBox(args.vg, 129, 16,120, module->debugDisplay4.c_str(), NULL);

			}
		}
		Widget::drawLayer(args, layer);
	}
};
*/

struct Modulator7Widget : ModuleWidget {
	Modulator7Widget(Modulator7* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Modulator7.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	 

		/*
		{
			Modulator7DebugDisplay *display = new Modulator7DebugDisplay();
			display->box.pos = Vec(23, 3);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/

		const float xRtKnob = 12;
		const float yRtKnob = 21;

		const float xRateAtnv = 18.8;
		const float yRateAtnv = 32;

		const float xRateIn = 11;
		const float yRateIn = 39;

		const float xSync = 30.5;
		const float ySync = 20;

		const float xSyncSw = 31;
		const float ySyncSw = 30;

		const float xSyncIn = 37;
		const float ySyncIn = 39;

		const float xPpc = 48;
		const float yPpc = 32;

		const float xRstBut = 41.7;
		const float xRstIn = 52.7;
		const float xRstOut = 63.7;
		const float yRstOut = 15.2;
		const float yRst = 18.2;

		const float xPolyKnob = 63.5;
		const float yPolyKnob = 29.7;

		const float xPoly = 63.5;
		const float yPoly = 39;

		const float yStart = 57;
		constexpr float yStartShift = 10;

		const float xPhR = 7.3;
		const float xType = 20;
		const float xRt = 32.6;
		const float xBi = 42.0;
		const float xScl = 51.6;
		const float xOut = 63.7;

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xRtKnob, yRtKnob)), module, Modulator7::RATE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xRateAtnv, yRateAtnv)), module, Modulator7::RATE_ATTENUV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRateIn, yRateIn)), module, Modulator7::RATE_INPUT));

		addParam(createLightParamCentered<VCVLightBezelLatch<BlueLight>>(mm2px(Vec(xSyncSw, ySyncSw)), module, Modulator7::SYNCSW_PARAM, Modulator7::SYNCSW_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xSyncIn, ySyncIn)), module, Modulator7::SYNCSW_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xSync, ySync)), module, Modulator7::SYNC_INPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPpc, yPpc)), module, Modulator7::PPC_PARAM));
		//addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPhR, yPhR)), module, Modulator7::PHASERST_PARAM));
		addParam(createLightParamCentered<VCVLightBezel<YellowLight>>(mm2px(Vec(xRstBut, yRst)), module, Modulator7::RST_PARAM, Modulator7::RST_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRstIn, yRst)), module, Modulator7::RST_INPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xRstOut, yRstOut)), module, Modulator7::RST_OUTPUT));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPolyKnob, yPolyKnob)), module, Modulator7::POLY_PARAM));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xPoly, yPoly)), module, Modulator7::POLY_OUTPUT));
		
		for (int i = 0; i < 7; i++) {
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xPhR, yStart + (yStartShift * i))), module, Modulator7::PHASERST_PARAM+i));
			addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(xType, yStart + (yStartShift * i))), module, Modulator7::WAVEFORM_PARAM+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xRt, yStart + (yStartShift * i))), module, Modulator7::XRATE_PARAM+i));
			addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(xBi, yStart + (yStartShift * i))), module, Modulator7::BIPOLAR_PARAM+i, Modulator7::BIPOLAR_LIGHT+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xScl, yStart + (yStartShift * i))), module, Modulator7::SCALE_PARAM+i));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yStart + (yStartShift * i))), module, Modulator7::OUT_OUTPUT+i));
		}
	}

	void appendContextMenu(Menu* menu) override {
		Modulator7* module = dynamic_cast<Modulator7*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Sync settings"));
		menu->addChild(createBoolPtrMenuItem("Wait full clock after reset", "", &module->wait2ndClock));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("xRate Presets"));

		menu->addChild(createMenuItem("Integer *", "", [=]() {module->setPreset(0);}));
		menu->addChild(createMenuItem("Integer /", "", [=]() {module->setPreset(1);}));
		menu->addChild(createMenuItem("Even *", "", [=]() {module->setPreset(2);}));
		menu->addChild(createMenuItem("Even /", "", [=]() {module->setPreset(3);}));
		menu->addChild(createMenuItem("Odd *", "", [=]() {module->setPreset(4);}));
		menu->addChild(createMenuItem("Odd /", "", [=]() {module->setPreset(5);}));		
		menu->addChild(createMenuItem("Prime *", "", [=]() {module->setPreset(6);}));
		menu->addChild(createMenuItem("Prime /", "", [=]() {module->setPreset(7);}));
		menu->addChild(createMenuItem("Fibonacci *", "", [=]() {module->setPreset(8);}));
		menu->addChild(createMenuItem("Fibonacci /", "", [=]() {module->setPreset(9);}));

	}
};

Model* modelModulator7 = createModel<Modulator7, Modulator7Widget>("Modulator7");