//#define FORWARD 0
//#define REVERSE 1

#include "plugin.hpp"

//using namespace std;

struct tpLength : ParamQuantity {
	std::string getDisplayValueString() override {
		const std::string valueDisplay[8] = {"2", "3", "4", "5", "6", "8", "12", "16"};
		return valueDisplay[int(getValue())];
	}
};

struct RandLoops : Module {
	enum ParamId {
		CTRL_PARAM,
		LENGTH_PARAM,
		SCALE_PARAM,
		DEL_BUTTON,
		ADD_BUTTON,
		RND_BUTTON,
		PARAMS_LEN
	};
	enum InputId {
		CTRL_INPUT,
		DEL_INPUT,
		ADD_INPUT,
		CLK_INPUT,
		RST_INPUT,
		RND_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		PULSE_OUTPUT,
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		DEL_LIGHT,
		ADD_LIGHT,
		RND_LIGHT,
		ENUMS(REGISTER_LIGHT, 8),
		LIGHTS_LEN
	};

	float oneMsTime = (APP->engine->getSampleRate()) / 1000;

	float delTrig = 0;
	float prevDelTrig = 0;
	bool delWait = false;

	float addTrig = 0;
	float prevAddTrig = 0;
	bool addWait = false;

	float clock = 0;
	float prevClock = 0;

	float rstValue = 0;
	float prevRstValue = 0;

	float rndTrig = 0;
	float prevRndTrig = 0;
	bool rndWait = false;
	
	float controlValue = 0;

	float probCtrl = 0;
	float probCtrlRnd = 0;
	float probRegister = 0;

	float out = 0;
	
	bool pulse = false;
	float pulseTime = 0;

	bool incomingRegister = false;

	int tableLength[8] = {1, 2, 3, 4, 5, 7, 11, 15};
	float tableVolts[8] = {0.039 , 0.078, 0.157, 0.314, 0.627, 1.255, 2.51, 5.02};
	//float tableVolts[8] = {5.02, 2.51, 1.255, 0.627, 0.314, 0.157, 0.078, 0.039};

	//int loopLength = 4;

	bool bufferedAddDel = true;
	bool bufferedRandom = true;
	bool initStart = false;

	bool shiftRegister[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	RandLoops() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(CTRL_PARAM, -1, 1.f, 0.f, "Control");
		configInput(CTRL_INPUT, "Ctrl CV");

		configParam<tpLength>(LENGTH_PARAM, 0.f, 7.f, 5.f, "Length");
		paramQuantities[LENGTH_PARAM]->snapEnabled = true;

		configParam(SCALE_PARAM, 0.f, 1.f, 1.f, "Scale", "%", 0, 100);

		configSwitch(DEL_BUTTON, 0.f, 1.f, 0.f, "Delete", {"OFF", "ON"});
		configSwitch(ADD_BUTTON, 0.f, 1.f, 0.f, "Add", {"OFF", "ON"});
		configSwitch(RND_BUTTON, 0.f, 1.f, 0.f, "Random", {"OFF", "ON"});

		configInput(DEL_INPUT, "Delete");
		configInput(ADD_INPUT, "Add");
		configInput(RND_INPUT, "Random");

		configInput(CLK_INPUT, "Clock");
		configInput(RST_INPUT, "Reset");

		configOutput(PULSE_OUTPUT, "Pulse");
		configOutput(OUT_OUTPUT, "Output");

		calcVoltage();

	}

	void onReset(const ResetEvent &e) override {
		for (int i=0; i < 8; i++) {
			shiftRegister[i] = false;
			lights[REGISTER_LIGHT+i].setBrightness(0.f);
		}

		for (int i=8; i < 16; i++)
			shiftRegister[i] = false;

		rndWait = false;
		addWait = false;
		delWait = false;

		Module::onReset(e);
	}

	void onSampleRateChange() override {
		oneMsTime = (APP->engine->getSampleRate()) / 1000;
	}

	
	json_t* dataToJson() override {

		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "bufferedAddDel", json_boolean(bufferedAddDel));
		json_object_set_new(rootJ, "bufferedRandom", json_boolean(bufferedRandom));
		json_object_set_new(rootJ, "initStart", json_boolean(initStart));
		if (initStart) {
			json_object_set_new(rootJ, "r0", json_boolean(false));
			json_object_set_new(rootJ, "r1", json_boolean(false));
			json_object_set_new(rootJ, "r2", json_boolean(false));
			json_object_set_new(rootJ, "r3", json_boolean(false));
			json_object_set_new(rootJ, "r4", json_boolean(false));
			json_object_set_new(rootJ, "r5", json_boolean(false));
			json_object_set_new(rootJ, "r6", json_boolean(false));
			json_object_set_new(rootJ, "r7", json_boolean(false));
			json_object_set_new(rootJ, "r8", json_boolean(false));
			json_object_set_new(rootJ, "r9", json_boolean(false));
			json_object_set_new(rootJ, "r10", json_boolean(false));
			json_object_set_new(rootJ, "r11", json_boolean(false));
			json_object_set_new(rootJ, "r12", json_boolean(false));
			json_object_set_new(rootJ, "r13", json_boolean(false));
			json_object_set_new(rootJ, "r14", json_boolean(false));
			json_object_set_new(rootJ, "r15", json_boolean(false));
		} else {
			json_object_set_new(rootJ, "r0", json_boolean(shiftRegister[0]));
			json_object_set_new(rootJ, "r1", json_boolean(shiftRegister[1]));
			json_object_set_new(rootJ, "r2", json_boolean(shiftRegister[2]));
			json_object_set_new(rootJ, "r3", json_boolean(shiftRegister[3]));
			json_object_set_new(rootJ, "r4", json_boolean(shiftRegister[4]));
			json_object_set_new(rootJ, "r5", json_boolean(shiftRegister[5]));
			json_object_set_new(rootJ, "r6", json_boolean(shiftRegister[6]));
			json_object_set_new(rootJ, "r7", json_boolean(shiftRegister[7]));
			json_object_set_new(rootJ, "r8", json_boolean(shiftRegister[8]));
			json_object_set_new(rootJ, "r9", json_boolean(shiftRegister[9]));
			json_object_set_new(rootJ, "r10", json_boolean(shiftRegister[10]));
			json_object_set_new(rootJ, "r11", json_boolean(shiftRegister[11]));
			json_object_set_new(rootJ, "r12", json_boolean(shiftRegister[12]));
			json_object_set_new(rootJ, "r13", json_boolean(shiftRegister[13]));
			json_object_set_new(rootJ, "r14", json_boolean(shiftRegister[14]));
			json_object_set_new(rootJ, "r15", json_boolean(shiftRegister[15]));
		}
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		
		json_t* bufferedAddDelJ = json_object_get(rootJ, "bufferedAddDel");
		if (bufferedAddDelJ)
			bufferedAddDel = json_boolean_value(bufferedAddDelJ);

		json_t* bufferedRandomJ = json_object_get(rootJ, "bufferedRandom");
		if (bufferedRandomJ)
			bufferedRandom = json_boolean_value(bufferedRandomJ);

		json_t* initStartJ = json_object_get(rootJ, "initStart");
		if (initStartJ) {
			initStart = json_boolean_value(initStartJ);
			if (!initStart) {
				json_t* r0J = json_object_get(rootJ, "r0");
				if (r0J)
					shiftRegister[0] = json_boolean_value(r0J);

				json_t* r1J = json_object_get(rootJ, "r1");
				if (r1J)
					shiftRegister[1] = json_boolean_value(r1J);

				json_t* r2J = json_object_get(rootJ, "r2");
				if (r2J)
					shiftRegister[2] = json_boolean_value(r2J);

				json_t* r3J = json_object_get(rootJ, "r3");
				if (r3J)
					shiftRegister[3] = json_boolean_value(r3J);

				json_t* r4J = json_object_get(rootJ, "r4");
				if (r4J)
					shiftRegister[4] = json_boolean_value(r4J);

				json_t* r5J = json_object_get(rootJ, "r5");
				if (r5J)
					shiftRegister[5] = json_boolean_value(r5J);

				json_t* r6J = json_object_get(rootJ, "r6");
				if (r6J)
					shiftRegister[6] = json_boolean_value(r6J);

				json_t* r7J = json_object_get(rootJ, "r7");
				if (r7J)
					shiftRegister[7] = json_boolean_value(r7J);

				json_t* r8J = json_object_get(rootJ, "r8");
				if (r8J)
					shiftRegister[8] = json_boolean_value(r8J);

				json_t* r9J = json_object_get(rootJ, "r9");
				if (r9J)
					shiftRegister[9] = json_boolean_value(r9J);

				json_t* r10J = json_object_get(rootJ, "r10");
				if (r10J)
					shiftRegister[10] = json_boolean_value(r10J);

				json_t* r11J = json_object_get(rootJ, "r11");
				if (r11J)
					shiftRegister[11] = json_boolean_value(r11J);

				json_t* r12J = json_object_get(rootJ, "r12");
				if (r12J)
					shiftRegister[12] = json_boolean_value(r12J);

				json_t* r13J = json_object_get(rootJ, "r13");
				if (r13J)
					shiftRegister[13] = json_boolean_value(r13J);

				json_t* r14J = json_object_get(rootJ, "r14");
				if (r14J)
					shiftRegister[14] = json_boolean_value(r14J);

				json_t* r15J = json_object_get(rootJ, "r15");
				if (r15J)
					shiftRegister[15] = json_boolean_value(r15J);

			}
		}
	}
	


	void inline calcVoltage() {
		out = 0;
		for (int i=0; i < 8; i++) {
			if (shiftRegister[i])
				out += tableVolts[i];
		}
	}

	void inline calcRandom() {
		int rndLength = 1 + tableLength[int(params[LENGTH_PARAM].getValue())];
		switch (rndLength) {
			case 2:
				for (int i = 0; i < 2; i++) {
					probRegister = random::uniform();
					if (probRegister > 0.5)
						shiftRegister[i] = true;
					else
						shiftRegister[i] = false;
				}
				shiftRegister[2] = shiftRegister[0];
				shiftRegister[3] = shiftRegister[1];
				shiftRegister[4] = shiftRegister[0];
				shiftRegister[5] = shiftRegister[1];
				shiftRegister[6] = shiftRegister[0];
				shiftRegister[7] = shiftRegister[1];
				shiftRegister[8] = shiftRegister[0];
				shiftRegister[9] = shiftRegister[1];
				shiftRegister[10] = shiftRegister[0];
				shiftRegister[11] = shiftRegister[1];
				shiftRegister[12] = shiftRegister[0];
				shiftRegister[13] = shiftRegister[1];
				shiftRegister[14] = shiftRegister[0];
				shiftRegister[15] = shiftRegister[1];

			break;

			case 3:
				for (int i = 0; i < 3; i++) {
					probRegister = random::uniform();
					if (probRegister > 0.5)
						shiftRegister[i] = true;
					else
						shiftRegister[i] = false;
				}
				shiftRegister[3] = shiftRegister[0];
				shiftRegister[4] = shiftRegister[1];
				shiftRegister[5] = shiftRegister[2];
				shiftRegister[6] = shiftRegister[0];
				shiftRegister[7] = shiftRegister[1];
				shiftRegister[8] = shiftRegister[2];
				shiftRegister[9] = shiftRegister[0];
				shiftRegister[10] = shiftRegister[1];
				shiftRegister[11] = shiftRegister[2];
				shiftRegister[12] = shiftRegister[0];
				shiftRegister[13] = shiftRegister[1];
				shiftRegister[14] = shiftRegister[2];
				shiftRegister[15] = shiftRegister[0];

			break;

			case 4:
				for (int i = 0; i < 4; i++) {
					probRegister = random::uniform();
					if (probRegister > 0.5)
						shiftRegister[i] = true;
					else
						shiftRegister[i] = false;
				}
				shiftRegister[4] = shiftRegister[0];
				shiftRegister[5] = shiftRegister[1];
				shiftRegister[6] = shiftRegister[2];
				shiftRegister[7] = shiftRegister[3];
				shiftRegister[8] = shiftRegister[0];
				shiftRegister[9] = shiftRegister[1];
				shiftRegister[10] = shiftRegister[2];
				shiftRegister[11] = shiftRegister[3];
				shiftRegister[12] = shiftRegister[0];
				shiftRegister[13] = shiftRegister[1];
				shiftRegister[14] = shiftRegister[2];
				shiftRegister[15] = shiftRegister[3];

			break;

			case 5:
				for (int i = 0; i < 5; i++) {
					probRegister = random::uniform();
					if (probRegister > 0.5)
						shiftRegister[i] = true;
					else
						shiftRegister[i] = false;
				}
				shiftRegister[5] = shiftRegister[0];
				shiftRegister[6] = shiftRegister[1];
				shiftRegister[7] = shiftRegister[2];
				shiftRegister[8] = shiftRegister[3];
				shiftRegister[9] = shiftRegister[4];
				shiftRegister[10] = shiftRegister[0];
				shiftRegister[11] = shiftRegister[1];
				shiftRegister[12] = shiftRegister[2];
				shiftRegister[13] = shiftRegister[3];
				shiftRegister[14] = shiftRegister[4];
				shiftRegister[15] = shiftRegister[0];

			break;

			case 6:
				for (int i = 0; i < 6; i++) {
					probRegister = random::uniform();
					if (probRegister > 0.5)
						shiftRegister[i] = true;
					else
						shiftRegister[i] = false;
				}
				shiftRegister[6] = shiftRegister[0];
				shiftRegister[7] = shiftRegister[1];
				shiftRegister[8] = shiftRegister[2];
				shiftRegister[9] = shiftRegister[3];
				shiftRegister[10] = shiftRegister[4];
				shiftRegister[11] = shiftRegister[5];
				shiftRegister[12] = shiftRegister[0];
				shiftRegister[13] = shiftRegister[1];
				shiftRegister[14] = shiftRegister[2];
				shiftRegister[15] = shiftRegister[3];
			break;

			case 8:
				for (int i = 0; i < 8; i++) {
					probRegister = random::uniform();
					if (probRegister > 0.5)
						shiftRegister[i] = true;
					else
						shiftRegister[i] = false;
				}
				shiftRegister[8] = shiftRegister[0];
				shiftRegister[9] = shiftRegister[1];
				shiftRegister[10] = shiftRegister[2];
				shiftRegister[11] = shiftRegister[3];
				shiftRegister[12] = shiftRegister[4];
				shiftRegister[13] = shiftRegister[5];
				shiftRegister[14] = shiftRegister[6];
				shiftRegister[15] = shiftRegister[7];
			break;

			case 12:
				for (int i = 0; i < 12; i++) {
					probRegister = random::uniform();
					if (probRegister > 0.5)
						shiftRegister[i] = true;
					else
						shiftRegister[i] = false;
				}
				shiftRegister[12] = shiftRegister[0];
				shiftRegister[13] = shiftRegister[1];
				shiftRegister[14] = shiftRegister[2];
				shiftRegister[15] = shiftRegister[3];
			break;

			case 16:
				for (int i = 0; i < 16; i++) {
					probRegister = random::uniform();
					if (probRegister > 0.5)
						shiftRegister[i] = true;
					else
						shiftRegister[i] = false;
				}
			break;
		}

		for (int i=0; i<8; i++)
			lights[REGISTER_LIGHT+i].setBrightness(shiftRegister[i]);
	}
	
	void process(const ProcessArgs& args) override {

		// -------------------------------- reset trigger

		rstValue = inputs[RST_INPUT].getVoltage();
		if (rstValue >= 1.f && prevRstValue < 1.f) {
			for (int i = 0; i < 8; i++) {
				shiftRegister[i] = false;
				lights[REGISTER_LIGHT+i].setBrightness(0.f);
			}

			for (int i = 8; i < 16; i++)
				shiftRegister[i] = false;

			rndWait = false;
			addWait = false;
			delWait = false;

			lights[RND_LIGHT].setBrightness(0.f);
			lights[ADD_LIGHT].setBrightness(0.f);
			lights[DEL_LIGHT].setBrightness(0.f);

		}
		prevRstValue = rstValue;

		// -------------------------------- random trigger

		rndTrig = inputs[RND_INPUT].getVoltage() + params[RND_BUTTON].getValue();
		if (rndTrig >= 1.f && prevRndTrig < 1.f) {
			rndTrig = 1;
			if (bufferedRandom) {
				rndWait = true;
				lights[RND_LIGHT].setBrightness(1.f);
			} else {
				calcRandom();
				rndWait = false;
			}
		}
		prevRndTrig = rndTrig;

		if (!bufferedRandom)
			lights[RND_LIGHT].setBrightness(rndTrig);

		// -------------------------------- del trigger

		delTrig = inputs[DEL_INPUT].getVoltage() + params[DEL_BUTTON].getValue();
		
		if (bufferedAddDel) {
			if (delTrig >= 1.f && prevDelTrig < 1.f) {
				delTrig = 1;
				delWait = true;
				lights[DEL_LIGHT].setBrightness(1.f);
			}
		} else {
			if (delTrig >= 1.f) {
				delTrig = 1;
				delWait = true;
			}
			lights[DEL_LIGHT].setBrightness(delTrig);
		}
		
		prevDelTrig = delTrig;
		
		// -------------------------------- add trigger

		addTrig = inputs[ADD_INPUT].getVoltage() + params[ADD_BUTTON].getValue();
		
		if (bufferedAddDel) {
			if (addTrig >= 1.f && prevAddTrig < 1.f) {
				addTrig = 1;
				addWait = true;
				lights[ADD_LIGHT].setBrightness(1.f);
			}
		} else {
			if (addTrig >= 1.f) {
				addTrig = 1;
				addWait = true;
			}
			lights[ADD_LIGHT].setBrightness(addTrig);
		}

		prevAddTrig = addTrig;

		// -------------------------------- clock trigger

		clock = inputs[CLK_INPUT].getVoltage();
		if (clock >= 1.f && prevClock < 1.f) {
			if (rndWait && bufferedRandom) {
				calcRandom();
				rndWait = false;
				lights[RND_LIGHT].setBrightness(0.f);
			}

			if (addWait) {

				incomingRegister = true;
				addWait = false;
				if (bufferedAddDel)
					lights[ADD_LIGHT].setBrightness(0.f);

			} else if (delWait) {

				incomingRegister = false;
				delWait = false;
				if (bufferedAddDel)
					lights[DEL_LIGHT].setBrightness(0.f);

			} else {

				controlValue = params[CTRL_PARAM].getValue() + (inputs[CTRL_INPUT].getVoltage() / 10.f);
				if (controlValue < -1.f)
					controlValue = -1.f;
				else if (controlValue > 1.f)
					controlValue = 1.f;

				probCtrl = int(abs(controlValue * 10));
				probCtrlRnd = int(random::uniform() * 10);
				if (probCtrlRnd > probCtrl) {
					probRegister = random::uniform();
					if (probRegister > 0.5)
						incomingRegister = true;
					else
						incomingRegister = false;
				} else {
					incomingRegister = shiftRegister[tableLength[int(params[LENGTH_PARAM].getValue())]];
					if (controlValue < 0)
						incomingRegister = !incomingRegister;
				}
			}

			shiftRegister[15] = shiftRegister[14];
			shiftRegister[14] = shiftRegister[13];
			shiftRegister[13] = shiftRegister[12];
			shiftRegister[12] = shiftRegister[11];
			shiftRegister[11] = shiftRegister[10];
			shiftRegister[10] = shiftRegister[9];
			shiftRegister[9] = shiftRegister[8];
			shiftRegister[8] = shiftRegister[7];

			shiftRegister[7] = shiftRegister[6];
			shiftRegister[6] = shiftRegister[5];
			shiftRegister[5] = shiftRegister[4];
			shiftRegister[4] = shiftRegister[3];
			shiftRegister[3] = shiftRegister[2];
			shiftRegister[2] = shiftRegister[1];
			shiftRegister[1] = shiftRegister[0];
			shiftRegister[0] = incomingRegister;

			calcVoltage();

			if (shiftRegister[0]) {
				pulse = true;
				pulseTime = oneMsTime;
				outputs[PULSE_OUTPUT].setVoltage(10.f);
			}

		}
		prevClock = clock;

		for (int i = 0; i < 8; i++)
			lights[REGISTER_LIGHT+i].setBrightness(shiftRegister[i]);

		outputs[OUT_OUTPUT].setVoltage(out * params[SCALE_PARAM].getValue());

		if (pulse) {
			pulseTime--;
			if (pulseTime < 0) {
				pulse = false;
				outputs[PULSE_OUTPUT].setVoltage(0.f);	
			}
		}

		if (!bufferedAddDel) {
			delWait = false;
			addWait = false;
		}

		if (!bufferedRandom) {
			rndWait = false;
		}

	}
};

struct RandLoopsWidget : ModuleWidget {
	RandLoopsWidget(RandLoops* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RandLoops.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const float xCtrl = 26;
		const float yCtrl = 20.5;

		const float xCtrlCV = 10;
		const float yCtrlCV = 24;

		const float xLength = 11.7;
		const float yLength = 44.5;

		const float xScale = 30.9;
		const float yScale = 49.4;

		const float xDelBut = 8.5;
		const float yDelBut = 69.5;

		const float xAddBut = 20.5;
		const float yAddBut = 69.5;

		const float xRndBut = 32.5;
		const float yRndBut = 69.5;

		const float xDelIn = 8.5;
		const float yDelIn = 79.5;

		const float xAddIn = 20.5;
		const float yAddIn = 79.5;

		const float xRndIn = 32.5;
		const float yRndIn = 79.5;

		const float xClk = 11.5;
		const float yClk = 98.5;

		const float xRst = 29.5;
		const float yRst = 98.5;

		

		const float xPulse = 11;
		const float yPulse = 116;

		const float xOut = 30;
		const float yOut = 116;

		const float xLgStart = 24.5;
		const float xLgShift = 4;
		const float yLgStart1 = 32;
		const float yLgStart2 = 36;
		//const float yLgStart3 = 40;
		//const float yLgStart4 = 44;
		
		addParam(createParamCentered<SickoBigKnob>(mm2px(Vec(xCtrl, yCtrl)), module, RandLoops::CTRL_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCtrlCV, yCtrlCV)), module, RandLoops::CTRL_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xLength, yLength)), module, RandLoops::LENGTH_PARAM));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xScale, yScale)), module, RandLoops::SCALE_PARAM));

		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xDelBut, yDelBut)), module, RandLoops::DEL_BUTTON, RandLoops::DEL_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<GreenLight>>(mm2px(Vec(xAddBut, yAddBut)), module, RandLoops::ADD_BUTTON, RandLoops::ADD_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xRndBut, yRndBut)), module, RandLoops::RND_BUTTON, RandLoops::RND_LIGHT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xDelIn, yDelIn)), module, RandLoops::DEL_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xAddIn, yAddIn)), module, RandLoops::ADD_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRndIn, yRndIn)), module, RandLoops::RND_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xClk, yClk)), module, RandLoops::CLK_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRst, yRst)), module, RandLoops::RST_INPUT));
		
		
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart, yLgStart1)), module, RandLoops::REGISTER_LIGHT+0));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + xLgShift, yLgStart1)), module, RandLoops::REGISTER_LIGHT+1));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart1)), module, RandLoops::REGISTER_LIGHT+2));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart1)), module, RandLoops::REGISTER_LIGHT+3));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart, yLgStart2)), module, RandLoops::REGISTER_LIGHT+4));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + xLgShift, yLgStart2)), module, RandLoops::REGISTER_LIGHT+5));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart2)), module, RandLoops::REGISTER_LIGHT+6));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart2)), module, RandLoops::REGISTER_LIGHT+7));

		/*
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart, yLgStart3)), module, RandLoops::REGISTER_LIGHT+8));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + xLgShift, yLgStart3)), module, RandLoops::REGISTER_LIGHT+9));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart3)), module, RandLoops::REGISTER_LIGHT+10));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart3)), module, RandLoops::REGISTER_LIGHT+11));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart, yLgStart4)), module, RandLoops::REGISTER_LIGHT+12));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + xLgShift, yLgStart4)), module, RandLoops::REGISTER_LIGHT+13));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 2), yLgStart4)), module, RandLoops::REGISTER_LIGHT+14));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(xLgStart + (xLgShift * 3), yLgStart4)), module, RandLoops::REGISTER_LIGHT+15));
		*/

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xPulse, yPulse)), module, RandLoops::PULSE_OUTPUT));
		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yOut)), module, RandLoops::OUT_OUTPUT));

	}

	
	void appendContextMenu(Menu* menu) override {
		RandLoops* module = dynamic_cast<RandLoops*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Buffered Del/Add", "", &module->bufferedAddDel));
		menu->addChild(createBoolPtrMenuItem("Buffered Random", "", &module->bufferedRandom));
		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));
	}
	

};

Model* modelRandLoops = createModel<RandLoops, RandLoopsWidget>("RandLoops");